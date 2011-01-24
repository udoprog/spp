#ifndef __SESSION_HPP__
#define __SESSION_HPP__

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_array.hpp>

#include <boost/archive/text_oarchive.hpp>

#include "data_digest.hpp"

class session
{
public:
  session(boost::asio::io_service& io_service);
  boost::asio::ip::tcp::socket& socket();
  void start();

  void write(std::string);
private:
  void handle_read_data(boost::shared_array<char> buffer, uint32_t s);
  void handle_write_size(const boost::system::error_code& ec, std::string response);
  void handle_write(const boost::system::error_code& ec);
  void handle_error(const boost::system::error_code& ec);

  void handle_ping(boost::archive::text_oarchive& oa);

  boost::asio::ip::tcp::socket socket_;
  data_digest digest;
};

#endif /*__SESSION_HPP__*/
