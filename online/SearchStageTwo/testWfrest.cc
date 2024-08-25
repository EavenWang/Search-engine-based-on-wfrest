#include "../include/SearchEngineServer.h"
#include "../include/KeyRecommander.h"
#include "../include/WebPageQuery.h"
#include "../include/SplitToolCppJieba.h"

#include <iostream>
using std::cout;
using std::cin;
using std::endl;

int main() {

    Dictionary * dictionary = Dictionary::getInstance();

    SplitTool * tool = SplitToolJieba::getIntance();

    WebPageQuery webPage(tool);
    dictionary->init("file");
    
    SearchEngineServer _server(dictionary,&webPage);
    _server.start();

    return 0;
}