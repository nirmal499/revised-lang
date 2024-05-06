#include <codeanalysis/BoundExpressionNode.hpp>

namespace trylang
{
    BoundUnaryExpression::BoundUnaryExpression(BoundUnaryOperatorKind operatorKind, std::unique_ptr<BoundExpressionNode> operand)
        : _operatorKind(operatorKind), _operand(std::move(operand))
    {}
    
    const std::type_info& BoundUnaryExpression::Type()
    {
        return _operand->Type();
    }

    BoundNodeKind BoundUnaryExpression::Kind()
    {
        return BoundNodeKind::UnaryExpression;
    }


    BoundLiteralExpression::BoundLiteralExpression(object_t value)
        : _value(std::move(value))
    {}

    const std::type_info& BoundLiteralExpression::Type()
    {   
        /* oobject {aka variant is <int>}*/
        auto index_in_variant = (*_value).index();

        if(index_in_variant == 0)
        {
            return typeid(int);
        }
        
        if(index_in_variant == 1)
        {
            return typeid(bool);
        }

        /* index_in_variant == std::variant_npos */
        throw std::logic_error("Unexpected type_info");
    }

    BoundNodeKind BoundLiteralExpression::Kind()
    {
        return BoundNodeKind::LiteralExpression;
    }

    BoundBinaryExpression::BoundBinaryExpression(std::unique_ptr<BoundExpressionNode> left, BoundBinaryOperatorKind operatorKind, std::unique_ptr<BoundExpressionNode> right)
        : _left(std::move(left)), _operatorKind(operatorKind), _right(std::move(right))
    {}

    const std::type_info& BoundBinaryExpression::Type()
    {
        return _left->Type();
    }
    
    BoundNodeKind BoundBinaryExpression::Kind()
    {
        return BoundNodeKind::BinaryExpression;
    }

}