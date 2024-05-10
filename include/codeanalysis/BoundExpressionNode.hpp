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
        /* const char* since typeid().name() returns that only */
        virtual const char* Type() = 0;
        ~BoundExpressionNode() override = default;
    };

    struct BoundStatementNode : public BoundNode
    {
        ~BoundStatementNode() override = default;
    };

    struct BoundBlockStatement : public BoundStatementNode
    {
        std::vector<std::unique_ptr<BoundStatementNode>> _statements;

        explicit BoundBlockStatement(std::vector<std::unique_ptr<BoundStatementNode>> statements);

        BoundNodeKind Kind() override;
    };

    struct BoundExpressionStatement : public BoundStatementNode
    {
        std::unique_ptr<BoundExpressionNode> _expression;

        explicit BoundExpressionStatement(std::unique_ptr<BoundExpressionNode> expression);

        BoundNodeKind Kind() override;
    };

    struct BoundVariableExpression : public BoundExpressionNode
    {
        VariableSymbol _variable;

        BoundVariableExpression(VariableSymbol variable);

        const char* Type() override;
        BoundNodeKind Kind() override;
    };

    struct BoundAssignmentExpression : public BoundExpressionNode
    {
        VariableSymbol _variable;
        std::unique_ptr<BoundExpressionNode> _expression;
        BoundAssignmentExpression(VariableSymbol variable, std::unique_ptr<BoundExpressionNode> expression);

        const char* Type() override;
        BoundNodeKind Kind() override;
    };

    struct BoundLiteralExpression : public BoundExpressionNode
    {
        object_t _value;

        explicit BoundLiteralExpression(const object_t& value);
        
        const char* Type() override;
        BoundNodeKind Kind() override;
    };

    struct BoundUnaryExpression : public BoundExpressionNode
    {
        BoundUnaryOperator* _op;
        std::unique_ptr<BoundExpressionNode> _operand;

        BoundUnaryExpression(BoundUnaryOperator* op, std::unique_ptr<BoundExpressionNode> operand);
        
        const char* Type() override;
        BoundNodeKind Kind() override;
    };

    struct BoundBinaryExpression : public BoundExpressionNode
    {
        std::unique_ptr<BoundExpressionNode> _left;
        BoundBinaryOperator* _op;
        std::unique_ptr<BoundExpressionNode> _right;

        BoundBinaryExpression(std::unique_ptr<BoundExpressionNode> left, BoundBinaryOperator* op, std::unique_ptr<BoundExpressionNode> right);
        
        const char* Type() override;
        BoundNodeKind Kind() override;
    };
}