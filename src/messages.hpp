#ifndef __SPP_MESSAGES_HPP__
#define __SPP_MESSAGES_HPP__

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace spp_messages {
  typedef enum {
    none = 0x0,
    ping = 0x10,
    pong = 0x11
  } message_type;

  class message_header 
  {
  public:
    message_header()
      : type(none) {}

    message_header(message_type type)
      : type(type) {}

    message_type get_type() {
      return type;
    }
  private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & type;
    }

    message_type type;
  };

  class message_base
  {
  public:
    virtual message_header get_header() = 0;
  private:
  };

  class message_ping : public message_base {
  public:
    message_header
    get_header()
    {
      return message_header(ping);
    }
  };

  class message_pong : public message_base {
  public:
    message_header
    get_header()
    {
      return message_header(pong);
    }
  };
}

#endif /*__SPP_MESSAGES_HPP__*/
