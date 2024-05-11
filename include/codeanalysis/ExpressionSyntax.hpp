#pragma once

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

    void PrettyPrint(SyntaxNode* node, std::string indent = "");

    struct SyntaxToken : SyntaxNode
    {
        SyntaxKind _kind;
        int _position;
        std::string _text;
        object_t _value;

        SyntaxToken(SyntaxKind kind, int position, std::string&& text, const object_t& value);

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

    struct PrintVisitor
    {
        void operator()(int number)
        {
            std::cout << " " << number;
        }

        void operator()(bool boolValue)
        {
            std::cout << std::boolalpha << boolValue;
        }
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

    struct CompilationUnitSyntax : public SyntaxNode
    {
        std::unique_ptr<StatementSyntax> _statement;
        std::shared_ptr<SyntaxToken> _endOfFileToken;

        CompilationUnitSyntax(std::unique_ptr<StatementSyntax> statement, const std::shared_ptr<SyntaxToken>& endOfFileToken);

        SyntaxKind Kind() override;
        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct BlockStatementSyntax: public StatementSyntax
    {
        std::shared_ptr<SyntaxToken> _openBraceToken;
        std::vector<std::unique_ptr<StatementSyntax>> _statements;
        std::shared_ptr<SyntaxToken> _closeBraceToken;

        BlockStatementSyntax(
                    const std::shared_ptr<SyntaxToken>& openBraceToken,
                    std::vector<std::unique_ptr<StatementSyntax>> statements ,
                    const std::shared_ptr<SyntaxToken>& closeBraceToken);

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct VariableDeclarationSyntax : public StatementSyntax
    {
        std::shared_ptr<SyntaxToken> _keyword;
        std::shared_ptr<SyntaxToken> _identifier;
        std::shared_ptr<SyntaxToken> _equalsToken;
        std::unique_ptr<ExpressionSyntax> _expression;


        VariableDeclarationSyntax(
                    const std::shared_ptr<SyntaxToken>& keyword,
                    const std::shared_ptr<SyntaxToken>& identifier,
                    const std::shared_ptr<SyntaxToken>& equalsToken,
                    std::unique_ptr<ExpressionSyntax> expression
                    );

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

    struct ElseClauseSyntax: public StatementSyntax
    {
        std::shared_ptr<SyntaxToken> _elseKeyword;
        std::unique_ptr<StatementSyntax> _elseStatement;

        ElseClauseSyntax(const std::shared_ptr<SyntaxToken>& elseKeyword, std::unique_ptr<StatementSyntax> elseStatement);

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;

    };

    struct IfStatementSyntax : public StatementSyntax
    {
        std::shared_ptr<SyntaxToken> _ifKeyword;
        std::unique_ptr<ExpressionSyntax> _condition;
        std::unique_ptr<StatementSyntax> _thenStatement;
        std::unique_ptr<StatementSyntax> _elseClause = nullptr;

        IfStatementSyntax(
                const std::shared_ptr<SyntaxToken>& ifKeyword,
                std::unique_ptr<ExpressionSyntax> condition,
                std::unique_ptr<StatementSyntax> thenStatement,
                std::unique_ptr<StatementSyntax> elseClause
                );

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct WhileStatementSyntax : public StatementSyntax
    {
        std::shared_ptr<SyntaxToken> _whileKeyword;
        std::unique_ptr<ExpressionSyntax> _condition;
        std::unique_ptr<StatementSyntax> _body;

        WhileStatementSyntax(
                const std::shared_ptr<SyntaxToken>& whileKeyword,
                std::unique_ptr<ExpressionSyntax> condition,
                std::unique_ptr<StatementSyntax> body
        );

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct ForStatementSyntax : public StatementSyntax
    {
        std::shared_ptr<SyntaxToken> _keyword;
        std::shared_ptr<SyntaxToken> _identifier;
        std::shared_ptr<SyntaxToken> _equalsToken;
        std::unique_ptr<ExpressionSyntax>_lowerBound;
        std::shared_ptr<SyntaxToken> _toKeyword;
        std::unique_ptr<ExpressionSyntax> _upperBound;
        std::unique_ptr<StatementSyntax> _body;

        ForStatementSyntax(
                const std::shared_ptr<SyntaxToken>& keyword,
                const std::shared_ptr<SyntaxToken>& identifierToken,
                const std::shared_ptr<SyntaxToken>& equalsToken,
                std::unique_ptr<ExpressionSyntax> lowerBound,
                const std::shared_ptr<SyntaxToken>& toKeyword,
                std::unique_ptr<ExpressionSyntax> upperBound,
                std::unique_ptr<StatementSyntax> body
        );

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct NameExpressionSyntax : public ExpressionSyntax
    {
        std::shared_ptr<SyntaxToken> _identifierToken;
        
        explicit NameExpressionSyntax(const std::shared_ptr<SyntaxToken>& identifierToken);

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct AssignmentExpressionSyntax : public ExpressionSyntax
    {

        std::shared_ptr<SyntaxToken> _identifierToken;
        std::shared_ptr<SyntaxToken> _equalsToken;
        std::unique_ptr<ExpressionSyntax> _expression;

        AssignmentExpressionSyntax(const std::shared_ptr<SyntaxToken>& identifierToken, const std::shared_ptr<SyntaxToken>& equalsToken, std::unique_ptr<ExpressionSyntax> expression);

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct LiteralExpressionSyntax : public ExpressionSyntax
    {
        std::shared_ptr<SyntaxToken> _literalToken;
        object_t _value;
        
        explicit LiteralExpressionSyntax(const std::shared_ptr<SyntaxToken>& literalToken);
        LiteralExpressionSyntax(const std::shared_ptr<SyntaxToken>& literalToken, const object_t& value);
        
        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct BinaryExpressionSyntax : public ExpressionSyntax
    {

        std::shared_ptr<SyntaxToken> _operatorToken;
        std::unique_ptr<ExpressionSyntax> _left;
        std::unique_ptr<ExpressionSyntax> _right;

        BinaryExpressionSyntax(std::unique_ptr<ExpressionSyntax> left, const std::shared_ptr<SyntaxToken>& operatorToken, std::unique_ptr<ExpressionSyntax> right);

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct UnaryExpressionSyntax : public ExpressionSyntax
    {

        std::shared_ptr<SyntaxToken> _operatorToken;
        std::unique_ptr<ExpressionSyntax> _operand;

        UnaryExpressionSyntax(const std::shared_ptr<SyntaxToken>& operatorToken, std::unique_ptr<ExpressionSyntax> operand);

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };

    struct ParenthesizedExpressionSyntax : public ExpressionSyntax
    {

        std::shared_ptr<SyntaxToken> _openParenthesisToken;
        std::unique_ptr<ExpressionSyntax> _expression;
        std::shared_ptr<SyntaxToken> _closeParenthesisToken;

        ParenthesizedExpressionSyntax(const std::shared_ptr<SyntaxToken>& openParenthesisToken, std::unique_ptr<ExpressionSyntax> expression, const std::shared_ptr<SyntaxToken>& closeParenthesisToken);

        SyntaxKind Kind() override;

        std::vector<SyntaxNode*> GetChildren() override;
    };
}