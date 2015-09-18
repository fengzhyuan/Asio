/**!
 * \file charServer.cpp
 */

#include "include/server.h"
#include "include/connection.h"

void Room::join(typeBSession session) {
    session_list_.insert(session);

    // send message history in this room to the new session
    std::for_each(msg_list_.begin(), msg_list_.end(),
        boost::bind(&BaseSession::deliver, session, _1));
    
    // notification on new member
    string str = "[ " + session->uid() + " ] entered";
    blackboard(str, MSG_ENTER);
}

void Room::leave(typeBSession session) {
    session_list_.erase(session);
    string str = "[ " + session->uid() + " ] leaved";
    blackboard(str, MSG_LEAVE);
}

/**
 * broadcast a new message to all members
 * @param msg new message
 */
void Room::deliver(const Message& msg) {
    msg_list_.push_back(msg);
    while (msg_list_.size() > MAX_MSG_RECORD)
      msg_list_.pop_front();
    
    for_each(session_list_.begin(), session_list_.end(),
        boost::bind(&BaseSession::deliver, _1, boost::ref(msg))); /* ref to session::msg_ */
}

void Room::blackboard(const string& str, MSG_TYPE msgType) {
    Message msg(msgType);
    msg.build(str);
    this->deliver(msg);
}

//------------------------------------------------------------------------------

Session::Session(boost::asio::io_service& service, Room& room)
      : socket_(service),
        room_(room) {}

tcp::socket& Session::socket() {
    return socket_;
}

void Session::start() {

    boost::asio::async_read(socket_,
        boost::asio::buffer(msg_.data(), HEADER_LENGTH),
        boost::bind(&Session::h_read_head, shared_from_this(),
          boost::asio::placeholders::error));
}

void Session::h_read_head(const boost::system::error_code& error) {
    if  (!error && msg_.decode_head()) {
        boost::asio::async_read(socket_,
            boost::asio::buffer(msg_.body(), msg_.body_length()),
            boost::bind(&Session::h_read_body, shared_from_this(),
              boost::asio::placeholders::error));
    }
    else {
        room_.leave(shared_from_this());
    }
}

void Session::h_read_body(const boost::system::error_code& error) {
    if (!error) {
        if (msg_.type() != MSG_INIT) {
            room_.deliver(msg_);
        }
        else {
            char body[MAX_BODY_LENGTH+1]="";
            memcpy(body, msg_.body(), msg_.body_length());
            this->set_uid(body);  
            room_.join(shared_from_this());
        }
        // prepare for the next session
        boost::asio::async_read(socket_,
            boost::asio::buffer(msg_.data(), HEADER_LENGTH),
            boost::bind(&Session::h_read_head, shared_from_this(),
              boost::asio::placeholders::error));
    }
    else {
        room_.leave(shared_from_this());
    }
}


void Session::deliver(const Message& msg) {
    bool bEmpty = msg_list_.empty();
    msg_list_.push_back(msg);

    if (bEmpty) {
        // send the message to client
        boost::asio::async_write(socket_,
            boost::asio::buffer(msg_list_.front().data(), msg_list_.front().length()),
            boost::bind(&Session::h_write, shared_from_this(),
              boost::asio::placeholders::error));
    }
}

void Session::h_write(const boost::system::error_code& error) {
    if (!error) {
        msg_list_.pop_front();
        if (!msg_list_.empty()) {
        boost::asio::async_write(socket_,
            boost::asio::buffer(msg_list_.front().data(),
              msg_list_.front().length()),
            boost::bind(&Session::h_write, shared_from_this(),
              boost::asio::placeholders::error));
        }
    }
    else {
        room_.leave(shared_from_this());
    }
}

//------------------------------------------------------------------------------

Server::Server(boost::asio::io_service& service, 
        const tcp::endpoint& endpoint)
        : session_count_(0),
          io_service_(service),
          acceptor_(service, endpoint){
    cout << "server established\n";
    start(); 
}

void Server::start() {
    // new request from a client
    typeSession new_session(new Session(io_service_, room_));
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&Server::h_start, this, new_session,
          boost::asio::placeholders::error));
}

void Server::sstart() {
    connection_ptr new_conn(new Connection(acceptor_.get_io_service()));
    acceptor_.async_accept(new_conn->socket(),
            boost::bind(&Server::h_sstart, this, new_conn,
            boost::asio::placeholders::error));
}

void Server::h_start(typeSession session, const boost::system::error_code& error) {
    if (!error) {
        fprintf(stdout, "new session started [%d]\n", session_count_++);
        session->start();
    }
    start();
}

void Server::h_sstart(connection_ptr conn, const boost::system::error_code& error) {
    if (!error) {
        fprintf(stdout, "new connection established [%d]\n", session_count_++);
        
    }
}
bool InitServerContext (int argc, char**argv) {
    try{
        if (argc < 3) {
            cout << "invalid params: [<port>] | [<port> <port> ...]\n";
            return false;
        }
        
        boost::asio::io_service mService;

        typeServerList mServerList;
        for  (int i = 2; i < argc; ++i) {
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
