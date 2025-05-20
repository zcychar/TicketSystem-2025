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

  void Management::ProcessLine(vector<std::string> &line) {
    auto size = line.size();
    if (size == 0) {
      return;
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
            m=line[cnt+1];
            break;
          }
          case 'g': {
            g=line[cnt+1];
            break;
          }
          case 'i': {
            i=line[cnt+1];
            break;
          }
          case 's': {
            s=line[cnt+1];
            break;
          }
          case 'x': {
            x=line[cnt+1];
            break;
          }
          case 't': {
            t=line[cnt+1];
            break;
          }
          case 'o': {
            o=line[cnt+1];
            break;
          }
          case 'd': {
            d=line[cnt+1];
            break;
          }
          case 'y': {
            y=line[cnt+1];
            break;
          }
          case 'q': {
            q=line[cnt+1];
            break;
          }
          case 'f': {
            f=line[cnt+1];
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
      UserInfo user_info{ToHash(u),{},{},{},{},{}};
      std::strcpy(user_info.username,u.c_str());
      std::strcpy(user_info.password,u.c_str());
      std::strcpy(user_info.name,u.c_str());
      std::strcpy(user_info.mailaddr,u.c_str());
      user_info.privilege=static_cast<num_t>(std::stoi(p));
      user_->AddUser(c,user_info);
    } else if (cmd == "login") {

    } else if (cmd == "logout") {

    } else if (cmd == "query_profile") {

    } else if (cmd == "modify_profile") {

    } else {
      return;
    }
  }
}
