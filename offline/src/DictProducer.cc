#include "../include/DictProducer.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <unordered_set>

using std::cout;
using std::cerr;
using std::ifstream;
using std::istringstream;
using std::ofstream;
using std::unordered_set;

#define INITSIZE 1024

std::unordered_set<string> chinese_punctuation = {
        "。", "，", "；", "：", "！", "？", "（", "）",
        "《", "》", "“", "”","、", "……", "——","【","】","·","-","‘","’"
        };

DictProducer::DictProducer(vector<string> & files, SplitTool *tool)
: _cuttor(tool)
{
    _files.reserve(INITSIZE);
    _dict.reserve(INITSIZE);
    _files = files;
}

DictProducer::DictProducer(string file)
{
    _files.push_back(file);
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

void readEnglishFile(const string &filePath, map<string, int> &wordMap)
{
    ifstream ifs(filePath);
    if (!ifs)
    {
        cerr << "ifs open " << filePath << " error!" << "\n";
        return;
    }

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
                ++wordMap[word];
            }
        }
    }
    ifs.close();
}

void DictProducer::buildEnDict()
{
    map<string, int> wordMap;
    for (int i = 0; i < _files.size(); i++)
    {
        string filePath = _files[i];
        readEnglishFile(filePath, wordMap);
    }

    unordered_set<string> stopWords;
    readEnglishStopWords(stopWords);

    for (const auto & wordPair : wordMap)
    {
        if(stopWords.find(wordPair.first) != stopWords.end()){
            continue;
        }
        _dict.push_back(wordPair);
    }
}

string dealChinesehWord(string &word)
{
    string newWord;
    for (size_t idx = 0; idx != word.size(); idx++)
    {
        if (word[idx] != '\r')
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

void DictProducer::buildCnDict()
{
    map<string, int> wordMap;

    for (int i = 0; i < _files.size(); i++)
    {
        string filePath = _files[i];
        ifstream ifs(filePath);
        if (!ifs)
        {
            cerr << "ifs open " << filePath << " error!" << "\n";
            return;
        }

        string line;
        string sentence;
        while (getline(ifs, line))  sentence += line;

        sentence = dealChinesehWord(sentence);
        vector<string> words = _cuttor->cut(sentence);

        for(int i = 0;i < words.size();i++){
            if(((words[i][0] & 0x80) == 0)) continue;
            wordMap[words[i]]++;
        }
        ifs.close();
    }

    unordered_set<string> stopWords;
    readChineseStopWords(stopWords);

    for (const auto & wordPair : wordMap)
    {   
        if(stopWords.find(wordPair.first) != stopWords.end()){
            continue;
        }
        _dict.push_back(wordPair);
    }
}

void DictProducer::createEnglishIndex()
{   
    string words;

    for(int i = 0;i < _dict.size();i++){
        words = _dict[i].first;

        for(int j = 0;j < words.size();j++){
            _index[string(1,words[j])].insert(i);
        }
    }
}

void DictProducer::createChineseIndex(){
    string words;
    vector<string> chars;
    for(int i = 0;i < _dict.size();i++){
        words = _dict[i].first;
        chars = _cuttor->cutToChar(words);
        for(int j = 0;j < chars.size();j++){
            _index[chars[j]].insert(i);
        }
    }
}


void DictProducer::storeEnglishDict()
{
    ofstream ofs("/home/mylinux/homework/project/data/EnglishDict.dat");
    if (!ofs)
    {
        cerr << "error in storeEnglishDict\n";
        return;
    }

    for (int i = 0; i < _dict.size(); i++)
    {   
        ofs << _dict[i].first << " " << _dict[i].second << "\n";
    }
    ofs.close();

    ofs.open("/home/mylinux/homework/project/data/dictIndex.dat",std::ios::app);
    if (!ofs)
    {
        cerr << "error in storeEnglishDict\n";
        return;
    }

    for(const auto & mapPair : _index){

        ofs << mapPair.first << " ";
        for(const auto & ele : mapPair.second){
            ofs << ele << " ";
        }
        ofs << "\n";
    }
    ofs.close();
}

void DictProducer::storeChineseDict()
{
    ofstream ofs("/home/mylinux/homework/project/data/ChineseDict.dat");
    if (!ofs)
    {
        cerr << "error in storeChineselishDict\n";
        return;
    }

    for (int i = 0; i < _dict.size(); i++)
    {   
        ofs << _dict[i].first << " " << _dict[i].second << "\n";
    }
    ofs.close();

    ofs.open("/home/mylinux/homework/project/data/dictIndex.dat",std::ios::app);
    if (!ofs)
    {
        cerr << "error in storeChineseDict\n";
        return;
    }

    for(const auto & mapPair : _index){

        ofs << mapPair.first << " ";
        for(const auto & ele : mapPair.second){
            ofs << ele << " ";
        }
        ofs << "\n";
    }
    ofs.close();

}