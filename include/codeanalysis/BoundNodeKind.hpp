#pragma once
#include <unordered_map>
#include <string>
#include <codeanalysis/SyntaxKind.hpp>
#include <typeinfo>
#include <memory>
#include <cstring>
#include <array>

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
        Negation,
        LogicalNegation
    };

    inline std::unordered_map<BoundUnaryOperatorKind, std::string> __boundUnaryOperatorKindStringMap = 
    {
        {BoundUnaryOperatorKind::Identity, "Identity"},
        {BoundUnaryOperatorKind::Negation, "Negation"},
        {BoundUnaryOperatorKind::LogicalNegation, "LogicalNegation"},

    };

    enum class BoundBinaryOperatorKind
    {
        Addition,
        Subtraction,
        Multiplication,
        Division,
        LogicalOr,
        LogicalAnd,
        LogicalEquality,
        LogicalNotEquality,
    };

    inline std::unordered_map<BoundBinaryOperatorKind, std::string> __boundBinaryOperatorKindStringMap =
    {
        {BoundBinaryOperatorKind::Addition, "Addition"},
        {BoundBinaryOperatorKind::Subtraction, "Subtraction"},
        {BoundBinaryOperatorKind::Multiplication, "Multiplication"},
        {BoundBinaryOperatorKind::Division, "Division"},
        {BoundBinaryOperatorKind::LogicalAnd, "LogicalAnd"},
        {BoundBinaryOperatorKind::LogicalOr, "LogicalOr"},
        {BoundBinaryOperatorKind::LogicalEquality, "LogicalEquality"},
        {BoundBinaryOperatorKind::LogicalNotEquality, "LogicalNotEquality"},

    };

    inline std::ostream& operator<<(std::ostream& out, BoundNodeKind kind)
    {
        return out << __boundNodeStringMap[kind];
    }

    struct BoundUnaryOperator
    {
        SyntaxKind _syntaxKind;
        BoundUnaryOperatorKind _kind;
        const std::type_info& _operandType;
        const std::type_info& _resultType;

        BoundUnaryOperator(SyntaxKind syntaxKind, BoundUnaryOperatorKind kind, const std::type_info& operandType, const std::type_info& resultType)
            : _syntaxKind(syntaxKind), _kind(kind), _operandType(operandType), _resultType(resultType)
        {}

        static BoundUnaryOperator* Bind(SyntaxKind syntaxKind, const std::type_info& operandType);

    };

    inline std::array<std::unique_ptr<BoundUnaryOperator>, 3> _boundUnaryOperatorArray = {
        std::make_unique<BoundUnaryOperator>(SyntaxKind::BangToken, BoundUnaryOperatorKind::LogicalNegation, typeid(bool),typeid(bool)),
        std::make_unique<BoundUnaryOperator>(SyntaxKind::PlusToken, BoundUnaryOperatorKind::Identity, typeid(int),typeid(int)),
        std::make_unique<BoundUnaryOperator>(SyntaxKind::MinusToken, BoundUnaryOperatorKind::Negation, typeid(int),typeid(int))
    };

    inline BoundUnaryOperator* BoundUnaryOperator::Bind(SyntaxKind syntaxKind, const std::type_info& operandType)
    {
        for(auto const& op: trylang::_boundUnaryOperatorArray)
        {
            if(op->_syntaxKind == syntaxKind && std::strcmp(op->_operandType.name(), operandType.name()) == 0)
            {
                return op.get();
            }
        }

        return nullptr;
    }

    struct BoundBinaryOperator
    {
        SyntaxKind _syntaxKind;
        BoundBinaryOperatorKind _kind;
        const std::type_info& _leftOperandType;
        const std::type_info& _rightOperandType;
        const std::type_info& _resultType;

        BoundBinaryOperator(SyntaxKind syntaxKind, BoundBinaryOperatorKind kind, const std::type_info& leftOperandType, const std::type_info& rightOperandType, const std::type_info& resultType)
                : _syntaxKind(syntaxKind), _kind(kind), _leftOperandType(leftOperandType), _rightOperandType(rightOperandType), _resultType(resultType)
        {}

        static BoundBinaryOperator* Bind(SyntaxKind syntaxKind, const std::type_info& leftOperandType, const std::type_info& rightOperandType);
    };

    inline std::array<std::unique_ptr<BoundBinaryOperator>, 10> _boundBinaryOperatorArray = {
        std::make_unique<BoundBinaryOperator>(SyntaxKind::PlusToken, BoundBinaryOperatorKind::Addition, typeid(int),typeid(int), typeid(int)),
        std::make_unique<BoundBinaryOperator>(SyntaxKind::MinusToken, BoundBinaryOperatorKind::Subtraction, typeid(int),typeid(int), typeid(int)),
        std::make_unique<BoundBinaryOperator>(SyntaxKind::SlashToken, BoundBinaryOperatorKind::Division, typeid(int),typeid(int), typeid(int)),
        std::make_unique<BoundBinaryOperator>(SyntaxKind::StarToken, BoundBinaryOperatorKind::Multiplication, typeid(int),typeid(int), typeid(int)),

        std::make_unique<BoundBinaryOperator>(SyntaxKind::EqualsEqualsToken, BoundBinaryOperatorKind::LogicalEquality, typeid(int),typeid(int), typeid(bool)),
        std::make_unique<BoundBinaryOperator>(SyntaxKind::BangsEqualsToken, BoundBinaryOperatorKind::LogicalNotEquality, typeid(int),typeid(int), typeid(bool)),

        std::make_unique<BoundBinaryOperator>(SyntaxKind::AmpersandAmpersandToken, BoundBinaryOperatorKind::LogicalAnd, typeid(bool),typeid(bool), typeid(bool)),
        std::make_unique<BoundBinaryOperator>(SyntaxKind::PipePipeToken, BoundBinaryOperatorKind::LogicalOr, typeid(bool),typeid(bool), typeid(bool)),

        std::make_unique<BoundBinaryOperator>(SyntaxKind::EqualsEqualsToken, BoundBinaryOperatorKind::LogicalEquality, typeid(bool),typeid(bool), typeid(bool)),
        std::make_unique<BoundBinaryOperator>(SyntaxKind::BangsEqualsToken, BoundBinaryOperatorKind::LogicalNotEquality, typeid(bool),typeid(bool), typeid(bool)),

    };

    inline BoundBinaryOperator* BoundBinaryOperator::Bind(SyntaxKind syntaxKind, const std::type_info& leftOperandType, const std::type_info& rightOperandType)
    {
        for(auto const& op:_boundBinaryOperatorArray)
        {
            if(op->_syntaxKind == syntaxKind && std::strcmp(op->_leftOperandType.name(), leftOperandType.name()) == 0 && std::strcmp(op->_rightOperandType.name(), rightOperandType.name()) == 0)
            {
                return op.get();
            }
        }

        return nullptr;
    }
}