#include <codeanalysis/Evaluator.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/SyntaxKind.hpp>
#include <variant>

namespace trylang
{
    std::unique_ptr<ExpressionSyntax> _root;

    Evaluator::Evaluator(std::unique_ptr<ExpressionSyntax> root) : _root(std::move(root))
    {}

    int Evaluator::Evaluate()
    {
        return this->EvaluateExpression(_root.get());
    }

    int Evaluator::EvaluateExpression(ExpressionSyntax* node)
    {   
        LiteralExpressionSyntax* NEnode = dynamic_cast<LiteralExpressionSyntax*>(node);
        if(NEnode != nullptr)
        {
            /* We are sure that we will have a 'int' */
            return std::get<int>(*(NEnode->_numberToken->_value));
        }

        UnaryExpressionSyntax* UEnode = dynamic_cast<UnaryExpressionSyntax*>(node);
        if(UEnode != nullptr)
        {
            int operand = this->EvaluateExpression(UEnode->_operand.get());
            
            if(UEnode->_operatorToken->_kind == SyntaxKind::MinusToken)
            {
                return -operand;
            }

            if(UEnode->_operatorToken->_kind == SyntaxKind::PlusToken)
            {
                return operand;
            }

            throw std::logic_error("Unexpected unary operator " + trylang::__syntaxStringMap[UEnode->_operatorToken->_kind]);
        }

        BinaryExpressionSyntax* BEnode = dynamic_cast<BinaryExpressionSyntax*>(node);
        if(BEnode != nullptr)
        {
            int left = this->EvaluateExpression(BEnode->_left.get());
            int right = this->EvaluateExpression(BEnode->_right.get());

            if(BEnode->_operatorToken->_kind == SyntaxKind::PlusToken)
            {
                return left + right;
            }

            if(BEnode->_operatorToken->_kind == SyntaxKind::MinusToken)
            {
                return left - right;
            }

            if(BEnode->_operatorToken->_kind == SyntaxKind::SlashToken)
            {
                return left / right;
            }

            if(BEnode->_operatorToken->_kind == SyntaxKind::StarToken)
            {
                return left * right;
            }

            throw std::logic_error("Unexpected binary operator " + trylang::__syntaxStringMap[BEnode->_operatorToken->_kind]);
        }

        ParenthesizedExpressionSyntax* PEnode = dynamic_cast<ParenthesizedExpressionSyntax*>(node);
        if(PEnode != nullptr)
        {
            return this->EvaluateExpression(PEnode->_expression.get());
        }

        throw std::logic_error("Unexpected node " + trylang::__syntaxStringMap[node->Kind()]);
    }
}