#pragma once

#include <string>
#include <memory>

namespace trylang
{
    struct SyntaxToken;
    struct ExpressionSyntax;
    
    struct SyntaxTree
    {
        std::string _errors;
        std::unique_ptr<ExpressionSyntax> _root;
        std::shared_ptr<SyntaxToken> _endOfFileToken;

        SyntaxTree(std::string errors, std::unique_ptr<ExpressionSyntax> root, const std::shared_ptr<SyntaxToken>& endOfFileToken);

        static std::unique_ptr<SyntaxTree> Parse(std::string text);

        SyntaxTree(const SyntaxTree&) = delete;
        SyntaxTree& operator=(const SyntaxTree&) = delete;

        SyntaxTree(SyntaxTree&&) = default;
        SyntaxTree& operator=(SyntaxTree&&) = default;
    };
}