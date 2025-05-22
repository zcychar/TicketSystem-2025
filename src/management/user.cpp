#include "management/user.h"

#include <cstring>

namespace sjtu {
User::User(std::string &name) {
  HashComp comp;
  user_db_ = std::make_unique<BPlusTree<hash_t, UserInfo, HashComp, HashComp> >(
      name + "_db", comp, comp,512);
}

void User::AddUser(std::string &cur_username, UserInfo &user) {
  if (user_db_->IsEmpty()) {
    user.privilege = 10;
    if (user_db_->Insert(user.username_hash, user)) {
      std::cout << "0\n";
      return;
    } else {
      std::cout << "-1\n";
      return;
    }
  }
  hash_t cur_user_hash = ToHash(cur_username);
  auto cur_user = logged_user_.find(cur_user_hash);
  if (cur_user == logged_user_.end()) {
    std::cout << "-1\n";
    return;
  }
  if (user.privilege >= cur_user->second.privilege) {
    std::cout << "-1\n";
    return;
  }
  if (user_db_->Insert(user.username_hash, user)) {
    std::cout << "0\n";
  } else {
    std::cout << "-1\n";
  }
}

void User::Login(std::string &username, std::string &password) {
  auto user_hash = ToHash(username);
  sjtu::vector<UserInfo> user_vector;
  user_db_->GetValue(user_hash, &user_vector);
  if (user_vector.empty()) {
    std::cout << "-1\n";
    return;
  }
  if (strcmp(password.c_str(), user_vector[0].password) != 0) {
    std::cout << "-1\n";
    return;
  }

  auto status=logged_user_.insert(user_hash, user_vector[0]);
  if(status.second==false) {
    std::cout<<"-1\n";
    return;
  }
  std::cout << "0\n";
}

void User::Logout(std::string &name) {
  auto user_hash = ToHash(name);
  if (logged_user_.count(user_hash) == 0) {
    std::cout << "-1\n";
    return;
  }
  logged_user_.erase(user_hash);
  std::cout << "0\n";
}

auto User::IsLogged(std::string &username) const -> bool {
  return logged_user_.count(ToHash(username)) == 1;
}

void User::QueryProfile(std::string &cur_username, std::string &username) {
  auto cur_user = logged_user_.find(ToHash(cur_username));
  if (cur_user == logged_user_.end()) {
    std::cout << "-1\n";
    return;
  }
  sjtu::vector<UserInfo> user_vector;
  user_db_->GetValue(ToHash(username), &user_vector);
  if (user_vector.empty()) {
    std::cout << "-1\n";
    return;
  }
  if (user_vector[0].username_hash == cur_user->second.username_hash) {
    std::cout << user_vector[0].username << ' ' << user_vector[0].name << ' '
              << user_vector[0].mailaddr << ' ' << user_vector[0].privilege
              << '\n';
    return;
  }
  if (cur_user->second.privilege <= user_vector[0].privilege) {
    std::cout << "-1\n";
    return;
  }
  std::cout << user_vector[0].username << ' ' << user_vector[0].name << ' '
            << user_vector[0].mailaddr << ' ' << user_vector[0].privilege
            << '\n';
}

void User::ModifyProfile(std::string &cur_username, UserInfoOptional &user) {
  auto cur_user_it = logged_user_.find(ToHash(cur_username));

  if (cur_user_it == logged_user_.end()) {
    std::cout << "-1\n";
    return;
  }
  auto cur_user = cur_user_it->second;
  if (user.privilege.has_value() && user.privilege >= cur_user.privilege) {
    std::cout << "-1\n";
    return;
  }

  sjtu::vector<UserInfo> user_vector;
  user_db_->GetValue(user.username_hash, &user_vector);
  if (user_vector.empty()) {
    std::cout << "-1\n";
    return;
  }

  if (user_vector[0].privilege >= cur_user.privilege &&
      cur_user.username_hash != user_vector[0].username_hash) {
    std::cout << "-1\n";
    return;
  }

  if (user.mailaddr.has_value()) {
    strncpy(user_vector[0].mailaddr, user.mailaddr.value().c_str(), 30);
  }
  if (user.name.has_value()) {
    strncpy(user_vector[0].name, user.name.value().c_str(), 15);
  }
  if (user.password.has_value()) {
    strncpy(user_vector[0].password, user.password.value().c_str(), 30);
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
  std::cout << user_vector[0].username << ' ' << user_vector[0].name << ' '
            << user_vector[0].mailaddr << ' ' << user_vector[0].privilege
            << '\n';
}
}  // namespace sjtu