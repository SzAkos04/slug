#include "ast.hpp"
#include "astPrinter.hpp"

#include <iostream>
#include <variant>

void ASTPrinter::visit(LiteralExpr &expr) {
    this->printIndent();
    std::visit([&](auto &&value) { std::cout << value; }, expr.value.get());
}

void ASTPrinter::visit(VariableExpr &expr) {
    this->printIndent();
    std::cout << expr.name;
}

void ASTPrinter::visit(BinaryExpr &expr) {
    this->printIndent();
    std::cout << "(";
    expr.lhs->accept(*this);

    switch (expr.op) {
    case BinaryOp::Add:
        std::cout << " + ";
        break;
    case BinaryOp::Sub:
        std::cout << " - ";
        break;
    case BinaryOp::Mul:
        std::cout << " * ";
        break;
    case BinaryOp::Div:
        std::cout << " / ";
        break;
    }

    expr.rhs->accept(*this);
    std::cout << ")";
}

void ASTPrinter::visit(UnaryExpr &expr) {
    this->printIndent();

    switch (expr.op) {
    case UnaryOp::Negate:
        std::cout << "-";
        break;
    case UnaryOp::Not:
        std::cout << "!";
        break;
    }

    expr.operand->accept(*this);
}

void ASTPrinter::visit(BlockStmt &stmt) {
    this->printIndent();
    std::cout << "{" << std::endl;
    ++indentLevel;
    for (const auto &s : stmt.stmts) {
        s->accept(*this);
    }
    --indentLevel;
    this->printIndent();
    std::cout << "}" << std::endl;
}

void ASTPrinter::visit(FnStmt &stmt) {
    this->printIndent();
    std::cout << "fn " << stmt.name << "(";
    for (size_t i = 0; i < stmt.params.size(); ++i) {
        std::cout << stmt.params[i].name << ": " << stmt.params[i].type.kind;
        if (i + 1 < stmt.params.size()) {
            std::cout << ", ";
        }
    }
    std::cout << ") -> " << stmt.retType.kind << " ";
    stmt.body->accept(*this);
    std::cout << std::endl;
}

void ASTPrinter::visit(LetStmt &stmt) {
    this->printIndent();
    std::cout << "let " << stmt.name << ": " << stmt.type.kind << " = ";
    if (dynamic_cast<LiteralExpr *>(stmt.initializer.get())) {
        stmt.initializer->accept(*this);
        std::cout << ";" << std::endl;
    } else {
        std::cout << std::endl;
        ++indentLevel;
        stmt.initializer->accept(*this);
        --indentLevel;
    }
}

void ASTPrinter::visit(ReturnStmt &stmt) {
    this->printIndent();
    if (stmt.value.has_value()) {
        std::cout << "return ";
        if (dynamic_cast<LiteralExpr *>(stmt.value->get())) {
            stmt.value->get()->accept(*this);
            std::cout << ";" << std::endl;
        } else {
            std::cout << std::endl;
            ++indentLevel;
            stmt.value->get()->accept(*this);
            --indentLevel;
        }
    } else {
        std::cout << "return;" << std::endl;
    }
}

void ASTPrinter::visit(Program &stmt) {
    for (const auto &s : stmt.stmts) {
        s->accept(*this);
    }
}
