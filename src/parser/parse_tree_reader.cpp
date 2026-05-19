#include "parse_tree_reader.h"

#include <cctype>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace
{
    const std::unordered_map<std::string, TokenType> &tokenTypeMap()
    {
        static const std::unordered_map<std::string, TokenType> mapping = {
            {"intcon", TokenType::INTCON},
            {"realcon", TokenType::REALCON},
            {"charcon", TokenType::CHARCON},
            {"string", TokenType::STRING},
            {"notsy", TokenType::NOTSY},
            {"andsy", TokenType::ANDSY},
            {"orsy", TokenType::ORSY},
            {"plus", TokenType::PLUS},
            {"minus", TokenType::MINUS},
            {"times", TokenType::TIMES},
            {"idiv", TokenType::IDIV},
            {"rdiv", TokenType::RDIV},
            {"imod", TokenType::IMOD},
            {"eql", TokenType::EQL},
            {"neq", TokenType::NEQ},
            {"gtr", TokenType::GTR},
            {"geq", TokenType::GEQ},
            {"lss", TokenType::LSS},
            {"leq", TokenType::LEQ},
            {"lparent", TokenType::LPARENT},
            {"rparent", TokenType::RPARENT},
            {"lbrack", TokenType::LBRACK},
            {"rbrack", TokenType::RBRACK},
            {"comma", TokenType::COMMA},
            {"semicolon", TokenType::SEMICOLON},
            {"period", TokenType::PERIOD},
            {"colon", TokenType::COLON},
            {"becomes", TokenType::BECOMES},
            {"constsy", TokenType::CONSTSY},
            {"typesy", TokenType::TYPESY},
            {"varsy", TokenType::VARSY},
            {"functionsy", TokenType::FUNCTIONSY},
            {"proceduresy", TokenType::PROCEDURESY},
            {"arraysy", TokenType::ARRAYSY},
            {"recordsy", TokenType::RECORDSY},
            {"programsy", TokenType::PROGRAMSY},
            {"ident", TokenType::IDENT},
            {"beginsy", TokenType::BEGINSY},
            {"ifsy", TokenType::IFSY},
            {"casesy", TokenType::CASESY},
            {"repeatsy", TokenType::REPEATSY},
            {"whilesy", TokenType::WHILESY},
            {"forsy", TokenType::FORSY},
            {"endsy", TokenType::ENDSY},
            {"elsesy", TokenType::ELSESY},
            {"untilsy", TokenType::UNTILSY},
            {"ofsy", TokenType::OFSY},
            {"dosy", TokenType::DOSY},
            {"tosy", TokenType::TOSY},
            {"downtosy", TokenType::DOWNTOSY},
            {"thensy", TokenType::THENSY},
            {"comment", TokenType::COMMENT},
            {"unknown", TokenType::UNKNOWN},
            {"error", TokenType::ERROR},
        };
        return mapping;
    }

    std::string trimRight(std::string value)
    {
        while (!value.empty() && (value.back() == '\r' || value.back() == ' ' || value.back() == '\t'))
        {
            value.pop_back();
        }
        return value;
    }

    std::string stripBom(std::string value)
    {
        const std::string bom = "\xEF\xBB\xBF";
        if (value.rfind(bom, 0) == 0)
        {
            return value.substr(bom.size());
        }
        return value;
    }

    bool startsWith(const std::string &text, std::size_t position, const std::string &prefix)
    {
        return text.compare(position, prefix.size(), prefix) == 0;
    }

    bool parseLine(const std::string &rawLine, int lineNumber, int &depth, std::string &label)
    {
        std::string line = stripBom(trimRight(rawLine));
        if (line.empty())
        {
            return false;
        }

        const std::string branch = "\xE2\x94\x9C\xE2\x94\x80\xE2\x94\x80 ";
        const std::string last = "\xE2\x94\x94\xE2\x94\x80\xE2\x94\x80 ";
        const std::string pipeGroup = "\xE2\x94\x82   ";
        const std::string blankGroup = "    ";

        if (line.rfind("<program>", 0) == 0)
        {
            depth = 0;
            label = "<program>";
            return true;
        }

        std::size_t position = 0;
        int groups = 0;
        while (position < line.size())
        {
            if (startsWith(line, position, pipeGroup))
            {
                position += pipeGroup.size();
                ++groups;
            }
            else if (startsWith(line, position, blankGroup))
            {
                position += blankGroup.size();
                ++groups;
            }
            else
            {
                break;
            }
        }

        if (startsWith(line, position, branch))
        {
            label = line.substr(position + branch.size());
        }
        else if (startsWith(line, position, last))
        {
            label = line.substr(position + last.size());
        }
        else
        {
            throw std::runtime_error("Invalid parse tree line " + std::to_string(lineNumber));
        }

        depth = groups + 1;
        return true;
    }

    std::unique_ptr<ParseTreeNode> makeNode(const std::string &label, int lineNumber)
    {
        if (!label.empty() && label.front() == '<' && label.back() == '>')
        {
            return std::make_unique<ParseTreeNode>(label);
        }

        std::string tokenName = label;
        std::string tokenValue;
        std::size_t openParen = label.find('(');
        if (openParen != std::string::npos && label.back() == ')')
        {
            tokenName = label.substr(0, openParen);
            tokenValue = label.substr(openParen + 1, label.size() - openParen - 2);
        }

        auto tokenIt = tokenTypeMap().find(tokenName);
        if (tokenIt == tokenTypeMap().end())
        {
            throw std::runtime_error("Invalid terminal in parse tree line " + std::to_string(lineNumber) + ": " + label);
        }

        Token token{tokenIt->second, tokenValue, lineNumber, 1};
        return std::make_unique<ParseTreeTerminalNode>(token);
    }
}

bool ParseTreeReader::looksLikeParseTree(const std::string &text)
{
    std::istringstream input(text);
    std::string line;
    while (std::getline(input, line))
    {
        line = stripBom(trimRight(line));
        if (line.empty())
        {
            continue;
        }
        return line == "<program>";
    }
    return false;
}

std::unique_ptr<ParseTreeNode> ParseTreeReader::read(const std::string &text)
{
    std::istringstream input(text);
    std::string line;
    std::unique_ptr<ParseTreeNode> root;
    std::vector<ParseTreeNode *> stack;
    int lineNumber = 0;

    while (std::getline(input, line))
    {
        ++lineNumber;

        int depth = 0;
        std::string label;
        if (!parseLine(line, lineNumber, depth, label))
        {
            continue;
        }

        auto node = makeNode(label, lineNumber);
        ParseTreeNode *nodePtr = node.get();

        if (depth == 0)
        {
            if (root)
            {
                throw std::runtime_error("Parse tree contains more than one root");
            }
            root = std::move(node);
            stack.clear();
            stack.push_back(root.get());
            continue;
        }

        if (!root || depth > static_cast<int>(stack.size()))
        {
            throw std::runtime_error("Invalid parse tree indentation at line " + std::to_string(lineNumber));
        }

        ParseTreeNode *parent = stack[depth - 1];
        parent->addChild(std::move(node));

        if (depth < static_cast<int>(stack.size()))
        {
            stack[depth] = nodePtr;
            stack.resize(depth + 1);
        }
        else
        {
            stack.push_back(nodePtr);
        }
    }

    if (!root || root->name() != "<program>")
    {
        throw std::runtime_error("Parse tree input must contain a <program> root");
    }

    return root;
}
