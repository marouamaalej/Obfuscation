//RUN: ./obfuscate.sh fact.c
//RUN: ./fact 0; check that 0! = 1
//RUN: ./fact n; check the result

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
