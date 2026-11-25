#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "AST.h"
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

// Types in Citu: every variable is int32; a comparison ('>') yields BOOL and may
// only be used as the condition of agar / jabtak.
enum class Type { INT32, BOOL, ERROR };

std::string typeToString(Type t);

// One row of the symbol table (Phase-1 Section 3.3).
struct SymbolEntry {
    std::string name;
    std::string type;        // "int32"
    int         scopeLevel;  // 0 = the single global scope
    int         lineDeclared;
    bool        used = false;   // read at least once
};

class SymbolTable {
public:
    // Returns false (and does not insert) if the name already exists.
    bool declare(const std::string& name, const std::string& type, int scopeLevel, int line);
    SymbolEntry* lookup(const std::string& name);
    const SymbolEntry* lookup(const std::string& name) const;
    const std::vector<SymbolEntry>& entries() const { return rows; }

    // Text of the ".sym" intermediate file (Phase-1 Section 4.3).
    std::string dump() const;

private:
    std::vector<SymbolEntry> rows;                       // declaration order
    std::unordered_map<std::string, size_t> index;       // name -> row
};

struct Diagnostic {
    int         line;
    bool        isError;     // false = warning
    std::string message;
};

// Semantic analysis over the AST produced by the parser. Checks:
//   E1  variable declared twice                      (error)
//   E2  variable used / assigned but never declared  (error; includes use before declaration)
//   E3  type errors: operands of + * > and the right-hand side of medalo / likho must be int32;
//       conditions must be comparisons                (error)
//   W1  variable read before it is definitely assigned (warning, flow-sensitive)
//   W2  variable declared but never read              (warning)
class SemanticAnalyzer {
public:
    void analyze(const ProgramNode& program);

    bool hadError() const;
    int  errorCount() const;
    int  warningCount() const;
    const std::vector<Diagnostic>& diagnostics() const { return diags; }   // sorted by line
    const SymbolTable& symbols() const { return table; }

    // "Semantic error at line N: ..." / "Semantic warning at line N: ..."
    std::vector<std::string> formattedDiagnostics() const;

private:
    SymbolTable table;
    std::vector<Diagnostic> diags;

    using AssignedSet = std::set<std::string>;   // variables definitely assigned at this point

    void error(int line, const std::string& msg);
    void warning(int line, const std::string& msg);

    void visitStatement(const ASTNode* node, AssignedSet& assigned);
    void visitList(const std::vector<ASTPtr>& stmts, AssignedSet& assigned);
    void checkCondition(const ASTNode* cond, int line, const AssignedSet& assigned);
    Type checkExpr(const ASTNode* node, int line, const AssignedSet& assigned);
};

#endif
