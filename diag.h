//
// Created by 田地 on 2021/6/29.
//

#ifndef COOL_DIAG_H
#define COOL_DIAG_H

#include <string>
#include <vector>
#include <ostream>
#include <unordered_map>

using namespace std;

namespace cool {

namespace diag {

struct TextInfo {
    int line;
    int pos;
    int fileno;
};

class FileMapper {
  private:
    unordered_map<string, int> name2no;
    unordered_map<int, string> no2name;

  public:
    static FileMapper& GetFileMapper();

    static int GetFileNo(const string& fname);

    static string GetFileName(int fileno);
};

class Diagnosis {
  private:
    enum level {
        WARN,
        ERROR,
        FATAL,
    };
    struct row {
        string file;
        int line;
        int pos;
        level level;
        string msg;
    };
    string file;
    vector<row> rows;

    unordered_map<level, string> levelStr = {
        {WARN, "warning"},
        {ERROR, "error"},
        {FATAL, "fatal"},
    };

  public:
    void SetCurrentFile(const string& _file) { file = _file; }

    void EmitWarn(int line, int pos, const string& msg) {
        rows.emplace_back(row{file, line, pos, WARN, msg});
    }

    void EmitError(int line, int pos, const string& msg) {
        rows.emplace_back(row{file, line, pos, ERROR, msg});
    }

    void EmitFatal(int line, int pos, const string& msg) {
        rows.emplace_back(row{file, line, pos, FATAL, msg});
    }

    void EmitWarn(const string& _file, int line, int pos, const string& msg) {
        rows.emplace_back(row{_file, line, pos, WARN, msg});
    }

    void EmitError(const string& _file, int line, int pos, const string& msg) {
        rows.emplace_back(row{_file, line, pos, ERROR, msg});
    }

    void EmitFatal(const string& _file, int line, int pos, const string& msg) {
        rows.emplace_back(row{_file, line, pos, FATAL, msg});
    }

    void EmitWarn(TextInfo textInfo, const string& msg) {
        EmitWarn(FileMapper::GetFileName(textInfo.fileno), textInfo.line, textInfo.pos, msg);
    }

    void EmitError(TextInfo textInfo, const string& msg) {
        EmitError(FileMapper::GetFileName(textInfo.fileno), textInfo.line, textInfo.pos, msg);
    }

    void EmitFatal(TextInfo textInfo, const string& msg) {
        EmitFatal(FileMapper::GetFileName(textInfo.fileno), textInfo.line, textInfo.pos, msg);
    }

    int Size() { return rows.size(); }

    void Output(ostream& ostm) {
        for (auto& row : rows)
            ostm<< row.file << ":" << row.line << ":" << row.pos << ": " <<
            levelStr.at(row.level) << ": " << row.msg <<endl;
        ostm.flush();
    }
};

} // namespace cool

} // namespace diag

#endif //COOL_DIAG_H
