# Doxygen AI Workflow

This folder contains the first test version of an OpenAI-powered Doxygen updater.

## What it does

- Looks at changed C/C++ files in the current Git diff
- Detects whether a file already appears to have Doxygen documentation
- Uses the repository's Doxygen rules and templates as prompt context
- Asks the OpenAI API to return the full updated file
- Rejects the result if non-comment code changed
- Commits the documentation update from GitHub Actions

## Current safety limits

- Restricted to the `ny-dev` branch in GitHub Actions
- Manual trigger available through `workflow_dispatch`
- Only processes the first 2 changed C/C++ files per run
- Skips files larger than 18,000 characters

## Required GitHub secret

Add this repository secret before enabling the workflow:

- `OPENAI_API_KEY`

## Recommended first tests

1. Push a small documentation-only code change to `ny-dev`.
2. Check the workflow logs for:
   - which files were selected
   - token usage per file
   - whether a commit was created
3. Review the generated comments carefully.
4. Tune prompts and limits before enabling more files or more branches.

## Rerunning failed files

Use the GitHub `Run workflow` form with:

- `rerun_mode = all` for a normal run
- `rerun_mode = remaining_only` to retry only previously rejected or deferred files

Do not use GitHub's built-in `Re-run failed jobs` button for this purpose. That reruns the original job configuration and does not trigger the workflow's custom `remaining_only` logic.

## Useful local test command

From the repository root:

```bash
python tools/doxygen_ai/update_docs.py --base HEAD~1 --head HEAD
```

Set `OPENAI_API_KEY` in your shell before running locally.
