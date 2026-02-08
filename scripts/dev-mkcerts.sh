#!/usr/bin/env bash
set -euo pipefail

APP_NAME="${APP_NAME:-bagofholding}"
TLS_DIR="${TLS_DIR:-$HOME/.config/${APP_NAME}/tls}"

DB_USER="${DB_USER:-bagofholding_state_backend}"
SERVER_CN="${SERVER_CN:-localhost}"
DAYS_CA="${DAYS_CA:-3650}"
DAYS_LEAF="${DAYS_LEAF:-825}"

mkdir -p "$TLS_DIR"
cd "$TLS_DIR"

echo "Writing certificates into: $TLS_DIR"
echo "DB user for client certificate CN: $DB_USER"

# CA
if [[ ! -f ca.key ]]; then
  openssl genrsa -out ca.key 4096
  chmod 600 ca.key
fi

if [[ ! -f ca.crt ]]; then
  openssl req -x509 -new -nodes -key ca.key -sha256 -days "$DAYS_CA" \
    -subj "/CN=${APP_NAME}-local-dev-ca" \
    -out ca.crt
  chmod 644 ca.crt
fi

# Server key and cert
if [[ ! -f server.key ]]; then
  openssl genrsa -out server.key 2048
  chmod 600 server.key
fi

openssl req -new -key server.key -subj "/CN=${SERVER_CN}" -out server.csr

cat >server.ext <<EOF
basicConstraints=CA:FALSE
keyUsage=digitalSignature,keyEncipherment
extendedKeyUsage=serverAuth
subjectAltName=DNS:${SERVER_CN},IP:127.0.0.1
EOF

openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial \
  -out server.crt -days "$DAYS_LEAF" -sha256 -extfile server.ext
chmod 644 server.crt

# Client key and cert
if [[ ! -f client.key ]]; then
  openssl genrsa -out client.key 2048
  chmod 600 client.key
fi

openssl req -new -key client.key -subj "/CN=${DB_USER}" -out client.csr

cat >client.ext <<'EOF'
basicConstraints=CA:FALSE
keyUsage=digitalSignature
extendedKeyUsage=clientAuth
EOF

openssl x509 -req -in client.csr -CA ca.crt -CAkey ca.key -CAcreateserial \
  -out client.crt -days "$DAYS_LEAF" -sha256 -extfile client.ext
chmod 644 client.crt

rm -f server.csr client.csr server.ext client.ext

echo "Done."
echo "Files:"
ls -1 "$TLS_DIR"/ca.crt "$TLS_DIR"/server.crt "$TLS_DIR"/server.key "$TLS_DIR"/client.crt "$TLS_DIR"/client.key
