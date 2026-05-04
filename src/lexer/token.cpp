#include "token.h"

// This converts a token type enum into the lowercase label used in output.
std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::INTCON:       return "intcon";
        case TokenType::REALCON:      return "realcon";
        case TokenType::CHARCON:      return "charcon";
        case TokenType::STRING:       return "string";
        case TokenType::NOTSY:        return "notsy";
        case TokenType::ANDSY:        return "andsy";
        case TokenType::ORSY:         return "orsy";
        case TokenType::PLUS:         return "plus";
        case TokenType::MINUS:        return "minus";
        case TokenType::TIMES:        return "times";
        case TokenType::IDIV:         return "idiv";
        case TokenType::RDIV:         return "rdiv";
        case TokenType::IMOD:         return "imod";
        case TokenType::EQL:          return "eql";
        case TokenType::NEQ:          return "neq";
        case TokenType::GTR:          return "gtr";
        case TokenType::GEQ:          return "geq";
        case TokenType::LSS:          return "lss";
        case TokenType::LEQ:          return "leq";
        case TokenType::LPARENT:      return "lparent";
        case TokenType::RPARENT:      return "rparent";
        case TokenType::LBRACK:       return "lbrack";
        case TokenType::RBRACK:       return "rbrack";
        case TokenType::COMMA:        return "comma";
        case TokenType::SEMICOLON:    return "semicolon";
        case TokenType::PERIOD:       return "period";
        case TokenType::COLON:        return "colon";
        case TokenType::BECOMES:      return "becomes";
        case TokenType::CONSTSY:      return "constsy";
        case TokenType::TYPESY:       return "typesy";
        case TokenType::VARSY:        return "varsy";
        case TokenType::FUNCTIONSY:   return "functionsy";
        case TokenType::PROCEDURESY:  return "proceduresy";
        case TokenType::ARRAYSY:      return "arraysy";
        case TokenType::RECORDSY:     return "recordsy";
        case TokenType::PROGRAMSY:    return "programsy";
        case TokenType::IDENT:        return "ident";
        case TokenType::BEGINSY:      return "beginsy";
        case TokenType::IFSY:         return "ifsy";
        case TokenType::CASESY:       return "casesy";
        case TokenType::REPEATSY:     return "repeatsy";
        case TokenType::WHILESY:      return "whilesy";
        case TokenType::FORSY:        return "forsy";
        case TokenType::ENDSY:        return "endsy";
        case TokenType::ELSESY:       return "elsesy";
        case TokenType::UNTILSY:      return "untilsy";
        case TokenType::OFSY:         return "ofsy";
        case TokenType::DOSY:         return "dosy";
        case TokenType::TOSY:         return "tosy";
        case TokenType::DOWNTOSY:     return "downtosy";
        case TokenType::THENSY:       return "thensy";
        case TokenType::COMMENT:      return "comment";
        case TokenType::UNKNOWN:      return "unknown";
        case TokenType::ERROR:        return "error";
    }
    return "unknown";
}

// This marks which token kinds must print an attached value.
bool tokenHasValue(TokenType type) {
    return type == TokenType::IDENT   ||
           type == TokenType::INTCON  ||
           type == TokenType::REALCON ||
           type == TokenType::CHARCON ||
           type == TokenType::STRING  ||
           type == TokenType::COMMENT ||
           type == TokenType::UNKNOWN ||
           type == TokenType::ERROR;
}

// This formats one token exactly the way the lexer output expects it.
std::string formatToken(const Token& token) {
    if (tokenHasValue(token.type)) {
        return tokenTypeToString(token.type) + " (" + token.value + ")";
    }
    return tokenTypeToString(token.type);
}
