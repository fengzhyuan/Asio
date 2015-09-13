/* 
 * File:   chatServer.h
 * Author: fengzh
 *
 * Created on September 11, 2015, 9:39 PM
 */

#ifndef CHATSERVER_H
#define	CHATSERVER_H

#include "utils.h"
#include "Message.h"

/*!
 * \class BaseMember
 * base class of members in chat room
 */
class BaseMember {
public:
    virtual ~BaseMember() {}
    virtual void deliver(const Message& msg) = 0;
};

class Room {
public:
    typedef boost::shared_ptr<BaseMember> typeMember;
    
public:
    void join( typeMember _member);
    void leave( typeMember _member);

    void deliver(const Message& _msg);

private:
    enum { MAX_MSG_RECORD = 100 };

private:
    set<typeMember> m_member_list;
    dqMsg      m_msgs; /**< most recent messages in the room */
};

//----------------------------------------------------------------------

class Session
  : public BaseMember,
    public boost::enable_shared_from_this<Session> {
    
public:
    Session(boost::asio::io_service& _service, Room& _room);

    tcp::socket& socket();
    void start();
    void deliver(const Message& _msg);
    void hParseHeader(const boost::system::error_code& _error);
    void hParseBody(const boost::system::error_code& _error);
    void hWrite(const boost::system::error_code& _error);
    
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
    Server(boost::asio::io_service& _service, const tcp::endpoint& _endpoint);
    void start();
    void hStart(typeSession _session, const boost::system::error_code& _error);

private:
  boost::asio::io_service& m_service;
  tcp::acceptor m_acceptor;
  Room m_room;
};

typedef boost::shared_ptr<Server> typeServer;
typedef list<typeServer> typeServerList;

bool initServerContext( int argc, char**argv);

#endif	/* CHATSERVER_H */

