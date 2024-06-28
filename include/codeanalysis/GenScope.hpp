#pragma once

#include <memory>
#include <unordered_map>

#include "llvm/IR/Value.h"

namespace trylang
{
    struct GenScope: public std::enable_shared_from_this<GenScope>
    {
        GenScope(const std::unordered_map<std::string, llvm::Value*>& record, const std::shared_ptr<GenScope>& parent);

        std::shared_ptr<GenScope> Resolve(const std::string& name);
        llvm::Value* LookUp(const std::string& name);
        llvm::Value* Define(const std::string& name, llvm::Value* value);

        std::unordered_map<std::string, llvm::Value*> _record;
        std::shared_ptr<GenScope> _parent = nullptr;
    };
}