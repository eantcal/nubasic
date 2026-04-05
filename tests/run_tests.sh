#!/usr/bin/env bash
# run_tests.sh — nuBASIC regression test runner (Bash)
#
# Usage:
#   ./run_tests.sh [--interpreter <path>] [--testdir <path>]
#
# If --interpreter is not given, the script searches common build output
# locations relative to the repo root.
#
# Exit code: 0 = all tests passed, 1 = one or more failures.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

INTERPRETER=""
TEST_DIR="$SCRIPT_DIR"

# ── parse args ────────────────────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        --interpreter|-i) INTERPRETER="$2"; shift 2 ;;
        --testdir|-d)     TEST_DIR="$2";    shift 2 ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

# ── locate interpreter ────────────────────────────────────────────────────────
if [[ -z "$INTERPRETER" ]]; then
    CANDIDATES=(
        "$REPO_ROOT/build/release/Release/nubasic.exe"
        "$REPO_ROOT/build/release/nubasic"
        "$REPO_ROOT/build/debug/Debug/nubasic.exe"
        "$REPO_ROOT/build/debug/nubasic"
        "$REPO_ROOT/x64/Release/nubasic.exe"
        "$REPO_ROOT/x64/Debug/nubasic.exe"
    )
    for c in "${CANDIDATES[@]}"; do
        if [[ -x "$c" ]]; then INTERPRETER="$c"; break; fi
    done
fi

if [[ -z "$INTERPRETER" || ! -x "$INTERPRETER" ]]; then
    echo "ERROR: nubasic interpreter not found. Build the project first, or pass --interpreter <path>."
    exit 1
fi

# ── collect test files ────────────────────────────────────────────────────────
mapfile -t TEST_FILES < <(find "$TEST_DIR" -maxdepth 1 -name "test_*.bas" | sort)

if [[ ${#TEST_FILES[@]} -eq 0 ]]; then
    echo "ERROR: No test_*.bas files found in $TEST_DIR"
    exit 1
fi

# ── helpers ───────────────────────────────────────────────────────────────────
WIDTH=72
SEP=$(printf '%*s' "$WIDTH" '' | tr ' ' '-')
HDR=$(printf '%*s' "$WIDTH" '' | tr ' ' '=')

total_pass=0
total_fail=0
suite_pass=0
suite_fail=0
failed_suites=()

echo ""
echo "$HDR"
echo " nuBASIC Regression Test Suite"
echo " Interpreter : $INTERPRETER"
echo " Test dir    : $TEST_DIR"
echo "$HDR"

# ── run each test suite ───────────────────────────────────────────────────────
for f in "${TEST_FILES[@]}"; do
    name=$(basename "$f" .bas)

    echo ""
    echo "$SEP"
    echo " Suite: $name"
    echo "$SEP"

    # Run interpreter in text mode so I/O goes to stdout/stderr
    output=$("$INTERPRETER" -t -e "$f" 2>&1) || true
    exit_code=$?

    # Echo all output
    while IFS= read -r line; do echo "  $line"; done <<< "$output"

    # Parse "Results: N passed,  M failed"
    suite_p=0
    suite_f=0
    if results=$(echo "$output" | grep -oE "Results:[^0-9]*([0-9]+)[^0-9]+([0-9]+)" | head -1); then
        suite_p=$(echo "$results" | grep -oE "[0-9]+" | head -1)
        suite_f=$(echo "$results" | grep -oE "[0-9]+" | tail -1)
        # If both numbers are the same (1 number matched), reparse
        suite_p=$(echo "$output" | grep -oP "(?<=Results:\s{0,10})(\d+)(?=\s+passed)" | head -1 || echo 0)
        suite_f=$(echo "$output" | grep -oP "(\d+)(?=\s+failed)" | head -1 || echo 0)
    fi
    suite_p=${suite_p:-0}
    suite_f=${suite_f:-0}

    # Fallback: count PASS/FAIL lines
    if [[ "$suite_p" -eq 0 && "$suite_f" -eq 0 ]]; then
        suite_p=$(echo "$output" | grep -cE "^\s+PASS\b" || true)
        suite_f=$(echo "$output" | grep -cE "^\s+FAIL\b" || true)
    fi

    total_pass=$(( total_pass + suite_p ))
    total_fail=$(( total_fail + suite_f ))

    if [[ "$suite_f" -eq 0 && "$exit_code" -eq 0 ]]; then
        echo "  [SUITE PASS] $name  ($suite_p assertions passed)"
        suite_pass=$(( suite_pass + 1 ))
    else
        echo "  [SUITE FAIL] $name  ($suite_p passed, $suite_f failed, exit=$exit_code)"
        suite_fail=$(( suite_fail + 1 ))
        failed_suites+=("$name")
    fi
done

# ── final summary ─────────────────────────────────────────────────────────────
echo ""
echo "$HDR"
echo " SUMMARY"
echo "$HDR"
echo "  Suites  : $(( suite_pass + suite_fail )) total,  $suite_pass passed,  $suite_fail failed"
echo "  Asserts : $(( total_pass + total_fail )) total,  $total_pass passed,  $total_fail failed"
if [[ ${#failed_suites[@]} -gt 0 ]]; then
    echo "  Failed suites:"
    for s in "${failed_suites[@]}"; do echo "    - $s"; done
fi
echo "$HDR"

if [[ "$total_fail" -eq 0 && "$suite_fail" -eq 0 ]]; then
    echo " ALL TESTS PASSED"
    echo "$HDR"
    exit 0
else
    echo " SOME TESTS FAILED"
    echo "$HDR"
    exit 1
fi
