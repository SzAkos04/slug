#include "lexer.hpp"
#include "token.hpp"

#include <cctype>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<Token> Lexer::lex(const std::string &infile) {
    this->src = this->readFile(infile);
    while (!this->isAtEnd()) {
        this->start = this->cur;
        this->scanTokens();
    }

    for (const auto &token : this->tokens) {
        std::cout << token.getLexeme() << "\t(" << token.getType() << ")"
                  << std::endl;
    }

    this->addToken(TokenType::Eof);

    return this->tokens;
}

void Lexer::scanTokens() {
    char c = this->advance();

    if (c == ' ' || c == '\r' || c == '\t') {
        return;
    }
    if (c == '\n') {
        this->line++;
        return;
    }

    switch (c) {
    case '(':
        this->addToken(TokenType::LeftParen);
        break;
    case ')':
        this->addToken(TokenType::RightParen);
        break;
    case '{':
        this->addToken(TokenType::LeftBrace);
        break;
    case '}':
        this->addToken(TokenType::RightBrace);
        break;
    case ',':
        this->addToken(TokenType::Comma);
        break;
    case '.':
        this->addToken(TokenType::Dot);
        break;
    case ';':
        this->addToken(TokenType::Semicolon);
        break;
    case ':':
        this->addToken(TokenType::Colon);
        break;
    case '!':
        this->addToken(this->match('=') ? TokenType::BangEqual
                                        : TokenType::Bang);
        break;
    case '=':
        this->addToken(this->match('=') ? TokenType::EqualEqual
                                        : TokenType::Equal);
        break;
    default:
        if (std::isdigit(c)) {
            this->number();
        } else if (std::isalpha(c) || c == '_') {
            this->identifier();
        } else {
            throw std::runtime_error("[line " + std::to_string(this->line) +
                                     "] Unexpected character: " + c);
        }
        break;
    }
}

void Lexer::addTokenWithLiteral(TokenType type,
                                std::optional<Literal> literal) {
    std::string lexeme = this->src.substr(this->start, this->cur - this->start);
    this->tokens.emplace_back(type, lexeme, literal, this->line);
}

void Lexer::addToken(TokenType type) {
    this->addTokenWithLiteral(type, std::nullopt);
}

void Lexer::number() {
    while (std::isdigit(this->peek())) {
        this->advance();
    }

    bool isFloat = false;

    if (this->peek() == '.' && std::isdigit(this->peekNext())) {
        isFloat = true;

        this->advance(); // consume '.'

        while (std::isdigit(this->peek())) {
            this->advance();
        }
    }

    std::string lexeme = this->src.substr(this->start, this->cur - this->start);

    if (isFloat) {
        double value = std::stod(lexeme);
        this->addTokenWithLiteral(TokenType::Number, Literal(value));
    } else {
        int value = std::stoi(lexeme);
        this->addTokenWithLiteral(TokenType::Number, Literal(value));
    }
}

void Lexer::identifier() {
    while (std::isalnum(this->peek())) {
        this->advance();
    }

    std::string lexeme = this->src.substr(this->start, this->cur - this->start);
    if (lexeme == "true") {
        this->addTokenWithLiteral(TokenType::Boolean, Literal(true));
    } else if (lexeme == "false") {
        this->addTokenWithLiteral(TokenType::Boolean, Literal(false));
    } // TODO: add keywords
    else if (lexeme == "fn") {
        this->addToken(TokenType::Fn);
    } else if (lexeme == "let") {
        this->addToken(TokenType::Let);
    } else if (lexeme == "return") {
        this->addToken(TokenType::Return);
    } else {
        this->addToken(TokenType::Identifier);
    }
}

std::string Lexer::readFile(const std::string &infile) {
    std::ifstream file(infile);
    if (!file.is_open()) {
        throw std::runtime_error("File `" + infile + "` not found");
    }

    std::stringstream ss;
    ss << file.rdbuf();
    std::string str = ss.str();

    file.close();

    return str;
}

char Lexer::advance() {
    return (this->isAtEnd()) ? '\0' : this->src[this->cur++];
}

char Lexer::peek() { return (this->isAtEnd()) ? '\0' : this->src[this->cur]; }

char Lexer::peekNext() {
    return (this->cur + 1 >= (int)this->src.length())
               ? '\0'
               : this->src[this->cur + 1];
}

bool Lexer::match(char expected) {
    if (this->isAtEnd())
        return false;

    if (this->src[this->cur] != expected)
        return false;

    this->cur++;
    return true;
}

bool Lexer::isAtEnd() { return this->cur >= (int)this->src.length(); }
