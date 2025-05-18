#pragma once

#include "storage/b_plus_tree.h"
#include "common/util.h"

namespace sjtu {
  struct UserInfo {
    hash_t username_hash = 0;
    char username[21] = {};
    char password[31] = {};
    char name[21] = {};
    char mailaddr[21] = {};
    num_t privilege = 0;
  }; //104 bytes

  struct UserInfoOptional {
    hash_t username_hash = 0;
    char username[21] = {};
    std::optional<std::string> password;
    std::optional<std::string> name;
    std::optional<std::string> mailaddr;
    std::optional<int> privilege;
  }; //Only used in User::ModifyProfile

  struct UserInfoComp {
    int operator()(const hash_t lhs, const hash_t rhs) const {
      if (lhs != rhs) {
        if (lhs < rhs) {
          return -1;
        }
        return 1;
      }
      return 0;
    }
  };

  class User {
  public:
    explicit User(std::string& name);

    ~User();

    auto AddUser(std::string &cur_username, UserInfo &user) -> bool;

    auto Login(std::string &username, std::string &password) -> bool;

    auto Logout(std::string &name) -> bool;

    auto QueryProfile(std::string &cur_username, std::string &username) -> std::optional<UserInfo>;

    auto ModifyProfile(std::string &cur_username, UserInfoOptional &user) -> std::optional<UserInfo>;

    auto IsLogged(std::string &username) const -> bool;

  private:
    BPlusTree<hash_t, UserInfo, UserInfoComp, UserInfoComp> * user_db_;

    sjtu::map<hash_t, UserInfo> logged_user_;
  };
}
