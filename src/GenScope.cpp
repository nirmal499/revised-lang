#include <codeanalysis/GenScope.hpp>
#include <stdexcept>

namespace trylang
{

    GenScope::GenScope(const std::unordered_map<std::string, llvm::Value*>& record, const std::shared_ptr<GenScope>& parent)
        : _record(record),
          _parent(parent)
    {
        
    }

    std::shared_ptr<GenScope> GenScope::Resolve(const std::string& name)
    {
        auto it = _record.find(name);
        if(it != _record.end())
        {
            /* We found the defined variable in our environment */
            return this->shared_from_this();
        }

        if(_parent == nullptr)
        {
            throw std::runtime_error("Variable " + name + " is not defined");
        }

        /* If we did not found the variable in our environment then check it in our parent environment */
        return _parent->Resolve(name);
    }

    llvm::Value* GenScope::LookUp(const std::string& name)
    {
        /* searches variable in the whole environment chain */
        return this->Resolve(name)->_record[name];
    }

    llvm::Value* GenScope::Define(const std::string& name, llvm::Value* value)
    {
        _record[name] = value;
        return value;
    }

}

