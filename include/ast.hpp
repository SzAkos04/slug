#pragma once

#include "literal.hpp"
#include "token.hpp"
#include "type.hpp"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

struct ASTVisitor;

struct ASTNode {
    int line = 0;
    int column = 0;
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor &visitor) = 0;
};

struct Expr : ASTNode {};
struct Stmt : ASTNode {};

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

struct LiteralExpr : Expr {
    Literal value;

    explicit LiteralExpr(Literal value) : value(value) {}
    void accept(ASTVisitor &visitor) override;
};

struct VariableExpr : Expr {
    std::string name;

    explicit VariableExpr(std::string name) : name(std::move(name)) {}
    void accept(ASTVisitor &visitor) override;
};

enum class BinaryOp {
    Add, // x + y
    Sub, // x - y
    Mul, // x * y
    Div, // x / y
    Mod, // x % y

    Eq,  // x == y
    Neq, // x != y
    Lt,  // x < y
    Lte, // x <= y
    Gt,  // x > y
    Gte, // x >= y

    // TODO: And, Or
};
BinaryOp tokenTypeToBinaryOp(TokenType tt);

struct BinaryExpr : Expr {
    BinaryOp op;
    ExprPtr lhs;
    ExprPtr rhs;

    BinaryExpr(BinaryOp op, ExprPtr lhs, ExprPtr rhs)
        : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    void accept(ASTVisitor &visitor) override;
};

enum class UnaryOp {
    Negate, // -x
    Not,    // !x
};
UnaryOp tokenTypeToUnaryOp(TokenType tt);

struct UnaryExpr : Expr {
    UnaryOp op;
    ExprPtr operand;

    UnaryExpr(UnaryOp op, ExprPtr operand)
        : op(op), operand(std::move(operand)) {}
    void accept(ASTVisitor &visitor) override;
};

struct BlockStmt : Stmt {
    std::vector<StmtPtr> stmts;

    explicit BlockStmt(std::vector<StmtPtr> &&stmts)
        : stmts(std::move(stmts)) {}

    void accept(ASTVisitor &visitor) override;
};

struct FnParam {
    std::string name;
    Type type;

    FnParam(std::string name, Type type) : name(std::move(name)), type(type) {}
};

struct FnStmt : Stmt {
    std::string name;
    std::vector<FnParam> params;
    Type retType;
    std::unique_ptr<BlockStmt> body;

    explicit FnStmt(std::string name, std::vector<FnParam> params, Type retType,
                    std::unique_ptr<BlockStmt> body)
        : name(std::move(name)), params(std::move(params)), retType(retType),
          body(std::move(body)) {}

    void accept(ASTVisitor &visitor) override;
};

struct LetStmt : Stmt {
    std::string name;
    bool mut = false;
    Type type;
    ExprPtr initializer;

    explicit LetStmt(std::string name, bool mut, Type type, ExprPtr initializer)
        : name(std::move(name)), mut(mut), type(type),
          initializer(std::move(initializer)) {}

    void accept(ASTVisitor &visitor) override;
};

struct ReturnStmt : Stmt {
    std::optional<ExprPtr> value;

    ReturnStmt() : value(std::nullopt) {}
    explicit ReturnStmt(ExprPtr value) : value(std::move(value)) {}

    void accept(ASTVisitor &visitor) override;
};

struct Program : ASTNode {
    std::vector<StmtPtr> stmts;

    void accept(ASTVisitor &visitor) override;
};

struct ASTVisitor {
    virtual ~ASTVisitor() = default;

    virtual void visit(LiteralExpr &expr) = 0;
    virtual void visit(VariableExpr &expr) = 0;
    virtual void visit(BinaryExpr &expr) = 0;
    virtual void visit(UnaryExpr &expr) = 0;

    virtual void visit(BlockStmt &stmt) = 0;
    virtual void visit(FnStmt &stmt) = 0;
    virtual void visit(LetStmt &stmt) = 0;
    virtual void visit(ReturnStmt &stmt) = 0;

    virtual void visit(Program &stmt) = 0;
};
