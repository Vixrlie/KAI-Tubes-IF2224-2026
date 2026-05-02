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

std::unique_ptr<ParseTreeTerminalNode> Parser::makeTerminalNode(const Token &token) const
{
    return std::make_unique<ParseTreeTerminalNode>(token);
}

void Parser::throwSyntaxError(const std::string &expectedName) const
{
    std::ostringstream message;
    if (isAtEnd())
    {
        int line = parserTokens.empty() ? 1 : parserTokens.back().line;
        int column = parserTokens.empty() ? 1 : parserTokens.back().column;
        message << "Syntax error at line " << line
                << ", column " << column
                << ": unexpected end of input, expected " << expectedName;
        throw ParserError(message.str(), line, column);
    }

    const Token &token = peek();
    message << "Syntax error at line " << token.line
            << ", column " << token.column
            << ": unexpected token " << tokenTypeToString(token.type)
            << ", expected " << expectedName;
    throw ParserError(message.str(), token.line, token.column);
}

std::unique_ptr<DeclarationPartNode> Parser::parseDeclarationPart()
{
    return std::make_unique<DeclarationPartNode>();
}

std::unique_ptr<CompoundStatementNode> Parser::parseCompoundStatement()
{
    auto compoundStatementNode = std::make_unique<CompoundStatementNode>();
    compoundStatementNode->addChild(makeTerminalNode(consume(TokenType::BEGINSY, "beginsy")));
    compoundStatementNode->addChild(parseStatementList());
    compoundStatementNode->addChild(makeTerminalNode(consume(TokenType::ENDSY, "endsy")));
    return compoundStatementNode;
}

std::unique_ptr<StatementListNode> Parser::parseStatementList()
{
    auto statementListNode = std::make_unique<StatementListNode>();
    if (check(TokenType::ENDSY))
    {
        statementListNode->addChild(std::make_unique<EmptyStatementNode>());
        return statementListNode;
    }

    throwSyntaxError("endsy");
}