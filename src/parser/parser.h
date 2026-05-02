#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "../lexer/token.h"
#include "parse_tree_node.h"

class ParserError : public std::runtime_error
{
public:
    ParserError(std::string message, int line, int column);

    int line() const;
    int column() const;

private:
    int errorLine;
    int errorColumn;
};

class Parser
{
public:
    explicit Parser(const std::vector<Token> &tokens);

    std::unique_ptr<ProgramNode> parseProgram();
    std::unique_ptr<ProgramHeaderNode> parseProgramHeader();

private:
    std::vector<Token> parserTokens;
    std::size_t currentIndex;

    bool isAtEnd() const;
    bool check(TokenType type) const;
    const Token &peek() const;
    const Token &advance();
    Token consume(TokenType type, const std::string &expectedName);
    std::unique_ptr<ParseTreeTerminalNode> makeTerminalNode(const Token &token) const;
    [[noreturn]] void throwSyntaxError(const std::string &expectedName) const;

    std::unique_ptr<DeclarationPartNode> parseDeclarationPart();
    std::unique_ptr<CompoundStatementNode> parseCompoundStatement();
    std::unique_ptr<StatementListNode> parseStatementList();
};

#endif
