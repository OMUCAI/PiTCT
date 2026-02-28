# PiTCT — Agent Guide

## Project Overview

PiTCT is a Python binding for TCT, a supervisory control synthesis tool for untimed discrete-event systems (DES).
It calls the C/C++ core (`libtct/`) via CFFI, making TCT functionality available from Python.

- Package name: `pitct`
- Python source code is in `pitct/`, and the C library is in `libtct/`.
- Tests are located in `tests/` and run with `pytest`.

## Virtual Environment

Always use a virtual environment (`venv`) for development and testing.
The expected location is `venv/` in the project root.

```bash
# Create virtual environment (first time only)
python -m venv venv

# Activate (Linux / macOS)
source venv/bin/activate

# Install dependencies
pip install -e ".[dev]"
```

Before running any command, confirm that the virtual environment is active (`which python` should point to `venv/bin/python`).
If it is not active, run `source venv/bin/activate` first.

## Language

All source code edits must be written in English. This includes comments, docstrings, variable names, commit messages, and any other text embedded in the codebase.

Responses to the user must be written in the same language as the user's question.
