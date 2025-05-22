#pragma once

#include <cstring>
#include "common/config.h"
#include "common/vector.h"
#include <iostream>


namespace sjtu {
inline auto ToHash(std::string &str) -> hash_t {
  auto size = str.size();
  auto hash = size;
  for (auto it : str) {
    hash = (hash << 5) ^ (hash >> 27) ^ it;
  }
  return hash;
}

inline auto ToHash(const char str[]) -> hash_t {
  auto size = strlen(str);
  auto hash = size;
  for (int i = 0; i < size; ++i) {
    hash = (hash << 5) ^ (hash >> 27) ^ str[i];
  }
  return hash;
}

inline void ParseCommand(std::string &command,
                         sjtu::vector<std::string> *parsed_command) {
  std::istringstream iss(command);
  std::string temp;
  while (!iss.eof()) {
    iss >> temp;
    parsed_command->push_back(temp);
  }
}

struct HashComp {
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

template <typename T>
struct PairCompare {
  int operator()(const T &lhs, const T &rhs) {
    if (lhs.first < rhs.first) {
      return -1;
    }
    if (lhs.first > rhs.first) {
      return 1;
    }
    if (lhs.second < rhs.second) {
      return -1;
    }
    if (lhs.second > rhs.second) {
      return 1;
    }
    return 0;
  }
};

template <typename T>
struct PairDegradedCompare {
  int operator()(const T &lhs, const T &rhs) {
    if (lhs.first < rhs.first) {
      return -1;
    }
    if (lhs.first > rhs.first) {
      return 1;
    }
    return 0;
  }
};

inline std::string ToDate(num_t date) {
  const char *month = (date <= 30) ? "06" : (date <= 61) ? "07" : (date <= 92) ? "08" : "09";
  int day = (date <= 30) ? date : (date <= 61) ? date - 30 : (date <= 92) ? date - 61 : date - 92;
  char buffer[6];
  snprintf(buffer, sizeof(buffer), "%s-%02d", month, day);
  return buffer;
}

inline num_t DateToNum(const std::string &date) {
  int month = std::stoi(date.substr(0, 2));
  int day = std::stoi(date.substr(3));
  if(month==6) {
    return day;
  }else if(month==7) {
    return day+30;
  }else if(month==8) {
    return day+61;
  }
  return -1;
}

inline std::string ToTime(num_t time) {
  int hour = time / 60;
  int minute = time % 60;
  char buffer[6];
  snprintf(buffer, sizeof(buffer), "%02d:%02d", hour, minute);
  return buffer;
}

inline num_t TimeToNum(const std::string &time) {
  return std::stoi(time.substr(0, 2)) * 60 + std::stoi(time.substr(3, 2));
}

inline num_t TimeCost(num_t date_1, num_t time_1, num_t date_2, num_t time_2) {
  return time_2 - time_1 + (date_2 - date_1) * 1440;
}

struct DateTime {
  num_t date = 0;
  num_t time = 0;

  DateTime(num_t d, num_t t): date(d), time(t) {
    while (time >= 1440) {
      time -= 1440;
      date += 1;
    }
  };

  friend std::ostream &operator<<(std::ostream &os, DateTime &date_time) {
    std::cout << ToDate(date_time.date) << ' ' << ToTime(date_time.time);
    return os;
  }

  void operator+=(num_t t) {
    time += t;
    while (time >= 1440) {
      time -= 1440;
      date += 1;
    }
  }

  void operator+=(DateTime &dt) {
    time += dt.time;
    date += dt.date;
    while (time >= 1440) {
      time -= 1440;
      date += 1;
    }
  }

  num_t operator-(DateTime &rhs) {
    return (date - rhs.date) * 1440 + (time - rhs.time);
  }

  bool operator<(const DateTime &rhs) const {
    if (date != rhs.date) {
      return date < rhs.date;
    }
    return time < rhs.time;
  }

  bool operator>=(const DateTime &rhs) const {
    return !operator<(rhs);
  }

  bool operator>(const DateTime &rhs) const {
    if (date != rhs.date) {
      return date > rhs.date;
    }
    return time > rhs.time;
  }

  bool operator<=(const DateTime &rhs) const {
    return !operator>(rhs);
  }
};

inline void InsertStations(char station[][31], std::string &s) {
  std::istringstream iss(s);
  std::string str;
  int cnt = 0;
  while (getline(iss, str, '|')) {
    strncpy(station[cnt++], str.c_str(), 30);
  }
}

inline void InsertNum(num_t time[], std::string &t) {
  if(t[0]=='_') {
    return;
  }
  std::istringstream iss(t);
  std::string str;
  int cnt = 0;
  while (getline(iss, str, '|')) {
    time[cnt++] = static_cast<num_t>(std::stoi(str));
  }
}


inline auto ToTimeStamp(std::string &t) -> int {
  return std::stoi(t.substr(1,t.size()-2));
}
} // namespace sjtu