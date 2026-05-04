#ifndef PARSE_TREE_NODE_H
#define PARSE_TREE_NODE_H

#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "../lexer/token.h"

// This header declares the parse tree node types used by the parser output.
class ParseTreeNode
{
public:
    explicit ParseTreeNode(std::string name);
    virtual ~ParseTreeNode() = default;

    const std::string &name() const;
    const std::vector<std::unique_ptr<ParseTreeNode>> &children() const;
    ParseTreeNode *parent();
    const ParseTreeNode *parent() const;

    void addChild(std::unique_ptr<ParseTreeNode> child);

    // This builds and attaches a child node without repeating the allocation boilerplate.
    template <typename NodeType, typename... Args>
    NodeType &emplaceChild(Args &&...args)
    {
        auto child = std::make_unique<NodeType>(std::forward<Args>(args)...);
        NodeType &reference = *child;
        addChild(std::move(child));
        return reference;
    }

    void print(std::ostream &out, const std::string &prefix = "", bool isLast = true) const;

protected:
    virtual std::string renderLabel() const;

private:
    std::string nodeName;
    ParseTreeNode *parentNode;
    std::vector<std::unique_ptr<ParseTreeNode>> childNodes;
};

class ParseTreeTerminalNode : public ParseTreeNode
{
public:
    explicit ParseTreeTerminalNode(const Token &token);

    const Token &token() const;

protected:
    std::string renderLabel() const override;

private:
    Token terminalToken;
};

class ProgramNode : public ParseTreeNode
{
public:
    ProgramNode();
};

class ProgramHeaderNode : public ParseTreeNode
{
public:
    ProgramHeaderNode();
};

class DeclarationPartNode : public ParseTreeNode
{
public:
    DeclarationPartNode();
};

class ConstDeclarationNode : public ParseTreeNode
{
public:
    ConstDeclarationNode();
};

class ConstantNode : public ParseTreeNode
{
public:
    ConstantNode();
};

class TypeDeclarationNode : public ParseTreeNode
{
public:
    TypeDeclarationNode();
};

class VarDeclarationNode : public ParseTreeNode
{
public:
    VarDeclarationNode();
};

class IdentifierListNode : public ParseTreeNode
{
public:
    IdentifierListNode();
};

class TypeNode : public ParseTreeNode
{
public:
    TypeNode();
};

class ArrayTypeNode : public ParseTreeNode
{
public:
    ArrayTypeNode();
};

class RangeNode : public ParseTreeNode
{
public:
    RangeNode();
};

class EnumeratedNode : public ParseTreeNode
{
public:
    EnumeratedNode();
};

class RecordTypeNode : public ParseTreeNode
{
public:
    RecordTypeNode();
};

class FieldListNode : public ParseTreeNode
{
public:
    FieldListNode();
};

class FieldPartNode : public ParseTreeNode
{
public:
    FieldPartNode();
};

class SubprogramDeclarationNode : public ParseTreeNode
{
public:
    SubprogramDeclarationNode();
};

class ProcedureDeclarationNode : public ParseTreeNode
{
public:
    ProcedureDeclarationNode();
};

class FunctionDeclarationNode : public ParseTreeNode
{
public:
    FunctionDeclarationNode();
};

class BlockNode : public ParseTreeNode
{
public:
    BlockNode();
};

class FormalParameterListNode : public ParseTreeNode
{
public:
    FormalParameterListNode();
};

class ParameterGroupNode : public ParseTreeNode
{
public:
    ParameterGroupNode();
};

class CompoundStatementNode : public ParseTreeNode
{
public:
    CompoundStatementNode();
};

class StatementListNode : public ParseTreeNode
{
public:
    StatementListNode();
};

class StatementNode : public ParseTreeNode
{
public:
    StatementNode();
};

class VariableNode : public ParseTreeNode
{
public:
    VariableNode();
};

class ComponentVariableNode : public ParseTreeNode
{
public:
    ComponentVariableNode();
};

class IndexListNode : public ParseTreeNode
{
public:
    IndexListNode();
};

class AssignmentStatementNode : public ParseTreeNode
{
public:
    AssignmentStatementNode();
};

class IfStatementNode : public ParseTreeNode
{
public:
    IfStatementNode();
};

class CaseStatementNode : public ParseTreeNode
{
public:
    CaseStatementNode();
};

class CaseBlockNode : public ParseTreeNode
{
public:
    CaseBlockNode();
};

class WhileStatementNode : public ParseTreeNode
{
public:
    WhileStatementNode();
};

class RepeatStatementNode : public ParseTreeNode
{
public:
    RepeatStatementNode();
};

class ForStatementNode : public ParseTreeNode
{
public:
    ForStatementNode();
};

class ProcedureFunctionCallNode : public ParseTreeNode
{
public:
    ProcedureFunctionCallNode();
};

class ParameterListNode : public ParseTreeNode
{
public:
    ParameterListNode();
};

class ExpressionNode : public ParseTreeNode
{
public:
    ExpressionNode();
};

class SimpleExpressionNode : public ParseTreeNode
{
public:
    SimpleExpressionNode();
};

class TermNode : public ParseTreeNode
{
public:
    TermNode();
};

class FactorNode : public ParseTreeNode
{
public:
    FactorNode();
};

class RelationalOperatorNode : public ParseTreeNode
{
public:
    RelationalOperatorNode();
};

class AdditiveOperatorNode : public ParseTreeNode
{
public:
    AdditiveOperatorNode();
};

class MultiplicativeOperatorNode : public ParseTreeNode
{
public:
    MultiplicativeOperatorNode();
};

class EmptyStatementNode : public ParseTreeNode
{
public:
    EmptyStatementNode();
};

#endif
