# Doxygen Documentation Standard (AI-Enforced Version)

This project uses a strict, AI-enforced Doxygen documentation standard.

The goal is to produce consistent, useful documentation while preserving code and keeping implementation files readable.

---

## AI Compliance Contract

When an AI agent processes any file in this project, it MUST:

1. Follow the rules in this document
2. Never modify logic, control flow, initialization, or signatures
3. Only add or update documentation and comments
4. Preserve existing comments and developer intent
5. Prefer concise, useful documentation over repetitive boilerplate

If a rule conflicts with code preservation, preserve the code and existing comments.

---

## File Roles

Different file types require different documentation depth:

- Header files (`.h`, `.hpp`) are the authoritative public API contract
- Source files (`.c`, `.cpp`) focus on implementation details and internal helpers
- Main, test, and demo files use minimal, practical documentation

Do not document source files as heavily as headers unless the code genuinely requires it.

---

## Mandatory Rules

### 1. Code Integrity

AI MUST NOT:

- Change logic
- Change control flow
- Change variable initialization
- Change function signatures
- Move code
- Remove code

AI MAY:

- Add or update Doxygen comments
- Add or update normal comments
- Add `// Suggestion:` comments

### 2. Suggestions Policy

All improvements must be written as comments:

```c
// Suggestion: <clear and specific improvement>
```

Suggestions must:

- Not affect execution
- Not replace existing code
- Remain optional and reviewable

### 3. Preservation Rules

AI MUST preserve:

- All existing comments in any language
- All debug prints (`printf`, `fprintf`, logs)
- All TODOs and notes
- All commented-out includes
- All commented-out code

AI MUST NOT:

- Remove existing comments
- Rewrite comments unnecessarily
- Translate original comments
- Relocate comments unless needed to place documentation directly above the documented item

### 4. Language Rules

- All new Doxygen documentation must be in English
- Original comments must remain unchanged

---

## Required File-Level Tags

Every documented file must include:

- `@file`
- `@brief`

Additionally:

- Header files must include `@defgroup`
- Source files must include `@ingroup`

---

## Header File Standard

Headers are the main source of truth for public API documentation.

Public types and functions in headers should usually contain:

- `@brief`
- `@param` for all parameters when applicable
- `@return` when the function returns a value

Add these when they communicate meaningful contract information:

- `@note`
- `@warning`
- `@pre`
- `@post`

Use these tags when they add value, not just to satisfy a pattern.

Typical reasons to include `@note`, `@warning`, `@pre`, or `@post`:

- ownership or lifetime rules
- blocking I/O
- thread-safety assumptions
- side effects that are not obvious from the signature
- real misuse risks

Simple debug, print, dump, or trace helper declarations in headers should usually stay lightweight.

For these functions, prefer:

- `@brief`
- `@param` when applicable

Do not add `@pre`, `@post`, `@warning`, or `@note` to simple debug helpers unless they communicate something non-obvious or important.
Do not expand simple debug helper declarations into full contract-style blocks by default.

---

## Source File Standard

Source files are implementation-oriented and should stay concise.

Public function implementations in source files should usually use one of these styles:

1. A short implementation summary
2. A short implementation summary plus a note such as:
   `See header for full contract documentation.`

Do not repeat full public API contracts in `.c` or `.cpp` files when the header already documents them.

Internal or `static` helper functions in source files must be documented, but briefly.

Include `@param`, `@return`, `@note`, or `@warning` only when they communicate something non-obvious or important, such as:

- internal ownership assumptions
- blocking behavior
- thread interaction
- hidden side effects
- unsafe usage constraints

Avoid low-value boilerplate, for example:

- obvious consequences already implied by the code
- generic warnings on every function
- notes that restate standard C behavior without project relevance

If two versions are both correct, prefer the more concise one.

---

## Main, Test, and Demo Files

These files should remain minimal.

They must include:

- `@file`
- `@brief`
- `@ingroup`

Document the file purpose and important side effects when relevant, but do not turn demos into full API references.

---

## Struct Documentation

Public structs should include:

- a short description of purpose
- ownership or lifetime notes when relevant
- array size or valid element notes when relevant

Do not invent ownership notes when the code does not imply a meaningful ownership rule.
Keep individual field comments short by default. Prefer putting the main explanation at the struct level and using concise field comments unless a field is subtle, safety-critical, or easy to misuse.
For ordinary fields, prefer short inline comments such as `/**< Number of valid entries. */` over full multi-line Doxygen blocks.
Do not turn each struct field into a mini documentation section unless the field truly needs that level of explanation.

---

## Function Documentation Rules

All functions should be documented, but the level of detail depends on the file role.

### Public Functions in Headers

Required:

- `@brief`
- `@param` for each parameter when applicable
- `@return` if non-void

Optional, when meaningful:

- `@note`
- `@warning`
- `@pre`
- `@post`

### Public Functions in Source Files

Required:

- `@brief`

Optional, only when useful:

- `@param`
- `@return`
- `@note`
- `@warning`

Prefer concise implementation-oriented wording. If the header already contains the contract, do not duplicate it.

### Internal Functions

Required:

- `@brief`

Optional, only when useful:

- `@param`
- `@return`
- `@note`
- `@warning`

---

## Target Style Examples

Use the examples below as the preferred style reference when multiple valid documentation styles are possible.

### Public Header Function

Public header functions should carry the main contract documentation.

```c
/**
 * @brief Fetches data for all configured sources.
 *
 * @param[out] output Structure to populate with fetched data.
 *
 * @return
 * - 0 on success
 * - -1 on critical failure
 *
 * @note Performs blocking network I/O.
 * @pre output != NULL
 * @post output contains fetched data for all successfully processed sources.
 */
int Module_FetchAll(ModuleData *output);
```

### Public Source Function

Public functions in source files should usually remain brief and defer full contract details to the header.

```c
/**
 * @brief Implementation of Module_FetchAll.
 *
 * See header for full contract documentation.
 */
int Module_FetchAll(ModuleData *output)
{
    ...
}
```

This is the preferred default for public non-static functions in `.c` or `.cpp` files.

### Debug Helper Function

Simple debug or print helpers should stay lightweight.

```c
/**
 * @brief Prints an entry for debugging.
 *
 * @param e Pointer to entry to print.
 */
void ModuleEntry_Print(const ModuleEntry *e);
```

Avoid adding `@pre`, `@post`, `@warning`, or `@note` to simple debug helpers unless they communicate something truly non-obvious.
Do not expand simple debug helper declarations into full contract-style documentation by default.

### Struct Documentation

Document important struct semantics, but keep individual field comments short.
Prefer short inline field comments over full multi-line field blocks unless the field is subtle or safety-critical.

```c
/**
 * @brief Data set for one logical source.
 *
 * Contains parsed entries and the raw upstream response.
 *
 * @note Owns all nested data.
 */
typedef struct
{
    char name[16];               /**< Source identifier. */
    size_t count;                /**< Number of valid entries in `entries`. */
    ModuleEntry entries[128];    /**< Parsed entries; only first `count` are valid. */
    char raw_data[32000];        /**< Raw upstream response, possibly truncated. */
} ModuleData;
```

Prefer this style over multi-line field-level explanations unless a field is subtle, safety-critical, or easy to misuse.

### Main, Test, and Demo File

Main or test files should use a practical middle ground: helpful context without becoming a design document.

```c
/**
 * @file moduletest.c
 * @brief Test entry point for the module.
 *
 * Fetches module data and sends it through the IPC path used by the consumer.
 *
 * @ingroup MODULE
 *
 * @note Performs upstream requests and blocking IPC I/O.
 * @warning The IPC write path may block until a reader connects.
 */
```

And for `main`:

```c
/**
 * @brief Runs the module test flow.
 *
 * Initializes dependencies, fetches module data, and writes the
 * resulting structure to the configured IPC path.
 *
 * @return
 * - 0 on success
 * - Negative error code on failure
 *
 * @note Intended for testing and integration rather than core business logic.
 */
int main(void)
{
    ...
}
```

Prefer this balanced style over both extremes:

- overly minimal comments that omit operationally important context
- overly long file headers that read like a mini specification

---

## Validation Checklist

Before returning a file, verify:

### File Comments

- `@file` exists
- `@brief` exists
- Header files use `@defgroup`
- Source files use `@ingroup`

### Structs and Types

- Public structs are documented
- Ownership or lifetime is described when relevant
- Arrays are described when relevant

### Functions

- All functions have at least an appropriate level of documentation
- Header public APIs contain clear contract documentation
- Source public APIs stay concise when headers already cover the contract
- Internal helpers are documented without unnecessary boilerplate

### Safety

- Original comments are preserved
- Logic is unchanged
- Initialization is unchanged
- Control flow is unchanged
- Debug prints are preserved
- Commented includes are preserved

### Suggestions

- Suggestions use `// Suggestion:`
- Suggestions do not affect execution

---

## Invalid Output Conditions

Output is invalid if:

- Code behavior changes
- Existing comments are removed or altered unnecessarily
- Required file-level tags are missing
- Public headers are undocumented
- Source files are filled with repetitive boilerplate instead of concise implementation docs

---

## Summary for AI Agents

You are required to:

- Preserve code and comments
- Document headers thoroughly
- Document source files concisely
- Use warnings and notes only when they add real value
- Prefer useful clarity over exhaustive repetition
