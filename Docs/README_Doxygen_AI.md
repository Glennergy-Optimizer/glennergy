# Doxygen AI Setup

This document explains the current GitHub setup and where to look for the active rules and workflow usage notes.

## Main Files

- GitHub Actions workflow: [`.github/workflows/doxygen-openai.yml`](/c:/Users/Axel/Documents/Skola2025ChasAcademy/glennergy/.github/workflows/doxygen-openai.yml)
- Update script: [`tools/doxygen_ai/update_docs.py`](/c:/Users/Axel/Documents/Skola2025ChasAcademy/glennergy/tools/doxygen_ai/update_docs.py)
- Workflow usage guide: [`tools/doxygen_ai/README.md`](/c:/Users/Axel/Documents/Skola2025ChasAcademy/glennergy/tools/doxygen_ai/README.md)
- Active documentation standard: [`Docs/README_Doxygen.md`](/c:/Users/Axel/Documents/Skola2025ChasAcademy/glennergy/Docs/README_Doxygen.md)
- Future follow-ups and open questions: [`Docs/README_Doxygen_TODO.md`](/c:/Users/Axel/Documents/Skola2025ChasAcademy/glennergy/Docs/README_Doxygen_TODO.md)

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

## Recommended Usage

- use [`tools/doxygen_ai/README.md`](/c:/Users/Axel/Documents/Skola2025ChasAcademy/glennergy/tools/doxygen_ai/README.md) for day-to-day workflow usage
- use [`Docs/README_Doxygen.md`](/c:/Users/Axel/Documents/Skola2025ChasAcademy/glennergy/Docs/README_Doxygen.md) as the source of truth for documentation style
- use [`Docs/README_Doxygen_TODO.md`](/c:/Users/Axel/Documents/Skola2025ChasAcademy/glennergy/Docs/README_Doxygen_TODO.md) for active concerns and future improvements

## Budget Notes

- the default budget-oriented model is still `gpt-5.4-mini`
- stronger models can be selected manually for comparison or quality-sensitive runs
- current workflow cost summaries are estimate-only and do not yet auto-switch pricing by selected model
