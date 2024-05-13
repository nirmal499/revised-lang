#include <codeanalysis/Binder.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/BoundNodeKind.hpp>
#include <codeanalysis/BoundScope.hpp>
#include <codeanalysis/BoundExpressionNode.hpp>
#include <algorithm>
#include <stack>

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
        auto flattened = Binder::Flatten(std::move(statement));

        auto variables = binder._scope->GetDeclaredVariable();
        auto errors = binder.Errors();

        return std::make_shared<BoundGlobalScope>(nullptr, std::move(errors), std::move(variables), std::move(flattened));
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

//    std::unique_ptr<BoundStatementNode> Binder::BindIfStatement(trylang::IfStatementSyntax *syntax)
//    {
//        auto condition = this->BindExpression(syntax->_condition.get(), Types::BOOL->Name());
//        auto statement = this->BindStatement(syntax->_thenStatement.get());
//
//        auto elseClause = static_cast<ElseClauseSyntax*>(syntax->_elseClause.get());
//        auto elseStatement = syntax->_elseClause == nullptr ? nullptr : this->BindStatement(elseClause->_elseStatement.get());
//
//        return std::make_unique<BoundIfStatement>(std::move(condition), std::move(statement), std::move(elseStatement));
//    }

    LabelSymbol Binder::GenerateLabel()
    {
        auto name = "Label{" + std::to_string(++_labelCount) + "}";
        LabelSymbol label(name);
        return label;
    }

    std::unique_ptr<BoundStatementNode> Binder::BindIfStatement(trylang::IfStatementSyntax *syntax)
    {
        /**
         * if <condition>
         *      <then>
         *
         * ----------------------------------------->
         *
         * gotoIfFalse <condition> end
         * <then>
         * end:
         *
         *
         * ================================================================================
         * if <condition>
         *      <then>
         * else
         *      <else>
         *
         * ----------------------------------------->
         *
         * gotoIfFalse <condition> else
         * <then>
         * goto end
         * else:
         * <else>
         * end:
         *
         * **/

        auto condition = this->BindExpression(syntax->_condition.get(), Types::BOOL->Name());
        auto statement = this->BindStatement(syntax->_thenStatement.get());

        std::unique_ptr<BoundStatementNode> elseStatement = nullptr;
        if(syntax->_elseClause != nullptr)
        {
            auto elseClause = static_cast<ElseClauseSyntax*>(syntax->_elseClause.get());
            elseStatement = this->BindStatement(elseClause->_elseStatement.get());
        }

        if(elseStatement == nullptr)
        {
            /**
            * if <condition>
            *      <then>
            *
            * ----------------------------------------->
            *
            * gotoIfFalse <condition> end
            * <then>
            * end:

            * **/
            auto endLabel = this->GenerateLabel();
            auto gotoFalse = std::make_unique<BoundConditionalGotoStatement>(endLabel, std::move(condition), true);
            auto endLabelStatement = std::make_unique<BoundLabelStatement>(endLabel);

            std::vector<std::unique_ptr<BoundStatementNode>> statements_1;
            statements_1.emplace_back(std::move(gotoFalse));
            statements_1.emplace_back(std::move(statement));
            statements_1.emplace_back(std::move(endLabelStatement));

            auto result = std::make_unique<BoundBlockStatement>(std::move(statements_1));

            return result;
        }
        else
        {
            /**
            * if <condition>
            *      <then>
            * else
            *      <else>
            *
            * ----------------------------------------->
            *
            * gotoIfFalse <condition> else
            * <then>
            * goto end
            * else:
            * <else>
            * end:
            *
            * **/

            auto elseLabel = this->GenerateLabel();
            auto endLabel = this->GenerateLabel();
            auto gotoFalse = std::make_unique<BoundConditionalGotoStatement>(elseLabel, std::move(condition), true);
            auto gotoEndStatement = std::make_unique<BoundGotoStatement>(endLabel);
            auto endLabelStatement = std::make_unique<BoundLabelStatement>(endLabel);
            auto elseLabelStatement = std::make_unique<BoundLabelStatement>(elseLabel);

            std::vector<std::unique_ptr<BoundStatementNode>> statements_1;
            statements_1.emplace_back(std::move(gotoFalse));
            statements_1.emplace_back(std::move(statement));
            statements_1.emplace_back(std::move(gotoEndStatement));
            statements_1.emplace_back(std::move(elseLabelStatement));
            statements_1.emplace_back(std::move(elseStatement));
            statements_1.emplace_back(std::move(endLabelStatement));

            auto result = std::make_unique<BoundBlockStatement>(std::move(statements_1));

            return result;
        }
    }

//    std::unique_ptr<BoundStatementNode> Binder::BindWhileStatement(WhileStatementSyntax *syntax)
//    {
//        auto condition = this->BindExpression(syntax->_condition.get(), Types::BOOL->Name());
//        auto body = this->BindStatement(syntax->_body.get());
//
//        return std::make_unique<BoundWhileStatement>(std::move(condition), std::move(body));
//    }

    std::unique_ptr<BoundStatementNode> Binder::BindWhileStatement(WhileStatementSyntax *syntax)
    {
        /**
         * while <condition>
         *      <body>
         *
         * ------------------------------------------------->
         * goto check
         * continue:
         * <body>
         * check:
         *      gotoIfTrue <condition> end
         * end:
         * */
        auto condition = this->BindExpression(syntax->_condition.get(), Types::BOOL->Name());
        auto body = this->BindStatement(syntax->_body.get());

        auto continueLabel = this->GenerateLabel();
        auto checkLabel = this->GenerateLabel();
        auto endLabel = this->GenerateLabel();

        auto gotoCheck = std::make_unique<BoundGotoStatement>(checkLabel);
        auto continueLabelStatement = std::make_unique<BoundLabelStatement>(continueLabel);
        auto checkLabelStatement = std::make_unique<BoundLabelStatement>(checkLabel);
        auto gotoTrue = std::make_unique<BoundConditionalGotoStatement>(continueLabel, std::move(condition), false);
        auto endLabelStatement = std::make_unique<BoundLabelStatement>(endLabel);

        std::vector<std::unique_ptr<BoundStatementNode>> statements_1;
        statements_1.emplace_back(std::move(gotoCheck));
        statements_1.emplace_back(std::move(continueLabelStatement));
        statements_1.emplace_back(std::move(body));
        statements_1.emplace_back(std::move(checkLabelStatement));
        statements_1.emplace_back(std::move(gotoTrue));
        statements_1.emplace_back(std::move(endLabelStatement));

        auto result = std::make_unique<BoundBlockStatement>(std::move(statements_1));

        return result;
    }

    std::unique_ptr<BoundStatementNode> Binder::BindWhileStatement(BoundWhileStatement *node)
    {
        /**
         * while <condition>
         *      <body>
         *
         * ------------------------------------------------->
         * goto check
         * continue:
         * <body>
         * check:
         *      gotoIfTrue <condition> end
         * end:
         * */

        auto continueLabel = this->GenerateLabel();
        auto checkLabel = this->GenerateLabel();
        auto endLabel = this->GenerateLabel();

        auto gotoCheck = std::make_unique<BoundGotoStatement>(checkLabel);
        auto continueLabelStatement = std::make_unique<BoundLabelStatement>(continueLabel);
        auto checkLabelStatement = std::make_unique<BoundLabelStatement>(checkLabel);
        auto gotoTrue = std::make_unique<BoundConditionalGotoStatement>(continueLabel, std::move(node->_condition), false);
        auto endLabelStatement = std::make_unique<BoundLabelStatement>(endLabel);

        std::vector<std::unique_ptr<BoundStatementNode>> statements_1;
        statements_1.emplace_back(std::move(gotoCheck));
        statements_1.emplace_back(std::move(continueLabelStatement));
        statements_1.emplace_back(std::move(node->_body));
        statements_1.emplace_back(std::move(checkLabelStatement));
        statements_1.emplace_back(std::move(gotoTrue));
        statements_1.emplace_back(std::move(endLabelStatement));

        auto result = std::make_unique<BoundBlockStatement>(std::move(statements_1));

        return result;
    }


//    std::unique_ptr<BoundStatementNode> Binder::BindForStatement(ForStatementSyntax *syntax)
//    {
//        auto lowerBound = this->BindExpression(syntax->_lowerBound.get(), Types::INT->Name());
//        auto upperBound = this->BindExpression(syntax->_upperBound.get(), Types::INT->Name());
//
//        _scope = std::make_shared<BoundScope>(_scope);
//
//        const auto& varname = syntax->_identifier->_text;
//        VariableSymbol variable(varname, /* isReadOnly */ true, Types::INT->Name());
//        if(!_scope->TryDeclare(variable))
//        {
//            _buffer << "Variable '" << varname << "' Already Declared\n"; /* This error will never occur */
//        }
//        auto body = this->BindStatement(syntax->_body.get());
//
//        _scope = _scope->_parent;
//
//        return std::make_unique<BoundForStatement>(std::move(variable), std::move(lowerBound), std::move(upperBound), std::move(body));
//    }

    std::unique_ptr<BoundStatementNode> Binder::BindForStatement(ForStatementSyntax *syntax)
    {
        /**
         *
         * for <var> = <lower> to <upper>
         *      <body>
         *
         * ----------------------------------------------------------------->
         *
         * {
         *      var <var> = <lower>
         *      let upperBound = <upper>
         *      while (<var> <= upperBound)
         *      {
         *          <body>
         *          <var> = <var> + 1
         *      }
         * }
         *
         * */

        auto lowerBound = this->BindExpression(syntax->_lowerBound.get(), Types::INT->Name());
        auto upperBound = this->BindExpression(syntax->_upperBound.get(), Types::INT->Name());

        _scope = std::make_shared<BoundScope>(_scope);

        const auto& varname = syntax->_identifier->_text;
        VariableSymbol variable(varname, /* isReadOnly */ true, Types::INT->Name());
        if(!_scope->TryDeclare(variable))
        {
            _buffer << "Variable '" << varname << "' Already Declared\n";
        }

        auto body = this->BindStatement(syntax->_body.get());

        VariableSymbol upperBoundSymbol("upperBound", true, Types::INT->Name());
        if(!_scope->TryDeclare(upperBoundSymbol))
        {
            _buffer << "Variable '" << "upperBound" << "' Already Declared\n"; /* This error is not possible */
        }

        _scope = _scope->_parent;

        auto variableDeclaration = std::make_unique<BoundVariableDeclaration>(variable, std::move(lowerBound));
        auto upperBoundVariableDeclaration = std::make_unique<BoundVariableDeclaration>(upperBoundSymbol, std::move(upperBound));

        auto condition = std::make_unique<BoundBinaryExpression>(
                std::make_unique<BoundVariableExpression>(variable),
                BoundBinaryOperator::Bind(SyntaxKind::LessThanEqualsToken, Types::INT->Name(), Types::INT->Name()),
                std::make_unique<BoundVariableExpression>(upperBoundSymbol)
        );
        auto increment = std::make_unique<BoundExpressionStatement>(
                std::make_unique<BoundAssignmentExpression>(
                        variable,
                        std::make_unique<BoundBinaryExpression>(
                                std::make_unique<BoundVariableExpression>(variable),
                                BoundBinaryOperator::Bind(SyntaxKind::PlusToken, Types::INT->Name(),Types::INT->Name()),
                                std::make_unique<BoundLiteralExpression>(1))
                ));

        /** This has to be done instead of doing std::make_unique<BoundBlockStatement>({std::move(body), std::move(increment)}) because BoundBlockStatement is explicit */
        std::vector<std::unique_ptr<BoundStatementNode>> statements_1(2);
        statements_1.emplace_back(std::move(body));
        statements_1.emplace_back(std::move(increment));

        auto whileBody = std::make_unique<BoundBlockStatement>(std::move(statements_1));
        auto whileStatement = std::make_unique<BoundWhileStatement>(std::move(condition), std::move(whileBody));

        auto loweredWhileStatement = this->BindWhileStatement(whileStatement.get());

        std::vector<std::unique_ptr<BoundStatementNode>> statements_2(2);
        statements_2.emplace_back(std::move(variableDeclaration));
        statements_2.emplace_back(std::move(upperBoundVariableDeclaration));
        statements_2.emplace_back(std::move(loweredWhileStatement));

        auto result = std::make_unique<BoundBlockStatement>(std::move(statements_2));

        return result;
    }

    /* All the BoundBlockStatement will be removed and flattened into their BoundStatementNode */
    std::unique_ptr<BoundBlockStatement> Binder::Flatten(std::unique_ptr<BoundStatementNode> statement)
    {
        std::vector<std::unique_ptr<BoundStatementNode>> statements;
        std::stack<std::unique_ptr<BoundStatementNode>> m_stack;

        m_stack.push(std::move(statement));

        while(!m_stack.empty())
        {
            auto current = std::move(m_stack.top());
            m_stack.pop();

            auto* BBnode = dynamic_cast<BoundBlockStatement*>(current.get());
            if(BBnode != nullptr)
            {
                for(const auto& stmt: BBnode->_statements){}
                for(auto it = BBnode->_statements.rbegin(); it != BBnode->_statements.rend(); ++it)
                {
                    m_stack.push(std::move(*it));
                }
            }
            else
            {
                statements.emplace_back(std::move(current));
            }
        }

        return std::make_unique<BoundBlockStatement>(std::move(statements));

    }
}