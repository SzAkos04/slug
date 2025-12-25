#include "ast.hpp"
#include "token.hpp"

#include <sstream>
#include <stdexcept>

void LiteralExpr::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void VariableExpr::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void BinaryExpr::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void UnaryExpr::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void CallExpr::accept(ASTVisitor &visitor) { visitor.visit(*this); }

void ExpressionStmt::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void BlockStmt::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void FnStmt::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void LetStmt::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ReturnStmt::accept(ASTVisitor &visitor) { visitor.visit(*this); }

void Program::accept(ASTVisitor &visitor) { visitor.visit(*this); }

BinaryOp tokenTypeToBinaryOp(TokenType tt) {
    switch (tt) {
    case TokenType::Plus:
        return BinaryOp::Add;
    case TokenType::Minus:
        return BinaryOp::Sub;
    case TokenType::Star:
        return BinaryOp::Mul;
    case TokenType::Slash:
        return BinaryOp::Div;
    case TokenType::Percent:
        return BinaryOp::Mod;
    case TokenType::EqualEqual:
        return BinaryOp::Eq;
    case TokenType::BangEqual:
        return BinaryOp::Neq;
    case TokenType::Less:
        return BinaryOp::Lt;
    case TokenType::LessEqual:
        return BinaryOp::Lte;
    case TokenType::Greater:
        return BinaryOp::Gt;
    case TokenType::GreaterEqual:
        return BinaryOp::Gte;
    default:
        std::ostringstream oss;
        oss << "Unknown binary operator: '" << tt << "'";
        throw std::runtime_error(oss.str());
    }
}

UnaryOp tokenTypeToUnaryOp(TokenType tt) {
    switch (tt) {
    case TokenType::Minus:
        return UnaryOp::Negate;
    case TokenType::Bang:
        return UnaryOp::Not;
    default:
        std::ostringstream oss;
        oss << "Unknown unary operator: '" << tt << "'";
        throw std::runtime_error(oss.str());
    }
}
