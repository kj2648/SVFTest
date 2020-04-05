/*
/usr/bin/clang-6.0 -emit-llvm -S ../out_test.c
*/

#include <malloc.h>
#include <stdio.h>

char decode(char in) {
    return in + 5;
}

int makeStr(char* input, char** output, int len) {
    if(!input) {
        return -1;
    }
    *output = (char *)malloc(sizeof(char)*(len+1));
    for(int i=0; i<len; ++i) {
        (*output)[i] = decode(input[i]);
    }
    (*output)[len] = 99;

    printf("%d", (*output)[len]);
    return 0;
}

int main() {
    int len=11;
    char* in = "it is input";
    char* out;

    makeStr(in, &out, 11);
    return 0;
}