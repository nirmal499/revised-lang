#pragma once

#include <string>
#include <memory>

namespace trylang
{
    struct SyntaxToken;
    struct ExpressionSyntax;
    struct CompilationUnitSyntax;
    
    struct SyntaxTree
    {
        std::string _errors;
        std::unique_ptr<CompilationUnitSyntax> _root;
        std::string _text;

        explicit SyntaxTree(std::string text);

        static std::unique_ptr<SyntaxTree> Parse(std::string text);

        SyntaxTree(const SyntaxTree&) = delete;
        SyntaxTree& operator=(const SyntaxTree&) = delete;

        SyntaxTree(SyntaxTree&&) = default;
        SyntaxTree& operator=(SyntaxTree&&) = default;
    };
}