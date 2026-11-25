No external API integration: the detector's `api` signal is a false positive on the
requirement identifiers `API-01`, `API-05` and `API-06` (this project's core-contract
requirement ids), and Phase 9 touches no network, SDK, service, endpoint or schema —
its whole surface is in-repo shell checkers, control fixtures, test suites and
`.planning/` records.
