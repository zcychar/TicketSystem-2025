#pragma once

#include <string.h>

#include "common/util.h"
#include "storage/b_plus_tree.h"

namespace sjtu {
class Ticket;

struct UserInfo {
  hash_t username_hash = 0;
  char username[21] = {};
  char password[31] = {};
  char name[16] = {};
  char mailaddr[31] = {};
  num_t privilege = 0;

  UserInfo(std::string &u, std::string &p, std::string &n, std::string &m,
           std::string &g) {
    username_hash = ToHash(u);
    strncpy(username, u.c_str(), 20);
    strncpy(password, p.c_str(), 30);
    strncpy(name, n.c_str(), 15);
    strncpy(mailaddr, m.c_str(), 30);
    privilege = static_cast<num_t>(std::stoi(g));
  }
};  // 100 bytes

struct UserInfoOptional {
  hash_t username_hash = 0;
  char username[21] = {};
  std::optional<std::string> password = {};
  std::optional<std::string> name = {};
  std::optional<std::string> mailaddr = {};
  std::optional<int> privilege = {};

  UserInfoOptional(std::string &u, std::string &p, std::string &n,
                   std::string &m, std::string &g) {
    username_hash = ToHash(u);
    strcpy(username, u.c_str());
    if (!p.empty()) {
      password = p;
    }
    if (!n.empty()) {
      name = n;
    }
    if (!m.empty()) {
      mailaddr = m;
    }
    if (!g.empty()) {
      privilege = static_cast<num_t>(std::stoi(g));
    }
  }
};  // Only used in User::ModifyProfile

class User {
  friend Ticket;

 public:
  explicit User(std::string &name);

  void AddUser(std::string &cur_username, UserInfo &user);

  void Login(std::string &username, std::string &password);

  void Logout(std::string &name);

  void QueryProfile(std::string &cur_username, std::string &username);

  void ModifyProfile(std::string &cur_username, UserInfoOptional &user);

  auto IsLogged(std::string &username) const -> bool;

 private:
  std::unique_ptr<BPlusTree<hash_t, UserInfo, HashComp, HashComp> > user_db_;

  sjtu::map<hash_t, UserInfo> logged_user_;
};
}  // namespace sjtu