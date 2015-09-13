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
class Message{
public:
    Message():m_body_length(0){}
    
    const char* data() const{
        return m_data;
    }
    
    char* data() {
        return m_data;
    }
    
    size_t length() const {
        return header_length + m_body_length;
    }
    
    const char* body() const{
        return m_data + header_length;
    }
    
    char* body() {
        return m_data + header_length;
    }
    
    size_t bodyLength() const {
        return m_body_length;
    }
    
    void bodyLength( size_t nlen) {
        m_body_length = ( nlen > max_body_length ? max_body_length : nlen);
    }
    
    // data encoding & decoding
    void encodeHeader() {
        char m_header[header_length + 1] = "";
        sprintf( m_header, "%4d", m_body_length);
        memcpy( m_data, m_header, header_length);
    }
    
    bool decodeHeader() {
        char m_header[header_length + 1] = "";
        strncat( m_header, m_data, header_length);
        m_body_length = atoi( m_header);
        
        if( m_body_length > max_body_length ){
            m_body_length = 0;
            cout << "error: decodeHeader\n";
            return false;
        }
        return true;
    }
public:
    enum { header_length = 4, max_body_length = 512};
    
private:
    char m_data[header_length + max_body_length];
    size_t m_body_length;
};

typedef std::deque<Message> dqMsg;

#endif	/* MESSAGE_H */

