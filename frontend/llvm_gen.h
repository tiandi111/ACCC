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

//======================================================================//
//                         LLVMGen Class                                //
//======================================================================//
class LLVMGen : public ProgramVisitor<llvm::Value*>, ClassVisitor<void>,
    FuncFeatureVisitor<llvm::Value*>, FieldFeatureVisitor<llvm::Type*>,
    FormalVisitor<llvm::Type*>, ExprVisitor<llvm::Value*> {

  private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::TargetMachine> target;
    adt::ScopedTableSpecializer<adt::SymbolTable>& stable;
    llvm::raw_os_ostream os; // todo: use diag

    //==================================================================//
    //                       SymbolTable Class                          //
    //==================================================================//
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
    llvm::Value* AllocLLVMConstStringStruct(const string& str);
    llvm::Type* GetLLVMType(const string& type);
    llvm::Value* GetPointedValueIfAPointer(llvm::Value*);
    bool IsMappedToLLVMStructPointerType(const string& type);
    bool IsStringLLVMType(llvm::Value* v);

    // C is the class name
    string FunctionName(const string& name, const string& C);
    // get the Function if existed, otherwise create one
    llvm::Function* CreateFunctionDeclIfNx(const string& name,
        const string& type, const string& selfType, vector<Formal*> args);

    llvm::Function* CreateFunctionMain();
    void CreateRuntimeFunctionDecls();

    // constructor
    llvm::Value* DefaultNewOperator(const string& type);
    llvm::Function* CreateNewOperatorDeclIfNx(const string& type);
    llvm::Function* CreateNewOperatorBody(Class& cls);
    llvm::Value* CreateNewOperatorCall(const string& type);

    llvm::Value* CreateMallocCall(int size, llvm::Type* ptrType);

    llvm::Value* CreateICmpAsCoolBool(
        llvm::CmpInst::Predicate, llvm::Value* left, llvm::Value* right);

    // for debug only
    void PrintPointer(llvm::Value* value);

public:
    LLVMGen(adt::ScopedTableSpecializer<adt::SymbolTable>& stable);
    LLVMGen(const LLVMGen& llvmGen) = delete;
    LLVMGen(const LLVMGen&& llvmGen) = delete;

    void DumpTextualIR(const string& filename);
    void EmitObjectFile(const string& filename);

    llvm::Value* genCall(const string& selfType,
        llvm::Value* self, repr::Call& call);

    llvm::Value* Visit(Program& prog);
    void Visit(Class&);
    llvm::Value* Visit(FuncFeature& );
    llvm::Type*  Visit(FieldFeature&);
    llvm::Type*  Visit(Formal&);
    llvm::Value* Visit(repr::Expr&);
    llvm::Value* Visit_(repr::LinkBuiltin&);
    llvm::Value* Visit_(repr::Assign&);
    llvm::Value* Visit_(repr::Add&);
    llvm::Value* Visit_(repr::Block&);
    llvm::Value* Visit_(repr::Case&);
    llvm::Value* Visit_(repr::Call&);
    llvm::Value* Visit_(repr::Divide&);
    llvm::Value* Visit_(repr::Equal&);
    llvm::Value* Visit_(repr::False&);
    llvm::Value* Visit_(repr::ID&);
    llvm::Value* Visit_(repr::IsVoid&);
    llvm::Value* Visit_(repr::Integer&);
    llvm::Value* Visit_(repr::If&);
    llvm::Value* Visit_(repr::LessThanOrEqual&);
    llvm::Value* Visit_(repr::LessThan&);
    llvm::Value* Visit_(repr::Let&);
    llvm::Value* Visit_(repr::MethodCall&);
    llvm::Value* Visit_(repr::Multiply&);
    llvm::Value* Visit_(repr::Minus&);
    llvm::Value* Visit_(repr::Negate&);
    llvm::Value* Visit_(repr::New&);
    llvm::Value* Visit_(repr::Not&);
    llvm::Value* Visit_(repr::String&);
    llvm::Value* Visit_(repr::True&);
    llvm::Value* Visit_(repr::While&);
};

} // namespace irgen

} // namespace cool

#endif //COOL_LLVM_H
