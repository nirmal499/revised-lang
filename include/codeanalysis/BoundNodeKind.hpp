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
        ParenthesizedExpression,
        VariableExpression,
        AssignmentExpression,

        BlockStatement,
        ExpressionStatement,
        BoundVariableDeclarationStatement,
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
        {BoundNodeKind::BoundVariableDeclarationStatement, "BoundVariableDeclarationStatement"},

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
        Less,
        Greater,
        LessEquals,
        GreaterEquals
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
        {BoundBinaryOperatorKind::Less, "Less"},
        {BoundBinaryOperatorKind::Greater, "Greater"},
        {BoundBinaryOperatorKind::LessEquals, "LessEquals"},
        {BoundBinaryOperatorKind::GreaterEquals, "GreaterEquals"},

    };

    inline std::ostream& operator<<(std::ostream& out, BoundNodeKind kind)
    {
        return out << __boundNodeStringMap[kind];
    }

    struct BoundUnaryOperator
    {
        SyntaxKind _syntaxKind;
        BoundUnaryOperatorKind _kind;
        const char* _operandType;
        const char* _resultType;

        BoundUnaryOperator(SyntaxKind syntaxKind, BoundUnaryOperatorKind kind, const char* operandType, const char* resultType)
            : _syntaxKind(syntaxKind), _kind(kind), _operandType(operandType), _resultType(resultType)
        {}

        static BoundUnaryOperator* Bind(SyntaxKind syntaxKind, const char* operandType);

    };

    inline std::array<std::unique_ptr<BoundUnaryOperator>, 3> _boundUnaryOperatorArray = {
        std::make_unique<BoundUnaryOperator>(SyntaxKind::BangToken, BoundUnaryOperatorKind::LogicalNegation, typeid(bool).name(),typeid(bool).name()),
        std::make_unique<BoundUnaryOperator>(SyntaxKind::PlusToken, BoundUnaryOperatorKind::Identity, typeid(int).name(),typeid(int).name()),
        std::make_unique<BoundUnaryOperator>(SyntaxKind::MinusToken, BoundUnaryOperatorKind::Negation, typeid(int).name(),typeid(int).name())
    };

    inline BoundUnaryOperator* BoundUnaryOperator::Bind(SyntaxKind syntaxKind, const char* operandType)
    {
        for(auto const& op: trylang::_boundUnaryOperatorArray)
        {
            if(op->_syntaxKind == syntaxKind && std::strcmp(op->_operandType, operandType) == 0)
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
        const char* _leftOperandType;
        const char* _rightOperandType;
        const char* _resultType;

        BoundBinaryOperator(SyntaxKind syntaxKind, BoundBinaryOperatorKind kind, const char* leftOperandType, const char* rightOperandType, const char* resultType)
                : _syntaxKind(syntaxKind), _kind(kind), _leftOperandType(leftOperandType), _rightOperandType(rightOperandType), _resultType(resultType)
        {}

        static BoundBinaryOperator* Bind(SyntaxKind syntaxKind, const char* leftOperandType, const char* rightOperandType);
    };

    inline std::array<std::unique_ptr<BoundBinaryOperator>, 14> _boundBinaryOperatorArray = {
        std::make_unique<BoundBinaryOperator>(SyntaxKind::PlusToken, BoundBinaryOperatorKind::Addition, typeid(int).name(),typeid(int).name(), typeid(int).name()),
        std::make_unique<BoundBinaryOperator>(SyntaxKind::MinusToken, BoundBinaryOperatorKind::Subtraction, typeid(int).name(),typeid(int).name(), typeid(int).name()),
        std::make_unique<BoundBinaryOperator>(SyntaxKind::SlashToken, BoundBinaryOperatorKind::Division, typeid(int).name(),typeid(int).name(), typeid(int).name()),
        std::make_unique<BoundBinaryOperator>(SyntaxKind::StarToken, BoundBinaryOperatorKind::Multiplication, typeid(int).name(),typeid(int).name(), typeid(int).name()),

        std::make_unique<BoundBinaryOperator>(SyntaxKind::EqualsEqualsToken, BoundBinaryOperatorKind::LogicalEquality, typeid(int).name(),typeid(int).name(), typeid(bool).name()),
        std::make_unique<BoundBinaryOperator>(SyntaxKind::BangsEqualsToken, BoundBinaryOperatorKind::LogicalNotEquality, typeid(int).name(),typeid(int).name(), typeid(bool).name()),

        std::make_unique<BoundBinaryOperator>(SyntaxKind::LessThanToken, BoundBinaryOperatorKind::Less, typeid(int).name(),typeid(int).name(), typeid(bool).name()),
        std::make_unique<BoundBinaryOperator>(SyntaxKind::LessThanEqualsToken, BoundBinaryOperatorKind::LessEquals, typeid(int).name(),typeid(int).name(), typeid(bool).name()),
        std::make_unique<BoundBinaryOperator>(SyntaxKind::GreaterThanToken, BoundBinaryOperatorKind::Greater, typeid(int).name(),typeid(int).name(), typeid(bool).name()),
        std::make_unique<BoundBinaryOperator>(SyntaxKind::GreaterThanEqualsToken, BoundBinaryOperatorKind::GreaterEquals, typeid(int).name(),typeid(int).name(), typeid(bool).name()),

        std::make_unique<BoundBinaryOperator>(SyntaxKind::AmpersandAmpersandToken, BoundBinaryOperatorKind::LogicalAnd, typeid(bool).name(),typeid(bool).name(), typeid(bool).name()),
        std::make_unique<BoundBinaryOperator>(SyntaxKind::PipePipeToken, BoundBinaryOperatorKind::LogicalOr, typeid(bool).name(),typeid(bool).name(), typeid(bool).name()),

        std::make_unique<BoundBinaryOperator>(SyntaxKind::EqualsEqualsToken, BoundBinaryOperatorKind::LogicalEquality, typeid(bool).name(),typeid(bool).name(), typeid(bool).name()),
        std::make_unique<BoundBinaryOperator>(SyntaxKind::BangsEqualsToken, BoundBinaryOperatorKind::LogicalNotEquality, typeid(bool).name(),typeid(bool).name(), typeid(bool).name()),

    };

    inline BoundBinaryOperator* BoundBinaryOperator::Bind(SyntaxKind syntaxKind, const char* leftOperandType, const char* rightOperandType)
    {
        for(auto const& op:_boundBinaryOperatorArray)
        {
            if(op->_syntaxKind == syntaxKind && std::strcmp(op->_leftOperandType, leftOperandType) == 0 && std::strcmp(op->_rightOperandType, rightOperandType) == 0)
            {
                return op.get();
            }
        }

        return nullptr;
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