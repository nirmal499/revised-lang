#include <codeanalysis/Binder.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/BoundNodeKind.hpp>

namespace trylang
{
    Binder::Binder(std::unordered_map<std::string, oobject_t>& variables)
        : _variables(variables)
    {}

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
            case SyntaxKind::ParenthesizedExpression:
                return this->BindParenthesizedExpression(static_cast<ParenthesizedExpressionSyntax*>(syntax));
            case SyntaxKind::NameExpression:
                return this->BindNameExpression(static_cast<NameExpressionSyntax*>(syntax));
            case SyntaxKind::AssignmentExpression:
                return this->BindAssignmentExpression(static_cast<AssignmentExpressionSyntax*>(syntax));
            default:
                throw std::logic_error("Unexpected syntax " + __syntaxStringMap[syntax->Kind()]);
        }
    }

    std::string Binder::Errors()
    {
        return _buffer.str();
    }

    std::unique_ptr<BoundExpressionNode> Binder::BindNameExpression(NameExpressionSyntax *syntax)
    {
        const auto& varname = syntax->_identifierToken->_text;
        auto it = _variables.find(varname);
        if(it == _variables.end())
        {
            _buffer << "Undefined Name " << varname << "\n";
            return std::make_unique<BoundLiteralExpression>(0);
        }

        const std::type_info& type_information = trylang::assign_type_info(it->second);
        return std::make_unique<BoundVariableExpression>(varname, type_information);
    }

    std::unique_ptr<BoundExpressionNode> Binder::BindAssignmentExpression(AssignmentExpressionSyntax *syntax)
    {
        const auto& varname = syntax->_identifierToken->_text;
        auto boundExpression = this->BindExpression(syntax->_expression.get());

        return std::make_unique<BoundAssignmentExpression>(varname, std::move(boundExpression));
    }

    std::unique_ptr<BoundExpressionNode> Binder::BindParenthesizedExpression(ParenthesizedExpressionSyntax* syntax)
    {
        return this->BindExpression(syntax->_expression.get());
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
        auto boundOperatorKind = BoundUnaryOperator::Bind(syntax->_operatorToken->Kind(),boundOperand->Type());

        if(boundOperatorKind == nullptr)
        {
            _buffer << "Unary operator '" << syntax->_operatorToken->_text << "' is not defined for type " << boundOperand->Type().name() << "\n";
            return boundOperand;
        }

        return std::make_unique<BoundUnaryExpression>(boundOperatorKind, std::move(boundOperand));
    }

    std::unique_ptr<BoundExpressionNode> Binder::BindBinaryExpression(BinaryExpressionSyntax* syntax)
    {
        auto boundLeft = this->BindExpression(syntax->_left.get());
        auto boundRight = this->BindExpression(syntax->_right.get());
        auto boundOperatorKind = BoundBinaryOperator::Bind(syntax->_operatorToken->Kind(), boundLeft->Type(), boundRight->Type());
        
        if(boundOperatorKind == nullptr)
        {
            _buffer << "Binary operator '" << syntax->_operatorToken->_text << "' is not defined for types " << boundLeft->Type().name() << " and " << boundRight->Type().name() << "\n";
            return boundLeft;
        }

        return std::make_unique<BoundBinaryExpression>(std::move(boundLeft), boundOperatorKind, std::move(boundRight));
    }
}