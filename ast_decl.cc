#include "ast_decl.h"
#include "ast_type.h"
//#include "ast_stmt.h"

Decl::Decl(Identifier *n) : Node() {
    (id=n)->SetParent(this);
}

VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n) {
    (type=t)->SetParent(this);
}