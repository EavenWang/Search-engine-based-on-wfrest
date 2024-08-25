#include <signal.h>
#include <iostream>
#include <workflow/WFFacilities.h>
#include <wfrest/HttpServer.h>
using std::cout;
using std::map;
using std::string;

static WFFacilities::WaitGroup waitGroup(1);

void sighandler(int signum){
    cout << "done!\n";
    waitGroup.done();
}
int main(){
    signal(SIGINT,sighandler);
    wfrest::HttpServer server;

    server.GET("/search",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp){
        resp->File("/home/mylinux/homework/project/online/html/client.html");
    });

    server.GET("/sugesstions",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp){
        cout << "hello\n";
        resp->String("<html>Hello</html>"); 
        resp->headers["Content-Type"] = "text/html";
    });
    server.GET("/world",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp){
        cout << "world\n";
    });
    server.GET("/login",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp){
        resp->File("postform.html");
    });
    server.POST("/login",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp, SeriesWork * series){
 
        map<string,string>  mapForm = req->form_kv();
        for(auto pair:mapForm){
            cout << "key = " << pair.first << " value = " << pair.second << "\n";
        }
        string password =  mapForm["password"];
        WFRedisTask * redisTask = WFTaskFactory::create_redis_task(
            "redis://127.0.0.1:6379",10,
            [resp,password](WFRedisTask *redisTask){
                protocol::RedisResponse *respRedis = redisTask->get_resp();
                protocol::RedisValue result;
                respRedis->get_result(result);
                if(result.is_string() && result.string_value() == password){
                    resp->String("success");
                }
                else{
                    resp->String("fail");
                }
            }
        );
        redisTask->get_req()->set_request("HGET",{"user",mapForm["username"]});
        series->push_back(redisTask);
    });

    server.POST("/upload",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp){
        map<string,std::pair<string,string>> & formData = req->form();
        
        int fd = open(formData["file"].first.c_str(), O_RDWR|O_CREAT, 0666);
        write(fd,formData["file"].second.c_str(),formData["file"].second.length());
        close(fd);
    });


    // 192.168.72.128:12345/login?username=niqiu&password=123
    //server.POST("/login",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp){
        // const map<string,string> &queryMap = req->query_list();
        // for(auto pair:queryMap){
        //     cout << "key = " << pair.first << " value = " << pair.second << "\n";
        // }
    //});
    if(server.track().start(12345) == 0){
        // start是非阻塞的
        server.list_routes();
        waitGroup.wait();
        cout << "finished!\n";
        server.stop();
        return 0;
    }
    else{
        perror("server start fail!");
        return -1;
    }
}