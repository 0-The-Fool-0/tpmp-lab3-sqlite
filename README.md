# TPMP Lab 3 — SQLite in C

SQLite C API examples and a console app for states and geographic areas. Task 5 demonstrates parameterized queries, BLOB import/export and transactions.

## Technologies and layout
`examples/` has five examples; `project5/` has `include/` and `src/`; `docs/` has study notes and test protocol. C, SQLite3, Make.

## Build and run
Install C compiler, Make and SQLite development headers; run `cd project5 && make && ./state_app [path-to-db]`. Creates a local DB. The tracked `examples/test.db` is excluded.

## Verification and contribution
History attributes project5 implementation and protocol to Andrey Borisovets. Snapshot has a new history. No tests/builds were run for this snapshot.
