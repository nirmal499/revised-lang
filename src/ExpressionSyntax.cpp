#include <codeanalysis/ExpressionSyntax.hpp>

namespace trylang
{
    void PrettyPrint(SyntaxNode* node, std::string indent)
    {
        std::cout << indent;
        std::cout << node->Kind();

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
                PrettyPrint(child, indent);
            }
        }

    }
    
    NumberExpressionSyntax::NumberExpressionSyntax(const std::shared_ptr<SyntaxToken>& numberToken)
        : _numberToken(numberToken) {}

    
    SyntaxKind NumberExpressionSyntax::Kind()
    {
        return SyntaxKind::NumberExpression;
    }

    std::vector<SyntaxNode*> NumberExpressionSyntax::GetChildren()
    {
        return std::vector<SyntaxNode*>{_numberToken.get()};
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

    SyntaxToken::SyntaxToken(SyntaxKind kind, int position, std::string&& text, object_t&& value)
            : _kind(kind), _position(position), _text(std::move(text)), _value(std::move(value))
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
}