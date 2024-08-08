#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <codeanalysis/utils/Symbol.hpp>
#include <codeanalysis/binder/utils/BoundExpressionNode.hpp>

namespace trylang
{
    struct BoundProgram
    {
        std::unordered_map<std::string, std::pair<std::shared_ptr<FunctionSymbol>, std::unique_ptr<BoundBlockStatement>>> _functionsInfoAndBody;
        std::unordered_map<std::string, std::shared_ptr<VariableSymbol>> _variables;
        std::unique_ptr<BoundBlockStatement> _statement;

        BoundProgram(
                    std::unordered_map<std::string, std::shared_ptr<VariableSymbol>>&& variables,
                    std::unordered_map<std::string, std::pair<std::shared_ptr<FunctionSymbol>, std::unique_ptr<BoundBlockStatement>>> functionsInfoAndBody,
                    std::unique_ptr<BoundBlockStatement> statement
                ) : _functionsInfoAndBody(std::move(functionsInfoAndBody)), _variables(std::move(variables)), _statement(std::move(statement))
        {
        }
        
    };
}