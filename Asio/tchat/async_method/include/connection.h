/* 
 * File:   Connection.h
 * Author: fengzh
 *
 * Created on September 15, 2015, 5:38 PM
 */

#ifndef ASIO_ASYNC_METHOD_INCLUDE_CONNECTION_H_
#define	ASIO_ASYNC_METHOD_INCLUDE_CONNECTION_H_

#include "message.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/tuple/tuple.hpp>



class Connection{
public:
    Connection(asio::io_service& service) : socket_(service), service_(service) {}
    
    tcp::socket& socket() {
        return socket_;
    }
    
    asio::io_service& service() {
        return service_;
    }
    
    /// Asynchronously write a data structure to the socket.
    template <typename T, typename Handler>
    void async_write(const T& t, Handler handler) {
        // Serialize the data first so we know how large it is.
        std::ostringstream archive_stream;
        boost::archive::text_oarchive archive(archive_stream);
        archive << t;
        outbound_data_ = archive_stream.str();

        // Format the header.
        std::ostringstream header_stream;
        header_stream << std::setw(Message::HEADER_LENGTH)
          << std::hex << outbound_data_.size();
        if (!header_stream || header_stream.str().size() != Message::HEADER_LENGTH) {
            // Something went wrong, inform the caller.
            boost::system::error_code error(boost::asio::error::invalid_argument);
            socket_.get_io_service().post(boost::bind(handler, error));
            return;
        }
        outbound_header_ = header_stream.str();

        // Write the serialized data to the socket. We use "gather-write" to send
        // both the header and the data in a single write operation.
        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::buffer(outbound_header_));
        buffers.push_back(boost::asio::buffer(outbound_data_));
        boost::asio::async_write(socket_, buffers, handler);
    }
    
    template <typename T, typename Handler>
    void async_read(T& t, Handler handler) {
      // Issue a read operation to read exactly the number of bytes in a header.
      void (Connection::*f)(
          const boost::system::error_code&,
          T&, boost::tuple<Handler>)
        = &Connection::handle_read_header<T, Handler>;
      boost::asio::async_read(socket_, boost::asio::buffer(inbound_header_),
          boost::bind(f,
            this, boost::asio::placeholders::error, boost::ref(t),
            boost::make_tuple(handler)));
    }

    /// Handle a completed read of a message header. The handler is passed using
    /// a tuple since boost::bind seems to have trouble binding a function object
    /// created using boost::bind as a parameter.
    template <typename T, typename Handler>
    void handle_read_header(const boost::system::error_code& e,
        T& t, boost::tuple<Handler> handler) {
        if (e) {
          boost::get<0>(handler)(e);
        }
        else {
          // Determine the length of the serialized data.
          std::istringstream is(std::string(inbound_header_, Message::HEADER_LENGTH));
          std::size_t inbound_data_size = 0;
          if (!(is >> std::hex >> inbound_data_size)) {
              // Header doesn't seem to be valid. Inform the caller.
              boost::system::error_code error(boost::asio::error::invalid_argument);
              boost::get<0>(handler)(error);
              return;
          }

          // Start an asynchronous call to receive the data.
          inbound_data_.resize(inbound_data_size);
          void (Connection::*f)(
              const boost::system::error_code&,
              T&, boost::tuple<Handler>)
            = &Connection::handle_read_data<T, Handler>;
          boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
            boost::bind(f, this,
              boost::asio::placeholders::error, boost::ref(t), handler));
        }
    }

  /// Handle a completed read of message data.
  template <typename T, typename Handler>
  void handle_read_data(const boost::system::error_code& e,
      T& t, boost::tuple<Handler> handler) {
    if (e) {
      boost::get<0>(handler)(e);
    }
    else {
        // Extract the data structure from the data just received.
        try {
            std::string archive_data(&inbound_data_[0], inbound_data_.size());
            std::istringstream archive_stream(archive_data);
            boost::archive::text_iarchive archive(archive_stream);
            archive >> t;
        }
        catch (std::exception& e) {
            // Unable to decode data.
            boost::system::error_code error(boost::asio::error::invalid_argument);
            boost::get<0>(handler)(error);
            return;
        }

        // Inform caller that data has been received ok.
        boost::get<0>(handler)(e);
    }
}
    
private:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::io_service& service_;
    string outbound_header_;
    /// Holds the outbound data.
    string outbound_data_;
    /// Holds an inbound header.
    char inbound_header_[Message::HEADER_LENGTH];
    /// Holds the inbound data.
    vector<char> inbound_data_;
};

#endif	/* ASIO_ASYNC_METHOD_INCLUDE_CONNECTION_H_ */

