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
REJECTED_OUTPUT_DIR = REPO_ROOT / "tools" / "doxygen_ai" / "rejected"
ALLOWED_SUFFIXES = {".c", ".h", ".cpp", ".hpp", ".cc", ".hh"}
HEADER_SUFFIXES = {".h", ".hpp", ".hh"}
SOURCE_SUFFIXES = {".c", ".cpp", ".cc"}
DEFAULT_MODEL = os.getenv("OPENAI_MODEL", "gpt-5.4-mini")
DEFAULT_MAX_FILES = int(os.getenv("MAX_FILES_PER_RUN", "2"))
DEFAULT_MAX_CHARS = int(os.getenv("MAX_CHARS_PER_FILE", "18000"))
DEFAULT_MAX_OUTPUT_TOKENS = int(os.getenv("OPENAI_MAX_OUTPUT_TOKENS", "12000"))
DEFAULT_INPUT_COST_PER_MILLION = float(os.getenv("OPENAI_INPUT_COST_PER_MILLION", "0.25"))
DEFAULT_OUTPUT_COST_PER_MILLION = float(os.getenv("OPENAI_OUTPUT_COST_PER_MILLION", "2.00"))
DEFAULT_USD_TO_SEK = float(os.getenv("OPENAI_USD_TO_SEK", "9.19981"))
DEFAULT_TEST_LABEL = os.getenv("DOXYGEN_AI_TEST_LABEL", "").strip()


@dataclass
class Usage:
    input_tokens: int = 0
    output_tokens: int = 0
    total_tokens: int = 0


@dataclass
class OpenAIResult:
    text: str
    usage: Usage
    model: str


@dataclass
class FileResult:
    path: str
    status: str
    model: str = ""
    input_tokens: int = 0
    output_tokens: int = 0
    total_tokens: int = 0
    estimated_cost_usd: float = 0.0
    estimated_cost_sek: float = 0.0
    details: str = ""


def estimate_cost_usd(usage: Usage, input_cost_per_million: float, output_cost_per_million: float) -> float:
    return (
        (usage.input_tokens / 1_000_000) * input_cost_per_million
        + (usage.output_tokens / 1_000_000) * output_cost_per_million
    )


def convert_usd_to_sek(usd_amount: float, usd_to_sek: float) -> float:
    return usd_amount * usd_to_sek


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


def parse_explicit_files(file_paths_raw: str) -> list[Path]:
    files: list[Path] = []
    seen: set[Path] = set()

    for part in file_paths_raw.split(","):
        raw = part.strip()
        if not raw:
            continue

        rel = Path(raw)
        abs_path = (REPO_ROOT / rel).resolve()

        try:
            abs_path.relative_to(REPO_ROOT.resolve())
        except ValueError as exc:
            raise RuntimeError(f"Explicit file path is outside the repository: {raw}") from exc

        if not abs_path.exists():
            raise RuntimeError(f"Explicit file path does not exist: {raw}")

        if abs_path.suffix.lower() not in ALLOWED_SUFFIXES:
            raise RuntimeError(f"Explicit file path has unsupported extension: {raw}")

        if abs_path not in seen:
            seen.add(abs_path)
            files.append(abs_path)

    return files


def expand_module_files(files: list[Path]) -> list[Path]:
    expanded: list[Path] = []
    seen: set[Path] = set()

    for path in files:
        if path not in seen:
            seen.add(path)
            expanded.append(path)

        paired_file = find_paired_file(path)
        if paired_file is not None and paired_file not in seen:
            seen.add(paired_file)
            expanded.append(paired_file)

    return expanded


def has_doxygen(text: str) -> bool:
    return bool(re.search(r"/\*\*[\s\S]*?@(file|brief|param|return|defgroup|ingroup)\b", text))


def is_entrypoint_or_test(path: Path, text: str) -> bool:
    lowered = path.name.lower()
    return lowered.startswith("main.") or "int main(" in text or "int main(void" in text


def find_paired_file(path: Path) -> Path | None:
    if path.suffix.lower() in HEADER_SUFFIXES:
        candidate_suffixes = SOURCE_SUFFIXES
    elif path.suffix.lower() in SOURCE_SUFFIXES:
        candidate_suffixes = HEADER_SUFFIXES
    else:
        return None

    for suffix in candidate_suffixes:
        candidate = path.with_suffix(suffix)
        if candidate.exists():
            return candidate
    return None


def build_prompt(
    path: Path,
    code: str,
    docs_rules: str,
    header_template: str,
    source_template: str,
    main_template: str,
    ai_context: str,
    paired_file_context: str,
) -> tuple[str, str]:
    file_kind = "header" if path.suffix.lower() in HEADER_SUFFIXES else "source"
    mode = "update_existing_docs" if has_doxygen(code) else "document_full_file"
    template = header_template if file_kind == "header" else main_template if is_entrypoint_or_test(path, code) else source_template
    source_file_guidance = ""

    if file_kind == "source" and not is_entrypoint_or_test(path, code):
        source_file_guidance = """Additional source-file guidance:
- Keep public function documentation in source files brief when the paired header already documents the public API contract.
- For public non-static functions in source files, the default style should be brief-only documentation.
- Preferred pattern for public non-static functions in source files:
  /**
   * @brief Implementation of <function name>.
   *
   * See header for full contract documentation.
   */
- Do not repeat @param, @return, @note, @warning, @pre, or @post blocks for public source-file functions unless they describe implementation-specific behavior that is not already documented in the header.
- Repeating header-level public API contract details in source files is undesirable output.
- Internal or static helper functions may include additional tags, but only when they add meaningful, non-obvious information.
"""

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

Paired module file context:
{paired_file_context}

{source_file_guidance}
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


def call_openai(system_prompt: str, user_prompt: str, model: str, max_output_tokens: int) -> OpenAIResult:
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
    actual_model = str(data.get("model") or model)
    return OpenAIResult(text=text, usage=usage, model=actual_model)


def write_file(path: Path, text: str) -> None:
    normalized = text if text.endswith("\n") else f"{text}\n"
    path.write_text(normalized, encoding="utf-8", newline="\n")


def write_rejected_output(path: Path, text: str) -> Path:
    REJECTED_OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    relative_name = path.relative_to(REPO_ROOT).as_posix().replace("/", "__")
    output_path = REJECTED_OUTPUT_DIR / f"{relative_name}.rejected.txt"
    output_path.write_text(text if text.endswith("\n") else f"{text}\n", encoding="utf-8", newline="\n")
    return output_path


def append_github_summary(
    file_results: list[FileResult],
    total_usage: Usage,
    total_estimated_cost_usd: float,
    total_estimated_cost_sek: float,
    requested_model: str,
    test_label: str,
) -> None:
    summary_path = os.getenv("GITHUB_STEP_SUMMARY", "").strip()
    if not summary_path:
        return

    updated_count = sum(1 for result in file_results if result.status == "updated")
    no_change_count = sum(1 for result in file_results if result.status == "no_change")
    skipped_count = sum(1 for result in file_results if result.status == "skipped")
    rejected_count = sum(1 for result in file_results if result.status == "rejected")

    lines = [
        "## Doxygen AI Summary",
        "",
        f"- Requested model: {requested_model}",
        f"- Test label: {test_label or '(none)'}",
        f"- Files considered: {len(file_results)}",
        f"- Files updated: {updated_count}",
        f"- Files unchanged: {no_change_count}",
        f"- Files skipped: {skipped_count}",
        f"- Files rejected: {rejected_count}",
        f"- Input tokens: {total_usage.input_tokens}",
        f"- Output tokens: {total_usage.output_tokens}",
        f"- Total tokens: {total_usage.total_tokens}",
        f"- Estimated cost (USD): ${total_estimated_cost_usd:.6f}",
        f"- Estimated cost (SEK): {total_estimated_cost_sek:.6f} kr",
        "",
        "| File | Status | Model | Input | Output | Total | USD | SEK | Details |",
        "| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | --- |",
    ]

    for result in file_results:
        lines.append(
            f"| {result.path} | {result.status} | {result.model or '-'} | "
            f"{result.input_tokens} | {result.output_tokens} | {result.total_tokens} | "
            f"${result.estimated_cost_usd:.6f} | {result.estimated_cost_sek:.6f} kr | {result.details or '-'} |"
        )

    with open(summary_path, "a", encoding="utf-8", newline="\n") as handle:
        handle.write("\n".join(lines) + "\n")


def process_files(
    files: Iterable[Path],
    model: str,
    max_chars: int,
    max_output_tokens: int,
    input_cost_per_million: float,
    output_cost_per_million: float,
    usd_to_sek: float,
) -> int:
    docs_rules = load_text(DOCS_DIR / "README_Doxygen.md")
    header_template = load_text(DOCS_DIR / "template_H.h")
    source_template = load_text(DOCS_DIR / "template_C.c")
    main_template = load_text(DOCS_DIR / "template_Main.C")
    ai_context = load_text(AI_CONTEXT_PATH)

    total_usage = Usage()
    changed_count = 0
    file_results: list[FileResult] = []
    rejection_messages: list[str] = []

    for path in files:
        original = load_text(path)
        rel = path.relative_to(REPO_ROOT).as_posix()
        paired_file = find_paired_file(path)
        paired_file_context = "No paired header/source file was found."

        if len(original) > max_chars:
            print(f"Skipping {rel}: file exceeds MAX_CHARS_PER_FILE ({len(original)} > {max_chars})")
            file_results.append(FileResult(path=rel, status="skipped", details="File exceeds MAX_CHARS_PER_FILE"))
            continue

        if paired_file is not None:
            paired_text = load_text(paired_file)
            paired_rel = paired_file.relative_to(REPO_ROOT).as_posix()
            if len(paired_text) <= max_chars:
                paired_file_context = (
                    f"Use this only as supporting module context. Do not modify it.\n"
                    f"Paired path: {paired_rel}\n"
                    f"Paired file contents:\n{paired_text}"
                )
            else:
                paired_file_context = (
                    f"Paired path: {paired_rel}\n"
                    f"Paired file exists but was omitted because it exceeds MAX_CHARS_PER_FILE."
                )

        system_prompt, user_prompt = build_prompt(
            path,
            original,
            docs_rules,
            header_template,
            source_template,
            main_template,
            ai_context,
            paired_file_context,
        )

        print(f"Processing {rel} with model {model}")
        try:
            result = call_openai(system_prompt, user_prompt, model, max_output_tokens)
        except urllib.error.HTTPError as exc:
            body = exc.read().decode("utf-8", errors="replace")
            file_results.append(FileResult(path=rel, status="rejected", details=f"HTTP {exc.code}"))
            rejection_messages.append(f"{rel}: OpenAI API request failed with HTTP {exc.code}: {body}")
            print(f"Rejected {rel}: OpenAI API request failed with HTTP {exc.code}")
            continue
        except urllib.error.URLError as exc:
            file_results.append(FileResult(path=rel, status="rejected", details="Network/API request error"))
            rejection_messages.append(f"{rel}: OpenAI API request failed: {exc}")
            print(f"Rejected {rel}: OpenAI API request failed: {exc}")
            continue

        updated = result.text
        usage = result.usage
        actual_model = result.model

        total_usage.input_tokens += usage.input_tokens
        total_usage.output_tokens += usage.output_tokens
        total_usage.total_tokens += usage.total_tokens

        if not updated.strip():
            print(f"Skipping {rel}: model returned empty output")
            file_results.append(FileResult(path=rel, status="skipped", model=actual_model, details="Model returned empty output"))
            continue

        if code_changed(original, updated):
            rejected_path = write_rejected_output(path, updated)
            details = f"Non-comment code changed; saved to {rejected_path.relative_to(REPO_ROOT).as_posix()}"
            file_results.append(
                FileResult(
                    path=rel,
                    status="rejected",
                    model=actual_model,
                    input_tokens=usage.input_tokens,
                    output_tokens=usage.output_tokens,
                    total_tokens=usage.total_tokens,
                    details=details,
                )
            )
            rejection_messages.append(f"{rel}: {details}")
            print(f"Rejected {rel}: {details}")
            continue

        original_normalized = original.replace("\r\n", "\n")
        updated_normalized = updated.replace("\r\n", "\n")
        if original_normalized == updated_normalized:
            print(f"No documentation changes needed for {rel}")
            file_results.append(
                FileResult(
                    path=rel,
                    status="no_change",
                    model=actual_model,
                    input_tokens=usage.input_tokens,
                    output_tokens=usage.output_tokens,
                    total_tokens=usage.total_tokens,
                    details="No changes were necessary",
                )
            )
            continue

        write_file(path, updated_normalized)
        changed_count += 1
        estimated_cost = estimate_cost_usd(usage, input_cost_per_million, output_cost_per_million)
        estimated_cost_sek = convert_usd_to_sek(estimated_cost, usd_to_sek)
        file_results.append(
            FileResult(
                path=rel,
                status="updated",
                model=actual_model,
                input_tokens=usage.input_tokens,
                output_tokens=usage.output_tokens,
                total_tokens=usage.total_tokens,
                estimated_cost_usd=estimated_cost,
                estimated_cost_sek=estimated_cost_sek,
                details="Documentation updated",
            )
        )
        print(
            f"Updated {rel} "
            f"(model={actual_model}, input_tokens={usage.input_tokens}, output_tokens={usage.output_tokens}, "
            f"total_tokens={usage.total_tokens}, estimated_cost_usd=${estimated_cost:.6f}, "
            f"estimated_cost_sek={estimated_cost_sek:.6f} kr)"
        )

    total_estimated_cost = estimate_cost_usd(
        total_usage,
        input_cost_per_million=input_cost_per_million,
        output_cost_per_million=output_cost_per_million,
    )
    total_estimated_cost_sek = convert_usd_to_sek(total_estimated_cost, usd_to_sek)
    print(
        "Run summary: "
        f"files_updated={changed_count}, "
        f"input_tokens={total_usage.input_tokens}, "
        f"output_tokens={total_usage.output_tokens}, "
        f"total_tokens={total_usage.total_tokens}, "
        f"estimated_cost_usd=${total_estimated_cost:.6f}, "
        f"estimated_cost_sek={total_estimated_cost_sek:.6f} kr"
    )
    append_github_summary(
        file_results=file_results,
        total_usage=total_usage,
        total_estimated_cost_usd=total_estimated_cost,
        total_estimated_cost_sek=total_estimated_cost_sek,
        requested_model=model,
        test_label=os.getenv("DOXYGEN_AI_TEST_LABEL", "").strip(),
    )

    if rejection_messages:
        raise RuntimeError(
            "One or more files were rejected during documentation generation:\n"
            + "\n".join(rejection_messages)
        )
    return changed_count


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--base", default=None)
    parser.add_argument("--head", default=None)
    parser.add_argument("--files", default="")
    parser.add_argument("--max-files", type=int, default=DEFAULT_MAX_FILES)
    parser.add_argument("--max-chars", type=int, default=DEFAULT_MAX_CHARS)
    parser.add_argument("--max-output-tokens", type=int, default=DEFAULT_MAX_OUTPUT_TOKENS)
    parser.add_argument("--model", default=DEFAULT_MODEL)
    parser.add_argument("--test-label", default=DEFAULT_TEST_LABEL)
    parser.add_argument("--input-cost-per-million", type=float, default=DEFAULT_INPUT_COST_PER_MILLION)
    parser.add_argument("--output-cost-per-million", type=float, default=DEFAULT_OUTPUT_COST_PER_MILLION)
    parser.add_argument("--usd-to-sek", type=float, default=DEFAULT_USD_TO_SEK)
    args = parser.parse_args()
    os.environ["DOXYGEN_AI_TEST_LABEL"] = args.test_label.strip()

    explicit_files = args.files.strip()
    if explicit_files:
        files = parse_explicit_files(explicit_files)
        files = expand_module_files(files)
        print(f"Manual file selection enabled for {len(files)} file(s) after module expansion")
    else:
        base, head = resolve_diff_range(args.base, args.head)
        files = changed_files(base, head)

    if not files:
        if explicit_files:
            print("No valid explicit files were provided")
        else:
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
        input_cost_per_million=args.input_cost_per_million,
        output_cost_per_million=args.output_cost_per_million,
        usd_to_sek=args.usd_to_sek,
    )
    return 0 if changed_count >= 0 else 1


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:  # noqa: BLE001
        print(f"ERROR: {exc}", file=sys.stderr)
        raise SystemExit(1)
