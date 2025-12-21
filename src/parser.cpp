#include "ast.hpp"
#include "parser.hpp"
#include "token.hpp"
#include "type.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

std::unique_ptr<Program> Parser::parse() {
    auto program = std::make_unique<Program>();

    while (!this->isAtEnd()) {
        program->stmts.push_back(this->declaration());
    }

    return program;
}

std::unique_ptr<Stmt> Parser::declaration() {
    if (this->peek().getType() == TokenType::Fn) {
        return this->fnDeclaration();
    } else if (this->peek().getType() == TokenType::Let) {
        return this->letDeclaration();
    } else if (this->peek().getType() == TokenType::Return) {
        return this->returnDeclaration();
    } else {
        throw std::runtime_error("Expected declaration");
    }
}

// fn foo(bar: i32): i32 {
//     [[block]]
// }
std::unique_ptr<Stmt> Parser::fnDeclaration() {
    this->consume(TokenType::Fn, "Expected 'fn' keyword");

    Token nameTok =
        this->consume(TokenType::Identifier, "Expected function name");

    this->consume(TokenType::LeftParen, "Expected '(' after function name");

    std::vector<FnParam> params;
    while (!this->match(TokenType::RightParen)) {
        do {
            Token paramNameTok =
                this->consume(TokenType::Identifier, "Expected parameter name");
            this->consume(TokenType::Colon,
                          "Expected ':' after parameter name");
            Token paramTypeTok =
                this->consume(TokenType::Identifier, "Expected type after ':'");
            Type paramType = this->parseType(paramTypeTok.getLexeme());
            params.emplace_back(paramNameTok.getLexeme(), paramType);
        } while (this->match(TokenType::Comma));
    } // ')' consumed

    this->consume(TokenType::Colon, "Expected ':'");

    Token typeTok =
        this->consume(TokenType::Identifier, "Expected type after ':'");
    Type retType = this->parseType(typeTok.getLexeme());

    auto body = this->parseBlock();

    return std::make_unique<FnStmt>(nameTok.getLexeme(), params, retType,
                                    std::move(body));
}

// let foo: i32 = 5;
std::unique_ptr<Stmt> Parser::letDeclaration() {
    this->consume(TokenType::Let, "Expected 'let' keyword");

    Token nameTok =
        this->consume(TokenType::Identifier, "Expected variable name");

    this->consume(TokenType::Colon, "Expected ':' after variable name");

    Token typeTok = this->consume(TokenType::Identifier, "Expected type");
    Type type = this->parseType(typeTok.getLexeme());

    this->consume(TokenType::Equal, "Expected '=' after type");

    auto initializer = this->expression();

    this->consume(TokenType::Semicolon, "Expected ';'");

    return std::make_unique<LetStmt>(nameTok.getLexeme(), type,
                                     std::move(initializer));
}

// return 0;
std::unique_ptr<Stmt> Parser::returnDeclaration() {
    this->consume(TokenType::Return, "Expected 'return' keyword");

    if (this->peek().getType() == TokenType::Semicolon) {
        this->consume(TokenType::Semicolon, "Expected ';'");
        return std::make_unique<ReturnStmt>();
    }

    auto value = this->expression();

    this->consume(TokenType::Semicolon, "Expected ';'");

    return std::make_unique<ReturnStmt>(std::move(value));
}

std::unique_ptr<BlockStmt> Parser::parseBlock() {
    this->consume(TokenType::LeftBrace, "Expected '{'");

    std::vector<std::unique_ptr<Stmt>> stmts;

    while (!this->isAtEnd() &&
           this->peek().getType() != TokenType::RightBrace) {
        stmts.push_back(this->declaration());
    }

    this->consume(TokenType::RightBrace, "Expected '}'");

    return std::make_unique<BlockStmt>(std::move(stmts));
}

std::unique_ptr<Expr> Parser::expression() {
    Token tok = advance();

    if (!tok.getLiteral().has_value()) {
        throw std::runtime_error("Expected literal value");
    }

    return std::make_unique<LiteralExpr>(*tok.getLiteral());
}

Type Parser::parseType(const std::string &lexeme) {
    if (lexeme == "void") {
        return Type(PrimitiveType::Void);
    } else if (lexeme == "i32") {
        return Type(PrimitiveType::I32);
    } else if (lexeme == "f64") {
        return Type(PrimitiveType::F64);
    } else if (lexeme == "bool") {
        return Type(PrimitiveType::Bool);
    } else if (lexeme == "string") {
        return Type(PrimitiveType::String);
    } else {
        throw std::runtime_error("Unknown type: " + lexeme);
    }
}

bool Parser::match(TokenType type) {
    if (this->isAtEnd()) {
        return false;
    }
    if (this->tokens[this->cur].getType() != type) {
        return false;
    }
    this->cur++;
    return true;
}

const Token &Parser::advance() {
    if (!this->isAtEnd()) {
        this->cur++;
    }
    return this->tokens[this->cur - 1];
}

const Token &Parser::consume(TokenType type, const std::string &msg) {
    if (!this->isAtEnd() && this->tokens[this->cur].getType() == type) {
        return this->tokens[this->cur++];
    }
    throw std::runtime_error("Parser error at line " +
                             std::to_string(this->peek().getLine()) + ": " +
                             msg);
}

const Token &Parser::peek() const {
    if (this->isAtEnd()) {
        static Token eofToken(TokenType::Eof, "", std::nullopt, 0);
        return eofToken;
    }
    return this->tokens[this->cur];
}

bool Parser::isAtEnd() const {
    return this->cur >= (int)this->tokens.size() ||
           this->tokens[this->cur].getType() == TokenType::Eof;
}
