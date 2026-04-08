# Doxygen AI Setup

This document explains the current GitHub setup and where to look for the active rules and workflow usage notes.

## Main Files

- GitHub Actions workflow: [`.github/workflows/doxygen-openai.yml`](/c:/Users/Axel/Documents/Skola2025ChasAcademy/glennergy/.github/workflows/doxygen-openai.yml)
- Update script: [`tools/doxygen_ai/update_docs.py`](/c:/Users/Axel/Documents/Skola2025ChasAcademy/glennergy/tools/doxygen_ai/update_docs.py)
- Workflow usage guide: [`tools/doxygen_ai/README.md`](/c:/Users/Axel/Documents/Skola2025ChasAcademy/glennergy/tools/doxygen_ai/README.md)
- Active documentation standard: [`Docs/Doxygen_Standard.md`](/c:/Users/Axel/Documents/Skola2025ChasAcademy/glennergy/Docs/Doxygen_Standard.md)
- Future follow-ups and open questions: [`Docs/Doxygen_TODO.md`](/c:/Users/Axel/Documents/Skola2025ChasAcademy/glennergy/Docs/Doxygen_TODO.md)

## First-Time Setup

1. In GitHub, open `Settings -> Secrets and variables -> Actions`
2. Add the repository secret:
   - `OPENAI_API_KEY`
3. Make sure the workflow file in `main` is up to date if you expect new manual-run inputs to appear in the GitHub UI
4. Push or manually run the workflow against `ny-dev`

## Current Safety Controls

- automatic runs are restricted to the development flow you have set up
- manual runs support explicit file selection
- only C/C++ source and header files are processed
- matching `.h` and `.c/.cpp` files are paired automatically
- generated output is rejected if non-comment code changed
- rejected outputs and run manifests are saved as artifacts

## How The Workflow Runs

At a high level:

1. GitHub shows the workflow UI from the default branch, usually `main`
2. The run checks out the selected `target_ref`, usually `ny-dev`
3. The script selects changed files or manually specified files
4. Matching header/source pairs are added automatically when names match
5. The script loads the active documentation standard and templates
6. The selected model generates full-file documentation updates
7. Output is rejected if non-comment code changed
8. Successful files are written to the workflow branch and summarized in GitHub Actions
9. Rejected outputs and manifests are uploaded as artifacts for troubleshooting or reruns

## Manual Run Examples

### Normal run

Use:

- `target_ref = ny-dev`
- `model = gpt-5.4-mini` or another allowed model
- `rerun_mode = all`
- `file_paths = API/Spotpris/Spotpris.h, API/Spotpris/Spotpris.c, API/Spotpris/spotpristest.c`

This processes the selected files and creates or updates the draft PR branch with successful results.

### Rerun only remaining files

Use this after an earlier run where one or more files were rejected or deferred.

Use:

- same `target_ref`
- same `model` unless you intentionally want to compare models
- same `test_label` if you used one earlier
- `rerun_mode = remaining_only`
- leave `file_paths` empty

This tells the workflow to restore earlier successful files and process only the remaining rejected or deferred files.

Do not use GitHub's built-in `Re-run failed jobs` button for this purpose. That repeats the old job and does not trigger the workflow's custom remaining-only logic.

## Recommended Usage

- use [`tools/doxygen_ai/README.md`](/c:/Users/Axel/Documents/Skola2025ChasAcademy/glennergy/tools/doxygen_ai/README.md) for day-to-day workflow usage
- use [`Docs/Doxygen_Standard.md`](/c:/Users/Axel/Documents/Skola2025ChasAcademy/glennergy/Docs/Doxygen_Standard.md) as the source of truth for documentation style
- use [`Docs/Doxygen_TODO.md`](/c:/Users/Axel/Documents/Skola2025ChasAcademy/glennergy/Docs/Doxygen_TODO.md) for active concerns and future improvements

## Budget Notes

- the default budget-oriented model is still `gpt-5.4-mini`
- stronger models can be selected manually for comparison or quality-sensitive runs
- current workflow cost summaries are estimate-only and do not yet auto-switch pricing by selected model
