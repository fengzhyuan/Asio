/**!
 * \file charServer.cpp
 */

#include "Server.h"

void Room::join(ptrParticipant member) {
    m_members.insert( member);
    for_each( m_msg_record.begin(), m_msg_record.end(),
            boost::bind( &Participant::deliver, member, _1));
}

void Room::leave(ptrParticipant member) {
    m_members.erase( member);
}

void Room::deliver(const Message& msg) {
    m_msg_record.push_back( msg);
    
    while( m_msg_record.size() > MAX_RECORD_MSG)
        m_msg_record.pop_front();

    for_each( m_members.begin(), m_members.end(),
            boost::bind( &Participant::deliver, _1, boost::ref(msg)));
}

Session::Session(io_service& _service, Room& _room):
        m_socket( _service), m_room( _room){}

tcp::socket& Session::getSocket() {
    return m_socket;
}

void Session::hParseHeader(const system::error_code& e) {
    if( !e && m_msg_in.decodeHeader()) {
        async_read( m_socket,
                buffer( m_msg_in.body(), m_msg_in.bodyLength()),
                boost::bind( &Session::hParseBody, shared_from_this(),
                    asio::placeholders::error));
    }
    else m_room.leave( shared_from_this());
}

void Session::hParseBody(const system::error_code& e) {
    if( !e){
        m_room.deliver( m_msg_in);
        async_read( m_socket,
                buffer( m_msg_in.data(), Message::header_length),
                boost::bind( &Session::hParseHeader, shared_from_this(),
                    asio::placeholders::error));
    }
    else m_room.leave( shared_from_this());
}

void Session::hWrite(const system::error_code& e) {
    if( !e){
        m_msgs.pop_front();
        if( !m_msgs.empty()){
            async_write( m_socket,
                    buffer( m_msgs.front().data(), m_msgs.front().length()),
                    boost::bind( &Session::hWrite, shared_from_this(),
                        asio::placeholders::error));
        }
    }
    else m_room.leave( shared_from_this());
}

void Session::start() {
    m_room.join( shared_from_this());
    async_read( m_socket,
            buffer( m_msg_in.data(), Message::header_length),
            boost::bind( &Session::hParseHeader, shared_from_this(),
            asio::placeholders::error));
}

void Session::deliver(const Message& msg) {
    bool bEmpty = m_msgs.empty();
    m_msgs.push_back( msg);
    if( bEmpty) {
        async_write( m_socket,
                buffer( m_msgs.front().data(), m_msgs.front().length()),
                boost::bind( &Session::hWrite, shared_from_this(), 
                asio::placeholders::error));
    }
}

Server::Server(io_service& _service, const tcp::endpoint& ep):
                m_service( _service), m_acceptor( _service, ep){
    start();
}

void Server::hStart(ptrSession session, const system::error_code& e) {
    if( !e) session->start();
    start();
}

void Server::start() {
    ptrSession m_NewSession ( new Session( m_service, m_room));
    m_acceptor.async_accept( m_NewSession->getSocket(),
            boost::bind( &Server::hStart, this, 
            m_NewSession,
            boost::asio::placeholders::error));

}

bool initServerContext( int argc, char**argv) {
    try{
        if( argc < 3) {
            cout << "invalid params: [<port>] | [<port> <port> ...]\n";
            return false;
        }
        
        asio::io_service m_Service;
        lstServer m_Servers;
        for( int i = 2; i < argc; ++i) { // enable multiple ports
            tcp::endpoint m_EndPoint( tcp::v4(), atoi( argv[i]));
            ptrServer m_Server( new Server( m_Service, m_EndPoint));
            m_Servers.push_back( m_Server);
        }
        
        m_Service.run();
    }
    catch( std::exception& e) {
        cout << e.what() << endl;
        return false;
    }
    return true;
}