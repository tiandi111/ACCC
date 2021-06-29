//
// Created by 田地 on 2021/6/21.
//

#ifndef COOL_VTABLE_H
#define COOL_VTABLE_H

#include <unordered_map>
#include <typeindex>
#include <string>

using namespace std;

template<typename R, class Obj, typename... Args>
class VirtualTable {
  public:
    using VFunc = function<R(Obj&, Args...)>;

    unordered_map<type_index, VFunc> table;

    R operator()(Obj& obj, Args... args) {
        if (table.find(type_index(typeid(obj))) == table.end()) {
            throw runtime_error(string("no available dispatch for type: ") + typeid(obj).name());
        }
        return table[type_index(typeid(obj))](obj, args...);
    }

    template<class ObjType>
    void SetDispatch(VFunc vfunc) {
        table.insert({type_index(typeid(ObjType)), vfunc});
    }
};

#endif //COOL_VTABLE_H
