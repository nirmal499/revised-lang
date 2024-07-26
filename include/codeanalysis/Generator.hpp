#pragma once

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "codeanalysis/BoundExpressionNode.hpp"
#include "codeanalysis/GenScope.hpp"
#include "codeanalysis/Symbol.hpp"
#include <llvm-14/llvm/IR/BasicBlock.h>
#include <memory>
#include <string>
#include <stack>
#include <utility>

namespace trylang
{   
    struct BoundStatementNode;
    struct BoundBlockStatement;
    struct BoundBoundStatement;
    struct BoundVariableDeclaration;
    struct BoundLabelStatement;
    struct BoundGotoStatement;
    struct BoundConditionalGotoStatement;
    struct BoundIfStatement;
    struct BoundWhileStatement;
    struct BoundForStatement;
    struct BoundExpressionStatement;

    struct BoundContinueStatement;
    struct BoundBreakStatement;

    struct BoundExpressionNode;
    struct BoundLiteralExpression;
    struct BoundVariableExpression;
    struct BoundAssignmentExpression;
    struct BoundCallExpression;
    struct BoundConversionExpression;
    struct BoundUnaryExpression;
    struct BoundBinaryExpression;

    struct BoundProgram;
    struct Evaluator;

    struct Generator
    {
        std::unique_ptr<llvm::LLVMContext> _ctx;
        std::unique_ptr<llvm::Module> _module;
        std::unique_ptr<llvm::IRBuilder<>> _builder;
        std::unique_ptr<llvm::IRBuilder<>> _varsBuilder;
        llvm::Function* _function = nullptr;

        std::unordered_map<std::string, llvm::Value*> _globalObjectRecord;

        std::unordered_map<std::string, llvm::BasicBlock*> _labelMap;
        std::stack<std::pair<llvm::BasicBlock*, llvm::BasicBlock*>> _loopStack; /* One for "body label BasicBlock" and other for "loopend label BasicBlock" */

        std::shared_ptr<GenScope> _scope = nullptr;
        void GenerateStatement(BoundStatementNode* node);

        /* All the below functions are created according to the ones describe in BoundNodeKind */
        void GenerateBlockStatement(BoundBlockStatement* node);
        void GenerateVariableDeclaration(BoundVariableDeclaration* node);
        void GenerateIfStatement(BoundIfStatement* node);
        void GenerateWhileStatement(BoundWhileStatement* node);
        void GenerateContinueStatement(BoundContinueStatement* node);
        void GenerateBreakStatement(BoundBreakStatement* node);
        // void GenerateForStatement(BoundForStatement* node);
        // void GenerateLabelStatement(BoundLabelStatement* node);
        // void GenerateGotoStatement(BoundGotoStatement* node);
        // void GenerateConditionalGotoStatement(BoundConditionalGotoStatement* node);
        void GenerateReturnStatement(BoundReturnStatement* node);
        void GenerateExpressionStatement(BoundExpressionStatement* node);

        llvm::Value* GenerateExpression(BoundExpressionNode* node);
        
        /* All the below functions are created according to the ones describe in BoundNodeKind */
        llvm::Value* GenerateErrorExpression(BoundErrorExpression* node);
        llvm::Value* GenerateLiteralExpression(BoundLiteralExpression* node);
        llvm::Value* GenerateVariableExpression(BoundVariableExpression* node);
        llvm::Value* GenerateAssignmentExpression(BoundAssignmentExpression* node);
        llvm::Value* GenerateCallExpression(BoundCallExpression* node);
        llvm::Value* GenerateConversionExpression(BoundConversionExpression* node);
        llvm::Value* GenerateUnaryExpression(BoundUnaryExpression* node);
        llvm::Value* GenerateBinaryExpression(BoundBinaryExpression* node);

        /* Conversion LLVM IR Generator functions************************ */
        llvm::Value* IntToBool(llvm::Value* intValue);
        llvm::Value* StringToBool(llvm::Value* stringValue);
        llvm::Value* BoolToBool(llvm::Value* boolValue);

        llvm::Value* IntToString(llvm::Value* intValue);
        llvm::Value* StringToString(llvm::Value* stringValue);
        llvm::Value* BoolToString(llvm::Value* boolValue);

        llvm::Value* IntToInt(llvm::Value* intValue);
        llvm::Value* StringToInt(llvm::Value* stringValue);
        llvm::Value* BoolToInt(llvm::Value* boolValue);
        /************************************************************** */

        static void GenerateProgram(BoundProgram* program, const char* IRfilePath);
        Generator(BoundProgram* program);
        void GenerateMain(BoundBlockStatement* rootBlock);
        void GenerateFunction(const std::unordered_map<std::string, std::shared_ptr<VariableSymbol>>& variables, const std::pair<std::shared_ptr<FunctionSymbol>, std::unique_ptr<BoundBlockStatement>>& functionInfoAndBody);

        void ModuleInitialization();
        void SetupExternalFunctions();
        void SetupGlobalEnv(const std::unordered_map<std::string, std::shared_ptr<VariableSymbol>>& variables);

        llvm::GlobalVariable* CreateGlobalVar(const std::string& name, llvm::Constant* initialize);

        llvm::FunctionType* ExtractFunctionType(FunctionSymbol* function);
        llvm::Type* ExtractType(const char* typeName);
        llvm::Function* CreateFunction(const std::string& fnName, llvm::FunctionType* fnType);
        llvm::Function* CreateFunctionProto(const std::string& fnName, llvm::FunctionType* fnType);
        llvm::Value* AllocVariable(const std::string& name, llvm::Type* type);

        void SaveModuleToFile(const char* IRfilePath);

    };

    struct GetValueVisitor
    {
        llvm::IRBuilder<>* _builder;
        GetValueVisitor(llvm::IRBuilder<>* builder): _builder(builder){}

        llvm::Value* operator()(int number)
        {
            return _builder->getInt32(number); /* i32 {aka int} */
        }

        llvm::Value* operator()(const std::string& str)
        {
            return _builder->CreateGlobalStringPtr(str.c_str());  /* i8* {aka char* } */
            /*
                It would also work.
                return llvm::ConstantDataArray::getString(*_ctx, str.c_str());
            */
        }

        llvm::Value* operator()(bool boolValue)
        {
            return _builder->getInt1(boolValue); /* i1 {aka bool} */
        }
    };
}