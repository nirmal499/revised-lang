#include <codeanalysis/Binder.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/BoundNodeKind.hpp>
#include <codeanalysis/BoundScope.hpp>
#include <codeanalysis/BoundExpressionNode.hpp>
#include <codeanalysis/Conversion.hpp>
#include <algorithm>
#include <stack>

namespace trylang
{

    Binder::Binder(const std::shared_ptr<BoundScope>& parent)
    {
        _scope = std::make_shared<BoundScope>(parent);

        (void)_scope->TryDeclareFunction(BUILT_IN_FUNCTIONS::MAP.at("print"));
        (void)_scope->TryDeclareFunction(BUILT_IN_FUNCTIONS::MAP.at("input"));
    }

    std::shared_ptr<BoundGlobalScope> Binder::BindGlobalScope(CompilationUnitSyntax* syntax)
    {
        Binder binder(nullptr);

        auto statement = binder.BindStatement(syntax->_statement.get());
        auto flattened = Binder::Flatten(std::move(statement));

        auto variables = binder._scope->GetDeclaredVariables();
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
        const char* type = this->BindTypeClause(syntax->_typeClause.get());
        auto expression = this->BindExpression(syntax->_expression.get());
        auto variableType = type == nullptr ? expression->Type() : type;
        auto conversionExpression = this->BindConversion(variableType, std::move(expression));

        VariableSymbol variable(varname, isReadOnly, variableType);
        if(!_scope->TryDeclareVariable(variable))
        {
            _buffer << "Variable '" << varname << "' already declared\n";
        }

        return std::make_unique<BoundVariableDeclaration>(std::move(variable), std::move(conversionExpression));
    }

    const char* Binder::BindTypeClause(TypeClauseSyntax *syntax)
    {
        if(syntax == nullptr)
        {
            return nullptr;
        }

        auto* type = trylang::LookUpType(syntax->_identifierToken->_text);
        if(type == nullptr)
        {
            _buffer << "Type '" << syntax->_identifierToken->_text <<  "' doesn't exists.\n";
            return nullptr;
        }

        return type->Name();
    }


    std::unique_ptr<BoundStatementNode> Binder::BindBlockStatement(BlockStatementSyntax *syntax)
    {
        std::vector<std::unique_ptr<BoundStatementNode>> statements;

        if(_turnOnScopingInBlockStatement)
        {
            _scope = std::make_shared<BoundScope>(_scope);
        }

        for(const auto& statementSyntax: syntax->_statements)
        {
            auto statement = this->BindStatement(statementSyntax.get());
            statements.emplace_back(std::move(statement));
        }

        if(_turnOnScopingInBlockStatement)
        {
            _scope = _scope->_parent;
        }

        return std::make_unique<BoundBlockStatement>(std::move(statements));
    }

    std::unique_ptr<BoundStatementNode> Binder::BindExpressionStatement(ExpressionStatementSyntax *syntax)
    {
        /**
         *
         * let name = print("Name") ---> will throw error
         * */
        auto expression = this->BindExpression(syntax->_expression.get(), /* canBeVoid */ true);
        return std::make_unique<BoundExpressionStatement>(std::move(expression));
    }

    std::unique_ptr<BoundExpressionNode> Binder::BindExpression(ExpressionSyntax* syntax, const char* targetType)
    {
        /*
         * BindConversion checks whatever type the expression syntax results in is convertible to type targetType */
        auto result = this->BindConversion(targetType, syntax);
        return result;
    }

    std::unique_ptr<BoundExpressionNode> Binder::BindExpression(ExpressionSyntax *syntax, bool canBeVoid)
    {
        auto result = this->BindExpressionInternal(syntax);
        if(!canBeVoid && (std::strcmp(result->Type(), Types::VOID->Name()) == 0))
        {
            _buffer << "Expression Must have a value\n";
            return std::make_unique<BoundErrorExpression>();
        }

        return result;
    }

    std::unique_ptr<BoundExpressionNode> Binder::BindExpressionInternal(ExpressionSyntax* syntax)
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
            case SyntaxKind::CallExpression:
                return this->BindCallExpression(static_cast<CallExpressionSyntax*>(syntax));
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
        if(!_scope->TryLookUpVariable(varname, variable))
        {
            _buffer << "Undefined Name " << varname << "\n";
            return std::make_unique<BoundErrorExpression>();
        }

        return std::make_unique<BoundVariableExpression>(std::move(variable));
    }

    /*
     * Assignments are right associative :- `a= b = c`
     * */
    std::unique_ptr<BoundExpressionNode> Binder::BindAssignmentExpression(AssignmentExpressionSyntax *syntax)
    {
        const auto& varname = syntax->_identifierToken->_text;
        auto boundExpression = this->BindExpression(syntax->_expression.get());

        VariableSymbol variable;
        if(!_scope->TryLookUpVariable(varname, variable))
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

        auto conversionExpression = this->BindConversion(variable._type, std::move(boundExpression));

        return std::make_unique<BoundAssignmentExpression>(std::move(variable), std::move(conversionExpression));
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

        /* Below commented code will stop the issue of cascading errors */
        if(std::strcmp(boundOperand->Type(), Types::ERROR->Name()) == 0)
        {
            return std::make_unique<BoundErrorExpression>();
        }

        auto boundOperatorKind = BoundUnaryOperator::Bind(syntax->_operatorToken->Kind(),boundOperand->Type());

        if(boundOperatorKind == nullptr)
        {
            _buffer << "Unary operator '" << syntax->_operatorToken->_text << "' is not defined for type " << boundOperand->Type() << "\n";
            return std::make_unique<BoundErrorExpression>();
        }

        return std::make_unique<BoundUnaryExpression>(boundOperatorKind, std::move(boundOperand));
    }

    std::unique_ptr<BoundExpressionNode> Binder::BindBinaryExpression(BinaryExpressionSyntax* syntax)
    {
        auto boundLeft = this->BindExpression(syntax->_left.get());
        auto boundRight = this->BindExpression(syntax->_right.get());

        /* Below commented code will stop the issue of cascading errors */
        if((std::strcmp(boundLeft->Type(), Types::ERROR->Name()) == 0) || (std::strcmp(boundRight->Type(), Types::ERROR->Name()) == 0))
        {
            return std::make_unique<BoundErrorExpression>();
        }

        auto boundOperatorKind = BoundBinaryOperator::Bind(syntax->_operatorToken->Kind(), boundLeft->Type(), boundRight->Type());
        
        if(boundOperatorKind == nullptr)
        {
            _buffer << "Binary operator '" << syntax->_operatorToken->_text << "' is not defined for types " << boundLeft->Type() << " and " << boundRight->Type() << "\n";
            return std::make_unique<BoundErrorExpression>();
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
//        if(!_scope->TryDeclareVariable(variable))
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
        if(!_scope->TryDeclareVariable(variable))
        {
            _buffer << "Variable '" << varname << "' Already Declared\n";
        }

        _turnOnScopingInBlockStatement = false;
        auto body = this->BindStatement(syntax->_body.get());
        _turnOnScopingInBlockStatement = true;

        VariableSymbol upperBoundSymbol("upperBound", true, Types::INT->Name());
        if(!_scope->TryDeclareVariable(upperBoundSymbol))
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

    std::unique_ptr<BoundExpressionNode> Binder::BindConversion(const char* type, ExpressionSyntax *syntax, bool allowExplicit)
    {
        auto expression = this->BindExpression(syntax);
        return this->BindConversion(type, std::move(expression), allowExplicit);
    }

    std::unique_ptr<BoundExpressionNode> Binder::BindConversion(const char* type, std::unique_ptr<BoundExpressionNode> expression, bool allowExplicit)
    {
        /**
         * Here we check whether the given expression is of allowed to be converted to type given by the const char* type
         * */
        auto conversion = trylang::Classify(/* fromType */ expression->Type(), /* ToType */ type);
        if(!conversion->_exists)
        {
            if(
                    (std::strcmp(expression->Type(), Types::ERROR->Name())) == 0 &&
                    (std::strcmp(type, Types::ERROR->Name())) == 0
            )
            {
                _buffer << "Cannot convert " << expression->Type() << " to " << type << "\n";
            }
            return std::make_unique<BoundErrorExpression>();
        }

        if(conversion->_isExplicit && !allowExplicit)
        {
            _buffer << "Cannot convert " << expression->Type() << " to " << type << ". An explicit conversion exists; are you missing a cast ?\n";
            return std::make_unique<BoundErrorExpression>();
        }

        /**
         * For not allowing Explicit conversions */
//        if(!conversion->_exists || conversion->_isExplicit)
//        {
//            if(
//                    ((std::strcmp(expression->Type(), Types::ERROR->Name())) == 0 &&
//                     (std::strcmp(type, Types::ERROR->Name())) == 0) || (std::strcmp(expression->Type(), type) != 0)
//                    )
//            {
//                _buffer << "Cannot convert " << expression->Type() << " to " << type << "\n";
//            }
//            return std::make_unique<BoundErrorExpression>();
//        }

        if(conversion->_isIdentity)
        {
            return expression;
        }

        /**
         * Here we are returning "BoundConversionExpression" containing {type, expression}. Basically we checked if the conversion is allowed or not in
         * above code and here we are returning "BoundConversionExpression", so that it can be evaluated in the evaluator
         * */
        return std::make_unique<BoundConversionExpression>(type, std::move(expression));
    }

    std::unique_ptr<BoundExpressionNode> Binder::BindCallExpression(CallExpressionSyntax *syntax)
    {

        auto* type = trylang::LookUpType(syntax->_identifer->_text);
        if(syntax->_arguments.size() == 1 && type != nullptr)
        {
            return this->BindConversion(type->_typeName, syntax->_arguments[0].get(), /* allowExplicit */ true);
        }

        std::vector<std::unique_ptr<BoundExpressionNode>> boundArguments;
        for(const auto& expr: syntax->_arguments)
        {
            auto boundExpr = this->BindExpression(expr.get());
            boundArguments.emplace_back(std::move(boundExpr));
        }

        FunctionSymbol function;
        if(!_scope->TryLookUpFunction(syntax->_identifer->_text, function))
        {
            _buffer << "Function '" << syntax->_identifer->_text << "' doesn't exist\n";
            return std::make_unique<BoundErrorExpression>();
        }

        if(syntax->_arguments.size() != function._parameters.size())
        {
            _buffer << "Wrong No.of Arguments Reported in function call " << syntax->_identifer->_text << "\n";
            return std::make_unique<BoundErrorExpression>();
        }

        for(auto i = 0; i < syntax->_arguments.size(); i++)
        {
            const auto& argument = boundArguments[i];
            const auto& parameter = function._parameters[i];

            if(argument->Type() != parameter._type)
            {
                _buffer << "Wrong Argument Type provided in function call " << syntax->_identifer->_text << "\n";
                return std::make_unique<BoundErrorExpression>();
            }
        }

        return std::make_unique<BoundCallExpression>(function, std::move(boundArguments));

    }

}