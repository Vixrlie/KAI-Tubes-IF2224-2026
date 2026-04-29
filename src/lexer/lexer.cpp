#include "lexer.h"
#include <cctype>
#include <algorithm>
#include <map>
#include <iostream>

static const std::map<std::string, TokenType> keywords = {
    {"program",   TokenType::PROGRAMSY},
    {"const",     TokenType::CONSTSY},
    {"type",      TokenType::TYPESY},
    {"var",       TokenType::VARSY},
    {"function",  TokenType::FUNCTIONSY},
    {"procedure", TokenType::PROCEDURESY},
    {"array",     TokenType::ARRAYSY},
    {"record",    TokenType::RECORDSY},
    {"begin",     TokenType::BEGINSY},
    {"end",       TokenType::ENDSY},
    {"if",        TokenType::IFSY},
    {"then",      TokenType::THENSY},
    {"else",      TokenType::ELSESY},
    {"case",      TokenType::CASESY},
    {"of",        TokenType::OFSY},
    {"repeat",    TokenType::REPEATSY},
    {"until",     TokenType::UNTILSY},
    {"while",     TokenType::WHILESY},
    {"do",        TokenType::DOSY},
    {"for",       TokenType::FORSY},
    {"to",        TokenType::TOSY},
    {"downto",    TokenType::DOWNTOSY},
    {"not",       TokenType::NOTSY},
    {"div",       TokenType::IDIV},
    {"mod",       TokenType::IMOD},
    {"and",       TokenType::ANDSY},
    {"or",        TokenType::ORSY}
};

Lexer::Lexer(const std::string& source)
    : source(source), pos(0), line(1), col(1),
      state(State::START), tokenStartLine(1), tokenStartCol(1),
      dotLine(0), dotCol(0), errorFlag(false) {}

char Lexer::current() const {
    if (isAtEnd()) return '\0';
    return source[pos];
}

bool Lexer::isAtEnd() const {
    return pos >= static_cast<int>(source.size());
}

bool Lexer::isSeparator(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void Lexer::advance() {
    if (!isAtEnd()) {
        if (source[pos] == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
        pos++;
    }
}

void Lexer::emitToken(TokenType type, const std::string& value) {
    Token token;
    token.type = type;
    token.value = value;
    token.line = tokenStartLine;
    token.column = tokenStartCol;
    tokens.push_back(token);
}

void Lexer::emitError(const std::string& message) {
    errorFlag = true;
    Token token;
    token.type = TokenType::ERROR;
    token.value = message;
    token.line = tokenStartLine;
    token.column = tokenStartCol;
    tokens.push_back(token);
    std::cerr << "Lexical error at line " << tokenStartLine
              << ", column " << tokenStartCol << ": " << message << std::endl;
}

TokenType Lexer::lookupKeyword(const std::string& identifier) const {
    std::string lower = identifier;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    auto it = keywords.find(lower);
    if (it != keywords.end()) {
        return it->second;
    }
    return TokenType::IDENT;
}

bool Lexer::hasErrors() const {
    return errorFlag;
}

std::vector<Token> Lexer::tokenize() {
    while (state != State::FOUND_EOF) {
        if (isAtEnd()) {
            handleEof();
            state = State::FOUND_EOF;
        } else {
            switch (state) {
                case State::START:               handleStart(); break;
                case State::IN_IDENT:            handleInIdent(); break;
                case State::IN_NUMBER:           handleInNumber(); break;
                case State::DOT_AFTER_NUM:       handleDotAfterNum(); break;
                case State::IN_REAL:             handleInReal(); break;
                case State::IN_STRING:           handleInString(); break;
                case State::SAW_COLON:           handleSawColon(); break;
                case State::SAW_LESS:            handleSawLess(); break;
                case State::SAW_GREATER:         handleSawGreater(); break;
                case State::SAW_EQUAL:           handleSawEqual(); break;
                case State::IN_CURLY_COMMENT:    handleInCurlyComment(); break;
                case State::SAW_LPAREN:          handleSawLparen(); break;
                case State::IN_PAREN_STAR_COMMENT: handleInParenStarComment(); break;
                case State::SAW_STAR_IN_PAREN_COMMENT: handleSawStarInParenComment(); break;
                case State::IN_UNKNOWN:          handleInUnknown(); break;
                case State::FOUND_EOF:           break;
            }
        }
    }

    return tokens;
}

void Lexer::handleStart() {
    char c = current();

    if (isSeparator(c)) {
        advance();
        return;
    }

    tokenStartLine = line;
    tokenStartCol = col;

    if (std::isalpha(c)) {
        buffer = c;
        state = State::IN_IDENT;
        advance();
    } else if (std::isdigit(c)) {
        buffer = c;
        state = State::IN_NUMBER;
        advance();
    } else if (c == '\'') {
        buffer = "";
        state = State::IN_STRING;
        advance();
    } else if (c == ':') {
        state = State::SAW_COLON;
        advance();
    } else if (c == '<') {
        state = State::SAW_LESS;
        advance();
    } else if (c == '>') {
        state = State::SAW_GREATER;
        advance();
    } else if (c == '=') {
        state = State::SAW_EQUAL;
        advance();
    } else if (c == '{') {
        buffer = "{";
        state = State::IN_CURLY_COMMENT;
        advance();
    } else if (c == '(') {
        state = State::SAW_LPAREN;
        advance();
    } else if (c == '+') {
        emitToken(TokenType::PLUS);
        advance();
    } else if (c == '-') {
        emitToken(TokenType::MINUS);
        advance();
    } else if (c == '*') {
        emitToken(TokenType::TIMES);
        advance();
    } else if (c == '/') {
        emitToken(TokenType::RDIV);
        advance();
    } else if (c == ')') {
        emitToken(TokenType::RPARENT);
        advance();
    } else if (c == '[') {
        emitToken(TokenType::LBRACK);
        advance();
    } else if (c == ']') {
        emitToken(TokenType::RBRACK);
        advance();
    } else if (c == ',') {
        emitToken(TokenType::COMMA);
        advance();
    } else if (c == ';') {
        emitToken(TokenType::SEMICOLON);
        advance();
    } else if (c == '.') {
        emitToken(TokenType::PERIOD);
        advance();
    } else {
        // Per revision: collect non-separator chars into one unknown token.
        buffer = std::string(1, c);
        state = State::IN_UNKNOWN;
        advance();
    }
}

void Lexer::handleInIdent() {
    char c = current();

    if (std::isalnum(c)) {
        buffer += c;
        advance();
    } else {
        TokenType type = lookupKeyword(buffer);
        if (type == TokenType::IDENT) {
            emitToken(TokenType::IDENT, buffer);
        } else {
            emitToken(type);
        }
        state = State::START;
    }
}

void Lexer::handleInNumber() {
    char c = current();

    if (std::isdigit(c)) {
        buffer += c;
        advance();
    } else if (c == '.') {
        // could be the decimal point of a real, or a standalone period — defer.
        dotLine = line;
        dotCol = col;
        state = State::DOT_AFTER_NUM;
        advance();
    } else {
        emitToken(TokenType::INTCON, buffer);
        state = State::START;
    }
}

void Lexer::handleDotAfterNum() {
    char c = current();

    if (std::isdigit(c)) {
        buffer += '.';
        buffer += c;
        state = State::IN_REAL;
        advance();
    } else {
        emitToken(TokenType::INTCON, buffer);

        int savedLine = tokenStartLine;
        int savedCol = tokenStartCol;
        tokenStartLine = dotLine;
        tokenStartCol = dotCol;
        emitToken(TokenType::PERIOD);
        tokenStartLine = savedLine;
        tokenStartCol = savedCol;

        state = State::START;
    }
}

void Lexer::handleInReal() {
    char c = current();

    if (std::isdigit(c)) {
        buffer += c;
        advance();
    } else {
        emitToken(TokenType::REALCON, buffer);
        state = State::START;
    }
}

void Lexer::handleInString() {
    char c = current();

    if (c == '\'') {
        std::string value = "'" + buffer + "'";
        if (buffer.length() == 1) {
            emitToken(TokenType::CHARCON, value);
        } else {
            emitToken(TokenType::STRING, value);
        }
        state = State::START;
        advance();
    } else if (c == '\n') {
        emitError("Unterminated string literal");
        state = State::START;
    } else {
        buffer += c;
        advance();
    }
}

void Lexer::handleSawColon() {
    char c = current();

    if (c == '=') {
        emitToken(TokenType::BECOMES);
        state = State::START;
        advance();
    } else {
        emitToken(TokenType::COLON);
        state = State::START;
    }
}

void Lexer::handleSawLess() {
    char c = current();

    if (c == '>') {
        emitToken(TokenType::NEQ);
        state = State::START;
        advance();
    } else if (c == '=') {
        emitToken(TokenType::LEQ);
        state = State::START;
        advance();
    } else {
        emitToken(TokenType::LSS);
        state = State::START;
    }
}

void Lexer::handleSawGreater() {
    char c = current();

    if (c == '=') {
        emitToken(TokenType::GEQ);
        state = State::START;
        advance();
    } else {
        emitToken(TokenType::GTR);
        state = State::START;
    }
}

void Lexer::handleSawEqual() {
    char c = current();

    if (c == '=') {
        emitToken(TokenType::EQL);
        state = State::START;
        advance();
    } else {
        emitError("Unexpected '=' (expected '==')");
        state = State::START;
    }
}

void Lexer::handleInCurlyComment() {
    char c = current();

    if (c == '}') {
        buffer += '}';
        emitToken(TokenType::COMMENT, buffer);
        state = State::START;
        advance();
    } else {
        buffer += c;
        advance();
    }
}

void Lexer::handleSawLparen() {
    char c = current();

    if (c == '*') {
        buffer = "(*";
        state = State::IN_PAREN_STAR_COMMENT;
        advance();
    } else {
        emitToken(TokenType::LPARENT);
        state = State::START;
    }
}

void Lexer::handleInParenStarComment() {
    char c = current();

    if (c == '*') {
        buffer += '*';
        state = State::SAW_STAR_IN_PAREN_COMMENT;
        advance();
    } else {
        buffer += c;
        advance();
    }
}

void Lexer::handleSawStarInParenComment() {
    char c = current();

    if (c == ')') {
        buffer += ')';
        emitToken(TokenType::COMMENT, buffer);
        state = State::START;
        advance();
    } else if (c == '*') {
        buffer += '*';
        advance();
    } else {
        buffer += c;
        state = State::IN_PAREN_STAR_COMMENT;
        advance();
    }
}

void Lexer::handleInUnknown() {
    char c = current();
    if (isSeparator(c)) {
        emitToken(TokenType::UNKNOWN, buffer);
        errorFlag = true;
        state = State::START;
    } else {
        buffer += c;
        advance();
    }
}

void Lexer::handleEof() {
    switch (state) {
        case State::START:
            break;

        case State::IN_IDENT: {
            TokenType type = lookupKeyword(buffer);
            if (type == TokenType::IDENT) {
                emitToken(TokenType::IDENT, buffer);
            } else {
                emitToken(type);
            }
            break;
        }

        case State::IN_NUMBER:
            emitToken(TokenType::INTCON, buffer);
            break;

        case State::DOT_AFTER_NUM:
            emitToken(TokenType::INTCON, buffer);
            tokenStartLine = dotLine;
            tokenStartCol = dotCol;
            emitToken(TokenType::PERIOD);
            break;

        case State::IN_REAL:
            emitToken(TokenType::REALCON, buffer);
            break;

        case State::IN_STRING:
            emitError("Unterminated string literal at end of file");
            break;

        case State::SAW_COLON:
            emitToken(TokenType::COLON);
            break;

        case State::SAW_LESS:
            emitToken(TokenType::LSS);
            break;

        case State::SAW_GREATER:
            emitToken(TokenType::GTR);
            break;

        case State::SAW_EQUAL:
            emitError("Unexpected '=' at end of file (expected '==')");
            break;

        case State::IN_CURLY_COMMENT:
        case State::IN_PAREN_STAR_COMMENT:
        case State::SAW_STAR_IN_PAREN_COMMENT:
            emitError("Unterminated comment at end of file");
            break;

        case State::SAW_LPAREN:
            emitToken(TokenType::LPARENT);
            break;

        case State::IN_UNKNOWN:
            emitToken(TokenType::UNKNOWN, buffer);
            errorFlag = true;
            break;

        case State::FOUND_EOF:
            break;
    }
}
