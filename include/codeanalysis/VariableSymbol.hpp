#pragma once

#include <string>
#include <typeinfo>
#include <memory>
#include <unordered_map>

namespace trylang
{

    struct TypeSymbol
    {
        const char* _typeName;

        explicit TypeSymbol(const char* typeName)
                : _typeName(typeName)
        {}

        const char* Name()
        {
            return _typeName;
        }
    };

    namespace Types
    {
        inline std::unique_ptr<TypeSymbol> INT = std::make_unique<TypeSymbol>("int");
        inline std::unique_ptr<TypeSymbol> BOOL = std::make_unique<TypeSymbol>("bool");
        inline std::unique_ptr<TypeSymbol> STRING = std::make_unique<TypeSymbol>("string");
    }

    struct VariableSymbol
    {
        std::string _name{};
        const char* _type = nullptr;
        bool _isReadOnly = false; /* Variable is declared by let keyword instead of var */

        VariableSymbol() = default;

        VariableSymbol(std::string name, bool isReadOnly, const char* type)
                : _name(std::move(name)), _type(type), _isReadOnly(isReadOnly)
        {}

        bool operator==(const VariableSymbol& other) const
        {
            return _name == other._name && _type == other._type;
        }
    };

    struct LabelSymbol
    {
        std::string _name{};

        LabelSymbol() = default;
        explicit LabelSymbol(std::string name)
                : _name(std::move(name))
        {}

        bool operator==(const LabelSymbol& other) const
        {
            return _name == other._name;
        }
    };

    /* Custom hash function for VariableSymbol */
    struct LabelSymbolHash
    {
        std::size_t operator()(const LabelSymbol& var) const
        {
            // hash of name
            return std::hash<std::string>{}(var._name);
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