#include "codeanalysis/BoundNodeKind.hpp"
#include "codeanalysis/Evaluator.hpp"
#include "codeanalysis/GenScope.hpp"
#include "codeanalysis/Symbol.hpp"
#include <codeanalysis/BoundExpressionNode.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/Generator.hpp>
#include <codeanalysis/BoundScope.hpp>
#include <codeanalysis/Types.hpp>
#include <cstddef>
#include <cstring>
#include <exception>
#include <llvm-14/llvm/IR/BasicBlock.h>
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
            case BoundNodeKind::ContinueStatement:
                return GenerateContinueStatement(static_cast<BoundContinueStatement*>(node));
            case BoundNodeKind::BreakStatement:
                return GenerateBreakStatement(static_cast<BoundBreakStatement*>(node));
            // case BoundNodeKind::ForStatement:
            //     return GenerateForStatement(static_cast<BoundForStatement*>(node));
            // case BoundNodeKind::ConditionalGotoStatement:
            //     return GenerateConditionalGotoStatement(static_cast<BoundConditionalGotoStatement*>(node));;
            // case BoundNodeKind::LabelStatement:
            //     return GenerateLabelStatement(static_cast<BoundLabelStatement*>(node));;
            // case BoundNodeKind::GotoStatement:
            //     return GenerateGotoStatement(static_cast<BoundGotoStatement*>(node));;
            case BoundNodeKind::ExpressionStatement:
                return GenerateExpressionStatement(static_cast<BoundExpressionStatement*>(node));
            case BoundNodeKind::ReturnStatement:
                return GenerateReturnStatement(static_cast<BoundReturnStatement*>(node));
            default:
                throw std::logic_error("Generator: Unexpected syntax " + __boundNodeStringMap[node->Kind()]);
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

        llvm::Value* varNameValue = _scope->LookUp(varname, false);

        if(_function->getName() == "main" && varNameValue != nullptr && llvm::isa<llvm::GlobalVariable>(varNameValue))
        {
            /*
                If we are in "main" and the varName is already declared as global variable
                then we don't need to allocate varName to be a localVariable inside the "main" function
            */
            return;
        }

        assert(node->_expression != nullptr);
        llvm::Value* value = this->GenerateExpression(node->_expression.get());
        assert(value != nullptr);

        auto vartype = this->ExtractType(node->_variable->_type);
        auto varbinding = this->AllocVariable(varname, vartype);

        /*

            varDeclStmt := "var" IDENTIFIER type "=" expression ";"
                ;
            As you can see in a VariableDeclarationStatement "expression" is not optional. It is required.

            _scope->Define(varname, value);
            No need for that because we register the create local variable in _scope in this->AllocVariable(...) itself
        */

        _builder->CreateStore(value, varbinding);
    }

    // void Generator::GenerateLabelStatement(BoundLabelStatement* node)
    // {
    //     llvm::BasicBlock* labelBlock = nullptr;
    //     try
    //     {
    //         labelBlock = _labelMap.at(node->_label._name);
    //     }
    //     catch(...)
    //     {
    //         /* We do not have any BasicBlock yet created with name "node->_label._name" */
    //         labelBlock = llvm::BasicBlock::Create(*_ctx, node->_label._name, _function);
    //         _labelMap[node->_label._name] = labelBlock;
    //     }

    //     _builder->SetInsertPoint(labelBlock);
    // }

    // void Generator::GenerateGotoStatement(BoundGotoStatement* node)
    // {
    //     llvm::BasicBlock* targetLabel = nullptr;

    //     try
    //     {
    //         targetLabel = _labelMap.at(node->_label._name); /* It throws id "node->_label" is not present */
    //     }
    //     catch(...)
    //     {
    //         /* We do not have any BasicBlock yet created with name "node->_label._name" */
    //         targetLabel = llvm::BasicBlock::Create(*_ctx, node->_label._name, _function);
    //         _labelMap[node->_label._name] = targetLabel;
    //     }
        
    //     _builder->CreateBr(targetLabel);
    // }

    // void Generator::GenerateConditionalGotoStatement(BoundConditionalGotoStatement* node)
    // {
    //     llvm::Value* cond = this->GenerateExpression(node->_condition.get());

    //     llvm::BasicBlock* targetLabel = nullptr;
    //     try
    //     {
    //         targetLabel = _labelMap.at(node->_label._name); /* It throws id "node->_label" is not present */
    //     }
    //     catch(...)
    //     {
    //         /* We do not have any BasicBlock yet created with name "node->_label._name" */
    //         targetLabel = llvm::BasicBlock::Create(*_ctx, node->_label._name, _function);
    //         _labelMap[node->_label._name] = targetLabel;
    //     }

    //     llvm::BasicBlock* jumpToThisBlock = nullptr;
    //     if(node->_linkedLabel.has_value())
    //     {
    //         const auto& linkedLabel = *node->_linkedLabel;
    //         try
    //         {
    //             jumpToThisBlock = _labelMap.at(linkedLabel._name); /* It throws id "node->_label" is not present */
    //         }
    //         catch(...)
    //         {
    //             /* We do not have any BasicBlock yet created with name "linkedLabel._name" */
    //             jumpToThisBlock = llvm::BasicBlock::Create(*_ctx, linkedLabel._name, _function);
    //             _labelMap[linkedLabel._name] = jumpToThisBlock;
    //         }
    //     }

    //     // Step 3: Determine the continuation block after the conditional branch
    //     llvm::BasicBlock* continuation = llvm::BasicBlock::Create(*_ctx, "after_conditional", _function);

    //     // Create conditional branch
    //     if(node->_jumpIfFalse)
    //     {
    //         if(node->_linkedLabel.has_value())
    //         {
    //             // Jump to targetLabel if condition is false, otherwise continue
    //             _builder->CreateCondBr(_builder->CreateNot(cond), targetLabel, jumpToThisBlock);
    //         }
    //         else
    //         {
    //             _builder->CreateCondBr(_builder->CreateNot(cond), targetLabel, continuation);
    //         }
            
    //     }
    //     else
    //     {
    //         if(node->_linkedLabel.has_value())
    //         {
    //             _builder->CreateCondBr(cond, targetLabel, jumpToThisBlock);
    //         }
    //         else
    //         {
    //             // Jump to targetLabel if condition is true, otherwise continue
    //             _builder->CreateCondBr(cond, targetLabel, continuation);
    //         }
    //     }

    //     // Set the insertion point to the continuation block for subsequent instructions
    //     _builder->SetInsertPoint(continuation);
    // }


    void Generator::GenerateContinueStatement(BoundContinueStatement* node)
    {
        if(_loopStack.empty())
        {
            throw std::runtime_error("GenerateContinueStatement: _loopStack is empty");
        }

        auto bodyBasicBlockLabel = _loopStack.top().first; /* Get the "body label BasicBlock" */
        
        _builder->CreateBr(bodyBasicBlockLabel);
    }

    void Generator::GenerateBreakStatement(BoundBreakStatement* node)
    {
        if(_loopStack.empty())
        {
            throw std::runtime_error("GenerateBreakStatement: _loopStack is empty");
        }

        auto loopEndBasicBlockLabel = _loopStack.top().second; /* Get the "loopend label BasicBlock" */
        
        _builder->CreateBr(loopEndBasicBlockLabel);
    }


    void Generator::GenerateIfStatement(BoundIfStatement* node)
    {
        llvm::Value* cond = this->GenerateExpression(node->_condition.get());

        if(node->_elseStatement == nullptr)
        {
            auto thenBlock = llvm::BasicBlock::Create(*_ctx, "then", _function);
            auto ifEndBlock = llvm::BasicBlock::Create(*_ctx, "ifend", _function);

            /* condition branch */
            _builder->CreateCondBr(cond, thenBlock, ifEndBlock);

            /* then branch */
            _builder->SetInsertPoint(thenBlock);
            this->GenerateStatement(node->_statement.get());
            if(!_builder->GetInsertBlock()->getTerminator())
            {
                _builder->CreateBr(ifEndBlock);
            }
            // _builder->CreateBr(ifEndBlock);

            /* if-end block */
            _builder->SetInsertPoint(ifEndBlock);
        }
        else
        {
            auto thenBlock = llvm::BasicBlock::Create(*_ctx, "then", _function);
            auto elseBlock = llvm::BasicBlock::Create(*_ctx, "else", _function);
            auto ifEndBlock = llvm::BasicBlock::Create(*_ctx, "ifend", _function);

            /* condition branch */
            _builder->CreateCondBr(cond, thenBlock, elseBlock);

            /* then branch */
            _builder->SetInsertPoint(thenBlock);
            this->GenerateStatement(node->_statement.get());
            if(!_builder->GetInsertBlock()->getTerminator())
            {
                _builder->CreateBr(ifEndBlock);
            }
            // _builder->CreateBr(ifEndBlock);

            /* else branch */
            _builder->SetInsertPoint(elseBlock);
            this->GenerateStatement(node->_elseStatement.get());
            if(!_builder->GetInsertBlock()->getTerminator())
            {
                _builder->CreateBr(ifEndBlock);
            }
            // _builder->CreateBr(ifEndBlock);

            /* if-end block */
            _builder->SetInsertPoint(ifEndBlock);

        }

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

        /* *******************************************Body************************************************ */
        _function->getBasicBlockList().push_back(bodyBlock);
        _builder->SetInsertPoint(bodyBlock);

        _loopStack.push({bodyBlock, loopEndBlock});
        this->GenerateStatement(node->_body.get());
        _loopStack.pop();

        _builder->CreateBr(condBlock);
        /* ****************************************************************************************** */

        _function->getBasicBlockList().push_back(loopEndBlock);
        _builder->SetInsertPoint(loopEndBlock);
    }

    // void Generator::GenerateForStatement(BoundForStatement* node)
    // {
    //     auto variableDeclaration = std::make_unique<BoundVariableDeclaration>(node->_variable, std::move(node->_lowerBound));
    //     auto upperBoundVariableDeclaration = std::make_unique<BoundVariableDeclaration>(node->_variableForUpperBoundToBeUsedDuringRewritingForIntoWhile, std::move(node->_upperBound));

    //     auto condition = std::make_unique<BoundBinaryExpression>(
    //             std::make_unique<BoundVariableExpression>(node->_variable),
    //             BoundBinaryOperator::Bind(SyntaxKind::LessThanEqualsToken, Types::INT->Name(), Types::INT->Name()),
    //             std::make_unique<BoundVariableExpression>(node->_variableForUpperBoundToBeUsedDuringRewritingForIntoWhile)
    //     );
    //     // auto continueLabelStatement = std::make_unique<BoundLabelStatement>(node->_loopLabel.second);
    //     auto increment = std::make_unique<BoundExpressionStatement>(
    //             std::make_unique<BoundAssignmentExpression>(
    //                     node->_variable,
    //                     std::make_unique<BoundBinaryExpression>(
    //                             std::make_unique<BoundVariableExpression>(node->_variable),
    //                             BoundBinaryOperator::Bind(SyntaxKind::PlusToken, Types::INT->Name(),Types::INT->Name()),
    //                             std::make_unique<BoundLiteralExpression>(1))
    //             ));

    //     /** This has to be done instead of doing std::make_unique<BoundBlockStatement>({std::move(body), std::move(increment)}) because BoundBlockStatement is explicit */
    //     std::vector<std::unique_ptr<BoundStatementNode>> statements_1(3);
    //     statements_1.emplace_back(std::move(node->_body));
    //     // statements_1.emplace_back(std::move(continueLabelStatement));
    //     statements_1.emplace_back(std::move(increment));

    //     auto whileBody = std::make_unique<BoundBlockStatement>(std::move(statements_1));

    //     // node->_loopLabel.second = LabelSymbol("continue{" + GenerateRandomText(3) + "}");
    //     auto whileStatement = std::make_unique<BoundWhileStatement>(std::move(condition), std::move(whileBody),std::pair<LabelSymbol, LabelSymbol>{});

    //     std::vector<std::unique_ptr<BoundStatementNode>> statements_2(3);
    //     statements_2.emplace_back(std::move(variableDeclaration));
    //     statements_2.emplace_back(std::move(upperBoundVariableDeclaration));
    //     statements_2.emplace_back(std::move(whileStatement));

    //     auto result = std::make_unique<BoundBlockStatement>(std::move(statements_2));

    //     this->GenerateStatement(result.get());
    // }


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
                throw std::logic_error("Generator: Unexpected syntax " + __boundNodeStringMap[node->Kind()]);

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
            /* Check the type of the global variable to decide how to load it */
            auto type = global_var->getValueType();

            if (type->isArrayTy() && type->getArrayElementType()->isIntegerTy(8))
            {
                /* Types::STRING */

                /* If it's a string (array of i8), use GEP to get a pointer to the first element */
                auto zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*_ctx), 0);
                llvm::Value* gep = _builder->CreateGEP(
                    type, /* The type of the value to get element pointer from */
                    global_var,
                    {zero, zero}, /* Index to get the first element */
                    varname.c_str() /*load to variable name */
                );

                return gep;
            }
            else
            {
                /* Types::BOOL, Types::INT */

                /* We need to load */
                return _builder->CreateLoad(
                    global_var->getInitializer()->getType(), 
                    global_var, /*load to variable name */ varname.c_str()
                );
            }
        }

        assert(false); /* We must not reach here becoz we are doing LookUp for function in GenerateCallExpression */

        return value; /* Reachable in case value is llvm::Function* */
    }

    llvm::Value* Generator::GenerateAssignmentExpression(BoundAssignmentExpression* node)
    {
        llvm::Value* value = this->GenerateExpression(node->_expression.get());

        auto prevValuePtr = _scope->LookUp(node->_variable->_name); /* It throws if "node->_variable->_name" is not present in _scope */
        _builder->CreateStore(value, prevValuePtr);

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

    llvm::Value* Generator::IntToBool(llvm::Value* intValue)
    {
        /*
            It returns the memory location{aka Ptr}
            "Ptr" represents the memory location of the resultant variable "%intToBool" where the result
            of CreateICmpNE generated IR will be stored.
        */
        return _builder->CreateICmpNE(intValue, llvm::ConstantInt::get(intValue->getType(), 0), "intToBool");
    }

    llvm::Value* Generator::StringToBool(llvm::Value* stringValue)
    {
        llvm::Function* strlenFunc = _module->getFunction("strlen");
        assert(strlenFunc != nullptr);

        llvm::Value* length = _builder->CreateCall(strlenFunc, { stringValue }, "length");
        return _builder->CreateICmpNE(length, llvm::ConstantInt::get(length->getType(), 0), "stringToBool");
    }

    llvm::Value* Generator::BoolToBool(llvm::Value* boolValue)
    {
        return boolValue;
    }

    llvm::Value* Generator::IntToInt(llvm::Value* intValue)
    {
        return intValue;
    }

    llvm::Value* Generator::StringToInt(llvm::Value* stringValue)
    {
        llvm::Function* atoiFunc = _module->getFunction("atoi");
        assert(atoiFunc != nullptr);

        return _builder->CreateCall(atoiFunc, { stringValue }, "stringToInt");
    }

    llvm::Value* Generator::BoolToInt(llvm::Value* boolValue)
    {   
        /*
            CreateZExt :- It is used to extend the bit-width of an integer value to a larger integer type by adding zeros to
            the higher-order bits.

            If the boolean value is true (which is 1 in binary), the result will be 00000001 in a 32-bit integer. 
            If the boolean value is false (which is 0 in binary), the result will be 00000000 in a 32-bit integer.
        */
        return _builder->CreateZExt(boolValue, llvm::Type::getInt32Ty(*_ctx), "boolToInt");
    }

    llvm::Value* Generator::IntToString(llvm::Value* intValue)
    {
        llvm::Function* snprintfFunc = _module->getFunction("snprintf");
        assert(snprintfFunc != nullptr);

        // Allocate space for the result string
        llvm::Value* buffer = _builder->CreateAlloca(llvm::Type::getInt8Ty(*_ctx), llvm::ConstantInt::get(llvm::Type::getInt32Ty(*_ctx), 32), "buffer");
        
        // Define the format string: "%d" for integers
        llvm::Value* formatString = _builder->CreateGlobalStringPtr("%d");

        // Define the buffer size
        llvm::Value* bufferSize = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*_ctx), 32);

        // Call snprintf to format the integer into the buffer
        llvm::Value* snprintfCall = _builder->CreateCall(snprintfFunc, 
                                                        { buffer, bufferSize, formatString, intValue }, 
                                                        "formattedString");

        return buffer;
    }

    llvm::Value* Generator::StringToString(llvm::Value* stringValue)
    {
        return stringValue;
    }

    llvm::Value* Generator::BoolToString(llvm::Value* boolValue)
    {
        llvm::Value* trueStr = _globalObjectRecord.at("trueConstantStringVALUE");
        llvm::Value* falseStr = _globalObjectRecord.at("falseConstantStringVALUE");

        return _builder->CreateSelect(boolValue, trueStr, falseStr, "boolToString");
    }

    llvm::Value* Generator::GenerateConversionExpression(BoundConversionExpression* node)
    {
        /*
            No need for this because "node" already contains all information. You don't even have to do _binder->BindExpression(node);
            auto value = _evaluator->EvaluateExpression(node);

            "node" contains all information. You just have to generate appropriate LLVM IR for each case of conversion.
        */

        /* Generate LLVM IR for node->_expression */
        llvm::Value* value = this->GenerateExpression(node->_expression.get());

        if(std::strcmp(node->_toType, Types::BOOL->Name()) == 0)
        {
            if(std::strcmp(node->_expression->Type(), Types::BOOL->Name()) == 0)
            {
                return this->BoolToBool(value);
            }

            if(std::strcmp(node->_expression->Type(), Types::INT->Name()) == 0)
            {
                return this->IntToBool(value);
            }

            if(std::strcmp(node->_expression->Type(), Types::STRING->Name()) == 0)
            {
                return this->StringToBool(value);
            }
        }

        if(std::strcmp(node->_toType, Types::INT->Name()) == 0)
        {
            if(std::strcmp(node->_expression->Type(), Types::BOOL->Name()) == 0)
            {
                return this->BoolToInt(value);
            }

            if(std::strcmp(node->_expression->Type(), Types::INT->Name()) == 0)
            {
                return this->IntToInt(value);
            }

            if(std::strcmp(node->_expression->Type(), Types::STRING->Name()) == 0)
            {
                return this->StringToInt(value);
            }
        }

        if(std::strcmp(node->_toType, Types::STRING->Name()) == 0)
        {
            if(std::strcmp(node->_expression->Type(), Types::BOOL->Name()) == 0)
            {
                return this->BoolToString(value);
            }

            if(std::strcmp(node->_expression->Type(), Types::INT->Name()) == 0)
            {
                return this->IntToString(value);
            }

            if(std::strcmp(node->_expression->Type(), Types::STRING->Name()) == 0)
            {
                return this->StringToString(value);
            }
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
                    if(strcmp(node->_left->Type(), Types::STRING->Name()) == 0)
                    {
                        /*
                            left and right are string
                        */

                        llvm::Function* strlenFunc = _module->getFunction("strlen");
                        llvm::Function* mallocFunc = _module->getFunction("malloc");
                        llvm::Function* strCatFunc = _module->getFunction("strcat");
                        llvm::Function* strCpyFunc = _module->getFunction("strcpy");
                        assert(strlenFunc != nullptr && mallocFunc != nullptr && strCatFunc != nullptr && strCpyFunc != nullptr);

                        // Calculate lengths of the left and right strings
                        llvm::Value* leftLen = _builder->CreateCall(strlenFunc, {left}, "leftLen");
                        llvm::Value* rightLen = _builder->CreateCall(strlenFunc, {right}, "rightLen");

                        // Total length = leftLen + rightLen + 1 (for null terminator)
                        llvm::Value* totalLen = _builder->CreateAdd(leftLen, rightLen, "totalLen");
                        llvm::Value* totalLenWithNull = _builder->CreateAdd(totalLen, llvm::ConstantInt::get(llvm::Type::getInt32Ty(*_ctx), 1), "totalLenWithNull");

                        // Convert total length to i64 type for malloc
                        llvm::Value* totalLenI64 = _builder->CreateZExt(totalLenWithNull, llvm::Type::getInt64Ty(*_ctx), "totalLenI64");

                        // Allocate memory for the concatenated string
                        llvm::Value* rawBuffer = _builder->CreateCall(mallocFunc, {totalLenI64}, "rawPtrBuffer");

                        // Cast the i8* (raw pointer) to a desired pointer type, e.g., int32_t* (i32*)
                        // llvm::Value* typedPtrBuffer = _builder->CreateBitCast(rawBuffer, llvm::PointerType::getUnqual(_builder->getInt32Ty()), "typedPtrBuffer");
                        /*
                            No need for bitcast because our raw pointer is already i8*
                        */

                        // Copy the first string into the buffer
                        _builder->CreateCall(strCpyFunc, {rawBuffer, left});

                        // Concatenate the second string to the buffer
                        _builder->CreateCall(strCatFunc, {rawBuffer, right});


                        return rawBuffer;
                    }
                    else
                    {
                        return _builder->CreateAdd(left, right, "tmpadd");
                    }
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

        _scope = std::make_shared<GenScope>(std::unordered_map<std::string, llvm::Value*>{}, _scope);

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

        _scope = _scope->_parent;
        
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
        /******************************************printf**************************************************** */
        /* i8* to substitute for 'char*', 'void*' */
        auto bytePtrTy = _builder->getInt8Ty()->getPointerTo();
        /* int printf(const char* format, ...)*/
        _module->getOrInsertFunction("printf", llvm::FunctionType::get(
            /* return type */ _builder->getInt32Ty(),
            /* type of format arg */ bytePtrTy,
            /* vararg? */ true
        ));
        /********************************************************************************************** */

        /*****************************************fgets***************************************************** */
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
        /********************************************************************************************** */

        /******************************************strlen**************************************************** */
        llvm::FunctionType* strlenType = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(*_ctx), { llvm::Type::getInt8PtrTy(*_ctx) }, false);
        _module->getOrInsertFunction("strlen", strlenType);
        /********************************************************************************************** */

        /*******************************************atoi*************************************************** */
        llvm::FunctionType* atoiType = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(*_ctx), { llvm::Type::getInt8PtrTy(*_ctx) }, false);
        _module->getOrInsertFunction("atoi", atoiType);
        /********************************************************************************************** */

        /*****************************************snprintf***************************************************** */
        // Define the function type for snprintf: int snprintf(char* str, size_t size, const char* format, ...)
        llvm::FunctionType* snprintfType = llvm::FunctionType::get(
            _builder->getInt32Ty(),             // Return type: int
            { _builder->getInt8PtrTy(),         // Argument 1: char* str (destination buffer)
            _builder->getInt64Ty(),           // Argument 2: size_t size (buffer size)
            _builder->getInt8PtrTy() },       // Argument 3: const char* format
            true);                              // IsVarArg: true (because snprintf takes variable arguments)

        _module->getOrInsertFunction("snprintf", snprintfType).getCallee();
        /********************************************************************************************** */

        /******************************************malloc**************************************************** */
        llvm::FunctionType* mallocType = llvm::FunctionType::get(_builder->getInt8PtrTy(), {_builder->getInt64Ty()}, false);
        _module->getOrInsertFunction("malloc", mallocType);
        /********************************************************************************************** */

        /*******************************************strcat*************************************************** */
        llvm::FunctionType* strcatType = llvm::FunctionType::get(_builder->getInt8PtrTy(), {_builder->getInt8PtrTy(), _builder->getInt8PtrTy()}, false);
        _module->getOrInsertFunction("strcat", strcatType);
        /********************************************************************************************** */

        /********************************************strcpy************************************************** */
        llvm::FunctionType* strcpyType = llvm::FunctionType::get(
            _builder->getInt8PtrTy(),                     // Return type: char* (pointer to destination string)
            { _builder->getInt8PtrTy(),              // Argument 1: char* (destination)
            _builder->getInt8PtrTy() },                     // Argument 2: const char* (source)
            false);                                   // IsVarArg: false, as strcpy does not take a variable number of arguments

        _module->getOrInsertFunction("strcpy", strcpyType);
        /********************************************************************************************** */


    }

    void Generator::SetupGlobalEnv(const std::unordered_map<std::string, std::shared_ptr<VariableSymbol>>& variables)
    {

        std::unordered_map<std::string, llvm::Value*> _globalObjectRecord = 
        {
            {"VERSION", _builder->getInt32(648)},
            {"trueConstantStringVALUE", llvm::ConstantDataArray::getString(*_ctx, "true")},
            {"falseConstantStringVALUE", llvm::ConstantDataArray::getString(*_ctx, "false")},
        };

        for(const auto& item: variables)
        {
            assert(item.second->Kind() == SymbolKind::GlobalVariable);
            /*
                all items in "variables" must be SymbolKind::GlobalVariable
            */
            auto* globalVariable = static_cast<GlobalVariableSymbol*>(item.second.get());

            /*
                "globalVariable->_name" is same as "item.first"
            */

            // if(std::strcmp(globalVariable->_type, Types::INT.get()->_typeName) == 0)
            // {
            //     _globalObjectRecord[item.first] = std::visit(GetValueVisitor{_builder.get()}, *globalVariable->_value);
            // }
            // else if(std::strcmp(globalVariable->_type, Types::STRING.get()->_typeName) == 0)
            // {
            //     _globalObjectRecord[item.first] = llvm::ConstantDataArray::getString(*_ctx, "Hello, LLVM IR!");
            // }
            // else if(std::strcmp(globalVariable->_type, Types::BOOL.get()->_typeName) == 0)
            // {
            //     _globalObjectRecord[item.first] = _builder->getInt1(false);
            // }
            // else
            // {
            //     throw std::runtime_error("Invalid Types Encountered!!!");
            // }

            _globalObjectRecord[item.first] = std::visit(GetValueVisitor{_builder.get()}, *globalVariable->_value);
        }

        std::unordered_map<std::string, llvm::Value*> global_record{};
        for(const auto& entry: _globalObjectRecord)
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