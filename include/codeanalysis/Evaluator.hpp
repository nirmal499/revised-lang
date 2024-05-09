#pragma once

#include <memory>
#include <codeanalysis/Types.hpp>
#include <unordered_map>

namespace trylang
{
    struct BoundExpressionNode;

    struct Evaluator
    {
        std::unordered_map<std::string, oobject_t>& _variables;
        std::unique_ptr<BoundExpressionNode> _root;

        explicit Evaluator(std::unique_ptr<BoundExpressionNode> root, std::unordered_map<std::string, oobject_t>& variables);
        oobject_t Evaluate();
        oobject_t EvaluateExpression(BoundExpressionNode* node);
    };
}