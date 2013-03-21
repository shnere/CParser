#ifndef _H_ast_expr
#define _H_ast_expr

#include "ast.h"
#include "ast_stmt.h"
#include "list.h"

class Type;

class Expr : public Stmt
{
  public:
    Expr() : Stmt() {}
    virtual Type* GetType() = 0;

  protected:
    Decl* GetFieldDecl(Identifier *field, Type *base);
};

class EmptyExpr : public Expr
{

};

class IntConstant : public Expr
{
  protected:
    int value;

  public:
    IntConstant(int val);
};

class DoubleConstant : public Expr
{
  protected:
    double value;

  public:
    DoubleConstant(double val);
};

class Operator : public Node
{
  protected:
    char tokenString[4];

  public:
    Operator(const char *tok);
 };

class CompoundExpr : public Expr
{
  protected:
    Operator *op;
    Expr *left, *right;

  public:
    CompoundExpr(Expr *lhs, Operator *op, Expr *rhs); // for binary
};

class ArithmeticExpr : public CompoundExpr
{
  public:
    ArithmeticExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
};

class RelationalExpr : public CompoundExpr
{
  public:
    RelationalExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
};

class EqualityExpr : public CompoundExpr
{
  public:
    EqualityExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}  
};

class LogicalExpr : public CompoundExpr
{
  public:
    LogicalExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
};

class AssignExpr : public CompoundExpr
{
  public:
    AssignExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
};

#endif
