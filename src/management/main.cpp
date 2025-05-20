#include "storage/b_plus_tree.h"
#include "management/management.h"
#include "common/util.h"

int main() {
  sjtu::Management management("ticket_system");
  std::string command;
  sjtu::vector<std::string>parsed_command;
  while(std::getline(std::cin,command)) {
    parsed_command.clear();
    ParseCommand(command,&parsed_command);
    management.ProcessLine(parsed_command);
  }
  return 0;
}