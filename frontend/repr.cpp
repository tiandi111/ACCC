//
// Created by 田地 on 2021/7/3.
//

#include <algorithm>

#include "repr.h"

using namespace cool;
using namespace repr;

//======================================================================//
//                               Case Class                             //
//======================================================================//
repr::Case* repr::Case::Clone() {
    vector<Branch*> _branches(branches.size());
    for (int i = 0; i < branches.size(); i++)
        _branches[i] = branches.at(i)->Clone();
    return new Case(expr->Clone(), _branches);
}

//======================================================================//
//                        FuncFeature Class                             //
//======================================================================//
repr::FuncFeature* repr::FuncFeature::Clone() {
    vector<Formal*> _args;
    for (auto& arg : args)
        _args.emplace_back(arg->Clone());
    return new FuncFeature(name, type, expr->Clone(), _args);
}

//======================================================================//
//                         Class Class                                //
//======================================================================//

repr::Class::Class(const StringAttr& _name, const StringAttr& _parent,
    vector<FuncFeature*> _funcs, vector<FieldFeature*> _fields)
    : name(_name), parent(_parent), funcs(move(_funcs)),
    fields(move(_fields)) {
    for (auto& func : funcs)
        funcMap.insert({func->GetName().Value(), func});
    for (auto& field : fields)
        fieldMap.insert({field->GetName().Value(), field});
}

::Class* repr::Class::Clone() {

    vector<FuncFeature*> _funcs(funcs.size());
    for(int i = 0; i < funcs.size(); i++)
        _funcs[i] = funcs.at(i)->Clone();

    vector<FieldFeature*> _fields(fields.size());
    for(int i = 0; i < fields.size(); i++)
        _fields[i] = fields.at(i)->Clone();

    return new Class(name, parent, _funcs, _fields);
}

void repr::Class::DeleteFuncFeature(const string& featName) {
    if (!GetFuncFeaturePtr(featName))
        return;
    funcMap.erase(featName);
    auto pred = [&featName](FuncFeature* feat) {
        return feat->GetName().Value() == featName;
    };
    auto it = find_if(funcs.begin(), funcs.end(), pred);
    funcs.erase(it);
}

void repr::Class::DeleteFieldFeature(const string& featName) {
    if (!GetFieldFeaturePtr(featName))
        return;
    fieldMap.erase(featName);
    auto pred = [&featName](FieldFeature* feat) {
        return feat->GetName().Value() == featName;
    };
    auto it = find_if(fields.begin(), fields.end(), pred);
    fields.erase(it);
}

//======================================================================//
//                         Program Class                                //
//======================================================================//

repr::Program::Program(const diag::TextInfo& _textInfo,
    const vector<Class*>& _classVec)
    : textInfo(_textInfo), classVec(_classVec) {
    for (auto& cls : classVec)
        classMap.insert({cls->GetName().Value(), cls});
}

repr::Program* repr::Program::Clone() {
    vector<Class*> _classVec(classVec.size());
    for (int i = 0; i < classVec.size(); i++)
        _classVec[i] = classVec.at(i)->Clone();
    return new Program(textInfo, _classVec);
}

void repr::Program::DeleteClass(const string& name) {
    if (!GetClassPtr(name))
        return;
    classMap.erase(name);
    auto pred = [&name](Class* cls) {
        return cls->GetName().Value() == name;
    };
    auto it = find_if(classVec.begin(), classVec.end(), pred);
    classVec.erase(it);
}