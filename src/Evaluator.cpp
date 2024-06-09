#include "codeanalysis/BoundNodeKind.hpp"
#include "codeanalysis/Types.hpp"
#include <codeanalysis/Evaluator.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/BoundExpressionNode.hpp>
#include <codeanalysis/SyntaxKind.hpp>
#include <codeanalysis/BoundScope.hpp>
#include <variant>

namespace trylang
{
    Evaluator::Evaluator(std::unique_ptr<BoundProgram> program, variable_map_t &variables)
        : _program(std::move(program)) ,_globals(variables)
    {}


    object_t Evaluator::Evaluate()
    {
        return this->EvaluateStatement(_program->_statement.get());
    }

    object_t Evaluator::EvaluateStatement(BoundBlockStatement* body)
    {

        std::unordered_map<LabelSymbol, int, LabelSymbolHash> labelToIndex;
        for(auto i = 0; i < body->_statements.size(); i++)
        {
            auto* BLSnode = dynamic_cast<BoundLabelStatement*>(body->_statements.at(i).get());
            if(BLSnode != nullptr)
            {
                labelToIndex[BLSnode->_label] = i + 1;
            }
        }

        auto index= 0;
        while(index < body->_statements.size())
        {
            if(body->_statements.at(index) == nullptr)
            {
                /* Some statement are nullptr. This is happened during lowering of while, if, for into gotos */
                index++;
                continue;
            }

            auto* s = body->_statements.at(index).get();

            auto* BVDSnode = dynamic_cast<BoundVariableDeclaration*>(s);
            if(BVDSnode != nullptr)
            {
                this->EvaluateVariableDeclaration(BVDSnode);
                index++;
                continue;
            }

            auto* BESnode = dynamic_cast<BoundExpressionStatement*>(s);
            if(BESnode != nullptr)
            {
                this->EvaluateExpressionStatement(BESnode);
                index++;
                continue;
            }

            auto* BGnode = dynamic_cast<BoundGotoStatement*>(s);
            if(BGnode != nullptr)
            {
                index = labelToIndex[BGnode->_label];
                continue;
            }

            auto* BCGnode = dynamic_cast<BoundConditionalGotoStatement*>(s);
            if(BCGnode != nullptr)
            {
                auto condition = this->EvaluateExpression(BCGnode->_condition.get());
                bool condition_result = std::get<bool>(*condition);

                if(condition_result && !BCGnode->_jumpIfFalse || !condition_result && BCGnode->_jumpIfFalse)
                {
                    index = labelToIndex[BCGnode->_label];
                }
                else
                {
                    index++;
                }
                continue;
            }

            auto* BLSnode = dynamic_cast<BoundLabelStatement*>(s);
            if(BLSnode != nullptr)
            {
                index++;
                continue;
            }

            throw std::logic_error("Unexpected node " + trylang::__boundNodeStringMap[s->Kind()]);
        }

        return _lastValue;
    }

    void Evaluator::EvaluateVariableDeclaration(BoundVariableDeclaration *node)
    {
        auto value = this->EvaluateExpression(node->_expression.get());
        _lastValue = value;

        if(node->_variable->Kind() == SymbolKind::GlobalVariable)
        {
            _globals[node->_variable->_name] = value;
        }
        else
        {
            if(_locals.empty())
            {
                throw std::logic_error("_locals ie EMPTY");
            }

            auto& locals = _locals.top();
            locals.insert({node->_variable->_name, value}); /* Here insert is IMP */
        }
    }

    void Evaluator::EvaluateExpressionStatement(BoundExpressionStatement *node)
    {
        _lastValue = this->EvaluateExpression(node->_expression.get());
    }

    object_t Evaluator::EvaluateExpression(BoundExpressionNode* node)
    {
        switch (node->Kind())
        {
            case BoundNodeKind::LiteralExpression:
                return this->EvaluateLiteralExpression(static_cast<BoundLiteralExpression*>(node));
                break;
            case BoundNodeKind::VariableExpression:
                return this->EvaluateVariableExpression(static_cast<BoundVariableExpression*>(node));
                break;
            case BoundNodeKind::AssignmentExpression:
                return this->EvaluateAssignmentExpression(static_cast<BoundAssignmentExpression*>(node));
                break;
            case BoundNodeKind::UnaryExpression:
                return this->EvaluateUnaryExpression(static_cast<BoundUnaryExpression*>(node));
                break;
            case BoundNodeKind::CallExpression:
                return this->EvaluateCallExpression(static_cast<BoundCallExpression*>(node));
                break;
            case BoundNodeKind::ConversionExpression:
                return this->EvaluateConversionExpression(static_cast<BoundConversionExpression*>(node));
                break;
            case BoundNodeKind::BinaryExpression:
                return this->EvaluateBinaryExpression(static_cast<BoundBinaryExpression*>(node));
                break;
            default:
                throw std::logic_error("Unexpected node " + trylang::__boundNodeStringMap[node->Kind()]);
        }
    }

    object_t Evaluator::EvaluateLiteralExpression(BoundLiteralExpression* node)
    {
        return *node->_value;
    }

    object_t Evaluator::EvaluateVariableExpression(BoundVariableExpression *node)
    {
        if(node->_variable->Kind() == SymbolKind::GlobalVariable)
        {
            return _globals.at(node->_variable->_name);
        }
        else
        {
            if(_locals.empty())
            {
                throw std::logic_error("_locals is EMPTY");
            }
            const auto& locals = _locals.top();
            return locals.at(node->_variable->_name);
        }
    }
    
    object_t Evaluator::EvaluateAssignmentExpression(BoundAssignmentExpression* node)
    {
        auto value = this->EvaluateExpression(node->_expression.get());

        if(node->_variable->Kind() == SymbolKind::GlobalVariable)
        {
            _globals[node->_variable->_name] = value;
        }
        else
        {
            if(_locals.empty())
            {
                throw std::logic_error("_locals ie EMPTY");
            }

            auto& locals = _locals.top();
            locals.at(node->_variable->_name) = value;
        }

        return value;
    }

    object_t Evaluator::EvaluateUnaryExpression(BoundUnaryExpression* node)
    {
         
        object_t operand = this->EvaluateExpression(node->_operand.get());
        
        if(node->_op->_kind == BoundNodeKind::Identity)
        {
            int operand_value = std::get<int>(*operand); /* If we are reaching here means operand has "int" */
            return operand_value;
        }

        if(node->_op->_kind == BoundNodeKind::Negation)
        {
            int operand_value = std::get<int>(*operand); /* If we are reaching here means operand has "int" */
            return -operand_value;
        }

        if(node->_op->_kind == BoundNodeKind::LogicalNegation)
        {
            bool operand_value = std::get<bool>(*operand); /* If we are reaching here means operand has "bool" */
            return !operand_value;
        }

        throw std::logic_error("Unexpected unary operator " + trylang::__boundNodeStringMap[node->_op->_kind]);
        
    }

    object_t Evaluator::EvaluateCallExpression(BoundCallExpression* node)
    {
        if(node->_function->_name == BUILT_IN_FUNCTIONS::MAP.at("input")->_name)
        {
            std::string input;
            std::getline(std::cin, input);

            return input;
        }
        else if(node->_function->_name == BUILT_IN_FUNCTIONS::MAP.at("print")->_name)
        {
            auto evaluated_first_argument_value = this->EvaluateExpression(node->_arguments[0].get());
            const auto& message = std::get<std::string>(*evaluated_first_argument_value);
            std::cout << message << "\n";

            return std::nullopt;
        }
        else
        {
            variable_map_t locals;
            for(auto i = 0 ; i < node->_arguments.size() ; i++)
            {
                auto parameter = node->_function->_parameters[i];
                auto value = this->EvaluateExpression(node->_arguments[i].get());
                locals[parameter._name] = value;
            }


            _locals.push(locals);

            auto it = _program->_functionBodies.find(node->_function->_name);
            if(it == _program->_functionBodies.end())
            {
                    throw std::logic_error("Unexpected function " + node->_function->_name); /* Logically this throw may never occur */
            }
            auto result = this->EvaluateStatement(it->second.second.get());

            _locals.pop();

            return result;
        }
    }

    object_t Evaluator::EvaluateConversionExpression(BoundConversionExpression* node)
    {   
        auto value = this->EvaluateExpression(node->_expression.get());
        if(std::strcmp(node->_toType, Types::BOOL->Name()) == 0)
        {
            /* It returns bool */
            return std::visit(BoolConvertVisitor{}, *value);
        }

        if(std::strcmp(node->_toType, Types::INT->Name()) == 0)
        {
            /* It returns int */
            return std::visit(IntConvertVisitor{}, *value);
        }

        if(std::strcmp(node->_toType, Types::STRING->Name()) == 0)
        {
            /* It returns std::string */
            return std::visit(StringConvertVisitor{}, *value);
        }

        throw std::logic_error("Unexpected Type " + std::string(node->_toType));
    }

    object_t Evaluator::EvaluateBinaryExpression(BoundBinaryExpression* node)
    {
        /* If we reach here we need to have a "int" or "bool" or "string" */
        oobject_t left = *(this->EvaluateExpression(node->_left.get()));
        oobject_t right = *(this->EvaluateExpression(node->_right.get()));

        if(node->_op->_kind == BoundNodeKind::Addition)
        {
            if(std::strcmp(node->Type(), Types::INT->Name()) == 0)
            {
                int left_value = std::get<int>(left);
                int right_value = std::get<int>(right);

                return left_value + right_value;
            }

            if(std::strcmp(node->Type(), Types::STRING->Name()) == 0)
            {
                const std::string& left_value = std::get<std::string>(left);
                const std::string& right_value = std::get<std::string>(right);

                return left_value + right_value;
            }

        }

        if(node->_op->_kind == BoundNodeKind::Subtraction)
        {
            int left_value = std::get<int>(left);
            int right_value = std::get<int>(right);
            return left_value - right_value;
        }

        if(node->_op->_kind == BoundNodeKind::Division)
        {
            int left_value = std::get<int>(left);
            int right_value = std::get<int>(right);
            return left_value / right_value;
        }

        if(node->_op->_kind == BoundNodeKind::Multiplication)
        {
            int left_value = std::get<int>(left);
            int right_value = std::get<int>(right);
            return left_value * right_value;
        }

        if(node->_op->_kind == BoundNodeKind::LogicalOr)
        {
            bool left_value = std::get<bool>(left);
            bool right_value = std::get<bool>(right);
            return (left_value || right_value);
        }

        if(node->_op->_kind == BoundNodeKind::LogicalAnd)
        {
            bool left_value = std::get<bool>(left);
            bool right_value = std::get<bool>(right);
            return (left_value && right_value);
        }

        if(node->_op->_kind == BoundNodeKind::LogicalEquality)
        {
            /* No need to check for both _right and _left. Checking any one of them would have been also fine */
            if((std::strcmp(node->_right->Type(), Types::INT->Name()) == 0) &&
                (std::strcmp(node->_left->Type(), Types::INT->Name()) == 0))
            {
                int left_value = std::get<int>(left);
                int right_value = std::get<int>(right);

                return left_value == right_value;
            }

            if((std::strcmp(node->_right->Type(), Types::STRING->Name()) == 0) &&
                (std::strcmp(node->_left->Type(), Types::STRING->Name()) == 0))
            {
                const std::string& left_value = std::get<std::string>(left);
                const std::string& right_value = std::get<std::string>(right);

                return left_value == right_value;
            }

            if((std::strcmp(node->_right->Type(), Types::BOOL->Name()) == 0) &&
                (std::strcmp(node->_left->Type(), Types::BOOL->Name()) == 0))
            {
                bool left_value = std::get<bool>(left);
                bool right_value = std::get<bool>(right);

                return left_value == right_value;
            }
        }

        if(node->_op->_kind == BoundNodeKind::LogicalNotEquality)
        {

            /* No need to check for both _right and _left. Checking any one of them would have been also fine */
            if((std::strcmp(node->_right->Type(), Types::INT->Name()) == 0) &&
                (std::strcmp(node->_left->Type(), Types::INT->Name()) == 0))
            {
                int left_value = std::get<int>(left);
                int right_value = std::get<int>(right);

                return left_value != right_value;
            }

            if((std::strcmp(node->_right->Type(), Types::STRING->Name()) == 0) &&
                (std::strcmp(node->_left->Type(), Types::STRING->Name()) == 0))
            {
                const std::string& left_value = std::get<std::string>(left);
                const std::string& right_value = std::get<std::string>(right);

                return left_value != right_value;
            }

            if((std::strcmp(node->_right->Type(), Types::BOOL->Name()) == 0) &&
                (std::strcmp(node->_left->Type(), Types::BOOL->Name()) == 0))
            {
                bool left_value = std::get<bool>(left);
                bool right_value = std::get<bool>(right);

                return left_value != right_value;
            }
        }

        if(node->_op->_kind == BoundNodeKind::Less)
        {
            int left_value = std::get<int>(left);
            int right_value = std::get<int>(right);
            return left_value < right_value;
        }

        if(node->_op->_kind == BoundNodeKind::LessEquals)
        {
            int left_value = std::get<int>(left);
            int right_value = std::get<int>(right);
            return left_value <= right_value;
        }

        if(node->_op->_kind == BoundNodeKind::Greater)
        {
            int left_value = std::get<int>(left);
            int right_value = std::get<int>(right);
            return left_value > right_value;
        }

        if(node->_op->_kind == BoundNodeKind::GreaterEquals)
        {
            int left_value = std::get<int>(left);
            int right_value = std::get<int>(right);
            return left_value >= right_value;
        }

        throw std::logic_error("Unexpected binary operator " + trylang::__boundNodeStringMap[node->_op->_kind]);
    }
}