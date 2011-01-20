
/* forkexec */
#include <string>
#include <cstdarg>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <cstring>

#include <sys/types.h>
#include <sys/wait.h>

#include <boost/foreach.hpp>

class process_error : public std::exception {
  private:
    const char* message;
  public:
    process_error(const char* message) : message(message) {  }
    
    virtual const char* what() throw() {
      return message;
    }
};

class forkexec {
  private:
    const std::string command;
    std::vector<std::string> args;
    pid_t process_id;
  public:
    forkexec(const std::string command, ...) : command(command) {
      va_list ap;
      va_start(ap, command);
      const char* argp = va_arg(ap, const char*);
      std::cout << argp << std::endl;
      va_end(ap);
    }
    
    void start() {
      std::cout << "Forking..." << std::endl;
      process_id = fork();

      if (process_id < 0) {
        throw process_error("Could not spawn process");
      }
      
      if (process_id > 0) {
        return;
      }
      
      int i = 1;
      char** argv = new char*[args.size() + 2];
      argv[args.size()] = NULL;

      memcpy(argv[0], command.c_str(), command.size());
      
      BOOST_FOREACH(std::string arg, args) {
        argv[i] = new char[arg.size()];
        memcpy(argv[i], arg.c_str(), arg.size());
        i++;
      }
      
      std::cout << command << std::endl;
      int r = execvp(command.c_str(), argv);
      std::cerr << "Something went wrong " << r << std::endl;
      exit(1);
    }
    
    int wait() {
      int status;
      waitpid(process_id, &status, 0);
      return status;
    }
};

int main() {
  forkexec cat("cat", "test.txt");
  cat.start();
  cat.wait();
  int w;
  std::cin >> w;
  return w;
}
