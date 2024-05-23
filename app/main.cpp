#include <iostream>
#include <codeanalysis/SyntaxKind.hpp>
#include <codeanalysis/Lexer.hpp>
#include <codeanalysis/Evaluator.hpp>
#include <codeanalysis/SyntaxTree.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/Binder.hpp>
#include <codeanalysis/BoundScope.hpp>
#include <codeanalysis/BoundNodePrinter.hpp>
#include <fstream>

#define FILE_PATH "/home/nbaskey/Desktop/nirmal/projects/compiler/source_file/"

void Run1();
void Run2();
void Run3();

trylang::variable_map_t g_variable_map;

int main()
{
    try
    {
        // Run1();
        Run2();
        // Run3();
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
    bool showprogram = false;
    bool showcompact = false;
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

            while(true)
            {
                std::cout << "| ";
                std::getline(std::cin, line);

                if(line.empty())
                {
                    continue;
                }

                if(line.at(line.size() - 1) == '@')
                {
                    g_buffer << line.substr(0, line.size() - 1);
                    break;
                }

                g_buffer << line << " ";
            }


            text = g_buffer.str();
        }
        else if(line == "#showast")
        {
            showast = !showast;
            std::cout << (showast ? "Showing AST\n": "Not Showing AST\n");
            continue;
        }
        else if(line == "#showprogram")
        {
            showprogram = !showprogram;
            std::cout << (showprogram ? "Showing Program Tree\n": "Not Showing Program Tree\n");
            continue;
        }
        else if(line == "#showcompact")
        {
            showcompact = !showcompact;
            std::cout << (showcompact ? "Showing Compact Version\n": "Not Showing Compact Version\n");
            continue;
        }
        else if(line == "#exit")
        {
            break;
        }

        std::unique_ptr<trylang::SyntaxTree> syntaxTree = trylang::SyntaxTree::Parse(std::move(text));
        auto program = trylang::Binder::BindProgram(syntaxTree->_root.get());

        errors.append(syntaxTree->_errors);
        errors.append(program->_errors);

        if(showast)
        {
            trylang::PrettyPrintSyntaxNodes(syntaxTree->_root.get());
            std::cout << "\n";
        }

        if(showprogram)
        {
            trylang::PrettyPrintBoundNodes((program->_globalScope->_statement.get()));
            std::cout << "\n";
        }

        if(showcompact)
        {
            trylang::NodePrinter::Write(program->_globalScope->_statement.get());
            std::cout << "\n";
        }

        if(!errors.empty())
        {
            std::cout << "\n" << errors << "\n";
        }
        else
        {

            trylang::Evaluator evaluator(std::move(program), g_variable_map);
            trylang::object_t result = evaluator.Evaluate();
            if(result.has_value())
            {
                std::visit(trylang::PrintVisitor{}, *result);
            }
            std::cout << "\n";
        }

        errors.clear();

    }
}

void Run3()
{
    std::string errors;

    std::ifstream infile(FILE_PATH "main.txt");
    if(!infile.is_open())
    {
        throw std::runtime_error("Not able to open file");
    }

    std::stringstream buffer;
    buffer << infile.rdbuf();

    std::unique_ptr<trylang::SyntaxTree> syntaxTree = trylang::SyntaxTree::Parse(buffer.str());
    auto program = trylang::Binder::BindProgram(syntaxTree->_root.get());

    errors.append(syntaxTree->_errors);
    errors.append(program->_errors);

    if(!errors.empty())
    {
        std::cout << "\n" << errors << "\n";
    }
    else
    {
        trylang::Evaluator evaluator(std::move(program), g_variable_map);
        trylang::object_t result = evaluator.Evaluate();
        if(result.has_value())
        {
            std::visit(trylang::PrintVisitor{}, *result);
        }
        std::cout << "\n";
    }
}