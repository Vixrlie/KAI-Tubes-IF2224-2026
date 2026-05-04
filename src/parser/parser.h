#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "../lexer/token.h"
#include "parse_tree_node.h"

// This header defines the parser interface and the nonterminal parsing entry points.
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
    bool isStatementFollow() const;
    bool isVariableSuffixStart() const;
    bool isIndexToken() const;
    std::unique_ptr<VariableNode> parseVariableWithBase(std::unique_ptr<VariableNode> baseVariable);

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
    std::unique_ptr<SubprogramDeclarationNode> parseSubprogramDeclaration();
    std::unique_ptr<ProcedureDeclarationNode> parseProcedureDeclaration();
    std::unique_ptr<FunctionDeclarationNode> parseFunctionDeclaration();
    std::unique_ptr<BlockNode> parseBlock();
    std::unique_ptr<FormalParameterListNode> parseFormalParameterList();
    std::unique_ptr<ParameterGroupNode> parseParameterGroup();
    std::unique_ptr<CompoundStatementNode> parseCompoundStatement();
    std::unique_ptr<StatementListNode> parseStatementList();
    std::unique_ptr<StatementNode> parseStatement();
    std::unique_ptr<VariableNode> parseVariable();
    std::unique_ptr<ComponentVariableNode> parseComponentVariable(std::unique_ptr<VariableNode> baseVariable);
    std::unique_ptr<IndexListNode> parseIndexList();
    std::unique_ptr<AssignmentStatementNode> parseAssignmentStatement();
    std::unique_ptr<IfStatementNode> parseIfStatement();
    std::unique_ptr<CaseStatementNode> parseCaseStatement();
    std::unique_ptr<CaseBlockNode> parseCaseBlock();
    std::unique_ptr<WhileStatementNode> parseWhileStatement();
    std::unique_ptr<RepeatStatementNode> parseRepeatStatement();
    std::unique_ptr<ForStatementNode> parseForStatement();
    std::unique_ptr<ProcedureFunctionCallNode> parseProcedureFunctionCall();
    std::unique_ptr<ParameterListNode> parseParameterList();
    std::unique_ptr<ExpressionNode> parseExpression();
    std::unique_ptr<SimpleExpressionNode> parseSimpleExpression();
    std::unique_ptr<TermNode> parseTerm();
    std::unique_ptr<FactorNode> parseFactor();
    std::unique_ptr<RelationalOperatorNode> parseRelationalOperator();
    std::unique_ptr<AdditiveOperatorNode> parseAdditiveOperator();
    std::unique_ptr<MultiplicativeOperatorNode> parseMultiplicativeOperator();
};

#endif
