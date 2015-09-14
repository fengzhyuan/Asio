/**!
 * \file charServer.cpp
 */

#include "chatServer.h"

Server::Server():acceptor( service, tcp::endpoint(tcp::v4(), 8001)), 
        clientList(new list<socket_ptr>), 
        pMsgQueue( new queue<clientMap_ptr>){}

bool Server::initContext(int argc, char** argv){
    
    boost::thread_group threads;
    try{
        threads.create_thread(boost::bind( &Server::acceptorLoop, this));
        boost::this_thread::sleep( posix_time::millisec(sml));

        threads.create_thread(boost::bind( &Server::requestLoop, this));
        boost::this_thread::sleep( posix_time::millisec(sml));

        threads.create_thread(boost::bind( &Server::respondLoop, this));
        boost::this_thread::sleep( posix_time::millisec(sml));

        threads.join_all();
    }
    catch( std::exception &e){
        cout << e.what() << endl;
        return false;
    }
    
    return true;
}

void Server::acceptorLoop() {
    
    for(;;) {
        socket_ptr clientSock( new tcp::socket( service));
        acceptor.accept( *clientSock);
        
        cout << "new client joined\n";
        
        mtx.lock();
        clientList->emplace_back( clientSock);
        mtx.unlock();
        
        cout << "total client size: " << clientList->size() << endl;
    }
}

void Server::requestLoop() {
    
    for(;;) {
        if( !clientList->empty()){
            mtx.lock();
            for( socket_ptr &clientSock : *clientList){
                if( clientSock->available() ){
                    char readBuf[sizeMsgBuff];
                    memset( readBuf, 0,sizeof(readBuf));

                    int bytesRead = clientSock->read_some(buffer(readBuf, sizeMsgBuff));

                    string_ptr msg(new string(readBuf, bytesRead));

                    if(clientSentExit(msg)) {
                        disconnectClient(clientSock);
                        break;
                    }

                    clientMap_ptr cm(new clientMap);
                    
                    cm->insert( make_pair(clientSock, msg));
                    pMsgQueue->push(cm);

                    cout << "ChatLog: " << *msg << endl;
                }
            }
            mtx.unlock();
        }
        boost::this_thread::sleep( boost::posix_time::millisec( lon));
    }
}

bool Server::clientSentExit(string_ptr msg) {
    if( msg->find( "exit") != string::npos )
        return true;
    else
        return false;
}

void Server::disconnectClient(socket_ptr sock) {
    auto pos = find( clientList->begin(), clientList->end(), sock);
    if( pos != clientList->end() ){
        sock->shutdown( tcp::socket::shutdown_both);
        sock->close();
        clientList->erase( pos);
        
        cout << "Client Disconnected! " << clientList->size() << " total clients" << endl;

    }
}

void Server::respondLoop() {
    
    for(;;) {
        if( !pMsgQueue->empty() ) {
            auto message = pMsgQueue->front();

            mtx.lock();
            for(socket_ptr& clientSock : *clientList) {
                clientSock->write_some(buffer(*(message->begin()->second), sizeMsgBuff));
            }
            mtx.unlock();

            mtx.lock();
            pMsgQueue->pop();
            mtx.unlock();
        }

        boost::this_thread::sleep( boost::posix_time::millisec(sleepLen::lon));
    }
}