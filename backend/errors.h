#pragma once

#include <stdexcept>
#include <string>

class DuplicatePhysicalLabel : public std::runtime_error {
public:
  explicit DuplicatePhysicalLabel(const std::string &label)
      : std::runtime_error("physical_label already exists: " + label) {}
};
