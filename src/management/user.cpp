#include "management/user.h"

#include <cstring>

namespace sjtu {
  User::User(std::string &name) {
    HashComp comp;
    user_db_ = std::make_unique<BPlusTree<hash_t, UserInfo, HashComp, HashComp>> (
      name+"_db", comp, comp);
  }

  auto User::AddUser(std::string &cur_username, UserInfo &user) -> bool {
    if (user_db_->IsEmpty()) {
      user.privilege = 10;
      return user_db_->Insert(user.username_hash, user);
    }
    hash_t cur_user_hash = ToHash(cur_username);
    auto cur_user = logged_user_.find(cur_user_hash);
    if (cur_user == logged_user_.end()) {
      return false;
    }
    if (user.privilege >= cur_user->second.privilege) {
      return false;
    }
    return user_db_->Insert(user.username_hash, user);
  }

  auto User::Login(std::string &username, std::string &password) -> bool {
    auto user_hash = ToHash(username);
    if (logged_user_.count(user_hash) != 0) {
      return false;
    }
    sjtu::vector<UserInfo> user_vector;
    user_db_->GetValue(user_hash, &user_vector);
    if (user_vector.empty()) {
      return false;
    }
    if (strcmp(password.c_str(), user_vector[0].password) != 0) {
      return false;
    }
    logged_user_.insert(user_hash, user_vector[0]);
    return true;
  }

  auto User::Logout(std::string &name) -> bool {
    auto user_hash = ToHash(name);
    if (logged_user_.count(user_hash) == 0) {
      return false;
    }
    logged_user_.erase(user_hash);
    return true;
  }

  auto User::IsLogged(std::string &username) const -> bool {
    return logged_user_.count(ToHash(username)) == 1;
  }

  auto User::QueryProfile(std::string &cur_username, std::string &username) -> std::optional<UserInfo> {
    auto cur_user = logged_user_.find(ToHash(cur_username));
    if (cur_user == logged_user_.end()) {
      std::optional<UserInfo> temp(std::nullopt);
      return temp;
    }
    sjtu::vector<UserInfo> user_vector;
    user_db_->GetValue(ToHash(username), &user_vector);
    if (user_vector.empty()) {
      std::optional<UserInfo> temp(std::nullopt);
      return temp;
    }
    if (cur_user->second.privilege < user_vector[0].privilege) {
      std::optional<UserInfo> temp(std::nullopt);
      return temp;
    }
    std::optional<UserInfo> temp(user_vector[0]);
    return temp;
  }

  auto User::ModifyProfile(std::string &cur_username, UserInfoOptional &user) -> std::optional<UserInfo> {
    auto cur_user = logged_user_.find(ToHash(cur_username));
    if (cur_user == logged_user_.end()) {
      std::optional<UserInfo> temp(std::nullopt);
      return temp;
    }
    if (user.privilege.has_value() && user.privilege >= cur_user->second.privilege) {
      std::optional<UserInfo> temp(std::nullopt);
      return temp;
    }

    sjtu::vector<UserInfo> user_vector;
    user_db_->GetValue(user.username_hash, &user_vector);
    if (user_vector.empty()) {
      std::optional<UserInfo> temp(std::nullopt);
      return temp;
    }
    if (user_vector[0].privilege > cur_user->second.privilege) {
      std::optional<UserInfo> temp(std::nullopt);
      return temp;
    }

    if (user.mailaddr.has_value()) {
      strcpy(user_vector[0].mailaddr, user.mailaddr.value().c_str());
    }
    if (user.name.has_value()) {
      strcpy(user_vector[0].name, user.name.value().c_str());
    }
    if (user.password.has_value()) {
      strcpy(user_vector[0].password, user.password.value().c_str());
    }
    if (user.privilege.has_value()) {
      user_vector[0].privilege = user.privilege.value();
    }
    user_db_->Remove(user.username_hash);
    user_db_->Insert(user.username_hash, user_vector[0]);

    if (logged_user_.count(user.username_hash) != 0) {
      logged_user_.erase(user.username_hash);
      logged_user_.insert(user.username_hash, user_vector[0]);
    }
    std::optional<UserInfo> temp(user_vector[0]);
    return temp;
  }
}
