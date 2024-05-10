#include <iostream>
#include <codeanalysis/SyntaxKind.hpp>
#include <codeanalysis/Lexer.hpp>
#include <codeanalysis/Evaluator.hpp>
#include <codeanalysis/SyntaxTree.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/Binder.hpp>
#include <codeanalysis/VariableSymbol.hpp>
#include <codeanalysis/BoundScope.hpp>

void Run1();
void Run2();

trylang::variable_map_t g_variable_map;

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
        std::cout << ">> ";
        std::getline(std::cin, line);

        std::string text = line;

        if(line.empty())
        {
            continue;
        }
        else if(line.at(0) == '@')
        {
            std::stringstream g_buffer;
            g_buffer << line.substr(1);

            std::cout << "| ";
            std::getline(std::cin, line);
            while(line.at(line.size() - 1) != '@')
            {
                g_buffer << line;
                std::cout << "| ";
                std::getline(std::cin, line);
            }

            g_buffer << line.substr(0, line.size() - 1);

            text = g_buffer.str();
        }
        else if(line == "#showast")
        {
            showast = !showast;
            std::cout << (showast ? "Showing AST\n": "Not Showing AST\n");
            continue;
        }
        else if(line == "#exit")
        {
            break;
        }

        std::unique_ptr<trylang::SyntaxTree> syntaxTree = trylang::SyntaxTree::Parse(std::move(text));
        std::shared_ptr<trylang::BoundGlobalScope> globalScope = trylang::Binder::BindGlobalScope(syntaxTree->_root.get());

        errors.append(syntaxTree->_errors);
        errors.append(globalScope->_errors);

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
            trylang::Evaluator evaluator(std::move(globalScope->_expression), g_variable_map);
            trylang::oobject_t result = evaluator.Evaluate();
            std::visit(trylang::PrintVisitor{}, result);
            std::cout << "\n";
        }

        errors.clear();

    }
}