#pragma once

#include <optional>
#include <variant>
#include <codeanalysis/Symbol.hpp>
#include <unordered_map>
#include <iostream>

namespace trylang
{
    /*
     *  If you make changes to oobject_t, you will have to add support for its
     *  typeinfo in BoundExpressionNode.cpp AND in PrintVisitor{}
     *
     *  NOTE: make sure to use primitive types in oobject_t because the we are typeid().name() which is like
     *
     *  int -> typeid().name() is "i"
     *  bool -> typeid().name() is "b"
     *  double -> typeid().name() is "d"
     *  std::string  -> typeid().name() is "NSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE"
     *
     *  https://en.cppreference.com/w/cpp/types/type_info/name
    */
    typedef std::variant<int, bool, std::string> oobject_t;
    typedef std::optional<oobject_t> object_t;

    typedef std::unordered_map<std::string, trylang::object_t> variable_map_t;

    struct BoolConvertVisitor
    {
        bool operator()(int number)
        {
            if(number == 0)
            {
                return false;
            }

            return true;
        }

        bool operator()(const std::string& str)
        {
            if(str.empty())
            {
                return false;
            }

            return true;
        }

        bool operator()(bool boolValue)
        {
            return boolValue;
        }
    };

    struct IntConvertVisitor
    {
        int operator()(int number)
        {
           return number;
        }

        int operator()(const std::string& str)
        {
            int value;

            try
            {
                value = std::stoi(str);

            }catch (const std::exception& e)
            {
                /*throw std::logic_error("Invalid String value provided for int conversion");*/
                value = -1;
            }

            return value;
        }

        int operator()(bool boolValue)
        {
            if(boolValue)
            {
                return 1;
            }

            return 0;
        }
    };

    struct StringConvertVisitor
    {
        std::string operator()(int number)
        {
            return std::to_string(number);
        }

        std::string operator()(const std::string& str)
        {
            return str;
        }

        std::string operator()(bool boolValue)
        {
            if(boolValue)
            {
                return "true";
            }

            return "false";
        }
    };

    struct PrintVisitor
    {
        void operator()(int number)
        {
            std::cout << " " << number;
        }

        void operator()(bool boolValue)
        {
            std::cout << std::boolalpha << boolValue;
        }

        void operator()(const std::string& str)
        {
            std::cout << str;
        }
    };
}