/* 
 * File:   main.cpp
 * Author: fengzh
 *
 * Created on September 11, 2015, 6:06 PM
 */

#include "include/client.h"
#include "include/server.h"

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
    string m_type = string(argv[1]);
    
    if (m_type == "client") {
        if (!InitClientContext(argc, argv))
            return -1;
    }
    else if (m_type == "server") {
        if (!InitServerContext(argc, argv))
            return -1;
    }
    
    return 0;
}

