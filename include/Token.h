#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <ostream>

enum class TokenType {
    // Keywords
    INT32, MEDALO, AGAR, MAGAR, JABTAK, SHURO, KHATAM, LIKHO,

    // Literals & Identifiers
    IDENTIFIER, NUMBER,

    // Operators
    PLUS, STAR, GT,

    // Delimiters
    LPAREN, RPAREN, SEMICOLON,

    // Special
    END_OF_FILE, UNKNOWN
};

struct Token {
    TokenType   type;
    std::string lexeme;
    int         line;
    int         column;

    Token(TokenType t = TokenType::UNKNOWN,
          const std::string& lex = "",
          int ln = 0, int col = 0)
        : type(t), lexeme(lex), line(ln), column(col) {}
};

// Name of the token type, e.g. "INT32", "PLUS" (used for debugging).
std::string tokenTypeToString(TokenType type);

// Phase-1 category: KEYWORD, IDENTIFIER, NUMBER, OPERATOR, DELIMITER, EOF, UNKNOWN.
std::string tokenCategory(TokenType type);

// One line of the ".tokens" intermediate file (Phase-1 format):
//     <TOKEN_TYPE> <lexeme> <line>
std::string formatTokenLine(const Token& tok);

std::ostream& operator<<(std::ostream& os, const Token& tok);

#endif
