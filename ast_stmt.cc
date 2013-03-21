
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "ast_type.h"

Program::Program(List<Decl*> *d) {
    (decls=d)->SetParentAll(this);
}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
}

ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) {
    (test=t)->SetParent(this);
    (body=b)->SetParent(this);
}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) {
    elseBody = eb;
    if (elseBody) elseBody->SetParent(this);
}

ReturnStmt::ReturnStmt(Expr *e) : Stmt() {
    (expr=e)->SetParent(this);
}
