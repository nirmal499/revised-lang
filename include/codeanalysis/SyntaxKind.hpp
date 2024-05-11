#pragma once
#include <unordered_map>
#include <string>

namespace trylang
{
    enum class SyntaxKind
    {

        /* Tokens */
        EndOfFileToken,
        BadToken,
        NumberToken,
        WhitespaceToken,
        PlusToken,
        MinusToken,
        StarToken,
        SlashToken,
        OpenParenthesisToken,
        CloseParenthesisToken,
        IdentifierToken,
        AmpersandAmpersandToken,
        PipePipeToken,
        BangToken,
        EqualsEqualsToken,
        BangsEqualsToken,
        EqualsToken,
        OpenBraceToken,
        CloseBraceToken,
        LessThanToken,
        LessThanEqualsToken,
        GreaterThanToken,
        GreaterThanEqualsToken,

        CompilationUnit,

        /* Keywords*/
        TrueKeyword,
        FalseKeyword,
        LetKeyword,
        VarKeyword,

        /* Expressions */
        LiteralExpression,
        BinaryExpression,
        UnaryExpression,
        ParenthesizedExpression,
        NameExpression,
        AssignmentExpression,

        /* Statements */
        BlockStatement,
        ExpressionStatement,
        VariableDeclarationStatement,
    };

    inline std::unordered_map<SyntaxKind, std::string> __syntaxStringMap = 
    {
        {SyntaxKind::NumberToken, "NumberToken"},
        {SyntaxKind::WhitespaceToken, "WhitespaceToken"},
        {SyntaxKind::PlusToken, "PlusToken"},
        {SyntaxKind::MinusToken, "MinusToken"},
        {SyntaxKind::StarToken, "StarToken"},
        {SyntaxKind::SlashToken, "SlashToken"},
        {SyntaxKind::OpenParenthesisToken, "OpenParenthesisToken"},
        {SyntaxKind::CloseParenthesisToken, "CloseParenthesisToken"},
        {SyntaxKind::EndOfFileToken, "EndOfFileToken"},
        {SyntaxKind::BadToken, "BadToken"},
        {SyntaxKind::IdentifierToken, "IdentifierToken"},
        {SyntaxKind::AmpersandAmpersandToken, "AmpersandAmpersandToken"},
        {SyntaxKind::PipePipeToken, "PipePipeToken"},
        {SyntaxKind::BangToken, "BangToken"},
        {SyntaxKind::EqualsEqualsToken, "EqualsEqualsToken"},
        {SyntaxKind::BangsEqualsToken, "BangsEqualsToken"},
        {SyntaxKind::EqualsToken, "EqualsToken"},
        {SyntaxKind::OpenBraceToken, "OpenBraceToken"},
        {SyntaxKind::CloseBraceToken, "CloseBraceToken"},
        {SyntaxKind::LessThanToken, "LessThanToken"},
        {SyntaxKind::GreaterThanToken, "GreaterThanToken"},
        {SyntaxKind::LessThanEqualsToken, "LessThanEqualsToken"},
        {SyntaxKind::GreaterThanEqualsToken, "GreaterThanEqualsToken"},

        {SyntaxKind::CompilationUnit, "CompilationUnit"},

        {SyntaxKind::TrueKeyword, "TrueKeyword"},
        {SyntaxKind::FalseKeyword, "FalseKeyword"},
        {SyntaxKind::LetKeyword, "LetKeyword"},
        {SyntaxKind::VarKeyword, "VarKeyword"},

        {SyntaxKind::LiteralExpression, "LiteralExpression"},
        {SyntaxKind::BinaryExpression, "BinaryExpression"},
        {SyntaxKind::UnaryExpression, "UnaryExpression"},
        {SyntaxKind::ParenthesizedExpression, "ParenthesizedExpression"},
        {SyntaxKind::NameExpression, "NameExpression"},
        {SyntaxKind::AssignmentExpression, "AssignmentExpression"},


        {SyntaxKind::BlockStatement, "BlockStatement"},
        {SyntaxKind::ExpressionStatement, "ExpressionStatement"},
        {SyntaxKind::VariableDeclarationStatement, "VariableDeclarationStatement"},
    };

    inline std::ostream& operator<<(std::ostream& out, SyntaxKind kind)
    {
        return out << __syntaxStringMap[kind];
    }
}