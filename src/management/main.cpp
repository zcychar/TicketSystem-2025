#include "storage/b_plus_tree.h"
#include "management/user.h"

#include "common/util.h"

int main() {
  std::string command;
  sjtu::vector<std::string>parsed_command;
  while(std::getline(std::cin,command)) {
    parsed_command.clear();
    ParseCommand(command,&parsed_command);
  }
  return 0;
}