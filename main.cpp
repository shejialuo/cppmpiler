#include "repl.hpp"

#include <cstring>
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2 || (strcmp(argv[1], "i") != 0 && strcmp(argv[1], "c") != 0)) {
    std::cout << "usage: ./cppmpiler [i|c] (i: interpreter mode, c : compiler mode)\n";
    return 0;
  }

  std::cout << "Hello! This is the Monkey programming language!\n";
  std::cout << "Feel free to type in commands\n";

  if (strcmp(argv[1], "i") == 0) {
    startInterpreter();
  } else {
    startCompiler();
  }
}
