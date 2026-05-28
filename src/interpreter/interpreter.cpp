#include "interpreter.h"

#include <sstream>

namespace Interpreter
{
    namespace
    {
        bool isComparisonOp(int opCode)
        {
            return opCode >= 7 && opCode <= 12;
        }
    } // namespace

    Interpreter::Result Interpreter::run(const std::vector<CodeGen::Instruction> &program)
    {
        reset();

        if (program.empty())
        {
            addError("Interpreter: empty program");
            return {false, output, errors};
        }

        while (!halted && ip >= 0 && ip < static_cast<int>(program.size()))
        {
            const CodeGen::Instruction &inst = program[static_cast<std::size_t>(ip)];
            bool advance = true;

            if (!executeInstruction(inst, program, advance))
            {
                break;
            }

            if (halted)
            {
                break;
            }

            if (advance)
            {
                ++ip;
            }
        }

        Result result;
        result.success = errors.empty();
        result.output = output;
        result.errors = errors;
        return result;
    }

    void Interpreter::reset()
    {
        stack.clear();
        sp = 0;
        bp = 0;
        ip = 0;
        output.clear();
        errors.clear();
        halted = false;
    }

    void Interpreter::addError(const std::string &message)
    {
        errors.push_back(message);
    }

    bool Interpreter::push(const RuntimeValue &value)
    {
        stack.push_back(value);
        sp = static_cast<int>(stack.size());
        return true;
    }

    bool Interpreter::pop(RuntimeValue &value)
    {
        if (stack.empty())
        {
            addError("Interpreter: stack underflow");
            return false;
        }

        value = stack.back();
        stack.pop_back();
        sp = static_cast<int>(stack.size());
        return true;
    }

    bool Interpreter::ensureStackSize(int size)
    {
        if (size < 0)
        {
            addError("Interpreter: invalid stack size request");
            return false;
        }

        if (static_cast<int>(stack.size()) < size)
        {
            stack.resize(static_cast<std::size_t>(size), RuntimeValue{0});
        }

        sp = static_cast<int>(stack.size());
        return true;
    }

    bool Interpreter::getIntFromStack(int index, int &out) const
    {
        if (index < 0 || index >= static_cast<int>(stack.size()))
        {
            return false;
        }

        return getIntValue(stack[static_cast<std::size_t>(index)], out);
    }

    bool Interpreter::setStackValue(int index, const RuntimeValue &value)
    {
        if (index < 0 || index >= static_cast<int>(stack.size()))
        {
            addError("Interpreter: stack access out of bounds");
            return false;
        }

        stack[static_cast<std::size_t>(index)] = value;
        return true;
    }

    int Interpreter::resolveBase(int levelDelta, int currentBp)
    {
        int base = currentBp;
        for (int i = 0; i < levelDelta; ++i)
        {
            int nextBase = 0;
            if (!getIntFromStack(base, nextBase))
            {
                addError("Interpreter: invalid static link while resolving base");
                return -1;
            }
            base = nextBase;
        }
        return base;
    }

    int Interpreter::resolveAddress(int levelDelta, int offset)
    {
        int base = resolveBase(levelDelta, bp);
        if (base < 0)
        {
            return -1;
        }

        return base + offset;
    }

    bool Interpreter::getIntOperand(const CodeGen::Operand &operand, int &out) const
    {
        if (const auto *ptr = std::get_if<int>(&operand))
        {
            out = *ptr;
            return true;
        }

        if (std::holds_alternative<std::monostate>(operand))
        {
            out = 0;
            return true;
        }

        return false;
    }

    bool Interpreter::executeInstruction(const CodeGen::Instruction &inst,
                                         const std::vector<CodeGen::Instruction> &program,
                                         bool &advance)
    {
        advance = true;

        switch (inst.op)
        {
        case CodeGen::OpCode::INT:
        {
            int size = 0;
            if (!getIntOperand(inst.operand, size))
            {
                addError("Interpreter: INT expects integer operand");
                return false;
            }

            if (stack.empty())
            {
                bp = 0;
            }

            int requiredSize = bp + size;
            if (!ensureStackSize(requiredSize))
            {
                return false;
            }

            // Only initialize frame header slots for the program's root frame (bp == 0).
            // For callee frames, the CAL instruction sets the header (static/dynamic/return).
            if (size >= kFrameHeaderSize && bp == 0)
            {
                setStackValue(bp + 0, RuntimeValue{0});
                setStackValue(bp + 1, RuntimeValue{0});
                setStackValue(bp + 2, RuntimeValue{0});
            }
            return true;
        }
        case CodeGen::OpCode::LIT:
        {
            if (std::holds_alternative<std::monostate>(inst.operand))
            {
                push(RuntimeValue{0});
                return true;
            }

            if (const auto *ptr = std::get_if<int>(&inst.operand))
            {
                push(RuntimeValue{*ptr});
                return true;
            }
            if (const auto *ptr = std::get_if<double>(&inst.operand))
            {
                push(RuntimeValue{*ptr});
                return true;
            }
            if (const auto *ptr = std::get_if<char>(&inst.operand))
            {
                push(RuntimeValue{*ptr});
                return true;
            }
            if (const auto *ptr = std::get_if<bool>(&inst.operand))
            {
                push(RuntimeValue{*ptr});
                return true;
            }
            if (const auto *ptr = std::get_if<std::string>(&inst.operand))
            {
                push(RuntimeValue{*ptr});
                return true;
            }

            addError("Interpreter: unsupported literal operand");
            return false;
        }
        case CodeGen::OpCode::LOD:
        {
            int offset = 0;
            if (!getIntOperand(inst.operand, offset))
            {
                addError("Interpreter: LOD expects integer operand");
                return false;
            }

            int address = resolveAddress(inst.level, offset);
            if (address < 0 || address >= static_cast<int>(stack.size()))
            {
                addError("Interpreter: LOD address out of bounds");
                return false;
            }

            push(stack[static_cast<std::size_t>(address)]);
            return true;
        }
        case CodeGen::OpCode::STO:
        {
            int offset = 0;
            if (!getIntOperand(inst.operand, offset))
            {
                addError("Interpreter: STO expects integer operand");
                return false;
            }

            int address = resolveAddress(inst.level, offset);
            if (address < 0 || address >= static_cast<int>(stack.size()))
            {
                addError("Interpreter: STO address out of bounds");
                return false;
            }

            RuntimeValue value;
            if (!pop(value))
            {
                return false;
            }

            stack[static_cast<std::size_t>(address)] = value;
            return true;
        }
        case CodeGen::OpCode::OPR:
        {
            int opcode = 0;
            if (!getIntOperand(inst.operand, opcode))
            {
                addError("Interpreter: OPR expects integer operand");
                return false;
            }

            if (!executeOp(inst.level, opcode))
            {
                return false;
            }

            return true;
        }
        case CodeGen::OpCode::JMP:
        {
            int target = 0;
            if (!getIntOperand(inst.operand, target))
            {
                addError("Interpreter: JMP expects integer operand");
                return false;
            }

            if (target < 0 || target >= static_cast<int>(program.size()))
            {
                addError("Interpreter: JMP target out of range");
                return false;
            }

            ip = target;
            advance = false;
            return true;
        }
        case CodeGen::OpCode::JPC:
        {
            int target = 0;
            if (!getIntOperand(inst.operand, target))
            {
                addError("Interpreter: JPC expects integer operand");
                return false;
            }

            RuntimeValue condition;
            if (!pop(condition))
            {
                return false;
            }

            bool cond = false;
            if (!getBoolValue(condition, cond))
            {
                addError("Interpreter: JPC expects boolean condition");
                return false;
            }

            if (!cond)
            {
                if (target < 0 || target >= static_cast<int>(program.size()))
                {
                    addError("Interpreter: JPC target out of range");
                    return false;
                }
                ip = target;
                advance = false;
            }

            return true;
        }
        case CodeGen::OpCode::CAL:
        {
            int target = 0;
            if (!getIntOperand(inst.operand, target))
            {
                addError("Interpreter: CAL expects integer operand");
                return false;
            }

            int staticLink = resolveBase(inst.level, bp);
            if (staticLink < 0)
            {
                return false;
            }

            int newBp = sp;
            if (!ensureStackSize(newBp + kFrameHeaderSize))
            {
                return false;
            }

            stack[static_cast<std::size_t>(newBp + 0)] = staticLink;
            stack[static_cast<std::size_t>(newBp + 1)] = bp;
            stack[static_cast<std::size_t>(newBp + 2)] = ip + 1;

            bp = newBp;
            ip = target;
            advance = false;
            return true;
        }
        case CodeGen::OpCode::RET:
        {
            int returnAddress = 0;
            int dynamicLink = 0;

            if (!getIntFromStack(bp + 2, returnAddress) || !getIntFromStack(bp + 1, dynamicLink))
            {
                addError("Interpreter: RET frame is invalid");
                return false;
            }

            if (bp == 0)
            {
                halted = true;
                return true;
            }

            if (dynamicLink < 0)
            {
                addError("Interpreter: RET dynamic link invalid");
                return false;
            }

            if (bp < 0 || bp > static_cast<int>(stack.size()))
            {
                addError("Interpreter: RET stack pointer invalid");
                return false;
            }

            stack.resize(static_cast<std::size_t>(bp));
            sp = static_cast<int>(stack.size());
            bp = dynamicLink;
            ip = returnAddress;
            advance = false;
            return true;
        }
        default:
            addError("Interpreter: unknown opcode encountered");
            return false;
        }
    }

    bool Interpreter::executeOp(int typeClass, int opCode)
    {
        if (opCode == 13 || opCode == 14)
        {
            RuntimeValue value;
            if (!pop(value))
            {
                return false;
            }

            return writeValue(value, opCode == 14);
        }

        RuntimeValue result;
        if (opCode == 1)
        {
            if (!unaryOp(typeClass, opCode, result))
            {
                return false;
            }
            return push(result);
        }

        if (!binaryOp(typeClass, opCode, result))
        {
            return false;
        }

        return push(result);
    }

    bool Interpreter::unaryOp(int typeClass, int opCode, RuntimeValue &result)
    {
        RuntimeValue value;
        if (!pop(value))
        {
            return false;
        }

        if (opCode != 1)
        {
            addError("Interpreter: unsupported unary OPR");
            return false;
        }

        if (typeClass == 0)
        {
            int intValue = 0;
            if (!getIntValue(value, intValue))
            {
                addError("Interpreter: unary NEG expects integer");
                return false;
            }
            result = -intValue;
            return true;
        }

        if (typeClass == 1)
        {
            double realValue = 0.0;
            if (!getDoubleValue(value, realValue))
            {
                addError("Interpreter: unary NEG expects real");
                return false;
            }
            result = -realValue;
            return true;
        }

        addError("Interpreter: unary NEG unsupported for this type");
        return false;
    }

    bool Interpreter::binaryOp(int typeClass, int opCode, RuntimeValue &result)
    {
        RuntimeValue right;
        RuntimeValue left;
        if (!pop(right) || !pop(left))
        {
            return false;
        }

        if (typeClass == 0)
        {
            int lhs = 0;
            int rhs = 0;
            if (!getIntValue(left, lhs) || !getIntValue(right, rhs))
            {
                addError("Interpreter: integer operation expects integer values");
                return false;
            }

            switch (opCode)
            {
            case 2:
                result = lhs + rhs;
                return true;
            case 3:
                result = lhs - rhs;
                return true;
            case 4:
                result = lhs * rhs;
                return true;
            case 5:
                if (rhs == 0)
                {
                    addError("Interpreter: division by zero");
                    return false;
                }
                result = lhs / rhs;
                return true;
            case 6:
                if (rhs == 0)
                {
                    addError("Interpreter: modulo by zero");
                    return false;
                }
                result = lhs % rhs;
                return true;
            case 7:
                result = lhs == rhs;
                return true;
            case 8:
                result = lhs != rhs;
                return true;
            case 9:
                result = lhs < rhs;
                return true;
            case 10:
                result = lhs >= rhs;
                return true;
            case 11:
                result = lhs > rhs;
                return true;
            case 12:
                result = lhs <= rhs;
                return true;
            default:
                break;
            }
        }
        else if (typeClass == 1)
        {
            double lhs = 0.0;
            double rhs = 0.0;
            if (!getDoubleValue(left, lhs) || !getDoubleValue(right, rhs))
            {
                addError("Interpreter: real operation expects real values");
                return false;
            }

            switch (opCode)
            {
            case 2:
                result = lhs + rhs;
                return true;
            case 3:
                result = lhs - rhs;
                return true;
            case 4:
                result = lhs * rhs;
                return true;
            case 5:
                if (rhs == 0.0)
                {
                    addError("Interpreter: division by zero");
                    return false;
                }
                result = lhs / rhs;
                return true;
            case 7:
                result = lhs == rhs;
                return true;
            case 8:
                result = lhs != rhs;
                return true;
            case 9:
                result = lhs < rhs;
                return true;
            case 10:
                result = lhs >= rhs;
                return true;
            case 11:
                result = lhs > rhs;
                return true;
            case 12:
                result = lhs <= rhs;
                return true;
            default:
                break;
            }
        }
        else if (typeClass == 2)
        {
            char lhs = '\0';
            char rhs = '\0';
            if (!std::holds_alternative<char>(left) || !std::holds_alternative<char>(right))
            {
                addError("Interpreter: char operation expects char values");
                return false;
            }
            lhs = std::get<char>(left);
            rhs = std::get<char>(right);

            switch (opCode)
            {
            case 7:
                result = lhs == rhs;
                return true;
            case 8:
                result = lhs != rhs;
                return true;
            case 9:
                result = lhs < rhs;
                return true;
            case 10:
                result = lhs >= rhs;
                return true;
            case 11:
                result = lhs > rhs;
                return true;
            case 12:
                result = lhs <= rhs;
                return true;
            default:
                break;
            }
        }
        else if (typeClass == 3)
        {
            bool lhs = false;
            bool rhs = false;
            if (!getBoolValue(left, lhs) || !getBoolValue(right, rhs))
            {
                addError("Interpreter: boolean operation expects boolean values");
                return false;
            }

            switch (opCode)
            {
            case 7:
                result = lhs == rhs;
                return true;
            case 8:
                result = lhs != rhs;
                return true;
            default:
                break;
            }
        }
        else if (typeClass == 4)
        {
            if (!std::holds_alternative<std::string>(left) || !std::holds_alternative<std::string>(right))
            {
                addError("Interpreter: string operation expects string values");
                return false;
            }
            const std::string &lhs = std::get<std::string>(left);
            const std::string &rhs = std::get<std::string>(right);

            switch (opCode)
            {
            case 2:
                result = lhs + rhs;
                return true;
            case 7:
                result = lhs == rhs;
                return true;
            case 8:
                result = lhs != rhs;
                return true;
            case 9:
                result = lhs < rhs;
                return true;
            case 10:
                result = lhs >= rhs;
                return true;
            case 11:
                result = lhs > rhs;
                return true;
            case 12:
                result = lhs <= rhs;
                return true;
            default:
                break;
            }
        }

        if (isComparisonOp(opCode))
        {
            addError("Interpreter: comparison failed due to type mismatch");
        }
        else
        {
            addError("Interpreter: arithmetic operation unsupported for this type");
        }
        return false;
    }

    bool Interpreter::writeValue(const RuntimeValue &value, bool newline)
    {
        output += runtimeValueToString(value);
        if (newline)
        {
            output += '\n';
        }
        return true;
    }
}
