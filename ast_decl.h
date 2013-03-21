#ifndef _H_ast_decl
#define _H_ast_decl

#include "ast.h"
#include "ast_stmt.h"
#include "ast_type.h"

class Type;
class Identifier;
class Stmt;

class Decl : public Node
{
  protected:
    Identifier *id;
  public:
    Decl(Identifier *name);
    const char* Name() { return id->Name(); }
};

class VarDecl : public Decl
{
  protected:
    Type *type;

  public:
    VarDecl(Identifier *name, Type *type);
    Type* GetType() { return type; }
};

#endif
