#ifndef _H_ast
#define _H_ast

#include <stdlib.h>   
#include <iostream>
using namespace std;

// class NodeStatement; // Statements son estructuras de control, whiles y asignaciones de variables
// class NodeExpression; // Expresiones sno todos aquellos nodos que se pueden reducir a un valor

// Lista Ligada para Statements y Expresiones
// typedef std::vector<NodeStatement*> StatementList;
// typedef std::vector<NodeExpression*> ExpressionList;

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

// // Tipos de Variables
// class NodeType : public Node {
// public:
//   std::string typeName;
//   NodeType(const std::string& typeName) : typeName(typeName) {}
// };


// // Declaration
// class Decl : public Node {
// public:
//   NodeIdentifier *id;
//   Decl(NodeIdentifier *id) : id(id) {}
// };

// class NodeExpression : public Node {};
// class NodeStatement : public Node {};

// // Enteros papayol
// class NodeInteger : public NodeExpression {
//   public:
//     int val;
//     NodeInteger(int value) : val(value) {} // Sintaxis de campeones. Getters/Setters gtfo
// };

// // Floats mirrey
// class NodeFloat : public NodeExpression {
//   public:
//     float val;
//     NodeFloat(float value) : val(value) {}
// };



// // Binarios chiquito
// class NodeBinaryOperator : public NodeExpression {
// public:
//   int binary_operator;
//   NodeExpression& left_expression;
//   NodeExpression& right_expression;

//   NodeBinaryOperator(
//     NodeExpression& left_expression,
//     int binary_operator,
//     NodeExpression& right_expression
//   ) : 
//   left_expression(left_expression), 
//   right_expression(right_expression),
//   binary_operator(binary_operator) {}
// };

// // asignaciones bebe
// class NodeAssignment : public NodeExpression {
// public:
//   NodeIdentifier& left_expression;
//   NodeExpression& right_expression;
//   NodeAssignment(
//     NodeIdentifier& left_expression,
//     NodeExpression& right_expression
//   ) :
//   left_expression(left_expression),
//   right_expression(right_expression) {}
// };

// // expresiones precioso (stmt -> expression)
// class NodeExpressionStatement : public NodeStatement {
// public:
//   NodeExpression& expression;
//   NodeExpressionStatement(
//     NodeExpression& expression
//   ) :
//   expression(expression) {}
// };

// // declaraciones de variable amor
// class NodeVariableDeclaration : public NodeStatement {
// public:
//   const NodeIdentifier& type;
//   NodeIdentifier& id;
//   NodeExpression *assignmentExpression;
//   NodeVariableDeclaration(
//     const NodeIdentifier& type,
//     NodeIdentifier& id
//   ) :
//   type(type),
//   id(id) {}

//   NodeVariableDeclaration(
//     const NodeIdentifier& type,
//     NodeIdentifier& id,
//     NodeExpression *assignmentExpression
//   ) :
//   type(type),
//   id(id),
//   assignmentExpression(assignmentExpression) {}
// };

#endif