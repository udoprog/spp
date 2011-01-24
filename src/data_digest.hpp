#ifndef __DATA_DIGEST_HPP__
#define __DATA_DIGEST_HPP__

#include <stdint.h>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/shared_array.hpp>

class data_digest {
public:
  typedef boost::shared_array<char> buffer_ptr;

  typedef boost::function<void(buffer_ptr, uint32_t)> success_handle;
  typedef boost::function<void(const boost::system::error_code)> error_handle;

  struct read_data_type {
    read_data_type(success_handle s) : success(s) {}

    union {
      uint32_t i;
      char     c[sizeof(uint32_t)];
    } size;
    buffer_ptr buffer;
    success_handle success;
  };

  typedef boost::shared_ptr<read_data_type> data_ptr;

  data_digest(boost::asio::ip::tcp::socket& socket_, error_handle error)
    : socket_(socket_), error(error)
  {
  }

  void async_read_data(success_handle success)
  {
    data_ptr data(new read_data_type(success));

    socket_.async_read_some(boost::asio::buffer(data->size.c, sizeof(data->size)),
      boost::bind(&data_digest::handle_data_size, this,
          boost::asio::placeholders::error, data));
  }

private:
  void handle_data_size(const boost::system::error_code& ec, data_ptr data)
  {
    if (ec) {
      error(ec);
      return;
    }
    
    data->size.i = ntohl(data->size.i);
    data->buffer.reset(new char[data->size.i]);

    socket_.async_read_some(boost::asio::buffer(data->buffer.get(), data->size.i),
      boost::bind(&data_digest::handle_data, this,
          boost::asio::placeholders::error, data)
    );
  }

  void handle_data(const boost::system::error_code& ec, data_ptr data)
  {
    if (ec) {
      error(ec);
      return;
    }
    
    data->success(data->buffer, data->size.i);
  }

  boost::asio::ip::tcp::socket& socket_;
  error_handle error;
};

#endif /*__DATA_DIGEST_HPP__*/
