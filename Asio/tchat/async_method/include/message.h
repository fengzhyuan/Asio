/* 
 * \File   Message.h
 * \Author fengzh
 *
 * Created on September 12, 2015, 1:20 PM
 */

#ifndef ASIO_ASYNC_METHOD_INCLUDE_MESSAGE_H_
#define ASIO_ASYNC_METHOD_INCLUDE_MESSAGE_H_

#include "utils.h"

enum HEADER_INFO{HEADER_LENGTH = 10, MAX_BODY_LENGTH = 512 };
enum MSG_CONTENT_TYPE{MSG_INIT=0,MSG_CONT,MSG_ENTER,MSG_LEAVE}; 
enum MSG_STREAM_TYPE{MSG_NS=MSG_LEAVE+1, MSG_S};
/*!
 * \class Message
 * data description of message used for communication
 */
class BaseMessage {
friend class boost::serialization::access;        
public:
    BaseMessage() {}
    BaseMessage(MSG_STREAM_TYPE iotype, MSG_CONTENT_TYPE type)
    : iotype_(iotype), type_(type) {}
    
    virtual ~BaseMessage() {}
    
    MSG_CONTENT_TYPE type() const {
        return type_;
    }
    MSG_STREAM_TYPE iotype() const {
        return iotype_;
    }
    virtual void build(const string &str) {}
    
    virtual const char* data() const {return NULL;}
    virtual char* data() {return NULL;}
    virtual const char* body() const {return NULL;}
    virtual char* body() {return NULL;}
    
    virtual size_t length() const {return 0;}
    virtual size_t body_length() const {return 0;}
    virtual void body_length(size_t) {}
    virtual void encode_head() {}
    virtual bool decode_head() {return false;}
    
    template<class Archive>
    void serialize(Archive &ar, const size_t version) {}
protected:
    MSG_CONTENT_TYPE type_;
    MSG_STREAM_TYPE  iotype_;
};

class Message : public BaseMessage {
 public:
    Message() : BaseMessage(MSG_NS, MSG_CONT), body_length_(0) {
        memset(data_, 0, sizeof(data_));
    }
    Message(MSG_CONTENT_TYPE type) : BaseMessage(MSG_NS, type){
        memset(data_, 0, sizeof(data_));
    }  

    virtual const char* data() const {
        return data_;
    }
    
    virtual void build(const string &str) {
        body_length(str.length());
        memcpy(body(), str.c_str(), body_length());
        encode_head();
    }
    virtual char* data() {
        return data_;
    }

    virtual size_t length() const {
        return HEADER_LENGTH + body_length_;
    }

    virtual const char* body() const {
        return data_ + HEADER_LENGTH;
    }

    virtual char* body() {
        return data_ + HEADER_LENGTH;
    }

    virtual size_t body_length() const {
        return body_length_;
    }

    virtual void body_length(size_t nlen) {
        body_length_ = (nlen > MAX_BODY_LENGTH ? MAX_BODY_LENGTH : nlen);
    }

    /** parse body length and message type
     * 
     */
    virtual bool decode_head() {
        sscanf(data_, "%d %d", &body_length_, &type_);
        
        if (body_length_ > MAX_BODY_LENGTH) {
            body_length_ = 0;
            return false;
        }
        return true;
    }

    virtual void encode_head() {
        char  header[HEADER_LENGTH + 1] = "";
        sprintf(header, "%d %d", body_length_, type_);
        memcpy(data_,  header, HEADER_LENGTH);
    }

 private:
    char    data_[HEADER_LENGTH + MAX_BODY_LENGTH];
    size_t  body_length_;
};

typedef boost::shared_ptr<BaseMessage> msg_ptr;
typedef std::deque<msg_ptr> dq_msg;
#endif  /* ASIO_ASYNC_METHOD_INCLUDE_MESSAGE_H_ */

