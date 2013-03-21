#include <iostream>
#include <vector>

class NodeStatement; // Statements son estructuras de control, whiles y asignaciones de variables
class NodeExpression; // Expresiones sno todos aquellos nodos que se pueden reducir a un valor

// Lista Ligada para Statements y Expresiones
typedef std::vector<NodeStatement*> StatementList;
typedef std::vector<NodeExpression*> ExpressionList;

class Node {
public:
  virtual ~Node() {};
};

class NodeExpression : public Node {};
class NodeStatement : public Node {};

// Enteros papayol
class NodeInteger : public NodeExpression {
  public:
    int val;
    NodeInteger(int value) : val(value) {} // Sintaxis de campeones. Getters/Setters gtfo
};

// Floats mirrey
class NodeFloat : public NodeExpression {
  public:
    float val;
    NodeFloat(float value) : val(value) {}
};

// Identificadores guapo
class NodeIdentifier : public NodeExpression {
public:
  std::string name;
  NodeIdentifier(const std::string& name) : name(name) {}
};

// Binarios chiquito
class NodeBinaryOperator : public NodeExpression {
public:
  int binary_operator;
  NodeExpression& left_expression;
  NodeExpression& right_expression;

  NodeBinaryOperator(
    NodeExpression& left_expression,
    int binary_operator,
    NodeExpression& right_expression
  ) : 
  left_expression(left_expression), 
  right_expression(right_expression),
  binary_operator(binary_operator) {}
};

// asignaciones bebe
class NodeAssignment : public NodeExpression {
public:
  NodeIdentifier& left_expression;
  NodeExpression& right_expression;
  NodeAssignment(
    NodeIdentifier& left_expression,
    NodeExpression& right_expression
  ) :
  left_expression(left_expression),
  right_expression(right_expression) {}
};

// expresiones precioso (stmt -> expression)
class NodeExpressionStatement : public NodeStatement {
public:
  NodeExpression& expression;
  NodeExpressionStatement(
    NodeExpression& expression
  ) :
  expression(expression) {}
};

// declaraciones de variable amor
class NodeVariableDeclaration : public NodeStatement {
public:
  const NodeIdentifier& type;
  NodeIdentifier& id;
  NodeExpression *assignmentExpression;
  NodeVariableDeclaration(
    const NodeIdentifier& type,
    NodeIdentifier& id
  ) :
  type(type),
  id(id) {}

  NodeVariableDeclaration(
    const NodeIdentifier& type,
    NodeIdentifier& id,
    NodeExpression *assignmentExpression
  ) :
  type(type),
  id(id),
  assignmentExpression(assignmentExpression) {}
};

