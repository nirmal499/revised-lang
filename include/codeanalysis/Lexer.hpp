#pragma once

#include <string>
#include <memory>
#include <sstream>
#include <codeanalysis/SyntaxKind.hpp>

namespace trylang
{
    struct SyntaxToken;
    struct Lexer
    {
        std::string _text;
        std::size_t _text_size;
        int _position;

        std::stringstream _buffer;

        std::string Errors();

        explicit Lexer(std::string text);

        char Current();
        char LookAhead();
        char Peek(int offset);

        void Next();

        std::unique_ptr<SyntaxToken> NextTokenize();
        SyntaxKind GetKeywordKind(const std::string& text);
    };
}