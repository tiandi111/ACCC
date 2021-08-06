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
#include "llvm/Support/raw_os_ostream.h"

#include "visitor.h"
#include "repr.h"
#include "adt.h"

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
    adt::ScopedTableSpecializer<adt::SymbolTable>& stable;
    llvm::raw_os_ostream os; // todo: use diag

    class SymbolTable {
      private:
        adt::ScopedTableSpecializer<adt::SymbolTable>& stable;
        unordered_map<uint32_t, unordered_map<string, llvm::Value*>> valueTable;

        llvm::Value* get(const string& name);
        void insert(const string& name, llvm::Value* value);

    public:
        SymbolTable(adt::ScopedTableSpecializer<adt::SymbolTable>& _stable);

        void InsertLocalVar(const string& name, llvm::Value* value);
        void InsertSelfVar(llvm::Value* value);
        void InsertArg(const string& name, llvm::Value* value);

        llvm::Value* GetLocalVar(const string& name);
        llvm::Value* GetSelfVar();
        llvm::Value* GetSelfField(uint32_t i);
        llvm::Value* GetArg(const string& name);
    };

    SymbolTable llvmStable;

    llvm::ConstantInt* ConstInt8(uint8_t v);
    llvm::ConstantInt* ConstInt32(uint32_t v);
    llvm::ConstantInt* ConstInt64(uint64_t v);
    vector<llvm::Value*> ConstInt32s(vector<uint32_t> v);

    // get the opaque StructType if existed, otherwise create one
    llvm::StructType* CreateOpaqueStructTypeIfNx(const string& name);

    llvm::PointerType* CreateStructPointerTypeIfNx(const string& name);

    // get the corresponding llvm type
    // Int -> int32
    // Bool -> int1
    // Others -> struct type
    // this will create an opaque struct type if not existed
    llvm::PointerType* GetStringLLVMType();
    llvm::AllocaInst* AllocLLVMConstStringStruct(const string& str);
    llvm::Type* GetLLVMType(const string& type);
    bool IsMappedToLLVMStructPointerType(const string& type);
    bool IsStringLLVMType(llvm::Value* v);

    // C is the class name
    string FunctionName(const string& name, const string& C);
    // get the Function if existed, otherwise create one
    llvm::Function* CreateFunctionDeclIfNx(const string& name, const string& type,
        const string& selfType, vector<Formal*> args);

    llvm::Function* CreateFunctionMain();
    void CreateRuntimeFunctionDecls();

    // constructor
    llvm::Value* DefaultNewOperator(const string& type);
    llvm::Function* CreateNewOperatorDeclIfNx(const string& type);
    llvm::Function* CreateNewOperatorBody(Class& cls);
    llvm::Value* CreateNewOperatorCall(const string& type);

    llvm::Value* CreateMallocCall(int size, llvm::Type* ptrType);

public:
    LLVMGen(adt::ScopedTableSpecializer<adt::SymbolTable>& stable);
    LLVMGen(const LLVMGen& llvmGen) = delete;
    LLVMGen(const LLVMGen&& llvmGen) = delete;

    void DumpTextualIR(const string& filename);
    void EmitObjectFile(const string& filename);

    llvm::Value* genCall(const string& selfType, llvm::Value* self, repr::Call& call);

    llvm::Value* Visit(Program& prog);
    void Visit(Class& cls);
    llvm::Value* Visit(FuncFeature& feat);
    llvm::Type*  Visit(FieldFeature& feat);
    llvm::Type* Visit(Formal& formal);
    llvm::Value* Visit(repr::Expr& expr);
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
