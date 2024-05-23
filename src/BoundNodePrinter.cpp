#include <codeanalysis/BoundNodePrinter.hpp>
#include <codeanalysis/BoundExpressionNode.hpp>

namespace trylang
{

    std::stringstream NodePrinter::_buffer{};
    std::vector<std::string> NodePrinter::_indentation{};

    void NodePrinter::Write(trylang::BoundNode *node)
    {
        _buffer.str("");
        _indentation.clear();

        NodePrinter np;
        np.WriteTo(node);

        std::cout << _buffer.str();
    }

    void NodePrinter::WriteTo(trylang::BoundNode *node)
    {
        switch (node->Kind())
        {
            case BoundNodeKind::LiteralExpression:
            {
                this->WriteLiteralExpression(static_cast<BoundLiteralExpression*>(node));
                break;
            }
            case BoundNodeKind::BinaryExpression:
            {
                this->WriteBinaryExpression(static_cast<BoundBinaryExpression*>(node));
                break;
            }
            case BoundNodeKind::UnaryExpression:
            {
                this->WriteUnaryExpression(static_cast<BoundUnaryExpression*>(node));
                break;
            }
            case BoundNodeKind::VariableExpression:
            {
                this->WriteVariableExpression(static_cast<BoundVariableExpression*>(node));
                break;
            }
            case BoundNodeKind::AssignmentExpression:
            {
                this->WriteAssignmentExpression(static_cast<BoundAssignmentExpression*>(node));
                break;
            }
            case BoundNodeKind::ErrorExpression:
            {
                this->WriteErrorExpression(static_cast<BoundErrorExpression*>(node));
                break;
            }
            case BoundNodeKind::CallExpression:
            {
                this->WriteCallExpression(static_cast<BoundCallExpression*>(node));
                break;
            }
            case BoundNodeKind::ConversionExpression:
            {
                this->WriteConversionExpression(static_cast<BoundConversionExpression*>(node));
                break;
            }
            case BoundNodeKind::BlockStatement:
            {
                this->WriteBlockStatement(static_cast<BoundBlockStatement*>(node));
                break;
            }
            case BoundNodeKind::ExpressionStatement:
            {
                this->WriteExpressionStatement(static_cast<BoundExpressionStatement*>(node));
                break;
            }
            case BoundNodeKind::VariableDeclarationStatement:
            {
                this->WriteVariableDeclarationStatement(static_cast<BoundVariableDeclaration*>(node));
                break;
            }
            /*
            case BoundNodeKind::IfStatement:
            {
                this->WriteIfStatement(static_cast<BoundIfStatement*>(node));
                break;
            }
            case BoundNodeKind::WhileStatement:
            {
                this->WriteWhileStatement(static_cast<BoundWhileStatement*>(node));
                break;
            }
            case BoundNodeKind::ForStatement:
            {
                this->WriteForStatement(static_cast<BoundForStatement*>(node));
                break;
            }
             */
            case BoundNodeKind::GotoStatement:
            {
                this->WriteGotoStatement(static_cast<BoundGotoStatement*>(node));
                break;
            }
            case BoundNodeKind::ConditionalGotoStatement:
            {
                this->WriteConditionalGotoStatement(static_cast<BoundConditionalGotoStatement*>(node));
                break;
            }
            case BoundNodeKind::LabelStatement:
            {
                this->WriteLabelStatement(static_cast<BoundLabelStatement*>(node));
                break;
            }
            default:
                throw std::logic_error("Unexpected Node " + trylang::__boundNodeStringMap[node->Kind()]);
        }
    }

    std::ostream& operator<<(std::ostream& out,const std::vector<std::string>& strVec)
    {
        for(const auto& str: strVec)
        {
            out << str;
        }

        return out;
    }

    void NodePrinter::WriteLiteralExpression(BoundLiteralExpression* node)
    {
        if(std::strcmp(Types::INT->Name(), node->Type()) == 0)
        {
            _buffer << _indentation << std::get<int>(*node->_value);
        }
        else if(std::strcmp(Types::BOOL->Name(), node->Type()) == 0)
        {
            _buffer << _indentation << std::boolalpha << std::get<bool>(*node->_value);
        }
        else if(std::strcmp(Types::STRING->Name(), node->Type()) == 0)
        {
            _buffer << _indentation << "\"" << std::get<std::string>(*node->_value) << "\"";
        }
        else
        {
            throw std::logic_error("Unexpected type " + std::string(node->Type()));
        }
    }

    void NodePrinter::WriteBinaryExpression(BoundBinaryExpression* node)
    {
        this->WriteTo(node->_left.get());
        _buffer << _indentation << " @" << node->_op->Kind() << "@ ";
        this->WriteTo(node->_right.get());
    }

    void NodePrinter::WriteUnaryExpression(BoundUnaryExpression* node)
    {
        _buffer << _indentation << " @" << node->_op->Kind() << "@ ";
        this->WriteTo(node->_operand.get());
    }

    void NodePrinter::WriteVariableExpression(BoundVariableExpression* node)
    {
        _buffer << _indentation << node->_variable->_name;
    }

    void NodePrinter::WriteAssignmentExpression(BoundAssignmentExpression* node)
    {
        _buffer << _indentation << node->_variable->_name;
        _buffer << " = ";
        this->WriteTo(node->_expression.get());
    }

    void NodePrinter::WriteErrorExpression(BoundErrorExpression* node)
    {
        _buffer << _indentation << "?";
    }

    void NodePrinter::WriteCallExpression(BoundCallExpression* node)
    {
        _buffer << _indentation << node->_function->_name << "(";
        bool isFirst = true;
        for(const auto& arg: node->_arguments)
        {
            if(isFirst)
            {
                isFirst = false;
            }
            else
            {
                _buffer << ", ";
            }

            this->WriteTo(arg.get());
        }
        _buffer << ")";
    }

    void NodePrinter::WriteConversionExpression(BoundConversionExpression* node)
    {
        _buffer << _indentation << node->_toType << "(";
        this->WriteTo(node->_expression.get());
        _buffer << ")";
    }

    void NodePrinter::WriteBlockStatement(BoundBlockStatement* node)
    {
        _buffer << _indentation << "{\n";
        _indentation.emplace_back(" "); /* 2 spaces */

        for(const auto& s: node->_statements)
        {
            if(s == nullptr)
            {
                continue;
            }

            this->WriteTo(s.get());
        }

        _indentation.pop_back();
        _buffer << _indentation << "}\n";
    }

    void NodePrinter::WriteExpressionStatement(BoundExpressionStatement* node)
    {
        this->WriteTo(node->_expression.get());
        _buffer << "\n";
    }

    void NodePrinter::WriteVariableDeclarationStatement(BoundVariableDeclaration* node)
    {
        _buffer << _indentation << (node->_variable->_isReadOnly ? "let " : "var ");
        _buffer << _indentation << node->_variable->_name;
        _buffer << _indentation << " = ";
        this->WriteTo(node->_expression.get());
        _buffer << _indentation << "\n";
    }

    /*
    void NodePrinter::WriteNestedStatement(BoundStatementNode* node)
    {
        bool needsIndentation = dynamic_cast<BoundBlockStatement*>(node) == nullptr ? true : false;

        if(needsIndentation)
        {
            _indentation.emplace_back(" ");
        }

        this->WriteTo(node);

        if(needsIndentation)
        {
            _indentation.pop_back();
        }
    }
    */

    /*
    void NodePrinter::WriteIfStatement(BoundIfStatement* node)
    {

    }

    void NodePrinter::WriteWhileStatement(BoundWhileStatement* node)
    {

    }

    void NodePrinter::WriteForStatement(BoundForStatement* node)
    {

    }
    */

    void NodePrinter::WriteGotoStatement(BoundGotoStatement* node)
    {
        _buffer << _indentation << "goto ";
        _buffer << node->_label._name << "\n";
    }

    void NodePrinter::WriteConditionalGotoStatement(BoundConditionalGotoStatement* node)
    {
        _buffer << _indentation << "goto ";
        _buffer << node->_label._name;
        _buffer << (node->_jumpIfFalse ? " unless " : " if ");
        this->WriteTo(node->_condition.get());
        _buffer << "\n";
    }

    void NodePrinter::WriteLabelStatement(BoundLabelStatement* node)
    {
        bool unindent = !_indentation.empty(); // no surprises
        if(unindent)
        {
            _indentation.pop_back();
        }

        _buffer << _indentation << node->_label._name << " : \n";

        if(unindent)
        {
            _indentation.emplace_back(" ");
        }
    }

}