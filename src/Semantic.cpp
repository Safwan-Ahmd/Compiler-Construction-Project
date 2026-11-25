#include "Semantic.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

std::string typeToString(Type t) {
    switch (t) {
        case Type::INT32: return "int32";
        case Type::BOOL:  return "bool";
        default:          return "<error>";
    }
}

// ------------------------------ SymbolTable ------------------------------

bool SymbolTable::declare(const std::string& name, const std::string& type, int scopeLevel, int line) {
    if (index.count(name)) return false;
    SymbolEntry e;
    e.name = name; e.type = type; e.scopeLevel = scopeLevel; e.lineDeclared = line;
    index[name] = rows.size();
    rows.push_back(e);
    return true;
}

SymbolEntry* SymbolTable::lookup(const std::string& name) {
    auto it = index.find(name);
    return it == index.end() ? nullptr : &rows[it->second];
}

const SymbolEntry* SymbolTable::lookup(const std::string& name) const {
    auto it = index.find(name);
    return it == index.end() ? nullptr : &rows[it->second];
}

std::string SymbolTable::dump() const {
    std::ostringstream os;
    os << std::left << std::setw(14) << "Name" << std::setw(9) << "Type"
       << std::setw(8) << "Scope" << "Line\n";
    os << std::string(36, '-') << "\n";
    for (const auto& e : rows) {
        os << std::left << std::setw(14) << e.name << std::setw(9) << e.type
           << std::setw(8) << e.scopeLevel << e.lineDeclared << "\n";
    }
    return os.str();
}

// --------------------------- SemanticAnalyzer ----------------------------

void SemanticAnalyzer::error(int line, const std::string& msg)   { diags.push_back({line, true,  msg}); }
void SemanticAnalyzer::warning(int line, const std::string& msg) { diags.push_back({line, false, msg}); }

bool SemanticAnalyzer::hadError() const { return errorCount() > 0; }
int SemanticAnalyzer::errorCount() const {
    return static_cast<int>(std::count_if(diags.begin(), diags.end(), [](const Diagnostic& d) { return d.isError; }));
}
int SemanticAnalyzer::warningCount() const {
    return static_cast<int>(diags.size()) - errorCount();
}

std::vector<std::string> SemanticAnalyzer::formattedDiagnostics() const {
    std::vector<std::string> out;
    for (const auto& d : diags)
        out.push_back(std::string("Semantic ") + (d.isError ? "error" : "warning") +
                      " at line " + std::to_string(d.line) + ": " + d.message);
    return out;
}

void SemanticAnalyzer::analyze(const ProgramNode& program) {
    AssignedSet assigned;
    visitList(program.statements, assigned);

    // W2: declared but never read
    for (const auto& e : table.entries())
        if (!e.used)
            warning(e.lineDeclared, "Variable '" + e.name + "' is declared but never used");

    std::stable_sort(diags.begin(), diags.end(),
                     [](const Diagnostic& a, const Diagnostic& b) { return a.line < b.line; });
}

void SemanticAnalyzer::visitList(const std::vector<ASTPtr>& stmts, AssignedSet& assigned) {
    for (const auto& s : stmts) visitStatement(s.get(), assigned);
}

void SemanticAnalyzer::visitStatement(const ASTNode* node, AssignedSet& assigned) {
    if (!node) return;

    if (auto d = dynamic_cast<const DeclNode*>(node)) {
        // Single global scope (Phase-1 Section 3.3): a name may be declared only once.
        SymbolEntry* prev = table.lookup(d->name);
        if (prev)
            error(d->line, "Variable '" + d->name + "' is already declared (first declared at line " +
                           std::to_string(prev->lineDeclared) + ")");
        else
            table.declare(d->name, "int32", 0, d->line);
        return;
    }

    if (auto a = dynamic_cast<const AssignNode*>(node)) {
        Type rhs = checkExpr(a->expr.get(), a->line, assigned);   // rhs is evaluated first
        SymbolEntry* sym = table.lookup(a->target);
        if (!sym) {
            error(a->line, "Variable '" + a->target + "' is not declared");
            return;
        }
        if (rhs != Type::ERROR && rhs != Type::INT32)
            error(a->line, "Cannot assign a " + typeToString(rhs) + " value to int32 variable '" + a->target + "'");
        assigned.insert(a->target);
        return;
    }

    if (auto p = dynamic_cast<const PrintNode*>(node)) {
        Type t = checkExpr(p->expr.get(), p->line, assigned);
        if (t != Type::ERROR && t != Type::INT32)
            error(p->line, "likho expects an int32 expression, found " + typeToString(t));
        return;
    }

    if (auto b = dynamic_cast<const BlockNode*>(node)) {
        visitList(b->statements, assigned);       // blocks share the single global scope
        return;
    }

    if (auto i = dynamic_cast<const IfNode*>(node)) {
        checkCondition(i->condition.get(), i->line, assigned);
        AssignedSet thenSet = assigned;
        visitStatement(i->thenBlock.get(), thenSet);
        if (i->elsePart) {
            AssignedSet elseSet = assigned;
            visitStatement(i->elsePart.get(), elseSet);
            // definitely assigned afterwards = assigned in BOTH branches
            for (const auto& v : thenSet)
                if (elseSet.count(v)) assigned.insert(v);
        }
        // no else part: nothing new is definitely assigned
        return;
    }

    if (auto w = dynamic_cast<const WhileNode*>(node)) {
        checkCondition(w->condition.get(), w->line, assigned);
        AssignedSet bodySet = assigned;
        visitStatement(w->body.get(), bodySet);   // loop may run zero times: discard its assignments
        return;
    }

    if (auto prog = dynamic_cast<const ProgramNode*>(node)) {
        visitList(prog->statements, assigned);
    }
}

void SemanticAnalyzer::checkCondition(const ASTNode* cond, int line, const AssignedSet& assigned) {
    Type t = checkExpr(cond, line, assigned);
    if (t != Type::ERROR && t != Type::BOOL)
        error(line, "Condition must be a comparison (expr > expr), found " + typeToString(t));
}

Type SemanticAnalyzer::checkExpr(const ASTNode* node, int line, const AssignedSet& assigned) {
    if (!node) return Type::ERROR;

    if (dynamic_cast<const NumberNode*>(node)) return Type::INT32;

    if (auto id = dynamic_cast<const IdentifierNode*>(node)) {
        SymbolEntry* sym = table.lookup(id->name);
        if (!sym) {
            error(id->line, "Variable '" + id->name + "' is not declared");
            return Type::ERROR;
        }
        sym->used = true;
        if (!assigned.count(id->name))
            warning(id->line, "Variable '" + id->name + "' may be used before it is assigned a value");
        return Type::INT32;
    }

    if (auto bin = dynamic_cast<const BinaryOpNode*>(node)) {
        Type l = checkExpr(bin->left.get(), line, assigned);
        Type r = checkExpr(bin->right.get(), line, assigned);
        bool ok = true;
        for (Type t : {l, r}) {
            if (t == Type::ERROR) { ok = false; continue; }          // already reported
            if (t != Type::INT32) {
                error(line, "Operator '" + bin->op + "' requires int32 operands, found " + typeToString(t));
                ok = false;
            }
        }
        if (!ok) return Type::ERROR;
        return bin->op == ">" ? Type::BOOL : Type::INT32;
    }

    return Type::ERROR;
}
