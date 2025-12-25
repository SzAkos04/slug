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

StmtPtr Parser::declaration() {
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

// fn [[identifier]]([[identifier]]: [[type]]): [[type]] [[block]]
StmtPtr Parser::fnDeclaration() {
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

// let (mut) [[identifier]]: [[type]] = [[expression]];
StmtPtr Parser::letDeclaration() {
    this->consume(TokenType::Let, "Expected 'let' keyword");

    bool mut = false;
    if (this->match(TokenType::Mut)) {
        mut = true;
    }

    Token nameTok =
        this->consume(TokenType::Identifier, "Expected variable name");

    this->consume(TokenType::Colon, "Expected ':' after variable name");

    Token typeTok = this->consume(TokenType::Identifier, "Expected type");
    Type type = this->parseType(typeTok.getLexeme());

    this->consume(TokenType::Equal, "Expected '=' after type");

    auto initializer = this->expression();

    this->consume(TokenType::Semicolon, "Expected ';'");

    return std::make_unique<LetStmt>(nameTok.getLexeme(), mut, type,
                                     std::move(initializer));
}

// return [[expression]];
StmtPtr Parser::returnDeclaration() {
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

    std::vector<StmtPtr> stmts;

    while (!this->isAtEnd() &&
           this->peek().getType() != TokenType::RightBrace) {
        stmts.push_back(this->declaration());
    }

    this->consume(TokenType::RightBrace, "Expected '}'");

    return std::make_unique<BlockStmt>(std::move(stmts));
}

ExprPtr Parser::expression() {
    auto lhs = this->unary();
    if (!lhs) {
        return nullptr;
    }

    return this->parseBinaryRhs(0, std::move(lhs));
}

ExprPtr Parser::parseBinaryRhs(int minPrecedence, ExprPtr lhs) {
    while (true) {
        const Token &tok = this->peek();
        int tokPrec = this->getPrecedence(tok.getType());

        // if the next token isn't a binary operator or it has lower precedence
        // stop
        if (tokPrec < minPrecedence) {
            return lhs;
        }

        // consume operator
        Token opToken = this->advance();

        // handle rhs
        auto rhs = this->unary();
        if (!rhs) {
            return nullptr;
        }

        // if the next operator has higher precendence, connect it to the rhs
        // recursively
        int nextPrec = this->getPrecedence(this->peek().getType());
        if (tokPrec < nextPrec) {
            rhs = this->parseBinaryRhs(tokPrec + 1, std::move(rhs));
            if (!rhs)
                return nullptr;
        }

        lhs =
            std::make_unique<BinaryExpr>(tokenTypeToBinaryOp(opToken.getType()),
                                         std::move(lhs), std::move(rhs));
    }
}

ExprPtr Parser::unary() {
    if (this->match(TokenType::Minus) || this->match(TokenType::Bang)) {
        Token opToken = this->previous();
        auto operand = this->unary(); // recursion
        return std::make_unique<UnaryExpr>(
            tokenTypeToUnaryOp(opToken.getType()), std::move(operand));
    }

    return this->primary();
}

ExprPtr Parser::primary() {
    const Token &tok = this->peek();

    // literals
    if (this->match(TokenType::Number) || this->match(TokenType::True) ||
        this->match(TokenType::False)) {
        return std::make_unique<LiteralExpr>(*this->previous().getLiteral());
    }

    // variable or function call
    if (this->match(TokenType::Identifier)) {
        std::string name = previous().getLexeme();

        // if next token is ( it's a function call
        if (match(TokenType::LeftParen)) {
            return finishCall(name);
        }

        // otherwise simple variable expression
        return std::make_unique<VariableExpr>(name);
    }

    // parentheses
    if (this->match(TokenType::LeftParen)) {
        auto expr = this->expression();
        this->consume(TokenType::RightParen, "Expected ')' after expression");
        return expr;
    }

    throw std::runtime_error(
        "Parser error at line " + std::to_string(tok.getLine()) +
        ": Expected expression, found '" + tok.getLexeme() + "'");
}

ExprPtr Parser::finishCall(const std::string &callee) {
    std::vector<ExprPtr> args;

    // if the next token is not ), parse args
    if (peek().getType() != TokenType::RightParen) {
        do {
            // every arg is an expression
            args.push_back(expression());
        } while (match(TokenType::Comma)); // separated with commas
    }

    consume(TokenType::RightParen, "Expected ')' after arguments.");

    return std::make_unique<CallExpr>(callee, std::move(args));
}

int Parser::getPrecedence(TokenType type) const {
    switch (type) {
    case TokenType::Star:
    case TokenType::Slash:
    case TokenType::Percent:
        return 6;
    case TokenType::Plus:
    case TokenType::Minus:
        return 5;
    case TokenType::Less:
    case TokenType::LessEqual:
    case TokenType::Greater:
    case TokenType::GreaterEqual:
        return 4;
    case TokenType::EqualEqual:
    case TokenType::BangEqual:
        return 3;
    // case TokenType::And:
    //     return 2;
    // case TokenType::Or:
    //     return 1;
    default:
        return -1;
    }
}

bool Parser::isBinaryOp(TokenType type) const {
    return this->getPrecedence(type) != -1;
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

const Token &Parser::consume(TokenType expected, const std::string &err) {
    if (!this->isAtEnd() && this->tokens[this->cur].getType() == expected) {
        return this->tokens[this->cur++];
    }
    throw std::runtime_error("Parser error at line " +
                             std::to_string(this->peek().getLine()) + ": " +
                             err);
}

const Token &Parser::peek() const {
    if (this->isAtEnd()) {
        static Token eofToken(TokenType::Eof, "", std::nullopt, 0);
        return eofToken;
    }
    return this->tokens[this->cur];
}

const Token &Parser::previous() const { return this->tokens[this->cur - 1]; }

bool Parser::isAtEnd() const {
    return this->cur >= (int)this->tokens.size() ||
           this->tokens[this->cur].getType() == TokenType::Eof;
}
