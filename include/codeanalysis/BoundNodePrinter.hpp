#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace trylang
{
    struct BoundNode;
    struct BoundLiteralExpression;
    struct BoundBinaryExpression;
    struct BoundUnaryExpression;
    struct BoundVariableExpression;
    struct BoundAssignmentExpression;
    struct BoundErrorExpression;
    struct BoundCallExpression;
    struct BoundConversionExpression;
    struct BoundBlockStatement;
    struct BoundExpressionStatement;
    struct BoundVariableDeclaration; /* BoundVariableDeclarationStatement */
    struct BoundIfStatement;
    struct BoundWhileStatement;
    struct BoundForStatement;
    struct BoundGotoStatement;
    struct BoundConditionalGotoStatement;
    struct BoundLabelStatement;

    struct BoundStatementNode;

    struct NodePrinter
    {
        static std::stringstream _buffer;
        static std::vector<std::string> _indentation;
        void WriteTo(BoundNode* node);
        static void Write(trylang::BoundNode *node);
        // void WriteNestedStatement(BoundStatementNode* node);

        void WriteLiteralExpression(BoundLiteralExpression* node);
        void WriteBinaryExpression(BoundBinaryExpression* node);
        void WriteUnaryExpression(BoundUnaryExpression* node);
        void WriteVariableExpression(BoundVariableExpression* node);
        void WriteAssignmentExpression(BoundAssignmentExpression* node);
        void WriteErrorExpression(BoundErrorExpression* node);
        void WriteCallExpression(BoundCallExpression* node);
        void WriteConversionExpression(BoundConversionExpression* node);
        void WriteBlockStatement(BoundBlockStatement* node);
        void WriteExpressionStatement(BoundExpressionStatement* node);
        void WriteVariableDeclarationStatement(BoundVariableDeclaration* node);
        // void WriteIfStatement(BoundIfStatement* node);
        // void WriteWhileStatement(BoundWhileStatement* node);
        // void WriteForStatement(BoundForStatement* node);
        void WriteGotoStatement(BoundGotoStatement* node);
        void WriteConditionalGotoStatement(BoundConditionalGotoStatement* node);
        void WriteLabelStatement(BoundLabelStatement* node);
    };


}