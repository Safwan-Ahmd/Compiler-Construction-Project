#!/bin/sh
# Usage: sh tests/run_tests.sh      (run from the project root after `make`)
BIN=./citu
pass=0; fail=0
mkdir -p out
ok()  { echo "PASS  $1"; pass=$((pass+1)); }
bad() { echo "FAIL  $1  ($2)"; fail=$((fail+1)); }

# 1. valid programs: accepted, AST / symbol table match expected files (if present)
for f in tests/valid/*.citu; do
    n=$(basename "$f" .citu)
    if ! $BIN "$f" -o "out/$n" >/dev/null 2>out/$n.err; then bad "valid/$n" "should be accepted"; continue; fi
    msg=""
    for ext in ast sym; do
        if [ -f "tests/expected/$n.$ext" ] && ! diff -q "tests/expected/$n.$ext" "out/$n.$ext" >/dev/null; then
            msg="$ext differs from tests/expected/$n.$ext"
        fi
    done
    [ -z "$msg" ] && ok "valid/$n" || bad "valid/$n" "$msg"
done
# assigned in both branches of if/else => no 'may be used' warning
if grep -q "may be used" out/assigned_both_branches.err; then bad "valid/assigned_both_branches" "unexpected warning"; else ok "valid/assigned_both_branches (no warning)"; fi

# 2. lexical / syntax errors: rejected
for f in tests/invalid/*.citu; do
    n=$(basename "$f" .citu)
    if $BIN "$f" >/dev/null 2>&1; then bad "invalid/$n" "should be rejected"; else ok "invalid/$n"; fi
done

# 3. semantic errors: rejected AND stderr contains the text in <name>.expect
for f in tests/semantic_invalid/*.citu; do
    n=$(basename "$f" .citu)
    if $BIN "$f" >/dev/null 2>out/$n.err; then bad "semantic_invalid/$n" "should be rejected"; continue; fi
    if grep -qF "$(cat tests/semantic_invalid/$n.expect)" out/$n.err; then ok "semantic_invalid/$n"; else bad "semantic_invalid/$n" "expected message not found"; fi
done

# 4. semantic warnings: accepted (exit 0) AND stderr contains <name>.expect
for f in tests/semantic_warn/*.citu; do
    n=$(basename "$f" .citu)
    if ! $BIN "$f" >/dev/null 2>out/$n.err; then bad "semantic_warn/$n" "warnings must not fail the build"; continue; fi
    if grep -qF "$(cat tests/semantic_warn/$n.expect)" out/$n.err; then ok "semantic_warn/$n"; else bad "semantic_warn/$n" "expected warning not found"; fi
done

echo "-----"; echo "passed: $pass   failed: $fail"
[ "$fail" -eq 0 ]
