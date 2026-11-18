#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>
#include <ostream>
#include <utility>

// Base node. print() writes the Phase-1 ".ast" textual format (2-space indent per level).
struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void print(std::ostream& os, int indent = 0) const = 0;
};

using ASTPtr = std::unique_ptr<ASTNode>;

void printIndent(std::ostream& os, int n);

struct ProgramNode : ASTNode {
    std::vector<ASTPtr> statements;
    void print(std::ostream& os, int indent = 0) const override;
};

struct DeclNode : ASTNode {
    std::string name;
    int line;
    DeclNode(std::string n, int ln) : name(std::move(n)), line(ln) {}
    void print(std::ostream& os, int indent = 0) const override;
};

struct AssignNode : ASTNode {
    std::string target;
    ASTPtr expr;
    int line;
    AssignNode(std::string t, ASTPtr e, int ln)
        : target(std::move(t)), expr(std::move(e)), line(ln) {}
    void print(std::ostream& os, int indent = 0) const override;
};

struct PrintNode : ASTNode {
    ASTPtr expr;
    int line;
    PrintNode(ASTPtr e, int ln) : expr(std::move(e)), line(ln) {}
    void print(std::ostream& os, int indent = 0) const override;
};

struct BlockNode : ASTNode {
    std::vector<ASTPtr> statements;
    void print(std::ostream& os, int indent = 0) const override;
};

struct IfNode : ASTNode {
    ASTPtr condition;
    ASTPtr thenBlock;   // always a BlockNode (shuro ... khatam)
    ASTPtr elsePart;    // BlockNode holding the else StatementList, or nullptr
    int line;
    IfNode(ASTPtr c, ASTPtr t, ASTPtr e, int ln)
        : condition(std::move(c)), thenBlock(std::move(t)),
          elsePart(std::move(e)), line(ln) {}
    void print(std::ostream& os, int indent = 0) const override;
};

struct WhileNode : ASTNode {
    ASTPtr condition;
    ASTPtr body;
    int line;
    WhileNode(ASTPtr c, ASTPtr b, int ln)
        : condition(std::move(c)), body(std::move(b)), line(ln) {}
    void print(std::ostream& os, int indent = 0) const override;
};

struct BinaryOpNode : ASTNode {
    std::string op;     // "+", "*", ">"
    ASTPtr left, right;
    BinaryOpNode(std::string o, ASTPtr l, ASTPtr r)
        : op(std::move(o)), left(std::move(l)), right(std::move(r)) {}
    void print(std::ostream& os, int indent = 0) const override;
};

struct IdentifierNode : ASTNode {
    std::string name;
    explicit IdentifierNode(std::string n) : name(std::move(n)) {}
    void print(std::ostream& os, int indent = 0) const override;
};

struct NumberNode : ASTNode {
    long long value;
    explicit NumberNode(long long v) : value(v) {}
    void print(std::ostream& os, int indent = 0) const override;
};

#endif
