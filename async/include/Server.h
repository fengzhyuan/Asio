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

using boost::asio::ip::tcp;

class Participant{
public:
    virtual ~Participant() {}
    virtual void deliver( const Message& msg) = 0;
};

class Room {
public:
    typedef boost::shared_ptr<Participant> ptrParticipant;
public:
    void join( ptrParticipant member);
    void leave( ptrParticipant member);
    void deliver( const Message& msg);
private:
    set<ptrParticipant> m_members;
    enum {MAX_RECORD_MSG = 128};
    dqMsg m_msg_record;
};

/**
 * enable returning shared_ptr to itself
 */
class Session:
    public Participant, 
    public boost::enable_shared_from_this<Session> {
    
public:
    Session( asio::io_service& _service, Room& _room);
    tcp::socket& getSocket();
    void start();
    void deliver( const Message& msg);
    void hParseHeader( const system::error_code& e);
    void hParseBody( const system::error_code& e);
    void hWrite( const system::error_code& e);
    
private:
    tcp::socket m_socket;
    Room&   m_room;
    Message m_msg_in;
    dqMsg   m_msgs;
};
typedef boost::shared_ptr<Session> ptrSession;

class Server{
public:
    Server( io_service& _service, const tcp::endpoint& ep);
    void start();
    void hStart(ptrSession session, const system::error_code& e);
    
private:
    io_service&     m_service;
    tcp::acceptor   m_acceptor;
    Room            m_room;
};

typedef boost::shared_ptr<Server> ptrServer;
typedef std::list<ptrServer> lstServer;

bool initServerContext( int argc, char**argv);

#endif	/* CHATSERVER_H */

