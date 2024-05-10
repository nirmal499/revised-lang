#pragma once

#include <string>
#include <typeinfo>

namespace trylang
{
    struct VariableSymbol
    {
        std::string _name{};
        const char* _type = nullptr;
        bool _isReadOnly = false; /* Variable is declared by let keyword instead of var */

        VariableSymbol() = default;

        VariableSymbol(std::string name, bool isReadOnly, const char* type)
                : _name(std::move(name)), _type(type), _isReadOnly(isReadOnly)
        {}

        /* No need for this, since we are not be using VariableSymbol as key in an unordered_map */
        bool operator==(const VariableSymbol& other) const
        {
            return _name == other._name && _type == other._type;
        }
    };

    /* Custom hash function for VariableSymbol */
    struct VariableSymbolHash
    {
        std::size_t operator()(const VariableSymbol& var) const
        {
            // Combine hash of name and hash code of type_info
            return std::hash<std::string>{}(var._name) ^ (std::hash<std::string>{}(var._type) << 1);
        }
    };

}