#ifndef INTERMEDIATE_CODE_H
#define INTERMEDIATE_CODE_H

#include <iosfwd>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "semantic/ast_node.h"
#include "semantic/symbol_table.h"

namespace CodeGen
{
    enum class OpCode
    {
        LIT,
        LOD,
        STO,
        ALOD,
        ASTO,
        CAL,
        INT,
        JMP,
        JPC,
        OPR,
        RET
    };

    using Operand = std::variant<std::monostate, int, double, char, bool, std::string>;

    struct Instruction
    {
        OpCode op = OpCode::INT;
        int level = 0;
        Operand operand;
    };

    class IntermediateCodeGenerator
    {
    public:
        struct Result
        {
            bool success = false;
            std::vector<Instruction> instructions;
            std::vector<std::string> errors;
        };

        Result generate(const AST::ProgramNode *root, const Semantic::SymbolTable &symbols);

        static void print(std::ostream &out, const std::vector<Instruction> &instructions);

    private:
        static constexpr int kFrameHeaderSize = 3;
        static constexpr int kOpNeg = 1;
        static constexpr int kOpAdd = 2;
        static constexpr int kOpSub = 3;
        static constexpr int kOpMul = 4;
        static constexpr int kOpDiv = 5;
        static constexpr int kOpMod = 6;
        static constexpr int kOpEql = 7;
        static constexpr int kOpNeq = 8;
        static constexpr int kOpLss = 9;
        static constexpr int kOpGeq = 10;
        static constexpr int kOpGtr = 11;
        static constexpr int kOpLeq = 12;
        static constexpr int kOpWrt = 13;
        static constexpr int kOpWrtn = 14;

        const Semantic::SymbolTable *symbols = nullptr;
        std::vector<Instruction> instructions;
        std::vector<std::string> errors;
        std::unordered_map<std::string, const AST::ASTNode *> constantValues;
        std::unordered_map<std::string, int> procedureEntryIndex;
        std::unordered_map<std::string, std::vector<int>> pendingCallSites;
        std::vector<std::unordered_map<int, int>> parameterOffsetStack;

        void addError(const std::string &message);
        void emit(OpCode op, int level, Operand operand);

        void collectProgramConstants(const AST::ProgramNode *root);
        int initialFrameSize(const AST::ProgramNode *root);

        void visitDeclaration(const AST::ASTNode *node);
        void visitProcedure(const AST::ProcedureDeclNode *node);
        void visitFunction(const AST::FunctionDeclNode *node);
        void visitIf(const AST::IfNode *node);
        void visitWhile(const AST::WhileNode *node);
        void visitRepeat(const AST::RepeatNode *node);
        void visitFor(const AST::ForNode *node);
        void visitCase(const AST::CaseNode *node);
        void visitStatement(const AST::ASTNode *node);
        void visitCompound(const AST::CompoundNode *node);
        void visitAssign(const AST::AssignNode *node);
        void visitWrite(const AST::ProcCallNode *node);
        void visitExpression(const AST::ASTNode *node);

        int emitLoad(const AST::VarRefNode *node);
        void emitStore(const AST::ASTNode *target);
        void emitLiteralNode(const AST::ASTNode *node);
        void emitConstantLiteral(const Semantic::TabEntry &entry, const AST::VarRefNode *node);
        void pushParameterScope(const AST::ASTNode *subprogramNode);
        void popParameterScope();
        bool lookupParameterRuntimeOffset(int tabIndex, int &offset) const;

        const Semantic::TabEntry *lookupEntry(int tabIndex) const;
        const Semantic::TabEntry *lookupVariableEntry(const AST::VarRefNode *node);
        int lexicalLevelDelta(const AST::Annotation &annotation, const Semantic::TabEntry &entry);
        int runtimeAddress(const Semantic::TabEntry &entry) const;

        int typeClassFromTypeCode(int typeCode) const;
        int typeClassFromBasicType(Semantic::BasicType type) const;
        int typeClassForNode(const AST::ASTNode *node) const;
        int typeClassForBinaryOp(const AST::BinOpNode *node) const;
        int opcodeForBinaryOp(const std::string &op) const;
        std::string describeNode(const AST::ASTNode *node) const;
    };
} // namespace CodeGen

#endif
