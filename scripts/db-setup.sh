#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DB_SETUP_DIR="${SCRIPT_DIR}/db-setup"

if ! command -v psql >/dev/null 2>&1; then
  echo "Error: psql is required but not installed."
  exit 1
fi

if [[ ! -d "${DB_SETUP_DIR}" ]]; then
  echo "Error: db setup directory not found at ${DB_SETUP_DIR}"
  exit 1
fi

echo "Checking database connection"
psql -v ON_ERROR_STOP=1 -X -qAtc "select 1;" >/dev/null

echo "Ensuring schema_migrations table exists"
psql -v ON_ERROR_STOP=1 -X <<'SQL'
create table if not exists schema_migrations (
  version text primary key,
  applied_at timestamptz not null default now()
);
SQL

shopt -s nullglob
migration_files=("${DB_SETUP_DIR}"/*.sql)

if [[ ${#migration_files[@]} -eq 0 ]]; then
  echo "No SQL files found in ${DB_SETUP_DIR}"
  exit 1
fi

for file in "${migration_files[@]}"; do
  version="$(basename "${file}")"

  already_applied="$({
    psql -v ON_ERROR_STOP=1 -X -qAt \
      -c "select 1 from schema_migrations where version = '${version}' limit 1;"
  } || true)"

  if [[ "${already_applied}" == "1" ]]; then
    echo "Skipping ${version} (already applied)"
    continue
  fi

  echo "Applying ${version}"
  psql -v ON_ERROR_STOP=1 -X -f "${file}"

  psql -v ON_ERROR_STOP=1 -X \
    -c "insert into schema_migrations (version) values ('${version}');"
done

echo "DB setup complete. Applied versions:"
psql -v ON_ERROR_STOP=1 -X -qAt -c "select version from schema_migrations order by version;"
