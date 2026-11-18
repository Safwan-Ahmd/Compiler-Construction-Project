#include "Lexer.h"
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <unordered_map>

namespace {
bool isLetter(char c) { return std::isalpha(static_cast<unsigned char>(c)) != 0; }
bool isDigit(char c)  { return std::isdigit(static_cast<unsigned char>(c)) != 0; }
bool isIdChar(char c) { return isLetter(c) || isDigit(c) || c == '_'; }
}

Lexer::Lexer(const std::string& source)
    : source(source), pos(0), line(1), column(1) {}

char Lexer::peek(int offset) const {
    size_t idx = pos + offset;
    if (idx >= source.size()) return '\0';
    return source[idx];
}

char Lexer::advance() {
    char c = source[pos++];
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

bool Lexer::isAtEnd() const {
    return pos >= source.size();
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') advance();
        else break;
    }
}

Token Lexer::makeToken(TokenType type, const std::string& lexeme, int startLine, int startCol) {
    return Token(type, lexeme, startLine, startCol);
}

void Lexer::addError(int ln, int col, const std::string& msg) {
    errors.push_back("Lexer error at line " + std::to_string(ln) +
                     ", col " + std::to_string(col) + ": " + msg);
}

// identifier := letter (letter | digit | '_')*   -- keywords take priority
Token Lexer::identifierOrKeyword() {
    int startLine = line, startCol = column;
    size_t start = pos;
    while (!isAtEnd() && isIdChar(peek())) advance();
    std::string text = source.substr(start, pos - start);

    static const std::unordered_map<std::string, TokenType> keywords = {
        {"int32",  TokenType::INT32},
        {"medalo", TokenType::MEDALO},
        {"agar",   TokenType::AGAR},
        {"magar",  TokenType::MAGAR},
        {"jabtak", TokenType::JABTAK},
        {"shuro",  TokenType::SHURO},
        {"khatam", TokenType::KHATAM},
        {"likho",  TokenType::LIKHO}
    };

    auto it = keywords.find(text);
    if (it != keywords.end()) return makeToken(it->second, text, startLine, startCol);
    return makeToken(TokenType::IDENTIFIER, text, startLine, startCol);
}

// A word that starts with '_' (identifiers must start with a letter).
Token Lexer::badWord() {
    int startLine = line, startCol = column;
    size_t start = pos;
    while (!isAtEnd() && isIdChar(peek())) advance();
    std::string text = source.substr(start, pos - start);
    addError(startLine, startCol,
             "Invalid identifier '" + text + "': identifiers must start with a letter");
    return makeToken(TokenType::UNKNOWN, text, startLine, startCol);
}

// number := '-'? digit+   (must fit in a signed 32-bit integer)
Token Lexer::number() {
    int startLine = line, startCol = column;
    size_t start = pos;
    if (peek() == '-') advance();
    while (!isAtEnd() && isDigit(peek())) advance();
    std::string text = source.substr(start, pos - start);

    errno = 0;
    long long v = std::strtoll(text.c_str(), nullptr, 10);
    if (errno == ERANGE || v < -2147483648LL || v > 2147483647LL) {
        addError(startLine, startCol,
                 "Integer literal '" + text + "' does not fit in int32");
    }
    return makeToken(TokenType::NUMBER, text, startLine, startCol);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (!isAtEnd()) {
        skipWhitespace();
        if (isAtEnd()) break;

        int startLine = line, startCol = column;
        char c = peek();

        if (isLetter(c))                          { tokens.push_back(identifierOrKeyword()); continue; }
        if (c == '_')                             { tokens.push_back(badWord()); continue; }
        if (isDigit(c))                           { tokens.push_back(number()); continue; }
        if (c == '-' && isDigit(peek(1)))         { tokens.push_back(number()); continue; }

        advance();
        switch (c) {
            case '+': tokens.push_back(makeToken(TokenType::PLUS,      "+", startLine, startCol)); break;
            case '*': tokens.push_back(makeToken(TokenType::STAR,      "*", startLine, startCol)); break;
            case '>': tokens.push_back(makeToken(TokenType::GT,        ">", startLine, startCol)); break;
            case '(': tokens.push_back(makeToken(TokenType::LPAREN,    "(", startLine, startCol)); break;
            case ')': tokens.push_back(makeToken(TokenType::RPAREN,    ")", startLine, startCol)); break;
            case ';': tokens.push_back(makeToken(TokenType::SEMICOLON, ";", startLine, startCol)); break;
            case '-':
                addError(startLine, startCol,
                         "'-' is only allowed as the sign of a number literal (Citu has no subtraction operator)");
                tokens.push_back(makeToken(TokenType::UNKNOWN, "-", startLine, startCol));
                break;
            default:
                addError(startLine, startCol, std::string("Unexpected character '") + c + "'");
                tokens.push_back(makeToken(TokenType::UNKNOWN, std::string(1, c), startLine, startCol));
                break;
        }
    }

    // EOF token: reported on the line of the last real token (matches the Phase-1 .tokens example)
    int eofLine = tokens.empty() ? 1 : tokens.back().line;
    tokens.push_back(Token(TokenType::END_OF_FILE, "", eofLine, column));
    return tokens;
}
