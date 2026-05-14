#ifndef AST_BUILDER_H
#define AST_BUILDER_H

#include <memory>
#include <string>
#include <vector>

#include "../parser/parse_tree_node.h"
#include "ast_node.h"

// Syntax-Directed Translation: converts a parse tree (CST) into an AST.
// Each visit method corresponds to a grammar production and applies
// semantic actions to build the appropriate AST node.

namespace AST
{
    class ASTBuilder
    {
    public:
        // Entry point: converts the root parse tree node into an AST ProgramNode.
        std::unique_ptr<ProgramNode> build(const ParseTreeNode *parseTree);

    private:
        // Visit functions for each parse tree node type (Syntax-Directed Translation)
        std::unique_ptr<ProgramNode> visitProgram(const ParseTreeNode *node);
        std::string visitProgramHeader(const ParseTreeNode *node);
        std::vector<std::unique_ptr<ASTNode>> visitDeclarationPart(const ParseTreeNode *node);
        std::vector<std::unique_ptr<ASTNode>> visitConstDeclaration(const ParseTreeNode *node);
        std::unique_ptr<ASTNode> visitConstant(const ParseTreeNode *node);
        std::vector<std::unique_ptr<ASTNode>> visitTypeDeclaration(const ParseTreeNode *node);
        std::vector<std::unique_ptr<ASTNode>> visitVarDeclaration(const ParseTreeNode *node);
        std::vector<std::string> visitIdentifierList(const ParseTreeNode *node);
        std::unique_ptr<ASTNode> visitType(const ParseTreeNode *node);
        std::unique_ptr<ArrayTypeNode> visitArrayType(const ParseTreeNode *node);
        std::unique_ptr<SubrangeTypeNode> visitRange(const ParseTreeNode *node);
        std::unique_ptr<EnumeratedTypeNode> visitEnumerated(const ParseTreeNode *node);
        std::unique_ptr<RecordTypeNode> visitRecordType(const ParseTreeNode *node);
        std::vector<std::unique_ptr<VarDeclNode>> visitFieldList(const ParseTreeNode *node);
        std::vector<std::unique_ptr<VarDeclNode>> visitFieldPart(const ParseTreeNode *node);
        std::unique_ptr<ASTNode> visitSubprogramDeclaration(const ParseTreeNode *node);
        std::unique_ptr<ProcedureDeclNode> visitProcedureDeclaration(const ParseTreeNode *node);
        std::unique_ptr<FunctionDeclNode> visitFunctionDeclaration(const ParseTreeNode *node);
        std::vector<std::unique_ptr<ParamNode>> visitFormalParameterList(const ParseTreeNode *node);
        std::vector<std::unique_ptr<ParamNode>> visitParameterGroup(const ParseTreeNode *node);
        std::unique_ptr<CompoundNode> visitCompoundStatement(const ParseTreeNode *node);
        std::vector<std::unique_ptr<ASTNode>> visitStatementList(const ParseTreeNode *node);
        std::unique_ptr<ASTNode> visitStatement(const ParseTreeNode *node);
        std::unique_ptr<ASTNode> visitVariable(const ParseTreeNode *node);
        std::unique_ptr<AssignNode> visitAssignmentStatement(const ParseTreeNode *node);
        std::unique_ptr<IfNode> visitIfStatement(const ParseTreeNode *node);
        std::unique_ptr<CaseNode> visitCaseStatement(const ParseTreeNode *node);
        std::vector<std::unique_ptr<CaseBranchNode>> visitCaseBlock(const ParseTreeNode *node);
        std::unique_ptr<WhileNode> visitWhileStatement(const ParseTreeNode *node);
        std::unique_ptr<RepeatNode> visitRepeatStatement(const ParseTreeNode *node);
        std::unique_ptr<ForNode> visitForStatement(const ParseTreeNode *node);
        std::unique_ptr<ASTNode> visitProcedureFunctionCall(const ParseTreeNode *node);
        std::vector<std::unique_ptr<ASTNode>> visitParameterList(const ParseTreeNode *node);
        std::unique_ptr<ASTNode> visitExpression(const ParseTreeNode *node);
        std::unique_ptr<ASTNode> visitSimpleExpression(const ParseTreeNode *node);
        std::unique_ptr<ASTNode> visitTerm(const ParseTreeNode *node);
        std::unique_ptr<ASTNode> visitFactor(const ParseTreeNode *node);

        // Helper: extracts the operator string from an operator node
        std::string extractOperator(const ParseTreeNode *node);

        // Helper: gets the terminal token from a node (first terminal child)
        const Token *getTerminalToken(const ParseTreeNode *node) const;

        // Helper: finds a child node by name
        const ParseTreeNode *findChild(const ParseTreeNode *node, const std::string &name) const;

        // Helper: collects all children matching a name
        std::vector<const ParseTreeNode *> findChildren(const ParseTreeNode *node, const std::string &name) const;

        // Helper: gets terminal children (leaf tokens)
        std::vector<const ParseTreeTerminalNode *> getTerminals(const ParseTreeNode *node) const;
    };

} // namespace AST

#endif
