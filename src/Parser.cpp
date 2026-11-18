#include "Parser.h"
#include <cerrno>
#include <cstdlib>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), current(0) {}

const Token& Parser::peek(int offset) const {
    size_t idx = current + offset;
    if (idx >= tokens.size()) return tokens.back(); // EOF
    return tokens[idx];
}

const Token& Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) { advance(); return true; }
    return false;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    error(peek(), message);
    return peek();
}

void Parser::error(const Token& token, const std::string& message) {
    std::string msg = "Parser error at line " + std::to_string(token.line) +
                      ", col " + std::to_string(token.column) + ": " + message;
    if (token.type == TokenType::END_OF_FILE)
        msg += " (at end of file)";
    else
        msg += " (found \"" + token.lexeme + "\")";
    errors.push_back(msg);
}

// Panic-mode recovery: skip tokens until a likely statement boundary.
// Always consumes at least one token (unless at EOF), so loops cannot get stuck.
void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;
        switch (peek().type) {
            case TokenType::INT32:
            case TokenType::AGAR:
            case TokenType::JABTAK:
            case TokenType::LIKHO:
            case TokenType::SHURO:
            case TokenType::KHATAM:
            case TokenType::MAGAR:
                return;
            default:
                break;
        }
        advance();
    }
}

// StatementList ::= Statement StatementList | epsilon
// Inside a block / else part the list stops at 'khatam'; at top level a stray
// 'khatam' is reported as an error by statement().
void Parser::statementList(std::vector<ASTPtr>& out, bool stopAtKhatam) {
    while (!isAtEnd()) {
        if (stopAtKhatam && check(TokenType::KHATAM)) break;
        ASTPtr stmt = statement();
        if (stmt) out.push_back(std::move(stmt));
        else      synchronize();
    }
}

// -------------------- Grammar --------------------

std::unique_ptr<ProgramNode> Parser::parse() {
    return program();
}

std::unique_ptr<ProgramNode> Parser::program() {
    auto prog = std::make_unique<ProgramNode>();
    statementList(prog->statements, /*stopAtKhatam=*/false);
    return prog;
}

ASTPtr Parser::statement() {
    if (check(TokenType::INT32))   return declaration();
    if (check(TokenType::AGAR))    return ifStatement();
    if (check(TokenType::JABTAK))  return whileStatement();
    if (check(TokenType::LIKHO))   return printStatement();
    if (check(TokenType::SHURO))   return block();
    if (check(TokenType::IDENTIFIER)) {
        if (peek(1).type == TokenType::MEDALO) return assignment();
        error(peek(1), "Expected 'medalo' after identifier '" + peek().lexeme + "'");
        return nullptr;
    }
    if (check(TokenType::KHATAM)) {
        error(peek(), "'khatam' without a matching 'shuro'");
        return nullptr;
    }
    if (check(TokenType::MAGAR)) {
        error(peek(), "'magar' without a matching 'agar' block");
        return nullptr;
    }
    error(peek(), "Expected a statement (declaration, assignment, if, while, print or block)");
    return nullptr;
}

ASTPtr Parser::declaration() {
    consume(TokenType::INT32, "Expected 'int32'");
    Token id = consume(TokenType::IDENTIFIER, "Expected variable name after int32");
    consume(TokenType::SEMICOLON, "Expected ';' after declaration");
    return std::make_unique<DeclNode>(id.lexeme, id.line);
}

ASTPtr Parser::assignment() {
    Token id = consume(TokenType::IDENTIFIER, "Expected variable name");
    consume(TokenType::MEDALO, "Expected 'medalo' after variable name");
    ASTPtr expr = expression();
    match(TokenType::SEMICOLON);   // OptSemi: official notes show both styles
    return std::make_unique<AssignNode>(id.lexeme, std::move(expr), id.line);
}

ASTPtr Parser::printStatement() {
    Token kw = consume(TokenType::LIKHO, "Expected 'likho'");
    consume(TokenType::LPAREN, "Expected '(' after likho");
    ASTPtr expr = expression();
    consume(TokenType::RPAREN, "Expected ')' after expression in likho");
    match(TokenType::SEMICOLON);   // OptSemi
    return std::make_unique<PrintNode>(std::move(expr), kw.line);
}

ASTPtr Parser::block() {
    consume(TokenType::SHURO, "Expected 'shuro'");
    auto blk = std::make_unique<BlockNode>();
    statementList(blk->statements, /*stopAtKhatam=*/true);
    consume(TokenType::KHATAM, "Expected 'khatam' to close block");
    return blk;
}

ASTPtr Parser::ifStatement() {
    Token kw = consume(TokenType::AGAR, "Expected 'agar'");
    consume(TokenType::LPAREN, "Expected '(' after agar");
    ASTPtr cond = condition();
    consume(TokenType::RPAREN, "Expected ')' after condition");

    ASTPtr thenPart = block();

    // ElsePart ::= magar StatementList | epsilon
    // As in the instructor's notes, the else part needs no shuro/khatam; its
    // StatementList extends to the next unmatched 'khatam' or the end of input.
    ASTPtr elsePart = nullptr;
    if (match(TokenType::MAGAR)) {
        auto elseBlk = std::make_unique<BlockNode>();
        statementList(elseBlk->statements, /*stopAtKhatam=*/true);
        elsePart = std::move(elseBlk);
    }
    return std::make_unique<IfNode>(std::move(cond), std::move(thenPart),
                                    std::move(elsePart), kw.line);
}

ASTPtr Parser::whileStatement() {
    Token kw = consume(TokenType::JABTAK, "Expected 'jabtak'");
    consume(TokenType::LPAREN, "Expected '(' after jabtak");
    ASTPtr cond = condition();
    consume(TokenType::RPAREN, "Expected ')' after condition");
    ASTPtr body = block();
    return std::make_unique<WhileNode>(std::move(cond), std::move(body), kw.line);
}

ASTPtr Parser::condition() {
    // Condition ::= Expression > Expression
    ASTPtr left = expression();
    consume(TokenType::GT, "Expected '>' in condition");
    ASTPtr right = expression();
    return std::make_unique<BinaryOpNode>(">", std::move(left), std::move(right));
}

ASTPtr Parser::expression() {
    // Term { + Term }   -- left-associative
    ASTPtr expr = term();
    while (match(TokenType::PLUS)) {
        ASTPtr right = term();
        expr = std::make_unique<BinaryOpNode>("+", std::move(expr), std::move(right));
    }
    return expr;
}

ASTPtr Parser::term() {
    // Factor { * Factor }   -- left-associative, binds tighter than '+'
    ASTPtr expr = factor();
    while (match(TokenType::STAR)) {
        ASTPtr right = factor();
        expr = std::make_unique<BinaryOpNode>("*", std::move(expr), std::move(right));
    }
    return expr;
}

ASTPtr Parser::factor() {
    if (match(TokenType::NUMBER)) {
        errno = 0;
        long long val = std::strtoll(previous().lexeme.c_str(), nullptr, 10);
        if (errno == ERANGE || val < -2147483648LL || val > 2147483647LL)
            error(previous(), "Integer literal does not fit in int32");
        return std::make_unique<NumberNode>(val);
    }
    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<IdentifierNode>(previous().lexeme);
    }
    if (match(TokenType::LPAREN)) {
        ASTPtr expr = expression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }
    error(peek(), "Expected number, identifier or '('");
    return nullptr;
}
