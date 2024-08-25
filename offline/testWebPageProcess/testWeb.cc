#include "../include/PageLib.h"
#include "../include/Configuration.h"

#include <iostream>

using std::cout;
using std::cin;
using std::endl;


int main() {

    Configuration * conf = Configuration::getInstance();
    DirScanner scann;

    PageLib pages(*conf,scann);

    pages.create();
    pages.buildInvertIndexMap();
    
    return 0;
}