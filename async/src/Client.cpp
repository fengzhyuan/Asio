/*!
 * \file chatClient.cpp
 * 
 */
#include "Client.h"

Client::Client(io_service& _service, tcp::resolver::iterator ep_it):
                m_service(_service), m_socket(_service), m_success(true){ 
    try {
        boost::asio::async_connect( m_socket, ep_it,
                boost::bind( &Client::hConnect, this, asio::placeholders::error));
    }
    catch( std::exception &e){
        cout << e.what() << endl;
        m_success = false;
    }
}

/**
 * 
 * @return return false if cstr failed
 */
bool Client::isCreated() const {
    return m_success;
}

/**
 * 
 * @param _msg message to be stored
 */
void Client::write( const Message& _msg) {
    m_service.post( boost::bind( &Client::doWrite, this, _msg));
}

void Client::close() {
    m_service.post( boost::bind( &Client::doClose, this));
}

void Client::hConnect( const system::error_code& err) {
    if( !err) {
        async_read( m_socket, 
                buffer( m_msg_in.data(), Message::header_length),
                boost::bind( &Client::hParseHeader, this, asio::placeholders::error));
    }
}

/**
 * sub-module of HandleParseBody
 * @param err error code
 */
void Client::hParseHeader(const system::error_code& err) {
    if( !err && m_msg_in.decodeHeader() ) {
        async_read( m_socket,
                buffer( m_msg_in.body(), m_msg_in.bodyLength()),
                boost::bind( &Client::hParseBody, this, asio::placeholders::error));
    }
    else {
        doClose();
    }
}

/**
 * call parse_header hr first
 * @param err
 */
void Client::hParseBody(const system::error_code& err) {
    if( !err) {
        cout.write( m_msg_in.body(), m_msg_in.bodyLength());
        cout << "\n";
        async_read( m_socket,
                buffer( m_msg_in.data(), Message::header_length),
                boost::bind( &Client::hParseHeader, this, asio::placeholders::error));
    }
    else {
        doClose();
    }
}

void Client::hWrite(const system::error_code& err) {
    if( !err) {
        m_msgs.pop_front();
        if( !m_msgs.empty()){
            async_write( m_socket,
                    buffer( m_msgs.front().data(), m_msgs.front().length()),
                    boost::bind( &Client::hWrite, this, asio::placeholders::error));
        }
        else {
            doClose();
        }
    }
}


void Client::doWrite(Message msg) {
    bool bEmpty = m_msgs.empty();
    m_msgs.push_back( msg);
    if( bEmpty) {
        async_write( m_socket,
                buffer( m_msgs.front().data(), m_msgs.front().length()),
                boost::bind( &Client::hWrite, this, asio::placeholders::error));
    }
}

void Client::doClose() {
    m_socket.close();
}

bool initClientContext(int argc, char**argv) {
    try{
        if( argc != 4) {
            cout << " invalid param [<host> <port>]\n";
            return false;
        }
        io_service m_Service;
        tcp::resolver m_Resolver( m_Service);
        tcp::resolver::query m_Query( argv[2], argv[3]);
        tcp::resolver::iterator m_Iterator = m_Resolver.resolve( m_Query);
        
        boost::shared_ptr<Client> m_pClient( new Client( m_Service, m_Iterator));
        
        boost::thread m_Thread( boost::bind( &io_service::run, &m_Service));
        
        char tmp[Message::max_body_length + 1];
        while( cin.getline( tmp, Message::max_body_length + 1)){
            Message msg;
            msg.bodyLength( strlen( tmp));
            memcpy( msg.body(), tmp, msg.bodyLength());
            msg.encodeHeader();
            m_pClient->write( msg);
        }
        m_Thread.join();
    }
    catch( std::exception& e){
        cout << e.what() << endl;
        return false;
    }
    return true;
}
