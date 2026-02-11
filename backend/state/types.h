#pragma once

#include <chrono>
#include <optional>
#include <string>

using Timestamp = std::chrono::sys_time<std::chrono::microseconds>;

struct Item {
  long id;
  std::optional<std::string> physical_label;
  std::string category;
  std::string sub_category;
  std::string contents;
  Timestamp created_at;
  Timestamp updated_at;
};

struct InsertInput {
  std::optional<std::string> physical_label;
  std::string category;
  std::string sub_category;
  std::string contents;
};

struct SearchHit {
  long id;
  std::optional<std::string> physical_label;
  std::string category;
  std::string sub_category;
  std::string contents;
};
