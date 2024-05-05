#pragma once

#include <string>
#include <memory>
#include <sstream>

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

        Lexer(const std::string& text);

        char Current();

        void Next();

        std::unique_ptr<SyntaxToken> NextToken();
    };
}