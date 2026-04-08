# AI Context for Glennergy

## Purpose

Glennergy fetches spot price and weather data, processes it through an optimization algorithm, and serves results over a local HTTP server.

This file gives AI assistants a minimal, stable baseline for working in this repository. It is intentionally short. Read the code for current behavior.

## Source of Truth

When answering questions or making changes, use this order of authority:

1. Current source code in the repository
2. Doxygen comments in the source files
3. Project docs such as [README.md](README.md) and [Docs/Doxygen_Standard.md](Docs/Doxygen_Standard.md)
4. Generated Doxygen output in `html/` and `latex/`

If documentation conflicts with code, trust the code and call out the mismatch.

## Repository Map

- `Server/`: HTTP/TCP server, request handling, logging, connection management
- `Algorithm/`: optimization and calculation logic
- `API/Spotpris/`: spot price fetching and parsing
- `API/Meteo/`: C implementation for weather data
- `API/Meteocpp/`: C++ implementation for weather data
- `Client-CPP/`: C++ client/request side code
- `Libs/`: shared helpers such as sockets, threads, shared memory, pipes, fetchers, cache
- `Docs/`: Doxygen standards and templates
- `html/`, `latex/`: generated documentation output

## Project Rules That Matter

- Prefer reading code and Doxygen comments before answering architecture or behavior questions.
- The repository uses Doxygen heavily. Side effects, ownership, warnings, and pre/postconditions are important and should be included in explanations when relevant.
- For documentation-only tasks, follow the rules in [Docs/Doxygen_Standard.md](Docs/Doxygen_Standard.md).
- For code changes, do not assume generated docs are up to date unless regenerated.

## Build and Run Notes

These rules come from the current repo docs and should be treated as important project conventions:

- Do not run `make` manually for normal development flow.
- After code changes, rerun `./glennergy_install.sh`.
- Run the installed binary as `Glennergy-Main`, not `./Glennergy-Main`.
- Stop the program with `Ctrl + C`, not `Ctrl + Z`.

If these rules appear outdated compared with the code or scripts, mention that explicitly.

## How To Answer Questions Well In This Repo

When explaining behavior, prefer this structure:

1. What happens
2. Which files and functions are involved
3. Important side effects
4. Ownership or lifetime concerns
5. Hidden assumptions, risks, or coupling

For non-trivial questions, inspect the relevant files instead of relying on this document alone.

## Question Recipes

These prompt patterns are good starting points for working in this repository.

### Understand A Code Path

Use `AI_CONTEXT.md` and `Docs/architecture-overview.md` as orientation, then inspect the relevant code before answering.
Explain:

1. What happens
2. Which files and functions are involved
3. Important side effects
4. Ownership or lifetime concerns
5. Hidden assumptions or risks

### Trace A Bug

Use `AI_CONTEXT.md` and `Docs/architecture-overview.md` as orientation, then trace the bug through the actual code path.
Do not propose fixes yet unless I ask.
Explain:

1. Most likely execution path
2. Where the failure could occur
3. Observability points such as logs, return values, and process boundaries
4. Likely root causes, ordered by confidence
5. Which files I should inspect first

### Review A Planned Change

Read the relevant code and Doxygen comments first.
Assume I want to preserve current behavior unless I explicitly say otherwise.
Explain:

1. What parts of the system this change touches
2. Possible side effects
3. Ownership, concurrency, IPC, or lifecycle risks
4. Whether the change crosses cache, shared memory, socket, thread, or subprocess boundaries
5. What tests or manual checks would give confidence

### Ask For A Safe Refactor

Read the relevant code first and preserve behavior.
Before changing anything, explain:

1. What can be refactored safely
2. What must remain unchanged
3. Which hidden couplings could break behavior
4. A small-step refactor plan

### Compare Documentation With Code

Inspect both the source code and the relevant Doxygen or markdown docs.
Use code as the source of truth.
List:

1. Confirmed matches
2. Mismatches
3. Ambiguities
4. Which document or comment should be updated

### Architecture Question

Use `AI_CONTEXT.md` and `Docs/architecture-overview.md` as orientation, then inspect the code that implements the relevant flow.
Explain the architecture in repo-specific terms, not generic patterns.
Call out:

1. Main modules involved
2. Data flow
3. Process or thread boundaries
4. IPC or shared-state boundaries
5. Operational assumptions

## When Making Changes

- Keep changes scoped to the user's request.
- Preserve existing project conventions unless the user asks for refactoring.
- If behavior depends on shared memory, pipes, threads, sockets, or subprocesses, review those interactions before changing logic.
- If behavior spans multiple modules, trace the flow across modules before answering confidently.
