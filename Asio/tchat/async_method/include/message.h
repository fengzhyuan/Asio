/* 
 * \File   Message.h
 * \Author fengzh
 *
 * Created on September 12, 2015, 1:20 PM
 */

#ifndef ASIO_ASYNC_METHOD_INCLUDE_MESSAGE_H_
#define ASIO_ASYNC_METHOD_INCLUDE_MESSAGE_H_

#include "utils.h"

/*!
 * \class Message
 * data description of message used for communication
 */
class BaseMessage {
public:
    virtual ~BaseMessage() {}
};

class Message : public BaseMessage {
 public:
    Message() : m_body_length(0), m_type(MSG_CONT), m_encoded(false) {
        memset(m_data, 0, sizeof(m_data));
    }
    Message(int type) : Message(){
        m_type = type;
        memset(m_data, 0, sizeof(m_data));
    }  
//    // check if encoded whenever use data
//    Message(const Message &inst) {
//        m_type = inst.m_type;
//        m_encoded = inst.m_encoded;
//        m_body_length = inst.m_body_length;
//        memcpy(m_data, inst.m_data, inst.length());
//        if (!m_encoded) {
//            this->encodeHeader();
//        }
//    }
//    Message& operator=(const Message &inst) {
//        if (!m_encoded) {
//            this->encodeHeader();
//        }
//    }
    const char* data() const {
        return m_data;
    }
    void buildMsg(const string &str) {
        bodyLength(str.length());
        memcpy(body(), str.c_str(), bodyLength());
        encodeHeader();
    }
    char* data() {
        return m_data;
    }

    size_t length() const {
        return HEADER_LENGTH + m_body_length;
    }

    const char* body() const {
        return m_data + HEADER_LENGTH;
    }

    char* body() {
        return m_data + HEADER_LENGTH;
    }

    size_t bodyLength() const {
        return m_body_length;
    }

    void bodyLength(size_t nlen) {
        m_body_length = (nlen > MAX_BODY_LENGTH ? MAX_BODY_LENGTH : nlen);
    }

    const bool isSysMsg() const{
        return m_type == MSG_INIT;
    }
    const int msgType() const {
        return m_type;
    }
    /** parse body length and message type
     * 
     */
    bool decodeHeader() {
        sscanf(m_data, "%d %d", &m_body_length, &m_type);
        
        if (m_body_length > MAX_BODY_LENGTH) {
            m_body_length = 0;
            return false;
        }
        return true;
    }

    void encodeHeader() {
        char  header[HEADER_LENGTH + 1] = "";
        sprintf(header, "%d %d", m_body_length, m_type);
        memcpy(m_data,  header, HEADER_LENGTH);
        m_encoded = true;
    }
    
public:
    enum {HEADER_LENGTH = 10, MAX_BODY_LENGTH = 512 };
    enum MSG_TYPE{MSG_INIT=0,MSG_CONT,MSG_ENTER,MSG_LEAVE};
 private:
    char    m_data[HEADER_LENGTH + MAX_BODY_LENGTH];
    size_t  m_body_length;
    bool    m_encoded;
    int     m_type;
};

typedef std::deque<Message> dqMsg;

#endif  /* ASIO_ASYNC_METHOD_INCLUDE_MESSAGE_H_ */

