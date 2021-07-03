//
// Created by 田地 on 2021/6/14.
//

#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <iostream>

#include "pass.h"

using namespace cool::pass;

void PassManager::Run(repr::Program& prog, PassContext& ctx) {
    auto& pm = GetPassManager();
    if (!pm.ready) {
        for (auto& edge : pm.dependency) {
            if (!pm.contains(edge.first) || !pm.contains(edge.second)) throw runtime_error("unregistered pass");
            pm.edges[pm.passMap[edge.first]].emplace_back(pm.passMap[edge.second]);
        }
        pm.topsort();
    }

    for (int i = 0; i < pm.passes.size() ; i++) {
        (*pm.passes.at(pm.sorted[i]))(prog, ctx);
        cout<< pm.sorted[i] <<endl;
    }
}

void PassManager::Refresh() {
    GetPassManager().passes.clear();
    GetPassManager().dependency.clear();
    GetPassManager().passMap.clear();
    GetPassManager().sorted.clear();
    GetPassManager().edges.clear();
    GetPassManager().ready = false;
}

bool PassManager::contains(PassID pid) {
    return passMap.find(pid) != passMap.end();
}

void PassManager::topsort() {
    enum state {
        unvisited,
        visiting,
        completed,
    };

    vector<state> states(passes.size(), unvisited);

    auto dfsOnce = [&](int pidx, stack<int>& stack) {
        if (states[pidx] == completed) return;
        while (true) {
            stack.push(pidx);
            states[pidx] = visiting;
            bool hasCompleted = true;
            for (auto next : edges[pidx]) {
                if (states[next] == visiting) throw runtime_error("cyclic dependency detected");
                if (states[next] == unvisited) {
                    hasCompleted = false;
                    pidx = next;
                    break;
                }
            }
            if (hasCompleted) {
                states[pidx] = completed;
                return;
            }
        }
    };

    auto dfs = [&](int pidx) {
        stack<int> stack;
        dfsOnce(pidx, stack);

        while (!stack.empty()) {
            assert(states[stack.top()] != unvisited);
            
            if (states[stack.top()] == completed) {
                sorted.push_back(stack.top());
                stack.pop();
            } else {
                int top = stack.top();
                stack.pop();
                dfsOnce(top, stack);
            }
        }
    };

    for (int i = 0; i < passes.size(); ++i) dfs(i);
}