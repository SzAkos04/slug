#include "ast.hpp"

void LiteralExpr::accept(ASTVisitor &visitor) { visitor.visit(*this); }

void BlockStmt::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void FnStmt::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void LetStmt::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ReturnStmt::accept(ASTVisitor &visitor) { visitor.visit(*this); }

void Program::accept(ASTVisitor &visitor) { visitor.visit(*this); }
