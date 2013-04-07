/* File: ast_stmt.h
 * ----------------
 * The Stmt class and its subclasses are used to represent
 * statements in the parse tree.  For each statment in the
 * language (for, if, return, etc.) there is a corresponding
 * node class for that construct.
 *
 * pp3: You will need to extend the Stmt classes to implement
 * semantic analysis for rules pertaining to statements.
 */

#ifndef _H_ast_stmt
#define _H_ast_stmt

#include "list.h"
#include "ast.h"
#include "ast_type.h"

class Decl;
class VarDecl;
class Expr;
class Type;
class LoopStmt;

class Program : public Node
{
  protected:
     List<Decl*> *decls;

  public:
     Program(List<Decl*> *declList);
};

class Stmt : public Node
{
  public:
     Stmt() : Node() {}
};

class StmtBlock : public Stmt
{
  protected:
    List<VarDecl*> *decls;
    List<Stmt*> *stmts;

  public:
    StmtBlock(List<VarDecl*> *variableDeclarations, List<Stmt*> *statements);
};

class ConditionalStmt : public Stmt
{
  protected:
    Expr *test;
    Stmt *body;

  public:
    ConditionalStmt(Expr *testExpr, Stmt *body);
};

class LoopStmt : public ConditionalStmt
{
  public:
    LoopStmt(Expr *testExpr, Stmt *body)
            : ConditionalStmt(testExpr, body) {}
};

class WhileStmt : public LoopStmt
{
  public:
    WhileStmt(Expr *test, Stmt *body) : LoopStmt(test, body) {}
};

class IfStmt : public ConditionalStmt
{
  protected:
    Stmt *elseBody;

  public:
    IfStmt(Expr *test, Stmt *thenBody, Stmt *elseBody);
};

class ReturnStmt : public Stmt
{
  protected:
    Expr *expr;

  public:
    ReturnStmt(Expr *expr);
};

#endif
