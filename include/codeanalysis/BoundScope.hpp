#pragma once

#include <memory>
#include <string>
#include <codeanalysis/VariableSymbol.hpp>
#include <unordered_map>
#include <vector>
#include <codeanalysis/BoundExpressionNode.hpp>

namespace trylang
{
    struct BoundScope
    {
        std::shared_ptr<BoundScope> _parent;
        std::unordered_map<std::string, std::unique_ptr<VariableSymbol>> _variables;

        explicit  BoundScope(const std::shared_ptr<BoundScope>& parent)
         : _parent(parent)
        {}

        bool TryDeclare(const VariableSymbol& variable)
        {
            auto it = _variables.find(variable._name);
            if(it != _variables.end())
            {
                return false;
            }

            _variables[variable._name] = std::make_unique<VariableSymbol>(variable);

            return true;
        }
        bool TryLookUp(const std::string& name,VariableSymbol& variable)
        {
            auto it = _variables.find(name);
            if(it != _variables.end())
            {
                variable = *it->second;
                return true;
            }

            if(_parent == nullptr)
            {
                return false;
            }

            return _parent->TryLookUp(name, variable);
        }

        std::vector<std::string> GetDeclaredVariable()
        {
            std::vector<std::string> keys(_variables.size());
            /* Extract keys and return it */
            for(auto const& pair: _variables)
            {
                keys.push_back(pair.first);
            }

            return keys; // RVO
        }
    };

    struct BoundGlobalScope
    {
        std::shared_ptr<BoundGlobalScope> _previous;
        std::string _errors;
        std::vector<std::string> _variables;
        std::unique_ptr<BoundStatementNode> _statement;

        BoundGlobalScope(
                const std::shared_ptr<BoundGlobalScope>& previous,
                std::string errors,
                std::vector<std::string> variables,
                std::unique_ptr<BoundStatementNode> statement)
                : _previous(previous), _errors(std::move(errors)), _variables(std::move(variables)), _statement(std::move(statement))
        {}
    };
}