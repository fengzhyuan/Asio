/* 
 * File:   chatServer.h
 * Author: fengzh
 *
 * Created on September 11, 2015, 9:39 PM
 */

#ifndef CHATSERVER_H
#define	CHATSERVER_H

#include "utils.h"

class Server{
public:
    Server();    
    bool clientSentExit( string_ptr);
    void disconnectClient( socket_ptr);
    void acceptorLoop( );
    void requestLoop();
    void respondLoop();
    bool initContext(int argc, char**argv);
    
    enum sleepLen{
        sml = 100,
        lon = 200
    };
private:
    io_service service;
    tcp::acceptor acceptor;
    boost::mutex mtx;
    clientList_ptr clientList;
    msgQueueServer_ptr pMsgQueue;
    const int sizeMsgBuff = 512; 
};


#endif	/* CHATSERVER_H */

