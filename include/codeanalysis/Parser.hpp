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
    struct SyntaxTree;
    struct CompilationUnitSyntax;
    struct StatementSyntax;
    struct MemberSyntax;
    struct ParameterSyntax;

    struct Parser
    {

        int _position;
        std::vector<std::shared_ptr<SyntaxToken>> _tokens;
        std::size_t _tokens_size;
        std::stringstream _buffer;

        std::string Errors();
        explicit Parser(std::string text);

        std::shared_ptr<SyntaxToken> Peek(int offset);
        std::shared_ptr<SyntaxToken> Current();
        std::shared_ptr<SyntaxToken> NextToken();
        std::shared_ptr<SyntaxToken> MatchToken(SyntaxKind kind);

        std::unique_ptr<CompilationUnitSyntax> ParseCompilationUnit();

        std::vector<std::unique_ptr<MemberSyntax>> ParseMembers();
        std::unique_ptr<MemberSyntax> ParseMember();
        std::unique_ptr<MemberSyntax> ParseFunctionDeclaration();
        std::vector<std::unique_ptr<ParameterSyntax>> ParseParameterList();
        std::unique_ptr<ParameterSyntax> ParseParameter();
        std::unique_ptr<MemberSyntax> ParseGlobalStatement();

        std::unique_ptr<StatementSyntax> ParseStatement();
        std::unique_ptr<StatementSyntax> ParseBlockStatement();
        std::unique_ptr<StatementSyntax> ParseExpressionStatement();
        std::unique_ptr<StatementSyntax> ParseVariableDeclaration();
        std::unique_ptr<TypeClauseSyntax> ParseOptionalTypeClause();
        std::unique_ptr<TypeClauseSyntax> ParseTypeClause();
        std::unique_ptr<StatementSyntax> ParseIfStatement();
        std::unique_ptr<StatementSyntax> ParseElseClause();
        std::unique_ptr<StatementSyntax> ParseWhileStatement();
        std::unique_ptr<StatementSyntax> ParseForStatement();
        std::unique_ptr<StatementSyntax> ParseBreakStatement();
        std::unique_ptr<StatementSyntax> ParseContinueStatement();


        std::unique_ptr<ExpressionSyntax> ParseExpression();
        std::unique_ptr<ExpressionSyntax> ParseAssignmentExpression();
        std::unique_ptr<ExpressionSyntax> ParseBinaryExpression(int parentPrecedance = 0);
        std::unique_ptr<ExpressionSyntax> ParsePrimaryExpression();

        int GetBinaryOperatorPrecedance(SyntaxKind kind);
        int GetUnaryOperatorPrecedance(SyntaxKind kind);

    };
}