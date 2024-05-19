#include <codeanalysis/Lexer.hpp>
#include <boost/lexical_cast.hpp>
#include <codeanalysis/ExpressionSyntax.hpp> /* SyntaxToken */

namespace trylang
{
    std::string Lexer::Errors()
    {
        return _buffer.str();
    }

    Lexer::Lexer(std::string text) : _text(std::move(text))
    {
        _position = 0;
        _text_size = _text.size();
        _buffer.str("");
    }

    char Lexer::Peek(int offset)
    {
        int index = _position + offset;
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

        if(text == "var")
        {
            return SyntaxKind::VarKeyword;
        }

        if(text == "let")
        {
            return SyntaxKind::LetKeyword;
        }

        if(text == "if")
        {
            return SyntaxKind::IfKeyword;
        }

        if(text == "else")
        {
            return SyntaxKind::ElseKeyword;
        }

        if(text == "while")
        {
            return  SyntaxKind::WhileKeyword;
        }

        if(text == "for")
        {
            return  SyntaxKind::ForKeyword;
        }

        if(text == "to")
        {
            return  SyntaxKind::ToKeyword;
        }

        if(text == "function")
        {
            return SyntaxKind::FunctionKeyword;
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

        if(this->Current() == '{')
        {
            return std::make_unique<SyntaxToken>(SyntaxKind::OpenBraceToken, _position++, "(", std::nullopt);
        }

        if(this->Current() == '}')
        {
            return std::make_unique<SyntaxToken>(SyntaxKind::CloseBraceToken, _position++, ")", std::nullopt);
        }

        if(this->Current() == ':')
        {
            return std::make_unique<SyntaxToken>(SyntaxKind::ColonToken, _position++, ":", std::nullopt);
        }

        if(this->Current() == ',')
        {
            return std::make_unique<SyntaxToken>(SyntaxKind::CommaToken, _position++, ",", std::nullopt);
        }

        if(this->Current() == '"')
        {
            /**
             *
             * "Test "" asd" ---> Test " asd
             */

            ++_position; /* Skip the current quote */

            std::stringstream buffer;

            bool done = false;

            auto saved_position = _position;

            while(!done)
            {
                switch (this->Current())
                {
                    case '\0':
                    case '\r':
                    case '\n':
                        _buffer << "Unterminated String Not Supported\n";
                        done = true;
                        break;
                    case '"':
                        if(this->LookAhead() == '"')
                        {
                            buffer << this->Current();
                            _position += 2;
                        }
                        else
                        {
                            /* Terminating quote */
                            _position++;
                            done = true;
                        }
                        break;
                    default:
                        buffer << this->Current();
                        ++_position;
                        break;
                }
            }

            std::string value = buffer.str();
            return std::make_unique<SyntaxToken>(SyntaxKind::StringToken, saved_position, buffer.str(), std::move(value));
        }

        if(this->Current() == '&')
        {
            if(this->LookAhead() == '&')
            {
                return std::make_unique<SyntaxToken>(SyntaxKind::AmpersandAmpersandToken, _position += 2, "&&", std::nullopt);
            }
        }
        else if(this->Current() == '|')
        {
            if(this->LookAhead() == '|')
            {
                return std::make_unique<SyntaxToken>(SyntaxKind::PipePipeToken, _position += 2, "||", std::nullopt);
            }
        }
        else if(this->Current() == '=')
        {
            if(this->LookAhead() == '=')
            {
                return std::make_unique<SyntaxToken>(SyntaxKind::EqualsEqualsToken, _position += 2, "==", std::nullopt);
            }
            else
            {
                return std::make_unique<SyntaxToken>(SyntaxKind::EqualsToken, _position++, "=", std::nullopt);
            }
        }
        else if(this->Current() == '<')
        {
            if(this->LookAhead() != '=')
            {
                return std::make_unique<SyntaxToken>(SyntaxKind::LessThanToken, _position++, "<", std::nullopt);
            }
            else
            {
                return std::make_unique<SyntaxToken>(SyntaxKind::LessThanEqualsToken, _position += 2, "<=", std::nullopt);
            }
        }
        else if(this->Current() == '>')
        {
            if(this->LookAhead() != '=')
            {
                return std::make_unique<SyntaxToken>(SyntaxKind::GreaterThanToken, _position++, ">", std::nullopt);
            }
            else
            {
                return std::make_unique<SyntaxToken>(SyntaxKind::GreaterThanEqualsToken, _position += 2, ">=", std::nullopt);
            }
        }
        else if(this->Current() == '!')
        {
            if(this->LookAhead() == '=')
            {
                return std::make_unique<SyntaxToken>(SyntaxKind::BangsEqualsToken, _position += 2, "!=", std::nullopt);
            }
            else
            {
                return std::make_unique<SyntaxToken>(SyntaxKind::BangToken, _position++, "!", std::nullopt);
            }
        }

        _buffer << "ERROR: bad character input: '" << this->Current() << "'\n";
        
        _position++;
        return std::make_unique<SyntaxToken>(SyntaxKind::BadToken, _position, _text.substr( _position - 1, 1), std::nullopt);

    }
}