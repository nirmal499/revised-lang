#include "codeanalysis/SyntaxKind.hpp"
#include <cctype>
#include <codeanalysis/Lexer.hpp>
#include <boost/lexical_cast.hpp>
#include <codeanalysis/ExpressionSyntax.hpp> /* SyntaxToken */
#include <memory>
#include <string>
#include <vector>

namespace trylang
{
    std::stringstream Lexer::_buffer; /* Defintion */
    std::string Lexer::Errors()
    {
        return _buffer.str();
    }

    Lexer::Lexer(std::string text) : _text(std::move(text))
    {
        _current = 0;
        _start = 0;
        _line = 1;
        _text_size = _text.size();
        _buffer.str("");
        _tokens = std::vector<std::unique_ptr<SyntaxToken>>();
    }

    bool Lexer::IsAtEnd()
    {
        return _current >= _text_size;
    }

    char Lexer::Peek(int offset)
    {
        int index = _current + offset;
        if(index >= _text_size)
        {
            return '\0';
        }

        return _text.at(index);
    }

    char Lexer::Current()
    {
       return this->Peek(0);
    }

    char Lexer::LookAhead()
    {
        return this->Peek(1);
    }

    bool Lexer::Match(char expected)
    {
        if(this->IsAtEnd() || this->Current() != expected)
        {
            return false;
        }

        _current++;
        return true;
    }

    char Lexer::Advance()
    {
        return _text.at(_current++);
    }

    void Lexer::AddToken(SyntaxKind kind)
    {
        this->AddToken(kind, std::nullopt);
    }

    void Lexer::AddToken(SyntaxKind kind, object_t&& value)
    {
        int length = _current - _start;
        std::string extracted_lexeme = _text.substr(_start, length);
        _tokens.emplace_back(std::make_unique<SyntaxToken>(kind, _line, std::move(extracted_lexeme), std::move(value)));
    }

    void Lexer::GenerateError(std::string message)
    {
        _buffer << "[line " << _line << "] Error: " << message << "\n";
    }

    void Lexer::ReadStringLiteral()
    {
        /*
            We consume characters until we hit the closing " that ends
            the string. We also handle running out of input before the string literal
            is closed and report an error for that.
        */
        while(this->Current() != '"' && !this->IsAtEnd())
        {
            /* We support multi-line string literals */
            if(this->Current() == '\n')
            {
                _line++;
            }

            this->Advance();
        }

        if(this->IsAtEnd())
        {
            this->GenerateError("Unterminated String");
            return;
        }

        /* Consume the closing " */
        this->Advance();

        /* Trim the staring quote and ending quote */
        int length = (_current - 1) - (_start + 1);
        std::string value = _text.substr(_start + 1, length);
        this->AddToken(SyntaxKind::StringToken, value);
    }

    void Lexer::ReadIdentifier()
    {
        while(std::isalnum(this->Current()))
        {
            this->Advance();
        }

        int length = _current - _start;
        std::string text = _text.substr(_start, length);
        SyntaxKind kind;

        auto it = _keywords.find(text);
        if(it != _keywords.end())
        {
            kind = it->second;
        }
        else
        {
            kind = SyntaxKind::IdentifierToken;
        }

        this->AddToken(kind, text);
    }

    void Lexer::ReadNumberLiteral()
    {
        /* Till now int is unsupported */
        while(std::isdigit(this->Current()))
        {
            /* m_current is changed to point to next character but the m_start is at the start of the number lexeme */
            this->Advance();
        }

        int length = _current - _start;
        auto text = _text.substr(_start, length);
            
        int value = -1;
        try
        {
            value = boost::lexical_cast<int>(text);
        }
        catch(const boost::bad_lexical_cast&)
        {
            this->GenerateError("The number '" + text + "' cannot be represented by an Int32");
        }

        this->AddToken(SyntaxKind::NumberToken, value);
    }

    void Lexer::ScanToken()
    {
        /* returns the current pointing character and advances to next character */
        char ch = this->Advance();

        switch (ch)
        {
            case '(': this->AddToken(SyntaxKind::OpenParenthesisToken); break;
            case ')': this->AddToken(SyntaxKind::CloseParenthesisToken); break;
            case '{': this->AddToken(SyntaxKind::OpenBraceToken); break;
            case '}': this->AddToken(SyntaxKind::CloseBraceToken); break;
            case ',': this->AddToken(SyntaxKind::CommaToken); break;
            case ':': this->AddToken(SyntaxKind::ColonToken); break;
            case '-': this->AddToken(SyntaxKind::MinusToken); break;
            case '+': this->AddToken(SyntaxKind::PlusToken); break;
            case ';': this->AddToken(SyntaxKind::SemicolonToken); break;
            case '*': this->AddToken(SyntaxKind::StarToken); break;
            case '!':
                this->AddToken(this->Match('=') ? SyntaxKind::BangsEqualsToken : SyntaxKind::BangToken);
                break;
            case '=': 
                this->AddToken(this->Match('=') ? SyntaxKind::EqualsEqualsToken : SyntaxKind::EqualsToken);
                break;
            case '<':
                this->AddToken(this->Match('=') ? SyntaxKind::LessThanEqualsToken : SyntaxKind::LessThanToken);
                break;
            case '>':
                this->AddToken(this->Match('=') ? SyntaxKind::GreaterThanEqualsToken : SyntaxKind::GreaterThanToken);
                break;
            case '&':
                {
                    if(this->Match('&'))
                    {
                        this->AddToken(SyntaxKind::AmpersandAmpersandToken);
                    }
                }
                break;
            case '|':
                {
                    if(this->Match('|'))
                    {
                        this->AddToken(SyntaxKind::PipePipeToken);
                    }
                }
                break;
            case '/':
                {
                    if(this->Match('/'))
                    {
                        /* A comment goes until the end of the line */
                        while(this->Current() != '\n' && !this->IsAtEnd())
                        {
                            this->Advance();
                        }
                    }
                    else
                    {
                        this->AddToken(SyntaxKind::SlashToken);
                    }
                }
                break;
            case ' ':
            case '\r':
            case '\t':
                /* 
                    Ignore whitespace
                    When encountering whitespace, we simply go back to the beginning of the scan loop
                    We dumped the returned consumed character by advance()
                */
                break;
            case '\n':
                _line++;
                break;
            case '"':
                this->ReadStringLiteral();
                break;
            default:
                if(std::isdigit(ch))
                {
                    this->ReadNumberLiteral();
                }
                else if(std::isalnum(ch))
                {
                    this->ReadIdentifier();
                }
                else
                {
                    this->GenerateError("Unexpected character.");
                    this->AddToken(SyntaxKind::BadToken);
                }
                break;
        }
    }

    std::vector<std::unique_ptr<SyntaxToken>> Lexer::Tokenize()
    {
        while(!this->IsAtEnd())
        {
            _start = _current;
            /* 
                scan_token() is called for each character
                and it adds token to the m_tokens using the
                m_start and m_current

                After adding token each time we set m_start to point to the next lexeme
            */
            this->ScanToken();
        }

        _tokens.emplace_back(std::make_unique<SyntaxToken>(SyntaxKind::EndOfFileToken, _current, "\0", std::nullopt));

        return std::move(_tokens);
    }

    std::vector<std::unique_ptr<SyntaxToken>> Lexer::Tokenizer(std::string&& text)
    {
        Lexer lexer(std::move(text));

        auto tokens = lexer.Tokenize();
        if(!Lexer::Errors().empty())
        {
            std::cout << "Tokenization Errors Reported:\n";
            std::cout << "\n" << Lexer::Errors() << "\n";
        }

        return tokens;
    }
}