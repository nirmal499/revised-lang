#pragma once

#include <memory>

namespace trylang
{
    struct ExpressionSyntax;

    struct Evaluator
    {
        std::unique_ptr<ExpressionSyntax> _root;

        Evaluator(std::unique_ptr<ExpressionSyntax> root);

        int Evaluate();

        int EvaluateExpression(ExpressionSyntax* node);
    };
}