#pragma once
#include <unordered_map>
#include <string>
#include <codeanalysis/SyntaxKind.hpp>
#include <typeinfo>
#include <memory>
#include <cstring>
#include <array>
#include <codeanalysis/Types.hpp>
#include <stdexcept>

namespace trylang
{
    enum class BoundNodeKind
    {
        LiteralExpression,
        BinaryExpression,
        UnaryExpression,
        ParenthesizedExpression, /* No use */
        VariableExpression,
        AssignmentExpression,

        BlockStatement,
        ExpressionStatement,
        VariableDeclarationStatement,
        IfStatement,
        WhileStatement,
        ForStatement,

        Identity,
        Negation,
        LogicalNegation,

        Addition,
        Subtraction,
        Multiplication,
        Division,
        LogicalOr,
        LogicalAnd,
        LogicalEquality,
        LogicalNotEquality,
        Less,
        Greater,
        LessEquals,
        GreaterEquals
    };

    inline std::unordered_map<BoundNodeKind, std::string> __boundNodeStringMap = 
    {
        {BoundNodeKind::LiteralExpression, "LiteralExpression"},
        {BoundNodeKind::BinaryExpression, "BinaryExpression"},
        {BoundNodeKind::UnaryExpression, "UnaryExpression"},
        {BoundNodeKind::ParenthesizedExpression, "ParenthesizedExpression"},
        {BoundNodeKind::VariableExpression, "VariableExpression"},
        {BoundNodeKind::AssignmentExpression, "AssignmentExpression"},

        {BoundNodeKind::BlockStatement, "BlockStatement"},
        {BoundNodeKind::ExpressionStatement, "ExpressionStatement"},
        {BoundNodeKind::VariableDeclarationStatement, "VariableDeclarationStatement"},
        {BoundNodeKind::IfStatement, "IfStatement"},
        {BoundNodeKind::WhileStatement, "WhileStatement"},

        {BoundNodeKind::Identity, "Identity"},
        {BoundNodeKind::Negation, "Negation"},
        {BoundNodeKind::LogicalNegation, "LogicalNegation"},

        {BoundNodeKind::Addition, "Addition"},
        {BoundNodeKind::Subtraction, "Subtraction"},
        {BoundNodeKind::Multiplication, "Multiplication"},
        {BoundNodeKind::Division, "Division"},
        {BoundNodeKind::LogicalAnd, "LogicalAnd"},
        {BoundNodeKind::LogicalOr, "LogicalOr"},
        {BoundNodeKind::LogicalEquality, "LogicalEquality"},
        {BoundNodeKind::LogicalNotEquality, "LogicalNotEquality"},
        {BoundNodeKind::Less, "Less"},
        {BoundNodeKind::Greater, "Greater"},
        {BoundNodeKind::LessEquals, "LessEquals"},
        {BoundNodeKind::GreaterEquals, "GreaterEquals"},

    };

    inline std::ostream& operator<<(std::ostream& out, BoundNodeKind kind)
    {
        return out << __boundNodeStringMap[kind];
    }

    inline auto assign_type_info = [](const object_t& data) -> const char *
    {
        auto index_in_variant = data->index();
        if(index_in_variant == 0)
        {
            return typeid(int).name();
        }

        if(index_in_variant == 1)
        {
            return typeid(bool).name();
        }

        /* index_in_variant == std::variant_npos */
        throw std::logic_error("Unexpected type_info");

        return typeid(int).name(); /* Unreachable */
    };
}