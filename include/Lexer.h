#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include <string>
#include <vector>

class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();
    const std::vector<std::string>& getErrors() const { return errors; }
    bool hadError() const { return !errors.empty(); }

private:
    std::string source;
    size_t      pos;
    int         line;
    int         column;
    std::vector<std::string> errors;

    char peek(int offset = 0) const;
    char advance();
    bool isAtEnd() const;
    void skipWhitespace();
    Token makeToken(TokenType type, const std::string& lexeme, int startLine, int startCol);
    Token identifierOrKeyword();
    Token number();
    Token badWord();
    void  addError(int ln, int col, const std::string& msg);
};

#endif
