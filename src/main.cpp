#include <fstream>
#include <iostream>
#include <sstream>

#include "lexer/lexer.h"
#include "parser/parser.h"

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
        std::vector<Token> tokens = lexer.tokenize();
        if (lexer.hasErrors())
        {
            return 1;
        }

        Parser parser(tokens);
        std::unique_ptr<ProgramNode> parseTree = parser.parseProgram();

        std::ostringstream outputBuffer;
        parseTree->print(outputBuffer);
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
