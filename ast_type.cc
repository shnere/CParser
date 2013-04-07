#include <string.h>
#include "ast_type.h"
#include "ast_decl.h"

Type *Type::intType    = new Type("int");
Type *Type::doubleType = new Type("double");

Type::Type(const char *n) {
    typeName = strdup(n);
}
