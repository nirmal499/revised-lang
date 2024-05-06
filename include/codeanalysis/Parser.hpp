#pragma once

#include <vector>
#include <memory>
#include <sstream>
#include <codeanalysis/SyntaxKind.hpp>

namespace trylang
{
    struct SyntaxToken;
    struct ExpressionSyntax;
    struct SyntaxTree;

    struct Parser
    {

        int _position;
        std::vector<std::shared_ptr<SyntaxToken>> _tokens;
        std::size_t _tokens_size;

        std::stringstream _buffer;

        std::string Errors();
        
        Parser(const std::string& text);

        std::shared_ptr<SyntaxToken> Peek(int offset);

        std::shared_ptr<SyntaxToken> Current();

        std::shared_ptr<SyntaxToken> NextToken();

        std::shared_ptr<SyntaxToken> MatchToken(SyntaxKind kind);

        std::unique_ptr<SyntaxTree> Parse();

        std::unique_ptr<ExpressionSyntax> ParseExpression(int parentPrecedance = 0);

        std::unique_ptr<ExpressionSyntax> ParsePrimaryExpression();

        int GetBinaryOperatorPrecedance(SyntaxKind kind);
        int GetUnaryOperatorPrecedance(SyntaxKind kind);

    };
}