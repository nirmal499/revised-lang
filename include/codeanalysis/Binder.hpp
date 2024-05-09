#pragma once

#include <memory>
#include <sstream>
#include <optional>
#include <codeanalysis/BoundExpressionNode.hpp>
#include <codeanalysis/SyntaxKind.hpp>
#include <codeanalysis/VariableSymbol.hpp>

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
        variable_map_t& _variable_map;

        explicit Binder(variable_map_t& variables);

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