/*
/usr/bin/clang-6.0 -emit-llvm -S ../field_test.c
*/

#include <malloc.h>
#include <stdio.h>

typedef struct {
    char* a, b;
    int alen, blen;
}S;

void setA(S* s, int len) {
    s->alen = len;
    s->a[len] = 'A';
}

int makeS(S* input, S** output, int len) {
    if(!input) {
        return -1;
    }
    *output = (S *)malloc(sizeof(S));
    (*output)->a = (char *)malloc(sizeof(char)*(len+1));
    (*output)->a[0] = input->a[0];
    setA(*output, len);
    

    printf("%d", (*output)->alen);
    return 0;
}

int main() {
    int len=11;
    S in;
    in.a = "it is input";
    S* out;

    makeS(&in, &out, 11);
    return 0;
}