/* 
 * File:   Client.h
 * Author: fengzh
 *
 * Created on September 11, 2015, 6:25 PM
 */

#ifndef ASIO_ASYNC_METHOD_INCLUDE_CLIENT_H_
#define ASIO_ASYNC_METHOD_INCLUDE_CLIENT_H_

#include "message.h"
#include "connection.h"

class Client{
 public:
    Client(io_service& _service, tcp::resolver::iterator _it);
    bool status() const;
    void write(const Message&);
    void swrite(const SeriableMessage&);
    void close();
    void sclose();
    const string getName() const;
 private:
    void h_connect(const system::error_code&);
    void h_sconnect(const system::error_code&);
    void h_read(const system::error_code&);
    void h_sread(const system::error_code&);
    void h_read_header(const system::error_code&);
    void h_read_body(const system::error_code&);
    void do_write(Message msg);
    void do_swrite(SeriableMessage msg);
    void h_write(const system::error_code&);
    void do_close();
    void display();
    void sdisplay();
    
 private:
    asio::io_service &service_; 
    tcp::socket     socket_;
    Connection      connection_; 
    Message         msg_;
    vector<SeriableMessage> smsg_;      /**< read message */
    dqMsg           msg_list_; /**< stored messages */
    bool            status_;   /**< cstr indicator */
    string          uid_; 
};

bool InitClientContext(int, char**);

#endif  /* ASIO_ASYNC_METHOD_INCLUDE_CLIENT_H_ */

