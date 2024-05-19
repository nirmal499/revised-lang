#pragma once

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
//        void EvaluateStatement(BoundStatementNode* node);

//        void EvaluateBlockStatement(BoundBlockStatement *node);
        void EvaluateExpressionStatement(BoundExpressionStatement* node);
        void EvaluateVariableDeclaration(BoundVariableDeclaration *node);

//        void EvaluateIfStatement(BoundIfStatement *node);
//        void EvaluateWhileStatement(BoundWhileStatement *node);
//        void EvaluateForStatement(BoundForStatement *node);
    };
}