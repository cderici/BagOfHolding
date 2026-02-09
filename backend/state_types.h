#pragma once

#include <chrono>
#include <optional>
#include <string>

using Timestamp = std::chrono::sys_time<std::chrono::microseconds>;

// Item is anything that's in the storage room
struct Item {
  long id;
  std::optional<std::string> physical_label; // can be null
  std::string category;
  std::string sub_category;
  std::string contents; // free text
  Timestamp created_at;
  Timestamp updated_at;
};

struct InsertInput {
  std::optional<std::string> physical_label;
  std::string category;
  std::string sub_category;
  std::string contents; // free text
};
