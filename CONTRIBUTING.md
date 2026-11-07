# Contributing and review standards

Open changes against a feature branch. Include issue context, changed CAN interpretation, evidence of compatibility with the nine-column input contract, new unit tests, a seeded failure case where applicable, and outputs of GCC/Clang builds and CTest. For source-code quality changes attach the static-analyzer and sanitizer run summaries. Never add original vehicle traces, credential files, proprietary DBC content, or any code that transmits frames to live vehicles without a separately reviewed scope change. Document dependencies and avoid blanket static-check suppressions.

For research modifications, explain whether the conclusion is a **synthetic contract check** or **empirically validated data-derived** result. Refer to `docs/requirements_traceability.md` and `docs/research_protocol.md`.
