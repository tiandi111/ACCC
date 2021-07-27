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
#include "llvm/Support/raw_os_ostream.h"

#include "builtin.h"
#include "llvm_gen.h"
#include "adt.h"

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

llvm::Value* LLVMGen::SymbolTable::get(const string& name) {
    auto& stack = stable.Stack();
    for (auto it = stack.rbegin(); it != stack.rend(); it++) {
        if (valueTable.at(*it).find(name) != valueTable.at(*it).end())
            return valueTable.at(*it).at(name);
    }
    return nullptr;
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
    assert(false);
    return nullptr;
}

llvm::Value * LLVMGen::SymbolTable::GetArg(const string &name) { return get(name); }

/*-------------------------------------*/
/*          LLVM IR Generator          */
/*-------------------------------------*/
LLVMGen::LLVMGen(adt::ScopedTableSpecializer<adt::SymbolTable>& _stable)
: stable(_stable), llvmStable(_stable), os(cerr) {
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

llvm::ConstantInt* LLVMGen::ConstInt8(uint8_t v) {
    return ConstantInt::get(Type::getInt8Ty(*context), v);
}

llvm::ConstantInt* LLVMGen::ConstInt32(uint32_t v) {
    return ConstantInt::get(Type::getInt32Ty(*context), v);
}

llvm::ConstantInt* LLVMGen::ConstInt64(uint64_t v) {
    return ConstantInt::get(Type::getInt64Ty(*context), v);
}

vector<llvm::Value*> LLVMGen::ConstInt32s(vector<uint32_t> v) {
    vector<Value*> arr;
    for (auto i : v)
        arr.emplace_back(ConstInt32(i));
    return arr;
}

StructType* LLVMGen::CreateOpaqueStructTypeIfNx(const string &name) {
    StructType *st = StructType::getTypeByName(*context, name);
    if (!st)
        st = StructType::create(*context, name);
    return st;
}

PointerType* LLVMGen::CreateStructPointerTypeIfNx(const string& name) {
    return PointerType::get(CreateOpaqueStructTypeIfNx(name), 0);
}

llvm::PointerType* LLVMGen::GetStringLLVMType() {
    StructType *st = CreateOpaqueStructTypeIfNx("String");
    st->setBody({Type::getInt32Ty(*context), PointerType::getInt8PtrTy(*context)});
    return PointerType::get(st, 0);
}

llvm::AllocaInst* LLVMGen::AllocLLVMConstStringStruct(const string& str) {
    auto alloca = builder->CreateAlloca(GetStringLLVMType()->getElementType());

    // create data
    vector<Constant*> chars;
    for (auto& c : str) chars.emplace_back(ConstInt8(c));
    chars.emplace_back(ConstInt8('\0'));
    auto constData = ConstantArray::get(ArrayType::get(Type::getInt8Ty(*context), chars.size()), chars);
    auto constDataPtr = builder->CreateGEP(constData->getType(), constData, ConstInt32s({0, 0}));

    // set size pointer
    auto sizePtr = builder->CreateGEP(alloca, ConstInt32s({0, 0}));
    builder->CreateStore(ConstInt32(chars.size() - 1), sizePtr);

    // set data pointer
    auto dataPtr = builder->CreateGEP(alloca, ConstInt32s({0, 1}));
    builder->CreateStore(constDataPtr, dataPtr);

    return alloca;
}

Type* LLVMGen::GetLLVMType(const string& name) {
    if (name == "Int") {
        return Type::getInt32Ty(*context);
    } else if (name == "Bool") {
        return Type::getInt32Ty(*context);
    } else if (name == "String") {
        return GetStringLLVMType();
    }
    return CreateStructPointerTypeIfNx(name);
}

bool LLVMGen::IsMappedToLLVMStructPointerType(const string& type) {
    return type != "Int" && type != "Bool";
}

bool LLVMGen::IsStringLLVMType(llvm::Value* v) {
    return v->getType()->isPointerTy() && v->getType()->getPointerElementType()->getStructName() == "String";
}

string LLVMGen::FunctionName(const string& name, const string& selfType) {
    return selfType + "_" + name;
}

Function* LLVMGen::CreateFunctionDeclIfNx(const string& name, const string& type, const string& selfType,
    vector<shared_ptr<Formal>>& args) {
    auto funcName = FunctionName(name, selfType);

    auto function = module->getFunction(funcName);
    if (function) return function;

    vector<Type *> argTypes = {GetLLVMType(selfType)};
    for(auto& arg : args) argTypes.emplace_back(GetLLVMType(arg->type.val));

    FunctionType* ft = FunctionType::get(GetLLVMType(type), argTypes, false);

    function = Function::Create(ft, Function::ExternalLinkage, funcName, module.get());

    unsigned idx = 0;
    for (auto& arg : function->args()) {
        if (idx == 0) arg.setName("self");
        else arg.setName(args.at(idx-1)->name.val);
        idx++;
    }

    return function;
}

llvm::Function* LLVMGen::CreateFunctionMain() {
    if (auto function = module->getFunction("coolmain") )
        return function;
    FunctionType* ft = FunctionType::get(Type::getVoidTy(*context), {}, false);
    return Function::Create(ft, Function::ExternalLinkage, "coolmain", module.get());
}

void LLVMGen::CreateRuntimeFunctionDecls() {
    //todo: create built in func decl

    // llvm built-in types
    auto int32Type = Type::getInt32Ty(*context);
    auto int64Type = Type::getInt64Ty(*context);
    auto voidPointerType = PointerType::get(int32Type, 0);
    auto int8Ptr = Type::getInt8PtrTy(*context);

    // customized types
    vector<Type*> args;
    FunctionType* ft;

    // runtime/runtime.h: int entry();
    ft = FunctionType::get(int32Type, false);
    Function::Create(ft, Function::ExternalLinkage, "entry", module.get());

    // runtime/runtime.h: void* mallocool(uint64_t size);
    args = {int64Type};
    ft = FunctionType::get(voidPointerType, args,false);
    Function::Create(ft, Function::ExternalLinkage, "mallocool", module.get());

    // runtime/runtime.h: void* out_int(int32_t i);
    args = {int32Type};
    ft = FunctionType::get(voidPointerType, args, false);
    Function::Create(ft, Function::ExternalLinkage, "out_int", module.get());

    // runtime/runtime.h: void* out_string(char* str);
    args= {int8Ptr};
    ft = FunctionType::get(voidPointerType, args, false);
    Function::Create(ft, Function::ExternalLinkage, "out_string", module.get());
}

llvm::Value* LLVMGen::DefaultNewOperator(const string& type) {
    if (type == "Int") {
        return ConstInt32(0);
    }
    if (type == "Bool") {
        auto f = ConstantInt::getFalse(Type::getInt32Ty(*context));
        return builder->CreateIntCast(f, Type::getInt32Ty(*context), true);
    }
    if (type == "String") {
        return AllocLLVMConstStringStruct("");
    }
//    if (type == "Object") {
//      todo
//    }
//    if (type == "IO") {
//      todo
//    }
    return ConstantPointerNull::get(CreateStructPointerTypeIfNx(type));
}

llvm::Function* LLVMGen::CreateNewOperatorDeclIfNx(const string& type) {
    assert(!type.empty() && isupper(type.at(0)));
    auto function = module->getFunction(type);
    if (!function) {
        FunctionType* ft = FunctionType::get(GetLLVMType(type), false);
        function = Function::Create(ft, Function::ExternalLinkage, type, module.get());
    }
    return function;
}


llvm::Function* LLVMGen::CreateNewOperatorBody(Class &cls) {
    auto function = CreateNewOperatorDeclIfNx(cls.name.val);

    BasicBlock* bb = BasicBlock::Create(*context, "entry", function);
    builder->SetInsertPoint(bb);

    if (builtin::IsBuiltinClass(cls.name.val)) {
        builder->CreateRet(DefaultNewOperator(cls.name.val));
        return function;
    }

    // malloc
    uint32_t size = cls.GetFieldFeatures().size() * 4;
    auto mallocFunc = module->getFunction("mallocool");
    auto orgPtr = builder->CreateCall(mallocFunc, {ConstInt64(size)});
    auto ptr = builder->CreatePointerCast(orgPtr, CreateStructPointerTypeIfNx(cls.name.val));

    // init fields
    uint32_t i = 0;
    for (auto& field : cls.GetFieldFeatures()) {
        Value* value;
        Value* fieldPtr = builder->CreateGEP(ptr, ConstInt32s({0, i++}));
        if (field->expr)
            value = Visit(*field->expr);
        else
            value = DefaultNewOperator(field->type.val);
        builder->CreateStore(value, fieldPtr);
    }

    builder->CreateRet(ptr);
    return function;
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

llvm::Value* LLVMGen::genCall(const string& selfType, llvm::Value* self, repr::Call& call) {
    auto function = CreateFunctionDeclIfNx(call.link->name.val, call.link->type.val, selfType, call.link->args);

    vector<Value*> args = {self};
    for (auto& arg : call.args)
        args.emplace_back(Visit(*arg));

    return builder->CreateCall(function, args);
}

Value * LLVMGen::Visit(Program &prog) {
    ENTER_SCOPE_GUARD(stable, {
        CreateRuntimeFunctionDecls();
        for (auto& cls : prog.GetClasses()) Visit(*cls);
    })
    verifyModule(*module, &os);
}

void LLVMGen::Visit(Class &cls) {
    ENTER_SCOPE_GUARD(stable, {
        vector<Type *> Fields;
        for (auto& feat : cls.GetFieldFeatures()) Fields.emplace_back(Visit(*feat));
        StructType* ST = CreateOpaqueStructTypeIfNx(cls.name.val);
        ST->setBody(Fields, false);

        for (auto& feat : cls.GetFuncFeatures()) Visit(*feat);

        CreateNewOperatorBody(cls);
    })
}

Value * LLVMGen::Visit(FuncFeature &feat) {
    Function* function;
    ENTER_SCOPE_GUARD(stable, {

        if (stable.GetClass()->name.val == "Main" && feat.name.val == "main") {

            function =  CreateFunctionMain();
            BasicBlock* bb = BasicBlock::Create(*context, "entry", function);
            builder->SetInsertPoint(bb);
            builder->Insert(Visit(*feat.expr));
            builder->CreateRetVoid();

        } else {

            auto funcName = FunctionName(feat.name.val, stable.GetClass()->name.val);
            function = module->getFunction(funcName);

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

            builder->CreateRet(Visit(*feat.expr));
        }

    })
    return function;
}

Type * LLVMGen::Visit(FieldFeature &feat) {
    return GetLLVMType(feat.type.val);
}

Type* LLVMGen::Visit(Formal& formal) {
    return GetLLVMType(formal.type.val);
}

Value* LLVMGen::Visit(repr::Expr& expr) {
    return ExprVisitor<Value *>::Visit(expr);
}

Value* LLVMGen::Visit_(repr::LinkBuiltin& expr) {
    auto function = module->getFunction(expr.name);
    vector<Value*> args;
    for (auto& param : expr.params) {
        auto arg = llvmStable.GetArg(param);
        if (IsStringLLVMType(arg)) {
            arg = builder->CreateGEP(arg, ConstInt32s({0, 1}));
            arg = builder->CreateLoad(arg);
        }
        args.emplace_back(arg);
    }
    auto ret = builder->CreateCall(function, args);
    // C runtime functions only return void pointer, cast it to the correct type.
    auto rType = GetLLVMType(expr.type);
    if (rType->isPointerTy())
        return builder->CreatePointerCast(ret, rType);
    return ret;
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
        for (auto& eExpr : expr.exprs) value = Visit(*eExpr);
    })
    return value;
}

Value* LLVMGen::Visit_(repr::Case& expr) {
    // todo
    return nullptr;
}

Value* LLVMGen::Visit_(repr::Call& expr) {
    Value* value;
    ENTER_SCOPE_GUARD(stable, {
        value = genCall(stable.GetClass()->name.val, llvmStable.GetSelfVar(), expr);
    })
    return value;
}

Value* LLVMGen::Visit_(repr::Divide& expr) {
    return builder->CreateSDiv(Visit(*expr.left), Visit(*expr.right));
}

Value* LLVMGen::Visit_(repr::Equal& expr) {
    auto left = Visit(*expr.left);
    auto right = Visit(*expr.right);
    if (IsStringLLVMType(left)) {
        if (!IsStringLLVMType(right)) throw runtime_error("");
        // todo: Create a built function to compare string content
        return ConstantInt::getFalse(Type::getInt32Ty(*context));
    }
    if (left->getType()->isIntegerTy()) {
        if (!right->getType()->isIntegerTy()) throw runtime_error("");
        return builder->CreateICmpEQ(left, right);
    }
    assert(left->getType()->isPointerTy() && right->getType()->isPointerTy());
    return builder->CreateICmpEQ(left, right);
}

Value* LLVMGen::Visit_(repr::False& expr) {
    return ConstantInt::getFalse(*context);
}

Value* LLVMGen::Visit_(repr::ID& expr) {
    auto idAttr = stable.GetIdAttr(expr.name.val);
    switch (idAttr->storageClass) {
        case attr::IdAttr::Field: {
            auto self = llvmStable.GetSelfVar();
            auto fieldPtr = builder->CreateGEP(self, ConstInt32s({0, uint32_t (idAttr->idx)}));
            return builder->CreateLoad(fieldPtr);
        }
        case attr::IdAttr::Local:
            return builder->CreateLoad(llvmStable.GetLocalVar(idAttr->name));
        case attr::IdAttr::Arg:
            return llvmStable.GetArg(idAttr->name);
        default:
            assert(false && "Invalid IdAttr:StorageClass Enum");
    }
}

Value* LLVMGen::Visit_(repr::IsVoid& expr) {
    auto value = Visit(*expr.expr);
    if (value->getType()->isPointerTy() && !IsStringLLVMType(value))
        return builder->CreateIsNull(value);     // todo: valid?
    return ConstantInt::getFalse(Type::getInt32Ty(*context));
}

Value* LLVMGen::Visit_(repr::Integer& expr) {
    return ConstInt32(expr.val.val);
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
        Value* value;
        // note: alloca is a pointer points to pointer of type 'formal.type.val'
        auto type = GetLLVMType(formal.type.val);
        auto alloca = builder->CreateAlloca(type, nullptr, formal.name.val);
        if (formal.expr)
            value = Visit(*formal.expr);
        else
            value = DefaultNewOperator(formal.type.val);
        builder->CreateStore(value, alloca);
        return alloca;
    };
    Value* value;
    for (int i = 0; i < expr.formals.size(); i++) {
        auto formal = expr.formals.at(i);
        stable.EnterScope();
        llvmStable.InsertLocalVar(formal->name.val, visitFormal(*formal));
    }
    value = Visit(*expr.expr);
    for (int i = 0; i < expr.formals.size(); i++)
        stable.LeaveScope();
    return value;
}

Value* LLVMGen::Visit_(repr::MethodCall& expr) {
    Value* value;
    ENTER_SCOPE_GUARD(stable, {
        value = genCall(expr.type, Visit(*expr.left), *static_pointer_cast<repr::Call>(expr.right));
    })
    return value;
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
    auto type = expr.type.val;
    auto newOp = module->getFunction(type);
    return builder->CreateCall(newOp, {});
}

Value* LLVMGen::Visit_(repr::Not& expr) {
    return builder->CreateNot(Visit(*expr.expr));
}

Value* LLVMGen::Visit_(repr::String& expr) {
    return AllocLLVMConstStringStruct(expr.val.val);
}

Value* LLVMGen::Visit_(repr::True& expr) {
    return ConstantInt::getTrue(*context);
}

Value* LLVMGen::Visit_(repr::While& expr) {
    // todo
    return nullptr;
}
