//
// Created by 田地 on 2021/7/20.
//

#ifndef COOL_LLVM_H
#define COOL_LLVM_H

#include <memory>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Target/TargetMachine.h"

#include "visitor.h"
#include "repr.h"

namespace cool {

using namespace std;
using namespace visitor;
using namespace repr;

namespace irgen {

class LLVMGen : public ProgramVisitor<llvm::Value*>, ClassVisitor<void>, FuncFeatureVisitor<llvm::Value*>,
    FieldFeatureVisitor<llvm::Type*>, FormalVisitor<llvm::Type*>, ExprVisitor<llvm::Value*> {
  private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::TargetMachine> target;

    // get the opaque StructType if existed, otherwise create one
    llvm::StructType* CreateOpaqueStructTypeIfNx(const string& name);

    // get the corresponding llvm type
    // Int -> int32
    // Bool -> int1
    // Others -> struct type
    // this will create an opaque struct type if not existed
    llvm::Type* GetLLVMType(const string& type);

    // get the Function if existed, otherwise create one
    llvm::Function* CreateFunctionDeclIfNx(const string& name, const string& type, vector<shared_ptr<Formal>>& args);

    void CreateRuntimeFunctionDecls();

public:
    LLVMGen();
    LLVMGen(const LLVMGen& llvmGen) = delete;
    LLVMGen(const LLVMGen&& llvmGen) = delete;

    void DumpTextualIR(const string& filename);
    void EmitObjectFile(const string& filename);

    llvm::Value* Visit(Program& prog);
    void Visit(Class& cls);
    llvm::Value* Visit(FuncFeature& feat);
    llvm::Type* Visit(FieldFeature& feat);
    llvm::Type* Visit(Formal& formal);

    llvm::Value* Visit_(repr::LinkBuiltin& expr);
    llvm::Value* Visit_(repr::Assign& expr);
    llvm::Value* Visit_(repr::Add& expr);
    llvm::Value* Visit_(repr::Block& expr);
    llvm::Value* Visit_(repr::Case& expr);
    llvm::Value* Visit_(repr::Call& expr);
    llvm::Value* Visit_(repr::Divide& expr);
    llvm::Value* Visit_(repr::Equal& expr);
    llvm::Value* Visit_(repr::False& expr);
    llvm::Value* Visit_(repr::ID& expr);
    llvm::Value* Visit_(repr::IsVoid& expr);
    llvm::Value* Visit_(repr::Integer& expr);
    llvm::Value* Visit_(repr::If& expr);
    llvm::Value* Visit_(repr::LessThanOrEqual& expr);
    llvm::Value* Visit_(repr::LessThan& expr);
    llvm::Value* Visit_(repr::Let& expr);
    llvm::Value* Visit_(repr::MethodCall& expr);
    llvm::Value* Visit_(repr::Multiply& expr);
    llvm::Value* Visit_(repr::Minus& expr);
    llvm::Value* Visit_(repr::Negate& expr);
    llvm::Value* Visit_(repr::New& expr);
    llvm::Value* Visit_(repr::Not& expr);
    llvm::Value* Visit_(repr::String& expr);
    llvm::Value* Visit_(repr::True& expr);
    llvm::Value* Visit_(repr::While& expr);
};

} // namespace irgen

} // namespace cool

#endif //COOL_LLVM_H