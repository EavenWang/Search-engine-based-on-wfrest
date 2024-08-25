#ifndef _CONFIGURATION_H__
#define _CONFIGURATION_H__

#include <string>
#include <fstream>
#include <map>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

using std::string;
using std::ifstream;
using std::map;
using std::cerr;

class Configuration
{
public:
    static Configuration * getInstance(){
        if(_conf == nullptr){
            _conf = new Configuration("/home/mylinux/homework/project/offline/conf");
        }
        return _conf;
    }

    static void destroy(){
        if(_conf){
            delete _conf;
            _conf = nullptr;
        }
    }

    map<string,string> getConfigMap(){
        return _configs;
    }


private:
    Configuration(const string & configPath)
    :_configFilePath(configPath)
    {
        DIR* dir = opendir(_configFilePath.c_str());

        if(dir == nullptr){
            cerr << "error in Configuration\n";
        }

        struct dirent * ptr;

        while((ptr = readdir(dir)) != nullptr){
            if(strcmp(ptr->d_name,".") == 0 || strcmp(ptr->d_name,"..") == 0){
                continue;
            }
            _configs[ptr->d_name] = _configFilePath + "/" + ptr->d_name;
        }

        closedir(dir);
    }

private:
    string _configFilePath;
    map<string,string> _configs;
    static Configuration * _conf;
};

#endif // _CONFIGURATION_H__