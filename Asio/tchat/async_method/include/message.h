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
enum MSG_TYPE{MSG_INIT=0,MSG_CONT,MSG_ENTER,MSG_LEAVE}; 
/*!
 * \class Message
 * data description of message used for communication
 */
class BaseMessage {
public:
    virtual ~BaseMessage() {}
    template<class Archive>
    void serialize(Archive &ar, const size_t version) {}
    
friend class boost::serialization::access;    
};

class Message : public BaseMessage {
 public:
    Message() : body_length_(0), type_(MSG_CONT), encoded_(false) {
        memset(data_, 0, sizeof(data_));
    }
    Message(int type) : Message(){
        type_ = type;
        memset(data_, 0, sizeof(data_));
    }  

    const char* data() const {
        return data_;
    }
    
    void build(const string &str) {
        body_length(str.length());
        memcpy(body(), str.c_str(), body_length());
        encode_head();
    }
    char* data() {
        return data_;
    }

    size_t length() const {
        return HEADER_LENGTH + body_length_;
    }

    const char* body() const {
        return data_ + HEADER_LENGTH;
    }

    char* body() {
        return data_ + HEADER_LENGTH;
    }

    size_t body_length() const {
        return body_length_;
    }

    void body_length(size_t nlen) {
        body_length_ = (nlen > MAX_BODY_LENGTH ? MAX_BODY_LENGTH : nlen);
    }

    const int type() const {
        return type_;
    }
    /** parse body length and message type
     * 
     */
    bool decode_head() {
        sscanf(data_, "%d %d", &body_length_, &type_);
        
        if (body_length_ > MAX_BODY_LENGTH) {
            body_length_ = 0;
            return false;
        }
        return true;
    }

    void encode_head() {
        char  header[HEADER_LENGTH + 1] = "";
        sprintf(header, "%d %d", body_length_, type_);
        memcpy(data_,  header, HEADER_LENGTH);
        encoded_ = true;
    }

 private:
    char    data_[HEADER_LENGTH + MAX_BODY_LENGTH];
    size_t  body_length_;
    bool    encoded_;
    int     type_;
};

class SerializedMessage : public BaseMessage {
public:
friend class boost::serialization::access; // serializable
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        // serialize base class information
        ar & boost::serialization::base_object<BaseMessage>(*this);
        ar & data_;
        ar & body_length_;
        ar & type_;
    }
    
public:
    SerializedMessage() : type_(MSG_CONT) {}
    SerializedMessage(const string& str) : SerializedMessage(){
        data_ = vector<char>(str.begin(), str.end());
    }  
    SerializedMessage(int type, const string& str) : type_(type) {
        data_ = vector<char>(str.begin(), str.end());
    }
    const int type() const {
        return type_;
    }
    vector<char>& data() {
        return data_;
    }
    const vector<char>& data() const {
        return data_;
    }
private:
    vector<char> data_;
    size_t  body_length_;
    int     type_;
};
typedef std::deque<Message> dqMsg;
typedef std::deque<SerializedMessage> dsSMsg;
#endif  /* ASIO_ASYNC_METHOD_INCLUDE_MESSAGE_H_ */

