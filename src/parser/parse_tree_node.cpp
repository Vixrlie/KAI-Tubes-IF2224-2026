#include "parse_tree_node.h"

#include <sstream>

namespace
{
    std::string terminalLabel(const Token &token)
    {
        return formatToken(token);
    }
}

ParseTreeNode::ParseTreeNode(std::string name)
    : nodeName(std::move(name)) {}

const std::string &ParseTreeNode::name() const
{
    return nodeName;
}

const std::vector<std::unique_ptr<ParseTreeNode>> &ParseTreeNode::children() const
{
    return childNodes;
}

void ParseTreeNode::addChild(std::unique_ptr<ParseTreeNode> child)
{
    childNodes.push_back(std::move(child));
}

std::string ParseTreeNode::renderLabel() const
{
    return nodeName;
}

void ParseTreeNode::print(std::ostream &out, const std::string &prefix, bool isLast) const
{
    out << prefix;
    if (!prefix.empty())
    {
        out << (isLast ? "└── " : "├── ");
    }
    out << renderLabel() << '\n';

    const std::string childPrefix = prefix + (prefix.empty() ? "" : (isLast ? "    " : "│   "));
    for (std::size_t index = 0; index < childNodes.size(); ++index)
    {
        const bool childIsLast = index + 1 == childNodes.size();
        childNodes[index]->print(out, childPrefix, childIsLast);
    }
}

ParseTreeTerminalNode::ParseTreeTerminalNode(const Token &token)
    : ParseTreeNode("<terminal>"), terminalToken(token) {}

const Token &ParseTreeTerminalNode::token() const
{
    return terminalToken;
}

std::string ParseTreeTerminalNode::renderLabel() const
{
    return terminalLabel(terminalToken);
}

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
