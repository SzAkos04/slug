#include "astPrinter.hpp"

#include <iostream>
#include <variant>

void ASTPrinter::visit(LiteralExpr &expr) {
    printIndent();
    std::visit([&](auto &&value) { std::cout << value; }, expr.value.get());
}

void ASTPrinter::visit(BlockStmt &stmt) {
    printIndent();
    std::cout << "{" << std::endl;
    ++indentLevel;
    for (const auto &s : stmt.stmts) {
        s->accept(*this);
    }
    --indentLevel;
    printIndent();
    std::cout << "}" << std::endl;
}

void ASTPrinter::visit(FnStmt &stmt) {
    printIndent();
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
    printIndent();
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
    printIndent();
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
