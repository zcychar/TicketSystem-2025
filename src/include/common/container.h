#pragma once

#include <iostream>
#include <string>

namespace sjtu {
  struct Key {
    char key_[64];
    int value;

    Key(const std::string &key, const int value = -1): value(value) {
      auto size = key.size();
      for (int i = 0; i < size; ++i) {
        key_[i] = key[i];
      }
      for (int i = size; i < 64; ++i) {
        key_[i] = '\0';
      }
    }
  };

  struct Comparator {
    int operator ()(const Key &lhs, const Key &rhs) {
      for (int i = 0; i < 64; ++i) {
        if (lhs.key_[i] != rhs.key_[i]) {
          if (lhs.key_[i] < rhs.key_[i]) {
            return -1;
          }
          return 1;
        }
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
      for (int i = 0; i < 64; ++i) {
        if (lhs.key_[i] != rhs.key_[i]) {
          if (lhs.key_[i] < rhs.key_[i]) {
            return -1;
          }
          return 1;
        }
      }
      return 0;
    }
  };
} // namespace sjtu
