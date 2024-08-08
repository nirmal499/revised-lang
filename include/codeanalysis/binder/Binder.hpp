#pragma once

#include <memory>
#include <sstream>
#include <optional>
#include <codeanalysis/parser/utils/SyntaxKind.hpp>
#include <codeanalysis/utils/Symbol.hpp>
#include <stack>

namespace trylang
{
    struct SyntaxToken;
    struct ExpressionSyntax;
    struct StatementSyntax;
    struct TypeClauseSyntax;
    struct FunctionDeclarationStatementSyntax;
    struct LiteralExpressionSyntax;
    struct UnaryExpressionSyntax;
    struct BinaryExpressionSyntax;
    struct ParenthesizedExpressionSyntax;
    struct NameExpressionSyntax;
    struct AssignmentExpressionSyntax;
    struct CallExpressionSyntax;
    struct BlockStatementSyntax;
    struct ExpressionStatementSyntax;
    struct VariableDeclarationStatementSyntax;
    struct IfStatementSyntax;
    struct WhileStatementSyntax;
    struct BreakStatementSyntax;
    struct ContinueStatementSyntax;
    struct ReturnStatementSyntax;
    struct BoundExpressionNode;
    struct BoundStatementNode;
    struct BoundBlockStatement;
    struct BoundWhileStatement;
    struct CompilationUnitSyntax;
    struct BoundScope;
    struct BoundProgram;

    struct Binder
    {
        std::shared_ptr<BoundScope> _scope = nullptr;
        FunctionSymbol* _function = nullptr;
        int _labelCountForBreakAndContinueStatement = 0;
        static std::stringstream _buffer;
        std::stack<std::pair<LabelSymbol, LabelSymbol>> _loopStack; /* One for BreakLabel and other for ContinueLabel */

        explicit Binder(const std::shared_ptr<BoundScope>& parent, FunctionSymbol* _function);
        static std::unique_ptr<BoundProgram> BindProgram(CompilationUnitSyntax* syntaxTree);
        static std::unique_ptr<BoundBlockStatement> Flatten(std::unique_ptr<BoundStatementNode> statement);
        static std::string Errors();
        LabelSymbol GenerateLabel();

        // std::unique_ptr<BoundExpressionNode> BindExpression(ExpressionSyntax* syntax, bool canBeVoid = false);
        std::unique_ptr<BoundExpressionNode> BindExpression(ExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindExpression(ExpressionSyntax* syntax, const char* targetType);
        std::unique_ptr<BoundExpressionNode> BindConversion(const char* type, ExpressionSyntax *syntax, bool allowExplicit = false);
        std::unique_ptr<BoundExpressionNode> BindConversion(const char* type, std::unique_ptr<BoundExpressionNode> expression, bool allowExplicit = false);
        std::shared_ptr<VariableSymbol> BindVariable(std::string varName, bool isReadOnly, const char* type);
        std::unique_ptr<BoundStatementNode> BindStatement(StatementSyntax* syntax);
        std::unique_ptr<BoundStatementNode> BindVariableDeclaration(VariableDeclarationStatementSyntax* syntax);
        void BindFunctionDeclaration(FunctionDeclarationStatementSyntax* syntax);
        const char* BindTypeClause(TypeClauseSyntax* syntax);
        std::unique_ptr<BoundStatementNode> BindBlockStatement(BlockStatementSyntax *syntax);
        std::unique_ptr<BoundStatementNode> BindExpressionStatement(ExpressionStatementSyntax *syntax);
        std::unique_ptr<BoundStatementNode> BindIfStatement(IfStatementSyntax *syntax);
        std::unique_ptr<BoundStatementNode> BindWhileStatement(WhileStatementSyntax *syntax);
        std::unique_ptr<BoundStatementNode> BindBreakStatement(BreakStatementSyntax *syntax);
        std::unique_ptr<BoundStatementNode> BindContinueStatement(ContinueStatementSyntax *syntax);
        std::unique_ptr<BoundStatementNode> BindReturnStatement(ReturnStatementSyntax *syntax);
        std::unique_ptr<BoundStatementNode> BindErrorStatement();
        std::pair<std::unique_ptr<BoundStatementNode>, std::pair<LabelSymbol, LabelSymbol>> BindLoopBody(StatementSyntax* body);
        std::unique_ptr<BoundExpressionNode> BindParenthesizedExpression(ParenthesizedExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindNameExpression(NameExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindAssignmentExpression(AssignmentExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindLiteralExpression(LiteralExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindUnaryExpression(UnaryExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindBinaryExpression(BinaryExpressionSyntax* syntax);
        std::unique_ptr<BoundExpressionNode> BindCallExpression(CallExpressionSyntax *syntax);
        std::unique_ptr<BoundStatementNode> BindWhileStatement(BoundWhileStatement *syntax);
    };
}