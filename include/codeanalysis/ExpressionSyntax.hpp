#pragma once

#include <vector>
#include <codeanalysis/SyntaxKind.hpp>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <iostream>

namespace trylang
{
    struct SyntaxNode
    {
        virtual SyntaxKind Kind() = 0;
        virtual std::vector<SyntaxNode*> GetChildren() = 0;
        virtual ~SyntaxNode(){}
    };

    void PrettyPrint(SyntaxNode* node, std::string indent = "");

    struct SyntaxToken : SyntaxNode
    {

        typedef std::optional<std::variant<int>> object_t;

        SyntaxKind _kind;
        int _position;
        std::string _text;
        object_t _value;

        SyntaxToken(SyntaxKind kind, int position, std::string&& text, object_t&& value);

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
    };
    
    struct ExpressionSyntax : public SyntaxNode
    {
        virtual ~ExpressionSyntax(){}
    };

    struct LiteralExpressionSyntax : public ExpressionSyntax
    {
        std::shared_ptr<SyntaxToken> _numberToken;
        
        LiteralExpressionSyntax(const std::shared_ptr<SyntaxToken>& numberToken);

        
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