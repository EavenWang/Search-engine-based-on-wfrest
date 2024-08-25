#include "../include/WebPageQuery.h"
#include "../../tinyxml/tinyxml2.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <fstream>
#include <map>
#include <math.h>
#include <wfrest/json.hpp>

using std::ifstream;
using std::cerr;
using std::istringstream;
using std::cout;
using std::unordered_set;
using std::ifstream;
using std::map;

using namespace tinyxml2;


string dealChinesehWord(const string &word)
{
    string newWord;
    for (size_t idx = 0; idx != word.size(); idx++)
    {   
        if (word[idx] != '\r' && word[idx] != '\n' && word[idx] != ' ')
        {
            newWord += word[idx];
        }
    }
    return newWord;
}

void readChineseStopWords(unordered_set<string> & stopWordsSet){

    ifstream ifs("/home/mylinux/homework/project/files/yuliao/stop_words_zh.txt");

    if(!ifs){
        cerr << "error in readChineseStopWords";
        return;
    }

    string word;
    string newWord;
    while(getline(ifs,word)){
        newWord = dealChinesehWord(word);
        stopWordsSet.insert(newWord);
    }
    ifs.close();
}

void dealEnglishWord(string &word)
{
    for (size_t idx = 0; idx != word.size(); idx++)
    {
        if (!isalpha(word[idx]))
        {
            word[idx] = ' ';
        }else{
            word[idx] = tolower(word[idx]);
        }
    }
}

void readEnglishStopWords(unordered_set<string> & stopWordsSet){
    ifstream ifs("/home/mylinux/homework/project/files/yuliao/stop_words_eng.txt");

    if(!ifs){
        cerr << "error in readEnglishStopWords";
        return;
    }

    string word;
    string line;
    while (getline(ifs, line))
    {
        dealEnglishWord(line);
        istringstream iss(line);
        string word;
        while (iss >> word)
        {
            if (string() != word)
            {
                stopWordsSet.insert(word);
            }
        }
    }
    ifs.close();
}

WebPageQuery::WebPageQuery(SplitTool * tool)
:_tool(tool)
{
    ifstream ifs("/home/mylinux/homework/project/data/newoffset.dat");
    if(!ifs){
        cerr << "error in WebPage\n";
    }

    string line;

    while(getline(ifs,line)){
        istringstream iss(line);

        int i = 0;
        int temp[3];

        string number;

        while(iss >> number){
            temp[i] = atoi(number.c_str());
            i++;
        }
        _offsetLib.push_back({temp[1],temp[2]});
    }

    ifs.close();

    ifs.open("/home/mylinux/homework/project/data/invertIndex.dat");

    while(getline(ifs,line)){
        istringstream iss(line);

        int i = 0;
        string word;
        string temp;
        double weight;
        int id;
        while(iss >> temp){
            if(i == 0){
                word = temp;
            }else if(i == 1){
                id = atoi(temp.c_str());
            }else if(i == 2){
                weight = atof(temp.c_str());
            }
            i++;
        }
        _invertIndex[word].insert({id,weight});
    }

    ifs.close();

    readChineseStopWords(_stopCn);
    readEnglishStopWords(_stopEn);

    // for(const auto & p : _offsetLib){
    //     cout << p.first << " " << p.second << "\n";
    //     break;
    // }

    // for(const auto & p : _invertIndex){
    //     cout << p.first << " ";
    //     break;
    // }
}

vector<string> WebPageQuery::doQuery(const string & str){
    vector<string> words = _tool->cut(str);

    vector<string> result;

    for(const auto & ele : words){
        if(_stopCn.find(ele) != _stopCn.end() && _stopEn.find(ele) != _stopEn.end()){
            result.push_back(ele);
        }
    }

    return words;
}


//计算每篇文章和用户输入的余弦相似度
vector<pair<int,double>> WebPageQuery::getCosinSimilarity(const vector<string> & queryWords){

    unordered_map<int,set<pair<string,double>>> intersection;
    unordered_map<int,map<string,double>>  resultMap;

    unordered_map<string,int> wordFreq; //记录每个词在用户输入关键词中的频次 

    for(const auto & word : queryWords){
        wordFreq[word]++;
        for(const auto & ele : _invertIndex[word]){
            intersection[ele.first].insert({word,ele.second});
        }
    }

    vector<WebPage> webs;
    for(const auto & ele : intersection){
        int id = ele.first;
        if(intersection[id].size() >= queryWords.size()){

            for(const pair<string,double> & word : intersection[id]){
                resultMap[id].insert(word);
            } 
        }
    }

    //计算用户输入关键词的权重
    unordered_map<string,double> wordWeight;
    double userSumWeight;
    for(const auto & p : queryWords){
        double TF = wordFreq[p];
        double DF = 1;
        double IDF = log2(1 / (DF + 1) + 1);
        double weight = TF * IDF;
        wordWeight[p] = weight;
        userSumWeight += (weight * weight);
    }


    //计算每篇文章与用户输入关键词的余弦值
    vector<pair<int,double>> cosinSimilarity;
    double docSumWeight;
    for(const auto & p : resultMap){
        int id = p.first;

        double cosinRec;
        for(const auto & wordW : p.second){
            string word = wordW.first;
            double weight = wordW.second;
            docSumWeight += (weight * weight);
            cosinRec += (weight * wordWeight[word]);
        }
        cosinRec = cosinRec / (sqrt(userSumWeight) + sqrt(docSumWeight));
        cosinSimilarity.push_back({id,cosinRec});
    }
    return cosinSimilarity;
}

//根据文章id和id对应的余弦相似度，搜索磁盘上的网页，插入webPage优先队列
void WebPageQuery::executeQuery(vector<pair<int,double>> & docSimilarity){
    
    ifstream ifs("/home/mylinux/homework/project/data/newripepage.dat");

    if(!ifs){
        cerr << "error in executeQuery\n";
    }

    for(const auto & p : docSimilarity){
        int pos = _offsetLib[p.first].first;    //文章位置
        int length = _offsetLib[p.first].second;

        char article[length + 1];
        memset(article,0,sizeof(article));
        ifs.seekg(pos);
        ifs.read(article,length);

        WebPage webPage;

        webPage.cosinSimilarity = p.second;
        webPage._doc = string(article);

        _priWebs.push(webPage);
    }

}


void WebPageQuery::response(wfrest::HttpResp * resp,SeriesWork * series,string search){
    nlohmann::json respJs;
    if(_priWebs.size() == 0) return;
    while(_priWebs.size()){
        respJs["result"].push_back(_priWebs.top()._doc);
        _priWebs.pop();
    }

    WFRedisTask * redisTask = WFTaskFactory::create_redis_task(
            "redis://127.0.0.1:6379",10,nullptr
    );
    redisTask->get_req()->set_request("set",{search,respJs.dump()});
    series->push_back(redisTask);
    resp->append_output_body(respJs.dump());
}