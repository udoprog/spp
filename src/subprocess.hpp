#ifndef __SUBPROCESS_HPP__
#define __SUBPROCESS_HPP__

#include <string>
#include <cstdarg>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <fstream>

#include <sys/types.h>
#include <sys/wait.h>

#include <boost/foreach.hpp>
#include <boost/scoped_array.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

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

#define PARENT_READ readpipe[0]
#define CHILD_WRITE readpipe[1]
#define CHILD_READ  writepipe[0]
#define PARENT_WRITE  writepipe[1]

class subprocess {
  public:
    const int buffer_size;
  private:
    const std::string command;
    std::vector<std::string> args;
    pid_t process_id;
    boost::scoped_array<int> readpipe;
    boost::scoped_array<int> writepipe;
    boost::scoped_array<char> buffer;
    int status;
    bool is_running;
    bool is_closed;
    
    boost::mutex mutex;
    
    void close() {
      if (is_closed) return;
      ::close(PARENT_READ);
      ::close(PARENT_WRITE);
      is_closed = true;
    }
  public:
    subprocess(const std::string command)
      : buffer_size(4096),
        command(command),
        readpipe(new int[2]),
        writepipe(new int[2]),
        buffer(new char[buffer_size]),
        status(0),
        is_running(false),
        is_closed(false)
    { }
    
    void arg(const std::string std) {
      args.push_back(std);
    }
    
    void start() {
      {
        boost::mutex::scoped_lock(mutex);
        
        if (is_running) {
          return;
        }
        
        is_running = true;
      }
      
      if (pipe(readpipe.get()) < 0) {
        throw process_error("Could not open read pipe");
      }
      
      if (pipe(writepipe.get()) < 0) {
        throw process_error("Could not open write pipe");
      }
      
      process_id = fork();
      
      if (process_id < 0) {
        throw process_error("Could not spawn process");
      }
      
      if (process_id > 0) {
        ::close(CHILD_READ);
        ::close(CHILD_WRITE);
        return;
      }
      
      ::close(PARENT_WRITE);
      ::close(PARENT_READ);
      
      dup2(CHILD_READ,  0);
      ::close(CHILD_READ);
      
      dup2(CHILD_WRITE, 1);
      ::close(CHILD_WRITE);
      
      /* setup arguments */
      char** argv = new char*[args.size() + 2];
      
      {
        int i = 1;
        argv[args.size()] = NULL;
        
        /* populate the command */
        argv[0] = new char[command.size() + 1];
        memcpy(argv[0], command.c_str(), command.size());
        argv[0][command.size()] = NULL;
        
        /* populate the arguments */
        BOOST_FOREACH(std::string arg, args) {
          argv[i] = new char[arg.size() + 1];
          memcpy(argv[i], arg.c_str(), arg.size());
          argv[i][arg.size()] = NULL;
          i++;
        }
      }
      
      execvp(argv[0], argv);
      _exit(1);
    }
    
    int wait() {
      boost::mutex::scoped_lock(mutex);
      
      waitpid(process_id, &status, 0);
      close();
      return WEXITSTATUS(status);
    }
    
    void flush() {
      boost::mutex::scoped_lock(mutex);
      
      if (is_closed) {
        return;
      }
      
      int pr = PARENT_READ;
      while ((read(pr, buffer.get(), buffer_size)) > 0) { }
      
      close();
    }
    
    bool running() {
      boost::mutex::scoped_lock(mutex);
      
      if (!is_running) {
        return false;
      }
      
      pid_t p = waitpid(process_id, &status, WNOHANG);
      
      if (p == 0) {
        return true;
      }
      
      is_running = false;
      return false;
    }
    
    int get_stdout() {
      return PARENT_READ;
    }
    
    int get_stdin() {
      return PARENT_WRITE;
    }
    
    pid_t get_pid() {
      return process_id;
    }
    
    int get_status() {
      return WEXITSTATUS(status);
    }

    friend subprocess& operator <<(subprocess& sp, const std::string arg) {
      sp.arg(arg);
      return sp;
    }
    
    friend subprocess& operator <<(subprocess& sp, int arg) {
      sp.arg(boost::lexical_cast<std::string>(arg));
      return sp;
    }
    
    friend subprocess& operator <<(subprocess& sp, float arg) {
      sp.arg(boost::lexical_cast<std::string>(arg));
      return sp;
    }
    
    friend subprocess& operator <<(subprocess& sp, long arg) {
      sp.arg(boost::lexical_cast<std::string>(arg));
      return sp;
    }
};

#endif /* __SUBPROCESS_HPP__ */
