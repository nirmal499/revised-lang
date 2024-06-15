#include <codeanalysis/BoundExpressionNode.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/Lower.hpp>
#include <memory>
#include <stack>
#include <random>

namespace trylang
{

    LabelSymbol Lower::GenerateLabel()
    {
        auto name = "Label{" + std::to_string(++_labelCountForIfStatement) + "}";
        LabelSymbol label(name);
        return label;
    }

    /* All the BoundBlockStatement will be removed and flattened into their BoundStatementNode */
    std::unique_ptr<BoundBlockStatement> Lower::Flatten(std::unique_ptr<BoundStatementNode> statement)
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

    std::unique_ptr<BoundStatementNode> Lower::RewriteStatement(std::unique_ptr<BoundStatementNode> node)
    {
        if(node == nullptr)
        {
            return nullptr;
        }
        
        switch (node->Kind())
        {
            case BoundNodeKind::BlockStatement:
                return RewriteBlockStatement(std::move(node));
            case BoundNodeKind::VariableDeclarationStatement:
                return RewriteVariableDeclaration(std::move(node));
            case BoundNodeKind::IfStatement:
                return RewriteIfStatement(std::move(node));
            case BoundNodeKind::WhileStatement:
                return RewriteWhileStatement(std::move(node));
            case BoundNodeKind::ForStatement:
                return RewriteForStatement(std::move(node));
            case BoundNodeKind::LabelStatement:
                return RewriteLabelStatement(std::move(node));
            case BoundNodeKind::GotoStatement:
                return RewriteGotoStatement(std::move(node));
            case BoundNodeKind::ConditionalGotoStatement:
                return RewriteConditionalGotoStatement(std::move(node));
            case BoundNodeKind::ExpressionStatement:
                return RewriteExpressionStatement(std::move(node));
            case BoundNodeKind::ReturnStatement:
                return RewriteReturnStatement(std::move(node));
            default:
                throw std::logic_error("Unexpected syntax " + __boundNodeStringMap[node->Kind()]);
        }
    }

    std::unique_ptr<BoundStatementNode> Lower::RewriteBlockStatement(std::unique_ptr<BoundStatementNode> node)
    {
        auto* stmt = static_cast<BoundBlockStatement*>(node.get());

        std::vector<std::unique_ptr<BoundStatementNode>> statements(stmt->_statements.size() + 2);

        LabelSymbol blockStartLabel("StartBlockLabel");
        statements.emplace_back(std::make_unique<BoundLabelStatement>(blockStartLabel));

        for(int i = 0; i < stmt->_statements.size(); i++)
        {
            statements.emplace_back(this->RewriteStatement(std::move(stmt->_statements.at(i))));
        }

        LabelSymbol blockEndLabel("EndBlockLabel");
        statements.emplace_back(std::make_unique<BoundLabelStatement>(blockEndLabel));

        return std::make_unique<BoundBlockStatement>(std::move(statements));
    }

    std::unique_ptr<BoundStatementNode> Lower::RewriteVariableDeclaration(std::unique_ptr<BoundStatementNode> node)
    {
        auto* stmt = static_cast<BoundVariableDeclaration*>(node.get());
        return node;
    }

    std::unique_ptr<BoundStatementNode> Lower::RewriteIfStatement(std::unique_ptr<BoundStatementNode> node)
    {
        auto* stmt = static_cast<BoundIfStatement*>(node.get());
        
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

        if(stmt->_elseStatement == nullptr)
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
            auto gotoFalse = std::make_unique<BoundConditionalGotoStatement>(endLabel, std::move(stmt->_condition), true);
            auto endLabelStatement = std::make_unique<BoundLabelStatement>(endLabel);

            std::vector<std::unique_ptr<BoundStatementNode>> statements_1;
            statements_1.emplace_back(std::move(gotoFalse));
            statements_1.emplace_back(std::move(stmt->_statement));
            statements_1.emplace_back(std::move(endLabelStatement));

            auto result = std::make_unique<BoundBlockStatement>(std::move(statements_1));

            return this->RewriteStatement(std::move(result));
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
            auto gotoFalse = std::make_unique<BoundConditionalGotoStatement>(elseLabel, std::move(stmt->_condition), true);
            auto gotoEndStatement = std::make_unique<BoundGotoStatement>(endLabel);
            auto endLabelStatement = std::make_unique<BoundLabelStatement>(endLabel);
            auto elseLabelStatement = std::make_unique<BoundLabelStatement>(elseLabel);

            std::vector<std::unique_ptr<BoundStatementNode>> statements_1;
            statements_1.emplace_back(std::move(gotoFalse));
            statements_1.emplace_back(std::move(stmt->_statement));
            statements_1.emplace_back(std::move(gotoEndStatement));
            statements_1.emplace_back(std::move(elseLabelStatement));
            statements_1.emplace_back(std::move(stmt->_elseStatement));
            statements_1.emplace_back(std::move(endLabelStatement));

            auto result = std::make_unique<BoundBlockStatement>(std::move(statements_1));

            return this->RewriteStatement(std::move(result));
        }
    
    }

    std::unique_ptr<BoundStatementNode> Lower::RewriteWhileStatement(std::unique_ptr<BoundStatementNode> node)
    {
        auto* stmt = static_cast<BoundWhileStatement*>(node.get());

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

        auto continueLabel = stmt->_loopLabel.second;
        auto checkLabel = this->GenerateLabel();
        auto breakLabel = stmt->_loopLabel.first;

        auto gotoCheck = std::make_unique<BoundGotoStatement>(checkLabel);
        auto continueLabelStatement = std::make_unique<BoundLabelStatement>(continueLabel);
        auto checkLabelStatement = std::make_unique<BoundLabelStatement>(checkLabel);
        auto gotoTrue = std::make_unique<BoundConditionalGotoStatement>(continueLabel, std::move(stmt->_condition), false);
        auto breakLabelStatement = std::make_unique<BoundLabelStatement>(breakLabel);

        std::vector<std::unique_ptr<BoundStatementNode>> statements_1;
        statements_1.emplace_back(std::move(gotoCheck));
        statements_1.emplace_back(std::move(continueLabelStatement));
        if(stmt->_body->Kind() == BoundNodeKind::BlockStatement)
        {
            auto* blockStmt = static_cast<BoundBlockStatement*>(stmt->_body.get());

            for(int i = 0; i < blockStmt->_statements.size(); i++)
            {
                statements_1.emplace_back(std::move(blockStmt->_statements.at(i)));
            }
        }
        else
        {
            statements_1.emplace_back(std::move(stmt->_body));
        }
        statements_1.emplace_back(std::move(checkLabelStatement));
        statements_1.emplace_back(std::move(gotoTrue));
        statements_1.emplace_back(std::move(breakLabelStatement));

        auto result = std::make_unique<BoundBlockStatement>(std::move(statements_1));

        return this->RewriteStatement(std::move(result));
    }

    std::unique_ptr<BoundStatementNode> Lower::RewriteForStatement(std::unique_ptr<BoundStatementNode> node)
    {
        auto* stmt = static_cast<BoundForStatement*>(node.get());

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

        auto variableDeclaration = std::make_unique<BoundVariableDeclaration>(stmt->_variable, std::move(stmt->_lowerBound));
        auto upperBoundVariableDeclaration = std::make_unique<BoundVariableDeclaration>(stmt->_variableForUpperBoundToBeUsedDuringRewritingForIntoWhile, std::move(stmt->_upperBound));

        auto condition = std::make_unique<BoundBinaryExpression>(
                std::make_unique<BoundVariableExpression>(stmt->_variable),
                BoundBinaryOperator::Bind(SyntaxKind::LessThanEqualsToken, Types::INT->Name(), Types::INT->Name()),
                std::make_unique<BoundVariableExpression>(stmt->_variableForUpperBoundToBeUsedDuringRewritingForIntoWhile)
        );
        auto continueLabelStatement = std::make_unique<BoundLabelStatement>(stmt->_loopLabel.second);
        auto increment = std::make_unique<BoundExpressionStatement>(
                std::make_unique<BoundAssignmentExpression>(
                        stmt->_variable,
                        std::make_unique<BoundBinaryExpression>(
                                std::make_unique<BoundVariableExpression>(stmt->_variable),
                                BoundBinaryOperator::Bind(SyntaxKind::PlusToken, Types::INT->Name(),Types::INT->Name()),
                                std::make_unique<BoundLiteralExpression>(1))
                ));

        /** This has to be done instead of doing std::make_unique<BoundBlockStatement>({std::move(body), std::move(increment)}) because BoundBlockStatement is explicit */
        std::vector<std::unique_ptr<BoundStatementNode>> statements_1(3);
        statements_1.emplace_back(std::move(stmt->_body));
        statements_1.emplace_back(std::move(continueLabelStatement));
        statements_1.emplace_back(std::move(increment));

        auto whileBody = std::make_unique<BoundBlockStatement>(std::move(statements_1));

        stmt->_loopLabel.second = LabelSymbol("continue{" + GenerateRandomText(3) + "}");
        auto whileStatement = std::make_unique<BoundWhileStatement>(std::move(condition), std::move(whileBody), std::move(stmt->_loopLabel));

        std::vector<std::unique_ptr<BoundStatementNode>> statements_2(3);
        statements_2.emplace_back(std::move(variableDeclaration));
        statements_2.emplace_back(std::move(upperBoundVariableDeclaration));
        statements_2.emplace_back(std::move(whileStatement));

        auto result = std::make_unique<BoundBlockStatement>(std::move(statements_2));

        return this->RewriteStatement(std::move(result));
    }

    std::unique_ptr<BoundStatementNode> Lower::RewriteLabelStatement(std::unique_ptr<BoundStatementNode> node)
    {
        auto* stmt = static_cast<BoundLabelStatement*>(node.get());
        return node;
    }

    std::unique_ptr<BoundStatementNode> Lower::RewriteGotoStatement(std::unique_ptr<BoundStatementNode> node)
    {
        auto* stmt = static_cast<BoundGotoStatement*>(node.get());
        return node;
    }

    std::unique_ptr<BoundStatementNode> Lower::RewriteConditionalGotoStatement(std::unique_ptr<BoundStatementNode> node)
    {
        auto* stmt = static_cast<BoundConditionalGotoStatement*>(node.get());
        return node;
    }

    std::unique_ptr<BoundStatementNode> Lower::RewriteReturnStatement(std::unique_ptr<BoundStatementNode> node)
    {
        auto* stmt = static_cast<BoundReturnStatement*>(node.get());
        return node;
    }

    std::unique_ptr<BoundStatementNode> Lower::RewriteExpressionStatement(std::unique_ptr<BoundStatementNode> node)
    {
        auto* stmt = static_cast<BoundExpressionStatement*>(node.get());
        return node;
    }

    std::unique_ptr<BoundExpressionNode> Lower::RewriteExpression(std::unique_ptr<BoundExpressionNode> node)
    {
        switch (node->Kind())
        {
            case BoundNodeKind::ErrorExpression:
                return RewriteErrorExpression(std::move(node));
            case BoundNodeKind::LiteralExpression:
                return RewriteLiteralExpression(std::move(node));
            case BoundNodeKind::VariableExpression:
                return RewriteVariableExpression(std::move(node));
            case BoundNodeKind::AssignmentExpression:
                return RewriteAssignmentExpression(std::move(node));
            case BoundNodeKind::UnaryExpression:
                return RewriteUnaryExpression(std::move(node));
            case BoundNodeKind::BinaryExpression:
                return RewriteBinaryExpression(std::move(node));
            case BoundNodeKind::CallExpression:
                return RewriteCallExpression(std::move(node));
            case BoundNodeKind::ConversionExpression:
                return RewriteConversionExpression(std::move(node));
            default:
                throw std::logic_error("Unexpected syntax " + __boundNodeStringMap[node->Kind()]);

        }
    }

    std::unique_ptr<BoundExpressionNode> Lower::RewriteErrorExpression(std::unique_ptr<BoundExpressionNode> node)
    {
        auto* expr = static_cast<BoundErrorExpression*>(node.get());
        return node;
    }

    std::unique_ptr<BoundExpressionNode> Lower::RewriteLiteralExpression(std::unique_ptr<BoundExpressionNode> node)
    {
        auto* expr = static_cast<BoundLiteralExpression*>(node.get());
        return node;
    }

    std::unique_ptr<BoundExpressionNode> Lower::RewriteVariableExpression(std::unique_ptr<BoundExpressionNode> node)
    {
        auto* expr = static_cast<BoundVariableExpression*>(node.get());
        return node;
    }

    std::unique_ptr<BoundExpressionNode> Lower::RewriteAssignmentExpression(std::unique_ptr<BoundExpressionNode> node)
    {
        auto* expr = static_cast<BoundAssignmentExpression*>(node.get());
        return node;   
    }

    std::unique_ptr<BoundExpressionNode> Lower::RewriteCallExpression(std::unique_ptr<BoundExpressionNode> node)
    {
        auto* expr = static_cast<BoundCallExpression*>(node.get());
        return node;
    }

    std::unique_ptr<BoundExpressionNode> Lower::RewriteConversionExpression(std::unique_ptr<BoundExpressionNode> node)
    {
        auto* expr = static_cast<BoundConversionExpression*>(node.get());
        return node;
    }

    std::unique_ptr<BoundExpressionNode> Lower::RewriteUnaryExpression(std::unique_ptr<BoundExpressionNode> node)
    {
        auto* expr = static_cast<BoundUnaryExpression*>(node.get());
        return node;
    }

    std::unique_ptr<BoundExpressionNode> Lower::RewriteBinaryExpression(std::unique_ptr<BoundExpressionNode> node)
    {
        auto* expr = static_cast<BoundBinaryExpression*>(node.get());
        return node;
    }

    std::unique_ptr<BoundBlockStatement> Lower::RewriteAndFlatten(std::unique_ptr<BoundStatementNode> statement)
    {
        Lower lower;
        
        auto loweredStmt = lower.RewriteStatement(std::move(statement));
        auto flattendStmt = lower.Flatten(std::move(loweredStmt));

        return flattendStmt;
    }

}