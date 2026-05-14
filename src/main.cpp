#include <fstream>
#include <iostream>
#include <sstream>

#include "lexer/lexer.h"
#include "lexer/token_stream_reader.h"
#include "parser/parser.h"
#include "semantic/ast_builder.h"
#include "semantic/ast_formatter.h"

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
        std::vector<Token> tokens;
        std::string tokenStreamError;

        if (TokenStreamReader::looksLikeTokenStream(source))
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

        // Phase 1: Parse source into parse tree (CST)
        Parser parser(tokens);
        std::unique_ptr<ProgramNode> parseTree = parser.parseProgram();

        // Phase 2: Convert parse tree to AST via Syntax-Directed Translation
        AST::ASTBuilder astBuilder;
        std::unique_ptr<AST::ProgramNode> ast = astBuilder.build(parseTree.get());

        // Output: Print the Decorated AST
        std::ostringstream outputBuffer;

        outputBuffer << "=== Parse Tree ===" << std::endl;
        parseTree->print(outputBuffer);
        outputBuffer << std::endl;

        outputBuffer << "=== Abstract Syntax Tree ===" << std::endl;
        AST::ASTFormatter formatter;
        formatter.print(outputBuffer, ast.get());

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

        return 0;
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
