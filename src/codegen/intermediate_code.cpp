#include "intermediate_code.h"

#include <iomanip>
#include <ostream>
#include <sstream>

namespace CodeGen
{
    namespace
    {
        std::string opcodeToString(OpCode op)
        {
            switch (op)
            {
            case OpCode::LIT:
                return "LIT";
            case OpCode::LOD:
                return "LOD";
            case OpCode::STO:
                return "STO";
            case OpCode::CAL:
                return "CAL";
            case OpCode::INT:
                return "INT";
            case OpCode::JMP:
                return "JMP";
            case OpCode::JPC:
                return "JPC";
            case OpCode::OPR:
                return "OPR";
            case OpCode::RET:
                return "RET";
            default:
                return "???";
            }
        }

        std::string formatOperand(const Operand &operand)
        {
            return std::visit([](const auto &value) -> std::string
                              {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, std::monostate>)
                {
                    return "0";
                }
                else if constexpr (std::is_same_v<T, int>)
                {
                    return std::to_string(value);
                }
                else if constexpr (std::is_same_v<T, double>)
                {
                    std::ostringstream out;
                    out << std::setprecision(15) << value;
                    return out.str();
                }
                else if constexpr (std::is_same_v<T, char>)
                {
                    std::string result = "'";
                    result.push_back(value);
                    result.push_back('\'');
                    return result;
                }
                else if constexpr (std::is_same_v<T, bool>)
                {
                    return value ? "true" : "false";
                }
                else
                {
                    return value;
                } },
                              operand);
        }
    } // namespace

    IntermediateCodeGenerator::Result IntermediateCodeGenerator::generate(const AST::ProgramNode *root,
                                                                          const Semantic::SymbolTable &symbolTable)
    {
        symbols = &symbolTable;
        instructions.clear();
        errors.clear();
        constantValues.clear();

        if (!root)
        {
            addError("IntermediateCodeGenerator: null AST root");
            return {false, {}, errors};
        }

        collectProgramConstants(root);
        emit(OpCode::INT, 0, initialFrameSize(root));

        const int mainJumpIndex = static_cast<int>(instructions.size());
        emit(OpCode::JMP, 0, 0);

        for (const auto &decl : root->declarations)
        {
            visitDeclaration(decl.get());
        }

        instructions[mainJumpIndex].operand = static_cast<int>(instructions.size());

        if (root->body)
        {
            visitCompound(root->body.get());
        }

        emit(OpCode::RET, 0, 0);

        Result result;
        result.success = errors.empty();
        result.instructions = instructions;
        result.errors = errors;
        return result;
    }

    void IntermediateCodeGenerator::print(std::ostream &out, const std::vector<Instruction> &instructions)
    {
        for (std::size_t i = 0; i < instructions.size(); ++i)
        {
            const Instruction &inst = instructions[i];
            out << i << ' '
                << opcodeToString(inst.op) << ' '
                << inst.level << ' '
                << formatOperand(inst.operand) << '\n';
        }
    }

    void IntermediateCodeGenerator::addError(const std::string &message)
    {
        errors.push_back(message);
    }

    void IntermediateCodeGenerator::emit(OpCode op, int level, Operand operand)
    {
        instructions.push_back(Instruction{op, level, std::move(operand)});
    }

    void IntermediateCodeGenerator::collectProgramConstants(const AST::ProgramNode *root)
    {
        if (!root)
        {
            return;
        }

        for (const auto &decl : root->declarations)
        {
            auto *constDecl = dynamic_cast<const AST::ConstDeclNode *>(decl.get());
            if (constDecl && constDecl->value)
            {
                constantValues[constDecl->name] = constDecl->value.get();
            }
        }
    }

    int IntermediateCodeGenerator::initialFrameSize(const AST::ProgramNode *root)
    {
        if (!root || !symbols)
        {
            return kFrameHeaderSize;
        }

        int blockIndex = root->annotation.blockIndex;
        const auto &btab = symbols->btab();
        if (blockIndex < 0 || blockIndex >= static_cast<int>(btab.size()))
        {
            addError("Program block index is invalid for memory initialization");
            return kFrameHeaderSize;
        }

        const Semantic::BtabEntry &block = btab[blockIndex];
        return kFrameHeaderSize + block.psze + block.vsze;
    }

    void IntermediateCodeGenerator::visitDeclaration(const AST::ASTNode *node)
    {
        if (!node)
        {
            return;
        }

        if (auto *proc = dynamic_cast<const AST::ProcedureDeclNode *>(node))
        {
            visitProcedure(proc);
            return;
        }

        if (auto *func = dynamic_cast<const AST::FunctionDeclNode *>(node))
        {
            visitFunction(func);
            return;
        }

        // other declarations (const / var / type) are ignored by generator
    }

    void IntermediateCodeGenerator::visitStatement(const AST::ASTNode *node)
    {
        if (!node || dynamic_cast<const AST::EmptyNode *>(node))
        {
            return;
        }

        if (auto *assign = dynamic_cast<const AST::AssignNode *>(node))
        {
            visitAssign(assign);
            return;
        }

        if (auto *compound = dynamic_cast<const AST::CompoundNode *>(node))
        {
            visitCompound(compound);
            return;
        }

        if (auto *ifStmt = dynamic_cast<const AST::IfNode *>(node))
        {
            visitIf(ifStmt);
            return;
        }

        if (auto *whileStmt = dynamic_cast<const AST::WhileNode *>(node))
        {
            visitWhile(whileStmt);
            return;
        }

        if (auto *procCall = dynamic_cast<const AST::ProcCallNode *>(node))
        {
            if (procCall->name == "writeln")
            {
                visitWrite(procCall);
            }
            else
            {
                const std::size_t instructionBase = instructions.size();
                const std::size_t errorBase = errors.size();

                // Evaluate arguments (simple pass-by-value handling)
                for (const auto &arg : procCall->args)
                {
                    visitExpression(arg.get());
                }

                // Determine lexical level delta if symbol metadata exists
                int levelDelta = 0;
                const Semantic::TabEntry *entry = nullptr;
                if (procCall->annotation.tabIndex >= 0)
                {
                    entry = lookupEntry(procCall->annotation.tabIndex);
                }
                if (entry)
                {
                    levelDelta = lexicalLevelDelta(procCall->annotation, *entry);
                }

                // Emit CAL with placeholder operand (to be backpatched when procedure emitted)
                int callIndex = static_cast<int>(instructions.size());
                emit(OpCode::CAL, levelDelta, 0);

                auto it = procedureEntryIndex.find(procCall->name);
                if (it != procedureEntryIndex.end())
                {
                    // target known, backpatch immediately
                    instructions[callIndex].operand = it->second;
                }
                else
                {
                    // record for later backpatch
                    pendingCallSites[procCall->name].push_back(callIndex);
                }

                // Clean up the actual arguments after the call returns.
                // The callee reads them as slots below its frame header, so the caller must pop them.
                if (!procCall->args.empty())
                {
                    emit(OpCode::INT, 0, -static_cast<int>(procCall->args.size()));
                }

                if (errors.size() > errorBase)
                {
                    instructions.resize(instructionBase);
                }
            }
            return;
        }

        addError("Unsupported statement for Bryan's intermediate code generator: " + describeNode(node));
    }

    void IntermediateCodeGenerator::visitCompound(const AST::CompoundNode *node)
    {
        if (!node)
        {
            return;
        }

        for (const auto &statement : node->statements)
        {
            visitStatement(statement.get());
        }
    }

    void IntermediateCodeGenerator::visitIf(const AST::IfNode *node)
    {
        if (!node)
        {
            return;
        }

        const std::size_t instructionBase = instructions.size();
        const std::size_t errorBase = errors.size();

        // Evaluate condition
        visitExpression(node->condition.get());

        // Jump to else (or end) if condition false
        int jpcIndex = static_cast<int>(instructions.size());
        emit(OpCode::JPC, 0, 0);

        // Then branch
        if (node->thenBranch)
        {
            visitStatement(node->thenBranch.get());
        }

        if (node->elseBranch)
        {
            // Jump over else branch after then
            int jmpIndex = static_cast<int>(instructions.size());
            emit(OpCode::JMP, 0, 0);

            // Backpatch JPC to start of else
            instructions[jpcIndex].operand = static_cast<int>(instructions.size());

            // Else branch
            visitStatement(node->elseBranch.get());

            // Backpatch JMP to end
            instructions[jmpIndex].operand = static_cast<int>(instructions.size());
        }
        else
        {
            // No else; backpatch JPC to point after then branch
            instructions[jpcIndex].operand = static_cast<int>(instructions.size());
        }

        if (errors.size() > errorBase)
        {
            instructions.resize(instructionBase);
        }
    }

    void IntermediateCodeGenerator::visitWhile(const AST::WhileNode *node)
    {
        if (!node)
        {
            return;
        }

        const std::size_t instructionBase = instructions.size();
        const std::size_t errorBase = errors.size();

        // Loop start
        int loopStart = static_cast<int>(instructions.size());

        // Evaluate condition
        visitExpression(node->condition.get());

        // If false, jump out of loop
        int jpcIndex = static_cast<int>(instructions.size());
        emit(OpCode::JPC, 0, 0);

        // Body
        if (node->body)
        {
            visitCompound(node->body.get());
        }

        // Jump back to loop start
        emit(OpCode::JMP, 0, loopStart);

        // Backpatch exit
        instructions[jpcIndex].operand = static_cast<int>(instructions.size());

        if (errors.size() > errorBase)
        {
            instructions.resize(instructionBase);
        }
    }

    void IntermediateCodeGenerator::visitAssign(const AST::AssignNode *node)
    {
        if (!node)
        {
            return;
        }

        const std::size_t instructionBase = instructions.size();
        const std::size_t errorBase = errors.size();

        visitExpression(node->value.get());
        emitStore(node->target.get());

        if (errors.size() > errorBase)
        {
            instructions.resize(instructionBase);
        }
    }

    void IntermediateCodeGenerator::visitWrite(const AST::ProcCallNode *node)
    {
        if (!node)
        {
            return;
        }

        const std::size_t instructionBase = instructions.size();
        const std::size_t errorBase = errors.size();

        if (node->args.empty())
        {
            emit(OpCode::OPR, 0, kOpWrtn);
            return;
        }

        for (std::size_t i = 0; i < node->args.size(); ++i)
        {
            const AST::ASTNode *arg = node->args[i].get();
            visitExpression(arg);

            const bool isLast = (i + 1 == node->args.size());
            int typeClass = typeClassForNode(arg);
            if (typeClass < 0)
            {
                addError("Unsupported writeln argument type in " + describeNode(arg));
                typeClass = 0;
            }

            emit(OpCode::OPR, typeClass, isLast ? kOpWrtn : kOpWrt);
        }

        if (errors.size() > errorBase)
        {
            instructions.resize(instructionBase);
        }
    }

    void IntermediateCodeGenerator::visitExpression(const AST::ASTNode *node)
    {
        if (!node)
        {
            addError("Encountered null expression during code generation");
            return;
        }

        if (dynamic_cast<const AST::NumberNode *>(node) ||
            dynamic_cast<const AST::RealNode *>(node) ||
            dynamic_cast<const AST::CharNode *>(node) ||
            dynamic_cast<const AST::StringNode *>(node) ||
            dynamic_cast<const AST::BooleanNode *>(node))
        {
            emitLiteralNode(node);
            return;
        }

        if (auto *var = dynamic_cast<const AST::VarRefNode *>(node))
        {
            emitLoad(var);
            return;
        }

        if (auto *unary = dynamic_cast<const AST::UnaryOpNode *>(node))
        {
            if (unary->op != "-")
            {
                addError("Unsupported unary operator for intermediate code generation: " + unary->op);
                return;
            }

            visitExpression(unary->operand.get());
            int typeClass = typeClassForNode(node);
            if (typeClass < 0)
            {
                addError("Unsupported unary operand type in " + describeNode(node));
                typeClass = 0;
            }
            emit(OpCode::OPR, typeClass, kOpNeg);
            return;
        }

        if (auto *binOp = dynamic_cast<const AST::BinOpNode *>(node))
        {
            int opcode = opcodeForBinaryOp(binOp->op);
            if (opcode < 0)
            {
                addError("Unsupported binary operator for intermediate code generation: " + binOp->op);
                return;
            }

            visitExpression(binOp->left.get());
            visitExpression(binOp->right.get());

            int typeClass = typeClassForBinaryOp(binOp);
            if (typeClass < 0)
            {
                addError("Unsupported binary operand type in " + describeNode(node));
                typeClass = 0;
            }

            emit(OpCode::OPR, typeClass, opcode);
            return;
        }

        addError("Unsupported expression for intermediate code generation: " + describeNode(node));
    }

    int IntermediateCodeGenerator::emitLoad(const AST::VarRefNode *node)
    {
        const Semantic::TabEntry *entry = lookupVariableEntry(node);
        if (!entry)
        {
            return -1;
        }

        if (entry->obj == Semantic::ObjectClass::CONST)
        {
            emitConstantLiteral(*entry, node);
            return 0;
        }

        if (entry->obj != Semantic::ObjectClass::VAR)
        {
            addError("Identifier is not loadable as a variable: " + node->name);
            return -1;
        }

        int levelDelta = lexicalLevelDelta(node->annotation, *entry);
        int runtimeOffset = runtimeAddress(*entry);
        if (lookupParameterRuntimeOffset(node->annotation.tabIndex, runtimeOffset))
        {
            emit(OpCode::LOD, levelDelta, runtimeOffset);
            return 0;
        }

        emit(OpCode::LOD, levelDelta, runtimeOffset);
        return 0;
    }

    void IntermediateCodeGenerator::emitStore(const AST::ASTNode *target)
    {
        auto *var = dynamic_cast<const AST::VarRefNode *>(target);
        if (!var)
        {
            addError("Unsupported assignment target for intermediate code generation: " + describeNode(target));
            return;
        }

        const Semantic::TabEntry *entry = lookupVariableEntry(var);
        if (!entry)
        {
            return;
        }

        if (entry->obj != Semantic::ObjectClass::VAR)
        {
            addError("Assignment target is not a variable: " + var->name);
            return;
        }

        int levelDelta = lexicalLevelDelta(var->annotation, *entry);
        int runtimeOffset = runtimeAddress(*entry);
        if (lookupParameterRuntimeOffset(var->annotation.tabIndex, runtimeOffset))
        {
            emit(OpCode::STO, levelDelta, runtimeOffset);
            return;
        }

        emit(OpCode::STO, levelDelta, runtimeOffset);
    }

    void IntermediateCodeGenerator::emitLiteralNode(const AST::ASTNode *node)
    {
        if (auto *num = dynamic_cast<const AST::NumberNode *>(node))
        {
            emit(OpCode::LIT, typeClassFromBasicType(Semantic::BasicType::INTEGER), num->value);
            return;
        }

        if (auto *real = dynamic_cast<const AST::RealNode *>(node))
        {
            emit(OpCode::LIT, typeClassFromBasicType(Semantic::BasicType::REAL), real->value);
            return;
        }

        if (auto *ch = dynamic_cast<const AST::CharNode *>(node))
        {
            emit(OpCode::LIT, typeClassFromBasicType(Semantic::BasicType::CHAR), ch->value);
            return;
        }

        if (auto *str = dynamic_cast<const AST::StringNode *>(node))
        {
            emit(OpCode::LIT, typeClassFromBasicType(Semantic::BasicType::STRING), str->value);
            return;
        }

        if (auto *boolNode = dynamic_cast<const AST::BooleanNode *>(node))
        {
            emit(OpCode::LIT, typeClassFromBasicType(Semantic::BasicType::BOOLEAN), boolNode->value);
            return;
        }

        addError("Unsupported literal node in intermediate code generation: " + describeNode(node));
    }

    void IntermediateCodeGenerator::emitConstantLiteral(const Semantic::TabEntry &entry, const AST::VarRefNode *node)
    {
        auto it = constantValues.find(node->name);
        if (it != constantValues.end())
        {
            emitLiteralNode(it->second);
            return;
        }

        switch (entry.type.kind)
        {
        case Semantic::BasicType::INTEGER:
        case Semantic::BasicType::SUBRANGE:
        case Semantic::BasicType::ENUMERATED:
            emit(OpCode::LIT, typeClassFromTypeCode(static_cast<int>(entry.type.kind)), entry.adr);
            return;
        case Semantic::BasicType::CHAR:
            emit(OpCode::LIT, typeClassFromBasicType(Semantic::BasicType::CHAR), static_cast<char>(entry.adr));
            return;
        case Semantic::BasicType::BOOLEAN:
            emit(OpCode::LIT, typeClassFromBasicType(Semantic::BasicType::BOOLEAN), entry.adr != 0);
            return;
        default:
            addError("Constant value is not representable yet for code generation: " + node->name);
            return;
        }
    }

    void IntermediateCodeGenerator::pushParameterScope(const AST::ASTNode *subprogramNode)
    {
        std::unordered_map<int, int> parameterOffsets;

        const auto *procNode = dynamic_cast<const AST::ProcedureDeclNode *>(subprogramNode);
        const auto *funcNode = dynamic_cast<const AST::FunctionDeclNode *>(subprogramNode);

        const std::vector<std::unique_ptr<AST::ParamNode>> *params = nullptr;
        int blockIndex = -1;
        if (procNode)
        {
            params = &procNode->params;
            blockIndex = procNode->annotation.blockIndex;
        }
        else if (funcNode)
        {
            params = &funcNode->params;
            blockIndex = funcNode->annotation.blockIndex;
        }

        int totalParamSlots = 0;
        if (symbols && blockIndex >= 0)
        {
            const auto &btab = symbols->btab();
            if (blockIndex < static_cast<int>(btab.size()))
            {
                totalParamSlots = btab[blockIndex].psze;
            }
        }

        if (params)
        {
            int firstParamTabIndex = -1;
            if (symbols && blockIndex >= 0)
            {
                const auto &btab = symbols->btab();
                if (blockIndex < static_cast<int>(btab.size()))
                {
                    const Semantic::BtabEntry &block = btab[blockIndex];
                    firstParamTabIndex = block.lpar - static_cast<int>(params->size()) + 1;
                    (void)block;
                }
            }

            for (std::size_t i = 0; i < params->size(); ++i)
            {
                const auto &param = (*params)[i];
                if (!param)
                {
                    continue;
                }

                int paramTabIndex = firstParamTabIndex >= 0 ? firstParamTabIndex + static_cast<int>(i)
                                                            : param->annotation.tabIndex;
                const Semantic::TabEntry *entry = lookupEntry(paramTabIndex);
                if (!entry)
                {
                    continue;
                }

                // Parameters live below the callee frame header. Their runtime offsets are negative.
                parameterOffsets[paramTabIndex] = entry->adr - totalParamSlots;
            }
        }

        parameterOffsetStack.push_back(std::move(parameterOffsets));
    }

    void IntermediateCodeGenerator::popParameterScope()
    {
        if (!parameterOffsetStack.empty())
        {
            parameterOffsetStack.pop_back();
        }
    }

    bool IntermediateCodeGenerator::lookupParameterRuntimeOffset(int tabIndex, int &offset) const
    {
        for (auto it = parameterOffsetStack.rbegin(); it != parameterOffsetStack.rend(); ++it)
        {
            const auto found = it->find(tabIndex);
            if (found != it->end())
            {
                offset = found->second;
                return true;
            }
        }

        return false;
    }

    void IntermediateCodeGenerator::visitProcedure(const AST::ProcedureDeclNode *node)
    {
        if (!node)
        {
            return;
        }

        // Record entry point for the procedure
        int entryIndex = static_cast<int>(instructions.size());
        procedureEntryIndex[node->name] = entryIndex;

        // Skip over nested declarations when execution falls through the declaration area.
        emit(OpCode::JMP, 0, 0);

        // Backpatch any earlier call sites
        auto pendingIt = pendingCallSites.find(node->name);
        if (pendingIt != pendingCallSites.end())
        {
            for (int callIdx : pendingIt->second)
            {
                if (callIdx >= 0 && callIdx < static_cast<int>(instructions.size()))
                {
                    instructions[callIdx].operand = entryIndex;
                }
            }
            pendingCallSites.erase(pendingIt);
        }

        pushParameterScope(node);

        // Generate nested declarations before the executable prologue.
        for (const auto &decl : node->localDecls)
        {
            visitDeclaration(decl.get());
        }

        instructions[entryIndex].operand = static_cast<int>(instructions.size());

        // Reserve frame for the procedure
        int frameSize = kFrameHeaderSize;
        if (symbols)
        {
            int blockIndex = node->annotation.blockIndex;
            const auto &btab = symbols->btab();
            if (blockIndex >= 0 && blockIndex < static_cast<int>(btab.size()))
            {
                const Semantic::BtabEntry &block = btab[blockIndex];
                frameSize = kFrameHeaderSize + block.psze + block.vsze;
            }
            else
            {
                addError("Invalid block index for procedure: " + node->name);
            }
        }

        emit(OpCode::INT, 0, frameSize);

        // Generate body
        if (node->body)
        {
            visitCompound(node->body.get());
        }

        // Procedure return
        emit(OpCode::RET, 0, 0);
        popParameterScope();
    }

    void IntermediateCodeGenerator::visitFunction(const AST::FunctionDeclNode *node)
    {
        if (!node)
        {
            return;
        }

        // Treat function similarly to procedure for now (scaffold)
        int entryIndex = static_cast<int>(instructions.size());
        procedureEntryIndex[node->name] = entryIndex;

        emit(OpCode::JMP, 0, 0);

        auto pendingIt = pendingCallSites.find(node->name);
        if (pendingIt != pendingCallSites.end())
        {
            for (int callIdx : pendingIt->second)
            {
                if (callIdx >= 0 && callIdx < static_cast<int>(instructions.size()))
                {
                    instructions[callIdx].operand = entryIndex;
                }
            }
            pendingCallSites.erase(pendingIt);
        }

        pushParameterScope(node);

        for (const auto &decl : node->localDecls)
        {
            visitDeclaration(decl.get());
        }

        instructions[entryIndex].operand = static_cast<int>(instructions.size());

        int frameSize = kFrameHeaderSize;
        if (symbols)
        {
            int blockIndex = node->annotation.blockIndex;
            const auto &btab = symbols->btab();
            if (blockIndex >= 0 && blockIndex < static_cast<int>(btab.size()))
            {
                const Semantic::BtabEntry &block = btab[blockIndex];
                frameSize = kFrameHeaderSize + block.psze + block.vsze;
            }
            else
            {
                addError("Invalid block index for function: " + node->name);
            }
        }

        emit(OpCode::INT, 0, frameSize);

        if (node->body)
        {
            visitCompound(node->body.get());
        }

        // Function return (placeholder)
        emit(OpCode::RET, 0, 0);
        popParameterScope();
    }

    const Semantic::TabEntry *IntermediateCodeGenerator::lookupEntry(int tabIndex) const
    {
        if (!symbols)
        {
            return nullptr;
        }

        const auto &tab = symbols->tab();
        if (tabIndex < 0 || tabIndex >= static_cast<int>(tab.size()))
        {
            return nullptr;
        }

        return &tab[tabIndex];
    }

    const Semantic::TabEntry *IntermediateCodeGenerator::lookupVariableEntry(const AST::VarRefNode *node)
    {
        if (!node)
        {
            return nullptr;
        }

        const Semantic::TabEntry *entry = lookupEntry(node->annotation.tabIndex);
        if (entry)
        {
            return entry;
        }

        addError("Variable reference is missing symbol-table metadata: " + node->name);
        return nullptr;
    }

    int IntermediateCodeGenerator::lexicalLevelDelta(const AST::Annotation &annotation,
                                                     const Semantic::TabEntry &entry)
    {
        if (annotation.level < 0)
        {
            return 0;
        }

        int delta = annotation.level - entry.lev;
        if (delta < 0)
        {
            addError("Invalid lexical level relationship while generating code for " + entry.identifier);
            return 0;
        }

        return delta;
    }

    int IntermediateCodeGenerator::runtimeAddress(const Semantic::TabEntry &entry) const
    {
        return kFrameHeaderSize + entry.adr;
    }

    int IntermediateCodeGenerator::typeClassFromTypeCode(int typeCode) const
    {
        if (typeCode < 0)
        {
            return -1;
        }

        return typeClassFromBasicType(static_cast<Semantic::BasicType>(typeCode));
    }

    int IntermediateCodeGenerator::typeClassFromBasicType(Semantic::BasicType type) const
    {
        switch (type)
        {
        case Semantic::BasicType::INTEGER:
        case Semantic::BasicType::SUBRANGE:
        case Semantic::BasicType::ENUMERATED:
            return 0;
        case Semantic::BasicType::REAL:
            return 1;
        case Semantic::BasicType::CHAR:
            return 2;
        case Semantic::BasicType::BOOLEAN:
            return 3;
        case Semantic::BasicType::STRING:
            return 4;
        default:
            return -1;
        }
    }

    int IntermediateCodeGenerator::typeClassForNode(const AST::ASTNode *node) const
    {
        if (!node)
        {
            return -1;
        }

        return typeClassFromTypeCode(node->annotation.typeCode);
    }

    int IntermediateCodeGenerator::typeClassForBinaryOp(const AST::BinOpNode *node) const
    {
        if (!node)
        {
            return -1;
        }

        if (node->op == "=" || node->op == "<>" || node->op == "<" ||
            node->op == "<=" || node->op == ">" || node->op == ">=")
        {
            return typeClassForNode(node->left.get());
        }

        return typeClassForNode(node);
    }

    int IntermediateCodeGenerator::opcodeForBinaryOp(const std::string &op) const
    {
        if (op == "+")
        {
            return kOpAdd;
        }
        if (op == "-")
        {
            return kOpSub;
        }
        if (op == "*")
        {
            return kOpMul;
        }
        if (op == "/" || op == "div")
        {
            return kOpDiv;
        }
        if (op == "mod")
        {
            return kOpMod;
        }
        if (op == "=")
        {
            return kOpEql;
        }
        if (op == "<>")
        {
            return kOpNeq;
        }
        if (op == "<")
        {
            return kOpLss;
        }
        if (op == ">=")
        {
            return kOpGeq;
        }
        if (op == ">")
        {
            return kOpGtr;
        }
        if (op == "<=")
        {
            return kOpLeq;
        }

        return -1;
    }

    std::string IntermediateCodeGenerator::describeNode(const AST::ASTNode *node) const
    {
        return node ? node->nodeType() : "null";
    }
} // namespace CodeGen
