#pragma once

#include "storage/b_plus_tree.h"
#include "common/util.h"

namespace sjtu {
  class Ticket;

  struct UserInfo {
    hash_t username_hash = 0;
    char username[20] = {};
    char password[30] = {};
    char name[20] = {};
    char mailaddr[20] = {};
    num_t privilege = 0;
  }; //100 bytes

  struct UserInfoOptional {
    hash_t username_hash = 0;
    char username[20] = {};
    std::optional<std::string> password;
    std::optional<std::string> name;
    std::optional<std::string> mailaddr;
    std::optional<int> privilege;
  }; //Only used in User::ModifyProfile


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
}
