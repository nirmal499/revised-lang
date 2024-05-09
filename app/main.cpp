#include <iostream>
#include <codeanalysis/SyntaxKind.hpp>
#include <codeanalysis/Lexer.hpp>
#include <codeanalysis/Evaluator.hpp>
#include <codeanalysis/SyntaxTree.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/Binder.hpp>

void Run1();
void Run2();

std::unordered_map<std::string, trylang::oobject_t> g_variables;

int main()
{
    try
    {
        // Run1();
        Run2();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return EXIT_SUCCESS;
}

void Run1()
{
    std::string line;

    while(true)
    {
        std::cout << " >> ";
        std::getline(std::cin, line);
        if(line.empty())
        {
            break;
        }

        trylang::Lexer lexer(line);
        while(true)
        {
            auto token = lexer.NextTokenize();
            if(token->_kind == trylang::SyntaxKind::EndOfFileToken)
            {
                break;
            }

            std::cout << *token;
        }

        std::cout << "\n";
    }
}

void Run2()
{
    std::string line;
    bool showast = false;
    std::string errors;

    while(true)
    {
        std::cout << " >> ";
        std::getline(std::cin, line);
        if(line.empty())
        {
            continue;
        }

        if(line == "#showast")
        {
            showast = !showast;
            std::cout << (showast ? "Showing AST\n": "Not Showing AST\n");
            continue;
        }
        else if(line == "#exit")
        {
            break;
        }

        std::unique_ptr<trylang::SyntaxTree> syntaxTree = trylang::SyntaxTree::Parse(line);
        std::unique_ptr<trylang::Binder> binder = std::make_unique<trylang::Binder>(g_variables);
        std::unique_ptr<trylang::BoundExpressionNode> boundExpression = binder->BindExpression(syntaxTree->_root.get());

        errors = syntaxTree->_errors.append(binder->Errors());

        if(showast)
        {
            trylang::PrettyPrint(syntaxTree->_root.get());
        }

        if(!errors.empty())
        {
            std::cout << "\n" << errors << "\n";
        }
        else
        {
            trylang::Evaluator evaluator(std::move(boundExpression), g_variables);
            trylang::oobject_t result = evaluator.Evaluate();
            std::visit(trylang::PrintVisitor{}, result);
            std::cout << "\n";
        }

        errors.clear();
    }
}