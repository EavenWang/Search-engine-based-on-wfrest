#ifndef __PAGELIB_H__
#define __PAGELIB_H__

#include "../../simhash/Simhasher.hpp"
#include "DirScanner.h"
#include <map>
#include <unordered_map>
#include <set>


using std::map;
using std::pair;
using std::unordered_map;
using std::set;
using namespace simhash;

class Configuration;

struct RssItem{
    int id;
    string title;
    string link;
    string description;
    string content;
};

class PageLib{

public:
    PageLib(Configuration & conf,DirScanner & dirScanner);
    
    void create();

    void buildInvertIndexMap();


private:
    void store(const RssItem & rss);
    bool cutRedundantPage(RssItem & rss);
    bool process(string fileName);
    void cutWords(const RssItem & rss);

private:
    DirScanner & _dirScanner;
    vector<string> _pages;

    map<int,pair<int,int>> _offsetLib;

    map<string,set<pair<int,int>>> _wordsFrequency;    //词语对应的文章docid，在文章中的频次

    map<int,double> _articleWeights; //文章docid 对应的所有权重的平方和

    unordered_map<string,set<pair<int,double>>> _invertIndexTable;  //词语对应的文章id和权重
    vector<uint64_t> _fingerprint;

    unordered_set<string>  _stopWords;
    Simhasher _simhasher;
    Jieba _jieba;
};


#endif // PAGELIB_H__