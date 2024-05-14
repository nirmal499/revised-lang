#include <codeanalysis/Evaluator.hpp>
#include <codeanalysis/ExpressionSyntax.hpp>
#include <codeanalysis/BoundExpressionNode.hpp>
#include <codeanalysis/SyntaxKind.hpp>
#include <variant>

namespace trylang
{
    Evaluator::Evaluator(std::unique_ptr<BoundBlockStatement> root, variable_map_t& variables)
        : _root(std::move(root)), _variable_map(variables)
    {}

    object_t Evaluator::Evaluate()
    {

        std::unordered_map<LabelSymbol, int, LabelSymbolHash> labelToIndex;
        for(auto i = 0; i < _root->_statements.size(); i++)
        {
            auto* BLSnode = dynamic_cast<BoundLabelStatement*>(_root->_statements.at(i).get());
            if(BLSnode != nullptr)
            {
                labelToIndex[BLSnode->_label] = i + 1;
            }
        }

        auto index= 0;
        while(index < _root->_statements.size())
        {
            if(_root->_statements.at(index) == nullptr)
            {
                index++;
                continue;
            }

            auto* s = _root->_statements.at(index).get();

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

//        this->EvaluateStatement(_root.get());

        return _lastValue;
    }

    /*
    void Evaluator::EvaluateStatement(BoundStatementNode* node)
    {

        auto* BBSnode = dynamic_cast<BoundBlockStatement*>(node);
        if(BBSnode != nullptr)
        {
            this->EvaluateBlockStatement(BBSnode);
            return;
        }

        auto* BESnode = dynamic_cast<BoundExpressionStatement*>(node);
        if(BESnode != nullptr)
        {
            this->EvaluateExpressionStatement(BESnode);
            return;
        }

        auto* BVDSnode = dynamic_cast<BoundVariableDeclaration*>(node);
        if(BVDSnode != nullptr)
        {
            this->EvaluateVariableDeclaration(BVDSnode);
            return;
        }

        auto* BIfSnode = dynamic_cast<BoundIfStatement*>(node);
        if(BIfSnode != nullptr)
        {
            this->EvaluateIfStatement(BIfSnode);
            return;
        }

        auto* BWhileSnode = dynamic_cast<BoundWhileStatement*>(node);
        if(BWhileSnode != nullptr)
        {
            this->EvaluateWhileStatement(BWhileSnode);
            return;
        }

//        auto* BForSnode = dynamic_cast<BoundForStatement*>(node);
//        if(BForSnode != nullptr)
//        {
//            this->EvaluateForStatement(BForSnode);
//            return;
//        }

        throw std::logic_error("Unexpected node " + trylang::__boundNodeStringMap[node->Kind()]);
    }
     */

    /*
    void Evaluator::EvaluateIfStatement(BoundIfStatement *node)
    {
        auto condition = this->EvaluateExpression(node->_condition.get());
        bool condition_result = std::get<bool>(condition);

        if(condition_result)
        {
            this->EvaluateStatement(node->_statement.get());
        }
        else if(node->_elseStatement != nullptr)
        {
            this->EvaluateStatement(node->_elseStatement.get());
        }
    }
     */


    void Evaluator::EvaluateVariableDeclaration(BoundVariableDeclaration *node)
    {
        auto value = this->EvaluateExpression(node->_expression.get());
        _variable_map[node->_variable] = value;
        _lastValue = value;
    }

    /*
    void Evaluator::EvaluateBlockStatement(BoundBlockStatement *node)
    {
        for(const auto& statement: node->_statements)
        {
            if(statement != nullptr)
            {
                this->EvaluateStatement(statement.get());
            }
        }
    }
     */

    void Evaluator::EvaluateExpressionStatement(BoundExpressionStatement *node)
    {
        _lastValue = this->EvaluateExpression(node->_expression.get());
    }

    object_t Evaluator::EvaluateExpression(BoundExpressionNode* node)
    {   
        auto* BLEnode = dynamic_cast<BoundLiteralExpression*>(node);
        if(BLEnode != nullptr)
        {
            return *BLEnode->_value;
        }

        auto* BVEnode = dynamic_cast<BoundVariableExpression*>(node);
        if(BVEnode != nullptr)
        {
            /* In the binder we ensured that "BVEnode->_variable" exists in _variable_map AND *object_value will always have a value */
            const auto& object_value = _variable_map.at(BVEnode->_variable);
            return *object_value;
        }

        auto* BAEnode = dynamic_cast<BoundAssignmentExpression*>(node);
        if(BAEnode != nullptr)
        {
            auto value = this->EvaluateExpression(BAEnode->_expression.get());
            _variable_map[BAEnode->_variable] = value;
            return value;
        }

        auto* BUEnode = dynamic_cast<BoundUnaryExpression*>(node);
        if(BUEnode != nullptr)
        {   
            object_t operand = this->EvaluateExpression(BUEnode->_operand.get());
            
            if(BUEnode->_op->_kind == BoundNodeKind::Identity)
            {
                int operand_value = std::get<int>(*operand); /* If we are reaching here means operand has "int" */
                return operand_value;
            }

            if(BUEnode->_op->_kind == BoundNodeKind::Negation)
            {
                int operand_value = std::get<int>(*operand); /* If we are reaching here means operand has "int" */
                return -operand_value;
            }

            if(BUEnode->_op->_kind == BoundNodeKind::LogicalNegation)
            {
                bool operand_value = std::get<bool>(*operand); /* If we are reaching here means operand has "bool" */
                return !operand_value;
            }
            throw std::logic_error("Unexpected unary operator " + trylang::__boundNodeStringMap[BUEnode->_op->_kind]);
        }

        auto* BCEnode = dynamic_cast<BoundCallExpression*>(node);
        if(BCEnode != nullptr)
        {
            if(BCEnode->_function._name == BUILT_IN_FUNCTIONS::MAP.at("input")._name)
            {
                std::string input;
                std::getline(std::cin, input);

                return input;
            }
            else if(BCEnode->_function._name == BUILT_IN_FUNCTIONS::MAP.at("print")._name)
            {
                auto evaluated_first_argument_value = this->EvaluateExpression(BCEnode->_arguments[0].get());
                const auto& message = std::get<std::string>(*evaluated_first_argument_value);
                std::cout << message << "\n";

                return std::nullopt;
            }
            else
            {
                throw std::logic_error("Unexpected function " + BCEnode->_function._name); /* Logical this throw may never occur */
            }
        }

        auto* BConEnode = dynamic_cast<BoundConversionExpression*>(node);
        if(BConEnode != nullptr)
        {
            auto value = this->EvaluateExpression(BConEnode->_expression.get());
            if(std::strcmp(BConEnode->_toType->Name(), Types::BOOL->Name()) == 0)
            {
                /* It returns bool */
                return std::visit(BoolConvertVisitor{}, *value);
            }

            if(std::strcmp(BConEnode->_toType->Name(), Types::INT->Name()) == 0)
            {
                /* It returns int */
                return std::visit(IntConvertVisitor{}, *value);
            }

            if(std::strcmp(BConEnode->_toType->Name(), Types::STRING->Name()) == 0)
            {
                /* It returns std::string */
                return std::visit(StringConvertVisitor{}, *value);
            }

            throw std::logic_error("Unexpected Type " + std::string(BConEnode->_toType->Name()));
        }

        auto* BBEnode = dynamic_cast<BoundBinaryExpression*>(node);
        if(BBEnode != nullptr)
        {
            /* If we reach here we need to have a "int" or "bool" or "string" */
            oobject_t left = *(this->EvaluateExpression(BBEnode->_left.get()));
            oobject_t right = *(this->EvaluateExpression(BBEnode->_right.get()));

            if(BBEnode->_op->_kind == BoundNodeKind::Addition)
            {
                if(std::strcmp(BBEnode->Type(), Types::INT->Name()) == 0)
                {
                    int left_value = std::get<int>(left);
                    int right_value = std::get<int>(right);

                    return left_value + right_value;
                }

                if(std::strcmp(BBEnode->Type(), Types::STRING->Name()) == 0)
                {
                    const std::string& left_value = std::get<std::string>(left);
                    const std::string& right_value = std::get<std::string>(right);

                    return left_value + right_value;
                }

            }

            if(BBEnode->_op->_kind == BoundNodeKind::Subtraction)
            {
                int left_value = std::get<int>(left);
                int right_value = std::get<int>(right);
                return left_value - right_value;
            }

            if(BBEnode->_op->_kind == BoundNodeKind::Division)
            {
                int left_value = std::get<int>(left);
                int right_value = std::get<int>(right);
                return left_value / right_value;
            }

            if(BBEnode->_op->_kind == BoundNodeKind::Multiplication)
            {
                int left_value = std::get<int>(left);
                int right_value = std::get<int>(right);
                return left_value * right_value;
            }

            if(BBEnode->_op->_kind == BoundNodeKind::LogicalOr)
            {
                bool left_value = std::get<bool>(left);
                bool right_value = std::get<bool>(right);
                return (left_value || right_value);
            }

            if(BBEnode->_op->_kind == BoundNodeKind::LogicalAnd)
            {
                bool left_value = std::get<bool>(left);
                bool right_value = std::get<bool>(right);
                return (left_value && right_value);
            }

            if(BBEnode->_op->_kind == BoundNodeKind::LogicalEquality)
            {
                /* No need to check for both _right and _left. Checking any one of them would have been also fine */
                if((std::strcmp(BBEnode->_right->Type(), Types::INT->Name()) == 0) &&
                   (std::strcmp(BBEnode->_left->Type(), Types::INT->Name()) == 0))
                {
                    int left_value = std::get<int>(left);
                    int right_value = std::get<int>(right);

                    return left_value == right_value;
                }

                if((std::strcmp(BBEnode->_right->Type(), Types::STRING->Name()) == 0) &&
                   (std::strcmp(BBEnode->_left->Type(), Types::STRING->Name()) == 0))
                {
                    const std::string& left_value = std::get<std::string>(left);
                    const std::string& right_value = std::get<std::string>(right);

                    return left_value == right_value;
                }

                if((std::strcmp(BBEnode->_right->Type(), Types::BOOL->Name()) == 0) &&
                   (std::strcmp(BBEnode->_left->Type(), Types::BOOL->Name()) == 0))
                {
                    bool left_value = std::get<bool>(left);
                    bool right_value = std::get<bool>(right);

                    return left_value == right_value;
                }
            }

            if(BBEnode->_op->_kind == BoundNodeKind::LogicalNotEquality)
            {

                /* No need to check for both _right and _left. Checking any one of them would have been also fine */
                if((std::strcmp(BBEnode->_right->Type(), Types::INT->Name()) == 0) &&
                   (std::strcmp(BBEnode->_left->Type(), Types::INT->Name()) == 0))
                {
                    int left_value = std::get<int>(left);
                    int right_value = std::get<int>(right);

                    return left_value != right_value;
                }

                if((std::strcmp(BBEnode->_right->Type(), Types::STRING->Name()) == 0) &&
                   (std::strcmp(BBEnode->_left->Type(), Types::STRING->Name()) == 0))
                {
                    const std::string& left_value = std::get<std::string>(left);
                    const std::string& right_value = std::get<std::string>(right);

                    return left_value != right_value;
                }

                if((std::strcmp(BBEnode->_right->Type(), Types::BOOL->Name()) == 0) &&
                   (std::strcmp(BBEnode->_left->Type(), Types::BOOL->Name()) == 0))
                {
                    bool left_value = std::get<bool>(left);
                    bool right_value = std::get<bool>(right);

                    return left_value != right_value;
                }
            }

            if(BBEnode->_op->_kind == BoundNodeKind::Less)
            {
                int left_value = std::get<int>(left);
                int right_value = std::get<int>(right);
                return left_value < right_value;
            }

            if(BBEnode->_op->_kind == BoundNodeKind::LessEquals)
            {
                int left_value = std::get<int>(left);
                int right_value = std::get<int>(right);
                return left_value <= right_value;
            }

            if(BBEnode->_op->_kind == BoundNodeKind::Greater)
            {
                int left_value = std::get<int>(left);
                int right_value = std::get<int>(right);
                return left_value > right_value;
            }

            if(BBEnode->_op->_kind == BoundNodeKind::GreaterEquals)
            {
                int left_value = std::get<int>(left);
                int right_value = std::get<int>(right);
                return left_value >= right_value;
            }

            throw std::logic_error("Unexpected binary operator " + trylang::__boundNodeStringMap[BBEnode->_op->_kind]);
        }

        throw std::logic_error("Unexpected node " + trylang::__boundNodeStringMap[node->Kind()]);
    }

    /*
    void Evaluator::EvaluateWhileStatement(BoundWhileStatement *node)
    {
        auto condition = this->EvaluateExpression(node->_condition.get());
        bool condition_result = std::get<bool>(condition);
        while(condition_result)
        {
            this->EvaluateStatement(node->_body.get());

            condition = this->EvaluateExpression((node->_condition.get()));
            condition_result = std::get<bool>(condition);
        }
    }
     */

//    void Evaluator::EvaluateForStatement(BoundForStatement *node)
//    {
//        auto lowerBound = this->EvaluateExpression(node->_lowerBound.get());
//        auto upperBound = this->EvaluateExpression(node->_upperBound.get());
//
//        int lowerBound_result = std::get<int>(lowerBound);
//        int upperBound_result = std::get<int>(upperBound);
//
//        for(int i = lowerBound_result; i <= upperBound_result; i++)
//        {
//            _variable_map[node->_variable] = i; /* It is IMP because the "this->EvaluateStatement(node->_body.get()); will be using the changed 'i' */
//            this->EvaluateStatement(node->_body.get());
//        }
//    }
}