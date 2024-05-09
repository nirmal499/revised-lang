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
    
    Parser::Parser(std::string text)
    {
        _position = 0;

        Lexer lexer(std::move(text));

        auto token = lexer.NextTokenize();
        while(token->_kind != SyntaxKind::EndOfFileToken)
        {
            if(token->_kind != SyntaxKind::WhitespaceToken && token->_kind != SyntaxKind::BadToken)
            {
                _tokens.emplace_back(std::move(token)); /* unique_ptr are getting converted into shared_ptr */
            }

            token = lexer.NextTokenize();
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

    std::shared_ptr<SyntaxToken> Parser::MatchToken(SyntaxKind kind)
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
        std::unique_ptr<ExpressionSyntax> expression = this->ParseExpression();

        /* After Parsing we are confirming that the end token is SyntaxKind::EndOfFileToken token*/
        std::shared_ptr<SyntaxToken> endOfFileToken = this->MatchToken(SyntaxKind::EndOfFileToken);

        return std::make_unique<SyntaxTree>(_buffer.str(), std::move(expression), endOfFileToken);
    }

    std::unique_ptr<ExpressionSyntax> Parser::ParseExpression()
    {
        return this->ParseAssignmentExpression();
    }

    /* This will take care of `a = b = 5` */
    std::unique_ptr<ExpressionSyntax> Parser::ParseAssignmentExpression()
    {
        if(this->Peek(0)->Kind() == SyntaxKind::IdentifierToken && this->Peek(1)->Kind() == SyntaxKind::EqualsToken)
        {
            auto identifierToken = this->NextToken();
            auto operatorToken = this->NextToken();
            auto right = this->ParseAssignmentExpression();

            return std::make_unique<AssignmentExpressionSyntax>(identifierToken, operatorToken, std::move(right));
        }

        return this->ParseBinaryExpression();
    }

    std::unique_ptr<ExpressionSyntax> Parser::ParseBinaryExpression(int parentPrecedance)
    {
        std::unique_ptr<ExpressionSyntax> left;

        auto unaryOperatorPrecedance = this->GetUnaryOperatorPrecedance(this->Current()->Kind());
        if(unaryOperatorPrecedance != 0 && unaryOperatorPrecedance >= parentPrecedance)
        {
            auto operatorToken = this->NextToken();
            auto operand = this->ParseBinaryExpression();
            left = std::make_unique<UnaryExpressionSyntax>(operatorToken, std::move(operand));
        }
        else
        {
            left = this->ParsePrimaryExpression();
        }

        while(true)
        {
            auto precedance = this->GetBinaryOperatorPrecedance(this->Current()->Kind());
            if(precedance == 0 || precedance <= parentPrecedance)
            {
                break;
            }
            auto operatorToken = this->NextToken();
            auto right = this->ParseBinaryExpression(precedance);
            left = std::make_unique<BinaryExpressionSyntax>(std::move(left), operatorToken, std::move(right));
        }

        return left;
    }

    int Parser::GetUnaryOperatorPrecedance(SyntaxKind kind)
    {
        switch (kind)
        {
        case SyntaxKind::PlusToken:
        case SyntaxKind::MinusToken:
        case SyntaxKind::BangToken:
            return 6;
        
        default:
            return 0;
        }
    }

    int Parser::GetBinaryOperatorPrecedance(SyntaxKind kind)
    {
        switch (kind)
        {
        case SyntaxKind::StarToken:
        case SyntaxKind::SlashToken:
            return 5;
        case SyntaxKind::PlusToken:
        case SyntaxKind::MinusToken:
            return 4;

        case SyntaxKind::EqualsEqualsToken:
        case SyntaxKind::BangsEqualsToken:
            return 3;

        case SyntaxKind::AmpersandAmpersandToken:
            return 2;
        case SyntaxKind::PipePipeToken:
            return 1;
        default:
            return 0;
        }
    }

    std::unique_ptr<ExpressionSyntax> Parser::ParsePrimaryExpression()
    {

        if(this->Current()->_kind == SyntaxKind::OpenParenthesisToken)
        {
            auto openParenthesisToken = this->NextToken();
            auto expression = this->ParseExpression();
            auto closeParenthesisToken = this->MatchToken(SyntaxKind::CloseParenthesisToken);

            return std::make_unique<ParenthesizedExpressionSyntax>(openParenthesisToken, std::move(expression), closeParenthesisToken);
        }
        else if(
            this->Current()->Kind() == SyntaxKind::TrueKeyword ||
            this->Current()->Kind() == SyntaxKind::FalseKeyword
        )
        {
            auto keywordToken = this->NextToken();
            auto value = keywordToken->Kind() == SyntaxKind::TrueKeyword;
            return std::make_unique<LiteralExpressionSyntax>(keywordToken, value);
        }
        else if(this->Current()->Kind() == SyntaxKind::IdentifierToken)
        {
            auto identifierToken = this->NextToken();
            return std::make_unique<NameExpressionSyntax>(identifierToken);
        }

        std::shared_ptr<SyntaxToken> numberToken = this->MatchToken(SyntaxKind::NumberToken);
        return std::make_unique<LiteralExpressionSyntax>(numberToken);
    }
}