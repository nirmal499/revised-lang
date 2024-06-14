#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <codeanalysis/Types.hpp>

namespace trylang
{
    struct Environment
    {
        std::shared_ptr<Environment> _parent;
        variable_map_t _variables;

        explicit Environment(const std::shared_ptr<Environment>& parent)
         : _parent(parent)
        {}

        void Define(const std::string& name, const object_t& value)
        {
            _variables[name] = value;
        }

        bool Assign(const std::string& name, const object_t& value)
        {
            auto it = _variables.find(name);
            if(it != _variables.end())
            {
                _variables[name] = value;
                return true;
            }

            if(_parent != nullptr)
            {
                return _parent->Assign(name, value);
            }

            return false;
        }

        std::optional<object_t> LookUpVariable(const std::string& name)
        {
            auto it = _variables.find(name);
            if(it != _variables.end())
            {
                return it->second;
            }

            if(_parent == nullptr)
            {
                return std::nullopt;
            }

            return _parent->LookUpVariable(name);
        }
    };
}