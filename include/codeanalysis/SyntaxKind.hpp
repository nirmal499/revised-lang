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
        StringToken,
        CommaToken,
        ColonToken,
        SemicolonToken,

        CompilationUnit,

        /* Keywords*/
        TrueKeyword,
        FalseKeyword,
        LetKeyword,
        VarKeyword,
        IfKeyword,
        ElseKeyword,
        WhileKeyword,
        ForKeyword,
        ToKeyword,
        FunctionKeyword,
        BreakKeyword,
        ContinueKeyword,

        /* Expressions */
        LiteralExpression,
        BinaryExpression,
        UnaryExpression,
        ParenthesizedExpression,
        NameExpression,
        AssignmentExpression,
        CallExpression,

        /* Statements */
        BlockStatement,
        ExpressionStatement,
        VariableDeclarationStatement,
        IfStatement,
        ElseStatement,
        WhileStatement,
        ForStatement,
        BreakStatement,
        ContinueStatement,

        ParameterExpression,
        FunctionDeclarationStatement,

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
        {SyntaxKind::StringToken, "StringToken"},
        {SyntaxKind::CommaToken, "CommaToken"},
        {SyntaxKind::ColonToken, "ColonToken"},
        {SyntaxKind::SemicolonToken, "SemicolonToken"},

        {SyntaxKind::CompilationUnit, "CompilationUnit"},

        {SyntaxKind::TrueKeyword, "TrueKeyword"},
        {SyntaxKind::FalseKeyword, "FalseKeyword"},
        {SyntaxKind::LetKeyword, "LetKeyword"},
        {SyntaxKind::VarKeyword, "VarKeyword"},
        {SyntaxKind::IfKeyword, "IfKeyword"},
        {SyntaxKind::ElseKeyword, "ElseKeyword"},
        {SyntaxKind::WhileKeyword, "WhileKeyword"},
        {SyntaxKind::ForKeyword, "ForKeyword"},
        {SyntaxKind::ToKeyword, "ToKeyword"},
        {SyntaxKind::FunctionKeyword, "FunctionKeyword"},
        {SyntaxKind::BreakKeyword, "BreakKeyword"},
        {SyntaxKind::ContinueKeyword, "ContinueKeyword"},

        {SyntaxKind::LiteralExpression, "LiteralExpression"},
        {SyntaxKind::BinaryExpression, "BinaryExpression"},
        {SyntaxKind::UnaryExpression, "UnaryExpression"},
        {SyntaxKind::ParenthesizedExpression, "ParenthesizedExpression"},
        {SyntaxKind::NameExpression, "NameExpression"},
        {SyntaxKind::AssignmentExpression, "AssignmentExpression"},
        {SyntaxKind::CallExpression, "CallExpression"},


        {SyntaxKind::BlockStatement, "BlockStatement"},
        {SyntaxKind::ExpressionStatement, "ExpressionStatement"},
        {SyntaxKind::VariableDeclarationStatement, "VariableDeclarationStatement"},
        {SyntaxKind::IfStatement, "IfStatement"},
        {SyntaxKind::ElseStatement, "ElseStatement"},
        {SyntaxKind::WhileStatement, "WhileStatement"},
        {SyntaxKind::ForStatement, "ForStatement"},
        {SyntaxKind::BreakStatement, "BreakStatement"},
        {SyntaxKind::ContinueStatement, "ContinueStatement"},

        {SyntaxKind::FunctionDeclarationStatement, "FunctionDeclarationStatement"},
        {SyntaxKind::ParameterExpression, "ParameterExpression"},
    };

    inline std::ostream& operator<<(std::ostream& out, SyntaxKind kind)
    {
        return out << __syntaxStringMap[kind];
    }
}