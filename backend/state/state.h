#pragma once

#include "types.h"
#include <cstddef>
#include <iterator>
#include <pqxx/pqxx>
#include <string>
#include <vector>

namespace state {

bool delete_item(pqxx::connection &conn, long itemID);
Item insert_item(pqxx::connection &conn, const InsertInput &in);
std::vector<SearchHit> search_items(pqxx::connection &conn,
                                    const std::string &term,
                                    std::size_t limit = 20);

// auth stuff
UserAuthRecord create_user(pqxx::connection &conn, const CreateUserInput &in);

// User Auth
std::optional<UserAuthRecord>
find_user_auth_by_username(pqxx::connection &conn, const std::string &username);
std::optional<UserAuthRecord> find_user_auth_by_id(pqxx::connection &conn,
                                                   long user_id);

bool update_user_password_hash(pqxx::connection &conn, long user_id,
                               const std::string &new_password_hash);

// Session Auth
AuthSessionRecord create_auth_session(pqxx::connection &conn,
                                      const CreateAuthSessionInput &in);
std::optional<AuthSessionRecord>
find_auth_session_by_refresh_hash(pqxx::connection &conn,
                                  const std::string &refresh_token_hash);

std::optional<AuthSessionRecord>
rotate_auth_session(pqxx::connection &conn, long session_id,
                    const std::string &new_refresh_token_hash,
                    Timestamp new_expires_at);
bool touch_auth_session_last_used_at(pqxx::connection &conn, long session_id);
bool revoke_auth_session(pqxx::connection &conn, long session_id);
std::size_t revoke_all_auth_sessions_for_user(pqxx::connection &conn,
                                              long user_id);

} // namespace state
