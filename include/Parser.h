#ifndef PARSER_H
#define PARSER_H

#include "Token.h"
#include "AST.h"
#include <vector>
#include <string>
#include <memory>
#include <initializer_list>

// Recursive-descent parser for the Citu grammar (Phase-1 report, Section 2.2):
//
//   Program        ::= StatementList EOF
//   StatementList  ::= Statement StatementList | epsilon
//   Statement      ::= Declaration | Assignment | IfStatement | WhileStatement
//                    | PrintStatement | Block
//   Declaration    ::= int32 Identifier ;
//   Assignment     ::= Identifier medalo Expression OptSemi
//   PrintStatement ::= likho ( Expression ) OptSemi
//   Block          ::= shuro StatementList khatam
//   IfStatement    ::= agar ( Condition ) Block ElsePart
//   ElsePart       ::= magar StatementList | epsilon   (runs to next unmatched khatam / EOF)
//   WhileStatement ::= jabtak ( Condition ) Block
//   Condition      ::= Expression > Expression
//   Expression     ::= Term { + Term }        Term ::= Factor { * Factor }
//   Factor         ::= Identifier | Number | ( Expression )
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    std::unique_ptr<ProgramNode> parse();
    const std::vector<std::string>& getErrors() const { return errors; }
    bool hadError() const { return !errors.empty(); }

private:
    const std::vector<Token>& tokens;
    size_t current;
    std::vector<std::string> errors;

    // Helpers
    const Token& peek(int offset = 0) const;
    const Token& previous() const;
    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    Token advance();
    Token consume(TokenType type, const std::string& message);
    void  synchronize();
    void  error(const Token& token, const std::string& message);
    void  statementList(std::vector<ASTPtr>& out, bool stopAtKhatam);

    // Grammar rules
    std::unique_ptr<ProgramNode> program();
    ASTPtr statement();
    ASTPtr declaration();
    ASTPtr assignment();
    ASTPtr printStatement();
    ASTPtr block();
    ASTPtr ifStatement();
    ASTPtr whileStatement();
    ASTPtr condition();
    ASTPtr expression();
    ASTPtr term();
    ASTPtr factor();
};

#endif
