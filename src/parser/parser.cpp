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

bool Parser::check(TokenType type, std::size_t offset) const
{
    const Token *token = peek(offset);
    return token != nullptr && token->type == type;
}

const Token &Parser::peek() const
{
    return parserTokens[currentIndex];
}

const Token *Parser::peek(std::size_t offset) const
{
    const std::size_t targetIndex = currentIndex + offset;
    if (targetIndex >= parserTokens.size())
    {
        return nullptr;
    }
    return &parserTokens[targetIndex];
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
    auto declarationPartNode = std::make_unique<DeclarationPartNode>();

    while (check(TokenType::CONSTSY))
    {
        declarationPartNode->addChild(parseConstDeclaration());
    }

    while (check(TokenType::TYPESY))
    {
        declarationPartNode->addChild(parseTypeDeclaration());
    }

    while (check(TokenType::VARSY))
    {
        declarationPartNode->addChild(parseVarDeclaration());
    }

    return declarationPartNode;
}

std::unique_ptr<ConstDeclarationNode> Parser::parseConstDeclaration()
{
    auto constDeclarationNode = std::make_unique<ConstDeclarationNode>();
    constDeclarationNode->addChild(makeTerminalNode(consume(TokenType::CONSTSY, "constsy")));

    do
    {
        constDeclarationNode->addChild(makeTerminalNode(consume(TokenType::IDENT, "ident")));
        constDeclarationNode->addChild(makeTerminalNode(consume(TokenType::EQL, "eql")));
        constDeclarationNode->addChild(parseConstant());
        constDeclarationNode->addChild(makeTerminalNode(consume(TokenType::SEMICOLON, "semicolon")));
    } while (check(TokenType::IDENT));

    return constDeclarationNode;
}

std::unique_ptr<ConstantNode> Parser::parseConstant()
{
    auto constantNode = std::make_unique<ConstantNode>();

    if (check(TokenType::CHARCON) || check(TokenType::STRING))
    {
        constantNode->addChild(makeTerminalNode(advance()));
        return constantNode;
    }

    if (check(TokenType::PLUS) || check(TokenType::MINUS))
    {
        constantNode->addChild(makeTerminalNode(advance()));
    }

    if (check(TokenType::IDENT) || check(TokenType::INTCON) || check(TokenType::REALCON))
    {
        constantNode->addChild(makeTerminalNode(advance()));
        return constantNode;
    }

    throwSyntaxError("constant");
}

std::unique_ptr<TypeDeclarationNode> Parser::parseTypeDeclaration()
{
    auto typeDeclarationNode = std::make_unique<TypeDeclarationNode>();
    typeDeclarationNode->addChild(makeTerminalNode(consume(TokenType::TYPESY, "typesy")));

    do
    {
        typeDeclarationNode->addChild(makeTerminalNode(consume(TokenType::IDENT, "ident")));
        typeDeclarationNode->addChild(makeTerminalNode(consume(TokenType::EQL, "eql")));
        typeDeclarationNode->addChild(parseType());
        typeDeclarationNode->addChild(makeTerminalNode(consume(TokenType::SEMICOLON, "semicolon")));
    } while (check(TokenType::IDENT));

    return typeDeclarationNode;
}

std::unique_ptr<VarDeclarationNode> Parser::parseVarDeclaration()
{
    auto varDeclarationNode = std::make_unique<VarDeclarationNode>();
    varDeclarationNode->addChild(makeTerminalNode(consume(TokenType::VARSY, "varsy")));

    do
    {
        varDeclarationNode->addChild(parseIdentifierList());
        varDeclarationNode->addChild(makeTerminalNode(consume(TokenType::COLON, "colon")));
        varDeclarationNode->addChild(parseType());
        varDeclarationNode->addChild(makeTerminalNode(consume(TokenType::SEMICOLON, "semicolon")));
    } while (check(TokenType::IDENT));

    return varDeclarationNode;
}

std::unique_ptr<IdentifierListNode> Parser::parseIdentifierList()
{
    auto identifierListNode = std::make_unique<IdentifierListNode>();
    identifierListNode->addChild(makeTerminalNode(consume(TokenType::IDENT, "ident")));

    while (check(TokenType::COMMA))
    {
        identifierListNode->addChild(makeTerminalNode(advance()));
        identifierListNode->addChild(makeTerminalNode(consume(TokenType::IDENT, "ident")));
    }

    return identifierListNode;
}

std::unique_ptr<TypeNode> Parser::parseType()
{
    auto typeNode = std::make_unique<TypeNode>();

    if (check(TokenType::ARRAYSY))
    {
        typeNode->addChild(parseArrayType());
        return typeNode;
    }

    if (check(TokenType::LPARENT))
    {
        typeNode->addChild(parseEnumerated());
        return typeNode;
    }

    if (check(TokenType::RECORDSY))
    {
        typeNode->addChild(parseRecordType());
        return typeNode;
    }

    if (isRangeStart())
    {
        typeNode->addChild(parseRange());
        return typeNode;
    }

    if (check(TokenType::IDENT))
    {
        typeNode->addChild(makeTerminalNode(advance()));
        return typeNode;
    }

    throwSyntaxError("type");
}

std::unique_ptr<ArrayTypeNode> Parser::parseArrayType()
{
    auto arrayTypeNode = std::make_unique<ArrayTypeNode>();
    arrayTypeNode->addChild(makeTerminalNode(consume(TokenType::ARRAYSY, "arraysy")));
    arrayTypeNode->addChild(makeTerminalNode(consume(TokenType::LBRACK, "lbrack")));

    if (isRangeStart())
    {
        arrayTypeNode->addChild(parseRange());
    }
    else
    {
        arrayTypeNode->addChild(makeTerminalNode(consume(TokenType::IDENT, "ident")));
    }

    arrayTypeNode->addChild(makeTerminalNode(consume(TokenType::RBRACK, "rbrack")));
    arrayTypeNode->addChild(makeTerminalNode(consume(TokenType::OFSY, "ofsy")));
    arrayTypeNode->addChild(parseType());
    return arrayTypeNode;
}

std::unique_ptr<RangeNode> Parser::parseRange()
{
    auto rangeNode = std::make_unique<RangeNode>();
    rangeNode->addChild(parseConstant());
    rangeNode->addChild(makeTerminalNode(consume(TokenType::PERIOD, "period")));
    rangeNode->addChild(makeTerminalNode(consume(TokenType::PERIOD, "period")));
    rangeNode->addChild(parseConstant());
    return rangeNode;
}

std::unique_ptr<EnumeratedNode> Parser::parseEnumerated()
{
    auto enumeratedNode = std::make_unique<EnumeratedNode>();
    enumeratedNode->addChild(makeTerminalNode(consume(TokenType::LPARENT, "lparent")));
    enumeratedNode->addChild(makeTerminalNode(consume(TokenType::IDENT, "ident")));

    while (check(TokenType::COMMA))
    {
        enumeratedNode->addChild(makeTerminalNode(advance()));
        enumeratedNode->addChild(makeTerminalNode(consume(TokenType::IDENT, "ident")));
    }

    enumeratedNode->addChild(makeTerminalNode(consume(TokenType::RPARENT, "rparent")));
    return enumeratedNode;
}

std::unique_ptr<RecordTypeNode> Parser::parseRecordType()
{
    auto recordTypeNode = std::make_unique<RecordTypeNode>();
    recordTypeNode->addChild(makeTerminalNode(consume(TokenType::RECORDSY, "recordsy")));
    recordTypeNode->addChild(parseFieldList());
    recordTypeNode->addChild(makeTerminalNode(consume(TokenType::ENDSY, "endsy")));
    return recordTypeNode;
}

std::unique_ptr<FieldListNode> Parser::parseFieldList()
{
    auto fieldListNode = std::make_unique<FieldListNode>();
    fieldListNode->addChild(parseFieldPart());

    while (check(TokenType::SEMICOLON))
    {
        fieldListNode->addChild(makeTerminalNode(advance()));
        if (check(TokenType::ENDSY))
        {
            break;
        }
        fieldListNode->addChild(parseFieldPart());
    }

    return fieldListNode;
}

std::unique_ptr<FieldPartNode> Parser::parseFieldPart()
{
    auto fieldPartNode = std::make_unique<FieldPartNode>();
    fieldPartNode->addChild(parseIdentifierList());
    fieldPartNode->addChild(makeTerminalNode(consume(TokenType::COLON, "colon")));
    fieldPartNode->addChild(parseType());
    return fieldPartNode;
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
