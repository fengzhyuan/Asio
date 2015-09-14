/* 
 * File:   main.cpp
 * Author: fengzh
 *
 * Created on September 11, 2015, 6:06 PM
 */

#include "chatClient.h"
#include "chatServer.h"

/*
 * 
 */
int main(int argc, char** argv) {
    if( argc < 2){
        cout << "too few params\n";
        return -1;
    }
    string type = string( argv[1]);
    Client *mClient = NULL;
    Server *mServer = NULL;
    
    if( type == "client") {
        mClient = new Client();
        mClient->initContext(argc, argv);
    }
    else if( type == "server") {
        mServer = new Server();
        mServer->initContext(argc, argv);
    }
    if( mClient) delete mClient;
    if( mServer) delete mServer;
    
    return 0;
}

