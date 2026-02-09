
<img width="200" height="200" alt="BagOfHoldingIcon" src="https://github.com/user-attachments/assets/bdf1510b-24a4-4e21-b439-c46853df19a3" /> 

# Bag Of Holding

Personal physical storage inventory app.

Imagine you have a physical storage room that you want to organize. You get some boxes, and put some labels on them (I use numbers). This app helps you note down the contents with tags, categories etc, and allows fuzzy lookup on various clients (cli, web, android).

🚧 Under Development 🚧

## Requirements

packages:

```bash
sudo apt install -y postgresql openssl libpq-dev libpqxx-dev build-essential cmake clang-20
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


### Environment variables

The backend uses libpq environment variables for connection configuration.

Minimum:

```bash
export PGHOST=localhost
export PGPORT=5432
export PGDATABASE=bagofholding_state
export PGUSER=bagofholding_state_backend

export PGSSLMODE=verify-full
export PGSSLROOTCERT="$HOME/.config/bagofholding/tls/ca.crt"
export PGSSLCERT="$HOME/.config/bagofholding/tls/client.crt"
export PGSSLKEY="$HOME/.config/bagofholding/tls/client.key"

```

2. Configure Postgres for mutual TLS

This copies server certificates into the Postgres data directory, enables TLS, configures Postgres to trust the local certificate authority, and adds pg hba rules requiring a verified client certificate for local connections. You'll need to make sure the order of the hba rules if you want to reject non-TLS connections.

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

4. Set up database schema

Apply DB setup SQL from `scripts/db-setup/` in order:

```bash
chmod +x scripts/db-setup.sh
./scripts/db-setup.sh
```

This creates a `schema_migrations` table and applies tracked schema files
(table, timestamps/trigger, search index).

5. Build and run the backend

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
