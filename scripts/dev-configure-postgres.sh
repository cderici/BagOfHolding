#!/usr/bin/env bash
set -euo pipefail

APP_NAME="${APP_NAME:-bagofholding}"
TLS_DIR="${TLS_DIR:-$HOME/.config/${APP_NAME}/tls}"

DB_NAME="${DB_NAME:-bagofholding_state}"
DB_USER="${DB_USER:-bagofholding_state_backend}"

require_file() {
  local p="$1"
  if [[ ! -f "$p" ]]; then
    echo "Missing file: $p"
    exit 1
  fi
}

require_file "$TLS_DIR/ca.crt"
require_file "$TLS_DIR/server.crt"
require_file "$TLS_DIR/server.key"

PGDATA="$(sudo -u postgres psql -tAc 'show data_directory;' | tr -d '[:space:]')"
CONF_FILE="$(sudo -u postgres psql -tAc 'show config_file;' | tr -d '[:space:]')"
HBA_FILE="$(sudo -u postgres psql -tAc 'show hba_file;' | tr -d '[:space:]')"

echo "Postgres data directory: $PGDATA"
echo "postgresql.conf: $CONF_FILE"
echo "pg_hba.conf: $HBA_FILE"

echo "Copying TLS files into Postgres data directory"
sudo cp "$TLS_DIR/server.crt" "$PGDATA/server.crt"
sudo cp "$TLS_DIR/server.key" "$PGDATA/server.key"
sudo cp "$TLS_DIR/ca.crt" "$PGDATA/ca.crt"

sudo chown postgres:postgres "$PGDATA/server.crt" "$PGDATA/server.key" "$PGDATA/ca.crt"
sudo chmod 600 "$PGDATA/server.key"
sudo chmod 644 "$PGDATA/server.crt" "$PGDATA/ca.crt"

set_conf_kv() {
  local key="$1"
  local val="$2"
  local file="$3"

  if sudo grep -Eq "^[[:space:]]*${key}[[:space:]]*=" "$file"; then
    sudo sed -i -E "s|^[[:space:]]*${key}[[:space:]]*=.*|${key} = ${val}|" "$file"
  else
    echo "${key} = ${val}" | sudo tee -a "$file" >/dev/null
  fi
}

echo "Enabling SSL in postgresql.conf"
set_conf_kv "ssl" "on" "$CONF_FILE"
set_conf_kv "ssl_cert_file" "'server.crt'" "$CONF_FILE"
set_conf_kv "ssl_key_file" "'server.key'" "$CONF_FILE"
set_conf_kv "ssl_ca_file" "'ca.crt'" "$CONF_FILE"

HBA_LINE_V4="hostssl  ${DB_NAME}  ${DB_USER}  127.0.0.1/32  cert clientcert=verify-full"
HBA_LINE_V6="hostssl  ${DB_NAME}  ${DB_USER}  ::1/128       cert clientcert=verify-full"

echo "Adding pg_hba.conf rules if missing"
if ! sudo grep -Fxq "$HBA_LINE_V4" "$HBA_FILE"; then
  echo "$HBA_LINE_V4" | sudo tee -a "$HBA_FILE" >/dev/null
fi

if ! sudo grep -Fxq "$HBA_LINE_V6" "$HBA_FILE"; then
  echo "$HBA_LINE_V6" | sudo tee -a "$HBA_FILE" >/dev/null
fi

echo "Restarting Postgres"
sudo systemctl restart postgresql

echo "Checking SSL status"
sudo -u postgres psql -tAc "show ssl;" | tr -d '[:space:]' | grep -qi "on" && echo "SSL is on."

echo "Done."
