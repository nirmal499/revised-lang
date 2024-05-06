#include <codeanalysis/Binder.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>

namespace trylang
{
    std::unique_ptr<BoundExpressionNode> Binder::BindExpression(ExpressionSyntax* syntax)
    {
        switch (syntax->Kind())
        {
        case SyntaxKind::LiteralExpression:
            return this->BindLiteralExpression(static_cast<LiteralExpressionSyntax*>(syntax));
        case SyntaxKind::UnaryExpression:
            return this->BindUnaryExpression(static_cast<UnaryExpressionSyntax*>(syntax));
        case SyntaxKind::BinaryExpression:
            return this->BindBinaryExpression(static_cast<BinaryExpressionSyntax*>(syntax));
        default:
            throw std::logic_error("Unexpected syntax " + __syntaxStringMap[syntax->Kind()]);
        }
    }

    std::string Binder::Errors()
    {
        return buffer.str();
    }

    std::unique_ptr<BoundExpressionNode> Binder::BindLiteralExpression(LiteralExpressionSyntax* syntax)
    {

        int value = 0;

        if(syntax->_value.has_value())
        {
            return std::make_unique<BoundLiteralExpression>(syntax->_value);
        }

        return std::make_unique<BoundLiteralExpression>(value);
    }

    std::unique_ptr<BoundExpressionNode> Binder::BindUnaryExpression(UnaryExpressionSyntax* syntax)
    {
        auto boundOperand = this->BindExpression(syntax->_operand.get());
        auto boundOperatorKind = this->BindUnaryOperatorKind(syntax->_operatorToken->Kind(), boundOperand->Type());
        if(!boundOperatorKind.has_value())
        {
            buffer << "Unary operator '" << syntax->_operatorToken->_text << "' is not defined for type " << boundOperand->Type().name() << "\n";
            return boundOperand;
        }

        return std::make_unique<BoundUnaryExpression>(*boundOperatorKind, std::move(boundOperand));
    }

    std::unique_ptr<BoundExpressionNode> Binder::BindBinaryExpression(BinaryExpressionSyntax* syntax)
    {
        auto boundLeft = this->BindExpression(syntax->_left.get());
        auto boundRight = this->BindExpression(syntax->_right.get());
        auto boundOperatorKind = this->BindBinaryOperatorKind(syntax->_operatorToken->Kind(), boundLeft->Type(), boundRight->Type());
        
        if(!boundOperatorKind.has_value())
        {
            buffer << "Binary operator '" << syntax->_operatorToken->_text << "' is not defined for types " << boundLeft->Type().name() << " and " << boundRight->Type().name() << "\n";
            return boundLeft;
        }

        return std::make_unique<BoundBinaryExpression>(std::move(boundLeft), *boundOperatorKind, std::move(boundRight));
    }

    std::optional<BoundBinaryOperatorKind> Binder::BindBinaryOperatorKind(SyntaxKind kind, const std::type_info& leftType, const std::type_info& rightType)
    {
        if(leftType != typeid(int) || rightType != typeid(int))
        {
            return std::nullopt;
        }

        switch (kind)
        {
        case SyntaxKind::PlusToken:
            return BoundBinaryOperatorKind::Addition;
        case SyntaxKind::MinusToken:
            return BoundBinaryOperatorKind::Subtraction;
        case SyntaxKind::SlashToken:
            return BoundBinaryOperatorKind::Division;
        case SyntaxKind::StarToken:
            return BoundBinaryOperatorKind::Multiplication;
        default:
            throw std::logic_error("Unexpected binary operator " + __syntaxStringMap[kind]);
        }
    }

    std::optional<BoundUnaryOperatorKind> Binder::BindUnaryOperatorKind(SyntaxKind kind, const std::type_info& operandType)
    {
        if(operandType != typeid(int))
        {
            return std::nullopt;
        }

        switch (kind)
        {
        case SyntaxKind::PlusToken:
            return BoundUnaryOperatorKind::Identity;
        case SyntaxKind::MinusToken:
            return BoundUnaryOperatorKind::Negation;
        default:
            throw std::logic_error("Unexpected unary operator " + __syntaxStringMap[kind]);
        }
    }
}