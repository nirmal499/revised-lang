#pragma once

#include <string>
#include <typeinfo>

namespace trylang
{
    struct VariableSymbol
    {
        std::string _name;
        const std::type_info& _type;

        VariableSymbol(std::string name, const std::type_info& type)
                : _name(std::move(name)), _type(type)
        {}

        bool operator==(const VariableSymbol& other) const
        {
            return _name == other._name;
        }
    };

    /* Custom hash function for VariableSymbol */
    struct VariableSymbolHash
    {
        std::size_t operator()(const VariableSymbol& var) const
        {
            // Combine hash of name and hash code of type_info
            return std::hash<std::string>{}(var._name) ^ (std::hash<std::string>{}(var._type.name()) << 1);
        }
    };
}