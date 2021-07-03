//
// Created by 田地 on 2021/6/14.
//

#ifndef COOL_PASS_H
#define COOL_PASS_H

#include <typeindex>
#include <memory>
#include <unordered_map>

#include "repr.h"
#include "diag.h"

#define PassID(PassClass) std::type_index(typeid(PassClass))
using namespace std;
typedef type_index PassID;

namespace cool {

namespace pass {

class PassContext {
  public:
    diag::Diagnosis& diag;

    PassContext(diag::Diagnosis& _diag) : diag(_diag) {}

    unordered_map<string, pair<type_index, shared_ptr<void>>> map;

    template<class T>
    void Set(string name, T& val) {
        map.insert({move(name), make_pair(type_index(typeid(T)), make_shared<T>(val))});
    }

    template<class T>
    shared_ptr<T> Get(string name) {
        if (map.find(name) == map.end()) throw runtime_error("object '" + name + "' not found");
        if (map.find(name)->second.first != type_index(typeid(T))) {
            throw runtime_error("get object of wrong type");
        }
        return static_pointer_cast<T>(map.find(name)->second.second);
    }
};

class Pass {
  public:
    Pass() {}
    virtual ~Pass() {};
    virtual void Required() {};
    virtual repr::Program operator()(repr::Program& prog, PassContext& ctx) { return prog; }
};

class ProgramPass : public Pass {
  public:
    virtual repr::Program operator()(repr::Program& prog, PassContext& ctx) { return prog; }
};

// todo: think, should we pass information by pass object or pass context?
class PassManager {
  public:
    PassManager(PassManager &pm) = delete;

    void operator=(PassManager &pm) = delete;

    static PassManager &GetPassManager() {
        static PassManager passManager;
        return passManager;
    }

    template<class PassClass>
    static void Register() {
        auto& pm = GetPassManager();
        if (pm.contains(PassID(PassClass))) throw runtime_error("duplicate pass" );
        pm.passes.emplace_back(make_shared<PassClass>(PassClass()));
        pm.edges.emplace_back(vector<int>());
        pm.passMap.insert({PassID(PassClass), pm.passes.size()-1});
        pm.passes.back()->Required();
    }

    // todo: is it good to return shared_ptr here?
    template<class PassClass>
    static shared_ptr<PassClass> Get() {
        auto& pm = GetPassManager();
        auto id = PassID(PassClass);
        if (!pm.contains(id)) return nullptr;
        return dynamic_pointer_cast<PassClass>(pm.passes[pm.passMap[id]]);
    }

    template<class Requirer, class Requiree>
    static void Required() {
        GetPassManager().dependency.emplace_back(make_pair(PassID(Requirer), PassID(Requiree)));
    }

    static void Run(repr::Program& prog, PassContext& ctx);

    static void Refresh();

  private:
    PassManager() : ready(false) {}

    void topsort();

    bool contains(PassID pid);

    unordered_map<PassID, int> passMap;
    vector<pair<PassID, PassID>> dependency;
    vector<shared_ptr<Pass>> passes;
    vector<vector<int>> edges;
    vector<int> sorted;
    bool ready;
};

} // cool

} // pass
#endif //COOL_PASS_H
