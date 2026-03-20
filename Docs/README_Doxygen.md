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

---

## AI-generated code suggestions

When using AI to generate or document code:

1. **Do not modify existing logic or initialization.**  
   - AI should **never** change the actual behavior of the code.  
   - This includes all function calls, variable assignments, initializations, and control flow — even if the code is suboptimal or unnecessary.  
   - Example: Do not remove or alter `socket = -1;` or `close(tcp_server->server_socket);` lines.

2. **Mark potential improvements as comments**  
   - Use the format:
     ```c
     // Suggestion: [description of the improvement]
     ```
   - Examples:
     ```c
     // Suggestion: Could initialize connection->bytesReadOut to 0 here to avoid garbage value
     // Suggestion: Free json_data before returning to avoid memory leak on error
     // Suggestion: Setting socket = -1 does not affect caller
     // Suggestion: Could check tcp_server->server_socket >= 0 to avoid double-close
     ```
   - These comments are **informative only** and do **not alter code execution**.

3. **Follow normal Doxygen conventions regardless**  
   - All structs, functions, and files must still be fully documented with `@file`, `@brief`, `@param`, `@return`, etc.

4. **Purpose**  
   - Ensures consistency across the project.  
   - Allows developers to review AI suggestions manually before applying any changes.

---

### Keep Original Comments

**AI must always keep all original code comments intact.**

- Do **not** remove, rewrite, or relocate existing comments.  
- Original TODOs, developer notes, clarifying comments, or inline explanations must remain exactly where they are.  
- AI may add new Doxygen comments, but they must **never replace or delete** existing comments.

**Example:**
```c
// TODO: implement caching
// Suggestion: Could pre-allocate cache array for performance