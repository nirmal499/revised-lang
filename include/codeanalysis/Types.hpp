#pragma once

#include <optional>
#include <variant>
#include <codeanalysis/Symbol.hpp>
#include <unordered_map>

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
}