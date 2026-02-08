# Local mutual TLS for Postgres

This repo uses mutual TLS between the backend and Postgres.

So,
1. The client verifies the Postgres server certificate and hostname.
2. Postgres verifies a client certificate, instead of a password.

## Files involved

Postgres uses:
- server.crt
- server.key
- ca.crt

Client side, used by the api client:
- client.crt
- client.key
- ca.crt

## Quick start

1. Generate certificates.

```bash
./scripts/dev-mkcerts.sh

