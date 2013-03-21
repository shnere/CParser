/* File: ast.cc
 * ------------
 */

#include "ast.h"
#include <string.h> // strdup, strcmp
#include <stdio.h>  // printf

Node::Node() {
    parent = NULL;
}

NodeIdentifier::NodeIdentifier(const char *n) : Node() {
    name = strdup(n);
}
