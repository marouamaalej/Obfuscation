//RUN: clang -c -emit-llvm fact.c -o fact.bc
//RUN: opt -load <PATH_to_Obfuscate0>/Obfuscate.so fact.bc -o factObfuscated.bc
//RUN: llvm-dis factObfuscated.bc

#include<stdio.h>
#include<stdlib.h>

int fact(int n) {
  if(n == 0) 
   return 1;
  int res = n * fact(n-1);
  return res;
}

int main(int argc, char** argv){
  if(argc != 2)
    return 1;
  int n = atoi(argv[1]);
  printf("%d! = %d\n", n, fact(n));
  return fact(n);
}
