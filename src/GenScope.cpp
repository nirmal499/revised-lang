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

    llvm::Value* GenScope::LookUp(const std::string& name, bool strictCheck)
    {
        /* searches variable in the whole environment chain */

        if(strictCheck)
        {
            /*
                If strictCheck is true then we will throw incase if the name is not found in _record
            */
            try
            {
                std::shared_ptr<GenScope> resolvedScope = this->Resolve(name);
                return resolvedScope->_record[name];
            }
            catch(const std::exception& e)
            {
                throw e;
            }
        }
        else
        {
            /*
                If strictCheck is false then we will not throw instead of that we will return nullptr
            */
            try
            {
                std::shared_ptr<GenScope> resolvedScope = this->Resolve(name);
                return resolvedScope->_record[name];
            }
            catch(const std::exception& e)
            {
                return nullptr;
            }
        }
        

        /* This is unreachable */
        return nullptr;
    }

    llvm::Value* GenScope::Define(const std::string& name, llvm::Value* value)
    {
        _record[name] = value;
        return value;
    }

}

