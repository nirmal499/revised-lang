#pragma once

#include <vector>
#include <codeanalysis/BoundNodeKind.hpp>
#include <codeanalysis/Types.hpp>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <iostream>
#include <typeinfo>
#include <codeanalysis/VariableSymbol.hpp>

namespace trylang
{
    struct BoundNode
    {
        virtual BoundNodeKind Kind() = 0;
        virtual ~BoundNode() = default;
    };
    
    struct BoundExpressionNode : public BoundNode
    {   
        /* const std::type_info& since typeid() returns that only */
        virtual const std::type_info& Type() = 0;
        ~BoundExpressionNode() override = default;
    };

    struct BoundVariableExpression : public BoundExpressionNode
    {
        VariableSymbol _variable;

        BoundVariableExpression(VariableSymbol variable);

        const std::type_info& Type() override;
        BoundNodeKind Kind() override;
    };

    struct BoundAssignmentExpression : public BoundExpressionNode
    {
        VariableSymbol _variable;
        std::unique_ptr<BoundExpressionNode> _expression;
        BoundAssignmentExpression(VariableSymbol variable, std::unique_ptr<BoundExpressionNode> expression);

        const std::type_info& Type() override;
        BoundNodeKind Kind() override;
    };

    struct BoundLiteralExpression : public BoundExpressionNode
    {
        object_t _value;

        explicit BoundLiteralExpression(const object_t& value);
        
        const std::type_info& Type() override;
        BoundNodeKind Kind() override;
    };

    struct BoundUnaryExpression : public BoundExpressionNode
    {
        BoundUnaryOperator* _op;
        std::unique_ptr<BoundExpressionNode> _operand;

        BoundUnaryExpression(BoundUnaryOperator* op, std::unique_ptr<BoundExpressionNode> operand);
        
        const std::type_info& Type() override;
        BoundNodeKind Kind() override;
    };

    struct BoundBinaryExpression : public BoundExpressionNode
    {
        std::unique_ptr<BoundExpressionNode> _left;
        BoundBinaryOperator* _op;
        std::unique_ptr<BoundExpressionNode> _right;

        BoundBinaryExpression(std::unique_ptr<BoundExpressionNode> left, BoundBinaryOperator* op, std::unique_ptr<BoundExpressionNode> right);
        
        const std::type_info& Type() override;
        BoundNodeKind Kind() override;
    };
}