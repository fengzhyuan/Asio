/* 
 * \File   Message.h
 * \Author fengzh
 *
 * Created on September 12, 2015, 1:20 PM
 */

#ifndef MESSAGE_H
#define	MESSAGE_H

#include "utils.h"

/*!
 * \class Message
 * data description of message used for communication
 */
class Message {
public:
    enum { HEADER_LENGTH = 4};
    enum {MAX_BODY_LENGTH = 512 };

    Message():m_body_length(0) { }

    const char* data() const {
        return m_data;
    }

    char* data(){
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
        char mHeader[HEADER_LENGTH + 1]; 
        memset( mHeader, 0, sizeof(mHeader));
        strncat(mHeader, m_data, HEADER_LENGTH);
        m_body_length = atoi( mHeader);
        if (m_body_length > MAX_BODY_LENGTH) {
            m_body_length = 0;
            return false;
        }
        return true;
    }

    void encodeHeader()
    {
      char mHeader[HEADER_LENGTH + 1] = "";
      sprintf( mHeader, "%4d", m_body_length);
      memcpy( m_data, mHeader, HEADER_LENGTH);
    }

private:
    char    m_data[HEADER_LENGTH + MAX_BODY_LENGTH];
    size_t  m_body_length;
};

typedef std::deque<Message> dqMsg;

#endif	/* MESSAGE_H */

