#pragma once

#include "management/ticket.h"
#include "management/train.h"
#include "management/user.h"

namespace sjtu {
class Management {
 public:
  explicit Management(std::string name);

  ~Management();

  auto ProcessLine(vector<std::string> &line) -> bool;

  void Clean(std::string name);

 private:
  User *user_;
  Ticket *ticket_;
  Train *train_;
};
}  // namespace sjtu