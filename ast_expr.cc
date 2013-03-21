#include <string.h>
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"

IntConstant::IntConstant(int val) : Expr() {
    value = val;
}

DoubleConstant::DoubleConstant(double val) : Expr() {
    value = val;
}

Operator::Operator(const char *tok) : Node() {
    strncpy(tokenString, tok, sizeof(tokenString));
}

CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r)
  : Expr() {
    (op=o)->SetParent(this);
    (left=l)->SetParent(this);
    (right=r)->SetParent(this);
}
