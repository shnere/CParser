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
  NodeExpression& left_binary_expresion;
  NodeExpression& right_primary_expression;

  NodeBinaryOperator(
    NodeExpression& left_binary_expresion,
    int binary_operator,
    NodeExpression& right_primary_expression
  ) : 
  left_binary_expresion(left_binary_expresion), 
  right_primary_expression(right_primary_expression),
  binary_operator(binary_operator) {}
};