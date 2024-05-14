#include <codeanalysis/ExpressionSyntax.hpp>

namespace trylang
{
    void PrettyPrintSyntaxNodes(SyntaxNode* node, std::string indent)
    {
        std::cout << indent;
        std::cout << node->Kind(); /* cout is overloaded for node->Kind() */

        SyntaxToken* data = dynamic_cast<SyntaxToken*>(node);
        if(data != nullptr && data->_value.has_value())
        {
            std::visit(PrintVisitor{}, *(data->_value));
        }

        std::cout << "\n";

        indent += "     ";

        for(auto const& child: node->GetChildren())
        {
            if(child != nullptr)
            {
                PrettyPrintSyntaxNodes(child, indent);
            }
        }

    }

    CompilationUnitSyntax::CompilationUnitSyntax(std::unique_ptr<StatementSyntax> statement, const std::shared_ptr<SyntaxToken>& endOfFileToken)
        : _statement(std::move(statement)), _endOfFileToken(endOfFileToken)
    {}


    SyntaxKind CompilationUnitSyntax::Kind()
    {
        return SyntaxKind::CompilationUnit;
    }

    std::vector<SyntaxNode*> CompilationUnitSyntax::GetChildren()
    {
        return {_statement.get()};
    }

    BlockStatementSyntax::BlockStatementSyntax(
            const std::shared_ptr<SyntaxToken>& openBraceToken,
            std::vector<std::unique_ptr<StatementSyntax>> statements ,
            const std::shared_ptr<SyntaxToken>& closeBraceToken) : _openBraceToken(openBraceToken), _statements(std::move(statements)), _closeBraceToken(closeBraceToken)
    {}

    SyntaxKind BlockStatementSyntax::Kind()
    {
        return SyntaxKind::BlockStatement;
    }

    std::vector<SyntaxNode*> BlockStatementSyntax::GetChildren()
    {
        std::vector<SyntaxNode*> children(_statements.size() + 2);

        children.push_back(_openBraceToken.get());
        for(const auto& stmt: _statements)
        {
            children.push_back(stmt.get());
        }
        children.push_back(_closeBraceToken.get());

        return children; // RVO
    }

    VariableDeclarationSyntax::VariableDeclarationSyntax(
            const std::shared_ptr<SyntaxToken>& keyword,
            const std::shared_ptr<SyntaxToken>& identifier,
            const std::shared_ptr<SyntaxToken>& equalsToken,
            std::unique_ptr<ExpressionSyntax> expression
        ) : _keyword(keyword), _identifier(identifier), _equalsToken(equalsToken), _expression(std::move(expression))
    {}

    SyntaxKind VariableDeclarationSyntax::Kind()
    {
        return SyntaxKind::VariableDeclarationStatement;
    }

    std::vector<SyntaxNode*> VariableDeclarationSyntax::GetChildren()
    {
        return {_keyword.get(), _identifier.get(), _equalsToken.get(), _expression.get()};
    }

    ExpressionStatementSyntax::ExpressionStatementSyntax(std::unique_ptr<ExpressionSyntax> expression) : _expression(std::move(expression))
    {}

    SyntaxKind ExpressionStatementSyntax::Kind()
    {
        return SyntaxKind::ExpressionStatement;
    }

    std::vector<SyntaxNode*> ExpressionStatementSyntax::GetChildren()
    {
        return {_expression.get()};
    }

    NameExpressionSyntax::NameExpressionSyntax(const std::shared_ptr<SyntaxToken>& identifierToken)
            : _identifierToken(identifierToken)
    {}

    SyntaxKind NameExpressionSyntax::Kind()
    {
        return SyntaxKind::NameExpression;
    }

    std::vector<SyntaxNode*> NameExpressionSyntax::GetChildren()
    {
        return std::vector<SyntaxNode*>{_identifierToken.get()};
    }

    AssignmentExpressionSyntax::AssignmentExpressionSyntax(const std::shared_ptr<SyntaxToken>& identifierToken, const std::shared_ptr<SyntaxToken>& equalsToken, std::unique_ptr<ExpressionSyntax> expression)
            : _identifierToken(identifierToken), _equalsToken(equalsToken), _expression(std::move(expression))
    {}

    SyntaxKind AssignmentExpressionSyntax::Kind()
    {
        return SyntaxKind::AssignmentExpression;
    }

    std::vector<SyntaxNode*> AssignmentExpressionSyntax::GetChildren()
    {
        return std::vector<SyntaxNode*>{_identifierToken.get(), _equalsToken.get(), _expression.get()};
    }
    
    LiteralExpressionSyntax::LiteralExpressionSyntax(const std::shared_ptr<SyntaxToken>& literalToken)
        : _literalToken(literalToken) 
    {
        _value = _literalToken->_value;
    }

    LiteralExpressionSyntax::LiteralExpressionSyntax(const std::shared_ptr<SyntaxToken>& literalToken, const object_t& value)
        : _literalToken(literalToken), _value(value)
    {}
    
    SyntaxKind LiteralExpressionSyntax::Kind()
    {
        return SyntaxKind::LiteralExpression;
    }

    std::vector<SyntaxNode*> LiteralExpressionSyntax::GetChildren()
    {
        return std::vector<SyntaxNode*>{_literalToken.get()};
    }

    BinaryExpressionSyntax::BinaryExpressionSyntax(std::unique_ptr<ExpressionSyntax> left, const std::shared_ptr<SyntaxToken>& operatorToken, std::unique_ptr<ExpressionSyntax> right)
        : _left(std::move(left)), _operatorToken(operatorToken), _right(std::move(right)) {}

    SyntaxKind BinaryExpressionSyntax::Kind()
    {
        return SyntaxKind::BinaryExpression;
    }

    std::vector<SyntaxNode*> BinaryExpressionSyntax::GetChildren()
    {
        return std::vector<SyntaxNode*>{ _left.get(), _operatorToken.get(), _right.get()};
    }

    UnaryExpressionSyntax::UnaryExpressionSyntax(const std::shared_ptr<SyntaxToken>& operatorToken, std::unique_ptr<ExpressionSyntax> operand)
        : _operatorToken(operatorToken), _operand(std::move(operand)) {}

    SyntaxKind UnaryExpressionSyntax::Kind()
    {
        return SyntaxKind::UnaryExpression;
    }

    std::vector<SyntaxNode*> UnaryExpressionSyntax::GetChildren()
    {
        return std::vector<SyntaxNode*>{_operatorToken.get(), _operand.get()};
    }

    ParenthesizedExpressionSyntax::ParenthesizedExpressionSyntax(const std::shared_ptr<SyntaxToken>& openParenthesisToken, std::unique_ptr<ExpressionSyntax> expression, const std::shared_ptr<SyntaxToken>& closeParenthesisToken)
        : _openParenthesisToken(openParenthesisToken), _expression(std::move(expression)), _closeParenthesisToken(closeParenthesisToken) {}

    SyntaxKind ParenthesizedExpressionSyntax::Kind()
    {
        return SyntaxKind::ParenthesizedExpression;
    }

    std::vector<SyntaxNode*> ParenthesizedExpressionSyntax::GetChildren()
    {
        return std::vector<SyntaxNode*>{_openParenthesisToken.get(), _expression.get(), _closeParenthesisToken.get()};
    }

    SyntaxToken::SyntaxToken(SyntaxKind kind, int position, std::string&& text, const object_t& value)
            : _kind(kind), _position(position), _text(std::move(text)), _value(value)
        {}

    /**********************************************************************************************/
    SyntaxKind SyntaxToken::Kind()
    {
        return _kind;
    }

    std::vector<SyntaxNode*> SyntaxToken::GetChildren()
    {
        return std::vector<SyntaxNode*>{nullptr};
    }
    /**********************************************************************************************/

    std::ostream& operator<<(std::ostream& out, const SyntaxToken& token)
    {
        out << trylang::__syntaxStringMap[token._kind] << ": '" << token._text << "' ";

        if(token._value.has_value())
        {
            std::visit(PrintVisitor{}, *token._value);
        }

        out << "\n";

        return out;
    }

    ElseClauseSyntax::ElseClauseSyntax(const std::shared_ptr<SyntaxToken> &elseKeyword, std::unique_ptr<StatementSyntax> elseStatement)
        : _elseKeyword(elseKeyword), _elseStatement(std::move(elseStatement))
    {

    }

    SyntaxKind ElseClauseSyntax::Kind()
    {
        return SyntaxKind::ElseStatement;
    }

    std::vector<SyntaxNode *> ElseClauseSyntax::GetChildren()
    {
        return std::vector<SyntaxNode *>{_elseKeyword.get(), _elseStatement.get()};
    }

    IfStatementSyntax::IfStatementSyntax(const std::shared_ptr<SyntaxToken> &ifKeyword,
                                         std::unique_ptr<ExpressionSyntax> condition,
                                         std::unique_ptr<StatementSyntax> thenStatement,
                                         std::unique_ptr<StatementSyntax> elseClause) : _ifKeyword(ifKeyword), _condition(std::move(condition)), _thenStatement(std::move(thenStatement)), _elseClause(std::move(elseClause))
    {

    }

    SyntaxKind IfStatementSyntax::Kind()
    {
        return SyntaxKind::IfStatement;
    }

    std::vector<SyntaxNode *> IfStatementSyntax::GetChildren()
    {
        std::vector<SyntaxNode*> children(4);

        children.push_back(_ifKeyword.get());
        children.push_back(_condition.get());
        children.push_back(_thenStatement.get());
        if(_elseClause != nullptr)
        {
            children.push_back(_elseClause.get());
        }

        return children; // RVO
    }

    WhileStatementSyntax::WhileStatementSyntax(const std::shared_ptr<SyntaxToken> &whileKeyword,
                                               std::unique_ptr<ExpressionSyntax> condition,
                                               std::unique_ptr<StatementSyntax> body) : _whileKeyword(whileKeyword), _condition(std::move(condition)), _body(std::move(body))
    {

    }

    SyntaxKind WhileStatementSyntax::Kind()
    {
        return SyntaxKind::WhileStatement;
    }

    std::vector<SyntaxNode *> WhileStatementSyntax::GetChildren()
    {
        return {_whileKeyword.get(), _condition.get(), _body.get()};
    }

    ForStatementSyntax::ForStatementSyntax(const std::shared_ptr<SyntaxToken> &keyword,
                                           const std::shared_ptr<SyntaxToken> &identifierToken,
                                           const std::shared_ptr<SyntaxToken> &equalsToken,
                                           std::unique_ptr<ExpressionSyntax> lowerBound,
                                           const std::shared_ptr<SyntaxToken> &toKeyword,
                                           std::unique_ptr<ExpressionSyntax> upperBound,
                                           std::unique_ptr<StatementSyntax> body) : _keyword(keyword), _identifier(identifierToken), _equalsToken(equalsToken), _lowerBound(std::move(lowerBound)), _toKeyword(toKeyword) ,_upperBound(std::move(upperBound)), _body(std::move(body))
    {

    }

    SyntaxKind ForStatementSyntax::Kind()
    {
        return SyntaxKind::ForStatement;
    }

    std::vector<SyntaxNode *> ForStatementSyntax::GetChildren()
    {
        return {_keyword.get(), _identifier.get(), _equalsToken.get(), _lowerBound.get(), _upperBound.get()};
    }

    SyntaxKind CallExpressionSyntax::Kind()
    {
        return SyntaxKind::CallExpression;
    }

    std::vector<SyntaxNode *> CallExpressionSyntax::GetChildren()
    {
        std::vector<SyntaxNode*> children(_arguments.size() + 3);
        children.push_back(_identifer.get());
        children.push_back(_openParenthesis.get());

        for(const auto& argument: _arguments)
        {
            children.push_back(argument.get());
        }

        children.push_back(_closeParenthesis.get());

        return children;
    }

    CallExpressionSyntax::CallExpressionSyntax(const std::shared_ptr<SyntaxToken> &identifier,
                                               const std::shared_ptr<SyntaxToken>& openParenthesis,
                                               std::vector<std::unique_ptr<ExpressionSyntax>> arguments,
                                               const std::shared_ptr<SyntaxToken>& closeParenthesis) : _identifer(identifier), _openParenthesis(openParenthesis), _arguments(std::move(arguments)), _closeParenthesis(closeParenthesis)
    {

    }
}