#pragma once

#include <codeanalysis/Lexer.hpp>
#include <vector>
#include <codeanalysis/SyntaxKind.hpp>
#include <codeanalysis/Types.hpp>
#include <memory>
#include <string>
#include <iostream>

namespace trylang
{
    struct SyntaxNode
    {
        virtual SyntaxKind Kind() = 0;
        virtual std::vector<SyntaxNode*> GetChildren() = 0;
        virtual ~SyntaxNode() = default;
    };

    void PrettyPrintSyntaxNodes(SyntaxNode* node, std::string indent = "");

    struct SyntaxToken : SyntaxNode
    {
        SyntaxKind _kind;
        int _line;
        std::string _text;
        object_t _value;

        SyntaxToken(SyntaxKind kind, int line, std::string&& text, object_t&& value);

        SyntaxToken(const SyntaxToken&) = delete;
        SyntaxToken& operator=(const SyntaxToken&) = delete;

        SyntaxToken(SyntaxToken&&) = default;
        SyntaxToken& operator=(SyntaxToken&&) = default;

        /**********************************************************************************************/
        SyntaxKind Kind() override;
        std::vector<SyntaxNode*> GetChildren() override;
        /**********************************************************************************************/

        friend std::ostream& operator<<(std::ostream& out, const SyntaxToken& token);
    };

    /* Abstract Class */
    struct ExpressionSyntax : public SyntaxNode
    {
        ~ExpressionSyntax() override = default;
    };

    /* Abstract Class */
    struct StatementSyntax : public SyntaxNode
    {
        ~StatementSyntax() override = default;
    };

    struct TypeClauseSyntax : public SyntaxNode
    {
        std::unique_ptr<SyntaxToken> _colonToken;
        std::unique_ptr<SyntaxToken> _identifierToken;

        TypeClauseSyntax(std::unique_ptr<SyntaxToken> colonToken, std::unique_ptr<SyntaxToken> identifierToken);
        SyntaxKind Kind() override;
        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct ParameterSyntax : public SyntaxNode
    {
        std::unique_ptr<SyntaxToken> _identifier;
        std::unique_ptr<TypeClauseSyntax> _type;

        ParameterSyntax(std::unique_ptr<SyntaxToken> identifier, std::unique_ptr<TypeClauseSyntax> type);
        SyntaxKind Kind() override;
        std::vector<SyntaxNode*> GetChildren() override;

    };

    struct FunctionDeclarationStatementSyntax : public StatementSyntax
    {
        std::unique_ptr<SyntaxToken> _functionKeyword;
        std::unique_ptr<SyntaxToken> _identifier;
        std::unique_ptr<SyntaxToken> _openParenthesisToken;
        std::vector<std::unique_ptr<ParameterSyntax>> _parameters;
        std::unique_ptr<SyntaxToken> _closeParenthesisToken;
        std::unique_ptr<TypeClauseSyntax> _typeClause;
        std::unique_ptr<StatementSyntax> _body;

        FunctionDeclarationStatementSyntax(
                std::unique_ptr<SyntaxToken> functionKeyword,
                std::unique_ptr<SyntaxToken> identifier,
                std::unique_ptr<SyntaxToken> openParenthesisToken,
                std::vector<std::unique_ptr<ParameterSyntax>> parameters,
                std::unique_ptr<SyntaxToken> closeParenthesisToken,
                std::unique_ptr<TypeClauseSyntax> typeClause,
                std::unique_ptr<StatementSyntax> body
        );

        SyntaxKind Kind() override;
        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct VariableDeclarationStatementSyntax : public StatementSyntax
    {
        std::unique_ptr<SyntaxToken> _keyword;
        std::unique_ptr<SyntaxToken> _identifier; /* identifierToken */
        std::unique_ptr<TypeClauseSyntax> _typeClause = nullptr;
        std::unique_ptr<SyntaxToken> _equalsToken;
        std::unique_ptr<ExpressionSyntax> _expression;


        VariableDeclarationStatementSyntax(
                    std::unique_ptr<SyntaxToken> keyword,
                    std::unique_ptr<SyntaxToken> identifier,
                    std::unique_ptr<TypeClauseSyntax> typeClause,
                    std::unique_ptr<SyntaxToken> equalsToken,
                    std::unique_ptr<ExpressionSyntax> expression
                );

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct CompilationUnitSyntax : public SyntaxNode
    {
        std::vector<std::unique_ptr<StatementSyntax>> _statements; /* This includes both VarDeclarationStatementSyntax and FunctionDeclarationStatementSyntax */

        CompilationUnitSyntax(std::vector<std::unique_ptr<StatementSyntax>> statements);

        SyntaxKind Kind() override;
        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct BlockStatementSyntax: public StatementSyntax
    {
        std::unique_ptr<SyntaxToken> _openBraceToken;
        std::vector<std::unique_ptr<StatementSyntax>> _statements;
        std::unique_ptr<SyntaxToken> _closeBraceToken;

        BlockStatementSyntax(
                    std::unique_ptr<SyntaxToken> openBraceToken,
                    std::vector<std::unique_ptr<StatementSyntax>> statements ,
                    std::unique_ptr<SyntaxToken> closeBraceToken);

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct ExpressionStatementSyntax : public StatementSyntax
    {
        std::unique_ptr<ExpressionSyntax> _expression;
        explicit ExpressionStatementSyntax(std::unique_ptr<ExpressionSyntax> expression);

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct ElseStatementSyntax: public StatementSyntax
    {
        std::unique_ptr<SyntaxToken> _elseKeyword;
        std::unique_ptr<StatementSyntax> _elseStatement;

        ElseStatementSyntax(std::unique_ptr<SyntaxToken> elseKeyword, std::unique_ptr<StatementSyntax> elseStatement);

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;

    };

    struct IfStatementSyntax : public StatementSyntax
    {
        std::unique_ptr<SyntaxToken> _ifKeyword;
        std::unique_ptr<ExpressionSyntax> _condition;
        std::unique_ptr<StatementSyntax> _thenStatement;
        std::unique_ptr<StatementSyntax> _elseClause = nullptr;

        IfStatementSyntax(
                std::unique_ptr<SyntaxToken> ifKeyword,
                std::unique_ptr<ExpressionSyntax> condition,
                std::unique_ptr<StatementSyntax> thenStatement,
                std::unique_ptr<StatementSyntax> elseClause
                );

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct WhileStatementSyntax : public StatementSyntax
    {
        std::unique_ptr<SyntaxToken> _whileKeyword;
        std::unique_ptr<ExpressionSyntax> _condition;
        std::unique_ptr<StatementSyntax> _body;

        WhileStatementSyntax(
                std::unique_ptr<SyntaxToken> whileKeyword,
                std::unique_ptr<ExpressionSyntax> condition,
                std::unique_ptr<StatementSyntax> body
        );

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct BreakStatementSyntax : public StatementSyntax
    {
        std::unique_ptr<SyntaxToken> _breakKeyword;

        explicit BreakStatementSyntax(std::unique_ptr<SyntaxToken> breakKeyword);

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct ContinueStatementSyntax : public StatementSyntax
    {
        std::unique_ptr<SyntaxToken> _continueKeyword;

        explicit ContinueStatementSyntax(std::unique_ptr<SyntaxToken> continueKeyword);

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct ForStatementSyntax : public StatementSyntax
    {
        std::unique_ptr<SyntaxToken> _keyword;
        std::unique_ptr<SyntaxToken> _identifier;
        std::unique_ptr<SyntaxToken> _equalsToken;
        std::unique_ptr<ExpressionSyntax>_lowerBound;
        std::unique_ptr<SyntaxToken> _toKeyword;
        std::unique_ptr<ExpressionSyntax> _upperBound;
        std::unique_ptr<StatementSyntax> _body;

        ForStatementSyntax(
                std::unique_ptr<SyntaxToken> keyword,
                std::unique_ptr<SyntaxToken> identifierToken,
                std::unique_ptr<SyntaxToken> equalsToken,
                std::unique_ptr<ExpressionSyntax> lowerBound,
                std::unique_ptr<SyntaxToken> toKeyword,
                std::unique_ptr<ExpressionSyntax> upperBound,
                std::unique_ptr<StatementSyntax> body
        );

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };


    struct CallExpressionSyntax: public ExpressionSyntax
    {
        std::unique_ptr<SyntaxToken> _identifier; /* name of the function during calling function */
        std::unique_ptr<SyntaxToken> _openParenthesis;
        std::unique_ptr<SyntaxToken> _closeParenthesis;
        std::vector<std::unique_ptr<ExpressionSyntax>> _arguments;

        CallExpressionSyntax(std::unique_ptr<SyntaxToken> identifier,
                             std::unique_ptr<SyntaxToken> openParenthesis,
                             std::vector<std::unique_ptr<ExpressionSyntax>> arguments,
                             std::unique_ptr<SyntaxToken> closeParenthesis);

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct NameExpressionSyntax : public ExpressionSyntax
    {
        std::unique_ptr<SyntaxToken> _identifierToken;
        
        explicit NameExpressionSyntax(std::unique_ptr<SyntaxToken> identifierToken);

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct AssignmentExpressionSyntax : public ExpressionSyntax
    {

        std::unique_ptr<SyntaxToken> _identifierToken;
        std::unique_ptr<SyntaxToken> _equalsToken;
        std::unique_ptr<ExpressionSyntax> _expression;

        AssignmentExpressionSyntax(std::unique_ptr<SyntaxToken> identifierToken, std::unique_ptr<SyntaxToken> equalsToken, std::unique_ptr<ExpressionSyntax> expression);

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct LiteralExpressionSyntax : public ExpressionSyntax
    {
        std::unique_ptr<SyntaxToken> _literalToken;
        object_t _value;
        
        explicit LiteralExpressionSyntax(std::unique_ptr<SyntaxToken> literalToken);
        LiteralExpressionSyntax(std::unique_ptr<SyntaxToken> literalToken, const object_t& value);
        
        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct BinaryExpressionSyntax : public ExpressionSyntax
    {

        std::unique_ptr<SyntaxToken> _operatorToken;
        std::unique_ptr<ExpressionSyntax> _left;
        std::unique_ptr<ExpressionSyntax> _right;

        BinaryExpressionSyntax(std::unique_ptr<ExpressionSyntax> left, std::unique_ptr<SyntaxToken> operatorToken, std::unique_ptr<ExpressionSyntax> right);

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct UnaryExpressionSyntax : public ExpressionSyntax
    {

        std::unique_ptr<SyntaxToken> _operatorToken;
        std::unique_ptr<ExpressionSyntax> _operand;

        UnaryExpressionSyntax(std::unique_ptr<SyntaxToken> operatorToken, std::unique_ptr<ExpressionSyntax> operand);

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct ParenthesizedExpressionSyntax : public ExpressionSyntax
    {

        std::unique_ptr<SyntaxToken> _openParenthesisToken;
        std::unique_ptr<ExpressionSyntax> _expression;
        std::unique_ptr<SyntaxToken> _closeParenthesisToken;

        ParenthesizedExpressionSyntax(std::unique_ptr<SyntaxToken> openParenthesisToken, std::unique_ptr<ExpressionSyntax> expression, std::unique_ptr<SyntaxToken> closeParenthesisToken);

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };
}