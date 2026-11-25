# Citu Compiler – Phase 3 (Lexer + Parser + Semantic Analyzer)

**lexer**, **recursive-descent parser** and **semantic analyzer** (C++17)
for the Citu educational language. It follows the grammar and the intermediate file formats
from the Phase-1 report (`doc/Citu_Phase1_Report.pdf`).

## Project structure

```
include/  Token.h  Lexer.h  AST.h  Parser.h  Semantic.h
src/      Token.cpp  Lexer.cpp  AST.cpp  Parser.cpp  Semantic.cpp  main.cpp
examples/ prog1.citu  prog2.citu  prog3.citu      (the 3 Phase-1 programs)
tests/    valid/  invalid/  semantic_invalid/  semantic_warn/  expected/  run_tests.sh
doc/      Citu_Phase1_Report.pdf / .tex            (Phase-1 report)
Makefile  README.md
```

## Build and run

```bash
make                                   # builds ./citu   (needs g++ with C++17)
./citu examples/prog2.citu             # tokens + AST + result
./citu examples/prog2.citu --tokens    # only tokens
./citu examples/prog2.citu --ast       # only AST
./citu examples/prog2.citu --sym       # only symbol table
./citu examples/prog2.citu -o out/p2   # also writes out/p2.tokens, out/p2.ast, out/p2.sym
make demo                              # runs the 3 examples, writes out/*.tokens, .ast, .sym
make test                              # 34 automated tests
```

Exit code: `0` = program accepted (warnings allowed), `1` = lexical / syntax / semantic
error (messages go to stderr with line numbers).

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

## Semantic analyzer (Phase 3)

Runs on the AST after a successful parse (`include/Semantic.h`, `src/Semantic.cpp`).
It builds the **symbol table** (Phase-1 Section 3.3: name, type, scope level, line
declared) and checks:

| Id | Kind    | Rule |
|----|---------|------|
| E1 | error   | A variable may be declared only once (single global scope, scope level 0; blocks do not open a new scope). |
| E2 | error   | Every variable that is read or assigned must have been declared **earlier** in the program (use before declaration is an error). |
| E3 | error   | Type rules: operands of `+`, `*`, `>` and the right-hand side of `medalo` / `likho` must be `int32`; `agar` / `jabtak` conditions must be comparisons. (`>` yields `bool`.) |
| W1 | warning | A variable read before it is *definitely assigned*. Flow-sensitive: an assignment counts after an `agar/magar` only if made in **both** branches, and assignments inside a `jabtak` body do not count after the loop. |
| W2 | warning | A variable is declared but never read. |

Errors make the compiler exit with code 1; warnings do not. Diagnostics are sorted
by line, e.g. `Semantic error at line 2: Variable 'y' is not declared`.

`.sym` format (Phase-1 Section 4.3):

```
Name          Type     Scope   Line
------------------------------------
a             int32    0       1
b             int32    0       2
result        int32    0       3
```
