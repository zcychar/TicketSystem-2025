#pragma once

#include "storage/b_plus_tree.h"
#include "common/util.h"

namespace sjtu {
  class user {
    struct UserInfo {
      int username_hash;
      char username[21];
      char password[31];
      char name[21];
      char mailaddr[21];
      int priviledge;
    };

    struct UserInfoComp {
      int operator()(UserInfo& lhs,UserInfo& rhs) {
        if(lhs.username_hash!=rhs.username_hash) {
          if(lhs.username_hash<rhs.username_hash) {
            return -1;
          }
          return 1;
        }
        return 0;
      }
    };




  private:
    BPlusTree<int,UserInfo,UserInfoComp,IGNORED_COMPARATOR> user_db_;
  };
}
