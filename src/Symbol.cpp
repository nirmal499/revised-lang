#include <codeanalysis/Symbol.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <random>

namespace trylang
{
    const char* TypeSymbol::Name()
    {
        return _typeName;
    }

    SymbolKind TypeSymbol::Kind()
    {
        return SymbolKind::Type;
    }

    TypeSymbol* LookUpType(const std::string& name)
    {
        if(name == "int")
        {
            return Types::INT.get();
        }
        else if(name == "bool")
        {
            return Types::BOOL.get();
        }
        else if(name == "string")
        {
            return Types::STRING.get();
        }
        else
        {
            return nullptr;
        }
    }

    bool FunctionSymbol::operator==(const FunctionSymbol& other) const
    {
        return _name == other._name && _type == other._type;
    }

    SymbolKind FunctionSymbol::Kind()
    {
        return SymbolKind::Function;
    }

    LabelSymbol::LabelSymbol(std::string name): _name(std::move(name))
    {}

    bool LabelSymbol::operator==(const LabelSymbol& other) const
    {
        return _name == other._name;
    }

    std::size_t LabelSymbolHash::operator()(const LabelSymbol& var) const
    {
        // hash of name
        return std::hash<std::string>{}(var._name);
    }

    std::size_t VariableSymbolHash::operator()(const VariableSymbol& var) const
    {
        // Combine hash of name and hash code of type_info
        return std::hash<std::string>{}(var._name) ^ (std::hash<std::string>{}(var._type) << 1);
    }

    std::size_t FunctionSymbolHash::operator()(const FunctionSymbol& var) const
    {
        // Combine hash of name and hash code of type_info
        return std::hash<std::string>{}(var._name) ^ (std::hash<std::string>{}(var._type) << 1);
    }

    SymbolKind LocalVariableSymbol::Kind()
    {
        return SymbolKind::LocalVariable;
    }

    SymbolKind ParameterSymbol::Kind()
    {
        return SymbolKind::Parameter;
    }

    SymbolKind GlobalVariableSymbol::Kind()
    {
        return SymbolKind::GlobalVariable;
    }

    std::string GenerateRandomText(size_t length)
    {
        const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::default_random_engine generator(std::random_device{}());
        std::uniform_int_distribution<size_t> distribution(0, characters.size() - 1);

        std::string randomText;
        randomText.reserve(length);

        for (size_t i = 0; i < length; ++i)
        {
            randomText += characters[distribution(generator)];
        }

        return randomText;
    }
}