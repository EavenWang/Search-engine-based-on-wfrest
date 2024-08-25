#include "../include/DictProducer.h"
#include "../include/SplitToolCppJieba.h"
#include "../include/DirScanner.h"

#include <iostream>
using std::cout;
using std::cin;
using std::endl;


int main() {

    DirScanner scan;
    SplitTool * tool = SplitToolJieba::getIntance();

    string ChineseDir = "/home/mylinux/homework/project/offline/conf/dictConf.conf";

    scan.traverse(ChineseDir);
    vector<string> files = scan.getFiles();

    DictProducer ChineseFile(files,tool);
    ChineseFile.buildCnDict();
    ChineseFile.createChineseIndex();
    ChineseFile.storeChineseDict();

    DictProducer EnglishFile("/home/mylinux/homework/project/files/yuliao/english.txt");

    EnglishFile.buildEnDict();
    EnglishFile.createEnglishIndex();
    EnglishFile.storeEnglishDict();

    return 0;
}