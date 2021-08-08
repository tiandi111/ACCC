//
// Created by 田地 on 2021/7/23.
//

#include <stdlib.h>
#include <stdio.h>

#include "runtime.h"

void start() {
//    printf("hello cool!");
    coolmain();
}

void* mallocool(uint64_t size) {
    void* ptr = malloc(size);
//    printf("%llu, %p\n", size, ptr);
    return ptr;
}

void out_int(int32_t i) {
    printf("%d\n", i);
}

void out_string( char* str) {
//    printf("addr %p\n", (void*) str);
    printf("%s\n", str);
}

void print_ptr(void* ptr) {
    printf("print_ptr: %p\n", ptr);
}

int main() {
    start();
}