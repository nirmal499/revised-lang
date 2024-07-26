#include "codeanalysis/Generator.hpp"
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

void Run3(const char* file_name);

int main(int argc, char* argv[])
{
    try
    {
        Run3(argv[1]);
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

void Run3(const char* file_name)
{
    std::string errors;
    bool generateIR = true;

    /* ROOT_PATH is set by the CMake */
    std::string sourceFilePath = std::string(ROOT_PATH) + "/source_file/" + file_name;
    std::string llvmFilePath = std::string(ROOT_PATH) + "/llvm_ir/" + file_name;

    std::ifstream infile(sourceFilePath);
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


    if(!generateIR)
    {
        auto program = trylang::Binder::BindProgram(compilationUnitSyntax.get(), true);

        if(!program)
        {
            return;
        }
        else
        {
            // std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::SyntaxTree::::::::::::::::::::::::::::::::::::::::::\n";
            // trylang::PrettyPrintSyntaxNodes(compilationUnitSyntax.get());
            // std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::BoundTree For WHOLE:::::::::::::::::::::::::::::::::::::::::::\n";
            // trylang::PrettyPrintBoundNodes((program->_statement.get()));
            // std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::BoundTree For FUNCTIONS:::::::::::::::::::::::::::::::::::::::::::\n";
            // trylang::PrettyPrintBoundNodesForFunctionBodies(program->_functionsInfoAndBody);
            // std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::WHOLE:::::::::::::::::::::::::::::::::::::::::::::::\n";
            // trylang::NodePrinter::Write(program->_statement.get());
            // std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::FUNCTIONS:::::::::::::::::::::::::::::::::::::::::::\n";
            // trylang::NodePrinter::WriteFunctions(program->_functionsInfoAndBody);

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
    else
    {
        auto program = trylang::Binder::BindProgram(compilationUnitSyntax.get(), false);

        if(!program)
        {
            return;
        }
        else
        {
            // std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::SyntaxTree::::::::::::::::::::::::::::::::::::::::::\n";
            // trylang::PrettyPrintSyntaxNodes(compilationUnitSyntax.get());
            // std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::BoundTree For WHOLE:::::::::::::::::::::::::::::::::::::::::::\n";
            // trylang::PrettyPrintBoundNodes((program->_statement.get()));
            // std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::BoundTree For FUNCTIONS:::::::::::::::::::::::::::::::::::::::::::\n";
            // trylang::PrettyPrintBoundNodesForFunctionBodies(program->_functionsInfoAndBody);
            // std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::LLVM IR:::::::::::::::::::::::::::::::::::::::::::\n";
            trylang::Generator::GenerateProgram(program.get(), llvmFilePath.c_str());
        }
    }
}