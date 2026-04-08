# Doxygen Documentation Future TODOs

This file tracks ideas and open questions for future iterations of the Doxygen workflow and documentation standard.

These notes are not part of the active documentation rules used by the automation prompt.

## Context Expansion Options

- Decide whether matching `.h` and `.c/.cpp` pairing should remain the only automatic context expansion rule.
- Revisit optional small-module support where a module may also include a closely related `main` or test file when that relationship is explicit and low-risk.
- Evaluate whether explicit per-module mapping would be useful for a few special cases, for example modules with one header, one source file, and one tightly coupled test harness.
- Revisit remaining-only rerun behavior so it can resume from the most relevant failed/deferred state when newer successful runs have happened in between.
- Add per-model pricing support so workflow token-cost summaries reflect the actual selected model instead of a single default pricing rate.

## Documentation Style Questions

- Revisit whether warnings about blocking behavior in main/test files should remain part of the preferred style by default, or only be included when especially important.
