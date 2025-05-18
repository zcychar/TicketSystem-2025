#pragma once

#include "common/config.h"

namespace sjtu {
  inline auto ToHash(std::string &str) -> hash_t {
    auto size = str.size();
    auto hash = size;
    for (auto it: str) {
      hash = (hash << 5) ^ (hash >> 27) ^ it;
    }
    return hash;
  }

  inline void ParseCommand(std::string &command, sjtu::vector<std::string> *parsed_command) {
    std::istringstream iss(command);
    std::string temp;
    while (!iss.eof()) {
      iss>>temp;
      parsed_command->push_back(temp);
    }
  }


}
