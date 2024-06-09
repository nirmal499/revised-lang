#pragma once

#include <codeanalysis/BoundExpressionNode.hpp>
#include <memory>
#include <codeanalysis/Types.hpp>
#include <unordered_map>
#include <codeanalysis/Symbol.hpp>
#include <stack>

namespace trylang
{
    struct BoundExpressionNode;
    struct BoundStatementNode;

    struct BoundProgram;

    struct BoundBlockStatement;
    struct BoundExpressionStatement;
    struct BoundVariableDeclaration;
    struct BoundIfStatement;
    struct BoundWhileStatement;
    struct BoundForStatement;

    struct BoundLiteralExpression;
    struct BoundVariableExpression;
    struct BoundAssignmentExpression;
    struct BoundUnaryExpression;
    struct BoundCallExpression;
    struct BoundConversionExpression;
    struct BoundBinaryExpression;

    struct Evaluator
    {

        variable_map_t& _globals;
        std::stack<variable_map_t> _locals;

        std::unique_ptr<BoundProgram> _program;

        object_t _lastValue;

        Evaluator(std::unique_ptr<BoundProgram> program,variable_map_t& variables);

        object_t Evaluate();
        object_t EvaluateStatement(BoundBlockStatement* body);

        object_t EvaluateExpression(BoundExpressionNode* node);
        object_t EvaluateLiteralExpression(BoundLiteralExpression* node);
        object_t EvaluateVariableExpression(BoundVariableExpression *node);
        object_t EvaluateAssignmentExpression(BoundAssignmentExpression* node);
        object_t EvaluateUnaryExpression(BoundUnaryExpression* node);
        object_t EvaluateCallExpression(BoundCallExpression* node);
        object_t EvaluateConversionExpression(BoundConversionExpression* node);
        object_t EvaluateBinaryExpression(BoundBinaryExpression* node);


        // void EvaluateStatement(BoundStatementNode* node);
        // void EvaluateBlockStatement(BoundBlockStatement *node);

        void EvaluateExpressionStatement(BoundExpressionStatement* node);
        void EvaluateVariableDeclaration(BoundVariableDeclaration *node);

        // void EvaluateIfStatement(BoundIfStatement *node);
        // void EvaluateWhileStatement(BoundWhileStatement *node);
        // void EvaluateForStatement(BoundForStatement *node);
    };
}