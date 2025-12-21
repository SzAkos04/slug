#pragma once

#include "literal.hpp"
#include "token.hpp"

#include <optional>
#include <string>
#include <vector>

class Lexer {
  public:
    Lexer() : start(0), cur(0), line(1) {}
    ~Lexer() = default;

    std::vector<Token> lex(const std::string &infile);

  private:
    std::string src;
    std::vector<Token> tokens;
    int start, cur;
    int line;

    std::string readFile(const std::string &infile);

    void scanTokens();

    void addTokenWithLiteral(TokenType type, std::optional<Literal> literal);
    void addToken(TokenType type);

    void number();
    void identifier();

    char advance();
    char peek();
    char peekNext();
    bool match(char expected);
    bool isAtEnd();
};
