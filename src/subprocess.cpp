#include "subprocess.hpp"

#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>

#define STDOUT_READ   stdout_pipe[0]
#define STDOUT_WRITE  stdout_pipe[1]
#define STDERR_READ   stderr_pipe[0]
#define STDERR_WRITE  stderr_pipe[1]
#define STDIN_READ    stdin_pipe[0]
#define STDIN_WRITE   stdin_pipe[1]
#define BUFFER_SIZE   4096

#include <iostream>

namespace sp {
  instance_data::instance_data(char** argv)
    : buffer_size(BUFFER_SIZE),
      argv(argv),
      stdin_pipe(new int[2]),
      stdout_pipe(new int[2]),
      stderr_pipe(new int[2]),
      buffer(new char[buffer_size]),
      status(0),
      state(initial)
  { }
  
  void
  instance_data::start() {
    {
      boost::mutex::scoped_lock(mutex);
      if (state != initial) {
        throw process_error("Instance data not in initial state");
      }
      state = running;
    }
    
    if (::pipe(stdin_pipe.get()) < 0) {
      throw process_error("Could not open stdin pipe");
    }
    
    if (::pipe(stdout_pipe.get()) < 0) {
      throw process_error("Could not open stdout pipe");
    }
    
    if (::pipe(stderr_pipe.get()) < 0) {
      throw process_error("Could not open stderr pipe");
    }
    
    process_id = fork();
    
    if (process_id < 0) {
      throw process_error("Could not spawn process");
    }
    
    if (process_id > 0) {
      ::close(STDIN_READ);
      ::close(STDOUT_WRITE);
      ::close(STDERR_WRITE);
      return;
    }
    
    ::close(STDIN_WRITE);
    ::close(STDOUT_READ);
    ::close(STDERR_READ);
    
    dup2(STDIN_READ,  0);
    ::close(STDIN_READ);
    
    dup2(STDOUT_WRITE, 1);
    ::close(STDOUT_WRITE);
    
    dup2(STDERR_WRITE, 2);
    ::close(STDERR_WRITE);
    
    int r = execvp(argv[0], argv);
    std::cerr << "execvp: Failed - " << strerror(errno) << std::endl;
    _exit(r);
  }
  
  void
  instance_data::cleanup() {
    ::close(STDOUT_READ);
    ::close(STDERR_READ);
    ::close(STDIN_WRITE);
  }
  
  int
  instance_data::wait() {
    boost::mutex::scoped_lock(mutex);

    if (state == initial) {
      throw process_error("Cannot wait for instance_data with state 'initial'");
    }
    
    if (state == stopped) {
      return WEXITSTATUS(status);
    }
    
    flush();
    waitpid(process_id, &status, 0);
    cleanup();
    state = stopped;
    return WEXITSTATUS(status);
  }
  
  void
  instance_data::flush() {
    boost::mutex::scoped_lock(mutex);
    
    if (state == stopped) {
      return;
    }
    
    int pr;
    
    pr = STDOUT_READ;
    while ((read(pr, buffer.get(), buffer_size)) > 0) { }
    
    pr = STDERR_READ;
    while ((read(pr, buffer.get(), buffer_size)) > 0) { }
    
    cleanup();
    state = stopped;
  }
  
  bool
  instance_data::is_running() {
    boost::mutex::scoped_lock(mutex);
    
    if (state != running) {
      return false;
    }
    
    pid_t p = waitpid(process_id, &status, WNOHANG);
    
    if (p == 0) {
      return true;
    }
    
    state = stopped;
    return false;
  }
  
  int
  instance_data::get_stdin() {
    return STDIN_WRITE;
  }
  
  int
  instance_data::get_stdout() {
    return STDOUT_READ;
  }
  
  int
  instance_data::get_stderr() {
    return STDERR_READ;
  }
  
  pid_t
  instance_data::get_pid() {
    return process_id;
  }
  
  int
  instance_data::get_status() {
    boost::mutex::scoped_lock(mutex);
    return WEXITSTATUS(status);
  }
  
  enum instance_state
  instance_data::get_state() {
    boost::mutex::scoped_lock(mutex);
    return state;
  }
  
  instance::instance(char** argv) : _data(new instance_data(argv)) {}
  
  void
  instance::start() {
    _data->start();
  }
  
  int
  instance::wait() {
    return _data->wait();
  }
  
  void
  instance::flush() {
    return _data->flush();
  }
  
  bool
  instance::is_running() {
    return _data->is_running();
  }
  
  int
  instance::get_stdin() {
    return _data->get_stdin();
  }
  
  int
  instance::get_stdout() {
    return _data->get_stdout();
  }
  
  int
  instance::get_stderr() {
    return _data->get_stderr();
  }
  
  pid_t
  instance::get_pid() {
    return _data->get_pid();
  }
  
  int
  instance::get_status() {
    return _data->get_status();
  }
  
  enum instance_state
  instance::get_state() {
    return _data->get_state();
  }
  
  process_desc::process_desc(const std::string command)
    :  command(command)
  { }
      
  void
  process_desc::arg(const std::string std) {
    args.push_back(std);
  }
  
  instance
  process_desc::spawn() {
    char** argv = new char*[args.size() + 2];
    
    {
      int i = 1;
      argv[args.size() + 1] = NULL;
      
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
    
    return instance(argv);
  }
}
