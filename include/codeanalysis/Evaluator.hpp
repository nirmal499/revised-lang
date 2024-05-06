#pragma once

#include <memory>
#include <codeanalysis/Types.hpp>

namespace trylang
{
    struct BoundExpressionNode;

    struct Evaluator
    {
        std::unique_ptr<BoundExpressionNode> _root;

        explicit Evaluator(std::unique_ptr<BoundExpressionNode> root);

        oobject_t Evaluate();

        oobject_t EvaluateExpression(BoundExpressionNode* node);
    };
}