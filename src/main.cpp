#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer/lexer.h"

int getEndLine(const Token &token)
{
    if (token.type == TokenType::COMMENT)
    {
        int newlines = 0;
        for (char c : token.value)
        {
            if (c == '\n')
                newlines++;
        }
        return token.line + newlines;
    }
    return token.line;
}

std::string formatOutput(const std::vector<Token> &tokens)
{
    std::string output;
    int prevEndLine = 0;

    for (const auto &token : tokens)
    {
        if (prevEndLine > 0 && token.line - prevEndLine > 1)
        {
            output += "\n";
        }
        output += formatToken(token) + "\n";
        prevEndLine = getEndLine(token);
    }

    return output;
}

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
    std::vector<Token> tokens = lexer.tokenize();

    std::string output = formatOutput(tokens);

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
        outputFile.close();
    }

    return lexer.hasErrors() ? 1 : 0;
}
