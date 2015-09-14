/**!
 * \file charServer.cpp
 */

#include "Server.h"

void Room::join(typeBSession session) {
    m_member_list.insert(session);
    // send message history in this room to the new session
    std::for_each(m_msgs.begin(), m_msgs.end(),
        boost::bind(&BaseSession::deliver, session, _1));
    // notification on new member
}

void Room::leave(typeBSession session) {
    m_member_list.erase(session);
}

/**
 * broadcast a new message to all members
 * @param msg new message
 */
void Room::deliver(const Message& msg) {
    m_msgs.push_back(msg);
    while (m_msgs.size() > MAX_MSG_RECORD)
      m_msgs.pop_front();
    
    for_each(m_member_list.begin(), m_member_list.end(),
        boost::bind(&BaseSession::deliver, _1, boost::ref(msg))); /* ref to session::m_msg_in */
}

void Room::blackboard(const Message& msg) {
    for_each(m_member_list.begin(), m_member_list.end(),
        boost::bind(&BaseSession::deliver, _1, boost::ref(msg))); /* ref to session::m_msg_in */
}

//------------------------------------------------------------------------------

Session::Session(boost::asio::io_service& service, Room& room)
      : m_socket(service),
        m_room(room) {}

tcp::socket& Session::getSocket() {
    return m_socket;
}

void Session::start() {
    m_room.join(shared_from_this());
    boost::asio::async_read(m_socket,
        boost::asio::buffer(m_msg_in.data(), Message::HEADER_LENGTH),
        boost::bind(&Session::hParseHeader, shared_from_this(),
          boost::asio::placeholders::error));
}

void Session::deliver(const Message& msg) {
    bool bEmpty = m_msgs.empty();
    m_msgs.push_back(msg);
    
    if (bEmpty) {
        // send the message to client
        boost::asio::async_write(m_socket,
            boost::asio::buffer(m_msgs.front().data(), m_msgs.front().length()),
            boost::bind(&Session::hWrite, shared_from_this(),
              boost::asio::placeholders::error));
    }
}

void Session::hParseHeader(const boost::system::error_code& error) {
    
    if  (!error && m_msg_in.decodeHeader()) {
      boost::asio::async_read(m_socket,
            boost::asio::buffer(m_msg_in.body(), m_msg_in.bodyLength()),
            boost::bind(&Session::hParseBody, shared_from_this(),
              boost::asio::placeholders::error));
    }
    else {
      m_room.leave(shared_from_this());
    }
}

void Session::hParseBody(const boost::system::error_code& error) {
    if (!error) {
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
    if (!_error) {
        m_msgs.pop_front();
        if (!m_msgs.empty()) {
        boost::asio::async_write(m_socket,
            boost::asio::buffer(m_msgs.front().data(),
              m_msgs.front().length()),
            boost::bind(&Session::hWrite, shared_from_this(),
              boost::asio::placeholders::error));
        }
    }
    else {
        m_room.leave(shared_from_this());
    }
}

//------------------------------------------------------------------------------

Server::Server(boost::asio::io_service& service, 
        const tcp::endpoint& endpoint)
        : m_service(service),
          m_acceptor(service, endpoint){
    start(); 
}

void Server::start() {
    // new request from a client
    typeSession mNewSession(new Session(m_service, m_room));
    m_acceptor.async_accept(mNewSession->getSocket(),
        boost::bind(&Server::hStart, this, mNewSession,
          boost::asio::placeholders::error));
}

void Server::hStart(typeSession session, const boost::system::error_code& error) {
    
    if (!error) {
        session->start();
    }
    start();
}

bool initServerContext (int argc, char**argv) {
    try{
        if (argc < 3) {
            cout << "invalid params: [<port>] | [<port> <port> ...]\n";
            return false;
        }
        
        boost::asio::io_service mService;

        typeServerList mServerList;
        for  (int i = 1; i < argc; ++i) {
            tcp::endpoint endpoint(tcp::v4(), atoi(argv[i]));
            typeServer server(new Server(mService, endpoint));
            mServerList.push_back(server);
        }

        mService.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return false;
    }
    return true;
}