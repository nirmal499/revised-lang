#pragma once

#include <memory>

namespace trylang
{
    struct BoundExpressionNode;

    struct Evaluator
    {
        std::unique_ptr<BoundExpressionNode> _root;

        explicit Evaluator(std::unique_ptr<BoundExpressionNode> root);

        int Evaluate();

        int EvaluateExpression(BoundExpressionNode* node);
    };
}