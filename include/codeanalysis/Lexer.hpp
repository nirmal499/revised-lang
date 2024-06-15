#pragma once

#include <string>
#include <memory>
#include <sstream>
#include <codeanalysis/SyntaxKind.hpp>
#include <codeanalysis/Types.hpp>
#include <vector>

namespace trylang
{
    struct SyntaxToken;
        

    struct Lexer
    {
        std::string _text;
        std::size_t _text_size;
        int _line;
        int _start;                     /* points to the first character in the lexeme being scanned */
        int _current;                   /* points to the character currently being considered */
        static std::stringstream _buffer;      /* stores the errors */


        std::vector<std::unique_ptr<SyntaxToken>> _tokens; /* It will be moved from Tokenize() */

        std::unordered_map<std::string, SyntaxKind> _keywords = {
            {"true", SyntaxKind::TrueKeyword},
            {"false", SyntaxKind::FalseKeyword},
            {"var", SyntaxKind::VarKeyword},
            {"let", SyntaxKind::LetKeyword},
            {"if", SyntaxKind::IfKeyword},
            {"else", SyntaxKind::ElseKeyword},
            {"while", SyntaxKind::WhileKeyword},
            {"for", SyntaxKind::ForKeyword},
            {"to", SyntaxKind::ToKeyword},
            {"function", SyntaxKind::FunctionKeyword},
            {"break", SyntaxKind::BreakKeyword},
            {"continue", SyntaxKind::ContinueKeyword},
            {"return", SyntaxKind::ReturnKeyword},
        };

        static std::string Errors();
        explicit Lexer(std::string text);
        std::vector<std::unique_ptr<SyntaxToken>> Tokenize();
        void ScanToken();

        char Current();
        char LookAhead();
        char Peek(int offset);
        char Advance();
        bool IsAtEnd();
        bool Match(char expected);

        void AddToken(SyntaxKind kind);
        void AddToken(SyntaxKind kind, object_t&& value);

        void ReadIdentifier();
        void ReadStringLiteral();
        void ReadNumberLiteral();
        void GenerateError(std::string message);

        static std::vector<std::unique_ptr<SyntaxToken>> Tokenizer(std::string&& text);

    };
}