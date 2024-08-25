#ifndef __SPLITTOOLCPPJIEBA_H__
#define __SPLITTOOLCPPJIEBA_H__

#include "SplitTool.h"

#include "../../cppjieba/Jieba.hpp"


class SplitToolJieba
:public SplitTool 
{
public: 
    static SplitToolJieba * getIntance(){
        if(_tool == nullptr){
            _tool = new SplitToolJieba();
            return _tool;
        }

        return _tool;
    }

    static void destroy(){
        if(_tool != nullptr){
            delete _tool;
            _tool = nullptr;
        }
    }

    vector<string> cut(const string sentence) override{
        vector<string> words;
        _jieba->Cut(sentence,words);
        return words;                    
    }

    vector<string> cutToChar(const string sentence) override{
        vector<string> chars;
        _jieba->CutSmall(sentence,chars,1);
        return chars;
    }

private:
    SplitToolJieba(){
        const char* const DICT_PATH = "/home/mylinux/homework/project/cppjieba/dict/jieba.dict.utf8";
        const char* const HMM_PATH = "/home/mylinux/homework/project/cppjieba/dict/hmm_model.utf8";
        const char* const USER_DICT_PATH = "/home/mylinux/homework/project/cppjieba/dict/user.dict.utf8";
        const char* const IDF_PATH = "/home/mylinux/homework/project/cppjieba/dict/idf.utf8";
        const char* const STOP_WORD_PATH = "/home/mylinux/homework/project/cppjieba/dict/stop_words.utf8";
        _jieba = new cppjieba::Jieba(DICT_PATH,HMM_PATH,
                                        USER_DICT_PATH,IDF_PATH,
                                        STOP_WORD_PATH);
    }

private:
    cppjieba::Jieba  * _jieba;
    static SplitToolJieba * _tool;
};
SplitToolJieba * SplitToolJieba::_tool = nullptr;

#endif // SPLITTOOLCPPJIEBA_H__