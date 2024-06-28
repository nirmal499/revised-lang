#pragma once

#include <memory>
#include <string>
#include <codeanalysis/Symbol.hpp>
#include <unordered_map>
#include <vector>
#include <codeanalysis/BoundExpressionNode.hpp>

namespace trylang
{
    struct BoundScope
    {
        std::shared_ptr<BoundScope> _parent;
        std::unordered_map<std::string, std::shared_ptr<VariableSymbol>> _variables;
        std::unordered_map<std::string, std::shared_ptr<FunctionSymbol>> _functions;

        explicit BoundScope(const std::shared_ptr<BoundScope>& parent)
         : _parent(parent)
        {}

        bool TryDeclareFunction(const std::shared_ptr<FunctionSymbol>& function)
        {
            auto it = _functions.find(function->_name);
            if(it != _functions.end())
            {
                return false;
            }

             _functions[function->_name] = function;

            return true;
        }

        std::shared_ptr<FunctionSymbol> TryLookUpFunction(const std::string& name)
        {
            auto it = _functions.find(name);
            if(it != _functions.end())
            {
                return it->second;
            }

            if(_parent == nullptr)
            {
                return nullptr;
            }

            return _parent->TryLookUpFunction(name);
        }

        bool TryDeclareVariable(const std::shared_ptr<VariableSymbol>& variable)
        {
            auto it = _variables.find(variable->_name);
            if(it != _variables.end())
            {
                return false;
            }

            _variables[variable->_name] = variable;

            return true;
        }

        std::shared_ptr<VariableSymbol> TryLookUpVariable(const std::string& name)
        {
            auto it = _variables.find(name);
            if(it != _variables.end())
            {
                return it->second;
            }

            if(_parent == nullptr)
            {
                return nullptr;
            }

            return _parent->TryLookUpVariable(name);
        }
    };

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