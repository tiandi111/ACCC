//
// Created by 田地 on 2021/7/20.
//

#include <vector>
#include <memory>

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

using namespace std;
using namespace cool;
using namespace irgen;
using namespace llvm;

LLVMGen::LLVMGen() {
    context = make_unique<LLVMContext>();
    builder = make_unique<IRBuilder<>>(*context);
    module = make_unique<Module>("main", *context);

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

Type* LLVMGen::GetLLVMType(const string& name) {
    if (name == "Int") {
        return Type::getInt32Ty(*context);
    } else if (name == "Bool") {
        return Type::getInt1Ty(*context);
    }
    return CreateOpaqueStructTypeIfNx(name);
}

Function* LLVMGen::CreateFunctionDeclIfNx(const string& name, const string& type, vector<shared_ptr<Formal>>& args) {
    Type* returnType = GetLLVMType(type);

    vector<Type *> argTypes(args.size());
    for(auto& arg : args) argTypes.emplace_back(Visit(*arg));

    FunctionType* ft = FunctionType::get(returnType, argTypes, false);

    Function* function = Function::Create(ft, Function::ExternalLinkage, name, module.get());

    unsigned idx = 0;
    for (auto& arg : function->args())
        arg.setName(args.at(idx++)->name.val);

    return function;
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
    CreateRuntimeFunctionDecls();
    for (auto& cls : prog.GetClasses()) Visit(*cls);
}

void LLVMGen::Visit(Class &cls) {
    vector<Type *> Attributes;
    for (auto& feat : cls.GetFieldFeatures()) Attributes.emplace_back(Visit(*feat));

    StructType* ST = CreateOpaqueStructTypeIfNx(cls.name.val);

    ST->setBody(Attributes, false);

    for (auto& feat : cls.GetFuncFeatures()) Visit(*feat);
}

Value * LLVMGen::Visit(FuncFeature &feat) {
    Function* function = module->getFunction(feat.name.val);

    if (!function)
        function = CreateFunctionDeclIfNx(feat.name.val, feat.type.val, feat.args);

    if (!function)
        throw "create llvm function failed";

    BasicBlock* bb = BasicBlock::Create(*context, "entry", function);
    builder->SetInsertPoint(bb);

    if (auto ret = ExprVisitor<Value*>::Visit(*feat.expr)) {
        builder->CreateRet(ret);

        verifyFunction(*function);

        return function;
    }

    function->eraseFromParent();
    return nullptr;
}

Type * LLVMGen::Visit(FieldFeature &feat) {
    return GetLLVMType(feat.type.val);
}

Type * LLVMGen::Visit(Formal& formal) {
    return GetLLVMType(formal.type.val);
}

Value* LLVMGen::Visit_(repr::LinkBuiltin& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Assign& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Add& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Block& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Case& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Call& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Divide& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Equal& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::False& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::ID& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::IsVoid& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Integer& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::If& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::LessThanOrEqual& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::LessThan& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Let& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::MethodCall& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Multiply& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Minus& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Negate& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::New& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Not& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::String& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::True& expr) {
    return nullptr;
}

Value* LLVMGen::Visit_(repr::While& expr) {
    return nullptr;
}
