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

    StmtPtr declaration();
    StmtPtr fnDeclaration();
    StmtPtr letDeclaration();
    StmtPtr returnDeclaration();

    std::unique_ptr<BlockStmt> parseBlock();

    ExprPtr expression();
    ExprPtr parseBinaryRhs(int precedence, ExprPtr lhs);
    ExprPtr unary();
    ExprPtr primary();
    ExprPtr finishCall(const std::string &callee);

    int getPrecedence(TokenType type) const;

    bool isBinaryOp(TokenType type) const;

    Type parseType(const std::string &lexeme);

    bool match(TokenType type);

    const Token &advance();

    const Token &consume(TokenType expected, const std::string &err);

    const Token &peek() const;

    const Token &previous() const;

    bool isAtEnd() const;
};
