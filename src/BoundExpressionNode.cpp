#include <codeanalysis/BoundExpressionNode.hpp>

namespace trylang
{
    BoundUnaryExpression::BoundUnaryExpression(BoundUnaryOperator* op, std::unique_ptr<BoundExpressionNode> operand)
        : _op(op), _operand(std::move(operand))
    {}
    
    const std::type_info& BoundUnaryExpression::Type()
    {
        return _op->_resultType;
    }

    BoundNodeKind BoundUnaryExpression::Kind()
    {
        return BoundNodeKind::UnaryExpression;
    }


    BoundLiteralExpression::BoundLiteralExpression(const object_t& value)
        : _value(value)
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

    BoundBinaryExpression::BoundBinaryExpression(std::unique_ptr<BoundExpressionNode> left, BoundBinaryOperator* op, std::unique_ptr<BoundExpressionNode> right)
        : _left(std::move(left)), _op(op), _right(std::move(right))
    {}

    const std::type_info& BoundBinaryExpression::Type()
    {
        return _op->_resultType;
    }
    
    BoundNodeKind BoundBinaryExpression::Kind()
    {
        return BoundNodeKind::BinaryExpression;
    }

}