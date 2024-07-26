#include <codeanalysis/SyntaxKind.hpp>
#include <codeanalysis/Parser.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/Lexer.hpp>
#include <codeanalysis/SyntaxTree.hpp>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace trylang
{
    std::stringstream Parser::_buffer; /* Definition */
    std::string Parser::Errors()
    {
        return _buffer.str();
    }

    bool Parser::IsAtEnd()
    {
        return this->Peek(0)->Kind() == SyntaxKind::EndOfFileToken;
    }

    Parser::Parser(std::vector<std::unique_ptr<SyntaxToken>>&& tokens) : _tokens(std::move(tokens))
    {
        _tokens_size = _tokens.size();
        _buffer.str("");
        _current = 0;
        _statements = std::vector<std::unique_ptr<StatementSyntax>>();
    }

    std::unique_ptr<CompilationUnitSyntax> Parser::AST(std::vector<std::unique_ptr<SyntaxToken>>&& tokens)
    {
        Parser parser(std::move(tokens));

        std::unique_ptr<CompilationUnitSyntax> compilationSyntax = parser.Parse();
        
        if(!Parser::Errors().empty())
        {
            std::cout << "Parsing Errors Reported:\n";
            std::cout << "\n" << Parser::Errors() << "\n";
            return nullptr;
        }

        return compilationSyntax;
    }

    SyntaxToken* Parser::Peek(int offset)
    {
        int index = _current + offset;
        if(index >= _tokens_size)
        {
            return _tokens[_tokens_size - 1].get();
        }

        return _tokens[index].get();
    }

    SyntaxToken* Parser::Current()
    {
        return this->Peek(0);
    }

    std::unique_ptr<SyntaxToken> Parser::Advance()
    {
        _current++;
        return std::move(_tokens.at(_current - 1));
    }

    std::unique_ptr<SyntaxToken> Parser::Consume(SyntaxKind kind, std::string message)
    {
        if(this->Check(kind))
        {
            return this->Advance();
        }

        this->Error(this->Current(), message);

        return nullptr; // Unreachable
    }

    void Parser::Error(SyntaxToken* token, std::string message)
    {
        if(token->Kind() == SyntaxKind::EndOfFileToken)
        {
            this->GenerateError(token->_line, " at end: " + message + " | Instead got " + trylang::__syntaxStringMap[this->Current()->Kind()]);
        }
        else
        {
            this->GenerateError(token->_line, " at '" + token->_text + "' " + message + " | Instead got " + trylang::__syntaxStringMap[this->Current()->Kind()]);
        }

        throw std::runtime_error("Throwing Exception In Parsing.");
    }

    void Parser::GenerateError(int line, std::string message)
    {
        _buffer << "[line " << line << "] Error : " << message << "\n";
    }

    bool Parser::Check(SyntaxKind kind)
    {
        if(this->IsAtEnd())
        {
            return false;
        }

        return this->Peek(0)->Kind() == kind;
    }

    std::unique_ptr<CompilationUnitSyntax> Parser::Parse()
    {
        while(!this->IsAtEnd())
        {
            _statements.emplace_back(this->ParseDeclaration());
        }

        return std::make_unique<CompilationUnitSyntax>(std::move(_statements));
    }

    std::unique_ptr<StatementSyntax> Parser::ParseDeclaration()
    {
        try
        {
            if(this->Current()->Kind() == SyntaxKind::FunctionKeyword)
            {
                return this->ParseFunctionDeclarationStatement();
            }

            return this->ParseStatement();
        }
        catch(const std::exception& e)
        {
            /* Skip the statement which caused the exception */
            this->SynchronizeAfterAnExpectionForInvalidTokenMatch();
            return nullptr;
        }
    }

    std::unique_ptr<StatementSyntax> Parser::ParseFunctionDeclarationStatement()
    {
        auto functionKeyword = this->Consume(SyntaxKind::FunctionKeyword, "Expected 'function' keyword");
        auto identifier = this->Consume(SyntaxKind::IdentifierToken, "Expected a function name");
        auto openParenthesis = this->Consume(SyntaxKind::OpenParenthesisToken, "Expected a '('");
        auto parameters = this->ParseParameterList();
        auto closeParenthesis = this->Consume(SyntaxKind::CloseParenthesisToken, "Expected a ')'");
        auto typeClause = this->ParseOptionalTypeClause(); /* return type of the function */
        auto body = this->ParseBlockStatement(true);

        return std::make_unique<FunctionDeclarationStatementSyntax>(std::move(functionKeyword), std::move(identifier), std::move(openParenthesis), std::move(parameters), std::move(closeParenthesis), std::move(typeClause), std::move(body));
    }

    std::vector<std::unique_ptr<ParameterSyntax>> Parser::ParseParameterList()
    {
        std::vector<std::unique_ptr<ParameterSyntax>> parameters;

        while(this->Current()->Kind() != SyntaxKind::CloseParenthesisToken && this->Current()->Kind() != SyntaxKind::EndOfFileToken)
        {
            auto parameter = this->ParseParameter();
            parameters.emplace_back(std::move(parameter));

            if(this->Current()->Kind() != SyntaxKind::CloseParenthesisToken)
            {
                auto commaToken = this->Consume(SyntaxKind::CommaToken, "Expected a ','.");
                // parameters.emplace_back(std::move(commaToken));
            }
        }

        return parameters; // RVO
    }

    std::unique_ptr<ParameterSyntax> Parser::ParseParameter()
    {
        auto identifier = this->Consume(SyntaxKind::IdentifierToken, "Expected a parameter name");
        auto typeClause = this->ParseTypeClause(); /* typeClause is not optional */
        return std::make_unique<ParameterSyntax>(std::move(identifier), std::move(typeClause));
    }

    std::unique_ptr<StatementSyntax> Parser::ParseVariableDeclarationStatement()
    {
        auto expected = this->Current()->Kind() == SyntaxKind::VarKeyword ? SyntaxKind::VarKeyword : SyntaxKind::LetKeyword;
        auto keyword = this->Consume(expected, "Expected 'var' or 'let'.");
        auto identifier = this->Consume(SyntaxKind::IdentifierToken, "Expected a variable name.");
        auto typeClause = this->ParseOptionalTypeClause();
        auto equalsToken = this->Consume(SyntaxKind::EqualsToken, "Expected a '='.");
        auto initializer = this->ParseExpression();

        (void)this->Consume(SyntaxKind::SemicolonToken, "Expected a ';'.");

        return std::make_unique<VariableDeclarationStatementSyntax>(std::move(keyword), std::move(identifier), std::move(typeClause) ,std::move(equalsToken), std::move(initializer));
    }

    std::unique_ptr<TypeClauseSyntax> Parser::ParseOptionalTypeClause()
    {
        if(this->Current()->Kind() != SyntaxKind::ColonToken)
        {
            return nullptr;
        }
        return this->ParseTypeClause();
    }

    std::unique_ptr<TypeClauseSyntax> Parser::ParseTypeClause()
    {
        auto colonToken = this->Consume(SyntaxKind::ColonToken, "Expected ':' here.");
        auto identifierToken = this->Consume(SyntaxKind::IdentifierToken, "Expected a type name.");

        return std::make_unique<TypeClauseSyntax>(std::move(colonToken), std::move(identifierToken));
    }

    void Parser::SynchronizeAfterAnExpectionForInvalidTokenMatch()
    {
        _current++; /* Advance to NextToken */
        while(this->Current()->Kind() != SyntaxKind::EndOfFileToken)
        {
            if(this->Current()->Kind() == SyntaxKind::SemicolonToken)
            {
                /* 
                    We have reached the end of the statement which caused an exception. 
                    So we return inorder to parse the other statements after this error statements
                */
                return;
            }

            _current++; /* Advance to NextToken */
        }
    }

    std::unique_ptr<StatementSyntax> Parser::ParseStatement()
    {
        switch (this->Current()->Kind())
        {
            case SyntaxKind::OpenBraceToken: return this->ParseBlockStatement(); break;
            case SyntaxKind::IfKeyword: return this->ParseIfStatement(); break;
            case SyntaxKind::WhileKeyword: return this->ParseWhileStatement(); break;
            case SyntaxKind::ForKeyword: return this->ParseForStatement(); break;
            case SyntaxKind::BreakKeyword: return this->ParseBreakStatement(); break;
            case SyntaxKind::ContinueKeyword: return this->ParseContinueStatement(); break;
            case SyntaxKind::ReturnKeyword: return this->ParseReturnStatement(); break;
            case SyntaxKind::VarKeyword:
            case SyntaxKind::LetKeyword:
                return this->ParseVariableDeclarationStatement(); break;

            default: return this->ParseExpressionStatement(); break;
        }
    }

    std::unique_ptr<StatementSyntax> Parser::ParseIfStatement()
    {
        auto keyword = this->Consume(SyntaxKind::IfKeyword, "Expected 'If' keyword.");
        (void)this->Consume(SyntaxKind::OpenParenthesisToken, "Expected a '('.");
        auto condition = this->ParseExpression();
        (void)this->Consume(SyntaxKind::CloseParenthesisToken, "Expected a ')'.");
        auto statement = this->ParseStatement(); /* {} */
        auto elseClause = this->ParseElseClause();

        return std::make_unique<IfStatementSyntax>(std::move(keyword), std::move(condition), std::move(statement), std::move(elseClause));
    }

    std::unique_ptr<StatementSyntax> Parser::ParseElseClause()
    {
        if(this->Current()->Kind() != SyntaxKind::ElseKeyword)
        {
            return nullptr;
        }

        auto keyword = this->Consume(SyntaxKind::ElseKeyword, "Expected 'else' keyword.");
        auto statement = this->ParseStatement();

        return std::make_unique<ElseStatementSyntax>(std::move(keyword), std::move(statement));
    }


    std::unique_ptr<StatementSyntax> Parser::ParseBlockStatement(bool isFunction)
    {
        std::vector<std::unique_ptr<StatementSyntax>> statements;

        auto openBraceToken = this->Consume(SyntaxKind::OpenBraceToken, "Expected '{'.");

        /* !(this->Current()->Kind() == SyntaxKind::EndOfFileToken || this->Current()->Kind() == SyntaxKind::CloseBraceToken) */
        while(this->Current()->Kind() != SyntaxKind::EndOfFileToken && this->Current()->Kind() != SyntaxKind::CloseBraceToken)
        {
            auto statement = this->ParseStatement();
            statements.emplace_back(std::move(statement));
        }

        if(isFunction && statements[statements.size() - 1]->Kind() != SyntaxKind::ReturnStatement)
        {
            _buffer << "The last statement of a function must be a return statement\n";
        }

        auto closeBraceToken = this->Consume(SyntaxKind::CloseBraceToken, "Expected '}'.");

        return std::make_unique<BlockStatementSyntax>(std::move(openBraceToken), std::move(statements), std::move(closeBraceToken));
    }

    std::unique_ptr<StatementSyntax> Parser::ParseExpressionStatement()
    {
        auto expression = this->ParseExpression();
        (void)this->Consume(SyntaxKind::SemicolonToken, "Expected a ';'.");
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
            auto identifierToken = this->Advance();
            auto operatorToken = this->Advance();
            auto right = this->ParseAssignmentExpression();

            return std::make_unique<AssignmentExpressionSyntax>(std::move(identifierToken), std::move(operatorToken), std::move(right));
        }

        return this->ParseLogicalOrExpression();
    }

    std::unique_ptr<ExpressionSyntax> Parser::ParseLogicalOrExpression()
    {
        auto expr = this->ParseLogicalAndExpression();

        while(this->Current()->Kind() == SyntaxKind::PipePipeToken)
        {
            auto op = this->Advance();
            auto right = this->ParseLogicalAndExpression();

            expr = std::make_unique<BinaryExpressionSyntax>(std::move(expr), std::move(op), std::move(right));
        }

        return expr;
    }

    std::unique_ptr<ExpressionSyntax> Parser::ParseLogicalAndExpression()
    {
        auto expr = this->ParseEqualityExpression();

        while(this->Current()->Kind() == SyntaxKind::AmpersandAmpersandToken)
        {
            auto op = this->Advance();
            auto right = this->ParseEqualityExpression();

            expr = std::make_unique<BinaryExpressionSyntax>(std::move(expr), std::move(op), std::move(right));
        }

        return expr;
    }

    std::unique_ptr<ExpressionSyntax> Parser::ParseEqualityExpression()
    {
        auto expr = this->ParseComparisonExpression();
        
        while(this->Current()->Kind() == SyntaxKind::BangsEqualsToken || this->Current()->Kind() == SyntaxKind::EqualsEqualsToken)
        {
            auto op = this->Advance();
            auto right = this->ParseComparisonExpression();

            expr = std::make_unique<BinaryExpressionSyntax>(std::move(expr), std::move(op), std::move(right));
        }

        return expr;
    }

    std::unique_ptr<ExpressionSyntax> Parser::ParseComparisonExpression()
    {
        auto expr = this->ParseTermExpression();
        
        while(this->Current()->Kind() == SyntaxKind::GreaterThanToken || this->Current()->Kind() == SyntaxKind::GreaterThanEqualsToken || this->Current()->Kind() == SyntaxKind::LessThanToken || this->Current()->Kind() == SyntaxKind::LessThanEqualsToken)
        {
            auto op = this->Advance();
            auto right = this->ParseTermExpression();

            expr = std::make_unique<BinaryExpressionSyntax>(std::move(expr), std::move(op), std::move(right));
        }

        return expr;
    }

    std::unique_ptr<ExpressionSyntax> Parser::ParseTermExpression()
    {
        auto expr = this->ParseFactorExpression();
        
        while(this->Current()->Kind() == SyntaxKind::PlusToken || this->Current()->Kind() == SyntaxKind::MinusToken)
        {
            auto op = this->Advance();
            auto right = this->ParseFactorExpression();

            expr = std::make_unique<BinaryExpressionSyntax>(std::move(expr), std::move(op), std::move(right));
        }

        return expr;
    }

    std::unique_ptr<ExpressionSyntax> Parser::ParseFactorExpression()
    {
        auto expr = this->ParseUnaryExpression();
        
        while(this->Current()->Kind() == SyntaxKind::StarToken || this->Current()->Kind() == SyntaxKind::SlashToken)
        {
            auto op = this->Advance();
            auto right = this->ParseUnaryExpression();

            expr = std::make_unique<BinaryExpressionSyntax>(std::move(expr), std::move(op), std::move(right));
        }

        return expr;
    }

    std::unique_ptr<ExpressionSyntax> Parser::ParseUnaryExpression()
    {
        if(this->Current()->Kind() == SyntaxKind::BangToken || this->Current()->Kind() == SyntaxKind::PlusToken || this->Current()->Kind() == SyntaxKind::MinusToken)
        {
            auto op = this->Advance();
            auto right = this->ParseUnaryExpression();

            return std::make_unique<UnaryExpressionSyntax>(std::move(op), std::move(right));
        }

        return this->ParsePrimaryExpression();
    }

    std::unique_ptr<ExpressionSyntax> Parser::ParseCallExpression()
    {
        auto identifierToken = this->Advance();
        auto openParenthesis = this->Consume(SyntaxKind::OpenParenthesisToken, "Expected '('.");

        std::vector<std::unique_ptr<ExpressionSyntax>> arguments;
        
        while(this->Current()->Kind() != SyntaxKind::CloseParenthesisToken && this->Current()->Kind() != SyntaxKind::EndOfFileToken)
        {
            auto expression = this->ParseExpression();
            arguments.emplace_back(std::move(expression));

            if(this->Current()->Kind() != SyntaxKind::CloseParenthesisToken)
            {
                auto commaToken = this->Consume(SyntaxKind::CommaToken, "Expected ','.");
                /* arguments.emplace_back(std::move(commaToken)); */
            }
        }

        auto closeParenthesis = this->Consume(SyntaxKind::CloseParenthesisToken, "Expected ')'.");

        return std::make_unique<CallExpressionSyntax>(std::move(identifierToken), std::move(openParenthesis), std::move(arguments), std::move(closeParenthesis));
        
    }

    std::unique_ptr<ExpressionSyntax> Parser::ParsePrimaryExpression()
    {
        switch (this->Current()->Kind())
        {
            case SyntaxKind::OpenParenthesisToken:
                return this->ParseParenthesizedExpression();
                break;
            case SyntaxKind::IdentifierToken:
                if(this->Peek(1)->Kind() == SyntaxKind::OpenParenthesisToken)
                {
                    return this->ParseCallExpression();
                }
                else
                {
                    return this->ParseNameExpression();
                }
                break;
            case SyntaxKind::TrueKeyword:
            case SyntaxKind::FalseKeyword:
            case SyntaxKind::StringToken:
            default:
                return this->ParseLiteralExpression();
                break;
        }
    }

    std::unique_ptr<ExpressionSyntax> Parser::ParseParenthesizedExpression()
    {
        auto openParenthesisToken = this->Advance();
        auto expression = this->ParseExpression();
        auto closeParenthesisToken = this->Consume(SyntaxKind::CloseParenthesisToken, "Expected ')'.");

        return std::make_unique<ParenthesizedExpressionSyntax>(std::move(openParenthesisToken), std::move(expression), std::move(closeParenthesisToken));
    }

    std::unique_ptr<ExpressionSyntax> Parser::ParseNameExpression()
    {
        auto identifierToken = this->Advance();
        return std::make_unique<NameExpressionSyntax>(std::move(identifierToken));
    }

    std::unique_ptr<ExpressionSyntax> Parser::ParseLiteralExpression()
    {
        if(this->Current()->Kind() == SyntaxKind::TrueKeyword || this->Current()->Kind() == SyntaxKind::FalseKeyword)
        {
            auto keywordToken = this->Advance();
            auto value = keywordToken->Kind() == SyntaxKind::TrueKeyword;
            return std::make_unique<LiteralExpressionSyntax>(std::move(keywordToken), value);
        }
        else if(this->Current()->Kind() == SyntaxKind::StringToken)
        {
            auto stringToken = this->Advance();
            return std::make_unique<LiteralExpressionSyntax>(std::move(stringToken));
        }
        else
        {
            auto numberToken = this->Consume(SyntaxKind::NumberToken, "Expected a number.");
            return std::make_unique<LiteralExpressionSyntax>(std::move(numberToken));
        }
    }

    /*
    
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
    */

    std::unique_ptr<StatementSyntax> Parser::ParseWhileStatement()
    {
        auto keyword = this->Advance();
        (void)this->Consume(SyntaxKind::OpenParenthesisToken, "Expected a '('.");
        auto condition = this->ParseExpression();
        (void)this->Consume(SyntaxKind::CloseParenthesisToken, "Expected a ')'.");
        auto body = this->ParseStatement();

        return std::make_unique<WhileStatementSyntax>(std::move(keyword), std::move(condition), std::move(body));

    }

    std::unique_ptr<StatementSyntax> Parser::ParseForStatement()
    {
        auto keyword = this->Advance();
        auto identifier = this->Consume(SyntaxKind::IdentifierToken, "Expected an variable name.");
        auto equalsToken = this->Consume(SyntaxKind::EqualsToken, "Expected an '='.");

        auto lowerBound = this->ParseExpression();

        auto toKeyword = this->Consume(SyntaxKind::ToKeyword, "Expected a 'to' keyword.");

        auto upperBound = this->ParseExpression();
        auto body = this->ParseStatement();

        return std::make_unique<ForStatementSyntax>(std::move(keyword), std::move(identifier), std::move(equalsToken), std::move(lowerBound), std::move(toKeyword),std::move(upperBound), std::move(body));
    }

    std::unique_ptr<StatementSyntax> Parser::ParseBreakStatement()
    {
        auto keyword = this->Advance();
        (void)this->Consume(SyntaxKind::SemicolonToken, "Expected a ';'.");
        return std::make_unique<BreakStatementSyntax>(std::move(keyword));
    }

    std::unique_ptr<StatementSyntax> Parser::ParseContinueStatement()
    {
        auto keyword = this->Advance();
        (void)this->Consume(SyntaxKind::SemicolonToken, "Expected a ';'.");
        return std::make_unique<ContinueStatementSyntax>(std::move(keyword));
    }

    /* Every return statement must have an expression */
    std::unique_ptr<StatementSyntax> Parser::ParseReturnStatement()
    {
        auto keyword = this->Advance();
        std::unique_ptr<ExpressionSyntax> expression = this->ParseExpression();
        (void)this->Consume(SyntaxKind::SemicolonToken, "Expected a ';'.");
        return std::make_unique<ReturnStatementSyntax>(std::move(keyword), std::move(expression));
    }
}