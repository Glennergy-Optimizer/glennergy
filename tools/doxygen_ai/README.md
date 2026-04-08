# Doxygen AI Workflow

This folder contains the OpenAI-powered Doxygen updater used by the GitHub Actions workflow.

## What It Does

- selects changed or manually chosen C/C++ files
- expands matching `.h` and `.c/.cpp` pairs automatically
- loads the repository Doxygen standard and target-style examples
- asks the OpenAI API to return the full updated file
- rejects output if non-comment code changed
- writes a GitHub Actions summary with token usage and estimated cost
- uploads rejected outputs and run manifests as workflow artifacts
- creates or updates a draft PR branch with successful documentation changes

## Current Behavior

- GitHub workflow UI is defined from the default branch, currently `main`
- workflow runs can still check out and process `ny-dev`
- default everyday model is `gpt-5.4-mini`
- manual file selection supports comma-separated paths
- selecting `Module.h` also processes the matching `Module.c` or `Module.cpp`, and vice versa
- a run can continue processing files even if one later file is rejected
- reruns can target only previously rejected or deferred files

## Manual Workflow Inputs

Use the `Run workflow` form in GitHub Actions.

| Input | Purpose |
| --- | --- |
| `target_ref` | Branch to check out and process, usually `ny-dev` |
| `model` | OpenAI model to use for the run |
| `test_label` | Optional label for comparison runs and separate PR branches |
| `rerun_mode` | `all` for a normal run, `remaining_only` for rejected/deferred files from an earlier run |
| `file_paths` | Comma-separated file paths for `all` mode; leave empty for `remaining_only` |

## Recommended Manual Flow

### Normal successful run

1. Open `Actions -> Doxygen AI -> Run workflow`
2. Set:
   - `target_ref = ny-dev`
   - `model = gpt-5.4-mini` or another allowed model
   - `rerun_mode = all`
   - `file_paths = API/Spotpris/Spotpris.h, API/Spotpris/Spotpris.c, API/Spotpris/spotpristest.c`
3. Start the run
4. Review:
   - the `Summary` tab
   - any draft PR that was created or updated
   - any rejected-output artifact if the run failed

### Partial failure, then rerun only remaining files

Example:
- first run processes 3 files
- 2 files succeed
- 1 file is rejected or deferred

Then:
1. Fix the prompt/standard/workflow if needed
2. Push the fix
3. Start a new manual run
4. Set:
   - `target_ref = ny-dev`
   - same `model` as before unless you intentionally want to compare models
   - same `test_label` if you used one before
   - `rerun_mode = remaining_only`
   - leave `file_paths` empty
5. Review whether the rerun restored the already successful files and processed only the remaining files

Important:
- use the workflow's manual `remaining_only` mode for this
- do not use GitHub's built-in `Re-run failed jobs` button for this purpose
- GitHub's rerun button repeats the old job; it does not use the workflow's custom rerun logic

## What To Check In A Run

- whether a file was `updated`, `no_change`, `skipped`, or `rejected`
- whether the generated docs match the repository standard
- whether code-token safety was preserved
- input, output, and total tokens
- estimated USD and SEK cost
- whether the draft PR branch was created or updated as expected

## Common Notes

- If the workflow form is missing a new input, the workflow YAML probably has not been merged to `main` yet.
- If a model was just enabled in the OpenAI project, access may take a little time to propagate.
- Current cost summaries are estimate-only and still use a single configured pricing baseline.
- The actual selected model is shown in the run summary.

## Local Test Command

From the repository root:

```bash
python tools/doxygen_ai/update_docs.py --base HEAD~1 --head HEAD
```

Set `OPENAI_API_KEY` in your shell before running locally.
