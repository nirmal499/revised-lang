#include <codeanalysis/Lexer.hpp>
#include <boost/lexical_cast.hpp>
#include <codeanalysis/ExpressionSyntax.hpp> /* SyntaxToken */

namespace trylang
{
    std::string Lexer::Errors()
    {
        return _buffer.str();
    }

    Lexer::Lexer(const std::string& text) : _text(text)
    {
        _position = 0;
        _text_size = _text.size();
        _buffer.str("");
    }

    char Lexer::Current()
    {
        if(_position >= _text_size)
        {
            return '\0';
        }

        return _text.at(_position);
    }

    void Lexer::Next()
    {
        _position++;
    }

    SyntaxKind Lexer::GetKeywordKind(const std::string& text)
    {
        if(text == "true")
        {
            return SyntaxKind::TrueKeyword;
        }
        
        if(text == "false")
        {
            return SyntaxKind::FalseKeyword;
        }

        return SyntaxKind::IdentifierToken;
    }

    std::unique_ptr<SyntaxToken> Lexer::NextTokenize()
    {

        if(_position >= _text_size)
        {
            /* _position points to '\0' */
            return std::make_unique<SyntaxToken>(SyntaxKind::EndOfFileToken, _position, "\0", std::nullopt);
        }

        if(std::isdigit(this->Current()))
        {
            int start = _position;

            while(std::isdigit(this->Current()))
            {
                this->Next();
            }

            int length = _position - start;
            auto text = _text.substr(start, length);
            
            int value = 0;
            try
            {
                value = boost::lexical_cast<int>(text);
            }
            catch(const boost::bad_lexical_cast&)
            {
                _buffer << "The number '" << text << "' cannot be represented by an Int32\n";
            }
            
            return std::make_unique<SyntaxToken>(SyntaxKind::NumberToken, start, std::move(text), value);
        }

        if(std::isspace(this->Current()))
        {
            int start = _position;

            while(std::isspace(this->Current()))
            {
                this->Next();
            }

            int length = _position - start;
            auto text = _text.substr(start, length);
            return std::make_unique<SyntaxToken>(SyntaxKind::WhitespaceToken, start, std::move(text), std::nullopt);
        }

        if(std::isalpha(this->Current()))
        {
            int start = _position;

            while(std::isalpha(this->Current()))
            {
                this->Next();
            }

            int length = _position - start;
            auto text = _text.substr(start, length);
            auto kind = this->GetKeywordKind(text);
            return std::make_unique<SyntaxToken>(kind, start, std::move(text), std::nullopt);
        }

        if(this->Current() == '+')
        {
            return std::make_unique<SyntaxToken>(SyntaxKind::PlusToken, _position++, "+", std::nullopt);
        }
        
        if(this->Current() == '-')
        {
            return std::make_unique<SyntaxToken>(SyntaxKind::MinusToken, _position++, "-", std::nullopt);
        }
        
        if(this->Current() == '*')
        {
            return std::make_unique<SyntaxToken>(SyntaxKind::StarToken, _position++, "*", std::nullopt);
        }
        
        if(this->Current() == '/')
        {
            return std::make_unique<SyntaxToken>(SyntaxKind::SlashToken, _position++, "/", std::nullopt);
        }
        
        if(this->Current() == '(')
        {
            return std::make_unique<SyntaxToken>(SyntaxKind::OpenParenthesisToken, _position++, "(", std::nullopt);
        }
        
        if(this->Current() == ')')
        {
            return std::make_unique<SyntaxToken>(SyntaxKind::CloseParenthesisToken, _position++, ")", std::nullopt);
        }

        _buffer << "ERROR: bad character input: '" << this->Current() << "'\n";
        
        _position++;
        return std::make_unique<SyntaxToken>(SyntaxKind::BadToken, _position, _text.substr( _position - 1, 1), std::nullopt);

    }
}