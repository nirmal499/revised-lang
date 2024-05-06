#pragma once
#include <unordered_map>
#include <string>

namespace trylang
{
    enum class BoundNodeKind
    {
        LiteralExpression,
        BinaryExpression,
        UnaryExpression,
        ParenthesizedExpression,
    };

    inline std::unordered_map<BoundNodeKind, std::string> __boundNodeStringMap = 
    {
        {BoundNodeKind::LiteralExpression, "LiteralExpression"},
        {BoundNodeKind::BinaryExpression, "BinaryExpression"},
        {BoundNodeKind::UnaryExpression, "UnaryExpression"},
        {BoundNodeKind::ParenthesizedExpression, "ParenthesizedExpression"},

    };

    enum class BoundUnaryOperatorKind
    {
        Identity,
        Negation
    };

    inline std::unordered_map<BoundUnaryOperatorKind, std::string> __boundUnaryOperatorKindStringMap = 
    {
        {BoundUnaryOperatorKind::Identity, "Identity"},
        {BoundUnaryOperatorKind::Negation, "Negation"},

    };

    enum class BoundBinaryOperatorKind
    {
        Addition,
        Subtraction,
        Multiplication,
        Division
    };

    inline std::unordered_map<BoundBinaryOperatorKind, std::string> __boundBinaryOperatorKindStringMap =
    {
        {BoundBinaryOperatorKind::Addition, "Addition"},
        {BoundBinaryOperatorKind::Subtraction, "Subtraction"},
        {BoundBinaryOperatorKind::Multiplication, "Multiplication"},
        {BoundBinaryOperatorKind::Division, "Division"},

    };

    inline std::ostream& operator<<(std::ostream& out, BoundNodeKind kind)
    {
        return out << __boundNodeStringMap[kind];
    }
}