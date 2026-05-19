#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include <string>
#include <unordered_map>
#include <vector>

#include "ast_node.h"
#include "symbol_table.h"

namespace Semantic
{
    class SemanticAnalyzer
    {
    public:
        struct Result
        {
            bool success = false;
            std::vector<std::string> errors;
        };

        Result analyze(AST::ProgramNode *root);
        const SymbolTable &symbolTable() const;

    private:
        struct TypeResult
        {
            TypeInfo info;
            std::string name;
            bool ok = false;
        };

        struct SubrangeInfo
        {
            TypeInfo baseType;
            bool hasIntBounds = false;
            int lowInt = 0;
            int highInt = 0;
            bool hasCharBounds = false;
            char lowChar = '\0';
            char highChar = '\0';
        };

        struct EnumInfo
        {
            std::vector<std::string> identifiers;
        };

        struct ProcSignature
        {
            bool isFunction = false;
            bool hasParamInfo = false;
            TypeInfo returnType;
            std::vector<TypeInfo> params;
            std::vector<bool> byRefParams;
        };

        SymbolTable symbols;
        std::vector<std::string> errors;
        std::vector<int> blockStack;
        std::vector<SubrangeInfo> subranges;
        std::vector<EnumInfo> enums;
        std::unordered_map<int, int> recordFieldLast;
        std::unordered_map<std::string, ProcSignature> procSignatures;
        std::vector<std::string> functionStack;

        void addError(const std::string &message);
        void addBuiltinProcedures();

        void enterScope();
        void exitScope();
        int currentBlock() const;
        int currentLevel() const;

        void annotateNode(AST::ASTNode *node, const TypeResult &typeResult, int tabIndex = -1);

        TypeResult unknownType() const;
        TypeResult basicType(BasicType kind) const;
        std::string typeNameFromKind(BasicType kind) const;

        TypeResult resolveTypeSpec(AST::ASTNode *node);
        TypeResult resolveNamedType(const std::string &name);

        TypeResult visitExpression(AST::ASTNode *node);
        TypeResult visitVariable(AST::ASTNode *node);
        TypeResult visitConstant(AST::ASTNode *node);

        void visitProgram(AST::ProgramNode *node);
        void visitDeclaration(AST::ASTNode *node);
        void visitConstDecl(AST::ConstDeclNode *node);
        void visitTypeDecl(AST::TypeDeclNode *node);
        void visitVarDecl(AST::VarDeclNode *node);
        void visitProcedureDecl(AST::ProcedureDeclNode *node);
        void visitFunctionDecl(AST::FunctionDeclNode *node);

        void visitStatement(AST::ASTNode *node);
        void visitCompound(AST::CompoundNode *node);
        void visitAssign(AST::AssignNode *node);
        void visitIf(AST::IfNode *node);
        void visitWhile(AST::WhileNode *node);
        void visitRepeat(AST::RepeatNode *node);
        void visitFor(AST::ForNode *node);
        void visitCase(AST::CaseNode *node);
        void visitCaseBranch(AST::CaseBranchNode *node, const TypeResult &selectorType);
        void visitProcCall(AST::ProcCallNode *node);
        TypeResult visitFuncCall(AST::FuncCallNode *node);

        bool isAssignableTarget(AST::ASTNode *node);
        bool isVariableLike(AST::ASTNode *node) const;
        bool guaranteesFunctionResult(AST::ASTNode *node, const std::string &functionName) const;
        bool assignsFunctionResult(AST::AssignNode *node, const std::string &functionName) const;
        int stringLiteralLength(const AST::ASTNode *node) const;

        bool isNumeric(const TypeInfo &type) const;
        bool isIntegerLike(const TypeInfo &type) const;
        bool isBoolean(const TypeInfo &type) const;
        bool isString(const TypeInfo &type) const;

        bool isSameType(const TypeInfo &lhs, const TypeInfo &rhs) const;
        bool isCompatible(const TypeInfo &lhs, const TypeInfo &rhs) const;
        bool isAssignmentCompatible(const TypeInfo &target, const TypeInfo &value, const AST::ASTNode *valueNode);
        TypeInfo unwrapSubrange(const TypeInfo &type) const;
    };
} // namespace Semantic

#endif
