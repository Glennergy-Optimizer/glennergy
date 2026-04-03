# Doxygen AI Setup

This document explains how to test the OpenAI-powered Doxygen updater in GitHub Actions with low cost and low risk.

## What is included

- GitHub Actions workflow: `.github/workflows/doxygen-openai.yml`
- Update script: `tools/doxygen_ai/update_docs.py`
- Tool-specific notes: `tools/doxygen_ai/README.md`

## First-time GitHub setup

1. Push these files to GitHub on your test branch.
2. In your GitHub repository, go to:
   `Settings -> Secrets and variables -> Actions`
3. Add a new repository secret:
   - Name: `OPENAI_API_KEY`
   - Value: your OpenAI API key
4. Push a small change to a `.c`, `.h`, `.cpp`, or `.hpp` file on `ny-dev`.
5. Open the `Actions` tab and inspect the `Doxygen AI` workflow run.

## Why this is safe for testing

- Only the `ny-dev` branch triggers automatically
- You can also run it manually with `workflow_dispatch`
- Only the first 2 changed C/C++ files are processed per run
- Files over 18,000 characters are skipped
- The script rejects model output if non-comment code changes are detected

## How to keep costs low

Recommended starting settings are already in the workflow:

- `OPENAI_MODEL: gpt-5.4-mini`
- `MAX_FILES_PER_RUN: 2`
- `MAX_CHARS_PER_FILE: 18000`

You can make testing even cheaper by temporarily changing:

- `MAX_FILES_PER_RUN` from `2` to `1`
- `OPENAI_MAX_OUTPUT_TOKENS` from `12000` to something lower if your files are small

## OpenAI usage control

For stronger budget control, create a dedicated OpenAI project for this workflow and set a project budget in the OpenAI platform. That keeps this experiment isolated from any future API usage.

## Suggested rollout path

1. Test on `ny-dev` only
2. Review the generated comments manually
3. Tune the prompts or limits
4. Increase `MAX_FILES_PER_RUN`
5. Add more branches or pull request triggers later
