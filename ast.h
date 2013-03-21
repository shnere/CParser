#ifndef _H_ast
#define _H_ast

#include <stdlib.h>   
#include <iostream>
using namespace std;

class Node  {
  protected:
    Node *parent;

  public:
    Node();
    virtual ~Node() {}

    void SetParent(Node *p)  { parent = p; }
    Node *GetParent()        { return parent; }
};

// Identificadores
class Identifier : public Node {
  protected:
    char *name;
  public:
    Identifier(const char *name);
    const char* Name() { return name; }
};

#endif