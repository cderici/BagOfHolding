#pragma once

#include "types.h"
#include <pqxx/pqxx>
#include <string>
#include <vector>

namespace state {
bool delete_item(pqxx::connection &conn, long itemID);
Item insert_item(pqxx::connection &conn, const InsertInput &in);
std::vector<SearchHit> search_items(pqxx::connection &conn,
                                    const std::string &term,
                                    std::size_t limit = 20);
} // namespace state
