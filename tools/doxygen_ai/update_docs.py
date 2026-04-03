#!/usr/bin/env python3
"""
Update Doxygen comments in changed C/C++ files using the OpenAI Responses API.

The script is intentionally conservative:
- Only changed files are processed
- Only selected source/header extensions are allowed
- A file-level documentation heuristic decides between "update" and "document"
- The generated output is rejected if non-comment code changed
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import urllib.error
import urllib.request
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


REPO_ROOT = Path(__file__).resolve().parents[2]
DOCS_DIR = REPO_ROOT / "Docs"
AI_CONTEXT_PATH = REPO_ROOT / "AI_CONTEXT.md"
ALLOWED_SUFFIXES = {".c", ".h", ".cpp", ".hpp", ".cc", ".hh"}
DEFAULT_MODEL = os.getenv("OPENAI_MODEL", "gpt-5.4-mini")
DEFAULT_MAX_FILES = int(os.getenv("MAX_FILES_PER_RUN", "2"))
DEFAULT_MAX_CHARS = int(os.getenv("MAX_CHARS_PER_FILE", "18000"))
DEFAULT_MAX_OUTPUT_TOKENS = int(os.getenv("OPENAI_MAX_OUTPUT_TOKENS", "12000"))


@dataclass
class Usage:
    input_tokens: int = 0
    output_tokens: int = 0
    total_tokens: int = 0


def run_git(args: list[str]) -> str:
    result = subprocess.run(
        ["git", *args],
        cwd=REPO_ROOT,
        capture_output=True,
        text=True,
        check=True,
    )
    return result.stdout.strip()


def load_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def resolve_diff_range(base: str | None, head: str | None) -> tuple[str, str]:
    if base and head:
        return base, head
    if base:
        return base, head or "HEAD"

    event_name = os.getenv("GITHUB_EVENT_NAME", "")
    if event_name == "push":
        before = os.getenv("GITHUB_EVENT_BEFORE", "")
        sha = os.getenv("GITHUB_SHA", "") or "HEAD"
        if before and before != "0000000000000000000000000000000000000000":
            return before, sha
    if event_name == "pull_request":
        base_ref = os.getenv("GITHUB_BASE_REF", "")
        sha = os.getenv("GITHUB_SHA", "") or "HEAD"
        if base_ref:
            return f"origin/{base_ref}", sha

    return "HEAD~1", "HEAD"


def changed_files(base: str, head: str) -> list[Path]:
    names = run_git(["diff", "--name-only", "--diff-filter=ACMR", base, head]).splitlines()
    files: list[Path] = []
    for name in names:
        rel = Path(name)
        if rel.suffix.lower() not in ALLOWED_SUFFIXES:
            continue
        abs_path = REPO_ROOT / rel
        if abs_path.exists():
            files.append(abs_path)
    return files


def has_doxygen(text: str) -> bool:
    return bool(re.search(r"/\*\*[\s\S]*?@(file|brief|param|return|defgroup|ingroup)\b", text))


def is_entrypoint_or_test(path: Path, text: str) -> bool:
    lowered = path.name.lower()
    return lowered.startswith("main.") or "int main(" in text or "int main(void" in text


def build_prompt(path: Path, code: str, docs_rules: str, header_template: str, source_template: str, main_template: str, ai_context: str) -> tuple[str, str]:
    file_kind = "header" if path.suffix.lower() in {".h", ".hpp", ".hh"} else "source"
    mode = "update_existing_docs" if has_doxygen(code) else "document_full_file"
    template = header_template if file_kind == "header" else main_template if is_entrypoint_or_test(path, code) else source_template

    system_prompt = f"""You are a documentation-only refactoring assistant for a C/C++ repository.

You must obey these rules:
- Only add or update Doxygen comments and normal comments.
- Do not change code logic, control flow, initialization, signatures, includes, ordering, or formatting unless required to insert comments.
- Preserve all existing comments, TODOs, debug prints, commented-out code, and commented-out includes.
- Return the full updated file content only.
- Do not wrap the result in Markdown fences.
- All Doxygen text must be in English.
- Suggestions, if any, must use exactly: // Suggestion: ...
- Follow the repository's mandatory Doxygen rules and templates below.

Repository AI context:
{ai_context}

Mandatory Doxygen rules:
{docs_rules}

Relevant template:
{template}
"""

    user_prompt = f"""Task mode: {mode}
Path: {path.relative_to(REPO_ROOT).as_posix()}

Interpret the task mode as follows:
- update_existing_docs: bring the file's documentation up to the repository standard, while preserving existing comments and code.
- document_full_file: add complete repository-standard Doxygen coverage for the file, while preserving existing comments and code.

Before finalizing internally, validate that:
- every required file/function/struct tag is present
- the documentation matches the repository rules
- code behavior is unchanged

Return only the full updated file.

Current file:
{code}
"""
    return system_prompt, user_prompt


def extract_output_text(data: dict) -> str:
    if isinstance(data.get("output_text"), str) and data["output_text"].strip():
        return data["output_text"]

    outputs = data.get("output", [])
    parts: list[str] = []
    for item in outputs:
        for content in item.get("content", []):
            if content.get("type") == "output_text":
                parts.append(content.get("text", ""))
    return "".join(parts)


def strip_markdown_fences(text: str) -> str:
    stripped = text.strip()
    if stripped.startswith("```") and stripped.endswith("```"):
        lines = stripped.splitlines()
        if len(lines) >= 3:
            return "\n".join(lines[1:-1]).strip() + "\n"
    return text


def normalize_code_without_comments(text: str) -> str:
    result: list[str] = []
    i = 0
    n = len(text)
    in_line_comment = False
    in_block_comment = False
    in_string = False
    in_char = False

    while i < n:
        ch = text[i]
        nxt = text[i + 1] if i + 1 < n else ""

        if in_line_comment:
            if ch == "\n":
                in_line_comment = False
                result.append("\n")
            i += 1
            continue

        if in_block_comment:
            if ch == "*" and nxt == "/":
                in_block_comment = False
                i += 2
            else:
                if ch == "\n":
                    result.append("\n")
                i += 1
            continue

        if in_string:
            result.append(ch)
            if ch == "\\" and i + 1 < n:
                result.append(text[i + 1])
                i += 2
                continue
            if ch == '"':
                in_string = False
            i += 1
            continue

        if in_char:
            result.append(ch)
            if ch == "\\" and i + 1 < n:
                result.append(text[i + 1])
                i += 2
                continue
            if ch == "'":
                in_char = False
            i += 1
            continue

        if ch == "/" and nxt == "/":
            in_line_comment = True
            i += 2
            continue

        if ch == "/" and nxt == "*":
            in_block_comment = True
            i += 2
            continue

        if ch == '"':
            in_string = True
            result.append(ch)
            i += 1
            continue

        if ch == "'":
            in_char = True
            result.append(ch)
            i += 1
            continue

        result.append(ch)
        i += 1

    lines = [" ".join(line.split()) for line in "".join(result).splitlines()]
    return "\n".join(line for line in lines if line)


def code_changed(original: str, updated: str) -> bool:
    return normalize_code_without_comments(original) != normalize_code_without_comments(updated)


def call_openai(system_prompt: str, user_prompt: str, model: str, max_output_tokens: int) -> tuple[str, Usage]:
    api_key = os.getenv("OPENAI_API_KEY")
    if not api_key:
        raise RuntimeError("OPENAI_API_KEY is required")

    payload = {
        "model": model,
        "input": [
            {
                "role": "system",
                "content": [{"type": "input_text", "text": system_prompt}],
            },
            {
                "role": "user",
                "content": [{"type": "input_text", "text": user_prompt}],
            },
        ],
        "max_output_tokens": max_output_tokens,
    }

    request = urllib.request.Request(
        "https://api.openai.com/v1/responses",
        data=json.dumps(payload).encode("utf-8"),
        headers={
            "Content-Type": "application/json",
            "Authorization": f"Bearer {api_key}",
        },
        method="POST",
    )

    with urllib.request.urlopen(request, timeout=180) as response:
        data = json.loads(response.read().decode("utf-8"))

    text = strip_markdown_fences(extract_output_text(data))
    usage_data = data.get("usage", {}) or {}
    usage = Usage(
        input_tokens=int(usage_data.get("input_tokens") or 0),
        output_tokens=int(usage_data.get("output_tokens") or 0),
        total_tokens=int(usage_data.get("total_tokens") or 0),
    )
    return text, usage


def write_file(path: Path, text: str) -> None:
    normalized = text if text.endswith("\n") else f"{text}\n"
    path.write_text(normalized, encoding="utf-8", newline="\n")


def process_files(files: Iterable[Path], model: str, max_chars: int, max_output_tokens: int) -> int:
    docs_rules = load_text(DOCS_DIR / "README_Doxygen.md")
    header_template = load_text(DOCS_DIR / "template_H.h")
    source_template = load_text(DOCS_DIR / "template_C.c")
    main_template = load_text(DOCS_DIR / "template_Main.C")
    ai_context = load_text(AI_CONTEXT_PATH)

    total_usage = Usage()
    changed_count = 0

    for path in files:
        original = load_text(path)
        rel = path.relative_to(REPO_ROOT).as_posix()

        if len(original) > max_chars:
            print(f"Skipping {rel}: file exceeds MAX_CHARS_PER_FILE ({len(original)} > {max_chars})")
            continue

        system_prompt, user_prompt = build_prompt(
            path,
            original,
            docs_rules,
            header_template,
            source_template,
            main_template,
            ai_context,
        )

        print(f"Processing {rel} with model {model}")
        try:
            updated, usage = call_openai(system_prompt, user_prompt, model, max_output_tokens)
        except urllib.error.HTTPError as exc:
            body = exc.read().decode("utf-8", errors="replace")
            raise RuntimeError(f"OpenAI API request failed for {rel}: HTTP {exc.code} {body}") from exc
        except urllib.error.URLError as exc:
            raise RuntimeError(f"OpenAI API request failed for {rel}: {exc}") from exc

        total_usage.input_tokens += usage.input_tokens
        total_usage.output_tokens += usage.output_tokens
        total_usage.total_tokens += usage.total_tokens

        if not updated.strip():
            print(f"Skipping {rel}: model returned empty output")
            continue

        if code_changed(original, updated):
            raise RuntimeError(
                f"Rejected model output for {rel}: non-comment code changed. "
                "The prompt or model behavior needs review."
            )

        original_normalized = original.replace("\r\n", "\n")
        updated_normalized = updated.replace("\r\n", "\n")
        if original_normalized == updated_normalized:
            print(f"No documentation changes needed for {rel}")
            continue

        write_file(path, updated_normalized)
        changed_count += 1
        print(
            f"Updated {rel} "
            f"(input_tokens={usage.input_tokens}, output_tokens={usage.output_tokens}, total_tokens={usage.total_tokens})"
        )

    print(
        "Run summary: "
        f"files_updated={changed_count}, "
        f"input_tokens={total_usage.input_tokens}, "
        f"output_tokens={total_usage.output_tokens}, "
        f"total_tokens={total_usage.total_tokens}"
    )
    return changed_count


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--base", default=None)
    parser.add_argument("--head", default=None)
    parser.add_argument("--max-files", type=int, default=DEFAULT_MAX_FILES)
    parser.add_argument("--max-chars", type=int, default=DEFAULT_MAX_CHARS)
    parser.add_argument("--max-output-tokens", type=int, default=DEFAULT_MAX_OUTPUT_TOKENS)
    parser.add_argument("--model", default=DEFAULT_MODEL)
    args = parser.parse_args()

    base, head = resolve_diff_range(args.base, args.head)
    files = changed_files(base, head)

    if not files:
        print(f"No changed C/C++ files found in diff {base}..{head}")
        return 0

    limited_files = files[: args.max_files]
    if len(files) > args.max_files:
        print(f"Limiting run to first {args.max_files} files out of {len(files)} changed files")

    changed_count = process_files(
        limited_files,
        model=args.model,
        max_chars=args.max_chars,
        max_output_tokens=args.max_output_tokens,
    )
    return 0 if changed_count >= 0 else 1


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:  # noqa: BLE001
        print(f"ERROR: {exc}", file=sys.stderr)
        raise SystemExit(1)
