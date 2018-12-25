

int add(int n, int a){
  if (n > a)
    n = n - a;
  else
    n = a - n;
  if (n == -10)
    return 0;
  else if (n > 0)
    return n + add(n - 1, a);
  else
    return -n + add(n - 1, a);
}
