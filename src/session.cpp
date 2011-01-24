#include "session.hpp"

#include "messages.hpp"

session::session(boost::asio::io_service& io_service)
  : socket_(io_service),
    digest(socket_,
      boost::bind(&session::handle_error, this, _1)
    )
{
}

boost::asio::ip::tcp::socket&
session::socket()
{
  return socket_;
}

#include <iostream>

void
session::start()
{
  boost::asio::ip::tcp::endpoint remote_ep = socket_.remote_endpoint();
  boost::asio::ip::address remote_ad = remote_ep.address();
  std::string ip = remote_ad.to_string();
  std::cout << "ACCEPTED: " << ip << std::endl;
  digest.async_read_data(boost::bind(&session::handle_read_data, this, _1, _2));
}

void
session::write(std::string response) {
  uint32_t len = htonl(response.size());
  boost::asio::async_write(socket_, 
    boost::asio::buffer(reinterpret_cast<char*>(&len), sizeof(uint32_t)), 
    boost::bind(&session::handle_write_size, this, 
      boost::asio::placeholders::error, response));
}

void
session::handle_write_size(const boost::system::error_code& ec, std::string response) {
  if (ec) {
    std::cout << "failed to write: " << ec.message() << std::endl;
    socket_.close();
  }

  boost::asio::async_write(socket_, 
    boost::asio::buffer(response, response.size()), 
    boost::bind(&session::handle_write, this, 
      boost::asio::placeholders::error));
}

void
session::handle_write(const boost::system::error_code& ec) {
  if (ec) {
    std::cout << "failed to write: " << ec.message() << std::endl;
    socket_.close();
  }
}

void
session::handle_read_data(boost::shared_array<char> buffer, uint32_t s)
{
  using spp_messages::message_header;
  using spp_messages::pong;

  message_header header;

  std::string res(buffer.get(), s);
  std::istringstream ifs(res);
  boost::archive::text_iarchive ia(ifs);
  ia >> header;

  std::ostringstream ofs;
  boost::archive::text_oarchive oa(ofs);

  switch (header.get_type()) {
    case spp_messages::ping:
      handle_ping(oa);
      break;
    default:
      socket_.close();
      return;
  }

  std::string response = ofs.str();
  write(response);
}

void
session::handle_ping(boost::archive::text_oarchive& oa) {
  using spp_messages::message_header;
  using spp_messages::message_pong;

  std::cout << "got ping" << std::endl;

  message_pong pong;
  message_header header = pong.get_header();
  oa << header;
}

void
session::handle_error(const boost::system::error_code& ec) {
  std::cout << "Got Error: " << ec.message() << std::endl;
  socket_.close();
}
