#!/bin/sh
# Usage: sh tests/run_tests.sh      (run from the project root after `make`)
BIN=./citu
pass=0; fail=0
mkdir -p out

for f in tests/valid/*.citu; do
    name=$(basename "$f" .citu)
    if $BIN "$f" -o "out/$name" >/dev/null 2>&1; then
        if [ -f "tests/expected/$name.ast" ] && ! diff -q "tests/expected/$name.ast" "out/$name.ast" >/dev/null; then
            echo "FAIL  valid/$name  (AST differs from tests/expected/$name.ast)"; fail=$((fail+1))
        else
            echo "PASS  valid/$name"; pass=$((pass+1))
        fi
    else
        echo "FAIL  valid/$name  (should be accepted)"; fail=$((fail+1))
    fi
done

for f in tests/invalid/*.citu; do
    name=$(basename "$f" .citu)
    if $BIN "$f" >/dev/null 2>&1; then
        echo "FAIL  invalid/$name  (should be rejected)"; fail=$((fail+1))
    else
        echo "PASS  invalid/$name"; pass=$((pass+1))
    fi
done

echo "-----"; echo "passed: $pass   failed: $fail"
[ "$fail" -eq 0 ]
