#pragma once

#include <memory>
#include <sstream>
#include <optional>
#include <codeanalysis/SyntaxKind.hpp>
#include <codeanalysis/VariableSymbol.hpp>

namespace trylang
{
    struct ExpressionSyntax;
    struct StatementSyntax;
    struct TypeClauseSyntax;

    struct LiteralExpressionSyntax;
    struct UnaryExpressionSyntax;
    struct BinaryExpressionSyntax;
    struct ParenthesizedExpressionSyntax;
    struct NameExpressionSyntax;
    struct AssignmentExpressionSyntax;
    struct CallExpressionSyntax;

    struct BlockStatementSyntax;
    struct ExpressionStatementSyntax;
    struct VariableDeclarationSyntax;
    struct IfStatementSyntax;
    struct WhileStatementSyntax;
    struct ForStatementSyntax;

    struct BoundExpressionNode;
    struct BoundStatementNode;
    struct BoundBlockStatement;
    struct BoundWhileStatement;

    struct CompilationUnitSyntax;

    struct BoundScope;
    struct BoundGlobalScope;

    struct Binder
    {
        std::shared_ptr<BoundScope> _scope = nullptr;
        bool _turnOnScopingInBlockStatement = true;

        int _labelCount = 0;

        explicit Binder(const std::shared_ptr<BoundScope>& parent);
        static std::shared_ptr<BoundGlobalScope> BindGlobalScope(CompilationUnitSyntax* syntax);

        std::stringstream _buffer;

        std::string Errors();

        std::unique_ptr<BoundExpressionNode> BindExpression(ExpressionSyntax* syntax, bool canBeVoid = false);
        std::unique_ptr<BoundExpressionNode> BindExpressionInternal(ExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindExpression(ExpressionSyntax* syntax, const char* targetType);
        std::unique_ptr<BoundExpressionNode> BindConversion(const char* type, ExpressionSyntax *syntax, bool allowExplicit = false);
        std::unique_ptr<BoundExpressionNode> BindConversion(const char* type, std::unique_ptr<BoundExpressionNode> expression, bool allowExplicit = false);

        std::unique_ptr<BoundStatementNode> BindStatement(StatementSyntax* syntax);
        std::unique_ptr<BoundStatementNode> BindVariableDeclaration(VariableDeclarationSyntax* syntax);
        const char* BindTypeClause(TypeClauseSyntax* syntax);

        std::unique_ptr<BoundStatementNode> BindBlockStatement(BlockStatementSyntax *syntax);
        std::unique_ptr<BoundStatementNode> BindExpressionStatement(ExpressionStatementSyntax *syntax);
        std::unique_ptr<BoundStatementNode> BindIfStatement(IfStatementSyntax *syntax);
        std::unique_ptr<BoundStatementNode> BindWhileStatement(WhileStatementSyntax *syntax);
        std::unique_ptr<BoundStatementNode> BindForStatement(ForStatementSyntax *syntax);

        std::unique_ptr<BoundExpressionNode> BindParenthesizedExpression(ParenthesizedExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindNameExpression(NameExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindAssignmentExpression(AssignmentExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindLiteralExpression(LiteralExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindUnaryExpression(UnaryExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindBinaryExpression(BinaryExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindCallExpression(CallExpressionSyntax *syntax);

        static std::unique_ptr<BoundBlockStatement> Flatten(std::unique_ptr<BoundStatementNode> statement);

        std::unique_ptr<BoundStatementNode> BindWhileStatement(BoundWhileStatement *syntax);

        LabelSymbol GenerateLabel();
    };
}