/* 
 * File:   main.cpp
 * Author: fengzh
 *
 * Created on September 11, 2015, 6:06 PM
 */

#include "tchat/async_method/include/Client.h"
#include "tchat/async_method/include/Server.h"

using namespace std;
/*
 * 
 */
int main(int argc, char** argv) {
    setvbuf(stdout, 0, _IONBF, 0);
    
    if (argc < 2){
        cout << "too few params\n";
        return -1;
    }
    string mType = string(argv[1]);
    
    if (mType == "client") {
        if (!initClientContext(argc, argv))
            return -1;
    }
    else if (mType == "server") {
        if (!initServerContext(argc, argv))
            return -1;
    }
    
    return 0;
}

