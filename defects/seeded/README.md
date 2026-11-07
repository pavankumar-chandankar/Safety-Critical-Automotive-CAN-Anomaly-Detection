# Intentionally defective samples (isolated from production)

These **nonproduction training mutants** demonstrate analyzer detection behavior. They are never compiled into `cansec`, `cansec_core`, or unit tests. Deliberately unsafe functions demonstrate null dereference, divide-by-zero, double delete, and out-of-bounds access. **Never execute these functions.** Use the Clang Static Analyzer on the source only.

`python3 scripts/check_seeded_defects.py` checks that the analyzer emits diagnostics for selected mutant families. Analyzer precision varies by LLVM version; the checker expects only the two most reliable examples (null dereference and division by zero). Additional mutants are educational, not guaranteed to be caught by every version.

The mutants are **not** a claim that similar defects existed in an automotive ECU, nor proof that a static analyzer can find every class of memory error.
