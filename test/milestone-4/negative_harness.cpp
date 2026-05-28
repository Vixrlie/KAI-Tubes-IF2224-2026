#include <fstream>
#include <vector>
#include <iostream>

#include "codegen/intermediate_code.h"
#include "interpreter/interpreter.h"

int main()
{
    // Invalid JMP target
    {
        std::vector<CodeGen::Instruction> prog;
        CodeGen::Instruction i0;
        i0.op = CodeGen::OpCode::INT;
        i0.level = 0;
        i0.operand = 3;
        CodeGen::Instruction i1;
        i1.op = CodeGen::OpCode::JMP;
        i1.level = 0;
        i1.operand = 9999;
        CodeGen::Instruction i2;
        i2.op = CodeGen::OpCode::RET;
        i2.level = 0;
        i2.operand = 0;
        prog.push_back(i0);
        prog.push_back(i1);
        prog.push_back(i2);

        Interpreter::Interpreter interp;
        auto res = interp.run(prog);

        std::ofstream out("test/milestone-4/invalid_jump_output.txt");
        out << "=== Interpreter Output ===\n"
            << res.output << "\n";
        out << "=== Interpreter Errors ===\n";
        for (const auto &e : res.errors)
        {
            out << "- " << e << "\n";
        }
    }

    // Frame-header smash attempt: INT 3 followed by STO offset 0
    {
        std::vector<CodeGen::Instruction> prog;
        CodeGen::Instruction i0;
        i0.op = CodeGen::OpCode::INT;
        i0.level = 0;
        i0.operand = 3;
        CodeGen::Instruction i1;
        i1.op = CodeGen::OpCode::LIT;
        i1.level = 0;
        i1.operand = 42;
        CodeGen::Instruction i2;
        i2.op = CodeGen::OpCode::STO;
        i2.level = 0;
        i2.operand = 0;
        CodeGen::Instruction i3;
        i3.op = CodeGen::OpCode::RET;
        i3.level = 0;
        i3.operand = 0;
        prog.push_back(i0);
        prog.push_back(i1);
        prog.push_back(i2);
        prog.push_back(i3);

        Interpreter::Interpreter interp;
        auto res = interp.run(prog);

        std::ofstream out("test/milestone-4/frame_smash_output.txt");
        out << "=== Interpreter Output ===\n"
            << res.output << "\n";
        out << "=== Interpreter Errors ===\n";
        for (const auto &e : res.errors)
        {
            out << "- " << e << "\n";
        }
    }

    // Invalid LOD address (out of bounds)
    {
        std::vector<CodeGen::Instruction> prog;
        CodeGen::Instruction i0;
        i0.op = CodeGen::OpCode::INT;
        i0.level = 0;
        i0.operand = 3;
        CodeGen::Instruction i1;
        i1.op = CodeGen::OpCode::LOD;
        i1.level = 0;
        i1.operand = 999;
        CodeGen::Instruction i2;
        i2.op = CodeGen::OpCode::RET;
        i2.level = 0;
        i2.operand = 0;
        prog.push_back(i0);
        prog.push_back(i1);
        prog.push_back(i2);

        Interpreter::Interpreter interp;
        auto res = interp.run(prog);

        std::ofstream out("test/milestone-4/invalid_lod_output.txt");
        out << "=== Interpreter Output ===\n"
            << res.output << "\n";
        out << "=== Interpreter Errors ===\n";
        for (const auto &e : res.errors)
        {
            out << "- " << e << "\n";
        }
    }

    // Invalid STO address (out of bounds write)
    {
        std::vector<CodeGen::Instruction> prog;
        CodeGen::Instruction i0;
        i0.op = CodeGen::OpCode::INT;
        i0.level = 0;
        i0.operand = 3;
        CodeGen::Instruction i1;
        i1.op = CodeGen::OpCode::LIT;
        i1.level = 0;
        i1.operand = 5;
        CodeGen::Instruction i2;
        i2.op = CodeGen::OpCode::STO;
        i2.level = 0;
        i2.operand = 9999;
        CodeGen::Instruction i3;
        i3.op = CodeGen::OpCode::RET;
        i3.level = 0;
        i3.operand = 0;
        prog.push_back(i0);
        prog.push_back(i1);
        prog.push_back(i2);
        prog.push_back(i3);

        Interpreter::Interpreter interp;
        auto res = interp.run(prog);

        std::ofstream out("test/milestone-4/invalid_sto_output.txt");
        out << "=== Interpreter Output ===\n"
            << res.output << "\n";
        out << "=== Interpreter Errors ===\n";
        for (const auto &e : res.errors)
        {
            out << "- " << e << "\n";
        }
    }

    return 0;
}
