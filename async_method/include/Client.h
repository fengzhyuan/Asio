/* 
 * File:   chatchat_client.h
 * Author: fengzh
 *
 * Created on September 11, 2015, 6:25 PM
 */

#ifndef CHATCLIENT_H
#define	CHATCLIENT_H

#include "utils.h"
#include "Message.h"

class Client{
public:
public:
    Client( io_service& _service, tcp::resolver::iterator it);
    bool isConnected( ) const;
    void write( const Message& _msg);
    void close();
    
private:
    void hConnect( const system::error_code& err);
    void hParseHeader( const system::error_code &err);
    void hParseBody( const system::error_code &err);
    void doWrite( Message msg);
    void hWrite( const system::error_code &err);
    void doClose();
    void displayMsg();
private:
    boost::asio::io_service&    m_service; /**< single service instance ? */
    tcp::socket                 m_socket;   
    Message                     m_msg_in;   /**< read message */
    dqMsg                       m_msgs;     /**< stored messages */
    bool                        m_status;  /**< cstr indicator */
};

bool initClientContext(int, char**);

#endif	/* CHATCLIENT_H */

