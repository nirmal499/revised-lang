#include <iostream>
#include <codeanalysis/SyntaxKind.hpp>
#include <codeanalysis/Lexer.hpp>
#include <codeanalysis/Evaluator.hpp>
#include <codeanalysis/SyntaxTree.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>

void Run1();
void Run2();

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
            auto token = lexer.NextToken();
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
        
        if(showast)
        {
            trylang::PrettyPrint(syntaxTree->_root.get());
        }

        if(!syntaxTree->_errors.empty())
        {
            std::cout << "\n" << syntaxTree->_errors << "\n";
        }
        else
        {
            trylang::Evaluator evaluator(std::move(syntaxTree->_root));
            int result = evaluator.Evaluate();
            std::cout << result << "\n";
        }
    }
}