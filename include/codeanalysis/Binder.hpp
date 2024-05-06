#pragma once

#include <memory>
#include <sstream>
#include <optional>
#include <codeanalysis/BoundExpressionNode.hpp>
#include <codeanalysis/SyntaxKind.hpp>

namespace trylang
{
    struct ExpressionSyntax;
    struct LiteralExpressionSyntax;
    struct UnaryExpressionSyntax;
    struct BinaryExpressionSyntax;

    struct BoundExpressionNode;

    struct Binder
    {
        std::stringstream buffer;

        std::string Errors();

        std::unique_ptr<BoundExpressionNode> BindExpression(ExpressionSyntax* syntax);

        std::unique_ptr<BoundExpressionNode> BindLiteralExpression(LiteralExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindUnaryExpression(UnaryExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindBinaryExpression(BinaryExpressionSyntax* syntax);

        std::optional<BoundUnaryOperatorKind> BindUnaryOperatorKind(SyntaxKind kind, const std::type_info& operandType);
        std::optional<BoundBinaryOperatorKind> BindBinaryOperatorKind(SyntaxKind kind, const std::type_info& leftType, const std::type_info& rightType);
    };
}