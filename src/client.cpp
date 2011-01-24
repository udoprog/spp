#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include <sstream>
#include <boost/scoped_array.hpp>

#include "messages.hpp"

using boost::asio::ip::tcp;

enum { max_length = 1024 };

class spp_client {
public:
  spp_client(std::string host, std::string port)
    : io_service(),
      socket(io_service)
  {
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(tcp::v4(), host, port);
    tcp::resolver::iterator iterator = resolver.resolve(query);

    socket.connect(*iterator);
  }

  void
  write(const char* b, size_t l) {
    uint32_t len = htonl(l);
    boost::asio::write(socket, boost::asio::buffer(reinterpret_cast<char*>(&len), sizeof(len)));
    boost::asio::write(socket, boost::asio::buffer(b, l));
  }

  std::string
  read() {
    uint32_t len;
    boost::asio::read(socket, boost::asio::buffer(reinterpret_cast<char*>(&len), sizeof(len)));
    len = ntohl(len);
    boost::scoped_array<char> buffer(new char[len]);
    boost::asio::read(socket, boost::asio::buffer(buffer.get(), len));
    return std::string(buffer.get(), len);
  }

  void
  send(spp_messages::message_base& message) {
    using spp_messages::message_header;
    message_header header = message.get_header();

    {
      std::ostringstream ofs;
      boost::archive::text_oarchive oa(ofs);
      oa << header;
      std::string message = ofs.str();
      write(message.c_str(), message.size());
    }
  }
private:
  boost::asio::io_service io_service;
  tcp::socket socket;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: spp-client <host> <port>\n";
      return 1;
    }

    spp_client client(argv[1], argv[2]);

    using namespace std; // For strlen.
    spp_messages::message_ping p;
    client.send(p);
    std::string response = client.read();
    std::cout << response << std::endl;
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
