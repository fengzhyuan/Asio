/* 
 * File:   chatClient.h
 * Author: fengzh
 *
 * Created on September 11, 2015, 6:25 PM
 */

#ifndef CHATCLIENT_H
#define	CHATCLIENT_H

#include "utils.h"

class Client{
public:
    Client();
    bool isOwnMessage(string_ptr);
    void displayLoop(socket_ptr sock);
    void inboundLoop(socket_ptr, string_ptr);
    void writeLoop(socket_ptr, string_ptr);
    string* buildPrompt();
    bool initContext(int argc, char** argv);
    
private:
    io_service service;
    msgQueueClient_ptr pMsgQueue;
    tcp::endpoint ep;
    string_ptr strPrompt;
    const int sizeInput   = 256;
};

#endif	/* CHATCLIENT_H */

