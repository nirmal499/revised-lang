#pragma once

#include <string>
#include <typeinfo>
#include <memory>
#include <unordered_map>
#include <vector>

namespace trylang
{
    enum class SymbolKind
    {
        Function,
        GlobalVariable,
        LocalVariable,
        Parameter,
        Type
    };

    struct FunctionDeclarationStatementSyntax;

    struct Symbol
    {
        std::string _name{};
        explicit Symbol(std::string name): _name(std::move(name)){}

        virtual SymbolKind Kind() = 0;

        /* We did not made it virtual because we did not want this to be overidden */
        std::string ToString()
        {
            return _name;
        }

        virtual ~Symbol() = default;

    };

    struct TypeSymbol : Symbol
    {
        const char* _typeName;

        explicit TypeSymbol(const char* typeName) : _typeName(typeName), Symbol(typeName){}
        const char* Name();
        SymbolKind Kind() override;
    };

    namespace Types
    {
        inline std::unique_ptr<TypeSymbol> INT = std::make_unique<TypeSymbol>("int");
        inline std::unique_ptr<TypeSymbol> BOOL = std::make_unique<TypeSymbol>("bool");
        inline std::unique_ptr<TypeSymbol> STRING = std::make_unique<TypeSymbol>("string");
        inline std::unique_ptr<TypeSymbol> VOID = std::make_unique<TypeSymbol>("void");
        inline std::unique_ptr<TypeSymbol> ERROR = std::make_unique<TypeSymbol>("?");
    }

    TypeSymbol* LookUpType(const std::string& name);

    /* Abstract Type */
    struct VariableSymbol : Symbol
    {
        const char* _type = nullptr;
        bool _isReadOnly = false; /* Variable is declared by let keyword instead of var */

        VariableSymbol(std::string name, bool isReadOnly, const char* type) : Symbol(std::move(name)), _type(type), _isReadOnly(isReadOnly){}
        ~VariableSymbol() override = default;
    };

    struct LocalVariableSymbol : VariableSymbol
    {
        LocalVariableSymbol(std::string name, bool isReadOnly, const char* type): VariableSymbol(std::move(name), isReadOnly, type){}
        SymbolKind Kind() override;
    };

    struct GlobalVariableSymbol : VariableSymbol
    {
        GlobalVariableSymbol(std::string name, bool isReadOnly, const char* type): VariableSymbol(std::move(name), isReadOnly, type){}
        SymbolKind Kind() override;
    };

    struct ParameterSymbol : LocalVariableSymbol
    {
        ParameterSymbol(std::string name, bool isReadOnly, const char* type): LocalVariableSymbol(std::move(name), isReadOnly, type){}
        SymbolKind Kind() override;
    };

    struct FunctionSymbol : Symbol
    {
        const char* _type = nullptr; /* Return type of function */
        std::vector<ParameterSymbol> _parameters;
        FunctionDeclarationStatementSyntax* _declaration = nullptr;

        FunctionSymbol(std::string name, std::vector<ParameterSymbol> parameters, const char* type, FunctionDeclarationStatementSyntax* _declaration = nullptr)
            : Symbol(std::move(name)), _type(type), _parameters(std::move(parameters)), _declaration(_declaration)
        {}

        bool operator==(const FunctionSymbol& other) const;
        SymbolKind Kind() override;
    };

    namespace BUILT_IN_FUNCTIONS
    {
        inline std::unordered_map<std::string, std::shared_ptr<FunctionSymbol>> MAP = {
                {"print", std::make_shared<FunctionSymbol>("print",std::vector<ParameterSymbol>{ParameterSymbol("text", true,Types::STRING->Name())},Types::VOID->Name())},
                {"input", std::make_shared<FunctionSymbol>("input", std::vector<ParameterSymbol>{}, Types::STRING->Name())}
        };
    }

    struct LabelSymbol
    {
        std::string _name{};

        LabelSymbol() = default;
        explicit LabelSymbol(std::string name);

        bool operator==(const LabelSymbol& other) const;
    };

    /* Custom hash function for VariableSymbol */
    struct LabelSymbolHash
    {
        std::size_t operator()(const LabelSymbol& var) const;
    };

    /* Custom hash function for VariableSymbol */
    struct VariableSymbolHash
    {
        std::size_t operator()(const VariableSymbol& var) const;
    };

    /* Custom hash function for VariableSymbol */
    struct FunctionSymbolHash
    {
        std::size_t operator()(const FunctionSymbol& var) const;
    };

    std::string GenerateRandomText(size_t length);
}