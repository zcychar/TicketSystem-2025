#include "storage/b_plus_tree.h"
#include "management/management.h"
#include "common/util.h"

int main() {
  // freopen("../testcases/36.in","r",stdin);
  // std::ios::sync_with_stdio(false);
  // std::cin.tie(0);
  // std::cout.tie(0);
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