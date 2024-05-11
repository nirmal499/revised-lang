#include <codeanalysis/BoundExpressionNode.hpp>

namespace trylang
{
    BoundUnaryExpression::BoundUnaryExpression(BoundUnaryOperator* op, std::unique_ptr<BoundExpressionNode> operand)
        : _op(op), _operand(std::move(operand))
    {}
    
    const char* BoundUnaryExpression::Type()
    {
        return _op->_resultType;
    }

    BoundNodeKind BoundUnaryExpression::Kind()
    {
        return BoundNodeKind::UnaryExpression;
    }

    BoundBlockStatement::BoundBlockStatement(std::vector<std::unique_ptr<BoundStatementNode>> statements)
        : _statements(std::move(statements))
    {}

    BoundNodeKind BoundBlockStatement::Kind()
    {
        return BoundNodeKind::BlockStatement;
    }

    BoundExpressionStatement::BoundExpressionStatement(std::unique_ptr<BoundExpressionNode> expression)
        : _expression(std::move(expression))
    {}

    BoundNodeKind BoundExpressionStatement::Kind()
    {
        return BoundNodeKind::ExpressionStatement;
    }

    BoundVariableDeclaration::BoundVariableDeclaration(VariableSymbol variable, std::unique_ptr<BoundExpressionNode> expression)
    : _variable(std::move(variable)), _expression(std::move(expression))
    {}


    BoundNodeKind BoundVariableDeclaration::Kind()
    {
        return BoundNodeKind::VariableDeclarationStatement;
    }

    BoundLiteralExpression::BoundLiteralExpression(const object_t& value)
        : _value(value)
    {}

    const char* BoundLiteralExpression::Type()
    {   
        return trylang::assign_type_info(*_value);
    }

    BoundNodeKind BoundLiteralExpression::Kind()
    {
        return BoundNodeKind::LiteralExpression;
    }

    BoundBinaryExpression::BoundBinaryExpression(std::unique_ptr<BoundExpressionNode> left, BoundBinaryOperator* op, std::unique_ptr<BoundExpressionNode> right)
        : _left(std::move(left)), _op(op), _right(std::move(right))
    {}

    const char* BoundBinaryExpression::Type()
    {
        return _op->_resultType;
    }
    
    BoundNodeKind BoundBinaryExpression::Kind()
    {
        return BoundNodeKind::BinaryExpression;
    }

    BoundVariableExpression::BoundVariableExpression(VariableSymbol variable)
        : _variable(std::move(variable))
    {}

    BoundNodeKind BoundVariableExpression::Kind()
    {
        return BoundNodeKind::VariableExpression;
    }

    const char* BoundVariableExpression::Type()
    {
        return _variable._type;
    }

    BoundAssignmentExpression::BoundAssignmentExpression(VariableSymbol variable, std::unique_ptr<BoundExpressionNode> expression)
        : _variable(std::move(variable)), _expression(std::move(expression))
    {}

    const char* BoundAssignmentExpression::Type()
    {
        return _expression->Type();
    }

    BoundNodeKind BoundAssignmentExpression::Kind()
    {
        return BoundNodeKind::AssignmentExpression;
    }


    BoundIfStatement::BoundIfStatement(std::unique_ptr<BoundExpressionNode> condition,
                                       std::unique_ptr<BoundStatementNode> statement,
                                       std::unique_ptr<BoundStatementNode> elseStatement) : _condition(std::move(condition)), _statement(std::move(statement)), _elseStatement(std::move(elseStatement))
    {

    }

    BoundNodeKind BoundIfStatement::Kind()
    {
        return BoundNodeKind::IfStatement;
    }
}