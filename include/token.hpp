#pragma once

#include "literal.hpp"

#include <optional>
#include <ostream>
#include <string>

enum class TokenType {
    // Single character tokens
    LeftParen,  // (
    RightParen, // )
    LeftBrace,  // {
    RightBrace, // }
    Comma,      // ,
    Dot,        // .
    Semicolon,  // ;
    Colon,      // :

    // One or two character tokens
    Bang,       // !
    BangEqual,  // !=
    Equal,      // =
    EqualEqual, // ==
    // ...

    Identifier,
    String,
    Number,
    Boolean,

    // keywords
    Fn,
    Let,
    Return,

    Eof,
};

std::ostream &operator<<(std::ostream &os, TokenType t);

class Token {
  public:
    Token(TokenType type_, const std::string &lexeme_,
          std::optional<Literal> literal_, int line_)
        : type(type_), lexeme(lexeme_), literal(literal_), line(line_) {}
    ~Token() = default;

    TokenType getType() const { return this->type; }
    const std::string &getLexeme() const { return this->lexeme; }
    const std::optional<Literal> &getLiteral() const { return this->literal; }
    int getLine() const { return this->line; }

  private:
    TokenType type;
    std::string lexeme;
    std::optional<Literal> literal;
    int line;
};
