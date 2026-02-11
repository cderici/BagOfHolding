#include "../../../backend/state/state.h"
#include <algorithm>
#include <exception>
#include <iostream>
#include <optional>
#include <pqxx/pqxx>
#include <stdexcept>
#include <string>
#include <vector>

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

state::Item insert_seed_item(pqxx::connection &conn,
                             std::optional<std::string> physical_label,
                             const std::string &category,
                             const std::string &sub_category,
                             const std::string &contents) {
  state::InsertInput in{physical_label, category, sub_category, contents};
  return state::insert_item(conn, in);
}

void test_search_items_returns_hits_for_term_in_searchable_fields() {
  pqxx::connection conn{""};
  reset_stash(conn);

  insert_seed_item(conn, std::optional<std::string>{"X-101"}, "misc", "other",
                   "lantern");
  const state::Item target =
      insert_seed_item(conn, std::optional<std::string>{"X-202"}, "x", "y", "santa");

  const std::vector<state::SearchHit> results =
      state::search_items(conn, "santa", 20);

  assert_true(!results.empty(), "expected at least one search hit for term santa");

  const auto it =
      std::find_if(results.begin(), results.end(), [&](const state::SearchHit &hit) {
        return hit.id == target.id;
      });

  assert_true(it != results.end(), "expected inserted santa item to appear in results");
}

void test_search_items_respects_limit() {
  pqxx::connection conn{""};
  reset_stash(conn);

  insert_seed_item(conn, std::optional<std::string>{"L-001"}, "x", "y", "santa");
  insert_seed_item(conn, std::optional<std::string>{"L-002"}, "x", "y", "santa box");
  insert_seed_item(conn, std::optional<std::string>{"L-003"}, "x", "y", "santa hat");

  const std::vector<state::SearchHit> results =
      state::search_items(conn, "santa", 2);

  assert_true(results.size() == 2, "expected search_items to respect limit=2");
}

void test_search_items_boosts_contents_prefix_match() {
  pqxx::connection conn{""};
  reset_stash(conn);

  const state::Item prefix_hit =
      insert_seed_item(conn, std::optional<std::string>{"S-001"}, "x", "y", "santa");

  insert_seed_item(conn, std::optional<std::string>{"S-002"}, "x", "y", "my santa");

  const std::vector<state::SearchHit> results =
      state::search_items(conn, "santa", 10);

  assert_true(results.size() >= 2,
              "expected at least two results to verify ranking behavior");
  assert_true(results.front().id == prefix_hit.id,
              "expected contents prefix match to be ranked first");
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

  run_test("search_items_returns_hits_for_term_in_searchable_fields",
           test_search_items_returns_hits_for_term_in_searchable_fields);
  run_test("search_items_respects_limit", test_search_items_respects_limit);
  run_test("search_items_boosts_contents_prefix_match",
           test_search_items_boosts_contents_prefix_match);

  return failures == 0 ? 0 : 1;
}
