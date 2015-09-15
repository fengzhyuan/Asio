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
    const string getUid() const{ 
        return m_uid;
    }
protected:
    void setUid( const char* name) { m_uid = string(name); }
    
protected:
    string m_uid;
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
    void blackboard(const string&, Message::MSG_TYPE);

private:
    enum { MAX_MSG_RECORD = 100 };

private:
    set<typeBSession> m_member_list;
    dqMsg      m_msgs; /**< most recent messages in the room */
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

    tcp::socket& getSocket();
    void start();
    void deliver(const Message&);
    void hParseHeader(const boost::system::error_code&);
    void hParseBody(const boost::system::error_code&);
    void hWrite(const boost::system::error_code&);
    
 private:
    tcp::socket m_socket;   
    Room        &m_room;
    Message     m_msg_in;
    dqMsg       m_msgs;
};

class Server {
public:
    typedef boost::shared_ptr<Session> typeSession;
public:
    Server(boost::asio::io_service&, const tcp::endpoint&);
    void start();
    void hStart(typeSession, const boost::system::error_code&);

private:
    int m_session_count;
    boost::asio::io_service& m_service;
    tcp::acceptor m_acceptor;
    Room m_room;
};

typedef boost::shared_ptr<Server> typeServer;
typedef list<typeServer> typeServerList;

bool initServerContext(int argc, char**argv);

#endif	/* ASIO_ASYNC_METHOD_INCLUDE_CHATSERVER_H */

