#include <iostream>
#include <codeanalysis/SyntaxKind.hpp>
#include <codeanalysis/Lexer.hpp>
#include <codeanalysis/Parser.hpp>
#include <codeanalysis/Binder.hpp>
#include <codeanalysis/Evaluator.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/BoundScope.hpp>
#include <codeanalysis/BoundNodePrinter.hpp>
#include <fstream>
#include <memory>
#include <vector>

#define FILE_PATH "/home/nbaskey/Desktop/nirmal/projects/compiler/source_file/"

void Run3();

int main()
{
    try
    {
        Run3();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return EXIT_SUCCESS;
}

/*
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

        auto tokens = trylang::Lexer::Tokenizer(std::move(text));
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
*/

void Run3()
{
    std::string errors;

    std::ifstream infile(FILE_PATH "main7.txt");
    if(!infile.is_open())
    {
        throw std::runtime_error("Not able to open file");
    }

    std::stringstream buffer;
    buffer << infile.rdbuf();

    auto tokens = trylang::Lexer::Tokenizer(std::move(buffer.str()));
    if(tokens.size() == 0)
    {
        return;
    }

    auto compilationUnitSyntax = trylang::Parser::AST(std::move(tokens));
    if(!compilationUnitSyntax)
    {
        return;
    }

    auto program = trylang::Binder::BindProgram(compilationUnitSyntax.get());

    if(!program)
    {
        return;
    }
    else
    {
        trylang::PrettyPrintSyntaxNodes(compilationUnitSyntax.get());
        trylang::PrettyPrintBoundNodes((program->_statement.get()));
        trylang::NodePrinter::Write(program->_statement.get());

        // return;

        trylang::Evaluator evaluator(std::move(program));
        trylang::object_t result = evaluator.Evaluate();
        if(result.has_value())
        {
            std::visit(trylang::PrintVisitor{}, *result);
        }
        std::cout << "\n";
    }
}