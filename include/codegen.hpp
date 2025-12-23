#pragma once

#include "ast.hpp"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <memory>
#include <utility>

struct CodeGenVisitor : ASTVisitor {
    virtual llvm::Value *visitExpr(Expr &) = 0;
    virtual void visitStmt(Stmt &) = 0;
};

struct VariableInfo {
    llvm::Value *value;
    bool mut;
    std::unique_ptr<Type> type;

    VariableInfo(llvm::Value *value, bool mut, std::unique_ptr<Type> type)
        : value(std::move(value)), mut(mut), type(std::move(type)) {}
};

class LLVMCodeGen : public ASTVisitor {
  public:
    LLVMCodeGen()
        : context(std::make_unique<llvm::LLVMContext>()),
          module(std::make_unique<llvm::Module>("main", *context)),
          builder(*context) {}

    llvm::Module *getModule() { return module.get(); }

    // Expr visitors
    void visit(LiteralExpr &) override;
    void visit(VariableExpr &) override;
    void visit(BinaryExpr &) override;
    void visit(UnaryExpr &) override;

    // Stmt visitors
    void visit(BlockStmt &) override;
    void visit(FnStmt &) override;
    void visit(LetStmt &) override;
    void visit(ReturnStmt &) override;
    void visit(Program &) override;

  private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    llvm::IRBuilder<> builder;

    std::unordered_map<std::string, VariableInfo> locals;

    llvm::Value *lastValue = nullptr;

    llvm::Type *toLLVMType(const Type &type);
    void declareLocal(const std::string &name, bool mut, const Type *type,
                      llvm::Value *value);
};
