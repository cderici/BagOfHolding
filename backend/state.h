#pragma once

#include "state_types.h"
#include <pqxx/pqxx>

namespace state {
bool delete_item(pqxx::connection &conn, long itemID);
Item insert_item(pqxx::connection &conn, const InsertInput &in);
} // namespace state
