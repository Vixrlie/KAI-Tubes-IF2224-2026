#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType {
    INTCON,
    REALCON,
    CHARCON,
    STRING,

    NOTSY,
    ANDSY,
    ORSY,

    PLUS,
    MINUS,
    TIMES,
    IDIV,
    RDIV,
    IMOD,

    EQL,
    NEQ,
    GTR,
    GEQ,
    LSS,
    LEQ,

    LPARENT,
    RPARENT,
    LBRACK,
    RBRACK,
    COMMA,
    SEMICOLON,
    PERIOD,
    COLON,
    BECOMES,

    CONSTSY,
    TYPESY,
    VARSY,
    FUNCTIONSY,
    PROCEDURESY,
    ARRAYSY,
    RECORDSY,
    PROGRAMSY,

    IDENT,

    BEGINSY,
    IFSY,
    CASESY,
    REPEATSY,
    WHILESY,
    FORSY,
    ENDSY,
    ELSESY,
    UNTILSY,
    OFSY,
    DOSY,
    TOSY,
    DOWNTOSY,
    THENSY,

    COMMENT,
    UNKNOWN,
    ERROR
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};

std::string tokenTypeToString(TokenType type);
bool tokenHasValue(TokenType type);
std::string formatToken(const Token& token);

#endif
