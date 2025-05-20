#pragma once

#include "management/ticket.h"
#include "management/user.h"
#include "management/train.h"

namespace sjtu {
  class Management {
  public:
    explicit Management(std::string name);

    ~Management();

    void ProcessLine(vector<std::string>& line);

  private:
    User *user_;
    Ticket *ticket_;
    Train *train_;
  };
}
