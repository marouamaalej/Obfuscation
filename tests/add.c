//RUN: clang -c -emit-llvm add.c -o add.bc
//RUN: opt -load <PATH_to_Obfuscate0>/Obfuscate.so add.bc -o addObfuscated.bc
//RUN: llvm-dis addObfuscated.bc

#include<stdio.h>
#include<stdlib.h>

int add(int n, int a){
  if (n < a)
    n = n + a;
  if (n == a)
    return 0;
  else
    return n + add(n - 1, a);
}


int main(int argc, char** argv){
  if(argc != 3)
    return 1;
  int n = atoi(argv[1]);
  int a = atoi(argv[2]);
  int s = add(n, a);
  printf("add %d %d = %d\n", n, a, add(n,a));

return 0;
}
