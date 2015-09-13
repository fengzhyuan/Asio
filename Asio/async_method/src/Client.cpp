/*!
 * \file chatClient.cpp
 * 
 */
#include "Client.h"

Client::Client( io_service& _service,
                tcp::resolver::iterator _iter)
    : m_service( _service),
      m_socket( _service),
      m_status( true){
    // check validity of _iter in handler functions
    boost::asio::async_connect( m_socket, _iter,
        boost::bind( &Client::hConnect, this,
          boost::asio::placeholders::error));
  }

/**
 * \TODO check if timeout during cstr initialization
 * @return return false if cstr failed
 */
bool Client::isConnected() const {
    return m_status;
}

/**
 * 
 * @param _msg message to be stored
 */
void Client::write( const Message& _msg) {
    m_service.post(boost::bind( &Client::doWrite, this, _msg));
}

void Client::close() {
    m_service.post(boost::bind( &Client::doClose, this));
}

/**
 * cstr connection handler
 * @param err
 */
void Client::hConnect( const system::error_code& err) {
    if( !err) {
        cout << "connected to server\n";
        // exec pParseHeader after read operation
        async_read(m_socket,
            buffer(m_msg_in.data(), Message::HEADER_LENGTH),
            bind( &Client::hParseHeader, this,
                asio::placeholders::error));
    }   
}

/**
 * sub-module of HandleParseBody
 * @param err error code (implicitly convert to bool type)
 */
void Client::hParseHeader( const boost::system::error_code& error) {
    if (!error && m_msg_in.decodeHeader()) {
      boost::asio::async_read( m_socket,
          boost::asio::buffer( m_msg_in.body(), m_msg_in.bodyLength()),
          boost::bind( &Client::hParseBody, this,
            boost::asio::placeholders::error));
    }
    else {
        doClose();
    }
}

/**
 * call parse_header hr first
 * @param err
 */
void Client::hParseBody( const boost::system::error_code& error) {
    if (!error) {
        displayMsg();
        boost::asio::async_read(m_socket,
            boost::asio::buffer(m_msg_in.data(), Message::HEADER_LENGTH),
            boost::bind( &Client::hParseHeader, this,
              boost::asio::placeholders::error));
    }
    else {
        doClose();
    }
}

/**
 * write all messages into socket
 * @param err
 */
void Client::doWrite( Message msg) {
    bool bEmpty = m_msgs.empty();
    m_msgs.push_back(msg);
    
    if ( bEmpty) {
      boost::asio::async_write(m_socket,
          boost::asio::buffer(m_msgs.front().data(),
            m_msgs.front().length()),
          boost::bind( &Client::hWrite, this,
            boost::asio::placeholders::error));
    }
}

void Client::hWrite( const boost::system::error_code& error)
  {
    if ( !error) {
        m_msgs.pop_front();
        if ( !m_msgs.empty()) {
            boost::asio::async_write(m_socket,
                boost::asio::buffer(m_msgs.front().data(),
                  m_msgs.front().length()),
                boost::bind( &Client::hWrite, this,
                  boost::asio::placeholders::error));
        }
    }
    else {
        doClose();
    }
}

void Client::doClose() {
    m_socket.close();
}

void Client::displayMsg() {
    
    cout.write(m_msg_in.body(), m_msg_in.bodyLength());
    cout << "\n";
}

/**
 * client context initialization
 * @param argc main argc
 * @param argv main argv 
 * @return [true: cstr established][false: cstr failed]
 */
bool initClientContext(int argc, char**argv) {
    try{
        if( argc != 4) {
            cout << " invalid param [<host> <port>]\n";
            return false;
        }
        
        io_service m_Service;

        tcp::resolver m_Resolver( m_Service);
        tcp::resolver::query m_Query( argv[2], argv[3]);
        // domain name: 1-to-many
        tcp::resolver::iterator m_Iterator = m_Resolver.resolve( m_Query);

        boost::shared_ptr<Client> m_pClient( new Client( m_Service, m_Iterator));

        boost::thread m_Thread(boost::bind( &io_service::run, &m_Service));

        char tmp[Message::MAX_BODY_LENGTH + 1];
        while (cin.getline( tmp, Message::MAX_BODY_LENGTH + 1)) {
            if (!strcmp( tmp, "EXIT") ) {
                break;
            }
            Message msg;
            msg.bodyLength( strlen( tmp));
            memcpy(msg.body(), tmp, msg.bodyLength());
            msg.encodeHeader();
            m_pClient->write(msg);
        }

        m_pClient->close();
        m_Thread.join();
    }
    catch (std::exception& e) {
        cerr << "Exception: " << e.what() << endl;
        return false;
    }
    return true;
}
