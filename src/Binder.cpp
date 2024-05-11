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
        auto statement = binder.BindStatement(syntax->_statement.get());
        auto variables = binder._scope->GetDeclaredVariable();
        auto errors = binder.Errors();

        return std::make_shared<BoundGlobalScope>(nullptr, std::move(errors), std::move(variables), std::move(statement));
    }

    std::unique_ptr<BoundStatementNode> Binder::BindStatement(StatementSyntax* syntax)
    {
        switch (syntax->Kind())
        {
            case SyntaxKind::BlockStatement:
                return this->BindBlockStatement(static_cast<BlockStatementSyntax*>(syntax));
            case SyntaxKind::ExpressionStatement:
                return this->BindExpressionStatement(static_cast<ExpressionStatementSyntax*>(syntax));
            case SyntaxKind::VariableDeclarationStatement:
                return this->BindVariableDeclaration(static_cast<VariableDeclarationSyntax*>(syntax));
            case SyntaxKind::IfStatement:
                return this->BindIfStatement(static_cast<IfStatementSyntax*>(syntax));
            case SyntaxKind::WhileStatement:
                return this->BindWhileStatement(static_cast<WhileStatementSyntax*>(syntax));
            case SyntaxKind::ForStatement:
                return this->BindForStatement(static_cast<ForStatementSyntax*>(syntax));
            default:
                throw std::logic_error("Unexpected syntax " + __syntaxStringMap[syntax->Kind()]);
        }
    }

    std::unique_ptr<BoundStatementNode> Binder::BindVariableDeclaration(VariableDeclarationSyntax *syntax)
    {
        const auto& varname = syntax->_identifier->_text;
        auto isReadOnly = syntax->_keyword->Kind() == SyntaxKind::LetKeyword;
        auto expression = this->BindExpression(syntax->_expression.get());

        VariableSymbol variable(varname, isReadOnly, expression->Type());

        if(!_scope->TryDeclare(variable))
        {
            _buffer << "Variable '" << varname << "' already declared\n";
        }

        return std::make_unique<BoundVariableDeclaration>(std::move(variable), std::move(expression));
    }

    std::unique_ptr<BoundStatementNode> Binder::BindBlockStatement(BlockStatementSyntax *syntax)
    {
        std::vector<std::unique_ptr<BoundStatementNode>> statements;

        _scope = std::make_shared<BoundScope>(_scope);

        for(const auto& statementSyntax: syntax->_statements)
        {
            auto statement = this->BindStatement(statementSyntax.get());
            statements.emplace_back(std::move(statement));
        }

        _scope = _scope->_parent;

        return std::make_unique<BoundBlockStatement>(std::move(statements));
    }

    std::unique_ptr<BoundStatementNode> Binder::BindExpressionStatement(ExpressionStatementSyntax *syntax)
    {
        auto expression = this->BindExpression(syntax->_expression.get());
        return std::make_unique<BoundExpressionStatement>(std::move(expression));
    }

    std::unique_ptr<BoundExpressionNode> Binder::BindExpression(ExpressionSyntax* syntax, const char* targetType)
    {
        auto result = this->BindExpression(syntax);
        if(std::strcmp(result->Type(), targetType) != 0)
        {
            _buffer << "Cannot convert from " << result->Type() << " to " << targetType << "\n";
        }

        return result;
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
            _buffer << "Undefined Name " << varname << "\n";
            return std::make_unique<BoundLiteralExpression>(0);
        }

        /* varname variable is declared already */
        if(variable._isReadOnly)
        {
            _buffer << "Variable '" << varname << "' is read-only and cannot be reassigned\n";
        }

        if(std::strcmp(variable._type, boundExpression->Type()) != 0)
        {
            _buffer << "Cannot convert from " << boundExpression->Type() << " to " << variable._type << "\n";
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

    std::unique_ptr<BoundStatementNode> Binder::BindIfStatement(trylang::IfStatementSyntax *syntax)
    {
        auto condition = this->BindExpression(syntax->_condition.get(), typeid(bool).name());
        auto statement = this->BindStatement(syntax->_thenStatement.get());

        auto elseClause = static_cast<ElseClauseSyntax*>(syntax->_elseClause.get());
        auto elseStatement = syntax->_elseClause == nullptr ? nullptr : this->BindStatement(elseClause->_elseStatement.get());

        return std::make_unique<BoundIfStatement>(std::move(condition), std::move(statement), std::move(elseStatement));
    }

    std::unique_ptr<BoundStatementNode> Binder::BindWhileStatement(WhileStatementSyntax *syntax)
    {
        auto condition = this->BindExpression(syntax->_condition.get(), typeid(bool).name());
        auto body = this->BindStatement(syntax->_body.get());

        return std::make_unique<BoundWhileStatement>(std::move(condition), std::move(body));
    }

    std::unique_ptr<BoundStatementNode> Binder::BindForStatement(ForStatementSyntax *syntax)
    {
        const auto& varname = syntax->_identifier->_text;
        VariableSymbol variable(varname, /* isReadOnly */ true, typeid(int).name());
        if(!_scope->TryDeclare(variable))
        {
            _buffer << "Variable '" << varname << "' Already Declared\n";
        }
        
        auto lowerBound = this->BindExpression(syntax->_lowerBound.get(), typeid(int).name());
        auto upperBound = this->BindExpression(syntax->_upperBound.get(), typeid(int).name());

        auto body = this->BindStatement(syntax->_body.get());

        return std::make_unique<BoundForStatement>(std::move(variable), std::move(lowerBound), std::move(upperBound), std::move(body));

    }
}