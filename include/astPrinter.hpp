#pragma once

#include "ast.hpp"

struct ASTPrinter : ASTVisitor {
    int indentLevel = 0;

    void visit(LiteralExpr &expr) override;
    void visit(VariableExpr &expr) override;
    void visit(BinaryExpr &expr) override;
    void visit(UnaryExpr &expr) override;

    void visit(BlockStmt &stmt) override;
    void visit(FnStmt &stmt) override;
    void visit(LetStmt &stmt) override;
    void visit(ReturnStmt &stmt) override;

    void visit(Program &stmt) override;

  private:
    void printIndent() const;
};
