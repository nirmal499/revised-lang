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

        _buffer << "ERROR: Unexpected token <" << trylang::__syntaxStringMap[this->Current()->_kind] <<">, expected <" << trylang::__syntaxStringMap[kind] << ">\n";
        // return std::make_shared<SyntaxToken>(kind, this->Current()->_position, "", std::nullopt);

        throw std::logic_error("ERROR: Unexpected token <" + trylang::__syntaxStringMap[this->Current()->_kind] + ">, expected <" + trylang::__syntaxStringMap[kind] + ">");
    }


    std::unique_ptr<CompilationUnitSyntax> Parser::ParseCompilationUnit()
    {
        auto statement = this->ParseStatement();

        /* After Parsing we are confirming that the end token is SyntaxKind::EndOfFileToken token*/
        std::shared_ptr<SyntaxToken> endOfFileToken = this->MatchToken(SyntaxKind::EndOfFileToken);

        return std::make_unique<CompilationUnitSyntax>(std::move(statement), endOfFileToken);
    }

    std::unique_ptr<StatementSyntax> Parser::ParseStatement()
    {
        if(this->Current()->Kind() == SyntaxKind::OpenBraceToken)
        {
            return this->ParseBlockStatement();
        }
        else if(this->Current()->Kind() == SyntaxKind::VarKeyword || this->Current()->Kind() == SyntaxKind::LetKeyword)
        {
            return this->ParseVariableDeclaration();
        }
        else if(this->Current()->Kind() == SyntaxKind::IfKeyword)
        {
            return this->ParseIfStatement();
        }
        else if(this->Current()->Kind() == SyntaxKind::WhileKeyword)
        {
            return this->ParseWhileStatement();
        }
        else if(this->Current()->Kind() == SyntaxKind::ForKeyword)
        {
            return this->ParseForStatement();
        }

        return this->ParseExpressionStatement();
    }

    std::unique_ptr<StatementSyntax> Parser::ParseIfStatement()
    {
        auto keyword = this->MatchToken(SyntaxKind::IfKeyword);
        auto condition = this->ParseExpression();
        auto statement = this->ParseStatement();
        auto elseClause = this->ParseElseClause();

        return std::make_unique<IfStatementSyntax>(keyword, std::move(condition), std::move(statement), std::move(elseClause));
    }

    std::unique_ptr<StatementSyntax> Parser::ParseElseClause()
    {
        if(this->Current()->Kind() != SyntaxKind::ElseKeyword)
        {
            return nullptr;
        }

        auto keyword = this->NextToken();
        auto statement = this->ParseStatement();

        return std::make_unique<ElseClauseSyntax>(keyword, std::move(statement));
    }


    std::unique_ptr<StatementSyntax> Parser::ParseVariableDeclaration()
    {
        auto expected = this->Current()->Kind() == SyntaxKind::VarKeyword ? SyntaxKind::VarKeyword : SyntaxKind::LetKeyword;
        auto keyword = this->MatchToken(expected);
        auto identifier = this->MatchToken(SyntaxKind::IdentifierToken);
        auto equalsToken = this->MatchToken(SyntaxKind::EqualsToken);
        auto initializer = this->ParseExpression();

        return std::make_unique<VariableDeclarationSyntax>(keyword, identifier, equalsToken, std::move(initializer));
    }

    std::unique_ptr<StatementSyntax> Parser::ParseBlockStatement()
    {
        std::vector<std::unique_ptr<StatementSyntax>> statements;

        auto openBraceToken = this->MatchToken(SyntaxKind::OpenBraceToken);

        while(this->Current()->Kind() != SyntaxKind::EndOfFileToken && this->Current()->Kind() != SyntaxKind::CloseBraceToken)
        {
            auto statement = this->ParseStatement();
            statements.emplace_back(std::move(statement));
        }

        auto closeBraceToken = this->MatchToken(SyntaxKind::CloseBraceToken);

        return std::make_unique<BlockStatementSyntax>(openBraceToken, std::move(statements), closeBraceToken);
    }

    std::unique_ptr<StatementSyntax> Parser::ParseExpressionStatement()
    {
        auto expression = this->ParseExpression();
        return std::make_unique<ExpressionStatementSyntax>(std::move(expression));
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
        case SyntaxKind::LessThanToken:
        case SyntaxKind::LessThanEqualsToken:
        case SyntaxKind::GreaterThanToken:
        case SyntaxKind::GreaterThanEqualsToken:
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

    std::unique_ptr<StatementSyntax> Parser::ParseWhileStatement()
    {
        auto keyword = this->MatchToken(SyntaxKind::WhileKeyword);
        auto condition = this->ParseExpression();
        auto body = this->ParseStatement();

        return std::make_unique<WhileStatementSyntax>(keyword, std::move(condition), std::move(body));

    }

    std::unique_ptr<StatementSyntax> Parser::ParseForStatement()
    {
        auto keyword = this->MatchToken(SyntaxKind::ForKeyword);
        auto identifier = this->MatchToken(SyntaxKind::IdentifierToken);
        auto equalsToken = this->MatchToken(SyntaxKind::EqualsToken);

        auto lowerBound = this->ParseExpression();

        auto toKeyword = this->MatchToken(SyntaxKind::ToKeyword);

        auto upperBound = this->ParseExpression();
        auto body = this->ParseStatement();

        return std::make_unique<ForStatementSyntax>(keyword, identifier, equalsToken, std::move(lowerBound), toKeyword,std::move(upperBound), std::move(body));
    }
}