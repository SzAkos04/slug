#pragma once

#include "literal.hpp"
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

struct LiteralExpr : Expr {
    Literal value;

    explicit LiteralExpr(Literal value) : value(value) {}
    void accept(ASTVisitor &visitor) override;
};

struct BlockStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;

    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> &&stmts)
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
    Type type;
    std::unique_ptr<Expr> initializer;

    explicit LetStmt(std::string name, Type type,
                     std::unique_ptr<Expr> initializer)
        : name(std::move(name)), type(type),
          initializer(std::move(initializer)) {}

    void accept(ASTVisitor &visitor) override;
};

struct ReturnStmt : Stmt {
    std::optional<std::unique_ptr<Expr>> value;

    ReturnStmt() : value(std::nullopt) {}
    explicit ReturnStmt(std::unique_ptr<Expr> value)
        : value(std::move(value)) {}

    void accept(ASTVisitor &visitor) override;
};

struct Program : ASTNode {
    std::vector<std::unique_ptr<Stmt>> stmts;

    void accept(ASTVisitor &visitor) override;
};

struct ASTVisitor {
    virtual ~ASTVisitor() = default;

    virtual void visit(LiteralExpr &expr) = 0;

    virtual void visit(BlockStmt &stmt) = 0;
    virtual void visit(FnStmt &stmt) = 0;
    virtual void visit(LetStmt &stmt) = 0;
    virtual void visit(ReturnStmt &stmt) = 0;

    virtual void visit(Program &stmt) = 0;
};
