#include "ast.hpp"
#include "astPrinter.hpp"

#include <iostream>
#include <variant>

void ASTPrinter::visit(LiteralExpr &expr) {
    std::visit([&](auto &&value) { std::cout << value; }, expr.value.get());
}

void ASTPrinter::visit(VariableExpr &expr) { std::cout << expr.name; }

void ASTPrinter::visit(BinaryExpr &expr) {
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
    case BinaryOp::Mod:
        std::cout << " % ";
        break;
    case BinaryOp::Eq:
        std::cout << " == ";
        break;
    case BinaryOp::Neq:
        std::cout << " != ";
        break;
    case BinaryOp::Lt:
        std::cout << " < ";
        break;
    case BinaryOp::Lte:
        std::cout << " <= ";
        break;
    case BinaryOp::Gt:
        std::cout << " > ";
        break;
    case BinaryOp::Gte:
        std::cout << " >= ";
        break;
    }

    expr.rhs->accept(*this);
    std::cout << ")";
}

void ASTPrinter::visit(UnaryExpr &expr) {
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

void ASTPrinter::visit(CallExpr &expr) {
    std::cout << expr.callee << "(";
    for (size_t i = 0; i < expr.args.size(); ++i) {
        expr.args[i]->accept(*this);
        if (i + 1 < expr.args.size()) {
            std::cout << ", ";
        }
    }
    std::cout << ")";
}

/////

void ASTPrinter::visit(ExpressionStmt &stmt) {
    this->printIndent();
    if (stmt.expr) {
        stmt.expr->accept(*this);
    }
    std::cout << ";" << std::endl;
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
    std::cout << "let " << (stmt.mut ? "mut " : "const ") << stmt.name << ": "
              << stmt.type.kind << " = ";
    if (dynamic_cast<LiteralExpr *>(stmt.initializer.get())) {
        stmt.initializer->accept(*this);
        std::cout << ";" << std::endl;
    } else {
        std::cout << std::endl;
        ++indentLevel;
        this->printIndent();
        stmt.initializer->accept(*this);
        --indentLevel;
        std::cout << ";" << std::endl;
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
            this->printIndent();
            stmt.value->get()->accept(*this);
            --indentLevel;
            std::cout << ";" << std::endl;
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

void ASTPrinter::printIndent() const {
    for (int i = 0; i < this->indentLevel; ++i) {
        std::cout << "  ";
    }
}
