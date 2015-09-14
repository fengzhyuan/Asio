/* 
 * File:   Client.h
 * Author: fengzh
 *
 * Created on September 11, 2015, 6:25 PM
 */

#ifndef ASIO_ASYNC_METHOD_INCLUDE_CLIENT_H_
#define ASIO_ASYNC_METHOD_INCLUDE_CLIENT_H_

#include "utils.h"
#include "Message.h"

class Client{
 public:
    Client(io_service& _service, tcp::resolver::iterator _it);
    bool isConnected() const;
    void write(const Message& _msg);
    void close();
    const string getName() const;
 private:
    void hConnect(const system::error_code&);
    void hRead(const system::error_code&);
    void hParseHeader(const system::error_code&);
    void hParseBody(const system::error_code&);
    void doWrite(Message msg);
    void hWrite(const system::error_code&);
    void doClose();
    void displayMsg();
 private:
    boost::asio::io_service&    m_service; /**< single service instance ? */
    tcp::socket                 m_socket;
    Message                     m_msg_in;   /**< read message */
    dqMsg                       m_msgs;     /**< stored messages */
    bool                        m_status;  /**< cstr indicator */
    string                      m_uid; 
};

bool initClientContext(int, char**);

#endif  /* ASIO_ASYNC_METHOD_INCLUDE_CLIENT_H_ */

