//
// Created by 田地 on 2021/7/23.
//

#ifndef COOL_RUNTIME_H
#define COOL_RUNTIME_H

#include "stddef.h"
#include "stdint.h"

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

void out_int(int32_t);
void out_string(char*);

// for debug use only
void print_ptr(void*);

#endif //COOL_RUNTIME_H
