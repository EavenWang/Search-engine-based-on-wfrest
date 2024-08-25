#ifndef __DICTPRODUCER_H__
#define __DICTPRODUCER_H__

#include "SplitTool.h"

#include <vector>
#include <map>
#include <string>
#include <set>

using std::vector;
using std::string;
using std::map;
using std::pair;
using std::set;

class DictProducer
{
public:
    DictProducer(vector<string>& files,SplitTool * tool);
    DictProducer(string);
    void buildEnDict();
    void buildCnDict();

    void createEnglishIndex();
    void createChineseIndex();

    void storeEnglishDict();
    void storeChineseDict();

private:
    vector<string> _files;
    vector<pair<string,int>> _dict;
    map<string,set<int>> _index;
    SplitTool * _cuttor;
};


#endif // DICTPRODUCER_H__