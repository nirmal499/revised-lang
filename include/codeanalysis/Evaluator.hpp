#pragma once

#include <memory>
#include <codeanalysis/Types.hpp>
#include <unordered_map>
#include <codeanalysis/VariableSymbol.hpp>

namespace trylang
{
    struct BoundExpressionNode;

    struct Evaluator
    {

        variable_map_t& _variable_map;
        std::unique_ptr<BoundExpressionNode> _root;

        explicit Evaluator(std::unique_ptr<BoundExpressionNode> root, variable_map_t& variables);
        oobject_t Evaluate();
        oobject_t EvaluateExpression(BoundExpressionNode* node);
    };
}