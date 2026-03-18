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
  - Internal functions should be documented as `static` with notes/warnings

- **Structs and functions**:
  - Describe each struct with `@note` on memory ownership
  - Document arrays with `@note` on size and valid elements
  - Document each function with `@param`, `@return`, `@pre`, `@post`, `@warning`, `@note`

- **File locations**:
  - Templates are in `Docs/DoxygenTemplates/`
  - Actual source code files copy/adapt these templates

## How to use

1. Copy the appropriate template for your file type.
2. Replace placeholders (`<MODULENAME>`, `<description>`, etc.).
3. Fill in struct and function documentation according to the standard.
4. Doxygen will generate grouped, navigable documentation automatically.

> This ensures all modules have consistent, clear documentation that includes ownership, pre/post conditions, errors, and side effects.