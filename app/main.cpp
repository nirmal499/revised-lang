// #include <codeanalysis/Generator.hpp>
#include <codeanalysis/parser/utils/SyntaxKind.hpp>
#include <codeanalysis/lexer/Lexer.hpp>
#include <codeanalysis/parser/Parser.hpp>
#include <codeanalysis/binder/Binder.hpp>
#include <codeanalysis/binder/utils/BoundProgram.hpp>
#include <codeanalysis/evaluator/Evaluator.hpp>
#include <codeanalysis/parser/utils/ExpressionSyntax.hpp>
#include <codeanalysis/printer/BoundNodePrinter.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <boost/program_options.hpp>

int main(int argc, char* argv[])
{
    /* Define the command-line options */
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("input,i", boost::program_options::value<std::string>()->required(), "input file");

    boost::program_options::variables_map vm;
    try {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);

        /* Check if help is requested */
        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return EXIT_SUCCESS;
        }

        /* Retrieve the filename */
        std::string filename = vm["input"].as<std::string>();
        // std::cout << "Input file: " << filename << std::endl;

        /* Handling the file */
        std::ifstream infile(filename);
        if(!infile.is_open())
        {
            throw std::runtime_error("Not able to open file");
        }

        std::stringstream buffer;
        buffer << infile.rdbuf();

        auto tokens = trylang::Lexer::Tokenizer(std::move(buffer.str()));
        if(tokens.size() == 0)
        {
            throw std::runtime_error("No Tokens Present");
        }

        auto compilationUnitSyntax = trylang::Parser::AST(std::move(tokens));
        if(!compilationUnitSyntax)
        {
            throw std::runtime_error("Error at Parser");
        }

        auto program = trylang::Binder::BindProgram(compilationUnitSyntax.get());
        if(!program)
        {
            throw std::runtime_error("Error at Binder");
        }

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

        trylang::Evaluator evaluator(std::move(program));
        trylang::object_t result = evaluator.Evaluate();
        if(result.has_value())
        {
            std::visit(trylang::PrintVisitor{}, *result);
        }
        std::cout << "\n";

    } catch (const boost::program_options::error& e) {

        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << desc << std::endl;

        return EXIT_FAILURE;

    } catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
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

// void Run3(char* argv[])
// {
//     const char* file_name = argv[1];
//     const char* flagToGenerateIR = argv[2];

//     bool generateIR;
//     if(strcmp(flagToGenerateIR, "ir") == 0)
//     {
//         generateIR = true;
//     }
//     else if(strcmp(flagToGenerateIR, "nonir") == 0)
//     {
//         generateIR = false;
//     }
//     else
//     {
//         throw std::runtime_error("flagToGenerateIR: can contain either 'true' or 'false'.");
//     }

//     std::string errors;

//     /* ROOT_PATH is set by the CMake */
//     std::string sourceFilePath = std::string(ROOT_PATH) + "/source_file/" + file_name;
//     std::string llvmFilePath = std::string(ROOT_PATH) + "/llvm_ir/" + file_name;

//     std::ifstream infile(sourceFilePath);
//     if(!infile.is_open())
//     {
//         throw std::runtime_error("Not able to open file");
//     }

//     std::stringstream buffer;
//     buffer << infile.rdbuf();

//     auto tokens = trylang::Lexer::Tokenizer(std::move(buffer.str()));
//     if(tokens.size() == 0)
//     {
//         return;
//     }

//     auto compilationUnitSyntax = trylang::Parser::AST(std::move(tokens));
//     if(!compilationUnitSyntax)
//     {
//         return;
//     }


//     if(!generateIR)
//     {
//         auto program = trylang::Binder::BindProgram(compilationUnitSyntax.get(), true);

//         if(!program)
//         {
//             return;
//         }
//         else
//         {
//             // std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::SyntaxTree::::::::::::::::::::::::::::::::::::::::::\n";
//             // trylang::PrettyPrintSyntaxNodes(compilationUnitSyntax.get());
//             // std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::BoundTree For WHOLE:::::::::::::::::::::::::::::::::::::::::::\n";
//             // trylang::PrettyPrintBoundNodes((program->_statement.get()));
//             // std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::BoundTree For FUNCTIONS:::::::::::::::::::::::::::::::::::::::::::\n";
//             // trylang::PrettyPrintBoundNodesForFunctionBodies(program->_functionsInfoAndBody);
//             // std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::WHOLE:::::::::::::::::::::::::::::::::::::::::::::::\n";
//             // trylang::NodePrinter::Write(program->_statement.get());
//             // std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::FUNCTIONS:::::::::::::::::::::::::::::::::::::::::::\n";
//             // trylang::NodePrinter::WriteFunctions(program->_functionsInfoAndBody);

//             trylang::Evaluator evaluator(std::move(program));
//             trylang::object_t result = evaluator.Evaluate();
//             if(result.has_value())
//             {
//                 std::visit(trylang::PrintVisitor{}, *result);
//             }
//             std::cout << "\n";
//         }
//     }
//     else
//     {
//         auto program = trylang::Binder::BindProgram(compilationUnitSyntax.get(), false);

//         if(!program)
//         {
//             return;
//         }
//         else
//         {
//             // std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::SyntaxTree::::::::::::::::::::::::::::::::::::::::::\n";
//             // trylang::PrettyPrintSyntaxNodes(compilationUnitSyntax.get());
//             // std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::BoundTree For WHOLE:::::::::::::::::::::::::::::::::::::::::::\n";
//             // trylang::PrettyPrintBoundNodes((program->_statement.get()));
//             // std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::BoundTree For FUNCTIONS:::::::::::::::::::::::::::::::::::::::::::\n";
//             // trylang::PrettyPrintBoundNodesForFunctionBodies(program->_functionsInfoAndBody);
//             // std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::LLVM IR:::::::::::::::::::::::::::::::::::::::::::\n";
//             // trylang::Generator::GenerateProgram(program.get(), llvmFilePath.c_str());
//         }
//     }
// }