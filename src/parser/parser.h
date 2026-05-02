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
    bool check(TokenType type, std::size_t offset) const;
    const Token &peek() const;
    const Token *peek(std::size_t offset) const;
    const Token &advance();
    Token consume(TokenType type, const std::string &expectedName);
    std::unique_ptr<ParseTreeTerminalNode> makeTerminalNode(const Token &token) const;
    [[noreturn]] void throwSyntaxError(const std::string &expectedName) const;
    bool isRangeStart() const;

    std::unique_ptr<DeclarationPartNode> parseDeclarationPart();
    std::unique_ptr<ConstDeclarationNode> parseConstDeclaration();
    std::unique_ptr<ConstantNode> parseConstant();
    std::unique_ptr<TypeDeclarationNode> parseTypeDeclaration();
    std::unique_ptr<VarDeclarationNode> parseVarDeclaration();
    std::unique_ptr<IdentifierListNode> parseIdentifierList();
    std::unique_ptr<TypeNode> parseType();
    std::unique_ptr<ArrayTypeNode> parseArrayType();
    std::unique_ptr<RangeNode> parseRange();
    std::unique_ptr<EnumeratedNode> parseEnumerated();
    std::unique_ptr<RecordTypeNode> parseRecordType();
    std::unique_ptr<FieldListNode> parseFieldList();
    std::unique_ptr<FieldPartNode> parseFieldPart();
    std::unique_ptr<CompoundStatementNode> parseCompoundStatement();
    std::unique_ptr<StatementListNode> parseStatementList();
};

#endif
