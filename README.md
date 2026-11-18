# Citu Compiler – Phase 2 (Lexer + Parser)

Hand-written **lexer** and **recursive-descent parser** (C++17) for the Citu
educational language. It follows the grammar and the intermediate file formats
from the Phase-1 report (`doc/Citu_Phase1_Report.pdf`).

## Project structure

```
include/  Token.h  Lexer.h  AST.h  Parser.h
src/      Token.cpp  Lexer.cpp  AST.cpp  Parser.cpp  main.cpp
examples/ prog1.citu  prog2.citu  prog3.citu      (the 3 Phase-1 programs)
tests/    valid/  invalid/  expected/  run_tests.sh
doc/      Citu_Phase1_Report.pdf / .tex            (Phase-1 report)
Makefile  README.md
```

## Build and run

```bash
make                                   # builds ./citu   (needs g++ with C++17)
./citu examples/prog2.citu             # tokens + AST + result
./citu examples/prog2.citu --tokens    # only tokens
./citu examples/prog2.citu --ast       # only AST
./citu examples/prog2.citu -o out/p2   # also writes out/p2.tokens and out/p2.ast
make demo                              # runs the 3 examples, writes out/*.tokens, out/*.ast
make test                              # 20 automated tests
```

Exit code: `0` = program accepted, `1` = lexical/syntax error (messages go to stderr
with line and column).

## Lexer

Keywords `int32 medalo agar magar jabtak shuro khatam likho`; identifiers
`letter (letter|digit|_)*` (keywords take priority); numbers `-? digit+`
(must fit in 32 bits); operators `+ * >`; delimiters `( ) ;`; whitespace skipped;
`EOF` token at the end. Illegal characters, identifiers starting with `_`,
a `-` that is not the sign of a number, and out-of-range integers are lexical errors.

`.tokens` format (one token per line): `<CATEGORY> <lexeme> <line>`, e.g.
`KEYWORD int32 1`, ending with `EOF $ <line>`.

## Parser

Grammar (Phase-1, Section 2.2):

```
Program        ::= StatementList EOF
StatementList  ::= Statement StatementList | ε
Statement      ::= Declaration | Assignment | IfStatement | WhileStatement | PrintStatement | Block
Declaration    ::= int32 Identifier ;
Assignment     ::= Identifier medalo Expression OptSemi
PrintStatement ::= likho ( Expression ) OptSemi
OptSemi        ::= ; | ε
Block          ::= shuro StatementList khatam
IfStatement    ::= agar ( Condition ) Block ElsePart
ElsePart       ::= magar StatementList | ε
WhileStatement ::= jabtak ( Condition ) Block
Condition      ::= Expression > Expression
Expression     ::= Term { + Term }        Term ::= Factor { * Factor }
Factor         ::= Identifier | Number | ( Expression )
```

* `*` binds tighter than `+`; both are left-associative.
* The `magar` (else) part is a StatementList and runs to the next unmatched
  `khatam` or the end of the program. Put code that must run afterwards *before*
  the `agar`, or wrap the if-else in `shuro … khatam`.
* Syntax errors use panic-mode recovery, so several errors can be reported at once.

`.ast` format: indented tree (`Program`, `Declaration name=x`, `Assignment target=x`,
`BinaryOp op=+`, `Number 25`, …). Example (`examples/prog1.citu`):

```
Program
  Declaration name=x
  Assignment target=x
    Number 25
  Print
    Identifier x
```

## Submission (Phase 2)

1. Push the whole project (including `doc/`) to your GitHub repository.
2. Make sure `umarsuleman@gmail.com` and `mscs24052@itu.edu.pk` have access.
3. On Google Classroom submit only:

```
Student ID: <your-id>
Repository URL: https://github.com/<user>/<repo>
Commit Hash: <hash from `git rev-parse HEAD`>
```
