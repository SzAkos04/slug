#pragma once

#include "ast.hpp"

#include <iostream>

struct ASTPrinter : ASTVisitor {
    int indentLevel = 0;

    void visit(LiteralExpr &expr) override;

    void visit(BlockStmt &stmt) override;
    void visit(FnStmt &stmt) override;
    void visit(LetStmt &stmt) override;
    void visit(ReturnStmt &stmt) override;

    void visit(Program &stmt) override;

  private:
    void printIndent() const {
        for (int i = 0; i < this->indentLevel; ++i) {
            std::cout << "  ";
        }
    }
};
