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
            /* If we reach here we need to have a "int" */
            int operand = std::get<int>(this->EvaluateExpression(BUEnode->_operand.get()));
            
            if(BUEnode->_operatorKind == BoundUnaryOperatorKind::Identity)
            {
                return operand;
            }

            if(BUEnode->_operatorKind == BoundUnaryOperatorKind::Negation)
            {
                return -operand;
            }

            throw std::logic_error("Unexpected unary operator " + trylang::__boundUnaryOperatorKindStringMap[BUEnode->_operatorKind]);
        }

        auto* BBEnode = dynamic_cast<BoundBinaryExpression*>(node);
        if(BBEnode != nullptr)
        {
            /* If we reach here we need to have a "int" */
            int left = std::get<int>(this->EvaluateExpression(BBEnode->_left.get()));
            int right = std::get<int>(this->EvaluateExpression(BBEnode->_right.get()));

            if(BBEnode->_operatorKind == BoundBinaryOperatorKind::Addition)
            {
                return left + right;
            }

            if(BBEnode->_operatorKind == BoundBinaryOperatorKind::Subtraction)
            {
                return left - right;
            }

            if(BBEnode->_operatorKind == BoundBinaryOperatorKind::Division)
            {
                return left / right;
            }

            if(BBEnode->_operatorKind == BoundBinaryOperatorKind::Multiplication)
            {
                return left * right;
            }

            throw std::logic_error("Unexpected binary operator " + trylang::__boundBinaryOperatorKindStringMap[BBEnode->_operatorKind]);
        }

        throw std::logic_error("Unexpected node " + trylang::__boundNodeStringMap[node->Kind()]);
    }
}