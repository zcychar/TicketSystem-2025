#include "storage/b_plus_tree.h"
#include "management/management.h"
#include "common/util.h"

int main() {
  // freopen("../testcases/5.in","r",stdin);
  sjtu::Management management("ticket_system");
  std::string command;
  sjtu::vector<std::string> parsed_command;
  while (std::getline(std::cin, command)) {
    if (command.empty()) {
      continue;
    }
    parsed_command.clear();
    ParseCommand(command, &parsed_command);
    if (!management.ProcessLine(parsed_command)) {
      break;
    }
  }
  return 0;
}