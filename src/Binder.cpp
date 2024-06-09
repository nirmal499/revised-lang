#include <codeanalysis/Binder.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/BoundNodeKind.hpp>
#include <codeanalysis/BoundScope.hpp>
#include <codeanalysis/BoundExpressionNode.hpp>
#include <codeanalysis/Conversion.hpp>
#include <algorithm>
#include <memory>
#include <sstream>
#include <stack>

namespace trylang
{
    std::stringstream Binder::_buffer; /* Definition */
    std::string Binder::Errors()
    {
        return _buffer.str();
    }

    Binder::Binder(const std::shared_ptr<BoundScope>& parent, FunctionSymbol* function)
    {
        _buffer.str("");

        _scope = std::make_shared<BoundScope>(parent);
        _function = function;

        if(_function != nullptr)
        {
            for(const auto& parameter: _function->_parameters)
            {
                auto parameter_sp = std::make_shared<ParameterSymbol>(parameter);
                _scope->TryDeclareVariable(parameter_sp);
            }
        }

        (void)_scope->TryDeclareFunction(BUILT_IN_FUNCTIONS::MAP.at("print"));
        (void)_scope->TryDeclareFunction(BUILT_IN_FUNCTIONS::MAP.at("input"));
    }

    std::unique_ptr<BoundProgram> Binder::BindProgram(CompilationUnitSyntax* syntaxTree)
    {
        Binder binder(nullptr, nullptr);
        std::vector<std::unique_ptr<BoundStatementNode>> statements;

        for(const auto& member: syntaxTree->_statements)
        {
            if(member->Kind() == SyntaxKind::FunctionDeclarationStatement)
            {
                binder.BindFunctionDeclaration(static_cast<FunctionDeclarationStatementSyntax*>(member.get()));
            }
            else
            {
                auto statement = binder.BindStatement(member.get());
                statements.emplace_back(std::move(statement));
            }
        }

        auto statement = std::make_unique<BoundBlockStatement>(std::move(statements));
        auto flattened = Binder::Flatten(std::move(statement));
        auto errors = Binder::Errors();

        auto scope = std::make_shared<BoundScope>(nullptr); /* Global Environment */

        scope->_variables = std::move(binder._scope->_variables);
        scope->_functions = std::move(binder._scope->_functions);

        std::unordered_map<std::string, std::pair<std::shared_ptr<FunctionSymbol>, std::unique_ptr<BoundBlockStatement>>> functionBodies;
        for(const auto& function: scope->_functions)
        {
            if(function.second->_declaration == nullptr)
            {
                /* These functions are global functions declared by the MAKER */
                continue;
            }

            Binder binder(scope, function.second.get());
            auto body = binder.BindStatement(function.second->_declaration->_body.get());
            auto flattenedBody = Binder::Flatten(std::move(body));
            functionBodies[function.first] = std::make_pair(function.second, std::move(flattenedBody));

            errors.append(Binder::Errors());
        }

        std::unique_ptr<BoundProgram> boundProgram;
        if(!errors.empty())
        {
            std::cout << "Binding Errors Reported:\n";
            std::cout << "\n" << errors << "\n";
        }
        else
        {
            boundProgram = std::make_unique<BoundProgram>(std::move(scope->_functions), std::move(scope->_variables),std::move(functionBodies), std::move(flattened));
        }

        return boundProgram;
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
                return this->BindVariableDeclaration(static_cast<VariableDeclarationStatementSyntax*>(syntax));
            case SyntaxKind::IfStatement:
                return this->BindIfStatement(static_cast<IfStatementSyntax*>(syntax));
            case SyntaxKind::WhileStatement:
                return this->BindWhileStatement(static_cast<WhileStatementSyntax*>(syntax));
            case SyntaxKind::ForStatement:
                return this->BindForStatement(static_cast<ForStatementSyntax*>(syntax));
            case SyntaxKind::BreakStatement:
                return this->BindBreakStatement(static_cast<BreakStatementSyntax*>(syntax));
            case SyntaxKind::ContinueStatement:
                return this->BindContinueStatement(static_cast<ContinueStatementSyntax*>(syntax));
            default:
                throw std::logic_error("Unexpected syntax " + __syntaxStringMap[syntax->Kind()]);
        }
    }

    std::unique_ptr<BoundStatementNode> Binder::BindVariableDeclaration(VariableDeclarationStatementSyntax *syntax)
    {
        auto isReadOnly = syntax->_keyword->Kind() == SyntaxKind::LetKeyword;
        const char* type = this->BindTypeClause(syntax->_typeClause.get());
        auto expression = this->BindExpression(syntax->_expression.get());
        auto variableType = type == nullptr ? expression->Type() : type;
        auto conversionExpression = this->BindConversion(variableType, std::move(expression));
        auto variable = this->BindVariable(syntax->_identifier->_text, isReadOnly, variableType);

        return std::make_unique<BoundVariableDeclaration>(variable, std::move(conversionExpression));
    }

    std::shared_ptr<VariableSymbol> Binder::BindVariable(std::string varName, bool isReadOnly, const char* type)
    {
        std::shared_ptr<VariableSymbol> variable = nullptr;
        if(_function == nullptr)
        {
            variable = std::make_shared<GlobalVariableSymbol>(std::move(varName), isReadOnly, type);
        }
        else
        {
            variable = std::make_shared<LocalVariableSymbol>(std::move(varName), isReadOnly, type);
        }

        if(!_scope->TryDeclareVariable(variable))
        {
            _buffer << "Symbol '" << variable->_name << "' already declared\n";
        }

        return variable;
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

    std::unique_ptr<BoundExpressionNode> Binder::BindNameExpression(NameExpressionSyntax *syntax)
    {
        const auto& varname = syntax->_identifierToken->_text;

        auto variable = _scope->TryLookUpVariable(varname);
        if(variable == nullptr)
        {
            _buffer << "Undefined Name " << varname << "\n";
            return std::make_unique<BoundErrorExpression>();
        }

        return std::make_unique<BoundVariableExpression>(variable);
    }

    /*
     * Assignments are right associative :- `a= b = c`
     * */
    std::unique_ptr<BoundExpressionNode> Binder::BindAssignmentExpression(AssignmentExpressionSyntax *syntax)
    {
        const auto& varname = syntax->_identifierToken->_text;
        auto boundExpression = this->BindExpression(syntax->_expression.get());

        auto variable = _scope->TryLookUpVariable(varname);
        if(variable == nullptr)
        {
            /* We did not have varname variable declared */
            _buffer << "Undefined Name " << varname << "\n";
            return std::make_unique<BoundLiteralExpression>(0);
        }

        /* varname variable is declared already */
        if(variable->_isReadOnly)
        {
            _buffer << "Variable '" << varname << "' is read-only and cannot be reassigned\n";
        }

        auto conversionExpression = this->BindConversion(variable->_type, std::move(boundExpression));

        return std::make_unique<BoundAssignmentExpression>(variable, std::move(conversionExpression));
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

        if(std::strcmp(boundOperand->Type(), Types::ERROR->Name()) == 0)
        {
            _buffer << "type " << boundOperand->Type() << " caused issue\n";
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

        if((std::strcmp(boundLeft->Type(), Types::ERROR->Name()) == 0) || (std::strcmp(boundRight->Type(), Types::ERROR->Name()) == 0))
        {
            _buffer << "type " << boundLeft->Type() << " and " << boundLeft->Type() << " caused issue\n";
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

    LabelSymbol Binder::GenerateLabel()
    {
        auto name = "Label{" + std::to_string(++_labelCountForIfStatement) + "}";
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
            auto elseClause = static_cast<ElseStatementSyntax*>(syntax->_elseClause.get());
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

    std::pair<std::unique_ptr<BoundStatementNode>, std::pair<LabelSymbol, LabelSymbol>> Binder::BindLoopBody(StatementSyntax* body)
    {
        _labelCountForBreakAndContinueStatement++;
        LabelSymbol breakLabel("break{" + std::to_string(_labelCountForBreakAndContinueStatement) + "}");
        LabelSymbol continueLabel("continue{" + std::to_string(_labelCountForBreakAndContinueStatement) + "}");

        auto loopLabel = std::make_pair(breakLabel, continueLabel);

        _loopStack.push(loopLabel);
        auto boundedBody = this->BindStatement(body);
        _loopStack.pop();

        return std::make_pair(std::move(boundedBody),std::move(loopLabel));
    }

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
         *      gotoIfTrue <condition> continue
         * break:
         * */
        auto condition = this->BindExpression(syntax->_condition.get(), Types::BOOL->Name());
        auto [boundedBody, loopLabel] = this->BindLoopBody(syntax->_body.get());

        auto continueLabel = loopLabel.second;
        auto checkLabel = this->GenerateLabel();
        auto breakLabel = loopLabel.first;

        auto gotoCheck = std::make_unique<BoundGotoStatement>(checkLabel);
        auto continueLabelStatement = std::make_unique<BoundLabelStatement>(continueLabel);
        auto checkLabelStatement = std::make_unique<BoundLabelStatement>(checkLabel);
        auto gotoTrue = std::make_unique<BoundConditionalGotoStatement>(continueLabel, std::move(condition), false);
        auto endLabelStatement = std::make_unique<BoundLabelStatement>(breakLabel);

        std::vector<std::unique_ptr<BoundStatementNode>> statements_1;
        statements_1.emplace_back(std::move(gotoCheck));
        statements_1.emplace_back(std::move(continueLabelStatement));
        statements_1.emplace_back(std::move(boundedBody));
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
         *      gotoIfTrue <condition> continue;
         * break:
         * */

        auto continueLabel = node->_loopLabel.second;
        auto checkLabel = this->GenerateLabel();
        auto breakLabel = node->_loopLabel.first;

        auto gotoCheck = std::make_unique<BoundGotoStatement>(checkLabel);
        auto continueLabelStatement = std::make_unique<BoundLabelStatement>(continueLabel);
        auto checkLabelStatement = std::make_unique<BoundLabelStatement>(checkLabel);
        auto gotoTrue = std::make_unique<BoundConditionalGotoStatement>(continueLabel, std::move(node->_condition), false);
        auto breakLabelStatement = std::make_unique<BoundLabelStatement>(breakLabel);

        std::vector<std::unique_ptr<BoundStatementNode>> statements_1;
        statements_1.emplace_back(std::move(gotoCheck));
        statements_1.emplace_back(std::move(continueLabelStatement));
        statements_1.emplace_back(std::move(node->_body));
        statements_1.emplace_back(std::move(checkLabelStatement));
        statements_1.emplace_back(std::move(gotoTrue));
        statements_1.emplace_back(std::move(breakLabelStatement));

        auto result = std::make_unique<BoundBlockStatement>(std::move(statements_1));

        return result;
    }

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
         *          continue:
         *          <var> = <var> + 1
         *      }
         * }
         *
         * */

        auto lowerBound = this->BindExpression(syntax->_lowerBound.get(), Types::INT->Name());
        auto upperBound = this->BindExpression(syntax->_upperBound.get(), Types::INT->Name());

        _scope = std::make_shared<BoundScope>(_scope);

        auto variable = this->BindVariable(syntax->_identifier->_text, true, Types::INT->Name());

        _turnOnScopingInBlockStatement = false;
        auto [boundedBody, loopLabel] = this->BindLoopBody(syntax->_body.get());
        _turnOnScopingInBlockStatement = true;

        auto upperBoundSymbol = this->BindVariable("upperBound", true, Types::INT->Name());

        _scope = _scope->_parent;

        auto variableDeclaration = std::make_unique<BoundVariableDeclaration>(variable, std::move(lowerBound));
        auto upperBoundVariableDeclaration = std::make_unique<BoundVariableDeclaration>(upperBoundSymbol, std::move(upperBound));

        auto condition = std::make_unique<BoundBinaryExpression>(
                std::make_unique<BoundVariableExpression>(variable),
                BoundBinaryOperator::Bind(SyntaxKind::LessThanEqualsToken, Types::INT->Name(), Types::INT->Name()),
                std::make_unique<BoundVariableExpression>(upperBoundSymbol)
        );
        auto continueLabelStatement = std::make_unique<BoundLabelStatement>(loopLabel.second);
        auto increment = std::make_unique<BoundExpressionStatement>(
                std::make_unique<BoundAssignmentExpression>(
                        variable,
                        std::make_unique<BoundBinaryExpression>(
                                std::make_unique<BoundVariableExpression>(variable),
                                BoundBinaryOperator::Bind(SyntaxKind::PlusToken, Types::INT->Name(),Types::INT->Name()),
                                std::make_unique<BoundLiteralExpression>(1))
                ));

        /** This has to be done instead of doing std::make_unique<BoundBlockStatement>({std::move(body), std::move(increment)}) because BoundBlockStatement is explicit */
        std::vector<std::unique_ptr<BoundStatementNode>> statements_1(3);
        statements_1.emplace_back(std::move(boundedBody));
        statements_1.emplace_back(std::move(continueLabelStatement));
        statements_1.emplace_back(std::move(increment));

        auto whileBody = std::make_unique<BoundBlockStatement>(std::move(statements_1));

        loopLabel.second = LabelSymbol("continue");
        auto whileStatement = std::make_unique<BoundWhileStatement>(std::move(condition), std::move(whileBody), std::move(loopLabel));

        auto loweredWhileStatement = this->BindWhileStatement(whileStatement.get());

        std::vector<std::unique_ptr<BoundStatementNode>> statements_2(3);
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

            _buffer << "Conversion does not exists from " << expression->Type() << " to " << type << "\n";
            return std::make_unique<BoundErrorExpression>();
        }

        if(conversion->_isExplicit && !allowExplicit)
        {
            _buffer << "Cannot convert " << expression->Type() << " to " << type << ". An explicit conversion exists; are you missing a cast ?\n";
            return std::make_unique<BoundErrorExpression>();
        }

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

        auto* type = trylang::LookUpType(syntax->_identifier->_text);
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

        auto function = _scope->TryLookUpFunction(syntax->_identifier->_text);
        if(function == nullptr)
        {
            _buffer << "Function '" << syntax->_identifier->_text << "' doesn't exist\n";
            return std::make_unique<BoundErrorExpression>();
        }

        if(syntax->_arguments.size() != function->_parameters.size())
        {
            _buffer << "Wrong No.of Arguments Reported in function call " << syntax->_identifier->_text << "\n";
            return std::make_unique<BoundErrorExpression>();
        }

        for(auto i = 0; i < syntax->_arguments.size(); i++)
        {
            const auto& argument = boundArguments[i];
            const auto& parameter = function->_parameters[i];

            if(argument->Type() != parameter._type)
            {
                _buffer << "Wrong Argument Type provided in function call " << syntax->_identifier->_text << "\n";
                return std::make_unique<BoundErrorExpression>();
            }
        }

        return std::make_unique<BoundCallExpression>(std::make_unique<FunctionSymbol>(*function), std::move(boundArguments));
    }

    void Binder::BindFunctionDeclaration(FunctionDeclarationStatementSyntax *syntax)
    {
        std::vector<ParameterSymbol> parameters;
        std::vector<std::string> seenParameterNames(syntax->_parameters.size());

        for(const auto& parameterSyntax: syntax->_parameters)
        {
            const auto& parameterName = parameterSyntax->_identifier->_text;
            auto parameterType = this->BindTypeClause(parameterSyntax->_type.get());

            if(std::find(seenParameterNames.begin(), seenParameterNames.end(), parameterName) != seenParameterNames.end())
            {
                _buffer << "Parameter '" <<  parameterName <<"' already declared\n";
            }
            else
            {
                ParameterSymbol parameter(parameterName, true ,parameterType);
                parameters.emplace_back(std::move(parameter));
            }

        }

        auto returnType = this->BindTypeClause(syntax->_typeClause.get());
        if(returnType == nullptr)
        {
            returnType = Types::VOID->Name();
        }

        if(std::strcmp(returnType, Types::VOID->Name()) != 0)
        {
            /* we have a non void returnType */
            _buffer << "Functions with return values are not supported\n";
        }

        auto function = std::make_shared<FunctionSymbol>(syntax->_identifier->_text, std::move(parameters), returnType, syntax);
        if(!_scope->TryDeclareFunction(function))
        {
            _buffer << "Function '" << syntax->_identifier->_text << "' already declared\n";
        }
    }

    std::unique_ptr<BoundStatementNode> Binder::BindBreakStatement(BreakStatementSyntax *syntax)
    {
        if(_loopStack.empty())
        {
            _buffer << "The keyword break can only be used inside loops.\n";
            return this->BindErrorStatement();
        }

        auto breakLabel = _loopStack.top().first;
        return std::make_unique<BoundGotoStatement>(breakLabel);
    }

    std::unique_ptr<BoundStatementNode> Binder::BindContinueStatement(ContinueStatementSyntax *syntax)
    {
        if(_loopStack.empty())
        {
            _buffer << "The keyword continue can only be used inside loops.\n";
            return this->BindErrorStatement();
        }

        auto continueLabel = _loopStack.top().second;
        return std::make_unique<BoundGotoStatement>(continueLabel);
    }

    std::unique_ptr<BoundStatementNode> Binder::BindErrorStatement()
    {
        return std::make_unique<BoundExpressionStatement>(std::make_unique<BoundErrorExpression>());
    }


}