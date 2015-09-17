/* 
 * File:   chatServer.h
 * Author: fengzh
 *
 * Created on September 11, 2015, 9:39 PM
 */

#ifndef ASIO_ASYNC_METHOD_INCLUDE_CHATSERVER_H
#define	ASIO_ASYNC_METHOD_INCLUDE_CHATSERVER_H

#include "utils.h"
#include "message.h"

/*!
 * \class BaseMember
 * base class of members in chat room
 */
class BaseSession {
public:
    virtual ~BaseSession() {}
    virtual void deliver(const Message& msg) = 0;
    const string uid() const{ 
        return uid_;
    }
protected:
    void set_uid( const char* name) { uid_ = string(name); }
    
protected:
    string uid_;
};

/*!
 * \class Room
 * chat room class
 */
class Room {
public:
    typedef boost::shared_ptr<BaseSession> typeBSession;
public:
    void join(typeBSession);/* join a new session */
    void leave(typeBSession);       /* remove a session */
    void deliver(const Message&);   /* deliver message to members */
    void blackboard(const string&, MSG_TYPE);

private:
    enum { MAX_MSG_RECORD = 100 };

private:
    set<typeBSession> session_list_;
    dqMsg      msg_list_; /**< most recent messages in the room */
};


/*!
 * \class Session
 * server side session manager; each session corresponds to a client
 */

class Session
  : public BaseSession,
    public boost::enable_shared_from_this<Session> {
    
 public:
    Session(boost::asio::io_service&, Room&);

    tcp::socket& socket();
    void start();
    void deliver(const Message&);
    void h_read_head(const boost::system::error_code&);
    void h_read_body(const boost::system::error_code&);
    void h_write(const boost::system::error_code&);
    
 private:
    tcp::socket socket_;   
    Room        &room_;
    Message     msg_;
    dqMsg       msg_list_;
};

class Server {
public:
    typedef boost::shared_ptr<Session> typeSession;
public:
    Server(boost::asio::io_service&, const tcp::endpoint&);
    void start();
    void h_start(typeSession, const boost::system::error_code&);

private:
    int session_count_;
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
    Room room_;
};

typedef boost::shared_ptr<Server> typeServer;
typedef list<typeServer> typeServerList;

bool InitServerContext(int argc, char**argv);

#endif	/* ASIO_ASYNC_METHOD_INCLUDE_CHATSERVER_H */

