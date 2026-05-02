#include "parser.h"

#include <sstream>

namespace
{
    bool shouldSkipToken(const Token &token)
    {
        return token.type == TokenType::COMMENT;
    }

    std::string describeToken(const Token &token)
    {
        return tokenHasValue(token.type) ? formatToken(token) : tokenTypeToString(token.type);
    }
}

ParserError::ParserError(std::string message, int line, int column)
    : std::runtime_error(std::move(message)), errorLine(line), errorColumn(column) {}

int ParserError::line() const
{
    return errorLine;
}

int ParserError::column() const
{
    return errorColumn;
}

Parser::Parser(const std::vector<Token> &tokens)
    : currentIndex(0)
{
    parserTokens.reserve(tokens.size());
    for (const Token &token : tokens)
    {
        if (shouldSkipToken(token))
        {
            continue;
        }

        if (token.type == TokenType::ERROR || token.type == TokenType::UNKNOWN)
        {
            std::ostringstream message;
            message << "Cannot parse token stream containing lexical issue: " << describeToken(token);
            throw ParserError(message.str(), token.line, token.column);
        }

        parserTokens.push_back(token);
    }
}

std::unique_ptr<ProgramNode> Parser::parseProgram()
{
    auto programNode = std::make_unique<ProgramNode>();
    programNode->addChild(parseProgramHeader());
    programNode->addChild(parseDeclarationPart());
    programNode->addChild(parseCompoundStatement());
    programNode->addChild(makeTerminalNode(consume(TokenType::PERIOD, "period")));

    if (!isAtEnd())
    {
        throwSyntaxError("end of input");
    }

    return programNode;
}

std::unique_ptr<ProgramHeaderNode> Parser::parseProgramHeader()
{
    auto programHeaderNode = std::make_unique<ProgramHeaderNode>();
    programHeaderNode->addChild(makeTerminalNode(consume(TokenType::PROGRAMSY, "programsy")));
    programHeaderNode->addChild(makeTerminalNode(consume(TokenType::IDENT, "ident")));
    programHeaderNode->addChild(makeTerminalNode(consume(TokenType::SEMICOLON, "semicolon")));
    return programHeaderNode;
}

bool Parser::isAtEnd() const
{
    return currentIndex >= parserTokens.size();
}

bool Parser::check(TokenType type) const
{
    return !isAtEnd() && peek().type == type;
}

const Token &Parser::peek() const
{
    return parserTokens[currentIndex];
}

const Token &Parser::advance()
{
    return parserTokens[currentIndex++];
}

Token Parser::consume(TokenType type, const std::string &expectedName)
{
    if (!check(type))
    {
        throwSyntaxError(expectedName);
    }

    return advance();
}