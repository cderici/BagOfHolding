# BagOfHolding

Personal physical storage inventory app.
This repo contains a backend state layer backed by PostgreSQL, plus clients.

Local development uses mutual TLS between the backend and PostgreSQL.
This means the backend authenticates to Postgres with a client certificate, and the backend verifies the Postgres server certificate and hostname.

No keys or certificates are committed to this repo.

## Requirements

Ubuntu packages:

```bash
sudo apt install -y postgresql openssl libpq-dev libpqxx-dev build-essential cmake
```


## Quick start (local dev)

1. Generate local dev certificates

This generates a local certificate authority, a Postgres server certificate, and a backend client certificate.

```bash
chmod +x scripts/dev-mkcerts.sh scripts/dev-configure-postgres.sh
./scripts/dev-mkcerts.sh
```


Default output directory:

```
~/.config/bagofholding/tls/
  ca.crt
  server.crt
  server.key
  client.crt
  client.key
```

2. Configure Postgres for mutual TLS

This copies server certificates into the Postgres data directory, enables TLS, configures Postgres to trust the local certificate authority, and adds pg hba rules requiring a verified client certificate for local connections.

```bash
./scripts/dev-configure-postgres.sh
```


Defaults:

- `DB_NAME` is `bagofholding_state`
- `DB_USER` is `bagofholding_state_backend`
- Restricts to localhost only

3. Create the database and user

If you have not already created them:

```bash
sudo -u postgres psql
```


Inside psql:

```sql
create user bagofholding_state_backend;
create database bagofholding_state owner bagofholding_state_backend;
```

4. Build and run the backend

See backend/ for build instructions and targets.

## Mutual TLS: What lives where

In the general case there are two machines or runtimes:

- Database machine running Postgres
- Backend machine running the backend process

Server side, lives on the Postgres machine:

- `server.key` and `server.crt`
- `ca.crt` used by Postgres to verify client certificates

Client side, lives on the backend machine:

- `client.key` and `client.crt` used by the backend to authenticate
- `ca.crt` used by the backend to verify the Postgres server certificate

The backend and database do not need to be on the same machine.
Each side only needs the files required for its role.

## Connection settings

The backend uses libpqxx.
A typical connection string looks like:

```
host=localhost
port=5432
dbname=bagofholding_state
user=bagofholding_state_backend
sslmode=verify-full
sslrootcert=/home/USER/.config/bagofholding/tls/ca.crt
sslcert=/home/USER/.config/bagofholding/tls/client.crt
sslkey=/home/USER/.config/bagofholding/tls/client.key
```


`sslmode=verify-full` enforces hostname verification, which requires the server certificate to include SAN entries for `localhost` and `127.0.0.1`.

## Docs

`docs/tls-local-dev.md` explains the local mutual TLS setup and how to verify it.
