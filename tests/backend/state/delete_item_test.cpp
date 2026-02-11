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

long insert_seed_item(pqxx::connection &conn) {
  InsertInput in{
      std::optional<std::string>{"D-001"},
      "seasonal",
      "decor",
      "wreath and pine cone ornaments",
  };
  Item item = state::insert_item(conn, in);
  return item.id;
}

void test_delete_item_returns_true_for_existing_id() {
  pqxx::connection conn{""};
  reset_stash(conn);

  const long id = insert_seed_item(conn);
  const bool deleted = state::delete_item(conn, id);

  assert_true(deleted, "expected delete_item to return true for existing id");
}

void test_delete_item_returns_false_for_missing_id() {
  pqxx::connection conn{""};
  reset_stash(conn);

  const bool deleted = state::delete_item(conn, 9999);

  assert_true(!deleted, "expected delete_item to return false for missing id");
}

void test_delete_item_is_idempotent() {
  pqxx::connection conn{""};
  reset_stash(conn);

  const long id = insert_seed_item(conn);

  const bool first_delete = state::delete_item(conn, id);
  const bool second_delete = state::delete_item(conn, id);

  assert_true(first_delete, "expected first delete_item call to return true");
  assert_true(!second_delete, "expected second delete_item call to return false");
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

  run_test("delete_item_returns_true_for_existing_id",
           test_delete_item_returns_true_for_existing_id);
  run_test("delete_item_returns_false_for_missing_id",
           test_delete_item_returns_false_for_missing_id);
  run_test("delete_item_is_idempotent", test_delete_item_is_idempotent);

  return failures == 0 ? 0 : 1;
}
