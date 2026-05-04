#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include "token.h"

// Defines the lexer state machine and the public lexer interface.
enum class State {
    START,
    IN_IDENT,
    IN_NUMBER,
    DOT_AFTER_NUM,
    IN_REAL,
    IN_STRING,
    SAW_COLON,
    SAW_LESS,
    SAW_GREATER,
    SAW_EQUAL,
    IN_CURLY_COMMENT,
    SAW_LPAREN,
    IN_PAREN_STAR_COMMENT,
    SAW_STAR_IN_PAREN_COMMENT,
    IN_UNKNOWN,
    FOUND_EOF
};

class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();
    bool hasErrors() const;

private:
    std::string source;
    int pos;
    int line;
    int col;
    State state;
    std::string buffer;
    int tokenStartLine;
    int tokenStartCol;
    int dotLine;
    int dotCol;
    bool errorFlag;
    std::vector<Token> tokens;

    char current() const;
    bool isAtEnd() const;
    void advance();
    void emitToken(TokenType type, const std::string& value = "");
    void emitError(const std::string& message);
    TokenType lookupKeyword(const std::string& identifier) const;

    static bool isSeparator(char c);

    void handleStart();
    void handleInIdent();
    void handleInNumber();
    void handleDotAfterNum();
    void handleInReal();
    void handleInString();
    void handleSawColon();
    void handleSawLess();
    void handleSawGreater();
    void handleSawEqual();
    void handleInCurlyComment();
    void handleSawLparen();
    void handleInParenStarComment();
    void handleSawStarInParenComment();
    void handleInUnknown();

    void handleEof();
};

#endif
