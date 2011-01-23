#ifndef __SUBPROCESS_HPP__
#define __SUBPROCESS_HPP__

#include <string>
#include <cstdarg>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <fstream>

#include <boost/foreach.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

namespace sp {
  class process_error : public std::exception {
    private:
      const char* message;
    public:
      process_error(const char* message) : message(message) {  }
      
      virtual const char* what() const throw() {
        return message;
      }
      
      virtual ~process_error() throw() {};
  };
  
  enum instance_state {
    initial,
    running,
    stopped
  };

  /**
   * Class to allow cheap copies of the instance class.
   */
  class instance_data {
    private:
      const int buffer_size;
      char** argv;
      boost::scoped_array<int> stdin_pipe;
      boost::scoped_array<int> stdout_pipe;
      boost::scoped_array<int> stderr_pipe;
      boost::scoped_array<char> buffer;
      
      boost::mutex mutex;
      
      pid_t process_id;
      int status;
      enum instance_state state;
      
      void cleanup();
    public:
      instance_data(char** argv);
      void start();
      
      int wait();
      void flush();
      
      int get_stdout();
      int get_stdin();
      int get_stderr();
      
      bool is_running();
      
      pid_t get_pid();
      int get_status();
      pid_t get_process_id();
      enum instance_state get_state();
      
      ~instance_data() {
        delete [] argv;
      }
  };
  
  class instance {
    private:
      boost::shared_ptr<instance_data> _data;
    public:
      instance(char** argv);
      void start();
      
      int wait();
      void flush();
      
      int get_stdout();
      int get_stdin();
      int get_stderr();
      
      bool is_running();
      
      pid_t get_pid();
      int get_status();
      pid_t get_process_id();
      enum instance_state get_state();
  };
  
  class process_desc {
    private:
      const std::string command;
      std::vector<std::string> args;
    public:
      process_desc(const std::string command);
      void arg(const std::string std);
      
      instance spawn();
  };
  
  class process {
    public:
      boost::shared_ptr<process_desc> p;
      
      process(const std::string command) : p(new process_desc(command)) {  }
      
      friend process operator <<(process sp, const std::string arg) {
        sp.p->arg(arg);
        return sp;
      }
      
      friend process operator <<(process sp, int arg) {
        sp.p->arg(boost::lexical_cast<std::string>(arg));
        return sp;
      }
      
      friend process operator <<(process sp, float arg) {
        sp.p->arg(boost::lexical_cast<std::string>(arg));
        return sp;
      }
      
      friend process operator <<(process sp, long arg) {
        sp.p->arg(boost::lexical_cast<std::string>(arg));
        return sp;
      }
      
      instance spawn() {
        return p->spawn();
      }
  };
}

#endif /* __SUBPROCESS_HPP__ */
