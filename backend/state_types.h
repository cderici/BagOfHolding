#pragma once

#include <chrono>
#include <optional>
#include <string>

using Timestamp = std::chrono::sys_time<std::chrono::microseconds>;

// Item is anything that's in the storage room
struct Item {
  long id;                                   // internal id used in the backend
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

// SearchHit represents a lightweight result to be used in fuzzy search as the
// user types. state::search_items returns std::vector<SearchHit>
struct SearchHit {
  long id;
  std::optional<std::string> physical_label; // can be null
  std::string category;
  std::string sub_category;
  std::string contents; // free text
};
