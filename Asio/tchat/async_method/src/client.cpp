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
          status_(true),
          msg_(new Message()){
    cout << "input name:" << endl;
    cin >> uid_; // TODO: check validity on server
    
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
void Client::write(msg_ptr msg) {
    service_.post(boost::bind(&Client::do_write, this, msg));
}

void Client::close() {
    service_.post(boost::bind(&Client::do_close, this));
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

        msg_ptr new_msg(new Message(MSG_INIT));
        new_msg->build(this->get_name());
        // send identification info to server
        boost::asio::async_write(socket_,
            boost::asio::buffer(new_msg->data(),
              new_msg->length()),
            boost::bind(&Client::h_read, this,
              boost::asio::placeholders::error));
    }   
}

void Client::h_read(const system::error_code& error) {
    if (!error) {
        async_read(socket_,
            buffer(msg_->data(), HEADER_LENGTH),
            bind(&Client::h_read_header, this,
                asio::placeholders::error));
    }
}
/**
 * read all msgs and display them
 * @param error error code (implicitly convert to bool type)
 */
void Client::h_read_header(const boost::system::error_code& error) {
    if (!error && msg_->decode_head()) {
      boost::asio::async_read(socket_,
          boost::asio::buffer(msg_->body(), msg_->body_length()),
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
void Client::do_write(msg_ptr msg) {
    bool empty = msg_list_.empty();
    msg_list_.push_back(msg);
    
    if (empty) { // each client can only do write operation synchronously
        boost::asio::async_write(socket_,
            boost::asio::buffer(msg_list_.front()->data(),
              msg_list_.front()->length()),
            boost::bind(&Client::h_write, this,
              boost::asio::placeholders::error));
    }
}

void Client::h_write(const boost::system::error_code& error) {
    if  (!error) {
        msg_list_.pop_front();
        if  (!msg_list_.empty()) {
            boost::asio::async_write(socket_,
                boost::asio::buffer(msg_list_.front()->data(),
                  msg_list_.front()->length()),
                boost::bind(&Client::h_write, this,
                  boost::asio::placeholders::error));
        }
    }
    else {
        do_close();
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
    int msgType = msg_->type();
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
    memcpy(body, msg_->body(), msg_->body_length());
    if (msgType == MSG_CONT)
        cout << cline << "----------------------------" << endl;
    cout << cfont << body << cref << endl;
    if (msgType == MSG_CONT)
        cout << cline << "----------------------------" << cref << endl;
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
        
        io_service m_service;

        tcp::resolver m_resolver(m_service);
        tcp::resolver::query m_query(argv[2], argv[3]);
        // domain name: 1-to-many
        tcp::resolver::iterator m_iterator = m_resolver.resolve(m_query);

        boost::shared_ptr<Client> m_client(new Client(m_service, m_iterator));

        boost::thread m_thread(boost::bind(&io_service::run, &m_service));

        char input[MAX_BODY_LENGTH + 1];
        while (cin.getline(input, MAX_BODY_LENGTH + 1)) {
            if (!strcmp(input, "EXIT") ) {
                break;
            } 
            else if (!strlen(input)) {
                continue;
            }
            
            msg_ptr new_msg(new Message());
            string str = m_client->get_name() + ":" + string(input);
            new_msg->build(str);
            m_client->write(new_msg);
        }

        m_client->close();
        m_thread.join();
    }
    catch (std::exception& e) {
        cerr << "Exception: " << e.what() << endl;
        return false;
    }
    return true;
}
