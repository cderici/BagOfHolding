#include <iostream>
#include <pqxx/pqxx>

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
