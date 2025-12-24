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

    case TokenType::Equal:
        return os << "Equal";
    case TokenType::EqualEqual:
        return os << "EqualEqual";
    case TokenType::Bang:
        return os << "Bang";
    case TokenType::BangEqual:
        return os << "BangEqual";
    case TokenType::Plus:
        return os << "Plus";
    case TokenType::PlusEqual:
        return os << "PlusEqual";
    case TokenType::Minus:
        return os << "Minus";
    case TokenType::MinusEqual:
        return os << "MinusEqual";
    case TokenType::Star:
        return os << "Star";
    case TokenType::StarEqual:
        return os << "StarEqual";
    case TokenType::Slash:
        return os << "Slash";
    case TokenType::SlashEqual:
        return os << "SlashEqual";
    case TokenType::Percent:
        return os << "Percent";
    case TokenType::PercentEqual:
        return os << "PercentEqual";
    case TokenType::Less:
        return os << "Less";
    case TokenType::LessEqual:
        return os << "LessEqual";
    case TokenType::Greater:
        return os << "Greater";
    case TokenType::GreaterEqual:
        return os << "GreaterEqual";

    case TokenType::Identifier:
        return os << "Identifier";

    case TokenType::Fn:
        return os << "Fn";
    case TokenType::Let:
        return os << "Let";
    case TokenType::Mut:
        return os << "Mut";
    case TokenType::Return:
        return os << "Return";

    case TokenType::Number:
        return os << "Number";
    case TokenType::True:
        return os << "True";
    case TokenType::False:
        return os << "False";

    case TokenType::Eof:
        return os << "Eof";
    }
    return os;
}
