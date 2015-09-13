/**!
 * \file charServer.cpp
 */

#include "Server.h"

void Room::join( typeMember _member) {
    m_member_list.insert( _member);
    std::for_each(m_msgs.begin(), m_msgs.end(),
        boost::bind(&BaseMember::deliver, _member, _1));
}

void Room::leave(typeMember _member) {
    m_member_list.erase( _member);
}

void Room::deliver(const Message& _msg) {
    m_msgs.push_back( _msg);
    while (m_msgs.size() > MAX_MSG_RECORD)
      m_msgs.pop_front();
    
    // board cast to all members
    for_each(m_member_list.begin(), m_member_list.end(),
        boost::bind( &BaseMember::deliver, _1, boost::ref( _msg)));
}

//------------------------------------------------------------------------------

Session::Session(boost::asio::io_service& _service, Room& _room)
      : m_socket( _service),
        m_room( _room)  {}

tcp::socket& Session::socket() {
    return m_socket;
}

void Session::start() {
    m_room.join( shared_from_this());
    boost::asio::async_read( m_socket,
        boost::asio::buffer( m_msg_in.data(), Message::HEADER_LENGTH),
        boost::bind( &Session::hParseHeader, shared_from_this(),
          boost::asio::placeholders::error));
}

void Session::deliver(const Message& _msg) {
    
    bool bEmpty = m_msgs.empty();
    m_msgs.push_back( _msg);
    
    if ( bEmpty) {
      boost::asio::async_write(m_socket,
          boost::asio::buffer(m_msgs.front().data(), m_msgs.front().length()),
          boost::bind(&Session::hWrite, shared_from_this(),
            boost::asio::placeholders::error));
    }
}

void Session::hParseHeader(const boost::system::error_code& _error) {
    
    if ( !_error && m_msg_in.decodeHeader()) {
      boost::asio::async_read( m_socket,
          boost::asio::buffer( m_msg_in.body(), m_msg_in.bodyLength()),
          boost::bind( &Session::hParseBody, shared_from_this(),
            boost::asio::placeholders::error));
    }
    else {
      m_room.leave( shared_from_this());
    }
}

void Session::hParseBody(const boost::system::error_code& _error) {
    if (!_error) {
        // broad case message
        m_room.deliver(m_msg_in);
        // prepare for the next session
        boost::asio::async_read(m_socket,
            boost::asio::buffer(m_msg_in.data(), Message::HEADER_LENGTH),
            boost::bind(&Session::hParseHeader, shared_from_this(),
              boost::asio::placeholders::error));
    }
    else {
      m_room.leave(shared_from_this());
    }
}

void Session::hWrite(const boost::system::error_code& _error) {
    if ( !_error) {
        m_msgs.pop_front();
        if ( !m_msgs.empty()) {
        boost::asio::async_write(m_socket,
            boost::asio::buffer(m_msgs.front().data(),
              m_msgs.front().length()),
            boost::bind(&Session::hWrite, shared_from_this(),
              boost::asio::placeholders::error));
        }
    }
    else {
        m_room.leave( shared_from_this());
    }
}

//------------------------------------------------------------------------------


Server::Server(boost::asio::io_service& _service, 
        const tcp::endpoint& _endpoint)
        : m_service( _service),
          m_acceptor( _service, _endpoint){
        start(); 
}

void Server::start() {
    // new request from a client
    typeSession m_NewSession( new Session(m_service, m_room));
    m_acceptor.async_accept( m_NewSession->socket(),
        boost::bind( &Server::hStart, this, m_NewSession,
          boost::asio::placeholders::error));
}

void Server::hStart(typeSession _session, const boost::system::error_code& _error) {
    
    if (!_error) {
        _session->start();
    }
    start();
}

bool initServerContext( int argc, char**argv) {
    try{
        if( argc < 3) {
            cout << "invalid params: [<port>] | [<port> <port> ...]\n";
            return false;
        }
        
        boost::asio::io_service m_Service;

        typeServerList m_Servers;
        for ( int i = 1; i < argc; ++i) {
            tcp::endpoint endpoint( tcp::v4(), atoi(argv[i]));
            typeServer server( new Server( m_Service, endpoint));
            m_Servers.push_back( server);
        }

        m_Service.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return false;
    }
    return true;
}