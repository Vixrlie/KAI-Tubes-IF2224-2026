#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "lexer/lexer.h"
#include "lexer/token_stream_reader.h"
#include "parser/parser.h"
#include "parser/parse_tree_reader.h"
#include "codegen/intermediate_code.h"
#include "interpreter/interpreter.h"
#include "semantic/ast_builder.h"
#include "semantic/ast_formatter.h"
#include "semantic/semantic_analyzer.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input.txt> [output.txt]" << std::endl;
        return 1;
    }

    std::ifstream inputFile(argv[1]);
    if (!inputFile.is_open())
    {
        std::cerr << "Error: Cannot open input file '" << argv[1] << "'" << std::endl;
        return 1;
    }

    std::stringstream ss;
    ss << inputFile.rdbuf();
    std::string source = ss.str();
    inputFile.close();

    Lexer lexer(source);
    try
    {
        std::unique_ptr<ParseTreeNode> parseTree;
        std::vector<Token> tokens;
        std::string tokenStreamError;

        if (ParseTreeReader::looksLikeParseTree(source))
        {
            parseTree = ParseTreeReader::read(source);
        }
        else if (TokenStreamReader::looksLikeTokenStream(source))
        {
            if (!TokenStreamReader::tryRead(source, tokens, tokenStreamError))
            {
                std::cerr << "Error: " << tokenStreamError << std::endl;
                return 1;
            }
        }
        else
        {
            tokens = lexer.tokenize();
            if (lexer.hasErrors())
            {
                return 1;
            }
        }

        // Phase 1: Parse source into parse tree (CST), unless a parse tree was provided.
        if (!parseTree)
        {
            Parser parser(tokens);
            parseTree = parser.parseProgram();
        }

        // Phase 2: Convert parse tree to AST via Syntax-Directed Translation
        AST::ASTBuilder astBuilder;
        std::unique_ptr<AST::ProgramNode> ast = astBuilder.build(parseTree.get());

        // Phase 3: Semantic analysis and AST decoration
        Semantic::SemanticAnalyzer analyzer;
        Semantic::SemanticAnalyzer::Result semResult = analyzer.analyze(ast.get());
        if (!semResult.success)
        {
            std::cerr << "=== Semantic Errors ===" << std::endl;
            for (const auto &message : semResult.errors)
            {
                std::cerr << "- " << message << std::endl;
            }
            return 1;
        }

        CodeGen::IntermediateCodeGenerator generator;
        CodeGen::IntermediateCodeGenerator::Result codegenResult =
            generator.generate(ast.get(), analyzer.symbolTable());

        auto objectClassLabel = [](Semantic::ObjectClass obj) {
            switch (obj)
            {
            case Semantic::ObjectClass::PROGRAM:
                return "program";
            case Semantic::ObjectClass::CONST:
                return "const";
            case Semantic::ObjectClass::VAR:
                return "variable";
            case Semantic::ObjectClass::TYPE:
                return "type";
            case Semantic::ObjectClass::PROCEDURE:
                return "procedure";
            case Semantic::ObjectClass::FUNCTION:
                return "function";
            default:
                return "unknown";
            }
        };

        // Output: Print symbol tables and the Decorated AST
        std::ostringstream outputBuffer;

        const Semantic::SymbolTable &symbols = analyzer.symbolTable();
        const auto &tab = symbols.tab();
        const auto &btab = symbols.btab();
        const auto &atab = symbols.atab();

        outputBuffer << "=== tab ===" << std::endl;
        outputBuffer << "idx  id        obj       type  ref  nrm  lev  adr  link" << std::endl;
        outputBuffer << "-------------------------------------------------------" << std::endl;
        if (tab.size() > 33)
        {
            outputBuffer << "...  (reserved words 0-32)" << std::endl;
        }
        for (std::size_t i = 33; i < tab.size(); ++i)
        {
            const auto &entry = tab[i];
            outputBuffer << std::setw(3) << i << "  "
                         << std::setw(8) << entry.identifier << "  "
                         << std::setw(9) << objectClassLabel(entry.obj) << "  "
                         << std::setw(4) << static_cast<int>(entry.type.kind) << "  "
                         << std::setw(3) << entry.type.ref << "  "
                         << std::setw(3) << entry.nrm << "  "
                         << std::setw(3) << entry.lev << "  "
                         << std::setw(3) << entry.adr << "  "
                         << std::setw(4) << entry.link << std::endl;
        }
        outputBuffer << std::endl;

        outputBuffer << "=== btab ===" << std::endl;
        outputBuffer << "idx  last  lpar  psze  vsze" << std::endl;
        outputBuffer << "---------------------------" << std::endl;
        for (std::size_t i = 0; i < btab.size(); ++i)
        {
            const auto &entry = btab[i];
            outputBuffer << std::setw(3) << i << "  "
                         << std::setw(4) << entry.last << "  "
                         << std::setw(4) << entry.lpar << "  "
                         << std::setw(4) << entry.psze << "  "
                         << std::setw(4) << entry.vsze << std::endl;
        }
        outputBuffer << std::endl;

        outputBuffer << "=== atab ===" << std::endl;
        if (atab.empty())
        {
            outputBuffer << "(empty)" << std::endl;
        }
        else
        {
            outputBuffer << "idx  xtyp  etyp  eref  low  high  elsz  size" << std::endl;
            outputBuffer << "--------------------------------------------" << std::endl;
            for (std::size_t i = 0; i < atab.size(); ++i)
            {
                const auto &entry = atab[i];
                outputBuffer << std::setw(3) << i << "  "
                             << std::setw(4) << entry.xtyp << "  "
                             << std::setw(4) << entry.etyp << "  "
                             << std::setw(4) << entry.eref << "  "
                             << std::setw(4) << entry.low << "  "
                             << std::setw(4) << entry.high << "  "
                             << std::setw(4) << entry.elsz << "  "
                             << std::setw(4) << entry.size << std::endl;
            }
        }
        outputBuffer << std::endl;

        outputBuffer << "=== Parse Tree ===" << std::endl;
        parseTree->print(outputBuffer);
        outputBuffer << std::endl;

        outputBuffer << "=== Decorated AST ===" << std::endl;
        AST::ASTFormatter formatter;
        formatter.print(outputBuffer, ast.get());
        outputBuffer << std::endl;

        outputBuffer << "=== Intermediate Code ===" << std::endl;
        CodeGen::IntermediateCodeGenerator::print(outputBuffer, codegenResult.instructions);

        if (!codegenResult.success)
        {
            outputBuffer << std::endl;
            outputBuffer << "=== Intermediate Code Errors ===" << std::endl;
            for (const auto &message : codegenResult.errors)
            {
                outputBuffer << "- " << message << std::endl;
            }
        }

        if (codegenResult.success)
        {
            Interpreter::Interpreter interpreter;
            Interpreter::Interpreter::Result runResult = interpreter.run(codegenResult.instructions);

            outputBuffer << std::endl;
            outputBuffer << "=== Interpreter Output ===" << std::endl;
            outputBuffer << runResult.output;

            if (!runResult.success)
            {
                outputBuffer << std::endl;
                outputBuffer << "=== Interpreter Errors ===" << std::endl;
                for (const auto &message : runResult.errors)
                {
                    outputBuffer << "- " << message << std::endl;
                }
            }
        }

        const std::string output = outputBuffer.str();
        std::cout << output;

        if (argc >= 3)
        {
            std::ofstream outputFile(argv[2]);
            if (!outputFile.is_open())
            {
                std::cerr << "Error: Cannot open output file '" << argv[2] << "'" << std::endl;
                return 1;
            }
            outputFile << output;
        }

        return codegenResult.success ? 0 : 1;
    }
    catch (const ParserError &error)
    {
        std::cerr << error.what() << std::endl;
        return 1;
    }
    catch (const std::exception &error)
    {
        std::cerr << "Error: " << error.what() << std::endl;
        return 1;
    }
}
