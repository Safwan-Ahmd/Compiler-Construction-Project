#include "AST.h"

void printIndent(std::ostream& os, int n) {
    for (int i = 0; i < n; ++i) os << "  ";
}

void ProgramNode::print(std::ostream& os, int indent) const {
    printIndent(os, indent);
    os << "Program\n";
    for (const auto& s : statements)
        if (s) s->print(os, indent + 1);
}

void DeclNode::print(std::ostream& os, int indent) const {
    printIndent(os, indent);
    os << "Declaration name=" << name << "\n";
}

void AssignNode::print(std::ostream& os, int indent) const {
    printIndent(os, indent);
    os << "Assignment target=" << target << "\n";
    if (expr) expr->print(os, indent + 1);
}

void PrintNode::print(std::ostream& os, int indent) const {
    printIndent(os, indent);
    os << "Print\n";
    if (expr) expr->print(os, indent + 1);
}

void BlockNode::print(std::ostream& os, int indent) const {
    printIndent(os, indent);
    os << "Block\n";
    for (const auto& s : statements)
        if (s) s->print(os, indent + 1);
}

void IfNode::print(std::ostream& os, int indent) const {
    printIndent(os, indent);
    os << "If\n";
    printIndent(os, indent + 1); os << "Condition\n";
    if (condition) condition->print(os, indent + 2);
    printIndent(os, indent + 1); os << "Then\n";
    if (thenBlock) thenBlock->print(os, indent + 2);
    if (elsePart) {
        printIndent(os, indent + 1); os << "Else\n";
        elsePart->print(os, indent + 2);
    }
}

void WhileNode::print(std::ostream& os, int indent) const {
    printIndent(os, indent);
    os << "While\n";
    printIndent(os, indent + 1); os << "Condition\n";
    if (condition) condition->print(os, indent + 2);
    printIndent(os, indent + 1); os << "Body\n";
    if (body) body->print(os, indent + 2);
}

void BinaryOpNode::print(std::ostream& os, int indent) const {
    printIndent(os, indent);
    os << "BinaryOp op=" << op << "\n";
    if (left)  left->print(os, indent + 1);
    if (right) right->print(os, indent + 1);
}

void IdentifierNode::print(std::ostream& os, int indent) const {
    printIndent(os, indent);
    os << "Identifier " << name << "\n";
}

void NumberNode::print(std::ostream& os, int indent) const {
    printIndent(os, indent);
    os << "Number " << value << "\n";
}
