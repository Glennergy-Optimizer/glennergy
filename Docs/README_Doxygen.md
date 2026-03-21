# Doxygen Documentation Standard (AI-Enforced Version)

This project uses a **strict, AI-enforced Doxygen documentation standard**.

This document is not a guideline — it is a **set of mandatory rules** that MUST be followed for every file.

---

## ⚠️ AI COMPLIANCE CONTRACT (MANDATORY)

When an AI agent processes any file in this project, it MUST:

1. **Follow ALL rules in this document without exception**
2. **NEVER modify logic, control flow, or initialization**
3. **ONLY add documentation and comments**
4. **VERIFY compliance using the checklist before returning output**
5. **REFUSE to return a file if any checklist item is not satisfied**

Failure to follow these rules results in **invalid output**.

---

## Overview

* Doxygen templates exist for three file types:

  * **Header (.h)** – full public API documentation
  * **Source (.c)** – implementation details, internal helpers, `@ingroup` usage
  * **Main / test (.c)** – entry points or demos, minimal docs

---

## 🔒 Mandatory Rules

### 1. Code Integrity (ABSOLUTE RULE)

AI MUST NOT:

* Change logic
* Change control flow (if/else, loops, switches)
* Change variable initialization
* Change function signatures
* Move code
* Remove code

AI MAY:

* Add Doxygen comments
* Add normal comments
* Add `// Suggestion:` comments

---

### 2. Suggestions Policy

All improvements MUST be written as comments:

```c
// Suggestion: <clear and specific improvement>
```

Rules:

* Suggestions MUST NOT affect execution
* Suggestions MUST NOT replace existing code
* Suggestions MUST be optional and reviewable

---

### 3. Preservation Rules (CRITICAL)

AI MUST preserve:

* All existing comments (ANY language)
* All debug prints (`printf`, `fprintf`, logs)
* All commented-out includes:

  ```c
  // #include <string.h>
  ```
* All TODOs and notes
* All commented-out code

AI MUST NOT:

* Remove
* Rewrite
* Translate
* Relocate

---

### 4. Language Rules

* All **Doxygen documentation MUST be in English**
* Original comments MUST remain unchanged (no translation)

---

### 5. Doxygen Requirements

#### File Level

Every file MUST contain:

* `@file`
* `@brief`

Additionally:

* Header files MUST include `@defgroup`
* Source files MUST include `@ingroup`

---

#### Struct Documentation

Every struct MUST include:

* Description of purpose
* `@note` describing **memory ownership**
* `@note` describing arrays:

  * Size
  * Valid elements

---

#### Function Documentation

Every function MUST include:

* `@brief`
* `@param` (all parameters)
* `@return`
* `@pre` (required state before call)
* `@post` (state after call)
* `@warning` (risks, undefined behavior, side effects)
* `@note` (ownership, memory, debug behavior, etc.)

---

### 6. Internal Functions

* MUST be documented
* MUST include warnings if unsafe
* MUST describe side effects

---

## 🔁 Mandatory AI Verification Step

Before returning ANY file, the AI MUST perform a **full checklist validation**.

If ANY item is missing:

* The AI MUST continue working
* The AI MUST NOT return incomplete output

---

## ✅ AI Doxygen Checklist (REQUIRED)

### File Comments

* [ ] `@file` exists
* [ ] `@brief` exists
* [ ] Header: `@defgroup` present
* [ ] Source: `@ingroup` present

### Structs / Typedefs

* [ ] All structs documented
* [ ] Memory ownership described (`@note`)
* [ ] Arrays documented (size + valid elements)

### Functions

* [ ] ALL functions documented (no exceptions)
* [ ] `@param` complete
* [ ] `@return` present
* [ ] `@pre` defined
* [ ] `@post` defined
* [ ] `@warning` included
* [ ] `@note` included

### Comments / Code Safety

* [ ] ALL original comments preserved
* [ ] NO logic modified
* [ ] NO initialization changed
* [ ] NO control flow changed
* [ ] Debug prints preserved
* [ ] Commented includes preserved

### Suggestions

* [ ] Suggestions use correct format
* [ ] Suggestions do NOT affect execution

### General

* [ ] Correct template used
* [ ] Groups (`@defgroup` / `@ingroup`) correct
* [ ] Documentation is consistent

---

## ❌ Invalid Output Conditions

The output is INVALID if:

* Any checklist item is missing
* Any code was modified
* Any comment was removed or altered
* Any required Doxygen tag is missing

---

## Purpose

* Enforce **strict consistency**
* Ensure **safe AI-assisted documentation**
* Guarantee **reviewable, non-destructive changes**
* Enable **automated validation of AI output**

---

## Summary for AI Agents

You are not allowed to:

* Skip rules
* Assume completeness
* Return partial documentation

You are required to:

* Follow ALL rules
* Validate using checklist
* Ensure 100% compliance before returning output
