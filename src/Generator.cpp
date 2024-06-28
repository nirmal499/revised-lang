#include "codeanalysis/Evaluator.hpp"
#include "codeanalysis/GenScope.hpp"
#include "codeanalysis/Symbol.hpp"
#include <codeanalysis/BoundExpressionNode.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/Generator.hpp>
#include <codeanalysis/BoundScope.hpp>
#include <codeanalysis/Types.hpp>
#include <cstring>
#include <llvm-14/llvm/IR/Value.h>
#include <memory>
#include <stack>
#include <random>
#include <stdexcept>
#include <variant>
#include "llvm/IR/Verifier.h"

namespace trylang
{
    void Generator::GenerateStatement(BoundStatementNode* node)
    {
        if(node == nullptr)
        {
            return;
        }
        
        switch (node->Kind())
        {
            case BoundNodeKind::BlockStatement:
                return GenerateBlockStatement(static_cast<BoundBlockStatement*>(node));
            case BoundNodeKind::VariableDeclarationStatement:
                return GenerateVariableDeclaration(static_cast<BoundVariableDeclaration*>(node));
            case BoundNodeKind::IfStatement:
                return GenerateIfStatement(static_cast<BoundIfStatement*>(node));
            case BoundNodeKind::WhileStatement:
                return GenerateWhileStatement(static_cast<BoundWhileStatement*>(node));
            case BoundNodeKind::ForStatement:
                return GenerateForStatement(static_cast<BoundForStatement*>(node));
            case BoundNodeKind::ConditionalGotoStatement:
                return;
            case BoundNodeKind::LabelStatement:
                return;
            case BoundNodeKind::GotoStatement:
                return;
            case BoundNodeKind::ExpressionStatement:
                return GenerateExpressionStatement(static_cast<BoundExpressionStatement*>(node));
            case BoundNodeKind::ReturnStatement:
                return GenerateReturnStatement(static_cast<BoundReturnStatement*>(node));
            default:
                throw std::logic_error("Unexpected syntax " + __boundNodeStringMap[node->Kind()]);
        }
    }

    void Generator::GenerateBlockStatement(BoundBlockStatement* node)
    {
        _scope = std::make_shared<GenScope>(std::unordered_map<std::string, llvm::Value*>{}, _scope);

        for(auto const& stmt: node->_statements)
        {
            this->GenerateStatement(stmt.get());
        }

        _scope = _scope->_parent;
    }

    void Generator::GenerateVariableDeclaration(BoundVariableDeclaration* node)
    {
        /* locals are allocated on the stack */
        auto varname = node->_variable->_name;

        assert(node->_expression != nullptr);
        llvm::Value* value = this->GenerateExpression(node->_expression.get());
        assert(value != nullptr);

        auto vartype = this->ExtractType(node->_variable->_type);
        auto varbinding = this->AllocVariable(varname, vartype);

        /* If the variable is defined earlier then it will be overidden */
        _scope->Define(varname, value);

        _builder->CreateStore(value, varbinding);
    }

    void Generator::GenerateIfStatement(BoundIfStatement* node)
    {
        llvm::Value* cond = this->GenerateExpression(node->_condition.get());

        auto thenBlock = llvm::BasicBlock::Create(*_ctx, "then", _function);
        auto elseBlock = llvm::BasicBlock::Create(*_ctx, "else", nullptr);
        auto ifEndBlock = llvm::BasicBlock::Create(*_ctx, "ifend", nullptr);

        /* condition branch */
        _builder->CreateCondBr(cond, thenBlock, elseBlock);

        /* then branch */
        _builder->SetInsertPoint(thenBlock);
        this->GenerateStatement(node->_statement.get());
        _builder->CreateBr(ifEndBlock);

        if(node->_elseStatement != nullptr)
        {
            /* else branch */
            _function->getBasicBlockList().push_back(elseBlock); /* Append the block to the function now*/
            _builder->SetInsertPoint(elseBlock);
            this->GenerateStatement(node->_elseStatement.get());
            _builder->CreateBr(ifEndBlock);
        }

        _function->getBasicBlockList().push_back(ifEndBlock);
        /* if-end block */
        _builder->SetInsertPoint(ifEndBlock);

        /*
            Result of the if expression is phi
            This was in the case where each block generates a value no matter what happens
            
            auto phi = _builder->CreatePHI(_builder->getInt32Ty(), 2, "tmpif");
            phi->addIncoming(thenRes, thenBlock);
            phi->addIncoming(elseRes, elseBlock);

            return phi;
        */
    }

    void Generator::GenerateWhileStatement(BoundWhileStatement* node)
    {
        auto condBlock = llvm::BasicBlock::Create(*_ctx, "cond", _function);
        _builder->CreateBr(condBlock);

        /* body, while-end blocks: */
        auto bodyBlock = llvm::BasicBlock::Create(*_ctx, "body", nullptr);
        auto loopEndBlock = llvm::BasicBlock::Create(*_ctx, "loopend", nullptr);

        /* compile <cond> IR */
        _builder->SetInsertPoint(condBlock);
        llvm::Value* cond = this->GenerateExpression(node->_condition.get());

        _builder->CreateCondBr(cond, bodyBlock, loopEndBlock);

        /* Body */
        _function->getBasicBlockList().push_back(bodyBlock);
        _builder->SetInsertPoint(bodyBlock);
        this->GenerateStatement(node->_body.get());
        _builder->CreateBr(condBlock);

        _function->getBasicBlockList().push_back(loopEndBlock);
        _builder->SetInsertPoint(loopEndBlock);
    }

    void Generator::GenerateForStatement(BoundForStatement* node)
    {
        auto variableDeclaration = std::make_unique<BoundVariableDeclaration>(node->_variable, std::move(node->_lowerBound));
        auto upperBoundVariableDeclaration = std::make_unique<BoundVariableDeclaration>(node->_variableForUpperBoundToBeUsedDuringRewritingForIntoWhile, std::move(node->_upperBound));

        auto condition = std::make_unique<BoundBinaryExpression>(
                std::make_unique<BoundVariableExpression>(node->_variable),
                BoundBinaryOperator::Bind(SyntaxKind::LessThanEqualsToken, Types::INT->Name(), Types::INT->Name()),
                std::make_unique<BoundVariableExpression>(node->_variableForUpperBoundToBeUsedDuringRewritingForIntoWhile)
        );
        // auto continueLabelStatement = std::make_unique<BoundLabelStatement>(node->_loopLabel.second);
        auto increment = std::make_unique<BoundExpressionStatement>(
                std::make_unique<BoundAssignmentExpression>(
                        node->_variable,
                        std::make_unique<BoundBinaryExpression>(
                                std::make_unique<BoundVariableExpression>(node->_variable),
                                BoundBinaryOperator::Bind(SyntaxKind::PlusToken, Types::INT->Name(),Types::INT->Name()),
                                std::make_unique<BoundLiteralExpression>(1))
                ));

        /** This has to be done instead of doing std::make_unique<BoundBlockStatement>({std::move(body), std::move(increment)}) because BoundBlockStatement is explicit */
        std::vector<std::unique_ptr<BoundStatementNode>> statements_1(3);
        statements_1.emplace_back(std::move(node->_body));
        // statements_1.emplace_back(std::move(continueLabelStatement));
        statements_1.emplace_back(std::move(increment));

        auto whileBody = std::make_unique<BoundBlockStatement>(std::move(statements_1));

        // node->_loopLabel.second = LabelSymbol("continue{" + GenerateRandomText(3) + "}");
        auto whileStatement = std::make_unique<BoundWhileStatement>(std::move(condition), std::move(whileBody),std::pair<LabelSymbol, LabelSymbol>{});

        std::vector<std::unique_ptr<BoundStatementNode>> statements_2(3);
        statements_2.emplace_back(std::move(variableDeclaration));
        statements_2.emplace_back(std::move(upperBoundVariableDeclaration));
        statements_2.emplace_back(std::move(whileStatement));

        auto result = std::make_unique<BoundBlockStatement>(std::move(statements_2));

        this->GenerateStatement(result.get());
    }


    void Generator::GenerateReturnStatement(BoundReturnStatement* node)
    {
        assert(node->_expression != nullptr);
        llvm::Value* value = this->GenerateExpression(node->_expression.get());

        _builder->CreateRet(value);
    }

    void Generator::GenerateExpressionStatement(BoundExpressionStatement* node)
    {
        this->GenerateExpression(node->_expression.get());
    }

    llvm::Value* Generator::GenerateExpression(BoundExpressionNode* node)
    {
        switch (node->Kind())
        {
            case BoundNodeKind::ErrorExpression:
                /* This will be never be executed because when there is ErrorExpression, we well never reach Generator stage */
                return GenerateErrorExpression(static_cast<BoundErrorExpression*>(node));
            case BoundNodeKind::LiteralExpression:
                return GenerateLiteralExpression(static_cast<BoundLiteralExpression*>(node));
            case BoundNodeKind::VariableExpression:
                return GenerateVariableExpression(static_cast<BoundVariableExpression*>(node));
            case BoundNodeKind::AssignmentExpression:
                return GenerateAssignmentExpression(static_cast<BoundAssignmentExpression*>(node));
            case BoundNodeKind::UnaryExpression:
                return GenerateUnaryExpression(static_cast<BoundUnaryExpression*>(node));
            case BoundNodeKind::BinaryExpression:
                return GenerateBinaryExpression(static_cast<BoundBinaryExpression*>(node));
            case BoundNodeKind::CallExpression:
                return GenerateCallExpression(static_cast<BoundCallExpression*>(node));
            case BoundNodeKind::ConversionExpression:
                return GenerateConversionExpression(static_cast<BoundConversionExpression*>(node));
            default:
                throw std::logic_error("Unexpected syntax " + __boundNodeStringMap[node->Kind()]);

        }
    }

    llvm::Value* Generator::GenerateErrorExpression(BoundErrorExpression* node)
    {
        assert(false);
        return nullptr;
    }

    llvm::Value* Generator::GenerateLiteralExpression(BoundLiteralExpression* node)
    {
        assert(node->_value.has_value());
        llvm::Value* value = std::visit(GetValueVisitor{_builder.get()}, *(node->_value));
        return value;
    }

    llvm::Value* Generator::GenerateVariableExpression(BoundVariableExpression* node)
    {
        auto varname = node->_variable->_name;
        llvm::Value* value = _scope->LookUp(varname); /* It throws std::runtime_error */

        /* Local Variable */
        if(auto local_var = llvm::dyn_cast<llvm::AllocaInst>(value))
        {
            /* We need to load them in current stack frame */
            return _builder->CreateLoad(
                local_var->getAllocatedType(),
                local_var, /* load to variable name */ varname.c_str()
            );
        }

        /* Global variables */
        if(auto global_var = llvm::dyn_cast<llvm::GlobalVariable>(value))
        {
            /* We need to load */
            return _builder->CreateLoad(
                global_var->getInitializer()->getType(), 
                global_var, /*load to variable name */ varname.c_str()
            );
        }

        assert(false); /* We must not reach here becoz we are doing LookUp for function in GenerateCallExpression */

        return value; /* Reachable in case value is llvm::Function* */
    }

    llvm::Value* Generator::GenerateAssignmentExpression(BoundAssignmentExpression* node)
    {
        llvm::Value* value = this->GenerateExpression(node->_expression.get());

        _scope->Define(node->_variable->_name, value);

        return value;
    }

    llvm::Value* Generator::GenerateCallExpression(BoundCallExpression* node)
    {
        /* We get callee as a function */
        // llvm::Value* callee = this->GenerateExpression(node); /* It falls in recursion */
        
        const std::string& functionName = node->_function->_name;
        llvm::Value* callee = nullptr;

        if(functionName == "print")
        {
            callee = _module->getFunction("printf");
        }
        else if(functionName == "input")
        {
            callee = _module->getFunction("fgets");
        }
        else
        {
            callee = _scope->LookUp(functionName); /* It throws std::runtime_error */
        }

        assert(callee != nullptr);

        std::vector<llvm::Value*> args{};
        for(auto const& argument: node->_arguments)
        {
            args.emplace_back(this->GenerateExpression(argument.get()));
        }

        auto fn = static_cast<llvm::Function*>(callee);

        return _builder->CreateCall(fn, args);
    }

    llvm::Value* Generator::GenerateConversionExpression(BoundConversionExpression* node)
    {
        auto value = _evaluator->EvaluateExpression(node);

        if(std::strcmp(node->_toType, Types::BOOL->Name()) == 0)
        {
            /* It returns bool */
            object_t convertedBoolValue = std::visit(BoolConvertVisitor{}, *value);
            return std::visit(GetValueVisitor{_builder.get()}, *convertedBoolValue);
        }

        if(std::strcmp(node->_toType, Types::INT->Name()) == 0)
        {
            /* It returns int */
            object_t convertedIntValue = std::visit(IntConvertVisitor{}, *value);
            return std::visit(GetValueVisitor{_builder.get()}, *convertedIntValue);
        }

        if(std::strcmp(node->_toType, Types::STRING->Name()) == 0)
        {
            /* It returns std::string */
            object_t convertedStringValue = std::visit(StringConvertVisitor{}, *value);
            return std::visit(GetValueVisitor{_builder.get()}, *convertedStringValue);

        }

        throw std::logic_error("Unexpected Type " + std::string(node->_toType));
    }

    llvm::Value* Generator::GenerateUnaryExpression(BoundUnaryExpression* node)
    {
        llvm::Value* operand = this->GenerateExpression(node->_operand.get());

        assert(operand != nullptr);

        switch(node->_op->Kind())
        {
            case BoundNodeKind::Identity:
                {
                    return operand;
                }
            case BoundNodeKind::Negation:
                {
                    return _builder->CreateNeg(operand);
                }
            case BoundNodeKind::LogicalNegation:
                {
                    return _builder->CreateNot(operand);
                }
            default:
                throw std::runtime_error("Unhandled switch case for " + __boundNodeStringMap[node->_op->Kind()]);
        }

    }

    llvm::Value* Generator::GenerateBinaryExpression(BoundBinaryExpression* node)
    {
        llvm::Value* left = this->GenerateExpression(node->_left.get());
        llvm::Value* right = this->GenerateExpression(node->_right.get());

        assert(left != nullptr && right != nullptr);

        switch(node->_op->Kind())
        {
            case BoundNodeKind::Greater:
                {
                    return _builder->CreateICmpUGT(left, right, "tmpcmp");
                }
            case BoundNodeKind::GreaterEquals:
                {
                    return _builder->CreateICmpUGE(left, right, "tmpcmp");
                }
            case BoundNodeKind::Less:
                {
                    return _builder->CreateICmpULT(left, right, "tmpcmp");
                }
            case BoundNodeKind::LessEquals:
                {
                    return _builder->CreateICmpULE(left, right, "tmpcmp");
                }
            case BoundNodeKind::Addition:
                {
                    return _builder->CreateAdd(left, right, "tmpadd");
                }
            case BoundNodeKind::Subtraction:
                {
                    return _builder->CreateSub(left, right, "tmpsub");
                }
            case BoundNodeKind::Multiplication:
                {
                    return _builder->CreateMul(left, right, "tmpmul");
                }
            case BoundNodeKind::Division:
                {
                    return _builder->CreateSDiv(left, right, "tmpdiv");
                }
            case BoundNodeKind::LogicalNotEquality:
                {
                    return _builder->CreateICmpNE(left, right, "tmpcmp");
                }
            case BoundNodeKind::LogicalEquality:
                {
                    return _builder->CreateICmpEQ(left, right, "tmpcmp");
                }   
            case BoundNodeKind::LogicalAnd:
                {
                    return _builder->CreateLogicalAnd(left, right, "tmpcmp");
                }
            case BoundNodeKind::LogicalOr:
                {
                    return _builder->CreateLogicalOr(left, right, "tmpcmp");
                }
            default:
                throw std::runtime_error("Unhandled switch case for " + __boundNodeStringMap[node->_op->Kind()]);
        }

        return nullptr; // Unreachable
    }

    void Generator::SaveModuleToFile(const char* IRfilePath)
    {
        std::error_code ec;
        llvm::raw_fd_ostream out(IRfilePath, ec);
        _module->print(out, nullptr);
    }

    void Generator::GenerateProgram(BoundProgram* program, const char* IRfilePath)
    {
        Generator generator(program);
    
        for(const auto& function: program->_functionsInfoAndBody)
        {
            if(function.second.first->_declaration == nullptr)
            {
                /* These functions are global functions declared by the CREATOR */
                /* They are registered in the setupGlobalExternal function */
                continue;
            }

            /* Generate IR for function defined */
            generator.GenerateFunction(program->_variables, function.second);
        }

        /* Generate IR for main function */
        generator.GenerateMain(program->_statement.get());

        generator.SaveModuleToFile(IRfilePath);
    }

    void Generator::GenerateMain(BoundBlockStatement* rootBlock)
    {
        /* Compile to LLVM IR */
        _function = this->CreateFunction("main", llvm::FunctionType::get(
                /* return type*/ _builder->getInt32Ty(),
                /* vararg */ false
        ));

        /* compile main body */
        this->GenerateBlockStatement(rootBlock);

        _builder->CreateRet(_builder->getInt32(0));

        _function = nullptr;
    }

    llvm::Value* Generator::AllocVariable(const std::string& name, llvm::Type* type)
    {
        _varsBuilder->SetInsertPoint(&_function->getEntryBlock());

        auto var_alloc = _varsBuilder->CreateAlloca(type, 0, name.c_str());

        /* Add to the environment */
        _scope->Define(name, var_alloc);

        return var_alloc;
    }

    void Generator::GenerateFunction(const std::unordered_map<std::string, std::shared_ptr<VariableSymbol>>& variables, const std::pair<std::shared_ptr<FunctionSymbol>, std::unique_ptr<BoundBlockStatement>>& functionInfoAndBody)
    {
        auto functionInfo = functionInfoAndBody.first.get();
        auto functionBody = functionInfoAndBody.second.get();

        const std::string& fnName = functionInfo->_name;
        llvm::FunctionType* fnType = this->ExtractFunctionType(functionInfo);
        auto newFn = this->CreateFunction(fnName, fnType);
        _function = newFn; /* Override _function to compile body */

        auto idx = 0; /* used for setting parameter name */

        auto newScope = std::make_shared<GenScope>(std::unordered_map<std::string, llvm::Value*>{}, _scope);

        for(auto& arg: _function->args())
        {
            auto argName = functionInfo->_parameters.at(idx)._name;

            arg.setName(argName);

            /* Allocate a local variable per argument to make arguments mutable */
            auto argBinding = this->AllocVariable(argName, arg.getType());
            _builder->CreateStore(&arg, argBinding);

            idx++;
        }

        this->GenerateBlockStatement(functionBody);
        
        /* At last we do a default create return IR. It means even if we have a return statement in "statement->body_stmts", we will have two return IR */
        // _builder->CreateRet(_builder->getInt1(true));

        /* Reset _function to nullptr */
        // m_builder->SetInsertPoint(prevBlock);
        _function = nullptr;
    }

    llvm::Function* Generator::CreateFunction(const std::string& fnName, llvm::FunctionType* fnType)
    {
        /* Function prototype might already be defined */
        auto function = _module->getFunction(fnName);

        /* If not, allocate the function */
        if(function == nullptr)
        {
            function = this->CreateFunctionProto(fnName, fnType);
        }

        auto entry = llvm::BasicBlock::Create(*_ctx, "entry", function);
        _builder->SetInsertPoint(entry);

        return function;
    }

    llvm::Function* Generator::CreateFunctionProto(const std::string& fnName, llvm::FunctionType* fnType)
    {
        auto fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, fnName, *_module);
        
        llvm::verifyFunction(*fn);

        /* Install the function in the top level environment in which the "_parent" is nullptr */
        
        /* 
            std::shared_ptr<GenScope> temp;
            while(_scope->_parent != nullptr)
            {
                _scope = _scope->_parent;
            }
        */
        assert(_scope->_parent == nullptr); /* Programmer hopes _scope->_parent == nullptr */
        _scope->Define(fnName, fn);

        return fn;
    }

    Generator::Generator(BoundProgram* program)
    {   
        _evaluator = std::make_unique<Evaluator>(nullptr);
        this->ModuleInitialization();
        this->SetupExternalFunctions();
        this->SetupGlobalEnv(program->_variables);
    }

    void Generator::ModuleInitialization()
    {
        _ctx = std::make_unique<llvm::LLVMContext>();
        _module = std::make_unique<llvm::Module>("trylang", *_ctx);

        _builder = std::make_unique<llvm::IRBuilder<>>(*_ctx);

        /* It will use it to always point to the begining of a function */
        _varsBuilder = std::make_unique<llvm::IRBuilder<>>(*_ctx);
    }

    void Generator::SetupExternalFunctions()
    {
        /* i8* to substitute for 'char*', 'void*' */
        auto bytePtrTy = _builder->getInt8Ty()->getPointerTo();

        /* int printf(const char* format, ...)*/
        _module->getOrInsertFunction("printf", llvm::FunctionType::get(
            /* return type */ _builder->getInt32Ty(),
            /* type of format arg */ bytePtrTy,
            /* vararg? */ true
        ));
        
        auto charPtrTy = _builder->getInt8Ty()->getPointerTo(); /* char* */
        auto intTy = _builder->getInt32Ty();                   /* int */
        auto filePtrTy = _builder->getInt8Ty()->getPointerTo(); /* FILE* */
        /* char *fgets(char *s, int size, FILE *stream) */
        auto fgetsFuncType = llvm::FunctionType::get(
            charPtrTy,           /* Return type: char* */
            {charPtrTy, intTy, filePtrTy}, /* Arguments: char*, int, FILE* */
            false                /* Not variadic */
        );

        _module->getOrInsertFunction("fgets", fgetsFuncType);
    }

    void Generator::SetupGlobalEnv(const std::unordered_map<std::string, std::shared_ptr<VariableSymbol>>& variables)
    {

        std::unordered_map<std::string, llvm::Value*> global_object = 
        {
            {"VERSION", _builder->getInt32(648)},
        };

        for(const auto& item: variables)
        {
            if(std::strcmp(item.second->_type, Types::INT.get()->_typeName) == 0)
            {
                global_object[item.first] = _builder->getInt32(0);
            }
            else if(std::strcmp(item.second->_type, Types::STRING.get()->_typeName) == 0)
            {
                global_object[item.first] = llvm::ConstantDataArray::getString(*_ctx, "Hello, LLVM IR!");
            }
            else if(std::strcmp(item.second->_type, Types::BOOL.get()->_typeName) == 0)
            {
                global_object[item.first] = _builder->getInt1(false);
            }
            else
            {
                throw std::runtime_error("Invalid Types Encountered!!!");
            }
        }

        std::unordered_map<std::string, llvm::Value*> global_record{};
        for(const auto& entry: global_object)
        {
            global_record[entry.first] = this->CreateGlobalVar(entry.first, static_cast<llvm::Constant*>(entry.second));
        }

        _scope = std::make_shared<GenScope>(global_record, nullptr);
    }

    llvm::GlobalVariable* Generator::CreateGlobalVar(const std::string& name, llvm::Constant* initializer)
    {
        _module->getOrInsertGlobal(name, initializer->getType());
        auto variable = _module->getNamedGlobal(name);

        // variable->setAlignment(llvm::MaybeAlign(4));
        variable->setConstant(false);
        variable->setInitializer(initializer);

        return variable;
    }

    llvm::FunctionType* Generator::ExtractFunctionType(FunctionSymbol* function)
    {
        auto returnType = this->ExtractType(function->_type);

        /* Parameters types */
        std::vector<llvm::Type*> paramTypes{};

        for(const auto& param: function->_parameters)
        {
            auto paramTy = this->ExtractType(param._type);
            paramTypes.push_back(paramTy);
        }

        return llvm::FunctionType::get(returnType, paramTypes, /* varargs */ false);
    }

    llvm::Type* Generator::ExtractType(const char* typeName)
    {
        if(std::strcmp(typeName, Types::INT.get()->_typeName) == 0)
        {
            return _builder->getInt32Ty(); /* i32 {aka int} */
        }
        else if(std::strcmp(typeName, Types::STRING.get()->_typeName) == 0)
        {
            return _builder->getInt8Ty()->getPointerTo(); /* i8* {aka char* } */
        }
        else if(std::strcmp(typeName, Types::BOOL.get()->_typeName) == 0)
        {
            return _builder->getInt1Ty(); /* i1 {aka bool} */
        }
        else
        {
            throw std::runtime_error("Invalid Type Encounter!!!");
        }
    }


}