#pragma once

#include "ast.hpp"
#include "token.hpp"
#include "type.hpp"

#include <memory>
#include <string>
#include <vector>

class Parser {
  public:
    explicit Parser(const std::vector<Token> &tokens) : tokens(tokens) {}

    std::unique_ptr<Program> parse();

  private:
    const std::vector<Token> &tokens;
    int cur = 0;

    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> fnDeclaration();
    std::unique_ptr<Stmt> letDeclaration();
    std::unique_ptr<Stmt> returnDeclaration();

    std::unique_ptr<BlockStmt> parseBlock();

    std::unique_ptr<Expr> expression();

    Type parseType(const std::string &lexeme);

    bool match(TokenType type);
    const Token &advance();
    const Token &consume(TokenType expected, const std::string &err);
    const Token &peek() const;
    bool isAtEnd() const;
};
