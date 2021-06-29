//
// Created by 田地 on 2021/6/29.
//

#ifndef COOL_DIAG_H
#define COOL_DIAG_H

#include <string>
#include <vector>
#include <ostream>

using namespace std;

namespace cool {

namespace diag {

class Diagnosis {
  private:
    struct row {
        int line;
        string msg;
    };
    vector<row> rows;

  public:

    void Emit(int line, const string& msg) {
        rows.emplace_back(row{line, msg});
    }

    void Output(ostream& ostm) {
        for (auto& row : rows)
            ostm<< "line " << row.line << ": " << row.msg;
        ostm.flush();
    }
};

} // namespace cool

} // namespace diag

#endif //COOL_DIAG_H
