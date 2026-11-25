#include "Lexer.h"
#include "Parser.h"
#include "Semantic.h"
#include "AST.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

static bool readFile(const std::string& path, std::string& out) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return false;
    std::stringstream buffer;
    buffer << file.rdbuf();
    out = buffer.str();
    return true;
}

static bool writeFile(const std::string& path, const std::string& text) {
    std::ofstream f(path);
    if (!f.is_open()) {
        std::cerr << "Error: cannot write '" << path << "' (does the directory exist?)\n";
        return false;
    }
    f << text;
    return true;
}

static void printUsage(const char* prog) {
    std::cout << "Usage: " << prog << " <source.citu> [--tokens] [--ast] [--sym] [-o <prefix>]\n"
              << "  --tokens     Print only the token stream\n"
              << "  --ast        Print only the Abstract Syntax Tree\n"
              << "  --sym        Print only the symbol table\n"
              << "  (default)    Print everything + success / error summary\n"
              << "  -o <prefix>  Also write <prefix>.tokens, <prefix>.ast and <prefix>.sym\n"
              << "Exit code: 0 = accepted (warnings allowed), 1 = lexical / syntax / semantic error or bad usage.\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) { printUsage(argv[0]); return 1; }

    std::string filename = argv[1];
    bool showTokens = false, showAST = false, showSym = false;
    std::string outPrefix;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--tokens") showTokens = true;
        else if (arg == "--ast") showAST = true;
        else if (arg == "--sym") showSym = true;
        else if (arg == "-o" && i + 1 < argc) outPrefix = argv[++i];
        else {
            std::cerr << "Unknown or incomplete option: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }
    bool explicitFlag = showTokens || showAST || showSym;
    if (!explicitFlag) { showTokens = true; showAST = true; showSym = true; }

    std::string source;
    if (!readFile(filename, source)) {
        std::cerr << "Error: cannot open file '" << filename << "'\n";
        return 1;
    }

    // ---------- Lexer ----------
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    std::ostringstream tokText;                       // Phase-1 ".tokens" format
    for (const auto& t : tokens) tokText << formatTokenLine(t) << "\n";

    if (showTokens) std::cout << "=== Token Stream ===\n" << tokText.str();
    if (!outPrefix.empty() && !writeFile(outPrefix + ".tokens", tokText.str())) return 1;

    if (lexer.hadError()) {
        std::cerr << "\n=== Lexer Errors ===\n";
        for (const auto& e : lexer.getErrors()) std::cerr << e << "\n";
        std::cerr << "\nLexing FAILED.\n";
        return 1;
    }

    // ---------- Parser ----------
    Parser parser(tokens);
    auto ast = parser.parse();

    if (parser.hadError()) {
        std::cerr << "\n=== Parser Errors ===\n";
        for (const auto& e : parser.getErrors()) std::cerr << e << "\n";
        std::cerr << "\nParsing FAILED.\n";
        return 1;
    }

    std::ostringstream astText;                       // Phase-1 ".ast" format
    ast->print(astText);

    if (showAST) {
        if (showTokens) std::cout << "\n";
        std::cout << "=== Abstract Syntax Tree ===\n" << astText.str();
    }
    if (!outPrefix.empty() && !writeFile(outPrefix + ".ast", astText.str())) return 1;

    // ---------- Semantic analyzer ----------
    SemanticAnalyzer sema;
    sema.analyze(*ast);

    if (showSym) {
        if (showTokens || showAST) std::cout << "\n";
        std::cout << "=== Symbol Table ===\n" << sema.symbols().dump();
    }
    if (!outPrefix.empty() && !writeFile(outPrefix + ".sym", sema.symbols().dump())) return 1;

    if (!sema.diagnostics().empty()) {
        std::cerr << "\n=== Semantic Diagnostics ===\n";
        for (const auto& d : sema.formattedDiagnostics()) std::cerr << d << "\n";
    }
    if (sema.hadError()) {
        std::cerr << "\nSemantic analysis FAILED (" << sema.errorCount() << " error(s), "
                  << sema.warningCount() << " warning(s)).\n";
        return 1;
    }

    if (!explicitFlag) {
        std::cout << "\n=== Result ===\n"
                  << "Lexing   : OK\n"
                  << "Parsing  : OK\n"
                  << "Semantics: OK (" << sema.warningCount() << " warning(s))\n"
                  << "Program accepted by the Citu compiler front end.\n";
    }
    return 0;
}
