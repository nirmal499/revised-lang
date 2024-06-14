#include <codeanalysis/BoundExpressionNode.hpp>
#include <codeanalysis/BoundNodeKind.hpp>

namespace trylang
{

    void PrettyPrintBoundNodes(BoundNode* node, std::string indent)
    {
        std::cout << indent;
        std::cout << node->Kind(); /* cout is overloaded for node->Kind() */

        std::cout << "\n";

        indent += "     ";

        for(auto const& child: node->GetChildren())
        {
            if(child != nullptr)
            {
                PrettyPrintBoundNodes(child, indent);
            }
        }

    }
    std::array<std::unique_ptr<BoundUnaryOperator>, 3> _boundUnaryOperatorArray = {
            std::make_unique<BoundUnaryOperator>(SyntaxKind::BangToken, BoundNodeKind::LogicalNegation, Types::BOOL->Name(),Types::BOOL->Name()),
            std::make_unique<BoundUnaryOperator>(SyntaxKind::PlusToken, BoundNodeKind::Identity, Types::INT->Name(),Types::INT->Name()),
            std::make_unique<BoundUnaryOperator>(SyntaxKind::MinusToken, BoundNodeKind::Negation, Types::INT->Name(),Types::INT->Name())
    };

    std::array<std::unique_ptr<BoundBinaryOperator>, 17> _boundBinaryOperatorArray = {
            std::make_unique<BoundBinaryOperator>(SyntaxKind::PlusToken, BoundNodeKind::Addition, Types::INT->Name(),Types::INT->Name(), Types::INT->Name()),
            std::make_unique<BoundBinaryOperator>(SyntaxKind::MinusToken, BoundNodeKind::Subtraction, Types::INT->Name(),Types::INT->Name(), Types::INT->Name()),
            std::make_unique<BoundBinaryOperator>(SyntaxKind::SlashToken, BoundNodeKind::Division, Types::INT->Name(),Types::INT->Name(), Types::INT->Name()),
            std::make_unique<BoundBinaryOperator>(SyntaxKind::StarToken, BoundNodeKind::Multiplication, Types::INT->Name(),Types::INT->Name(), Types::INT->Name()),

            std::make_unique<BoundBinaryOperator>(SyntaxKind::EqualsEqualsToken, BoundNodeKind::LogicalEquality, Types::INT->Name(),Types::INT->Name(), Types::BOOL->Name()),
            std::make_unique<BoundBinaryOperator>(SyntaxKind::BangsEqualsToken, BoundNodeKind::LogicalNotEquality, Types::INT->Name(),Types::INT->Name(), Types::BOOL->Name()),

            std::make_unique<BoundBinaryOperator>(SyntaxKind::LessThanToken, BoundNodeKind::Less, Types::INT->Name(),Types::INT->Name(), Types::BOOL->Name()),
            std::make_unique<BoundBinaryOperator>(SyntaxKind::LessThanEqualsToken, BoundNodeKind::LessEquals, Types::INT->Name(),Types::INT->Name(), Types::BOOL->Name()),
            std::make_unique<BoundBinaryOperator>(SyntaxKind::GreaterThanToken, BoundNodeKind::Greater, Types::INT->Name(),Types::INT->Name(), Types::BOOL->Name()),
            std::make_unique<BoundBinaryOperator>(SyntaxKind::GreaterThanEqualsToken, BoundNodeKind::GreaterEquals, Types::INT->Name(),Types::INT->Name(), Types::BOOL->Name()),

            std::make_unique<BoundBinaryOperator>(SyntaxKind::AmpersandAmpersandToken, BoundNodeKind::LogicalAnd, Types::BOOL->Name(),Types::BOOL->Name(), Types::BOOL->Name()),
            std::make_unique<BoundBinaryOperator>(SyntaxKind::PipePipeToken, BoundNodeKind::LogicalOr, Types::BOOL->Name(),Types::BOOL->Name(), Types::BOOL->Name()),

            std::make_unique<BoundBinaryOperator>(SyntaxKind::EqualsEqualsToken, BoundNodeKind::LogicalEquality, Types::BOOL->Name(),Types::BOOL->Name(), Types::BOOL->Name()),
            std::make_unique<BoundBinaryOperator>(SyntaxKind::BangsEqualsToken, BoundNodeKind::LogicalNotEquality, Types::BOOL->Name(),Types::BOOL->Name(), Types::BOOL->Name()),

            std::make_unique<BoundBinaryOperator>(SyntaxKind::EqualsEqualsToken, BoundNodeKind::LogicalEquality, Types::STRING->Name(),Types::STRING->Name(), Types::BOOL->Name()),
            std::make_unique<BoundBinaryOperator>(SyntaxKind::BangsEqualsToken, BoundNodeKind::LogicalNotEquality, Types::STRING->Name(),Types::STRING->Name(), Types::BOOL->Name()),

            std::make_unique<BoundBinaryOperator>(SyntaxKind::PlusToken, BoundNodeKind::Addition, Types::STRING->Name(),Types::STRING->Name(), Types::STRING->Name())

    };

    BoundUnaryOperator* BoundUnaryOperator::Bind(SyntaxKind syntaxKind, const char* operandType)
    {
        for(auto const& op: trylang::_boundUnaryOperatorArray)
        {
            if(op->_syntaxKind == syntaxKind && std::strcmp(op->_operandType, operandType) == 0)
            {
                return op.get();
            }
        }

        return nullptr;
    }

    BoundBinaryOperator* BoundBinaryOperator::Bind(SyntaxKind syntaxKind, const char* leftOperandType, const char* rightOperandType)
    {
        for(auto const& op:_boundBinaryOperatorArray)
        {
            if(op->_syntaxKind == syntaxKind && std::strcmp(op->_leftOperandType, leftOperandType) == 0 && std::strcmp(op->_rightOperandType, rightOperandType) == 0)
            {
                return op.get();
            }
        }

        return nullptr;
    }

    BoundNodeKind BoundBinaryOperator::Kind()
    {
        return _kind;
    }

    std::vector<BoundNode *> BoundBinaryOperator::GetChildren()
    {
        return {nullptr};
    }

    BoundNodeKind BoundUnaryOperator::Kind()
    {
        return _kind;
    }

    std::vector<BoundNode *> BoundUnaryOperator::GetChildren()
    {
        return {nullptr};
    }

    BoundUnaryExpression::BoundUnaryExpression(BoundUnaryOperator* op, std::unique_ptr<BoundExpressionNode> operand)
        : _op(op), _operand(std::move(operand))
    {}
    
    const char* BoundUnaryExpression::Type()
    {
        return _op->_resultType;
    }

    BoundNodeKind BoundUnaryExpression::Kind()
    {
        return BoundNodeKind::UnaryExpression;
    }

    std::vector<BoundNode *> BoundUnaryExpression::GetChildren()
    {
        return {_op, _operand.get()};
    }

    BoundBlockStatement::BoundBlockStatement(std::vector<std::unique_ptr<BoundStatementNode>> statements)
        : _statements(std::move(statements))
    {}

    BoundNodeKind BoundBlockStatement::Kind()
    {
        return BoundNodeKind::BlockStatement;
    }

    std::vector<BoundNode *> BoundBlockStatement::GetChildren()
    {
        std::vector<BoundNode*> children(_statements.size());

        for(const auto& stmt: _statements)
        {
            children.push_back(stmt.get());
        }

        return children; // RVO
    }

    BoundExpressionStatement::BoundExpressionStatement(std::unique_ptr<BoundExpressionNode> expression)
        : _expression(std::move(expression))
    {}

    BoundNodeKind BoundExpressionStatement::Kind()
    {
        return BoundNodeKind::ExpressionStatement;
    }

    std::vector<BoundNode *> BoundExpressionStatement::GetChildren()
    {
        return {_expression.get()};
    }

    BoundVariableDeclaration::BoundVariableDeclaration(const std::shared_ptr<VariableSymbol>& variable, std::unique_ptr<BoundExpressionNode> expression)
    : _variable(variable), _expression(std::move(expression))
    {}


    BoundNodeKind BoundVariableDeclaration::Kind()
    {
        return BoundNodeKind::VariableDeclarationStatement;
    }

    std::vector<BoundNode *> BoundVariableDeclaration::GetChildren()
    {
        return {_expression.get()};
    }

    BoundLiteralExpression::BoundLiteralExpression(const object_t& value)
        : _value(value)
    {}

    const char* BoundLiteralExpression::Type()
    {   
        return trylang::assign_type_info(*_value);
    }

    BoundNodeKind BoundLiteralExpression::Kind()
    {
        return BoundNodeKind::LiteralExpression;
    }

    std::vector<BoundNode *> BoundLiteralExpression::GetChildren()
    {
        return {nullptr};
    }

    BoundBinaryExpression::BoundBinaryExpression(std::unique_ptr<BoundExpressionNode> left, BoundBinaryOperator* op, std::unique_ptr<BoundExpressionNode> right)
        : _left(std::move(left)), _op(op), _right(std::move(right))
    {}

    const char* BoundBinaryExpression::Type()
    {
        return _op->_resultType;
    }
    
    BoundNodeKind BoundBinaryExpression::Kind()
    {
        return BoundNodeKind::BinaryExpression;
    }

    std::vector<BoundNode *> BoundBinaryExpression::GetChildren()
    {
        return {_left.get(), _op, _right.get()};
    }

    BoundVariableExpression::BoundVariableExpression(const std::shared_ptr<VariableSymbol>& variable)
        : _variable(variable)
    {}

    BoundNodeKind BoundVariableExpression::Kind()
    {
        return BoundNodeKind::VariableExpression;
    }

    const char* BoundVariableExpression::Type()
    {
        return _variable->_type;
    }

    std::vector<BoundNode *> BoundVariableExpression::GetChildren()
    {
        return {nullptr};
    }

    BoundAssignmentExpression::BoundAssignmentExpression(const std::shared_ptr<VariableSymbol>& variable, std::unique_ptr<BoundExpressionNode> expression)
        : _variable(variable), _expression(std::move(expression))
    {}

    const char* BoundAssignmentExpression::Type()
    {
        return _expression->Type();
    }

    BoundNodeKind BoundAssignmentExpression::Kind()
    {
        return BoundNodeKind::AssignmentExpression;
    }

    std::vector<BoundNode *> BoundAssignmentExpression::GetChildren()
    {
        return {_expression.get()};
    }


    BoundIfStatement::BoundIfStatement(std::unique_ptr<BoundExpressionNode> condition,
                                       std::unique_ptr<BoundStatementNode> statement,
                                       std::unique_ptr<BoundStatementNode> elseStatement) : _condition(std::move(condition)), _statement(std::move(statement)), _elseStatement(std::move(elseStatement))
    {

    }

    BoundNodeKind BoundIfStatement::Kind()
    {
        return BoundNodeKind::IfStatement;
    }

    std::vector<BoundNode *> BoundIfStatement::GetChildren()
    {
        std::vector<BoundNode*> children(3);
        children.push_back(_condition.get());
        children.push_back(_statement.get());
        if(_elseStatement != nullptr)
        {
            children.push_back(_elseStatement.get());
        }

        return children; // RVO
    }

    BoundWhileStatement::BoundWhileStatement(std::unique_ptr<BoundExpressionNode> condition,
                                             std::unique_ptr<BoundStatementNode> body,
                                             std::pair<LabelSymbol, LabelSymbol> loopLabel
                                             ) : _condition(std::move(condition)), _body(std::move(body)), _loopLabel(std::move(loopLabel))
    {

    }

    BoundNodeKind BoundWhileStatement::Kind()
    {
        return BoundNodeKind::WhileStatement;
    }

    std::vector<BoundNode *> BoundWhileStatement::GetChildren()
    {
        return {_condition.get(), _body.get()};
    }

    BoundForStatement::BoundForStatement(const std::shared_ptr<VariableSymbol>& variable, std::unique_ptr<BoundExpressionNode> lowerBound,
                                         std::unique_ptr<BoundExpressionNode> upperBound,
                                         const std::shared_ptr<VariableSymbol>& variableForUpperBoundToBeUsedDuringRewritingForIntoWhile,
                                         std::unique_ptr<BoundStatementNode> body,
                                         std::pair<LabelSymbol, LabelSymbol> loopLabel
                                         ) : _variable(variable), _lowerBound(std::move(lowerBound)), _upperBound(std::move(upperBound)), _body(std::move(body)), _loopLabel(std::move(loopLabel)), _variableForUpperBoundToBeUsedDuringRewritingForIntoWhile(variableForUpperBoundToBeUsedDuringRewritingForIntoWhile)
    {

    }

    BoundNodeKind BoundForStatement::Kind()
    {
        return BoundNodeKind::ForStatement;
    }

    std::vector<BoundNode *> BoundForStatement::GetChildren()
    {
        return {_lowerBound.get(), _upperBound.get(), _body.get()};
    }

    BoundNodeKind BoundGotoStatement::Kind()
    {
        return BoundNodeKind::GotoStatement;
    }

    BoundGotoStatement::BoundGotoStatement(LabelSymbol label) : _label(std::move(label))
    {

    }

    std::vector<BoundNode *> BoundGotoStatement::GetChildren()
    {
        return {nullptr};
    }

    BoundLabelStatement::BoundLabelStatement(LabelSymbol label) : _label(std::move(label))
    {

    }

    BoundNodeKind BoundLabelStatement::Kind()
    {
        return BoundNodeKind::LabelStatement;
    }

    std::vector<BoundNode *> BoundLabelStatement::GetChildren()
    {
        return {nullptr};
    }

    BoundConditionalGotoStatement::BoundConditionalGotoStatement(LabelSymbol label,
                                                                 std::unique_ptr<BoundExpressionNode> condition,
                                                                 bool jumpIfFalse) : _label(std::move(label)), _condition(std::move(condition)), _jumpIfFalse(jumpIfFalse)
    {

    }

    BoundNodeKind BoundConditionalGotoStatement::Kind()
    {
        return BoundNodeKind::ConditionalGotoStatement;
    }

    std::vector<BoundNode *> BoundConditionalGotoStatement::GetChildren()
    {
        return {nullptr};
    }

    const char *BoundErrorExpression::Type()
    {
        return Types::ERROR->Name();
    }

    BoundNodeKind BoundErrorExpression::Kind()
    {
        return BoundNodeKind::ErrorExpression;
    }

    std::vector<BoundNode *> BoundErrorExpression::GetChildren()
    {
        return {nullptr};
    }

    BoundCallExpression::BoundCallExpression(const std::shared_ptr<FunctionSymbol>& function,
                                             std::vector<std::unique_ptr<BoundExpressionNode>> arguments) : _function(function), _arguments(std::move(arguments))
    {

    }

    const char *BoundCallExpression::Type()
    {
        return _function->_type;
    }

    BoundNodeKind BoundCallExpression::Kind()
    {
        return BoundNodeKind::CallExpression;
    }

    std::vector<BoundNode *> BoundCallExpression::GetChildren()
    {
        std::vector<BoundNode*> children(_arguments.size());

        for(const auto& arg: _arguments)
        {
            children.push_back(arg.get());
        }

        return std::move(children);
    }


    BoundConversionExpression::BoundConversionExpression(const char *toType, std::unique_ptr<BoundExpressionNode> expression): _toType(toType), _expression(std::move(expression))
    {

    }

    const char *BoundConversionExpression::Type()
    {
        return _toType;
    }

    BoundNodeKind BoundConversionExpression::Kind()
    {
        return BoundNodeKind::ConversionExpression;
    }

    std::vector<BoundNode *> BoundConversionExpression::GetChildren()
    {
        return {_expression.get()};
    }
}