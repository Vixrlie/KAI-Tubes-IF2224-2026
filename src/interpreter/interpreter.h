#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <string>
#include <vector>

#include "../codegen/intermediate_code.h"
#include "runtime_value.h"

namespace Interpreter
{
    class Interpreter
    {
    public:
        struct Result
        {
            bool success = false;
            std::string output;
            std::vector<std::string> errors;
        };

        Result run(const std::vector<CodeGen::Instruction> &program);

    private:
        static constexpr int kFrameHeaderSize = 3;
        static constexpr int kMaxStackSlots = 10000;

        std::vector<RuntimeValue> stack;
        int sp = 0;
        int bp = 0;
        int ip = 0;
        std::string output;
        std::vector<std::string> errors;
        bool halted = false;

        void reset();
        void addError(const std::string &message);

        bool push(const RuntimeValue &value);
        bool pop(RuntimeValue &value);

        bool ensureStackSize(int size);
        bool getIntFromStack(int index, int &out) const;
        bool setStackValue(int index, const RuntimeValue &value);
        bool validateJumpTargets(const std::vector<CodeGen::Instruction> &program);
        bool isProtectedFrameSlot(int address) const;
        bool checkedNeg(int value, int &out) const;
        bool checkedAdd(int lhs, int rhs, int &out) const;
        bool checkedSub(int lhs, int rhs, int &out) const;
        bool checkedMul(int lhs, int rhs, int &out) const;

        int resolveBase(int levelDelta, int currentBp);
        int resolveAddress(int levelDelta, int offset);

        bool getIntOperand(const CodeGen::Operand &operand, int &out) const;
        bool executeInstruction(const CodeGen::Instruction &inst,
                                const std::vector<CodeGen::Instruction> &program,
                                bool &advance);

        bool executeOp(int typeClass, int opCode);
        bool unaryOp(int typeClass, int opCode, RuntimeValue &result);
        bool binaryOp(int typeClass, int opCode, RuntimeValue &result);

        bool writeValue(const RuntimeValue &value, bool newline);
    };
}

#endif
