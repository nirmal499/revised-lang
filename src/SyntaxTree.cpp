#include <codeanalysis/SyntaxTree.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/Parser.hpp>

namespace trylang
{
    SyntaxTree::SyntaxTree(std::string text)
    {
        Parser parser(text);

        _text = std::move(text);
        _root = parser.ParseCompilationUnit();
        _errors += parser.Errors();
    }
    
    std::unique_ptr<SyntaxTree> SyntaxTree::Parse(std::string text)
    {
        /* It calls the ctor of SyntaxTree */
        return std::make_unique<SyntaxTree>(std::move(text));
    }
}