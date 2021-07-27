//
// Created by 田地 on 2021/7/23.
//

#include <stdlib.h>
#include <stdio.h>

#include "runtime.h"

void start() {
    printf("hello cool!");
    coolmain();
}

void* mallocool(uint64_t size) {
    return malloc(size);
}

int main() {
    start();
}