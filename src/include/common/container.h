#pragma once

#include <iostream>
#include <string>

namespace sjtu {
  constexpr int MOD = 1e9 + 7;
  constexpr int BASE = 19260817;

  struct Key {
    int hashed_key = 0;
    int value;

    Key(const std::string &key, const int value = -2147483648): value(value) {
      hashed_key = 0;
      auto size = key.size();
      for (int i = 0; i < size; ++i) {
        hashed_key = (1ll * hashed_key * BASE + key[i]) % MOD;
      }
    }

    friend std::ostream &operator<<(std::ostream &os, const Key &x) {
      os << x.hashed_key;
      return os;
    }
  };

  struct Comparator {
    int operator ()(const Key &lhs, const Key &rhs) {
      if (lhs.hashed_key > rhs.hashed_key) {
        return 1;
      }
      if (lhs.hashed_key < rhs.hashed_key) {
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
      if (lhs.hashed_key > rhs.hashed_key) {
        return 1;
      }
      if (lhs.hashed_key < rhs.hashed_key) {
        return -1;
      }
      return 0;
    }
  };
} // namespace sjtu
