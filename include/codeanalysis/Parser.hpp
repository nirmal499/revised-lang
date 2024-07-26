#pragma once

#include <vector>
#include <memory>
#include <sstream>
#include <codeanalysis/SyntaxKind.hpp>

namespace trylang
{
    struct SyntaxToken;
    struct TypeClauseSyntax;
    struct ExpressionSyntax;
    struct CompilationUnitSyntax;
    struct StatementSyntax;
    struct MemberSyntax;
    struct ParameterSyntax;

    struct Parser
    {

        int _current = 0;
        std::vector<std::unique_ptr<StatementSyntax>> _statements;
        std::vector<std::unique_ptr<SyntaxToken>> _tokens;
        std::size_t _tokens_size;
        static std::stringstream _buffer;

        static std::string Errors();
        explicit Parser(std::vector<std::unique_ptr<SyntaxToken>>&& tokens);

        SyntaxToken* Peek(int offset);
        SyntaxToken* Current();
        bool IsAtEnd();
        std::unique_ptr<SyntaxToken> Advance();
        std::unique_ptr<SyntaxToken> Consume(SyntaxKind kind, std::string message);
        bool Check(SyntaxKind kind);
        void SynchronizeAfterAnExpectionForInvalidTokenMatch();

        void GenerateError(int line, std::string message);
        void Error(SyntaxToken* token, std::string message);

        std::unique_ptr<StatementSyntax> ParseFunctionDeclarationStatement();
        std::vector<std::unique_ptr<ParameterSyntax>> ParseParameterList();
        std::unique_ptr<ParameterSyntax> ParseParameter();

        std::unique_ptr<StatementSyntax> ParseStatement();
        std::unique_ptr<StatementSyntax> ParseBlockStatement(bool isFunction = false);
        std::unique_ptr<StatementSyntax> ParseExpressionStatement();
        std::unique_ptr<StatementSyntax> ParseVariableDeclarationStatement();
        std::unique_ptr<TypeClauseSyntax> ParseOptionalTypeClause();
        std::unique_ptr<TypeClauseSyntax> ParseTypeClause();
        std::unique_ptr<StatementSyntax> ParseIfStatement();
        std::unique_ptr<StatementSyntax> ParseElseClause();
        std::unique_ptr<StatementSyntax> ParseWhileStatement();
        std::unique_ptr<StatementSyntax> ParseForStatement();
        std::unique_ptr<StatementSyntax> ParseBreakStatement();
        std::unique_ptr<StatementSyntax> ParseContinueStatement();
        std::unique_ptr<StatementSyntax> ParseReturnStatement();

        std::unique_ptr<StatementSyntax> ParseDeclaration();

        std::unique_ptr<ExpressionSyntax> ParseLogicalOrExpression();
        std::unique_ptr<ExpressionSyntax> ParseLogicalAndExpression();
        std::unique_ptr<ExpressionSyntax> ParseEqualityExpression();
        std::unique_ptr<ExpressionSyntax> ParseComparisonExpression();
        std::unique_ptr<ExpressionSyntax> ParseTermExpression();
        std::unique_ptr<ExpressionSyntax> ParseFactorExpression();
        std::unique_ptr<ExpressionSyntax> ParseUnaryExpression();
        std::unique_ptr<ExpressionSyntax> ParseCallExpression();

        std::unique_ptr<ExpressionSyntax> ParseParenthesizedExpression();
        std::unique_ptr<ExpressionSyntax> ParseNameExpression();
        std::unique_ptr<ExpressionSyntax> ParseLiteralExpression();

        std::unique_ptr<ExpressionSyntax> ParseExpression();
        std::unique_ptr<ExpressionSyntax> ParseAssignmentExpression();
        std::unique_ptr<ExpressionSyntax> ParsePrimaryExpression();
        // int GetBinaryOperatorPrecedance(SyntaxKind kind);
        // int GetUnaryOperatorPrecedance(SyntaxKind kind);

        static std::unique_ptr<CompilationUnitSyntax> AST(std::vector<std::unique_ptr<SyntaxToken>>&& tokens);
        std::unique_ptr<CompilationUnitSyntax> Parse();
    };
}