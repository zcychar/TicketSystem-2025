#pragma once

#include <iostream>
#include <string>

namespace sjtu {
  constexpr int MOD_1 = 1e9 + 7;
  constexpr int BASE_1 = 19260817;

  constexpr int MOD_2 = 1e9 + 9;
  constexpr int BASE_2 = 12255871;

  struct Key {
    int hashed_key_1 = 0;
    int hashed_key_2 = 0;
    char key_[65];
    int value;


    Key(const std::string &key, const int value = -2147483648): value(value) {
      hashed_key_1 = 0;
      hashed_key_2 = 0;
      auto size = key.size();
      for (int i = 0; i < size; ++i) {
        hashed_key_1 = (1ll * hashed_key_1 * BASE_1 + key[i]) % MOD_1;
        hashed_key_2 = (1ll * hashed_key_2 * BASE_2 + key[i]) % MOD_2;
      }
    }

    friend std::ostream &operator<<(std::ostream &os, const Key &x) {
      os << x.hashed_key_1;
      return os;
    }
  };

  struct Comparator {
    int operator ()(const Key &lhs, const Key &rhs) {
      if (lhs.hashed_key_1 > rhs.hashed_key_1) {
        return 1;
      }
      if (lhs.hashed_key_1 < rhs.hashed_key_1) {
        return -1;
      }
      if (lhs.hashed_key_2 > rhs.hashed_key_2) {
        return 1;
      }
      if (lhs.hashed_key_2 < rhs.hashed_key_2) {
        return -1;
      }
      if (lhs.value < rhs.value) {
        return -1;
      }
      if (lhs.value > rhs.value) {
        return 1;
      }
      return 0;
    }
  };

  struct DegradedComparator {
    int operator ()(const Key &lhs, const Key &rhs) {
      if (lhs.hashed_key_1 > rhs.hashed_key_1) {
        return 1;
      }
      if (lhs.hashed_key_1 < rhs.hashed_key_1) {
        return -1;
      }
      if (lhs.hashed_key_2 > rhs.hashed_key_2) {
        return 1;
      }
      if (lhs.hashed_key_2 < rhs.hashed_key_2) {
        return -1;
      }

      return 0;
    }
  };
} // namespace sjtu
