#include <codeanalysis/SyntaxTree.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/Parser.hpp>

namespace trylang
{
    
    SyntaxTree::SyntaxTree(std::string errors, std::unique_ptr<ExpressionSyntax> root, const std::shared_ptr<SyntaxToken>& endOfFileToken)
        : _errors(std::move(errors)), _root(std::move(root)), _endOfFileToken(endOfFileToken)
    {}
    
    std::unique_ptr<SyntaxTree> SyntaxTree::Parse(std::string text)
    {
        Parser parser(std::move(text));
        return parser.Parse();
    }
}