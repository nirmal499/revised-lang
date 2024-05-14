#pragma once

#include <string>
#include <typeinfo>
#include <memory>
#include <unordered_map>
#include <vector>

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
        inline std::unique_ptr<TypeSymbol> VOID = std::make_unique<TypeSymbol>("void");
        inline std::unique_ptr<TypeSymbol> ERROR = std::make_unique<TypeSymbol>("?");
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

    struct ParameterSymbol : VariableSymbol
    {
        ParameterSymbol(std::string name, const char* type)
        : VariableSymbol(std::move(name), true, type)
        {}
    };

    struct FunctionSymbol
    {
        std::string _name{};
        const char* _type = nullptr; /* Return type of function */
        std::vector<ParameterSymbol> _parameters;

        FunctionSymbol() = default;

        FunctionSymbol(std::string name, std::vector<ParameterSymbol> parameters, const char* type)
                : _name(std::move(name)), _type(type), _parameters(std::move(parameters))
        {}

        bool operator==(const FunctionSymbol& other) const
        {
            return _name == other._name && _type == other._type;
        }
    };

    namespace BUILT_IN_FUNCTIONS
    {
        inline std::unordered_map<std::string, FunctionSymbol> MAP = {
                    {"print", FunctionSymbol{"print", std::vector<ParameterSymbol>{ParameterSymbol("text", Types::STRING->Name())}, Types::VOID->Name()}},
                    {"input", FunctionSymbol{"input", std::vector<ParameterSymbol>{}, Types::STRING->Name()}}};
    }

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

    /* Custom hash function for VariableSymbol */
    struct FunctionSymbolHash
    {
        std::size_t operator()(const FunctionSymbol& var) const
        {
            // Combine hash of name and hash code of type_info
            return std::hash<std::string>{}(var._name) ^ (std::hash<std::string>{}(var._type) << 1);
        }
    };

}