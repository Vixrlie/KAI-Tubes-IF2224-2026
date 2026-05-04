#include "parser.h"

#include <sstream>

// This file implements the recursive descent rules for the Arion grammar.
namespace
{
    // This tells the parser to ignore comment tokens that survived the lexer stage.
    bool shouldSkipToken(const Token &token)
    {
        return token.type == TokenType::COMMENT;
    }

    std::string describeToken(const Token &token)
    {
        return tokenHasValue(token.type) ? formatToken(token) : tokenTypeToString(token.type);
    }
}

// This stores the parser failure message together with its source location.
ParserError::ParserError(std::string message, int line, int column)
    : std::runtime_error(std::move(message)), errorLine(line), errorColumn(column) {}

// This returns the source line where the parser failed.
int ParserError::line() const
{
    return errorLine;
}

// This returns the source column where the parser failed.
int ParserError::column() const
{
    return errorColumn;
}

// This constructor filters out comments and rejects lexical error tokens up front.
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

// This parses a complete Arion program from header to final period.
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

// This parses the fixed 'program ident ;' header at the start of the file.
std::unique_ptr<ProgramHeaderNode> Parser::parseProgramHeader()
{
    auto programHeaderNode = std::make_unique<ProgramHeaderNode>();
    programHeaderNode->addChild(makeTerminalNode(consume(TokenType::PROGRAMSY, "programsy")));
    programHeaderNode->addChild(makeTerminalNode(consume(TokenType::IDENT, "ident")));
    programHeaderNode->addChild(makeTerminalNode(consume(TokenType::SEMICOLON, "semicolon")));
    return programHeaderNode;
}

// This checks whether the parser has already consumed every token.
bool Parser::isAtEnd() const
{
    return currentIndex >= parserTokens.size();
}

// This checks whether the current token matches the expected type.
bool Parser::check(TokenType type) const
{
    return !isAtEnd() && peek().type == type;
}

// This checks one lookahead position without advancing the parser cursor.
bool Parser::check(TokenType type, std::size_t offset) const
{
    const Token *token = peek(offset);
    return token != nullptr && token->type == type;
}

// This returns the current token and assumes the parser is not at the end.
const Token &Parser::peek() const
{
    return parserTokens[currentIndex];
}

// This returns one lookahead token when it exists or null when it does not.
const Token *Parser::peek(std::size_t offset) const
{
    const std::size_t targetIndex = currentIndex + offset;
    if (targetIndex >= parserTokens.size())
    {
        return nullptr;
    }
    return &parserTokens[targetIndex];
}

// This consumes the current token and moves the parser cursor forward.
const Token &Parser::advance()
{
    return parserTokens[currentIndex++];
}

// This consumes one expected token or throws a syntax error immediately.
Token Parser::consume(TokenType type, const std::string &expectedName)
{
    if (!check(type))
    {
        throwSyntaxError(expectedName);
    }

    return advance();
}

// This wraps a consumed token so it can live as a terminal parse tree node.
std::unique_ptr<ParseTreeTerminalNode> Parser::makeTerminalNode(const Token &token) const
{
    return std::make_unique<ParseTreeTerminalNode>(token);
}

// This centralizes parser error text so every rule reports failures consistently.
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

// This checks whether the current token sequence can start a range production.
bool Parser::isRangeStart() const
{
    if (check(TokenType::PLUS) || check(TokenType::MINUS) ||
        check(TokenType::INTCON) || check(TokenType::REALCON) ||
        check(TokenType::CHARCON) || check(TokenType::STRING))
    {
        return true;
    }

    return check(TokenType::IDENT) &&
           check(TokenType::PERIOD, 1) &&
           check(TokenType::PERIOD, 2);
}

// This checks whether the current token can legally end an optional statement slot.
bool Parser::isStatementFollow() const
{
    return check(TokenType::SEMICOLON) ||
           check(TokenType::ENDSY) ||
           check(TokenType::ELSESY) ||
           check(TokenType::UNTILSY);
}

// This checks whether a variable continues with indexing or field access.
bool Parser::isVariableSuffixStart() const
{
    return check(TokenType::LBRACK) || check(TokenType::PERIOD);
}

// This checks whether the current token is valid inside an array index list.
bool Parser::isIndexToken() const
{
    return check(TokenType::INTCON) ||
           check(TokenType::CHARCON) ||
           check(TokenType::IDENT);
}

// This keeps wrapping a base variable while more component suffixes are present.
std::unique_ptr<VariableNode> Parser::parseVariableWithBase(std::unique_ptr<VariableNode> baseVariable)
{
    if (!isVariableSuffixStart())
    {
        return baseVariable;
    }

    auto wrappedVariableNode = std::make_unique<VariableNode>();
    wrappedVariableNode->addChild(parseComponentVariable(std::move(baseVariable)));
    return parseVariableWithBase(std::move(wrappedVariableNode));
}

// This parses the declaration section that sits between the header and executable block.
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

    while (check(TokenType::PROCEDURESY) || check(TokenType::FUNCTIONSY))
    {
        declarationPartNode->addChild(parseSubprogramDeclaration());
    }

    return declarationPartNode;
}

// This parses one or more constant declarations after the const keyword.
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

// This parses a constant literal or signed identifier allowed by the grammar.
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

// This parses one or more user-defined type declarations after the type keyword.
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

// This parses one or more variable declarations after the var keyword.
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

// This parses a comma-separated list of identifiers.
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

// This dispatches to the correct type production based on the current token.
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

// This parses an array type including its index domain and element type.
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

// This parses a subrange written as two constants separated by two periods.
std::unique_ptr<RangeNode> Parser::parseRange()
{
    auto rangeNode = std::make_unique<RangeNode>();
    rangeNode->addChild(parseConstant());
    rangeNode->addChild(makeTerminalNode(consume(TokenType::PERIOD, "period")));
    rangeNode->addChild(makeTerminalNode(consume(TokenType::PERIOD, "period")));
    rangeNode->addChild(parseConstant());
    return rangeNode;
}

// This parses an enumerated type written inside parentheses.
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

// This parses a record type and its internal field list.
std::unique_ptr<RecordTypeNode> Parser::parseRecordType()
{
    auto recordTypeNode = std::make_unique<RecordTypeNode>();
    recordTypeNode->addChild(makeTerminalNode(consume(TokenType::RECORDSY, "recordsy")));
    recordTypeNode->addChild(parseFieldList());
    recordTypeNode->addChild(makeTerminalNode(consume(TokenType::ENDSY, "endsy")));
    return recordTypeNode;
}

// This parses the semicolon-separated field declarations inside a record.
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

// This parses one record field declaration of the form identifiers ':' type.
std::unique_ptr<FieldPartNode> Parser::parseFieldPart()
{
    auto fieldPartNode = std::make_unique<FieldPartNode>();
    fieldPartNode->addChild(parseIdentifierList());
    fieldPartNode->addChild(makeTerminalNode(consume(TokenType::COLON, "colon")));
    fieldPartNode->addChild(parseType());
    return fieldPartNode;
}

// This chooses between a procedure and function declaration inside the declaration part.
std::unique_ptr<SubprogramDeclarationNode> Parser::parseSubprogramDeclaration()
{
    auto subprogramDeclarationNode = std::make_unique<SubprogramDeclarationNode>();

    if (check(TokenType::PROCEDURESY))
    {
        subprogramDeclarationNode->addChild(parseProcedureDeclaration());
        return subprogramDeclarationNode;
    }

    if (check(TokenType::FUNCTIONSY))
    {
        subprogramDeclarationNode->addChild(parseFunctionDeclaration());
        return subprogramDeclarationNode;
    }

    throwSyntaxError("proceduresy or functionsy");
}

// This parses a full procedure declaration including parameters and body.
std::unique_ptr<ProcedureDeclarationNode> Parser::parseProcedureDeclaration()
{
    auto procedureDeclarationNode = std::make_unique<ProcedureDeclarationNode>();
    procedureDeclarationNode->addChild(makeTerminalNode(consume(TokenType::PROCEDURESY, "proceduresy")));
    procedureDeclarationNode->addChild(makeTerminalNode(consume(TokenType::IDENT, "ident")));

    if (check(TokenType::LPARENT))
    {
        procedureDeclarationNode->addChild(parseFormalParameterList());
    }

    procedureDeclarationNode->addChild(makeTerminalNode(consume(TokenType::SEMICOLON, "semicolon")));
    procedureDeclarationNode->addChild(parseBlock());
    procedureDeclarationNode->addChild(makeTerminalNode(consume(TokenType::SEMICOLON, "semicolon")));
    return procedureDeclarationNode;
}

// This parses a full function declaration including its return type and body.
std::unique_ptr<FunctionDeclarationNode> Parser::parseFunctionDeclaration()
{
    auto functionDeclarationNode = std::make_unique<FunctionDeclarationNode>();
    functionDeclarationNode->addChild(makeTerminalNode(consume(TokenType::FUNCTIONSY, "functionsy")));
    functionDeclarationNode->addChild(makeTerminalNode(consume(TokenType::IDENT, "ident")));

    if (check(TokenType::LPARENT))
    {
        functionDeclarationNode->addChild(parseFormalParameterList());
    }

    functionDeclarationNode->addChild(makeTerminalNode(consume(TokenType::COLON, "colon")));
    functionDeclarationNode->addChild(makeTerminalNode(consume(TokenType::IDENT, "ident")));
    functionDeclarationNode->addChild(makeTerminalNode(consume(TokenType::SEMICOLON, "semicolon")));
    functionDeclarationNode->addChild(parseBlock());
    functionDeclarationNode->addChild(makeTerminalNode(consume(TokenType::SEMICOLON, "semicolon")));
    return functionDeclarationNode;
}

// This parses the nested declaration part followed by the compound statement body.
std::unique_ptr<BlockNode> Parser::parseBlock()
{
    auto blockNode = std::make_unique<BlockNode>();
    blockNode->addChild(parseDeclarationPart());
    blockNode->addChild(parseCompoundStatement());
    return blockNode;
}

// This parses the parenthesized parameter groups on a procedure or function header.
std::unique_ptr<FormalParameterListNode> Parser::parseFormalParameterList()
{
    auto formalParameterListNode = std::make_unique<FormalParameterListNode>();
    formalParameterListNode->addChild(makeTerminalNode(consume(TokenType::LPARENT, "lparent")));
    formalParameterListNode->addChild(parseParameterGroup());

    while (check(TokenType::SEMICOLON))
    {
        formalParameterListNode->addChild(makeTerminalNode(advance()));
        formalParameterListNode->addChild(parseParameterGroup());
    }

    formalParameterListNode->addChild(makeTerminalNode(consume(TokenType::RPARENT, "rparent")));
    return formalParameterListNode;
}

// This parses one parameter group with shared type information.
std::unique_ptr<ParameterGroupNode> Parser::parseParameterGroup()
{
    auto parameterGroupNode = std::make_unique<ParameterGroupNode>();
    parameterGroupNode->addChild(parseIdentifierList());
    parameterGroupNode->addChild(makeTerminalNode(consume(TokenType::COLON, "colon")));

    if (check(TokenType::ARRAYSY))
    {
        parameterGroupNode->addChild(parseArrayType());
        return parameterGroupNode;
    }

    parameterGroupNode->addChild(makeTerminalNode(consume(TokenType::IDENT, "ident")));
    return parameterGroupNode;
}

// This parses a begin-end block that wraps a statement list.
std::unique_ptr<CompoundStatementNode> Parser::parseCompoundStatement()
{
    auto compoundStatementNode = std::make_unique<CompoundStatementNode>();
    compoundStatementNode->addChild(makeTerminalNode(consume(TokenType::BEGINSY, "beginsy")));
    compoundStatementNode->addChild(parseStatementList());
    compoundStatementNode->addChild(makeTerminalNode(consume(TokenType::ENDSY, "endsy")));
    return compoundStatementNode;
}

// This parses semicolon-separated statements until the surrounding construct closes.
std::unique_ptr<StatementListNode> Parser::parseStatementList()
{
    auto statementListNode = std::make_unique<StatementListNode>();
    statementListNode->addChild(parseStatement());

    while (check(TokenType::SEMICOLON))
    {
        statementListNode->addChild(makeTerminalNode(advance()));
        if (check(TokenType::ENDSY) || check(TokenType::UNTILSY))
        {
            break;
        }
        statementListNode->addChild(parseStatement());
    }

    return statementListNode;
}

// This dispatches to the concrete statement form that matches the current token.
std::unique_ptr<StatementNode> Parser::parseStatement()
{
    auto statementNode = std::make_unique<StatementNode>();

    if (check(TokenType::IDENT))
    {
        if (check(TokenType::BECOMES, 1) || check(TokenType::LBRACK, 1) || check(TokenType::PERIOD, 1))
        {
            statementNode->addChild(parseAssignmentStatement());
        }
        else
        {
            statementNode->addChild(parseProcedureFunctionCall());
        }
        return statementNode;
    }

    if (check(TokenType::BEGINSY))
    {
        statementNode->addChild(parseCompoundStatement());
        return statementNode;
    }

    if (check(TokenType::IFSY))
    {
        statementNode->addChild(parseIfStatement());
        return statementNode;
    }

    if (check(TokenType::CASESY))
    {
        statementNode->addChild(parseCaseStatement());
        return statementNode;
    }

    if (check(TokenType::WHILESY))
    {
        statementNode->addChild(parseWhileStatement());
        return statementNode;
    }

    if (check(TokenType::REPEATSY))
    {
        statementNode->addChild(parseRepeatStatement());
        return statementNode;
    }

    if (check(TokenType::FORSY))
    {
        statementNode->addChild(parseForStatement());
        return statementNode;
    }

    if (isStatementFollow())
    {
        statementNode->addChild(std::make_unique<EmptyStatementNode>());
        return statementNode;
    }

    throwSyntaxError("statement");
}

// This parses a variable and then folds any chained component or index access.
std::unique_ptr<VariableNode> Parser::parseVariable()
{
    auto variableNode = std::make_unique<VariableNode>();
    variableNode->addChild(makeTerminalNode(consume(TokenType::IDENT, "ident")));
    return parseVariableWithBase(std::move(variableNode));
}

// This parses one array index or field access step on top of an existing variable.
std::unique_ptr<ComponentVariableNode> Parser::parseComponentVariable(std::unique_ptr<VariableNode> baseVariable)
{
    auto componentVariableNode = std::make_unique<ComponentVariableNode>();
    componentVariableNode->addChild(std::move(baseVariable));

    if (check(TokenType::LBRACK))
    {
        componentVariableNode->addChild(makeTerminalNode(advance()));
        componentVariableNode->addChild(parseIndexList());
        componentVariableNode->addChild(makeTerminalNode(consume(TokenType::RBRACK, "rbrack")));
        return componentVariableNode;
    }

    componentVariableNode->addChild(makeTerminalNode(consume(TokenType::PERIOD, "period")));
    componentVariableNode->addChild(makeTerminalNode(consume(TokenType::IDENT, "ident")));
    return componentVariableNode;
}

// This parses the simple index list used by milestone 2 array access.
std::unique_ptr<IndexListNode> Parser::parseIndexList()
{
    auto indexListNode = std::make_unique<IndexListNode>();

    if (!isIndexToken())
    {
        throwSyntaxError("intcon, charcon, or ident");
    }

    indexListNode->addChild(makeTerminalNode(advance()));
    while (check(TokenType::COMMA))
    {
        indexListNode->addChild(makeTerminalNode(advance()));
        if (!isIndexToken())
        {
            throwSyntaxError("intcon, charcon, or ident");
        }
        indexListNode->addChild(makeTerminalNode(advance()));
    }

    return indexListNode;
}

// This parses a variable followed by an assignment expression.
std::unique_ptr<AssignmentStatementNode> Parser::parseAssignmentStatement()
{
    auto assignmentStatementNode = std::make_unique<AssignmentStatementNode>();
    assignmentStatementNode->addChild(parseVariable());
    assignmentStatementNode->addChild(makeTerminalNode(consume(TokenType::BECOMES, "becomes")));
    assignmentStatementNode->addChild(parseExpression());
    return assignmentStatementNode;
}

// This parses an if statement with an optional else branch.
std::unique_ptr<IfStatementNode> Parser::parseIfStatement()
{
    auto ifStatementNode = std::make_unique<IfStatementNode>();
    ifStatementNode->addChild(makeTerminalNode(consume(TokenType::IFSY, "ifsy")));
    ifStatementNode->addChild(parseExpression());
    ifStatementNode->addChild(makeTerminalNode(consume(TokenType::THENSY, "thensy")));
    ifStatementNode->addChild(parseStatement());

    if (check(TokenType::ELSESY))
    {
        ifStatementNode->addChild(makeTerminalNode(advance()));
        ifStatementNode->addChild(parseStatement());
    }

    return ifStatementNode;
}

// This parses a case statement and its nested blocks.
std::unique_ptr<CaseStatementNode> Parser::parseCaseStatement()
{
    auto caseStatementNode = std::make_unique<CaseStatementNode>();
    caseStatementNode->addChild(makeTerminalNode(consume(TokenType::CASESY, "casesy")));
    caseStatementNode->addChild(parseExpression());
    caseStatementNode->addChild(makeTerminalNode(consume(TokenType::OFSY, "ofsy")));
    caseStatementNode->addChild(parseCaseBlock());
    caseStatementNode->addChild(makeTerminalNode(consume(TokenType::ENDSY, "endsy")));
    return caseStatementNode;
}

// This parses one case arm and chains the next arm when a semicolon follows.
std::unique_ptr<CaseBlockNode> Parser::parseCaseBlock()
{
    auto caseBlockNode = std::make_unique<CaseBlockNode>();
    caseBlockNode->addChild(parseConstant());

    while (check(TokenType::COMMA))
    {
        caseBlockNode->addChild(makeTerminalNode(advance()));
        caseBlockNode->addChild(parseConstant());
    }

    caseBlockNode->addChild(makeTerminalNode(consume(TokenType::COLON, "colon")));
    caseBlockNode->addChild(parseStatement());

    if (check(TokenType::SEMICOLON))
    {
        caseBlockNode->addChild(makeTerminalNode(advance()));
        if (!check(TokenType::ENDSY))
        {
            caseBlockNode->addChild(parseCaseBlock());
        }
    }

    return caseBlockNode;
}

// This parses a while-do loop with a single nested statement body.
std::unique_ptr<WhileStatementNode> Parser::parseWhileStatement()
{
    auto whileStatementNode = std::make_unique<WhileStatementNode>();
    whileStatementNode->addChild(makeTerminalNode(consume(TokenType::WHILESY, "whilesy")));
    whileStatementNode->addChild(parseExpression());
    whileStatementNode->addChild(makeTerminalNode(consume(TokenType::DOSY, "dosy")));
    whileStatementNode->addChild(parseStatement());
    return whileStatementNode;
}

// This parses a repeat-until loop whose body is a statement list.
std::unique_ptr<RepeatStatementNode> Parser::parseRepeatStatement()
{
    auto repeatStatementNode = std::make_unique<RepeatStatementNode>();
    repeatStatementNode->addChild(makeTerminalNode(consume(TokenType::REPEATSY, "repeatsy")));
    repeatStatementNode->addChild(parseStatementList());
    repeatStatementNode->addChild(makeTerminalNode(consume(TokenType::UNTILSY, "untilsy")));
    repeatStatementNode->addChild(parseExpression());
    return repeatStatementNode;
}

// This parses a for loop with either to or downto as its direction.
std::unique_ptr<ForStatementNode> Parser::parseForStatement()
{
    auto forStatementNode = std::make_unique<ForStatementNode>();
    forStatementNode->addChild(makeTerminalNode(consume(TokenType::FORSY, "forsy")));
    forStatementNode->addChild(makeTerminalNode(consume(TokenType::IDENT, "ident")));
    forStatementNode->addChild(makeTerminalNode(consume(TokenType::BECOMES, "becomes")));
    forStatementNode->addChild(parseExpression());

    if (check(TokenType::TOSY) || check(TokenType::DOWNTOSY))
    {
        forStatementNode->addChild(makeTerminalNode(advance()));
    }
    else
    {
        throwSyntaxError("tosy or downtosy");
    }

    forStatementNode->addChild(parseExpression());
    forStatementNode->addChild(makeTerminalNode(consume(TokenType::DOSY, "dosy")));
    forStatementNode->addChild(parseStatement());
    return forStatementNode;
}

// This parses a procedure or function call and enforces a non-empty argument list when parentheses exist.
std::unique_ptr<ProcedureFunctionCallNode> Parser::parseProcedureFunctionCall()
{
    auto procedureFunctionCallNode = std::make_unique<ProcedureFunctionCallNode>();
    procedureFunctionCallNode->addChild(makeTerminalNode(consume(TokenType::IDENT, "ident")));

    if (check(TokenType::LPARENT))
    {
        procedureFunctionCallNode->addChild(makeTerminalNode(advance()));
        if (check(TokenType::RPARENT))
        {
            throwSyntaxError("parameter-list");
        }
        procedureFunctionCallNode->addChild(parseParameterList());
        procedureFunctionCallNode->addChild(makeTerminalNode(consume(TokenType::RPARENT, "rparent")));
    }

    return procedureFunctionCallNode;
}

// This parses a comma-separated list of call arguments.
std::unique_ptr<ParameterListNode> Parser::parseParameterList()
{
    auto parameterListNode = std::make_unique<ParameterListNode>();
    parameterListNode->addChild(parseExpression());

    while (check(TokenType::COMMA))
    {
        parameterListNode->addChild(makeTerminalNode(advance()));
        parameterListNode->addChild(parseExpression());
    }

    return parameterListNode;
}

// This parses an optional relational comparison between two simple expressions.
std::unique_ptr<ExpressionNode> Parser::parseExpression()
{
    auto expressionNode = std::make_unique<ExpressionNode>();
    expressionNode->addChild(parseSimpleExpression());

    if (check(TokenType::EQL) || check(TokenType::NEQ) ||
        check(TokenType::GTR) || check(TokenType::GEQ) ||
        check(TokenType::LSS) || check(TokenType::LEQ))
    {
        expressionNode->addChild(parseRelationalOperator());
        expressionNode->addChild(parseSimpleExpression());
    }

    return expressionNode;
}

// This parses leading sign handling and additive chaining for an expression.
std::unique_ptr<SimpleExpressionNode> Parser::parseSimpleExpression()
{
    auto simpleExpressionNode = std::make_unique<SimpleExpressionNode>();

    if (check(TokenType::PLUS) || check(TokenType::MINUS))
    {
        simpleExpressionNode->addChild(makeTerminalNode(advance()));
    }

    simpleExpressionNode->addChild(parseTerm());
    while (check(TokenType::PLUS) || check(TokenType::MINUS) || check(TokenType::ORSY))
    {
        simpleExpressionNode->addChild(parseAdditiveOperator());
        simpleExpressionNode->addChild(parseTerm());
    }

    return simpleExpressionNode;
}

// This parses multiplicative chains to preserve the grammar's operator precedence.
std::unique_ptr<TermNode> Parser::parseTerm()
{
    auto termNode = std::make_unique<TermNode>();
    termNode->addChild(parseFactor());

    while (check(TokenType::TIMES) || check(TokenType::RDIV) ||
           check(TokenType::IDIV) || check(TokenType::IMOD) ||
           check(TokenType::ANDSY))
    {
        termNode->addChild(parseMultiplicativeOperator());
        termNode->addChild(parseFactor());
    }

    return termNode;
}

// This parses the smallest expression unit, including literals, variables, calls, and grouping.
std::unique_ptr<FactorNode> Parser::parseFactor()
{
    auto factorNode = std::make_unique<FactorNode>();

    if (check(TokenType::IDENT))
    {
        if (check(TokenType::LPARENT, 1))
        {
            factorNode->addChild(parseProcedureFunctionCall());
        }
        else if (check(TokenType::LBRACK, 1) || check(TokenType::PERIOD, 1))
        {
            factorNode->addChild(parseVariable());
        }
        else
        {
            factorNode->addChild(makeTerminalNode(advance()));
        }
        return factorNode;
    }

    if (check(TokenType::INTCON) || check(TokenType::REALCON) ||
        check(TokenType::CHARCON) || check(TokenType::STRING))
    {
        factorNode->addChild(makeTerminalNode(advance()));
        return factorNode;
    }

    if (check(TokenType::LPARENT))
    {
        factorNode->addChild(makeTerminalNode(advance()));
        factorNode->addChild(parseExpression());
        factorNode->addChild(makeTerminalNode(consume(TokenType::RPARENT, "rparent")));
        return factorNode;
    }

    if (check(TokenType::NOTSY))
    {
        factorNode->addChild(makeTerminalNode(advance()));
        factorNode->addChild(parseFactor());
        return factorNode;
    }

    throwSyntaxError("factor");
}

// This wraps a relational token inside the dedicated parse tree node.
std::unique_ptr<RelationalOperatorNode> Parser::parseRelationalOperator()
{
    auto relationalOperatorNode = std::make_unique<RelationalOperatorNode>();

    if (check(TokenType::EQL) || check(TokenType::NEQ) ||
        check(TokenType::GTR) || check(TokenType::GEQ) ||
        check(TokenType::LSS) || check(TokenType::LEQ))
    {
        relationalOperatorNode->addChild(makeTerminalNode(advance()));
        return relationalOperatorNode;
    }

    throwSyntaxError("relational operator");
}

// This wraps an additive token inside the dedicated parse tree node.
std::unique_ptr<AdditiveOperatorNode> Parser::parseAdditiveOperator()
{
    auto additiveOperatorNode = std::make_unique<AdditiveOperatorNode>();

    if (check(TokenType::PLUS) || check(TokenType::MINUS) || check(TokenType::ORSY))
    {
        additiveOperatorNode->addChild(makeTerminalNode(advance()));
        return additiveOperatorNode;
    }

    throwSyntaxError("additive operator");
}

// This wraps a multiplicative token inside the dedicated parse tree node.
std::unique_ptr<MultiplicativeOperatorNode> Parser::parseMultiplicativeOperator()
{
    auto multiplicativeOperatorNode = std::make_unique<MultiplicativeOperatorNode>();

    if (check(TokenType::TIMES) || check(TokenType::RDIV) ||
        check(TokenType::IDIV) || check(TokenType::IMOD) ||
        check(TokenType::ANDSY))
    {
        multiplicativeOperatorNode->addChild(makeTerminalNode(advance()));
        return multiplicativeOperatorNode;
    }

    throwSyntaxError("multiplicative operator");
}
