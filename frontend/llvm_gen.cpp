//
// Created by 田地 on 2021/7/20.
//

#include <vector>
#include <memory>
#include <string>

#include <stdlib.h>

#include "llvm-c/Core.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/IR/Verifier.h"

#include "llvm_gen.h"
#include "stable.h"

using namespace std;
using namespace cool;
using namespace irgen;
using namespace llvm;

/*-------------------------------------*/
/* Symbol Table For LLVM IR Generator  */
/*-------------------------------------*/
LLVMGen::SymbolTable::SymbolTable(adt::ScopedTableSpecializer<adt::SymbolTable> &_stable)
: stable(_stable) {
    for (int i = 0; i < stable.Size(); i++) valueTable.insert({i, {}});
    stable.InitTraverse();
}

// todo: recursively look up
llvm::Value* LLVMGen::SymbolTable::get(const string& name) {
    return valueTable.at(stable.Idx()).at(name);
}

void LLVMGen::SymbolTable::insert(const string& name, llvm::Value* value) {
    valueTable.at(stable.Idx()).insert({name, value});
}

void LLVMGen::SymbolTable::InsertLocalVar(const string& name, llvm::Value* value) {
    insert(name, value);
}

void LLVMGen::SymbolTable::InsertSelfVar(llvm::Value* value) {
    insert("self", value);
}

void LLVMGen::SymbolTable::InsertArg(const string& name, llvm::Value* value) {
    insert(name, value);
}

llvm::Value * LLVMGen::SymbolTable::GetLocalVar(const string &name) { return get(name); }

llvm::Value * LLVMGen::SymbolTable::GetSelfVar() { return get("self"); }

llvm::Value * LLVMGen::SymbolTable::GetSelfField(uint32_t i) {

}

llvm::Value * LLVMGen::SymbolTable::GetArg(const string &name) { return get(name); }

/*-------------------------------------*/
/*          LLVM IR Generator          */
/*-------------------------------------*/
LLVMGen::LLVMGen(adt::ScopedTableSpecializer<adt::SymbolTable>& _stable)
: stable(_stable), llvmStable(_stable) {
    // initialize fields
    context = make_unique<LLVMContext>();
    builder = make_unique<IRBuilder<>>(*context);
    module = make_unique<Module>("main", *context);

    // initialize llvm
    InitializeNativeTarget();
    InitializeNativeTargetAsmParser();
    InitializeNativeTargetAsmPrinter();

    auto TargetTriple = sys::getDefaultTargetTriple();
    std::string error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, error);
    if (!Target) throw runtime_error("target not found: " + error);

    auto CPU = "generic";
    auto Features = "";

    TargetOptions opt;
    auto RM = Optional<Reloc::Model>();
    target = unique_ptr<TargetMachine>(Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM));

    module->setDataLayout(target->createDataLayout());
    module->setTargetTriple(TargetTriple);
}

StructType* LLVMGen::CreateOpaqueStructTypeIfNx(const string &name) {
    StructType *st = StructType::getTypeByName(*context, name);
    if (!st)
        st = StructType::create(*context, name);
    return st;
}

llvm::StructType* LLVMGen::GetStringLLVMType() {
    return StructType::get(
        *context,
        {Type::getInt32Ty(*context),
         PointerType::get(Type::getInt8Ty(*context), 0)});
}

Type* LLVMGen::GetLLVMType(const string& name) {
    if (name == "Int") {
        return Type::getInt32Ty(*context);
    } else if (name == "Bool") {
        return Type::getInt1Ty(*context);
    } else if (name == "String") {
        return GetStringLLVMType();
    }
    return CreateOpaqueStructTypeIfNx(name);
}

llvm::Value* LLVMGen::GetStringConstant(const string& str) {
    vector<Constant*> data;
    for (auto& c : str) data.emplace_back(ConstantInt::get(Type::getInt8Ty(*context), c));
    return ConstantArray::get(
        ArrayType::get(Type::getInt8Ty(*context), str.size()), data);
}

Function* LLVMGen::CreateFunctionDeclIfNx(const string& name, const string& type, const string& selfType,
    vector<shared_ptr<Formal>>& args) {

    vector<Type *> argTypes;
    argTypes.emplace_back(GetLLVMType(selfType));
    for(auto& arg : args) argTypes.emplace_back(GetLLVMType(arg->type.val));

    FunctionType* ft = FunctionType::get(GetLLVMType(type), argTypes, false);

    Function* function = Function::Create(ft, Function::ExternalLinkage, name, module.get());

    unsigned idx = 0;
    for (auto& arg : function->args()) {
        if (idx == 0) arg.setName("self");
        else arg.setName(args.at(idx-1)->name.val);
        idx++;
    }

    return function;
}

llvm::Function* LLVMGen::CreateFunctionMain() {
    FunctionType* ft = FunctionType::get(Type::getVoidTy(*context), {}, false);
    return Function::Create(ft, Function::ExternalLinkage, "coolmain", module.get());
}

void LLVMGen::CreateRuntimeFunctionDecls() {
    // create runtime/runtime.h: int entry();
    Type* int32Type = Type::getInt32Ty(*context);
    FunctionType* entryFt = FunctionType::get(int32Type, false);
    Function* function = Function::Create(entryFt, Function::ExternalLinkage, "entry", module.get());
}

void LLVMGen::DumpTextualIR(const string &filename) {
    std::error_code ec;
    raw_fd_ostream dest(filename, ec, sys::fs::OF_Text);
    if (ec)
        throw runtime_error("create raw_fd_ostream error: " + ec.message());

    module->print(dest, nullptr);

    dest.close();

    if (dest.has_error())
        throw runtime_error("print to destination error: " + dest.error().message());
}

void LLVMGen::EmitObjectFile(const string &filename) {
    std::error_code ec;
    raw_fd_ostream dest(filename, ec, sys::fs::OF_None);
    if (ec)
        throw runtime_error("create raw_fd_ostream error: " + ec.message());

    legacy::PassManager pass;
    auto FileType = CGFT_ObjectFile;

    if (target->addPassesToEmitFile(pass, dest, nullptr, FileType))
        throw runtime_error("target machine can't emit a file of this type");

    pass.run(*module);
    dest.flush();
}

Value * LLVMGen::Visit(Program &prog) {
    ENTER_SCOPE_GUARD(stable, {
        CreateRuntimeFunctionDecls();
        for (auto& cls : prog.GetClasses()) Visit(*cls);
    })
}

void LLVMGen::Visit(Class &cls) {
    ENTER_SCOPE_GUARD(stable, {
        vector<Type *> Fields;
        for (auto& feat : cls.GetFieldFeatures()) Fields.emplace_back(Visit(*feat));

        StructType* ST = CreateOpaqueStructTypeIfNx(cls.name.val);

        ST->setBody(Fields, false);

        for (auto& feat : cls.GetFuncFeatures()) Visit(*feat);
    })
}

Value * LLVMGen::Visit(FuncFeature &feat) {
    Function* function;
    stable.EnterScope();
//    ENTER_SCOPE_GUARD(stable, {

        if (stable.GetClass()->name.val == "Main" && feat.name.val == "main") {

            function = CreateFunctionMain();
            BasicBlock* bb = BasicBlock::Create(*context, "entry", function);
            builder->SetInsertPoint(bb);
            if (auto val = Visit(*feat.expr)) {
                builder->Insert(val);
                builder->CreateRetVoid();
                verifyFunction(*function);
            } else {
                assert(false);
            }

        } else {

            function = module->getFunction(feat.name.val);

            if (!function)
                function = CreateFunctionDeclIfNx(feat.name.val, feat.type.val, stable.GetClass()->name.val, feat.args);

            if (!function)
                throw runtime_error("create llvm function failed");

            BasicBlock* bb = BasicBlock::Create(*context, "entry", function);
            builder->SetInsertPoint(bb);

            int i = 0;
            for (auto& arg : function->args()) {
                if (i == 0) llvmStable.InsertSelfVar(function->args().begin());
                else llvmStable.InsertArg(arg.getName().str(), &arg);
                i++;
            }

            if (auto ret = Visit(*feat.expr)) {

                builder->CreateRet(ret);
                verifyFunction(*function);

            } else {

                function->eraseFromParent();
                function = nullptr;

            }

        }
//    })
    stable.LeaveScope();

    return function;
}

Type * LLVMGen::Visit(FieldFeature &feat) {
    return GetLLVMType(feat.type.val);
}

Value * LLVMGen::Visit(Formal& formal) {
    // todo
    return nullptr;     //    return builder->CreateAlloca(GetLLVMType(formal.type.val), nullptr, formal.name.val);
}

Value* LLVMGen::Visit(repr::Expr& expr) {
    return ExprVisitor<Value *>::Visit(expr);
}

Value* LLVMGen::Visit_(repr::LinkBuiltin& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Assign& expr) {
    builder->CreateStore(Visit(*expr.expr), Visit(*expr.id));
    return Visit(*expr.id);
}

Value* LLVMGen::Visit_(repr::Add& expr) {
    return builder->CreateAdd(Visit(*expr.left),Visit(*expr.right));
}

Value* LLVMGen::Visit_(repr::Block& expr) {
    Value* value;
    ENTER_SCOPE_GUARD(stable, {
        for (auto& eExpr : expr.exprs)
            value = Visit(*eExpr);
    })
    return value;
}

Value* LLVMGen::Visit_(repr::Case& expr) {
    // todo
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Call& expr) {
    Function* function = CreateFunctionDeclIfNx(expr.link->name.val, expr.link->type.val,
        stable.GetClass()->name.val, expr.link->args);
    vector<Value*> args;
    for (auto& arg : expr.args) args.emplace_back(Visit(*arg));
    return builder->CreateCall(function->getFunctionType(), function, args);
}

Value* LLVMGen::Visit_(repr::Divide& expr) {
    return builder->CreateSDiv(Visit(*expr.left), Visit(*expr.right));
}

Value* LLVMGen::Visit_(repr::Equal& expr) {
    // todo
    return nullptr;
}

Value* LLVMGen::Visit_(repr::False& expr) {
    return ConstantInt::getFalse(*context);
}

Value* LLVMGen::Visit_(repr::ID& expr) {
    auto idAttr = stable.GetIdAttr(expr.name.val);
    switch (idAttr->storageClass) {
        case attr::IdAttr::Field:
            return builder->CreateLoad(llvmStable.GetSelfField(idAttr->idx));
        case attr::IdAttr::Local:
            return builder->CreateLoad(llvmStable.GetLocalVar(idAttr->name));
        case attr::IdAttr::Arg:
            // todo: how to get arg?
            break;
        default:
            assert(false && "Invalid IdAttr:StorageClass Enum");
    }
}

Value* LLVMGen::Visit_(repr::IsVoid& expr) {
    // todo
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Integer& expr) {
    return ConstantInt::getIntegerValue(
        Type::getInt32Ty(*context),
        APInt(64, expr.val.val, true));
}

Value* LLVMGen::Visit_(repr::If& expr) {
    // todo
    return nullptr;
}

Value* LLVMGen::Visit_(repr::LessThanOrEqual& expr) {
    return builder->CreateICmp(CmpInst::ICMP_SLE, Visit(*expr.left), Visit(*expr.right));
}

Value* LLVMGen::Visit_(repr::LessThan& expr) {
    return builder->CreateICmp(CmpInst::ICMP_SLT, Visit(*expr.left), Visit(*expr.right));
}

Value* LLVMGen::Visit_(repr::Let& expr) {
    auto visitFormal = [&](repr::Let::Formal& formal) {
        auto alloca = builder->CreateAlloca(GetLLVMType(formal.type.val), nullptr, formal.name.val);
        if (formal.expr) builder->CreateStore(Visit(*formal.expr), alloca);
        return alloca;
    };
    Value* value;
    for (auto it = expr.formals.begin(); it != expr.formals.end(); it++) {
        ENTER_SCOPE_GUARD(stable, {
            llvmStable.InsertLocalVar((*it)->name.val, visitFormal(**it));
            if (it+1 == expr.formals.end())
                value = Visit(*expr.expr);
        })
    }
    return value;
}

Value* LLVMGen::Visit_(repr::MethodCall& expr) {
    // todo
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Multiply& expr) {
    return builder->CreateMul(Visit(*expr.left),Visit(*expr.right));
}

Value* LLVMGen::Visit_(repr::Minus& expr) {
    return builder->CreateSub(Visit(*expr.left),Visit(*expr.right));
}

Value* LLVMGen::Visit_(repr::Negate& expr) {
    return builder->CreateNeg(Visit(*expr.expr));
}

Value* LLVMGen::Visit_(repr::New& expr) {
    // todo
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Not& expr) {
    return builder->CreateNot(Visit(*expr.expr));
}

Value* LLVMGen::Visit_(repr::String& expr) {
    return GetStringConstant(expr.val.val);
}

Value* LLVMGen::Visit_(repr::True& expr) {
    return ConstantInt::getTrue(*context);
}

Value* LLVMGen::Visit_(repr::While& expr) {
    // todo
    return nullptr;
}
