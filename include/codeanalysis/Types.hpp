#pragma once

#include <optional>
#include <variant>

namespace trylang
{
    /* If you make changes to oobject_t, you will have to add support for its 
        typeinfo in BoundExpressionNode.cpp AND 
        in PrintVisitor{}
    */
    typedef std::variant<int, bool> oobject_t;
    typedef std::optional<oobject_t> object_t;
}