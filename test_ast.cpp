#include <iostream>
#include "ast.h"

int main() {
  NodeExpression stub1, stub2;
  NodeInteger i(5);
  NodeFloat f(5.0);
  NodeIdentifier s("int");
  NodeBinaryOperator b(stub1, 0, stub2);
  return 0;
}