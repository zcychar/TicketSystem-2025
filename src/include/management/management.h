#pragma once

#include "management/ticket.h"
#include "management/user.h"
#include "management/train.h"

namespace sjtu {
class Management {
public:
  explicit Management(std::string name);

  ~Management();

  auto ProcessLine(vector<std::string> &line) -> bool;

private:
  User *user_;
  Ticket *ticket_;
  Train *train_;
};
}