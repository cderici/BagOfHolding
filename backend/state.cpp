#include "state.h"
#include "errors.h"
#include <chrono>
#include <exception>
#include <optional>
#include <pqxx/pqxx>

std::vector<SearchHit> state::search_items(pqxx::connection &conn,
                                           const std::string &term,
                                           std::size_t limit) {
  std::vector<SearchHit> results;

  try {
    pqxx::read_transaction tx{conn};

    std::string query = R"( 
select id, physical_label, category, sub_category, contents from stash
where
  lower(
    immutable_unaccent(
      coalesce(physical_label, '') || ' ' ||
      coalesce(category, '') || ' ' ||
      coalesce(sub_category, '') || ' ' ||
      coalesce(contents, '')
    )
  ) % lower(immutable_unaccent($1::text))
order by
  (lower(coalesce(immutable_unaccent(contents), '')) like lower(immutable_unaccent($1::text)) || '%') desc,
  similarity(
    lower(
      immutable_unaccent(
        coalesce(physical_label, '') || ' ' ||
        coalesce(category, '') || ' ' ||
        coalesce(sub_category, '') || ' ' ||
        coalesce(contents, '')
      )
    ),
    lower(immutable_unaccent($1::text))
  ) desc,
  id desc
limit $2;
      )";

    pqxx::result r = tx.exec_params(query, term, limit);

    // not really necessary for read_transactions but still keeping it to be
    // explicit
    tx.commit();

    for (const auto &row : r) {
      results.push_back(SearchHit{
          row["id"].as<long>(),
          row["physical_label"].is_null()
              ? std::nullopt
              : std::optional<std::string>{row["physical_label"].c_str()},
          row["category"].c_str(),
          row["sub_category"].c_str(),
          row["contents"].c_str(),
      });
    }

    return results;

  } catch (const pqxx::sql_error &e) {
    // try catching the SQL-related problems
    std::throw_with_nested(DBException("search_items failed. sqlstate=" +
                                       e.sqlstate() + ", query=" + e.query()));
  } catch (const pqxx::failure &e) {
    // broader error for DBMS problems
    std::throw_with_nested(DBException(
        std::string("search_items failed (pqxx::failure): ") + e.what()));
  }
}

// delete_item
// returns true --> item deleted as expected
// return false --> item was not there anyways
// may throw exception if postgres throws
bool state::delete_item(pqxx::connection &conn, long itemID) {
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
Item state::insert_item(pqxx::connection &conn, const InsertInput &in) {
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
