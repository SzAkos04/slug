#pragma once

#include "ast.hpp"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

struct CodeGenVisitor : ASTVisitor {
    virtual llvm::Value *visitExpr(Expr &) = 0;
    virtual void visitStmt(Stmt &) = 0;
};

struct VariableInfo {
    llvm::Value *value;
    bool mut;
    std::shared_ptr<Type> type;

    VariableInfo(llvm::Value *value, bool mut, std::shared_ptr<Type> type)
        : value(std::move(value)), mut(mut), type(std::move(type)) {}
};

class LLVMCodeGen : public ASTVisitor {
  public:
    LLVMCodeGen()
        : context(std::make_unique<llvm::LLVMContext>()),
          module(std::make_unique<llvm::Module>("main", *context)),
          builder(*context) {}

    llvm::Module *getModule() { return module.get(); }

    void emitObjectFile(const std::string &filename);

    // Expr visitors
    void visit(LiteralExpr &) override;
    void visit(VariableExpr &) override;
    void visit(BinaryExpr &) override;
    void visit(UnaryExpr &) override;
    void visit(CallExpr &) override;

    // Stmt visitors
    void visit(ExpressionStmt &) override;
    void visit(BlockStmt &) override;
    void visit(FnStmt &) override;
    void visit(LetStmt &) override;
    void visit(ReturnStmt &) override;
    void visit(Program &) override;

    void dumpIR() const { this->module->print(llvm::outs(), nullptr); }

  private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    llvm::IRBuilder<> builder;

    // index 0 = global scope
    std::vector<std::unordered_map<std::string, VariableInfo>> scopeStack;

    void pushScope() { this->scopeStack.push_back({}); }
    void popScope() {
        if (!this->scopeStack.empty()) {
            this->scopeStack.pop_back();
        }
    }
    void declareSymbol(const std::string &name, bool mut, const Type *type,
                       llvm::Value *value);
    VariableInfo *findSymbol(const std::string &);
    void dumpScopes() const;

    llvm::Value *lastValue = nullptr;

    FnStmt *curFunc;

    void declareGlobals(const Program &);
    llvm::Value *generateFnPrototype(const FnStmt &);
    void generateFnBody(FnStmt &);
    void declareGlobalVariable(const LetStmt &);

    llvm::Type *toLLVMType(const Type &type);
};
