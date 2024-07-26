#pragma once

#include "codeanalysis/Symbol.hpp"
#include <memory>

namespace trylang
{   
    struct IfStatementSyntax;
    struct WhileStatementSyntax;
    struct ForStatementSyntax;

    struct BoundStatementNode;
    struct BoundBlockStatement;
    struct BoundBoundStatement;
    struct BoundVariableDeclaration;
    struct BoundLabelStatement;
    struct BoundGotoStatement;
    struct BoundConditionalGotoStatement;
    struct BoundIfStatement;
    struct BoundWhileStatement;
    struct BoundForStatement;
    struct BoundExpressionStatement;

    struct BoundExpressionNode;
    struct BoundLiteralExpression;
    struct BoundVariableExpression;
    struct BoundAssignmentExpression;
    struct BoundCallExpression;
    struct BoundConversionExpression;
    struct BoundUnaryExpression;
    struct BoundBinaryExpression;

    struct Lower
    {
        int _labelCountForIfStatement = 0;

        LabelSymbol GenerateLabel();
        std::unique_ptr<BoundBlockStatement> Flatten(std::unique_ptr<BoundStatementNode> statement);

        std::unique_ptr<BoundStatementNode> RewriteStatement(std::unique_ptr<BoundStatementNode> node);

        /* All the below functions are created according to the ones describe in BoundNodeKind */
        std::unique_ptr<BoundStatementNode> RewriteBlockStatement(std::unique_ptr<BoundStatementNode> node);
        std::unique_ptr<BoundStatementNode> RewriteVariableDeclaration(std::unique_ptr<BoundStatementNode> node);
        std::unique_ptr<BoundStatementNode> RewriteIfStatement(std::unique_ptr<BoundStatementNode> node);
        std::unique_ptr<BoundStatementNode> RewriteWhileStatement(std::unique_ptr<BoundStatementNode> node);
        // std::unique_ptr<BoundStatementNode> RewriteForStatement(std::unique_ptr<BoundStatementNode> node);
        std::unique_ptr<BoundStatementNode> RewriteLabelStatement(std::unique_ptr<BoundStatementNode> node);
        std::unique_ptr<BoundStatementNode> RewriteGotoStatement(std::unique_ptr<BoundStatementNode> node);
        std::unique_ptr<BoundStatementNode> RewriteConditionalGotoStatement(std::unique_ptr<BoundStatementNode> node);
        std::unique_ptr<BoundStatementNode> RewriteReturnStatement(std::unique_ptr<BoundStatementNode> node);
        std::unique_ptr<BoundStatementNode> RewriteExpressionStatement(std::unique_ptr<BoundStatementNode> node);

        std::unique_ptr<BoundExpressionNode> RewriteExpression(std::unique_ptr<BoundExpressionNode> node);
        
        /* All the below functions are created according to the ones describe in BoundNodeKind */
        std::unique_ptr<BoundExpressionNode> RewriteErrorExpression(std::unique_ptr<BoundExpressionNode> node);
        std::unique_ptr<BoundExpressionNode> RewriteLiteralExpression(std::unique_ptr<BoundExpressionNode> node);
        std::unique_ptr<BoundExpressionNode> RewriteVariableExpression(std::unique_ptr<BoundExpressionNode> node);
        std::unique_ptr<BoundExpressionNode> RewriteAssignmentExpression(std::unique_ptr<BoundExpressionNode> node);
        std::unique_ptr<BoundExpressionNode> RewriteCallExpression(std::unique_ptr<BoundExpressionNode> node);
        std::unique_ptr<BoundExpressionNode> RewriteConversionExpression(std::unique_ptr<BoundExpressionNode> node);
        std::unique_ptr<BoundExpressionNode> RewriteUnaryExpression(std::unique_ptr<BoundExpressionNode> node);
        std::unique_ptr<BoundExpressionNode> RewriteBinaryExpression(std::unique_ptr<BoundExpressionNode> node);

        static std::unique_ptr<BoundBlockStatement> RewriteAndFlatten(std::unique_ptr<BoundStatementNode> statement);
    };   
}