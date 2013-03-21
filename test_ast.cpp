#include <iostream>
#include "ast.h"

int main() {
  NodeInteger a(5);
  printf("%d\n", a.val);
  return 0;
}