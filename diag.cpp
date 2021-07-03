//
// Created by 田地 on 2021/7/4.
//

#include "diag.h"

using namespace cool;
using namespace diag;

FileMapper& FileMapper::GetFileMapper() {
    static FileMapper fileMapper;
    return fileMapper;
}

int FileMapper::GetFileNo(const string& fname) {
    auto& fm = GetFileMapper();
    if (fm.name2no.find(fname) == fm.name2no.end()) {
        fm.name2no.insert({fname, fm.name2no.size()});
        fm.no2name.insert({fm.no2name.size(), fname});
    }
    return fm.name2no.at(fname);
}

string FileMapper::GetFileName(int fileno) {
    if (fileno == -1) return "";
    auto& fm = GetFileMapper();
    if (fm.no2name.find(fileno) == fm.no2name.end()) throw runtime_error("invalid file number");
    return fm.no2name.at(fileno);
}