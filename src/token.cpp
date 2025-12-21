#include "token.hpp"

std::ostream &operator<<(std::ostream &os, TokenType t) {
    switch (t) {
    case TokenType::LeftParen:
        return os << "LeftParen";
    case TokenType::RightParen:
        return os << "RightParen";
    case TokenType::LeftBrace:
        return os << "LeftBrace";
    case TokenType::RightBrace:
        return os << "RightBrace";
    case TokenType::Comma:
        return os << "Comma";
    case TokenType::Dot:
        return os << "Dot";
    case TokenType::Semicolon:
        return os << "Semicolon";
    case TokenType::Colon:
        return os << "Colon";

    case TokenType::Bang:
        return os << "Bang";
    case TokenType::BangEqual:
        return os << "BangEqual";
    case TokenType::Equal:
        return os << "Equal";
    case TokenType::EqualEqual:
        return os << "EqualEqual";

    case TokenType::Identifier:
        return os << "Identifier";
    case TokenType::String:
        return os << "String";
    case TokenType::Number:
        return os << "Number";
    case TokenType::Boolean:
        return os << "Boolean";

    case TokenType::Fn:
        return os << "Fn";
    case TokenType::Let:
        return os << "Let";
    case TokenType::Return:
        return os << "Return";

    case TokenType::Eof:
        return os << "Eof";
    }
    return os;
}
