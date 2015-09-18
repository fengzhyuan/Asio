/*!
 * \file chatClient.cpp
 * 
 */
#include "include/client.h"

Client::Client(io_service& service,
                tcp::resolver::iterator iter)
        : service_(service),
          connection_(new Connection(service)),
          socket_(service),
          status_(true) {
    cout << "input name:" << endl;
    cin >> uid_; // TODO: check validity on server

//    boost::asio::async_connect(connection_->socket(), iter,
//        boost::bind(&Client::h_sconnect, this,
//          boost::asio::placeholders::error));
        boost::asio::async_connect(socket_, iter,
        boost::bind(&Client::h_connect, this,
          boost::asio::placeholders::error));
  }

/**
 * \TODO check if timeout during ctor initialization
 * // boost::asio::deadline_timer
 * @return return false if ctor failed
 */
bool Client::status() const {
    return status_;
}

/**
 * 
 * @param _msg message to be stored
 */
void Client::write(const Message& msg) {
    service_.post(boost::bind(&Client::do_write, this, msg));
}

void Client::swrite(const SerializedMessage& msg) {
//    connection_->service().post(boost::bind(&Client::do_swrite, this, msg));
}

void Client::close() {
    service_.post(boost::bind(&Client::do_close, this));
}

void Client::sclose() {
    connection_->service().post(boost::bind(&Client::do_close, this));
}

/**
 * cstr connection handler
 * @param err
 */
void Client::h_connect(const system::error_code& err) {
    if (!err) {
        cout << "connected to server " << endl;
        fflush(stdout);
        // TODO: add password
        Message msgInitInfo(MSG_INIT);
        msgInitInfo.build(this->uid_);

        // send identification info to server
        boost::asio::async_write(socket_,
            boost::asio::buffer(msgInitInfo.data(),
              msgInitInfo.length()),
            boost::bind(&Client::h_read, this,
              boost::asio::placeholders::error));
    }   
}

void Client::h_sconnect(const system::error_code& error) {
    if (!error) {
        cout << "connected to server " << endl;
        fflush(stdout);
        // TODO: add password
        SerializedMessage msgInit(MSG_INIT, this->uid_);
        smsg_list_.push_back(msgInit);
        // send identification info to server
        connection_->async_write(smsg_list_, 
            boost::bind(&Client::h_sread, this, 
            asio::placeholders::error));
    }
}

void Client::h_read(const system::error_code& error) {
    if (!error) {
        async_read(socket_,
            buffer(msg_.data(), HEADER_LENGTH),
            bind(&Client::h_read_header, this,
                asio::placeholders::error));
    }
}

void Client::h_sread(const system::error_code& error) {
    if (!error) {
        sdisplay();
        connection_->async_read(smsg_list_, 
                boost::bind(&Client::h_sread, this, 
                    asio::placeholders::error));
    }
}
/**
 * read all msgs and display them
 * @param error error code (implicitly convert to bool type)
 */
void Client::h_read_header(const boost::system::error_code& error) {
    if (!error && msg_.decode_head()) {
      boost::asio::async_read(socket_,
          boost::asio::buffer(msg_.body(), msg_.body_length()),
          boost::bind(&Client::h_read_body, this,
            boost::asio::placeholders::error));
    }
    else {
        do_close();
    }
}

/**
 * call parse_header hr first
 * @param err
 */
void Client::h_read_body(const boost::system::error_code& error) {
    if (!error) {
        display();
        h_read(error);
    }
    else {
        do_close();
    }
}

/**
 * write all messages into socket
 * @param err
 */
void Client::do_write(Message msg) {
    bool bEmpty = msg_list_.empty();
    msg_list_.push_back(msg);
    
    if (bEmpty) { // each client can only do write operation synchronously
        boost::asio::async_write(socket_,
            boost::asio::buffer(msg_list_.front().data(),
              msg_list_.front().length()),
            boost::bind(&Client::h_write, this,
              boost::asio::placeholders::error));
    }
}

void Client::do_swrite(SerializedMessage msg) {
    bool empty = smsg_list_.empty();
    smsg_list_.push_back(msg);
    if (empty) {
        connection_->async_write(smsg_list_, 
            boost::bind(&Client::h_swrite, this,
                boost::asio::placeholders::error));
    }
}

void Client::h_write(const boost::system::error_code& error) {
    if  (!error) {
        msg_list_.pop_front();
        if  (!msg_list_.empty()) {
            boost::asio::async_write(socket_,
                boost::asio::buffer(msg_list_.front().data(),
                  msg_list_.front().length()),
                boost::bind(&Client::h_write, this,
                  boost::asio::placeholders::error));
        }
    }
    else {
        do_close();
    }
}

void Client::h_swrite(const system::error_code& error) {
    if (!error) {
        connection_->async_write(smsg_list_, 
            boost::bind(&Client::h_swrite, this, asio::placeholders::error));
    }
}

void Client::do_close() {
    socket_.close();
}

const string Client::get_name() const {
    return uid_;
}
void Client::display() {
    Color::Modifier cfont;
    int msgType = msg_.type();
    switch (msgType) {
        case MSG_ENTER :
            cfont = Color::Modifier(Color::FG_GREEN); break;
        case MSG_LEAVE :
            cfont = Color::Modifier(Color::FG_RED); break;
        default:
            cfont = Color::Modifier(Color::FG_DEFAULT);
    }
    Color::Modifier cref, cline = Color::Modifier(Color::FG_BLUE);
    
    char body[MAX_BODY_LENGTH+1]="";
    memcpy(body, msg_.body(), msg_.body_length());
    if (msgType == MSG_CONT)
        cout << cline << "----------------------------" << endl;
    cout << cfont << body << cref << endl;
    if (msgType == MSG_CONT)
        cout << cline << "----------------------------" << cref << endl;
}

void Client::sdisplay() {
    
    for (SerializedMessage msg : smsg_list_) {
        Color::Modifier cfont;
        int msg_type = msg.type();
        switch (msg_type) {
            case MSG_ENTER :
                cfont = Color::Modifier(Color::FG_GREEN); break;
            case MSG_LEAVE :
                cfont = Color::Modifier(Color::FG_RED); break;
            default:
                cfont = Color::Modifier(Color::FG_DEFAULT);
        }
        Color::Modifier cref, cline = Color::Modifier(Color::FG_BLUE);

        char body[MAX_BODY_LENGTH+1]="";
        memcpy(body, msg_.body(), msg_.body_length());
        if (msg_type == MSG_CONT)
            cout << cline << "----------------------------" << endl;
        cout << cfont << body << cref << endl;
        if (msg_type == MSG_CONT)
            cout << cline << "----------------------------" << cref << endl;
    }
}
/**
 * client context initialization
 * @param argc main argc
 * @param argv main argv 
 * @return [true: cstr established][false: cstr failed]
 */
bool InitClientContext(int argc, char**argv) {
    try{
        if (argc != 4) {
            cout << " invalid param [<host> <port>]\n";
            return false;
        }
        
        io_service mService;

        tcp::resolver mResolver(mService);
        tcp::resolver::query mQuery(argv[2], argv[3]);
        // domain name: 1-to-many
        tcp::resolver::iterator mIterator = mResolver.resolve(mQuery);

        boost::shared_ptr<Client> pClient(new Client(mService, mIterator));

        boost::thread mThread(boost::bind(&io_service::run, &mService));

        char input[MAX_BODY_LENGTH + 1];
        while (cin.getline(input, MAX_BODY_LENGTH + 1)) {
            if (!strcmp(input, "EXIT") ) {
                break;
            } 
            else if (!strlen(input)) {
                continue;
            }
            
            Message msg;
            string strMsg = pClient->get_name() + ":" + string(input);
            msg.build(strMsg);
            pClient->write(msg);
        }

        pClient->close();
        mThread.join();
    }
    catch (std::exception& e) {
        cerr << "Exception: " << e.what() << endl;
        return false;
    }
    return true;
}
