#pragma once

#include <memory>
#include <codeanalysis/utils/Symbol.hpp>
#include <cstring>

namespace trylang
{
    struct Conversion
    {
        bool _exists;
        bool _isIdentity;
        bool _isImplicit;
        bool _isExplicit;

        Conversion(bool exists, bool isIdentity, bool isImplicit):
                _exists(exists), _isIdentity(isIdentity), _isImplicit(isImplicit)
        {
            _isExplicit = _exists && !_isImplicit;
        }
    };

    namespace CONVERSION_TYPES
    {
        inline std::unique_ptr<Conversion> NONE = std::make_unique<Conversion>(false, false, false);
        inline std::unique_ptr<Conversion> IDENTITY = std::make_unique<Conversion>(true, true, true);
        inline std::unique_ptr<Conversion> IMPLICIT = std::make_unique<Conversion>(true, false, true);
        inline std::unique_ptr<Conversion> EXPLICIT = std::make_unique<Conversion>(true, false, false);
    }

    inline Conversion* Classify(const char* from, const char* to)
    {
        if(std::strcmp(from, to) == 0)
        {
            return CONVERSION_TYPES::IDENTITY.get();
        }

        /* bool or int to string */
        if((std::strcmp(from, Types::BOOL->Name()) == 0 || (std::strcmp(from,Types::INT->Name())) == 0))
        {
            if(std::strcmp(to, Types::STRING->Name()) == 0)
            {
                return CONVERSION_TYPES::EXPLICIT.get();
            }
        }

        /* string to bool or int */
        if(std::strcmp(from, Types::STRING->Name()) == 0)
        {
            if((std::strcmp(to, Types::BOOL->Name()) == 0 || (std::strcmp(to,Types::INT->Name())) == 0))
            {
                return CONVERSION_TYPES::EXPLICIT.get();
            }
        }

        return CONVERSION_TYPES::NONE.get();
    }
}

