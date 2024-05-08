#include <codeanalysis/Evaluator.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/BoundExpressionNode.hpp>
#include <codeanalysis/SyntaxKind.hpp>
#include <variant>

namespace trylang
{
    std::unique_ptr<BoundExpressionNode> _root;

    Evaluator::Evaluator(std::unique_ptr<BoundExpressionNode> root) : _root(std::move(root))
    {}

    oobject_t Evaluator::Evaluate()
    {
        return this->EvaluateExpression(_root.get());
    }

    oobject_t Evaluator::EvaluateExpression(BoundExpressionNode* node)
    {   
        auto* BLEnode = dynamic_cast<BoundLiteralExpression*>(node);
        if(BLEnode != nullptr)
        {
            return *BLEnode->_value;
        }

        auto* BUEnode = dynamic_cast<BoundUnaryExpression*>(node);
        if(BUEnode != nullptr)
        {   
            oobject_t operand = this->EvaluateExpression(BUEnode->_operand.get());
            
            if(BUEnode->_operatorKind == BoundUnaryOperatorKind::Identity)
            {
                int operand_value = std::get<int>(operand); /* If we are reaching here means operand has "int" */
                return operand_value;
            }

            if(BUEnode->_operatorKind == BoundUnaryOperatorKind::Negation)
            {
                int operand_value = std::get<int>(operand); /* If we are reaching here means operand has "int" */
                return -operand_value;
            }

            if(BUEnode->_operatorKind == BoundUnaryOperatorKind::LogicalNegation)
            {
                bool operand_value = std::get<bool>(operand); /* If we are reaching here means operand has "bool" */
                return !operand_value;
            }
            throw std::logic_error("Unexpected unary operator " + trylang::__boundUnaryOperatorKindStringMap[BUEnode->_operatorKind]);
        }

        auto* BBEnode = dynamic_cast<BoundBinaryExpression*>(node);
        if(BBEnode != nullptr)
        {
            /* If we reach here we need to have a "int" */
            oobject_t left = this->EvaluateExpression(BBEnode->_left.get());
            oobject_t right = this->EvaluateExpression(BBEnode->_right.get());

            if(BBEnode->_operatorKind == BoundBinaryOperatorKind::Addition)
            {
                int left_value = std::get<int>(left);
                int right_value = std::get<int>(right);
                return left_value + right_value;
            }

            if(BBEnode->_operatorKind == BoundBinaryOperatorKind::Subtraction)
            {
                int left_value = std::get<int>(left);
                int right_value = std::get<int>(right);
                return left_value - right_value;
            }

            if(BBEnode->_operatorKind == BoundBinaryOperatorKind::Division)
            {
                int left_value = std::get<int>(left);
                int right_value = std::get<int>(right);
                return left_value / right_value;
            }

            if(BBEnode->_operatorKind == BoundBinaryOperatorKind::Multiplication)
            {
                int left_value = std::get<int>(left);
                int right_value = std::get<int>(right);
                return left_value * right_value;
            }

            if(BBEnode->_operatorKind == BoundBinaryOperatorKind::LogicalOr)
            {
                bool left_value = std::get<bool>(left);
                bool right_value = std::get<bool>(right);
                return (left_value || right_value);
            }

            if(BBEnode->_operatorKind == BoundBinaryOperatorKind::LogicalAnd)
            {
                bool left_value = std::get<bool>(left);
                bool right_value = std::get<bool>(right);
                return (left_value && right_value);
            }

            throw std::logic_error("Unexpected binary operator " + trylang::__boundBinaryOperatorKindStringMap[BBEnode->_operatorKind]);
        }

        throw std::logic_error("Unexpected node " + trylang::__boundNodeStringMap[node->Kind()]);
    }
}