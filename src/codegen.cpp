#include "codegen.hpp"
#include "type.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <variant>

void LLVMCodeGen::visit(LiteralExpr &expr) {
    std::visit(
        [this](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int>) {
                this->lastValue = llvm::ConstantInt::get(
                    llvm::Type::getInt32Ty(*this->context), arg);
            } else if constexpr (std::is_same_v<T, double>) {
                this->lastValue = llvm::ConstantFP::get(
                    llvm::Type::getDoubleTy(*this->context), arg);
            } else if constexpr (std::is_same_v<T, bool>) {
                this->lastValue = llvm::ConstantInt::get(
                    llvm::Type::getInt1Ty(*this->context), arg);
            }
        },
        expr.value.get());
}

void LLVMCodeGen::visit(VariableExpr &expr) {
    auto it = this->locals.find(expr.name);
    if (it == this->locals.end()) {
        throw std::runtime_error("Undefined variable: " + expr.name);

        VariableInfo &info = it->second;
        llvm::Value *ptr = info.value;
        llvm::Type *ptrTy = this->toLLVMType(*info.type);

        this->lastValue = this->builder.CreateLoad(ptrTy, ptr, "loadtmp");
    }
}

void LLVMCodeGen::visit(BinaryExpr &expr) {
    (void)expr;
    throw std::runtime_error("not yet implemented");
}

void LLVMCodeGen::visit(UnaryExpr &expr) {
    (void)expr;
    throw std::runtime_error("not yet implemented");
}

////////

void LLVMCodeGen::visit(BlockStmt &stmt) {
    for (const auto &stmt : stmt.stmts) {
        stmt->accept(*this);
    }
}

void LLVMCodeGen::visit(FnStmt &stmt) {
    (void)stmt;
    throw std::runtime_error("not yet implemented");
}

void LLVMCodeGen::visit(LetStmt &stmt) {
    llvm::Type *llvmTy = this->toLLVMType(stmt.type);

    llvm::BasicBlock *currentBlock = this->builder.GetInsertBlock();
    if (!currentBlock) {
        throw std::runtime_error(
            "IRBuilder has no insertion block when allocating variable");
    }
    llvm::Function *func = currentBlock->getParent();
    llvm::IRBuilder<> tmpBuilder(&func->getEntryBlock(),
                                 func->getEntryBlock().begin());
    llvm::AllocaInst *alloca =
        tmpBuilder.CreateAlloca(llvmTy, nullptr, stmt.name);

    if (stmt.initializer) {
        stmt.initializer.get()->accept(*this); // FIXME: not sure
        llvm::Value *initVal = this->lastValue;

        this->builder.CreateStore(initVal, alloca);
    } else {
        llvm::Value *zeroVal = llvm::Constant::getNullValue(llvmTy);
        this->builder.CreateStore(zeroVal, alloca);
    }

    this->declareLocal(stmt.name, stmt.mut, &stmt.type, alloca);
}

void LLVMCodeGen::visit(ReturnStmt &stmt) {
    (void)stmt;
    throw std::runtime_error("not yet implemented");
}

void LLVMCodeGen::visit(Program &stmt) {
    (void)stmt;
    throw std::runtime_error("not yet implemented");
}

llvm::Type *LLVMCodeGen::toLLVMType(const Type &type) {
    switch (type.kind) {
    case PrimitiveType::Void:
        return llvm::Type::getVoidTy(*this->context);
    case PrimitiveType::I32:
        return llvm::Type::getInt32Ty(*this->context);
    case PrimitiveType::F64:
        return llvm::Type::getDoubleTy(*this->context);
    case PrimitiveType::Bool:
        return llvm::Type::getInt1Ty(*this->context);
    default:
        throw std::runtime_error("Unexpected type");
    }
}

void LLVMCodeGen::declareLocal(const std::string &name, bool mut,
                               const Type *type, llvm::Value *value) {
    this->locals.insert_or_assign(
        name, VariableInfo(value, mut, std::make_unique<Type>(*type)));
}
