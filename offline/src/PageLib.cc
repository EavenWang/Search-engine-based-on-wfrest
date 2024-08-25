#include "../include/PageLib.h"
#include "../../tinyxml/tinyxml2.h"
#include "../include/Configuration.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <math.h>

using namespace tinyxml2;
using std::ofstream;
using std::cerr;
using std::regex;
using std::cout;


regex tag_regex("<[^>]*>");
static int id = 0;

string dealChinesehWord(string &word)
{
    string newWord;
    for (size_t idx = 0; idx != word.size(); idx++)
    {   
        if (word[idx] != '\r' && word[idx] != '\n' && word[idx] != ' ' && word[idx] != '\t')
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

PageLib::PageLib(Configuration & conf,DirScanner & dirScanner)
    :_dirScanner(dirScanner)
    ,_simhasher("../../simhash/dict/jieba.dict.utf8",
            "../../simhash/dict/hmm_model.utf8",
            "../../simhash/dict/idf.utf8",
            "../../simhash/dict/stop_words.utf8")
    ,_jieba("../../simhash/dict/jieba.dict.utf8",
            "../../simhash/dict/hmm_model.utf8",
            "../../simhash/dict/user.dict.utf8")
    {
        _dirScanner.traverse(conf.getConfigMap()["webConf.conf"]);
        _pages = _dirScanner.getFiles();
        _fingerprint.reserve(1024);
        readChineseStopWords(_stopWords);
    }


bool PageLib::process(string fileName){
    XMLDocument doc;
    doc.LoadFile(fileName.c_str());
    XMLElement * element = doc.FirstChildElement()->FirstChildElement("channel");
    if(element == nullptr) return false;
    element = element->FirstChildElement("item");

    while(element != nullptr){
        XMLElement* titleElement = element;
        RssItem rss;
        rss.id = id;
        titleElement = titleElement->FirstChildElement("title");
        if(titleElement == nullptr) return false;
        rss.title = titleElement->GetText();
        rss.title = regex_replace(rss.title, tag_regex, "");

        titleElement = titleElement->NextSiblingElement("link");
        if(titleElement == nullptr) return false;
        rss.link = titleElement->GetText();

        titleElement = titleElement->NextSiblingElement("description");
        if(titleElement == nullptr) return false;
        rss.description = titleElement->GetText();

        rss.description = regex_replace(rss.description, tag_regex, "");

        titleElement = titleElement->NextSiblingElement("content:encoded");
        if(titleElement == nullptr){
            rss.content = "";

            if(cutRedundantPage(rss)){
                rss.description = dealChinesehWord(rss.description);
                store(rss);
                id++;
            }
            element = element->NextSiblingElement("item");
            continue;
        }

        rss.content = titleElement->GetText();
        rss.content = regex_replace(rss.content, tag_regex, "");

        if(cutRedundantPage(rss)){
            rss.description = dealChinesehWord(rss.description);
            rss.content = dealChinesehWord(rss.content);
            store(rss);
            id++;
        }

        element = element->NextSiblingElement("item");
    }
    return true;
}

void PageLib::create(){
    _pages = _dirScanner.getFiles();

    for(int i = 0;i < _pages.size();i++){
        bool result = process(_pages[i]);
        if(result == false){
            cout << _pages[i] << "\n";
        }
    }

    ofstream ofs("/home/mylinux/homework/project/data/newoffset.dat");
    for(const auto & offset : _offsetLib){
        ofs << offset.first << " "
        << offset.second.first << " "
        << offset.second.second << "\n";
    }
    ofs.close();
}

void PageLib::store(const RssItem & rss){
    ofstream ofs("/home/mylinux/homework/project/data/newripepage.dat",std::ios::app);

    if(!ofs){
        cerr << "error in PageLig::store\n";
        return;
    }

    int start = ofs.tellp();
    _offsetLib[rss.id].first = start;

    ofs << "<doc>\n";
    ofs << "<docid>" << rss.id << "</docid>\n";
    ofs << "<title>" << rss.title<< "</title>\n";
    ofs << "<link>" << rss.link << "</link>\n";
    ofs << "<description>" << rss.description << "</description>\n";
    ofs << "<content>" << rss.content << "</content>\n";
    ofs << "</doc>\n";
    int end = ofs.tellp();
    _offsetLib[rss.id].second = end - start - 1;

    ofs.close();
    cutWords(rss);

    // for(const auto & word : _wordsFrequency){
    //     _words[word.first]++;
    // }
}

bool PageLib::cutRedundantPage(RssItem & rss){

    string article = rss.content + rss.description;
    uint64_t u64 = 0;
    size_t topN = 20;
    _simhasher.make(article,topN,u64);

    for(int i = 0;i < _fingerprint.size();i++){
        if(Simhasher::isEqual(_fingerprint[i],u64)){
            return false;
        }
    }
    _fingerprint.push_back(u64);
    return true;
}

void PageLib::buildInvertIndexMap(){
    ofstream ofs("/home/mylinux/homework/project/data/invertIndex.dat");
    if(!ofs){
        cerr << "error in buildInvertIndexMap\n";
    }

    string word;
    int docid,frequency,articles;
    int weight,TF;
    double DF;
    double IDF;
    int N = _offsetLib.size();
    for(const auto & wordF : _wordsFrequency){
        word = wordF.first;

        for(const auto & s : wordF.second){
            docid = s.first;
            frequency = s.second;
            articles = wordF.second.size();

            TF = frequency;
            DF = articles;
            IDF = log2(N /(DF + 1) + 1);
            weight = IDF * TF;

            _invertIndexTable[word].insert({docid,weight});
            _articleWeights[docid] += weight * weight;
        }
    }

    for(auto & invertWord : _invertIndexTable){
        string wordInTable = invertWord.first;
        set<pair<int,double>> temp;
        for(auto & pairInSet : invertWord.second){
            int id = pairInSet.first;
            double weigthOfWord = pairInSet.second;
            weigthOfWord = weigthOfWord / (sqrt(_articleWeights[id]));
            temp.insert({id,weigthOfWord});
        }
        _invertIndexTable[wordInTable] = temp;
    }

    for(const auto & invertWord : _invertIndexTable){
        for(const auto & pairInSet : invertWord.second){
            ofs << invertWord.first << " " 
                << pairInSet.first << " "
                << pairInSet.second << "\n";
        }
    }
    ofs.close();
}

void PageLib::cutWords(const RssItem & rss){

    int docid = rss.id;

    string sentence = rss.content + rss.description;
    sentence = dealChinesehWord(sentence);
    
    vector<string> result;

    _jieba.Cut(sentence,result);

    unordered_map<string,pair<int,int>> wordF;

    for(int i = 0;i < result.size();i++){
        if(_stopWords.find(result[i]) == _stopWords.end()){
            wordF[result[i]].first = docid;
            wordF[result[i]].second++;
        }
    }

    for(const auto & info : wordF){
        _wordsFrequency[info.first].insert(info.second);
    }
}