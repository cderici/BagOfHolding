#include "errors.h"
#include "state_types.h"
#include <chrono>
#include <exception>
#include <iostream>
#include <optional>
#include <pqxx/pqxx>

// delete_item
// returns true --> item deleted as expected
// return false --> item was not there anyways
// may throw exception if postgres throws
bool delete_item(pqxx::connection &conn, long itemID) {
  try {
    pqxx::work tx{conn};

    pqxx::result r = tx.exec_params("delete from stash where id=$1", itemID);

    tx.commit();

    return r.affected_rows() != 0;

  } catch (const pqxx::sql_error &e) {
    // try catching the SQL-related problems
    std::throw_with_nested(DBException("delete_item failed. sqlstate=" +
                                       e.sqlstate() + ", query=" + e.query()));
  } catch (const pqxx::failure &e) {
    // broader error for DBMS problems
    std::throw_with_nested(DBException(
        std::string("delete_item failed (pqxx::failure): ") + e.what()));
  }
}

// insert_item
// returns Item if success
// throws DuplicatePhysicalLabel if failure
Item insert_item(pqxx::connection &conn, const InsertInput &in) {
  try {
    pqxx::work tx{conn};

    pqxx::result r = tx.exec_params(
        "insert into stash (physical_label, category, sub_category, contents) "
        "values ($1, $2, $3, $4) "
        "returning id, physical_label, category, sub_category, contents, "
        "(extract(epoch from created_at) * 1000000)::bigint as created_at_us, "
        "(extract(epoch from updated_at) * 1000000)::bigint as updated_at_us",
        in.physical_label, in.category, in.sub_category, in.contents);

    tx.commit();

    const auto &row = r[0];

    auto created_us = row["created_at_us"].as<long long>();
    auto updated_us = row["updated_at_us"].as<long long>();

    return Item{row["id"].as<long>(),
                row["physical_label"].is_null()
                    ? std::nullopt
                    : std::optional<std::string>{row["physical_label"].c_str()},
                row["category"].c_str(),
                row["sub_category"].c_str(),
                row["contents"].c_str(),
                std::chrono::sys_time<std::chrono::microseconds>{
                    std::chrono::microseconds{created_us}},
                std::chrono::sys_time<std::chrono::microseconds>{
                    std::chrono::microseconds{updated_us}}};
  } catch (const pqxx::sql_error &e) {
    if (e.sqlstate() == "23505") {
      // 23505 is unique constraint violation
      // if label is null this won't happen, but better safe than sorry
      throw DuplicatePhysicalLabel(in.physical_label.value_or("<null>"));
    }
    std::throw_with_nested(DBException("insert_item failed. sqlstate=" +
                                       e.sqlstate() + ", query=" + e.query()));
  }
}

void dump_stash_to_stdout() {
  pqxx::connection conn{""};

  pqxx::read_transaction tx{conn};
  pqxx::result r = tx.exec("select * from stash;");

  // header
  for (pqxx::row::size_type c = 0; c < r.columns(); ++c) {
    if (c)
      std::cout << " | ";
    std::cout << r.column_name(c);
  }
  std::cout << "\n";

  // rows
  for (const auto &row : r) {
    for (pqxx::row::size_type c = 0; c < row.size(); ++c) {
      if (c)
        std::cout << " | ";
      std::cout << (row[c].is_null() ? "NULL" : row[c].c_str());
    }
    std::cout << "\n";
  }
}

int main() { dump_stash_to_stdout(); }
