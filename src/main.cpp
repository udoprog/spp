#include "subprocess.hpp"

#include <iostream>

int main() {
  subprocess cat("echo");
  cat << "Fudge" << "This" << "Shit" << " " << "Hello" << 1231L << 12.12f;
  cat.start();
  
  std::cout << "Waiting for " << cat.get_pid() << std::endl;
  
  char buffer[1024];
  
  size_t r = read(cat.get_stdout(), buffer, 1024);
  buffer[r] = NULL;
  std::cout << buffer << std::endl;
  
  cat.flush();
  return cat.wait();
}
