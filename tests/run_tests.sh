#!/usr/bin/env bash
# run_tests.sh — nuBASIC regression test runner
#
# Usage:
#   ./run_tests.sh [--interpreter <path>] [--testdir <path>]
#
# Test-file meta-comments (first 5 lines of each .bas file):
#   ' ARGS: arg1 arg2   — extra CLI args passed to the interpreter
#   ' OUTFILE: name.txt — read test output from this file instead of stdout
#   ' SKIP               — skip this test with a note
#
#   ' EXPECT_ERROR: text -- output must contain text
#   ' EXPECT_OUTPUT: a|b -- output must contain all pipe-separated fragments
#   ' EXPECT_NOT_OUTPUT: a|b -- output must not contain these fragments
#   ' COMMANDS: name.txt -- feed commands to the interactive interpreter
#
# Exit code: 0 = all passed, 1 = one or more failures.

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

INTERPRETER=""
TEST_DIR="$SCRIPT_DIR"

# ── parse CLI args ────────────────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        --interpreter|-i) INTERPRETER="$2"; shift 2 ;;
        --testdir|-d)     TEST_DIR="$2";    shift 2 ;;
        *) echo "Unknown option: $1" >&2; exit 1 ;;
    esac
done

# ── locate interpreter ────────────────────────────────────────────────────────
if [[ -z "$INTERPRETER" ]]; then
    CANDIDATES=(
        # Visual Studio cmake layout — debug build dir (most recently built)
        "$REPO_ROOT/build/debug/cli/win/Debug/nubasic.exe"
        "$REPO_ROOT/build/debug/cli/win/Release/nubasic.exe"
        # Visual Studio cmake layout — release build dir
        "$REPO_ROOT/build/release/cli/win/Release/nubasic.exe"
        "$REPO_ROOT/build/release/cli/win/Debug/nubasic.exe"
        # Ninja / single-config cmake layout
        "$REPO_ROOT/build/debug/cli/win/nubasic.exe"
        "$REPO_ROOT/build/release/cli/win/nubasic.exe"
        # Generic cmake build roots
        "$REPO_ROOT/build/nubasic"
        "$REPO_ROOT/build/Release/nubasic"
        "$REPO_ROOT/build/Debug/nubasic"
        "$REPO_ROOT/build/release/nubasic"
        "$REPO_ROOT/build/release/Release/nubasic"
        "$REPO_ROOT/build/release/Release/nubasic.exe"
        "$REPO_ROOT/build/debug/Debug/nubasic.exe"
        "$REPO_ROOT/build/debug/nubasic"
        "$REPO_ROOT/x64/Release/nubasic.exe"
        "$REPO_ROOT/x64/Debug/nubasic.exe"
    )
    for c in "${CANDIDATES[@]}"; do
        if [[ -x "$c" ]]; then INTERPRETER="$c"; break; fi
    done

    # Last resort: search anywhere under the repo tree
    if [[ -z "$INTERPRETER" ]]; then
        INTERPRETER=$(find "$REPO_ROOT" -type f \( -name "nubasic" -o -name "nubasic.exe" \) \
                      ! -path "*/CMakeFiles/*" 2>/dev/null | head -1)
    fi
fi

if [[ -z "$INTERPRETER" || ! -x "$INTERPRETER" ]]; then
    echo "ERROR: nubasic not found. Build first, or use --interpreter <path>." >&2
    exit 1
fi

# ── helpers ───────────────────────────────────────────────────────────────────

# Strip ANSI/VT escape sequences (BSD sed compatible)
strip_ansi() {
    sed $'s/\x1b\\[[0-9;]*[a-zA-Z]//g'
}

# Run the interpreter; return exit code via global RUN_EXIT.
# Prints cleaned output to stdout.
RUN_EXIT=0
run_interp() {
    local bas_file="$1"; shift
    local raw
    RUN_EXIT=0
    raw=$(TERM=xterm "$INTERPRETER" -t -e "$bas_file" "$@" 2>&1) || RUN_EXIT=$?
    printf '%s' "$raw" | strip_ansi
}

run_commands() {
    local cmd_file="$1"
    local raw
    RUN_EXIT=0
    raw=$(TERM=xterm "$INTERPRETER" -t < "$cmd_file" 2>&1) || RUN_EXIT=$?
    printf '%s' "$raw" | strip_ansi
}

# Count occurrences of a regex in a string; sets COUNT (never fails).
count_matches() {
    local text="$1" pattern="$2"
    COUNT=$(printf '%s' "$text" | grep -cE "$pattern" 2>/dev/null) || COUNT=0
}

# Extract first integer before/after a keyword; sets N (never fails).
extract_int_before() {
    local text="$1" keyword="$2"
    N=$(printf '%s' "$text" | grep -oE "[0-9]+ $keyword" | grep -oE "^[0-9]+" | tail -1) || true
    N=${N:-0}
}

# Read a meta-comment "' KEY: value" or "' KEY" from first 5 lines of a file.
read_meta() {
    local file="$1" key="$2"
    head -5 "$file" 2>/dev/null \
        | grep -i "^[[:space:]]*'[[:space:]]*${key}" \
        | head -1 \
        | sed "s/^[[:space:]]*'[[:space:]]*${key}[[:space:]]*:*[[:space:]]*//"
}

contains_ci() {
    local haystack="$1" needle="$2"
    local had_nocasematch=0

    if shopt -q nocasematch; then
        had_nocasematch=1
    fi

    shopt -s nocasematch
    [[ "$haystack" == *"$needle"* ]]
    local res=$?

    if [[ "$had_nocasematch" -eq 0 ]]; then
        shopt -u nocasematch
    fi

    return "$res"
}

# ── collect test files (bash 3 compatible) ───────────────────────────────────
TEST_FILES=()
while IFS= read -r f; do
    TEST_FILES+=("$f")
done < <(find "$TEST_DIR" -maxdepth 1 -name "test_*.bas" 2>/dev/null | sort)

if [[ ${#TEST_FILES[@]} -eq 0 ]]; then
    echo "ERROR: No test_*.bas files found in $TEST_DIR" >&2
    exit 1
fi

# ── formatting ────────────────────────────────────────────────────────────────
W=72
SEP=$(printf '%*s' "$W" '' | tr ' ' '-')
HDR=$(printf '%*s' "$W" '' | tr ' ' '=')

total_pass=0; total_fail=0
suite_pass=0; suite_fail=0; suite_skip=0
failed_suites=()

echo ""
echo "$HDR"
printf ' nuBASIC Regression Test Suite\n'
printf ' Interpreter : %s\n' "$INTERPRETER"
printf ' Test dir    : %s\n' "$TEST_DIR"
echo "$HDR"

# ── run suites ────────────────────────────────────────────────────────────────
for f in "${TEST_FILES[@]}"; do
    name=$(basename "$f" .bas)
    echo ""
    echo "$SEP"
    printf ' Suite: %s\n' "$name"
    echo "$SEP"

    # ── meta: SKIP ────────────────────────────────────────────────────────────
    skip_val=$(read_meta "$f" "SKIP")
    if [[ -n "$skip_val" ]]; then
        printf '  [SKIPPED] %s\n' "${skip_val}"
        suite_skip=$(( suite_skip + 1 ))
        continue
    fi

    # ── meta: ARGS ────────────────────────────────────────────────────────────
    extra_args=()
    args_line=$(read_meta "$f" "ARGS")
    if [[ -n "$args_line" ]]; then
        # Split on spaces; works for simple tokens without quoting
        IFS=' ' read -r -a extra_args <<< "$args_line"
    fi

    # ── meta: OUTFILE ─────────────────────────────────────────────────────────
    outfile=$(read_meta "$f" "OUTFILE")
    commands_file=$(read_meta "$f" "COMMANDS")
    expect_error=$(read_meta "$f" "EXPECT_ERROR")
    expect_output=$(read_meta "$f" "EXPECT_OUTPUT")
    expect_not_output=$(read_meta "$f" "EXPECT_NOT_OUTPUT")

    # ── run ───────────────────────────────────────────────────────────────────
    if [[ -n "$commands_file" ]]; then
        output=$(cd "$TEST_DIR" && run_commands "$commands_file") || true
    else
        output=$(cd "$TEST_DIR" && run_interp "$f" "${extra_args[@]+"${extra_args[@]}"}") || true
    fi
    interp_exit=$RUN_EXIT

    # If the test writes results to a side file, append its content
    if [[ -n "$outfile" && -f "$TEST_DIR/$outfile" ]]; then
        output+=$'\n'"$(cat "$TEST_DIR/$outfile")"
        rm -f "$TEST_DIR/$outfile"
    fi

    # ── print output (indented) ───────────────────────────────────────────────
    while IFS= read -r line; do printf '  %s\n' "$line"; done <<< "$output"

    if [[ -n "$expect_error" ]]; then
        if contains_ci "$output" "$expect_error"; then
            printf '  [EXPECTED ERROR PASS] %s\n' "$name"
            total_pass=$(( total_pass + 1 ))
            suite_pass=$(( suite_pass + 1 ))
        else
            printf '  [EXPECTED ERROR FAIL] %s  expected=%q exit=%d\n' \
                   "$name" "$expect_error" "$interp_exit"
            total_fail=$(( total_fail + 1 ))
            suite_fail=$(( suite_fail + 1 ))
            failed_suites+=("$name")
        fi
        continue
    fi

    if [[ -n "$expect_output" || -n "$expect_not_output" ]]; then
        expected_parts=()
        not_expected_parts=()
        if [[ -n "$expect_output" ]]; then
            IFS='|' read -r -a expected_parts <<< "$expect_output"
        fi
        if [[ -n "$expect_not_output" ]]; then
            IFS='|' read -r -a not_expected_parts <<< "$expect_not_output"
        fi
        missing_parts=()
        forbidden_parts=()

        for expected in "${expected_parts[@]}"; do
            if ! contains_ci "$output" "$expected"; then
                missing_parts+=("$expected")
            fi
        done

        for forbidden in "${not_expected_parts[@]}"; do
            if contains_ci "$output" "$forbidden"; then
                forbidden_parts+=("$forbidden")
            fi
        done

        assertion_count=$(( ${#expected_parts[@]} + ${#not_expected_parts[@]} ))
        if [[ ${#missing_parts[@]} -eq 0 && ${#forbidden_parts[@]} -eq 0 && "$interp_exit" -eq 0 ]]; then
            printf '  [EXPECTED OUTPUT PASS] %s  (%d assertions)\n' \
                   "$name" "$assertion_count"
            total_pass=$(( total_pass + assertion_count ))
            suite_pass=$(( suite_pass + 1 ))
        else
            printf '  [EXPECTED OUTPUT FAIL] %s  missing=%q forbidden=%q exit=%d\n' \
                   "$name" "${missing_parts[*]}" "${forbidden_parts[*]}" "$interp_exit"
            total_fail=$(( total_fail + ${#missing_parts[@]} + ${#forbidden_parts[@]} + (interp_exit == 0 ? 0 : 1) ))
            suite_fail=$(( suite_fail + 1 ))
            failed_suites+=("$name")
        fi
        continue
    fi

    # ── parse pass/fail counts ────────────────────────────────────────────────
    suite_p=0; suite_f=0

    if printf '%s' "$output" | grep -qE "Results:.*passed" 2>/dev/null; then
        extract_int_before "$output" "passed"; suite_p=$N
        extract_int_before "$output" "failed"; suite_f=$N
    else
        # Fallback: count PASS / FAIL assertion lines
        count_matches "$output" "^[[:space:]]*PASS"; suite_p=$COUNT
        count_matches "$output" "^[[:space:]]*FAIL"; suite_f=$COUNT
    fi

    total_pass=$(( total_pass + suite_p ))
    total_fail=$(( total_fail + suite_f ))

    # Detect runtime errors in output
    count_matches "$output" "^(Runtime Error|At line [0-9]+:)"; runtime_errors=$COUNT

    if [[ "$suite_p" -gt 0 && "$suite_f" -eq 0 && "$interp_exit" -eq 0 && "$runtime_errors" -eq 0 ]]; then
        printf '  [SUITE PASS] %s  (%d assertions)\n' "$name" "$suite_p"
        suite_pass=$(( suite_pass + 1 ))
    else
        printf '  [SUITE FAIL] %s  (%d passed, %d failed, exit=%d)\n' \
               "$name" "$suite_p" "$suite_f" "$interp_exit"
        suite_fail=$(( suite_fail + 1 ))
        failed_suites+=("$name")
    fi
done

# ── summary ───────────────────────────────────────────────────────────────────
echo ""
echo "$HDR"
printf ' SUMMARY\n'
echo "$HDR"
printf '  Suites  : %d total  —  %d passed, %d failed, %d skipped\n' \
       "$(( suite_pass + suite_fail + suite_skip ))" \
       "$suite_pass" "$suite_fail" "$suite_skip"
printf '  Asserts : %d total  —  %d passed, %d failed\n' \
       "$(( total_pass + total_fail ))" "$total_pass" "$total_fail"

if [[ ${#failed_suites[@]} -gt 0 ]]; then
    printf '  Failed suites:\n'
    for s in "${failed_suites[@]}"; do printf '    - %s\n' "$s"; done
fi
echo "$HDR"

if [[ "$suite_fail" -eq 0 ]]; then
    printf ' ALL TESTS PASSED\n'
    echo "$HDR"
    exit 0
else
    printf ' SOME TESTS FAILED\n'
    echo "$HDR"
    exit 1
fi
