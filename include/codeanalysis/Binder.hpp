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
    struct ParenthesizedExpressionSyntax;
    struct NameExpressionSyntax;
    struct AssignmentExpressionSyntax;

    struct BoundExpressionNode;

    struct Binder
    {
        std::unordered_map<std::string, oobject_t>& _variables;

        explicit Binder(std::unordered_map<std::string, oobject_t>& variables);

        std::stringstream _buffer;

        std::string Errors();

        std::unique_ptr<BoundExpressionNode> BindExpression(ExpressionSyntax* syntax);

        std::unique_ptr<BoundExpressionNode> BindParenthesizedExpression(ParenthesizedExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindNameExpression(NameExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindAssignmentExpression(AssignmentExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindLiteralExpression(LiteralExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindUnaryExpression(UnaryExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindBinaryExpression(BinaryExpressionSyntax* syntax);
    };
}