#include "../../../backend/state/state.h"
#include <exception>
#include <iostream>
#include <optional>
#include <pqxx/pqxx>
#include <stdexcept>
#include <string>

namespace {

void reset_stash(pqxx::connection &conn) {
  pqxx::work tx{conn};
  tx.exec("truncate table stash restart identity;");
  tx.commit();
}

void assert_true(bool condition, const std::string &message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

void test_insert_item_with_label() {
  pqxx::connection conn{""};
  reset_stash(conn);

  state::InsertInput in{
      std::optional<std::string>{"A-001"},
      "decor",
      "holiday",
      "santa decorations and lights",
  };

  state::Item item = state::insert_item(conn, in);

  assert_true(item.id == 1, "expected inserted item id to be 1");
  assert_true(item.physical_label.has_value(), "expected physical_label to exist");
  assert_true(item.physical_label.value() == "A-001",
              "expected physical_label to match input");
  assert_true(item.category == "decor", "expected category to match input");
  assert_true(item.sub_category == "holiday",
              "expected sub_category to match input");
  assert_true(item.contents == "santa decorations and lights",
              "expected contents to match input");
  assert_true(item.created_at.time_since_epoch().count() > 0,
              "expected created_at to be initialized");
  assert_true(item.updated_at.time_since_epoch().count() > 0,
              "expected updated_at to be initialized");
}

void test_insert_item_with_null_label() {
  pqxx::connection conn{""};
  reset_stash(conn);

  state::InsertInput in{
      std::nullopt,
      "kitchen",
      "appliances",
      "small blender and mixer",
  };

  state::Item item = state::insert_item(conn, in);

  assert_true(item.id == 1, "expected inserted item id to be 1");
  assert_true(!item.physical_label.has_value(),
              "expected physical_label to be nullopt");
  assert_true(item.category == "kitchen", "expected category to match input");
  assert_true(item.sub_category == "appliances",
              "expected sub_category to match input");
  assert_true(item.contents == "small blender and mixer",
              "expected contents to match input");
}

} // namespace

int main() {
  int failures = 0;

  const auto run_test = [&](const char *name, void (*fn)()) {
    try {
      fn();
      std::cout << "- " << name << " : PASS\n";
    } catch (const std::exception &e) {
      ++failures;
      std::cout << "- " << name << " : FAIL (" << e.what() << ")\n";
    } catch (...) {
      ++failures;
      std::cout << "- " << name << " : FAIL (unknown exception)\n";
    }
  };

  run_test("insert_item_with_label", test_insert_item_with_label);
  run_test("insert_item_with_null_label", test_insert_item_with_null_label);

  return failures == 0 ? 0 : 1;
}
