#pragma once

#include <optional>
#include <variant>

namespace trylang
{
    typedef std::variant<int> oobject_t;
    typedef std::optional<oobject_t> object_t;
}