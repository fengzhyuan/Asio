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
class Message {
 public:
    Message() : m_body_length(0) {}

    const char* data() const {
        return m_data;
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

    bool decodeHeader() {
        char header[HEADER_LENGTH + 1]; 
        memset(header, 0, sizeof(header));
        strncat(header, m_data, HEADER_LENGTH);
        m_body_length = atoi(header);
        if (m_body_length > MAX_BODY_LENGTH) {
            m_body_length = 0;
            return false;
        }
        return true;
    }

    void encodeHeader() {
        char  header[HEADER_LENGTH + 1] = "";
        sprintf( header, "%4d", m_body_length);
        memcpy(m_data,  header, HEADER_LENGTH);
    }
    
public:
    enum {HEADER_LENGTH = 4, MAX_BODY_LENGTH = 512 };

 private:
    char    m_data[HEADER_LENGTH + MAX_BODY_LENGTH];
    size_t  m_body_length;
};

typedef std::deque<Message> dqMsg;

#endif  /* ASIO_ASYNC_METHOD_INCLUDE_MESSAGE_H_ */

