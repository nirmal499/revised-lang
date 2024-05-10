#include <codeanalysis/Binder.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/BoundNodeKind.hpp>
#include <codeanalysis/BoundScope.hpp>
#include <algorithm>

namespace trylang
{
    Binder::Binder(const std::shared_ptr<BoundScope>& parent)
    {
        _scope = std::make_shared<BoundScope>(parent);
    }

    std::shared_ptr<BoundGlobalScope> Binder::BindGlobalScope(CompilationUnitSyntax* syntax)
    {
        Binder binder(nullptr);
        auto rootExpression = binder.BindExpression(syntax->_rootExpression.get());
        auto variables = binder._scope->GetDeclaredVariable();
        auto errors = binder.Errors();

        return std::make_shared<BoundGlobalScope>(nullptr, std::move(errors), std::move(variables), std::move(rootExpression));
    }

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

        VariableSymbol variable;
        if(!_scope->TryLookUp(varname, variable))
        {
            _buffer << "Undefined Name " << varname << "\n";
            return std::make_unique<BoundLiteralExpression>(0);
        }

        return std::make_unique<BoundVariableExpression>(std::move(variable));
    }

    std::unique_ptr<BoundExpressionNode> Binder::BindAssignmentExpression(AssignmentExpressionSyntax *syntax)
    {
        const auto& varname = syntax->_identifierToken->_text;
        auto boundExpression = this->BindExpression(syntax->_expression.get());

        VariableSymbol variable;
        if(!_scope->TryLookUp(varname, variable))
        {
            /* We did not have varname variable declared */

            variable._name = varname;
            variable._type = boundExpression->Type();
            (void)_scope->TryDeclare(variable);

        }

        /* varname variable is declared already */
        if(std::strcmp(variable._type, boundExpression->Type()) != 0)
        {
            _buffer << "Cannot convert from " << variable._type << " to " << boundExpression->Type() << "\n";
            return boundExpression;
        }

        return std::make_unique<BoundAssignmentExpression>(std::move(variable), std::move(boundExpression));
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
            _buffer << "Unary operator '" << syntax->_operatorToken->_text << "' is not defined for type " << boundOperand->Type() << "\n";
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
            _buffer << "Binary operator '" << syntax->_operatorToken->_text << "' is not defined for types " << boundLeft->Type() << " and " << boundRight->Type() << "\n";
            return boundLeft;
        }

        return std::make_unique<BoundBinaryExpression>(std::move(boundLeft), boundOperatorKind, std::move(boundRight));
    }
}