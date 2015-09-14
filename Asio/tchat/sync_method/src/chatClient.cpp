/*!
 * \file chatClient.cpp
 * 
 */
#include "chatClient.h"

Client::Client():pMsgQueue(new queue<string_ptr>), 
        ep( tcp::endpoint (ip::address_v4::from_string("127.0.0.1"), 8001)) {
    
}
bool Client::initContext( int argc, char**argv){
        
    try{
        boost::thread_group threads;
        
        socket_ptr sock( new tcp::socket(service));
        
        string_ptr m_prompt( buildPrompt());
        strPrompt = m_prompt;
        
        sock->connect(ep);
        
        cout << "init\n";
        
        threads.create_thread( boost::bind(&Client::displayLoop, this, sock) );
        threads.create_thread( boost::bind(&Client::inboundLoop, this, sock, m_prompt));
        threads.create_thread( boost::bind(&Client::writeLoop, this, sock, m_prompt));
        
        threads.join_all();
    }catch( std::exception &e){
        cout << e.what() << endl;
        return false;
    }
    
    cout << "bye\n";
    getchar();
    
    return true;
}

string* Client::buildPrompt() {
    
    char nameBuf[sizeInput];
    string* prompt = new string(": ");

    cout << "Please input a new username: ";
    cin.getline(nameBuf, sizeInput);
    *prompt = (string)nameBuf + *prompt;
    boost::algorithm::to_lower(*prompt);

    return prompt;
}

void Client::inboundLoop(socket_ptr sock, string_ptr prompt) {
    int sizeBufRd = 0;
    char buff[1024] = {0};
    
    while( true){
        if( sock->available()){
            sizeBufRd = sock->read_some( buffer( buff, sizeInput));
            string_ptr msg( new string( buff, sizeBufRd));
            pMsgQueue->push( msg);
        }
        
//        boost::this_thread::sleep( boost::posix_time::millisec(100));
    }
}

void Client::writeLoop(socket_ptr sock, string_ptr prompt) {
    
    char inBuf[sizeInput]; memset( inBuf, 0, sizeof(inBuf));
    string strMsg;
    
    while( true){
        cin.getline( inBuf, sizeInput);
        strMsg = *prompt + (string)inBuf + "\n";
        
        if( !strMsg.empty() ){
            sock->write_some( buffer(inBuf, sizeInput) );
        }
        if(strMsg.find("exit") != string::npos)
            exit(1);
        strMsg = "";
        memset( inBuf, 0, sizeof(inBuf));
    }
}

void Client::displayLoop(socket_ptr sock) {
    
    for(;;) {
        if(!pMsgQueue->empty()) {
            if(!isOwnMessage(pMsgQueue->front())) {
                cout << "\n" + *(pMsgQueue->front());
            }
            pMsgQueue->pop();
        }

//        boost::this_thread::sleep( boost::posix_time::millisec(1000));
    }
}

bool Client::isOwnMessage(string_ptr message)
{
    if(message->find(*strPrompt) != string::npos)
        return true;
    else
        return false;
}



