#include "ast.hpp"
#include "codegen.hpp"
#include "type.hpp"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

void LLVMCodeGen::emitObjectFile(const std::string &filename) {
    // 1. Initialize all targets for the host machine
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    // 2. Define the target triple (use host machine's triple)
    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    this->module->setTargetTriple(targetTriple);

    // 3. Look up the target
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    if (!target) {
        throw std::runtime_error(error);
    }

    // 4. Configure the Target Machine
    auto CPU = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    std::optional<llvm::Reloc::Model> RM = llvm::Reloc::PIC_;
    auto targetMachine =
        target->createTargetMachine(targetTriple, CPU, features, opt, RM);

    // 5. Set Data Layout (important for pointer sizes, etc.)
    this->module->setDataLayout(targetMachine->createDataLayout());

    // 6. Open the output file
    std::error_code ec;
    llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);
    if (ec) {
        throw std::runtime_error("Could not open file: " + ec.message());
    }

    // 7. Run the pass manager to emit the object file
    llvm::legacy::PassManager pass;
    auto fileType =
        llvm::CodeGenFileType::ObjectFile; // Use .CGFT_ObjectFile in newer LLVM

    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
        throw std::runtime_error(
            "TargetMachine can't emit a file of this type");
    }

    pass.run(*this->module);
    dest.flush();
}

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
    auto it = this->symbolTable.find(expr.name);
    if (it == this->symbolTable.end()) {
        throw std::runtime_error("Undefined variable: " + expr.name);

        VariableInfo &info = it->second;
        llvm::Value *ptr = info.value;
        llvm::Type *ptrTy = this->toLLVMType(*info.type);

        this->lastValue = this->builder.CreateLoad(ptrTy, ptr, "loadtmp");
    }
}

void LLVMCodeGen::visit(BinaryExpr &expr) {
    // generate lhs
    expr.lhs->accept(*this);
    llvm::Value *lhsValue = this->lastValue;

    // generate rhs
    expr.rhs->accept(*this);
    llvm::Value *rhsValue = this->lastValue;

    bool isFP = lhsValue->getType()->isFloatingPointTy() ||
                rhsValue->getType()->isFloatingPointTy();

    switch (expr.op) {
    case BinaryOp::Add:
        this->lastValue =
            isFP ? this->builder.CreateFAdd(lhsValue, rhsValue, "addtmp")
                 : this->builder.CreateAdd(lhsValue, rhsValue, "addtmp");
        break;
    case BinaryOp::Sub:
        this->lastValue =
            isFP ? this->builder.CreateFSub(lhsValue, rhsValue, "subtmp")
                 : this->builder.CreateSub(lhsValue, rhsValue, "subtmp");
        break;
    case BinaryOp::Mul:
        this->lastValue =
            isFP ? this->builder.CreateFMul(lhsValue, rhsValue, "multmp")
                 : this->builder.CreateMul(lhsValue, rhsValue, "multmp");
        break;
    case BinaryOp::Div:
        this->lastValue =
            isFP ? this->builder.CreateFDiv(lhsValue, rhsValue, "divtmp")
                 : this->builder.CreateSDiv(lhsValue, rhsValue, "divtmp");
        break;
    case BinaryOp::Mod:
        this->lastValue =
            isFP ? this->builder.CreateFRem(lhsValue, rhsValue, "modtmp")
                 : this->builder.CreateSRem(lhsValue, rhsValue, "modtmp");

    case BinaryOp::Eq:
        this->lastValue =
            isFP ? this->builder.CreateFCmpOEQ(lhsValue, rhsValue, "cmptmp")
                 : this->lastValue =
                       this->builder.CreateICmpEQ(lhsValue, rhsValue, "cmptmp");
    case BinaryOp::Neq:
        this->lastValue =
            isFP ? this->builder.CreateFCmpONE(lhsValue, rhsValue, "netmp")
                 : this->builder.CreateICmpNE(lhsValue, rhsValue, "netmp");
        break;
    case BinaryOp::Lt:
        this->lastValue =
            isFP ? this->builder.CreateFCmpOLT(lhsValue, rhsValue, "lttmp")
                 : this->builder.CreateICmpSLT(lhsValue, rhsValue, "lttmp");
        break;
    case BinaryOp::Lte:
        this->lastValue =
            isFP ? this->builder.CreateFCmpOLE(lhsValue, rhsValue, "ltetmp")
                 : this->builder.CreateICmpSLE(lhsValue, rhsValue, "ltetmp");
        break;
    case BinaryOp::Gt:
        this->lastValue =
            isFP ? this->builder.CreateFCmpOGT(lhsValue, rhsValue, "gttmp")
                 : this->builder.CreateICmpSGT(lhsValue, rhsValue, "gttmp");
        break;
    case BinaryOp::Gte:
        this->lastValue =
            isFP ? this->builder.CreateFCmpOGE(lhsValue, rhsValue, "gtetmp")
                 : this->builder.CreateICmpSGE(lhsValue, rhsValue, "gtetmp");
        break;
    }
}

void LLVMCodeGen::visit(UnaryExpr &expr) {
    (void)expr;
    throw std::runtime_error("unary expressions not yet implemented");
}

////////

void LLVMCodeGen::visit(BlockStmt &stmt) {
    for (const auto &stmt : stmt.stmts) {
        stmt->accept(*this);
    }
}

void LLVMCodeGen::visit(FnStmt &stmt) {
    this->curFunc = &stmt;
    llvm::Function *func = this->module->getFunction(stmt.name);
    if (!func) {
        throw std::runtime_error("Use of undeclared function '" + stmt.name +
                                 "'");
    }

    llvm::BasicBlock *entry =
        llvm::BasicBlock::Create(*this->context, "entry", func);
    this->builder.SetInsertPoint(entry);

    unsigned idx = 0;
    for (auto &arg : func->args()) {
        const std::string &paramName = stmt.params[idx].name;
        arg.setName(paramName);
        this->declareSymbol(paramName, /*mut=*/false, &stmt.params[idx].type,
                            &arg);
        ++idx;
    }

    stmt.body->accept(*this);

    if (!this->builder.GetInsertBlock()->getTerminator()) {
        if (stmt.name == "main") {
            this->builder.CreateRet(llvm::ConstantInt::get(
                llvm::Type::getInt32Ty(*this->context), 0));
        } else {
            llvm::Type *retTy = func->getReturnType();
            if (retTy->isVoidTy()) {
                this->builder.CreateRetVoid();
            } else {
                this->builder.CreateRet(llvm::Constant::getNullValue(retTy));
            }
        }
    }

    // clear local symbols
    for (auto &param : stmt.params) {
        this->symbolTable.erase(param.name);
    }
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
        stmt.initializer.get()->accept(*this);
        llvm::Value *initVal = this->lastValue;

        this->builder.CreateStore(initVal, alloca);
    } else {
        llvm::Value *zeroVal = llvm::Constant::getNullValue(llvmTy);
        this->builder.CreateStore(zeroVal, alloca);
    }

    this->declareSymbol(stmt.name, stmt.mut, &stmt.type, alloca);
}

void LLVMCodeGen::visit(ReturnStmt &stmt) {
    llvm::Function *curFunc = this->builder.GetInsertBlock()->getParent();
    llvm::Type *expectedRetTy = curFunc->getReturnType();
    std::string funcName = curFunc->getName().str();

    if (stmt.value.has_value()) {
        // check return type in source code so e.g. `return 5;` is not possible
        // in `main()`
        if (this->curFunc->retType.kind == PrimitiveType::Void) {
            throw std::runtime_error(
                "Error in function '" + funcName +
                "': " + "cannot return a value from a void function.");
        }

        stmt.value->get()->accept(*this);
        llvm::Value *retVal = this->lastValue;

        if (retVal->getType() != expectedRetTy) {
            std::string actualTyStr, expectedTyStr;
            llvm::raw_string_ostream actualOS(actualTyStr),
                expectedOS(expectedTyStr);
            retVal->getType()->print(actualOS);
            expectedRetTy->print(expectedOS);

            throw std::runtime_error("Type mismatch in function '" + funcName +
                                     "': " + "returning '" + actualOS.str() +
                                     "' but expected '" + expectedOS.str() +
                                     "'");
        }

        this->builder.CreateRet(retVal);
    } else {
        if (expectedRetTy->isVoidTy()) {
            this->builder.CreateRetVoid();
        } else if (expectedRetTy->isIntegerTy(32) &&
                   curFunc->getName() == "main") {
            // main function returns void in source code but it should return
            // i32 in llvm ir
            this->builder.CreateRet(llvm::ConstantInt::get(expectedRetTy, 0));
        } else {
            throw std::runtime_error(
                "Empty return in function with non-void return type.");
        }
    }
}

void LLVMCodeGen::visit(Program &stmt) {
    this->declareGlobals(stmt);

    bool hasMain = false;

    for (const auto &stmt : stmt.stmts) {
        if (auto *fn = dynamic_cast<FnStmt *>(stmt.get())) {
            if (fn->name == "main") {
                hasMain = true;
            }
            fn->accept(*this);
        } else {
            throw std::runtime_error("Top level code not yet implemented");
        }

        if (!hasMain) {
            throw std::runtime_error("Program is missing 'fn main(): void");
        }
    }

    // verify IR
    std::string errStr;
    llvm::raw_string_ostream errStream(errStr);

    if (llvm::verifyModule(*this->module, &errStream)) {
        this->dumpIR();
        throw std::runtime_error("IR verification failed:\n" + errStream.str());
    }
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

void LLVMCodeGen::declareSymbol(const std::string &name, bool mut,
                                const Type *type, llvm::Value *value) {
    this->symbolTable.insert_or_assign(
        name, VariableInfo(value, mut, std::make_unique<Type>(*type)));
}

void LLVMCodeGen::declareGlobals(const Program &program) {
    for (const auto &stmt : program.stmts) {
        if (auto *fn = dynamic_cast<FnStmt *>(stmt.get())) {
            this->declareSymbol(fn->name, /*mut=*/false, /*type=*/&fn->retType,
                                this->generateFnPrototype(*fn));
        } else if (auto *let = dynamic_cast<LetStmt *>(stmt.get())) {
            this->declareGlobalVariable(*let);
        } else {
            throw std::runtime_error("Only functions and variable declarations "
                                     "can be declared at top level");
        }
    }
}

llvm::Value *LLVMCodeGen::generateFnPrototype(const FnStmt &fn) {
    std::vector<llvm::Type *> paramTypes;
    for (const auto &param : fn.params) {
        paramTypes.push_back(this->toLLVMType(param.type));
    }

    llvm::Type *retType = fn.name == "main"
                              ? llvm::Type::getInt32Ty(*this->context)
                              : this->toLLVMType(fn.retType);

    llvm::FunctionType *fnType =
        llvm::FunctionType::get(retType, paramTypes, /*isVarArg=*/false);

    llvm::Function *function = llvm::Function::Create(
        fnType, llvm::Function::ExternalLinkage, fn.name, *this->module);

    unsigned idx = 0;
    for (auto &arg : function->args()) {
        arg.setName(fn.params[idx++].name);
    }

    return function;
}

void LLVMCodeGen::declareGlobalVariable(const LetStmt &let) {
    llvm::Constant *initConstant = nullptr;

    if (let.initializer) {
        let.initializer.get()->accept(*this);
        llvm::Value *initVal = this->lastValue;

        if (llvm::isa<llvm::ConstantExpr>(initVal) ||
            llvm::isa<llvm::Constant>(initVal)) {
            initConstant = llvm::cast<llvm::Constant>(initVal);
        } else {
            throw std::runtime_error(
                "Global variable initializer must be constant");
        }
    } else {
        llvm::Type *llvmTy = this->toLLVMType(let.type);
        initConstant = llvm::Constant::getNullValue(llvmTy);
    }

    llvm::Type *llvmTy = initConstant->getType();

    llvm::GlobalVariable *globalVar = new llvm::GlobalVariable(
        *this->module, llvmTy, /*isConstant=*/!let.mut,
        llvm::GlobalValue::ExternalLinkage, initConstant, let.name);

    this->declareSymbol(let.name, let.mut, &let.type, globalVar);
}
