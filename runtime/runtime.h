//
// Created by 田地 on 2021/7/23.
//

#ifndef COOL_RUNTIME_H
#define COOL_RUNTIME_H

#include "stddef.h"

void coolmain();

// program entry point
void start();

// todo: object layout
//struct Object {
//    uint32_t gcTag;
//    uint32_t classTag;
//    uint32_t size;
//    void*    data;
//};

void* mallocool(uint64_t size);

void* out_int(void*, int32_t i);
void* out_string(void*, char* str);

#endif //COOL_RUNTIME_H
