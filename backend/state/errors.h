#pragma once

#include <exception>
#include <stdexcept>
#include <string>

namespace state {

class DuplicatePhysicalLabel : public std::runtime_error {
public:
  explicit DuplicatePhysicalLabel(const std::string &label)
      : std::runtime_error("physical_label already exists: " + label) {}
};

class DBException : public std::runtime_error, public std::nested_exception {
public:
  explicit DBException(const std::string msg)
      : std::runtime_error(std::move(msg)) {}
};

} // namespace state
