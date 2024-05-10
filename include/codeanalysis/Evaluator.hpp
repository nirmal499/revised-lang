#pragma once

#include <memory>
#include <codeanalysis/Types.hpp>
#include <unordered_map>
#include <codeanalysis/VariableSymbol.hpp>

namespace trylang
{
    struct BoundExpressionNode;
    struct BoundStatementNode;

    struct BoundBlockStatement;
    struct BoundExpressionStatement;

    struct Evaluator
    {

        variable_map_t& _variable_map;
        std::unique_ptr<BoundStatementNode> _root;

        oobject_t _lastValue;

        explicit Evaluator(std::unique_ptr<BoundStatementNode> root, variable_map_t& variables);
        oobject_t Evaluate();

        oobject_t EvaluateExpression(BoundExpressionNode* node);
        void EvaluateStatement(BoundStatementNode* node);

        void EvaluateBlockStatement(BoundBlockStatement *node);
        void EvaluateExpressionStatement(BoundExpressionStatement* node);
    };
}