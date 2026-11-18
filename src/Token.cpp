#include "Token.h"
#include <unordered_map>

std::string tokenTypeToString(TokenType type) {
    static const std::unordered_map<TokenType, std::string> names = {
        {TokenType::INT32,      "INT32"},
        {TokenType::MEDALO,     "MEDALO"},
        {TokenType::AGAR,       "AGAR"},
        {TokenType::MAGAR,      "MAGAR"},
        {TokenType::JABTAK,     "JABTAK"},
        {TokenType::SHURO,      "SHURO"},
        {TokenType::KHATAM,     "KHATAM"},
        {TokenType::LIKHO,      "LIKHO"},
        {TokenType::IDENTIFIER, "IDENTIFIER"},
        {TokenType::NUMBER,     "NUMBER"},
        {TokenType::PLUS,       "PLUS"},
        {TokenType::STAR,       "STAR"},
        {TokenType::GT,         "GT"},
        {TokenType::LPAREN,     "LPAREN"},
        {TokenType::RPAREN,     "RPAREN"},
        {TokenType::SEMICOLON,  "SEMICOLON"},
        {TokenType::END_OF_FILE,"EOF"},
        {TokenType::UNKNOWN,    "UNKNOWN"}
    };
    auto it = names.find(type);
    return it != names.end() ? it->second : "???";
}

std::string tokenCategory(TokenType type) {
    switch (type) {
        case TokenType::INT32:
        case TokenType::MEDALO:
        case TokenType::AGAR:
        case TokenType::MAGAR:
        case TokenType::JABTAK:
        case TokenType::SHURO:
        case TokenType::KHATAM:
        case TokenType::LIKHO:      return "KEYWORD";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER:     return "NUMBER";
        case TokenType::PLUS:
        case TokenType::STAR:
        case TokenType::GT:         return "OPERATOR";
        case TokenType::LPAREN:
        case TokenType::RPAREN:
        case TokenType::SEMICOLON:  return "DELIMITER";
        case TokenType::END_OF_FILE:return "EOF";
        default:                    return "UNKNOWN";
    }
}

std::string formatTokenLine(const Token& tok) {
    std::string lex = (tok.type == TokenType::END_OF_FILE) ? "$" : tok.lexeme;
    return tokenCategory(tok.type) + " " + lex + " " + std::to_string(tok.line);
}

std::ostream& operator<<(std::ostream& os, const Token& tok) {
    os << tokenTypeToString(tok.type)
       << "  \"" << tok.lexeme << "\""
       << "  (line " << tok.line << ", col " << tok.column << ")";
    return os;
}
