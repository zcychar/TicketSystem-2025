#include "management/management.h"

namespace sjtu {
  Management::Management(std::string name) {
    user_ = new User(name);
    ticket_ = new Ticket(name, user_);
    train_ = new Train(name, ticket_);
  }

  Management::~Management() {
    delete train_;
    delete ticket_;
    delete user_;
  }

  bool Management::ProcessLine(vector<std::string> &line) {
    auto size = line.size();
    if (size == 0) {
      return true;;
    }
    std::cout << line[0] << ' ';
    auto cmd = line[1];
    std::string c, u, p, n, m, g, i, s, x, t, o, d, y, q, f;
    for (int cnt = 2; cnt < size; ++cnt) {
      if (line[cnt][0] == '-' && line[cnt].size() == 2) {
        switch (line[cnt][1]) {
          case 'c': {
            c = line[cnt + 1];
            break;
          }
          case 'u': {
            u = line[cnt + 1];
            break;
          }
          case 'p': {
            p = line[cnt + 1];
            break;
          }
          case 'n': {
            n = line[cnt + 1];
            break;
          }
          case 'm': {
            m = line[cnt + 1];
            break;
          }
          case 'g': {
            g = line[cnt + 1];
            break;
          }
          case 'i': {
            i = line[cnt + 1];
            break;
          }
          case 's': {
            s = line[cnt + 1];
            break;
          }
          case 'x': {
            x = line[cnt + 1];
            break;
          }
          case 't': {
            t = line[cnt + 1];
            break;
          }
          case 'o': {
            o = line[cnt + 1];
            break;
          }
          case 'd': {
            d = line[cnt + 1];
            break;
          }
          case 'y': {
            y = line[cnt + 1];
            break;
          }
          case 'q': {
            q = line[cnt + 1];
            break;
          }
          case 'f': {
            f = line[cnt + 1];
            break;
          }
          default: {
            break;
          }
        }
        cnt++;
      }
    }
    if (cmd == "add_user") {
      UserInfo user_info(u, p, n, m, g);
      user_->AddUser(c, user_info);
    } else if (cmd == "login") {
      user_->Login(u, p);
    } else if (cmd == "logout") {
      user_->Logout(u);
    } else if (cmd == "query_profile") {
      user_->QueryProfile(c, u);
    } else if (cmd == "modify_profile") {
      UserInfoOptional user_info(u, p, n, m, g);
      user_->ModifyProfile(c, user_info);
    } else if (cmd == "exit") {
      std::cout << "bye";
      return false;
    }
    return true;
  }
}
