#ifndef AST_NODE_H
#define AST_NODE_H

#include <memory>
#include <string>
#include <vector>

// AST node hierarchy for the Arion language.
// Each node represents a semantic construct (operator-operand relationship)
// rather than a syntactic production rule.

namespace AST
{
    // Forward declarations
    class ASTNode;
    class ProgramNode;
    class BlockNode;
    class VarDeclNode;
    class ConstDeclNode;
    class TypeDeclNode;
    class ArrayTypeNode;
    class RecordTypeNode;
    class SubrangeTypeNode;
    class EnumeratedTypeNode;
    class NamedTypeNode;
    class ProcedureDeclNode;
    class FunctionDeclNode;
    class ParamNode;
    class AssignNode;
    class BinOpNode;
    class UnaryOpNode;
    class NumberNode;
    class RealNode;
    class CharNode;
    class StringNode;
    class BooleanNode;
    class VarRefNode;
    class ArrayAccessNode;
    class FieldAccessNode;
    class ProcCallNode;
    class FuncCallNode;
    class IfNode;
    class WhileNode;
    class RepeatNode;
    class ForNode;
    class CaseNode;
    class CaseBranchNode;
    class CompoundNode;
    class EmptyNode;

    // Annotation structure attached to each AST node after semantic analysis
    struct Annotation
    {
        int typeCode = -1;       // type index or BasicType enum value
        std::string typeName;    // human-readable type name
        int tabIndex = -1;       // index into symbol table (tab)
        int blockIndex = -1;     // index into block table (btab)
        int level = -1;          // lexical level
        bool initialized = false;
    };

    // Base class for all AST nodes
    class ASTNode
    {
    public:
        virtual ~ASTNode() = default;

        // Returns the node type name for display purposes
        virtual std::string nodeType() const = 0;

        // Annotation set by semantic analyzer
        Annotation annotation;

    protected:
        ASTNode() = default;
    };

    // Root node: represents the entire program
    class ProgramNode : public ASTNode
    {
    public:
        std::string name;
        std::vector<std::unique_ptr<ASTNode>> declarations;
        std::unique_ptr<CompoundNode> body;

        std::string nodeType() const override { return "Program"; }
    };

    // A compound statement (begin...end block)
    class CompoundNode : public ASTNode
    {
    public:
        std::vector<std::unique_ptr<ASTNode>> statements;

        std::string nodeType() const override { return "Compound"; }
    };

    // Variable declaration: one entry per identifier
    class VarDeclNode : public ASTNode
    {
    public:
        std::string name;
        std::unique_ptr<ASTNode> typeSpec; // type specification node

        std::string nodeType() const override { return "VarDecl"; }
    };

    // Constant declaration
    class ConstDeclNode : public ASTNode
    {
    public:
        std::string name;
        std::unique_ptr<ASTNode> value;

        std::string nodeType() const override { return "ConstDecl"; }
    };

    // Type declaration
    class TypeDeclNode : public ASTNode
    {
    public:
        std::string name;
        std::unique_ptr<ASTNode> typeSpec;

        std::string nodeType() const override { return "TypeDecl"; }
    };

    // Named type reference (e.g., "integer", "MyType")
    class NamedTypeNode : public ASTNode
    {
    public:
        std::string name;

        std::string nodeType() const override { return "NamedType"; }
    };

    // Array type: array[range] of elementType
    class ArrayTypeNode : public ASTNode
    {
    public:
        std::unique_ptr<ASTNode> indexType; // range or named type
        std::unique_ptr<ASTNode> elementType;

        std::string nodeType() const override { return "ArrayType"; }
    };

    // Record type
    class RecordTypeNode : public ASTNode
    {
    public:
        std::vector<std::unique_ptr<VarDeclNode>> fields;

        std::string nodeType() const override { return "RecordType"; }
    };

    // Subrange type: low..high
    class SubrangeTypeNode : public ASTNode
    {
    public:
        std::unique_ptr<ASTNode> low;
        std::unique_ptr<ASTNode> high;

        std::string nodeType() const override { return "SubrangeType"; }
    };

    // Enumerated type: (ident1, ident2, ...)
    class EnumeratedTypeNode : public ASTNode
    {
    public:
        std::vector<std::string> identifiers;

        std::string nodeType() const override { return "EnumeratedType"; }
    };

    // Procedure declaration
    class ProcedureDeclNode : public ASTNode
    {
    public:
        std::string name;
        std::vector<std::unique_ptr<ParamNode>> params;
        std::vector<std::unique_ptr<ASTNode>> localDecls;
        std::unique_ptr<CompoundNode> body;

        std::string nodeType() const override { return "ProcedureDecl"; }
    };

    // Function declaration
    class FunctionDeclNode : public ASTNode
    {
    public:
        std::string name;
        std::vector<std::unique_ptr<ParamNode>> params;
        std::string returnTypeName;
        std::vector<std::unique_ptr<ASTNode>> localDecls;
        std::unique_ptr<CompoundNode> body;

        std::string nodeType() const override { return "FunctionDecl"; }
    };

    // Parameter node (for procedure/function params)
    class ParamNode : public ASTNode
    {
    public:
        std::string name;
        std::string typeName;
        bool isVar = false; // var parameter (pass by reference)

        std::string nodeType() const override { return "Param"; }
    };

    // Assignment statement: target := value
    class AssignNode : public ASTNode
    {
    public:
        std::unique_ptr<ASTNode> target;
        std::unique_ptr<ASTNode> value;

        std::string nodeType() const override { return "Assign"; }
    };

    // Binary operation: left op right
    class BinOpNode : public ASTNode
    {
    public:
        std::string op;
        std::unique_ptr<ASTNode> left;
        std::unique_ptr<ASTNode> right;

        std::string nodeType() const override { return "BinOp"; }
    };

    // Unary operation: op operand
    class UnaryOpNode : public ASTNode
    {
    public:
        std::string op;
        std::unique_ptr<ASTNode> operand;

        std::string nodeType() const override { return "UnaryOp"; }
    };

    // Integer literal
    class NumberNode : public ASTNode
    {
    public:
        int value = 0;

        std::string nodeType() const override { return "Number"; }
    };

    // Real literal
    class RealNode : public ASTNode
    {
    public:
        double value = 0.0;

        std::string nodeType() const override { return "Real"; }
    };

    // Character literal
    class CharNode : public ASTNode
    {
    public:
        char value = '\0';

        std::string nodeType() const override { return "Char"; }
    };

    // String literal
    class StringNode : public ASTNode
    {
    public:
        std::string value;

        std::string nodeType() const override { return "String"; }
    };

    // Boolean literal
    class BooleanNode : public ASTNode
    {
    public:
        bool value = false;

        std::string nodeType() const override { return "Boolean"; }
    };

    // Variable reference
    class VarRefNode : public ASTNode
    {
    public:
        std::string name;

        std::string nodeType() const override { return "Var"; }
    };

    // Array element access: base[index]
    class ArrayAccessNode : public ASTNode
    {
    public:
        std::unique_ptr<ASTNode> base;
        std::vector<std::unique_ptr<ASTNode>> indices;

        std::string nodeType() const override { return "ArrayAccess"; }
    };

    // Record field access: base.field
    class FieldAccessNode : public ASTNode
    {
    public:
        std::unique_ptr<ASTNode> base;
        std::string fieldName;

        std::string nodeType() const override { return "FieldAccess"; }
    };

    // Procedure call
    class ProcCallNode : public ASTNode
    {
    public:
        std::string name;
        std::vector<std::unique_ptr<ASTNode>> args;

        std::string nodeType() const override { return "ProcCall"; }
    };

    // Function call (used in expressions)
    class FuncCallNode : public ASTNode
    {
    public:
        std::string name;
        std::vector<std::unique_ptr<ASTNode>> args;

        std::string nodeType() const override { return "FuncCall"; }
    };

    // If statement
    class IfNode : public ASTNode
    {
    public:
        std::unique_ptr<ASTNode> condition;
        std::unique_ptr<ASTNode> thenBranch;
        std::unique_ptr<ASTNode> elseBranch; // may be nullptr

        std::string nodeType() const override { return "If"; }
    };

    // While loop
    class WhileNode : public ASTNode
    {
    public:
        std::unique_ptr<ASTNode> condition;
        std::unique_ptr<CompoundNode> body;

        std::string nodeType() const override { return "While"; }
    };

    // Repeat-until loop
    class RepeatNode : public ASTNode
    {
    public:
        std::vector<std::unique_ptr<ASTNode>> statements;
        std::unique_ptr<ASTNode> condition;

        std::string nodeType() const override { return "Repeat"; }
    };

    // For loop
    class ForNode : public ASTNode
    {
    public:
        std::string varName;
        std::unique_ptr<ASTNode> startExpr;
        std::unique_ptr<ASTNode> endExpr;
        bool isDownTo = false;
        std::unique_ptr<CompoundNode> body;

        std::string nodeType() const override { return "For"; }
    };

    // Case statement
    class CaseNode : public ASTNode
    {
    public:
        std::unique_ptr<ASTNode> selector;
        std::vector<std::unique_ptr<CaseBranchNode>> branches;

        std::string nodeType() const override { return "Case"; }
    };

    // One branch of a case statement
    class CaseBranchNode : public ASTNode
    {
    public:
        std::vector<std::unique_ptr<ASTNode>> labels;
        std::unique_ptr<ASTNode> body;

        std::string nodeType() const override { return "CaseBranch"; }
    };

    // Empty statement (placeholder)
    class EmptyNode : public ASTNode
    {
    public:
        std::string nodeType() const override { return "Empty"; }
    };

} // namespace AST

#endif
