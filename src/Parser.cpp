#include <codeanalysis/Parser.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/Lexer.hpp>
#include <codeanalysis/SyntaxTree.hpp>

namespace trylang
{
    std::string Parser::Errors()
    {
        return _buffer.str();
    }
    
    Parser::Parser(const std::string& text)
    {
        _position = 0;

        Lexer lexer(text);

        auto token = lexer.NextToken();
        while(token->_kind != SyntaxKind::EndOfFileToken)
        {
            if(token->_kind != SyntaxKind::WhitespaceToken && token->_kind != SyntaxKind::BadToken)
            {
                _tokens.emplace_back(std::move(token)); /* unique_ptr are getting converted into shared_ptr */
            }

            token = lexer.NextToken();
        }
        /* Here token->_kind is SyntaxKind::EndOfFileToken. Add that to the vector */
        _tokens.emplace_back(std::move(token));
        _tokens_size = _tokens.size();

        _buffer << lexer.Errors();
    }

    std::shared_ptr<SyntaxToken> Parser::Peek(int offset)
    {
        int index = _position + offset;
        if(index >= _tokens_size)
        {
            return _tokens[_tokens_size - 1];
        }

        return _tokens[index];
    }

    std::shared_ptr<SyntaxToken> Parser::Current()
    {
        return this->Peek(0);
    }

    std::shared_ptr<SyntaxToken> Parser::NextToken()
    {
        std::shared_ptr<SyntaxToken> current = this->Current();
        _position++;
        return current;
    }

    std::shared_ptr<SyntaxToken> Parser::Match(SyntaxKind kind)
    {
        if(this->Current()->_kind == kind)
        {
            return this->NextToken();
        }

        _buffer << "ERROR: Unexpected token <" << this->Current()->_kind <<">, expected <" << kind << ">\n";
        return std::make_shared<SyntaxToken>(kind, this->Current()->_position, "", std::nullopt);
    }

    std::unique_ptr<SyntaxTree> Parser::Parse()
    {
        std::unique_ptr<ExpressionSyntax> expression = this->ParseTerm();

        /* After Parsing we are confirming that the end token is SyntaxKind::EndOfFileToken token*/
        std::shared_ptr<SyntaxToken> endOfFileToken = this->Match(SyntaxKind::EndOfFileToken);

        return std::make_unique<SyntaxTree>(_buffer.str(), std::move(expression), endOfFileToken);
    }

    std::unique_ptr<ExpressionSyntax> Parser::ParseExpression()
    {
        return this->ParseTerm();
    }

    std::unique_ptr<ExpressionSyntax> Parser::ParseTerm()
    {
        auto left = this->ParseFactor();

        while(
                this->Current()->_kind == SyntaxKind::PlusToken || 
                this->Current()->_kind == SyntaxKind::MinusToken
            )
        {
            std::shared_ptr<SyntaxToken> operatorToken = this->NextToken();
            auto right = this->ParseFactor();

            left = std::make_unique<BinaryExpressionSyntax>(std::move(left), operatorToken, std::move(right));
        }

        return left;
    }

    std::unique_ptr<ExpressionSyntax> Parser::ParseFactor()
    {
        auto left = this->ParsePrimaryExpression();

        while( this->Current()->_kind == SyntaxKind::SlashToken ||
                this->Current()->_kind == SyntaxKind::StarToken)
        {
            std::shared_ptr<SyntaxToken> operatorToken = this->NextToken();
            auto right = this->ParsePrimaryExpression();

            left = std::make_unique<BinaryExpressionSyntax>(std::move(left), operatorToken, std::move(right));
        }

        return left;
    }


    std::unique_ptr<ExpressionSyntax> Parser::ParsePrimaryExpression()
    {

        if(this->Current()->_kind == SyntaxKind::OpenParenthesisToken)
        {
            auto openParenthesisToken = this->NextToken();
            auto expression = this->ParseExpression();
            auto closeParenthesisToken = this->Match(SyntaxKind::CloseParenthesisToken);

            return std::make_unique<ParenthesizedExpressionSyntax>(openParenthesisToken, std::move(expression), closeParenthesisToken);
        }

        std::shared_ptr<SyntaxToken> numberToken = this->Match(SyntaxKind::NumberToken);
        return std::make_unique<NumberExpressionSyntax>(numberToken);
    }
}