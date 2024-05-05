#pragma once
#include <unordered_map>
#include <string>

namespace trylang
{
    enum class SyntaxKind
    {
        NumberToken,
        WhitespaceToken,
        PlusToken,
        MinusToken,
        StarToken,
        SlashToken,
        OpenParenthesisToken,
        CloseParenthesisToken,
        EndOfFileToken,
        BadToken,

        NumberExpression,
        BinaryExpression,
        ParenthesizedExpression,
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
        {SyntaxKind::NumberExpression, "NumberExpression"},
        {SyntaxKind::BinaryExpression, "BinaryExpression"},
        {SyntaxKind::ParenthesizedExpression, "ParenthesizedExpression"},

    };

    inline std::ostream& operator<<(std::ostream& out, SyntaxKind kind)
    {
        return out << __syntaxStringMap[kind];
    }
}