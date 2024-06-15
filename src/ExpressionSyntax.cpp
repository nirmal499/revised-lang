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
            std::cout << " (";
            std::visit(PrintVisitor{}, *(data->_value));
            std::cout << ")";
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

    CompilationUnitSyntax::CompilationUnitSyntax(std::vector<std::unique_ptr<StatementSyntax>> statements)
        : _statements(std::move(statements))
    {}


    SyntaxKind CompilationUnitSyntax::Kind()
    {
        return SyntaxKind::CompilationUnit;
    }

    std::vector<SyntaxNode*> CompilationUnitSyntax::GetChildren()
    {
        std::vector<SyntaxNode*> children(_statements.size());
        for(const auto& stmt: _statements)
        {
            children.push_back(stmt.get());
        }

        return children; //RVO
    }

    BlockStatementSyntax::BlockStatementSyntax(
            std::unique_ptr<SyntaxToken> openBraceToken,
            std::vector<std::unique_ptr<StatementSyntax>> statements ,
            std::unique_ptr<SyntaxToken> closeBraceToken) : _openBraceToken(std::move(openBraceToken)), _statements(std::move(statements)), _closeBraceToken(std::move(closeBraceToken))
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

    VariableDeclarationStatementSyntax::VariableDeclarationStatementSyntax(
            std::unique_ptr<SyntaxToken> keyword,
            std::unique_ptr<SyntaxToken> identifier,
            std::unique_ptr<TypeClauseSyntax> typeClause,
            std::unique_ptr<SyntaxToken> equalsToken,
            std::unique_ptr<ExpressionSyntax> expression
        ) : _keyword(std::move(keyword)), _identifier(std::move(identifier)), _typeClause(std::move(typeClause)) ,_equalsToken(std::move(equalsToken)), _expression(std::move(expression))
    {}

    SyntaxKind VariableDeclarationStatementSyntax::Kind()
    {
        return SyntaxKind::VariableDeclarationStatement;
    }

    std::vector<SyntaxNode*> VariableDeclarationStatementSyntax::GetChildren()
    {
        return {_keyword.get(), _identifier.get(), _typeClause.get() ,_equalsToken.get(), _expression.get()};
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

    NameExpressionSyntax::NameExpressionSyntax(std::unique_ptr<SyntaxToken> identifierToken)
            : _identifierToken(std::move(identifierToken))
    {}

    SyntaxKind NameExpressionSyntax::Kind()
    {
        return SyntaxKind::NameExpression;
    }

    std::vector<SyntaxNode*> NameExpressionSyntax::GetChildren()
    {
        return std::vector<SyntaxNode*>{_identifierToken.get()};
    }

    AssignmentExpressionSyntax::AssignmentExpressionSyntax(std::unique_ptr<SyntaxToken> identifierToken, std::unique_ptr<SyntaxToken> equalsToken, std::unique_ptr<ExpressionSyntax> expression)
            : _identifierToken(std::move(identifierToken)), _equalsToken(std::move(equalsToken)), _expression(std::move(expression))
    {}

    SyntaxKind AssignmentExpressionSyntax::Kind()
    {
        return SyntaxKind::AssignmentExpression;
    }

    std::vector<SyntaxNode*> AssignmentExpressionSyntax::GetChildren()
    {
        return std::vector<SyntaxNode*>{_identifierToken.get(), _equalsToken.get(), _expression.get()};
    }
    
    LiteralExpressionSyntax::LiteralExpressionSyntax(std::unique_ptr<SyntaxToken> literalToken)
        : _literalToken(std::move(literalToken)) 
    {
        _value = _literalToken->_value;
    }

    LiteralExpressionSyntax::LiteralExpressionSyntax(std::unique_ptr<SyntaxToken> literalToken, const object_t& value)
        : _literalToken(std::move(literalToken)), _value(value)
    {}
    
    SyntaxKind LiteralExpressionSyntax::Kind()
    {
        return SyntaxKind::LiteralExpression;
    }

    std::vector<SyntaxNode*> LiteralExpressionSyntax::GetChildren()
    {
        return std::vector<SyntaxNode*>{_literalToken.get()};
    }

    BinaryExpressionSyntax::BinaryExpressionSyntax(std::unique_ptr<ExpressionSyntax> left, std::unique_ptr<SyntaxToken> operatorToken, std::unique_ptr<ExpressionSyntax> right)
        : _left(std::move(left)), _operatorToken(std::move(operatorToken)), _right(std::move(right)) {}

    SyntaxKind BinaryExpressionSyntax::Kind()
    {
        return SyntaxKind::BinaryExpression;
    }

    std::vector<SyntaxNode*> BinaryExpressionSyntax::GetChildren()
    {
        return std::vector<SyntaxNode*>{ _left.get(), _operatorToken.get(), _right.get()};
    }

    UnaryExpressionSyntax::UnaryExpressionSyntax(std::unique_ptr<SyntaxToken> operatorToken, std::unique_ptr<ExpressionSyntax> operand)
        : _operatorToken(std::move(operatorToken)), _operand(std::move(operand)) {}

    SyntaxKind UnaryExpressionSyntax::Kind()
    {
        return SyntaxKind::UnaryExpression;
    }

    std::vector<SyntaxNode*> UnaryExpressionSyntax::GetChildren()
    {
        return std::vector<SyntaxNode*>{_operatorToken.get(), _operand.get()};
    }

    ParenthesizedExpressionSyntax::ParenthesizedExpressionSyntax(std::unique_ptr<SyntaxToken> openParenthesisToken, std::unique_ptr<ExpressionSyntax> expression, std::unique_ptr<SyntaxToken> closeParenthesisToken)
        : _openParenthesisToken(std::move(openParenthesisToken)), _expression(std::move(expression)), _closeParenthesisToken(std::move(closeParenthesisToken)) {}

    SyntaxKind ParenthesizedExpressionSyntax::Kind()
    {
        return SyntaxKind::ParenthesizedExpression;
    }

    std::vector<SyntaxNode*> ParenthesizedExpressionSyntax::GetChildren()
    {
        return std::vector<SyntaxNode*>{_openParenthesisToken.get(), _expression.get(), _closeParenthesisToken.get()};
    }

    SyntaxToken::SyntaxToken(SyntaxKind kind, int line, std::string&& text, object_t&& value)
            : _kind(kind), _line(line), _text(std::move(text)), _value(value)
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

    ElseStatementSyntax::ElseStatementSyntax(std::unique_ptr<SyntaxToken> elseKeyword, std::unique_ptr<StatementSyntax> elseStatement)
        : _elseKeyword(std::move(elseKeyword)), _elseStatement(std::move(elseStatement))
    {

    }

    SyntaxKind ElseStatementSyntax::Kind()
    {
        return SyntaxKind::ElseStatement;
    }

    std::vector<SyntaxNode *> ElseStatementSyntax::GetChildren()
    {
        return std::vector<SyntaxNode *>{_elseKeyword.get(), _elseStatement.get()};
    }

    IfStatementSyntax::IfStatementSyntax(std::unique_ptr<SyntaxToken> ifKeyword,
                                         std::unique_ptr<ExpressionSyntax> condition,
                                         std::unique_ptr<StatementSyntax> thenStatement,
                                         std::unique_ptr<StatementSyntax> elseClause) : _ifKeyword(std::move(ifKeyword)), _condition(std::move(condition)), _thenStatement(std::move(thenStatement)), _elseClause(std::move(elseClause))
    {

    }

    SyntaxKind IfStatementSyntax::Kind()
    {
        return SyntaxKind::IfStatement;
    }

    std::vector<SyntaxNode *> IfStatementSyntax::GetChildren()
    {
        return {_ifKeyword.get(), _condition.get(), _thenStatement.get(), _elseClause.get()};
    }

    WhileStatementSyntax::WhileStatementSyntax(std::unique_ptr<SyntaxToken> whileKeyword,
                                               std::unique_ptr<ExpressionSyntax> condition,
                                               std::unique_ptr<StatementSyntax> body) : _whileKeyword(std::move(whileKeyword)), _condition(std::move(condition)), _body(std::move(body))
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

    ForStatementSyntax::ForStatementSyntax(std::unique_ptr<SyntaxToken> keyword,
                                           std::unique_ptr<SyntaxToken> identifierToken,
                                           std::unique_ptr<SyntaxToken> equalsToken,
                                           std::unique_ptr<ExpressionSyntax> lowerBound,
                                           std::unique_ptr<SyntaxToken> toKeyword,
                                           std::unique_ptr<ExpressionSyntax> upperBound,
                                           std::unique_ptr<StatementSyntax> body) : _keyword(std::move(keyword)), _identifier(std::move(identifierToken)), _equalsToken(std::move(equalsToken)), _lowerBound(std::move(lowerBound)), _toKeyword(std::move(toKeyword)) ,_upperBound(std::move(upperBound)), _body(std::move(body))
    {

    }

    SyntaxKind ForStatementSyntax::Kind()
    {
        return SyntaxKind::ForStatement;
    }

    std::vector<SyntaxNode *> ForStatementSyntax::GetChildren()
    {
        return {_keyword.get(), _identifier.get(), _equalsToken.get(), _lowerBound.get(),_toKeyword.get(), _upperBound.get(), _body.get()};
    }

    SyntaxKind CallExpressionSyntax::Kind()
    {
        return SyntaxKind::CallExpression;
    }

    std::vector<SyntaxNode *> CallExpressionSyntax::GetChildren()
    {
        std::vector<SyntaxNode*> children(_arguments.size() + 3);
        children.push_back(_identifier.get());
        children.push_back(_openParenthesis.get());

        for(const auto& argument: _arguments)
        {
            children.push_back(argument.get());
        }

        children.push_back(_closeParenthesis.get());

        return children;
    }

    CallExpressionSyntax::CallExpressionSyntax(std::unique_ptr<SyntaxToken> identifier,
                                               std::unique_ptr<SyntaxToken> openParenthesis,
                                               std::vector<std::unique_ptr<ExpressionSyntax>> arguments,
                                               std::unique_ptr<SyntaxToken> closeParenthesis) : _identifier(std::move(identifier)), _openParenthesis(std::move(openParenthesis)), _arguments(std::move(arguments)), _closeParenthesis(std::move(closeParenthesis))
    {

    }

    TypeClauseSyntax::TypeClauseSyntax(std::unique_ptr<SyntaxToken> colonToken,
                                       std::unique_ptr<SyntaxToken> identifierToken): _colonToken(std::move(colonToken)), _identifierToken(std::move(identifierToken))
    {

    }

    SyntaxKind TypeClauseSyntax::Kind()
    {
        return SyntaxKind::ColonToken;
    }

    std::vector<SyntaxNode *> TypeClauseSyntax::GetChildren()
    {
        return {_colonToken.get(), _identifierToken.get()};
    }

    ParameterSyntax::ParameterSyntax(std::unique_ptr<SyntaxToken> identifier,
                                     std::unique_ptr<TypeClauseSyntax> type) : _identifier(std::move(identifier)), _type(std::move(type))
    {

    }

    SyntaxKind ParameterSyntax::Kind()
    {
        return SyntaxKind::ParameterExpression;
    }

    std::vector<SyntaxNode *> ParameterSyntax::GetChildren()
    {
        return {_identifier.get(), _type.get()};
    }

    FunctionDeclarationStatementSyntax::FunctionDeclarationStatementSyntax(std::unique_ptr<SyntaxToken> functionKeyword,
                                                         std::unique_ptr<SyntaxToken> identifier,
                                                         std::unique_ptr<SyntaxToken> openParenthesisToken,
                                                         std::vector<std::unique_ptr<ParameterSyntax>> parameters,
                                                         std::unique_ptr<SyntaxToken> closeParenthesisToken,
                                                         std::unique_ptr<TypeClauseSyntax> typeClause,
                                                         std::unique_ptr<StatementSyntax> body) : _functionKeyword(std::move(functionKeyword)), _identifier(std::move(identifier)), _openParenthesisToken(std::move(openParenthesisToken)), _parameters(std::move(parameters)), _closeParenthesisToken(std::move(closeParenthesisToken)), _typeClause(std::move(typeClause)), _body(std::move(body))
    {

    }

    SyntaxKind FunctionDeclarationStatementSyntax::Kind()
    {
        return SyntaxKind::FunctionDeclarationStatement;
    }

    std::vector<SyntaxNode *> FunctionDeclarationStatementSyntax::GetChildren()
    {
        std::vector<SyntaxNode*> children(5 + _parameters.size());
        children.push_back(_functionKeyword.get());
        children.push_back(_identifier.get());
        children.push_back(_openParenthesisToken.get());
        for(const auto& param: _parameters)
        {
            children.push_back(param.get());
        }
        children.push_back(_closeParenthesisToken.get());
        children.push_back(_typeClause.get());
        children.push_back(_body.get());

        return children; // RVO
    }

    BreakStatementSyntax::BreakStatementSyntax(std::unique_ptr<SyntaxToken> breakKeyword): _breakKeyword(std::move(breakKeyword))
    {

    }

    SyntaxKind BreakStatementSyntax::Kind()
    {
        return SyntaxKind::BreakStatement;
    }

    std::vector<SyntaxNode *> BreakStatementSyntax::GetChildren()
    {
        return {_breakKeyword.get()};
    }

    ContinueStatementSyntax::ContinueStatementSyntax(std::unique_ptr<SyntaxToken> continueKeyword): _continueKeyword(std::move(continueKeyword))
    {

    }

    SyntaxKind ContinueStatementSyntax::Kind()
    {
        return SyntaxKind::ContinueStatement;
    }

    std::vector<SyntaxNode *> ContinueStatementSyntax::GetChildren()
    {
        return {_continueKeyword.get()};
    }

    ReturnStatementSyntax::ReturnStatementSyntax(std::unique_ptr<SyntaxToken> returnKeyword, std::unique_ptr<ExpressionSyntax> expression)
        : _returnKeyword(std::move(returnKeyword)),
          _expression(std::move(expression))
    {
        
    }

    SyntaxKind ReturnStatementSyntax::Kind()
    {
        return SyntaxKind::ReturnStatement;
    }

    std::vector<SyntaxNode*> ReturnStatementSyntax::GetChildren()
    {
        return {_returnKeyword.get(), _expression.get()};
    }
}