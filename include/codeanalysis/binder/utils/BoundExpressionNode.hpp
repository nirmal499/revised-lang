#pragma once

#include <vector>
#include <codeanalysis/binder/utils/BoundNodeKind.hpp>
#include <codeanalysis/utils/Types.hpp>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <iostream>
#include <typeinfo>
#include <codeanalysis/utils/Symbol.hpp>

namespace trylang
{
    struct BoundNode
    {
        virtual BoundNodeKind Kind() = 0;
        virtual std::vector<BoundNode*> GetChildren() = 0;
        virtual ~BoundNode() = default;
    };

    struct BoundUnaryOperator : public BoundNode
    {
        SyntaxKind _syntaxKind;
        BoundNodeKind _kind;
        const char* _operandType;
        const char* _resultType;

        BoundUnaryOperator(SyntaxKind syntaxKind, BoundNodeKind kind, const char* operandType, const char* resultType)
                : _syntaxKind(syntaxKind), _kind(kind), _operandType(operandType), _resultType(resultType)
        {}

        static BoundUnaryOperator* Bind(SyntaxKind syntaxKind, const char* operandType);

        /*****************************************************************************************************************************************************/
        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
        /*****************************************************************************************************************************************************/

    };

    struct BoundBinaryOperator : public BoundNode
    {
        SyntaxKind _syntaxKind;
        BoundNodeKind _kind;
        const char* _leftOperandType;
        const char* _rightOperandType;
        const char* _resultType;

        BoundBinaryOperator(SyntaxKind syntaxKind, BoundNodeKind kind, const char* leftOperandType, const char* rightOperandType, const char* resultType)
                : _syntaxKind(syntaxKind), _kind(kind), _leftOperandType(leftOperandType), _rightOperandType(rightOperandType), _resultType(resultType)
        {}

        static BoundBinaryOperator* Bind(SyntaxKind syntaxKind, const char* leftOperandType, const char* rightOperandType);

        /*****************************************************************************************************************************************************/
        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
        /*****************************************************************************************************************************************************/

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
        std::vector<BoundNode*> GetChildren() override;
    };

    struct BoundContinueStatement : public BoundStatementNode
    {
        BoundContinueStatement();

        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };

    struct BoundBreakStatement : public BoundStatementNode
    {
        BoundBreakStatement();

        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };

    /*********************************************************************************************************************/
    struct BoundGotoStatement : public BoundStatementNode
    {
        LabelSymbol _label;

        explicit BoundGotoStatement(LabelSymbol label);

        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };

    struct BoundConditionalGotoStatement : public BoundStatementNode
    {
        LabelSymbol _label;
        std::unique_ptr<BoundExpressionNode> _condition;
        bool _jumpIfFalse;

        BoundConditionalGotoStatement(LabelSymbol label, std::unique_ptr<BoundExpressionNode> condition, bool jumpIfFalse);

        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };

    struct BoundLabelStatement : public BoundStatementNode
    {
        LabelSymbol _label;

        explicit BoundLabelStatement(LabelSymbol label);

        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };
    /*********************************************************************************************************************/

    struct BoundReturnStatement : public BoundStatementNode
    {
        std::unique_ptr<BoundExpressionNode> _expression;

        explicit BoundReturnStatement(std::unique_ptr<BoundExpressionNode> expression);

        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };

    struct BoundExpressionStatement : public BoundStatementNode
    {
        std::unique_ptr<BoundExpressionNode> _expression;

        explicit BoundExpressionStatement(std::unique_ptr<BoundExpressionNode> expression);

        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };

    struct BoundIfStatement : public BoundStatementNode
    {
        std::unique_ptr<BoundExpressionNode> _condition;
        std::unique_ptr<BoundStatementNode> _statement;
        std::unique_ptr<BoundStatementNode> _elseStatement = nullptr;

        BoundIfStatement(std::unique_ptr<BoundExpressionNode> condition, std::unique_ptr<BoundStatementNode> statement, std::unique_ptr<BoundStatementNode> elseStatement);

        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };

    struct BoundWhileStatement : public BoundStatementNode
    {
        std::unique_ptr<BoundExpressionNode> _condition;
        std::unique_ptr<BoundStatementNode> _body;

        std::pair<LabelSymbol, LabelSymbol> _loopLabel; /* break and continue Label */

        BoundWhileStatement(std::unique_ptr<BoundExpressionNode> condition, std::unique_ptr<BoundStatementNode> body, std::pair<LabelSymbol, LabelSymbol> loopLabel);

        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };

    struct BoundForStatement : public BoundStatementNode
    {
        std::shared_ptr<VariableSymbol> _variable;
        std::unique_ptr<BoundExpressionNode>_lowerBound;
        std::unique_ptr<BoundExpressionNode> _upperBound;
        std::shared_ptr<VariableSymbol> _variableForUpperBoundToBeUsedDuringRewritingForIntoWhile;
        std::unique_ptr<BoundStatementNode> _body;

        std::pair<LabelSymbol, LabelSymbol> _loopLabel; /* break and continue Label */

        BoundForStatement(
                const std::shared_ptr<VariableSymbol>& variable,
                std::unique_ptr<BoundExpressionNode> lowerBound,
                std::unique_ptr<BoundExpressionNode> upperBound,
                const std::shared_ptr<VariableSymbol>& variableForUpperBoundToBeUsedDuringRewritingForIntoWhile,
                std::unique_ptr<BoundStatementNode> body,
                std::pair<LabelSymbol, LabelSymbol> loopLabel
                );

        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };

    struct BoundVariableDeclaration : public BoundStatementNode
    {
        std::shared_ptr<VariableSymbol> _variable;
        std::unique_ptr<BoundExpressionNode> _expression;

        BoundVariableDeclaration(const std::shared_ptr<VariableSymbol>& variable, std::unique_ptr<BoundExpressionNode> expression);

        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };

    struct BoundVariableExpression : public BoundExpressionNode
    {
        std::shared_ptr<VariableSymbol> _variable;

        explicit BoundVariableExpression(const std::shared_ptr<VariableSymbol>& variable);

        const char* Type() override;
        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };

    struct BoundErrorExpression : public BoundExpressionNode
    {
        BoundErrorExpression() = default;

        const char* Type() override;
        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };


    struct BoundCallExpression : public BoundExpressionNode
    {

        std::shared_ptr<FunctionSymbol> _function;
        std::vector<std::unique_ptr<BoundExpressionNode>> _arguments;

        BoundCallExpression(const std::shared_ptr<FunctionSymbol>& function, std::vector<std::unique_ptr<BoundExpressionNode>> arguments);

        const char* Type() override;
        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };

    struct BoundConversionExpression : public BoundExpressionNode
    {
        const char* _toType; /* expression to be converted to _toType in the evaluator */
        std::unique_ptr<BoundExpressionNode> _expression;

        BoundConversionExpression(const char* toType, std::unique_ptr<BoundExpressionNode> expression);

        const char* Type() override;
        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };

    struct BoundAssignmentExpression : public BoundExpressionNode
    {
        std::shared_ptr<VariableSymbol> _variable;
        std::unique_ptr<BoundExpressionNode> _expression;
        BoundAssignmentExpression(const std::shared_ptr<VariableSymbol>& variable, std::unique_ptr<BoundExpressionNode> expression);

        const char* Type() override;
        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };

    struct BoundLiteralExpression : public BoundExpressionNode
    {
        object_t _value;

        explicit BoundLiteralExpression(const object_t& value);
        
        const char* Type() override;
        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };

    struct BoundUnaryExpression : public BoundExpressionNode
    {
        BoundUnaryOperator* _op;
        std::unique_ptr<BoundExpressionNode> _operand;

        BoundUnaryExpression(BoundUnaryOperator* op, std::unique_ptr<BoundExpressionNode> operand);
        
        const char* Type() override;
        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };

    struct BoundBinaryExpression : public BoundExpressionNode
    {
        std::unique_ptr<BoundExpressionNode> _left;
        BoundBinaryOperator* _op;
        std::unique_ptr<BoundExpressionNode> _right;

        BoundBinaryExpression(std::unique_ptr<BoundExpressionNode> left, BoundBinaryOperator* op, std::unique_ptr<BoundExpressionNode> right);
        
        const char* Type() override;
        BoundNodeKind Kind() override;
        std::vector<BoundNode*> GetChildren() override;
    };

    void PrettyPrintBoundNodes(BoundNode* node, std::string indent = "");
    void PrettyPrintBoundNodesForFunctionBodies(const std::unordered_map<std::string, std::pair<std::shared_ptr<FunctionSymbol>, std::unique_ptr<BoundBlockStatement>>>& functionBodies, std::string indent = "");

}