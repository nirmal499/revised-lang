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

    struct BoundLiteralExpression : public BoundExpressionNode
    {
        object_t _value;

        explicit BoundLiteralExpression(object_t&& value);
        
        const std::type_info& Type() override;
        BoundNodeKind Kind() override;
    };

    struct BoundUnaryExpression : public BoundExpressionNode
    {
        BoundUnaryOperatorKind _operatorKind;
        std::unique_ptr<BoundExpressionNode> _operand;

        BoundUnaryExpression(BoundUnaryOperatorKind operatorKind, std::unique_ptr<BoundExpressionNode> operand);
        
        const std::type_info& Type() override;
        BoundNodeKind Kind() override;
    };

    struct BoundBinaryExpression : public BoundExpressionNode
    {
        std::unique_ptr<BoundExpressionNode> _left;
        BoundBinaryOperatorKind _operatorKind;
        std::unique_ptr<BoundExpressionNode> _right;

        BoundBinaryExpression(std::unique_ptr<BoundExpressionNode> left, BoundBinaryOperatorKind operatorKind, std::unique_ptr<BoundExpressionNode> right);
        
        const std::type_info& Type() override;
        BoundNodeKind Kind() override;
    };
}