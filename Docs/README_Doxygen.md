# Doxygen Documentation Standard

This project uses a **customized Doxygen standard** for consistent documentation.

## Overview

- Doxygen templates exist for three file types:
  - **Header (.h)** – full public API documentation
  - **Source (.c)** – implementation details, internal helpers, `@ingroup` usage
  - **Main / test (.c)** – entry points or demos, minimal docs

- **Key conventions**:
  - Use `@file`, `@brief` for all files
  - Use `@defgroup` in headers and `@ingroup` in .c files
  - Document **ownership of memory**, pre/post conditions, side effects, errors
  - Keep original code comments if helpful for context
  - Internal functions should be documented with notes/warnings

- **Language requirement**:
  - All **Doxygen documentation must be written in English**
  - Original code comments must **never be modified or translated**, regardless of language
  - This ensures consistency in generated documentation while preserving developer intent

- **Structs and functions**:
  - Describe each struct with `@note` on memory ownership
  - Document arrays with `@note` on size and valid elements
  - Document each function with `@param`, `@return`, `@pre`, `@post`, `@warning`, `@note`

- **File locations**:
  - Templates are in `Docs/DoxygenTemplates/`
  - Actual source code files copy/adapt these templates

---

## How to use

1. Copy the appropriate template for your file type.
2. Replace placeholders (`<MODULENAME>`, `<description>`, etc.).
3. Fill in struct and function documentation according to the standard.
4. Doxygen will generate grouped, navigable documentation automatically.

> This ensures all modules have consistent, clear documentation that includes ownership, pre/post conditions, errors, and side effects.

---

## AI Handling Instructions

When using AI to generate or document code, follow these rules:

1. **Do not modify existing logic, control flow, or initialization.**  
   - AI must **never** change behavior, assignments, loops, if-checks, `static` qualifiers, or initializations.
   - All original code logic must remain intact.

2. **Mark potential improvements only as comments**  
   - Use the format:
     ```c
     // Suggestion: [description of the improvement]
     ```
   - Examples:
     ```c
     // Suggestion: Could initialize connection->bytesReadOut to 0 here
     // Suggestion: Free json_data before returning to avoid memory leak
     // Suggestion: Consider logging errors for socket(), bind(), etc.
     ```
   - Suggestions **do not alter execution**.

3. **Follow all Doxygen conventions**  
   - Add `@file`, `@brief`, `@param`, `@return`, `@pre`, `@post`, `@warning`, `@note` consistently.
   - Document structs, arrays, and all functions fully.
   - All Doxygen documentation must be written in English.

4. **Never remove existing comments or code**  
   - Old commented-out code, TODOs, and clarifying notes must remain.
   - AI may add new Doxygen comments or suggestions **but never delete or move old code/comments**.

5. **Checklist verification**  
   - AI must self-check the following before returning a file:

---

### AI Doxygen Checklist

For every file:

**File comments**
- [ ] `@file` exists
- [ ] `@brief` exists
- [ ] Header files: `@defgroup` present
- [ ] Source files: `@ingroup` present

**Structs / Typedefs**
- [ ] All structs documented
- [ ] Memory ownership described (`@note`)
- [ ] Arrays documented with size & valid elements (`@note`)

**Functions**
- [ ] All functions documented
- [ ] `@param` and `@return` included
- [ ] `@pre` and `@post` for required conditions
- [ ] `@warning` for risks or side effects
- [ ] `@note` for extra context (ownership, side effects, arrays, etc.)

**Comments / code**
- [ ] All original comments preserved
- [ ] Suggestions only as `// Suggestion: ...`
- [ ] No code logic or initialization modified

**General**
- [ ] Doxygen groups correct and consistent
- [ ] Templates from `Docs/DoxygenTemplates/` followed
- [ ] Documentation consistent throughout the file

---

## Purpose

- Ensures **consistent, high-quality documentation** across the project.
- Allows developers and AI agents to **verify compliance** using a checklist.
- Supports review of AI suggestions before manual approval.

---

### Keep Original Comments