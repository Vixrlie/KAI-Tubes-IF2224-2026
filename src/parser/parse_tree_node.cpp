#include "parse_tree_node.h"

#include <sstream>

// This file owns how parse tree nodes store children and print themselves.
namespace
{
    // This keeps terminal nodes formatted the same way the spec prints tokens.
    std::string terminalLabel(const Token &token)
    {
        if (tokenHasValue(token.type))
        {
            return tokenTypeToString(token.type) + "(" + token.value + ")";
        }
        return tokenTypeToString(token.type);
    }
}

// This constructor stores the printable label for one parse tree node.
ParseTreeNode::ParseTreeNode(std::string name)
    : nodeName(std::move(name)), parentNode(nullptr) {}

// This returns the node label used by helper code and searches.
const std::string &ParseTreeNode::name() const
{
    return nodeName;
}

// This exposes the node children without copying the tree.
const std::vector<std::unique_ptr<ParseTreeNode>> &ParseTreeNode::children() const
{
    return childNodes;
}

// This gives mutable access to the parent pointer when tree helpers need it.
ParseTreeNode *ParseTreeNode::parent()
{
    return parentNode;
}

// This gives read-only access to the parent pointer for const tree traversals.
const ParseTreeNode *ParseTreeNode::parent() const
{
    return parentNode;
}

// This attaches a child and wires its parent pointer in one place.
void ParseTreeNode::addChild(std::unique_ptr<ParseTreeNode> child)
{
    if (!child)
    {
        return;
    }

    child->parentNode = this;
    childNodes.push_back(std::move(child));
}

// This returns the default label unless a subclass wants a custom terminal rendering.
std::string ParseTreeNode::renderLabel() const
{
    return nodeName;
}

// This prints the whole subtree using box-drawing indentation.
void ParseTreeNode::print(std::ostream &out, const std::string &prefix, bool isLast) const
{
    out << prefix;
    if (parentNode != nullptr)
    {
        out << (isLast ? "└── " : "├── ");
    }
    out << renderLabel() << '\n';

    const std::string childPrefix = prefix + (parentNode == nullptr ? "" : (isLast ? "    " : "│   "));
    for (std::size_t index = 0; index < childNodes.size(); ++index)
    {
        const bool childIsLast = index + 1 == childNodes.size();
        childNodes[index]->print(out, childPrefix, childIsLast);
    }
}

// This constructor wraps one terminal token as a leaf parse tree node.
ParseTreeTerminalNode::ParseTreeTerminalNode(const Token &token)
    : ParseTreeNode("<terminal>"), terminalToken(token) {}

// This returns the stored terminal token for downstream inspection.
const Token &ParseTreeTerminalNode::token() const
{
    return terminalToken;
}

// This renders terminal leaves using token text instead of the generic node name.
std::string ParseTreeTerminalNode::renderLabel() const
{
    return terminalLabel(terminalToken);
}

// These constructors only bind each concrete node type to its grammar label.
ProgramNode::ProgramNode() : ParseTreeNode("<program>") {}
ProgramHeaderNode::ProgramHeaderNode() : ParseTreeNode("<program-header>") {}
DeclarationPartNode::DeclarationPartNode() : ParseTreeNode("<declaration-part>") {}
ConstDeclarationNode::ConstDeclarationNode() : ParseTreeNode("<const-declaration>") {}
ConstantNode::ConstantNode() : ParseTreeNode("<constant>") {}
TypeDeclarationNode::TypeDeclarationNode() : ParseTreeNode("<type-declaration>") {}
VarDeclarationNode::VarDeclarationNode() : ParseTreeNode("<var-declaration>") {}
IdentifierListNode::IdentifierListNode() : ParseTreeNode("<identifier-list>") {}
TypeNode::TypeNode() : ParseTreeNode("<type>") {}
ArrayTypeNode::ArrayTypeNode() : ParseTreeNode("<array-type>") {}
RangeNode::RangeNode() : ParseTreeNode("<range>") {}
EnumeratedNode::EnumeratedNode() : ParseTreeNode("<enumerated>") {}
RecordTypeNode::RecordTypeNode() : ParseTreeNode("<record-type>") {}
FieldListNode::FieldListNode() : ParseTreeNode("<field-list>") {}
FieldPartNode::FieldPartNode() : ParseTreeNode("<field-part>") {}
SubprogramDeclarationNode::SubprogramDeclarationNode() : ParseTreeNode("<subprogram-declaration>") {}
ProcedureDeclarationNode::ProcedureDeclarationNode() : ParseTreeNode("<procedure-declaration>") {}
FunctionDeclarationNode::FunctionDeclarationNode() : ParseTreeNode("<function-declaration>") {}
BlockNode::BlockNode() : ParseTreeNode("<block>") {}
FormalParameterListNode::FormalParameterListNode() : ParseTreeNode("<formal-parameter-list>") {}
ParameterGroupNode::ParameterGroupNode() : ParseTreeNode("<parameter-group>") {}
CompoundStatementNode::CompoundStatementNode() : ParseTreeNode("<compound-statement>") {}
StatementListNode::StatementListNode() : ParseTreeNode("<statement-list>") {}
StatementNode::StatementNode() : ParseTreeNode("<statement>") {}
VariableNode::VariableNode() : ParseTreeNode("<variable>") {}
ComponentVariableNode::ComponentVariableNode() : ParseTreeNode("<component-variable>") {}
IndexListNode::IndexListNode() : ParseTreeNode("<index-list>") {}
AssignmentStatementNode::AssignmentStatementNode() : ParseTreeNode("<assignment-statement>") {}
IfStatementNode::IfStatementNode() : ParseTreeNode("<if-statement>") {}
CaseStatementNode::CaseStatementNode() : ParseTreeNode("<case-statement>") {}
CaseBlockNode::CaseBlockNode() : ParseTreeNode("<case-block>") {}
WhileStatementNode::WhileStatementNode() : ParseTreeNode("<while-statement>") {}
RepeatStatementNode::RepeatStatementNode() : ParseTreeNode("<repeat-statement>") {}
ForStatementNode::ForStatementNode() : ParseTreeNode("<for-statement>") {}
ProcedureFunctionCallNode::ProcedureFunctionCallNode() : ParseTreeNode("<procedure/function-call>") {}
ParameterListNode::ParameterListNode() : ParseTreeNode("<parameter-list>") {}
ExpressionNode::ExpressionNode() : ParseTreeNode("<expression>") {}
SimpleExpressionNode::SimpleExpressionNode() : ParseTreeNode("<simple-expression>") {}
TermNode::TermNode() : ParseTreeNode("<term>") {}
FactorNode::FactorNode() : ParseTreeNode("<factor>") {}
RelationalOperatorNode::RelationalOperatorNode() : ParseTreeNode("<relational-operator>") {}
AdditiveOperatorNode::AdditiveOperatorNode() : ParseTreeNode("<additive-operator>") {}
MultiplicativeOperatorNode::MultiplicativeOperatorNode() : ParseTreeNode("<multiplicative-operator>") {}
EmptyStatementNode::EmptyStatementNode() : ParseTreeNode("<empty-statement>") {}
