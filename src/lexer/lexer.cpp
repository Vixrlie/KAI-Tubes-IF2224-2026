/**
 *
 * Implementasi Lexer berbasis Deterministic Finite Automata (DFA)
 * untuk bahasa pemrograman Arion.
 *
 * Lexer membaca source code karakter demi karakter dan melakukan
 * transisi antar state DFA untuk mengenali dan menghasilkan token.
 * Setiap karakter yang dibaca menentukan transisi ke state berikutnya.
 */

#include "lexer.h"
#include <cctype>
#include <algorithm>
#include <map>
#include <iostream>

/**
 * Tabel keyword untuk bahasa Arion (case-insensitive).
 * Ketika sebuah identifier cocok dengan salah satu keyword,
 * token yang dihasilkan adalah keyword tersebut, bukan IDENT.
 */
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

/* ========== Konstruktor ========== */

Lexer::Lexer(const std::string& source)
    : source(source), pos(0), line(1), col(1),
      state(State::START), tokenStartLine(1), tokenStartCol(1),
      dotLine(0), dotCol(0), errorFlag(false) {}

/* ========== Fungsi Utilitas ========== */

char Lexer::current() const {
    if (isAtEnd()) return '\0';
    return source[pos];
}

bool Lexer::isAtEnd() const {
    return pos >= static_cast<int>(source.size());
}

/**
 * Ambil karakter saat ini, dan maju ke karakter selanjutnya
 */
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

/**
 * Menambahkan token ke daftar hasil.
 */
void Lexer::emitToken(TokenType type, const std::string& value) {
    Token token;
    token.type = type;
    token.value = value;
    token.line = tokenStartLine;
    token.column = tokenStartCol;
    tokens.push_back(token);
}

/**
 * Print error (ke token)
 */
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

/**
 * Mencari keyword yang sesuai
 * @return Tipe keyword jika cocok, IDENT jika bukan keyword
 */
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

/* ========== Fungsi Utama Tokenisasi ========== */

/**
 * Cek state, lalu melanjutkan ke state DFA yang bersangkutan
 */
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
                case State::FOUND_EOF:           break;
            }
        }
    }
    
    return tokens;
}

/* ========== Handler State DFA ========== */

/**
 * State START - State awal DFA.
 * Membaca karakter pertama dari setiap token baru dan
 * menentukan state selanjutnya
 * 
 * Transisi:
 *   huruf       -> IN_IDENT
 *   digit       -> IN_NUMBER
 *   '           -> IN_STRING
 *   :           -> SAW_COLON
 *   <           -> SAW_LESS
 *   >           -> SAW_GREATER
 *   =           -> SAW_EQUAL
 *   {           -> IN_COMMENT
 *   (           -> SAW_LPAREN
 *   +,-,*,/,dll -> emit token langsung, kembali ke START
 *   whitespace  -> diabaikan, tetap di START
 *   lainnya     -> error
 */
void Lexer::handleStart() {
    char c = current();

    // ignore whitespace
    if (std::isspace(c)) {
        advance();
        return;
    }

    tokenStartLine = line;
    tokenStartCol = col;

    if (std::isalpha(c)) { // huruf
        buffer = c;
        state = State::IN_IDENT;
        advance();
    } else if (std::isdigit(c)) { // integer
        buffer = c;
        state = State::IN_NUMBER;
        advance();
    } else if (c == '\'') { // petik = string
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
    } else if (c == '{') { // comment curly
        buffer = "{";
        state = State::IN_CURLY_COMMENT;
        advance();
    } else if (c == '(') { // comment star
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
    } else { // unknown
        emitError("Unexpected character '" + std::string(1, c) + "'");
        advance();
    }
}

/**
 * State IN_IDENT - Membaca identifier atau keyword.
 * Identifier dimulai dengan huruf dan diikuti oleh huruf atau digit.
 *
 * Transisi:
 *   huruf/digit -> tetap di IN_IDENT
 *   lainnya     -> emit token (keyword atau ident), kembali ke START
 */
void Lexer::handleInIdent() {
    char c = current();

    if (std::isalnum(c)) { // lanjut baca ampe gadapet
        buffer += c;
        advance();
    } else { // cek keyword exist?
        TokenType type = lookupKeyword(buffer);
        if (type == TokenType::IDENT) {
            emitToken(TokenType::IDENT, buffer);
        } else {
            emitToken(type);
        }
        state = State::START;
    }
}

/**
 * State IN_NUMBER - Membaca digit bilangan integer.
 *
 * Transisi:
 *   digit -> tetap di IN_NUMBER
 *   .     -> DOT_AFTER_NUM (mungkin bilangan riil atau period)
 *   lain  -> emit INTCON, kembali ke START (reprocess)
 */
void Lexer::handleInNumber() {
    char c = current();

    if (std::isdigit(c)) { // lanjut
        buffer += c;
        advance();
    } else if (c == '.') { // bs jadi float ato titik biasa
        dotLine = line;
        dotCol = col;
        state = State::DOT_AFTER_NUM;
        advance();
    } else {
        emitToken(TokenType::INTCON, buffer);
        state = State::START;
    }
}

/**
 * State DOT_AFTER_NUM - Melihat '.' setelah digit integer.
 * Menentukan apakah titik adalah bagian dari bilangan riil atau period.
 *
 * Transisi:
 *   digit -> IN_REAL (titik adalah desimal, gabung ke buffer)
 *   lain  -> emit INTCON + emit PERIOD, kembali ke START (reprocess)
 */
void Lexer::handleDotAfterNum() {
    char c = current();

    if (std::isdigit(c)) { // float
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

/**
 * State IN_REAL - Membaca bagian desimal bilangan riil.
 * Buffer sudah berisi digit integer + '.' + minimal satu digit desimal.
 *
 * Transisi:
 *   digit -> tetap di IN_REAL
 *   lain  -> emit REALCON, kembali ke START (reprocess)
 */
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

/**
 * State IN_STRING - Membaca karakter di antara tanda kutip tunggal.
 * Membaca karakter ke buffer sampai menemukan kutip penutup.
 * Setelah selesai, token diklasifikasikan:
 *   - 1 karakter  -> CHARCON
 *   - sisanya     -> STRING
 *
 * Transisi:
 *   '       -> emit charcon/string, kembali ke START
 *   newline -> error
 *   lain    -> tetap di IN_STRING
 */
void Lexer::handleInString() {
    char c = current();

    if (c == '\'') { // kutip tutup
        std::string value = "'" + buffer + "'";
        if (buffer.length() == 1) {
            emitToken(TokenType::CHARCON, value);
        } else {
            emitToken(TokenType::STRING, value);
        }
        state = State::START;
        advance();
    } else if (c == '\n') { // error
        emitError("Unterminated string literal");
        state = State::START;
    } else {
        buffer += c;
        advance();
    }
}

/**
 * State SAW_COLON - Melihat karakter ':'.
 * Menentukan apakah ini colon biasa atau assignment operator (:=).
 *
 * Transisi:
 *   = -> emit BECOMES, kembali ke START
 *   l -> emit COLON, kembali ke START (reprocess)
 */
void Lexer::handleSawColon() {
    char c = current();

    if (c == '=') { // operator
        emitToken(TokenType::BECOMES);
        state = State::START;
        advance();
    } else {
        emitToken(TokenType::COLON);
        state = State::START;
    }
}

/**
 * State SAW_LESS - Melihat karakter '<'.
 * Menentukan token: <, <=, atau <>.
 *
 * Transisi:
 *   > -> emit NEQ, kembali ke START
 *   = -> emit LEQ, kembali ke START
 *   l -> emit LSS, kembali ke START (reprocess)
 */
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

/**
 * State SAW_GREATER - Melihat karakter '>'.
 * Menentukan token: > atau >=.
 *
 * Transisi:
 *   = -> emit GEQ, kembali ke START
 *   l -> emit GTR, kembali ke START (reprocess)
 */
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

/**
 * State SAW_EQUAL - Melihat karakter '=' pertama.
 * Mengharapkan '=' kedua untuk membentuk operator '=='.
 * '=' tunggal bukan token valid di Arion.
 *
 * Transisi:
 *   = -> emit EQL (==), kembali ke START
 *   l -> emit error, kembali ke START (reprocess)
 */
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

/**
 * State IN_CURLY_COMMENT - Di dalam komentar curly braces {...}.
 *
 * Transisi:
 *   } -> emit COMMENT, kembali ke START
 *   l -> tetap di IN_CURLY_COMMENT (akumulasi)
 */
void Lexer::handleInCurlyComment() {
    char c = current();

    if (c == '}') { // tutup
        buffer += '}';
        emitToken(TokenType::COMMENT, buffer);
        state = State::START;
        advance();
    } else {
        buffer += c;
        advance();
    }
}

/**
 * State SAW_LPAREN - Melihat karakter '('.
 * Menentukan apakah ini left parenthesis atau awal komentar '(*'.
 *
 * Transisi:
 *   * -> IN_PAREN_STAR_COMMENT (awal komentar)
 *   l -> emit LPARENT, kembali ke START (reprocess)
 */
void Lexer::handleSawLparen() {
    char c = current();

    if (c == '*') { // awal star comment
        buffer = "(*";
        state = State::IN_PAREN_STAR_COMMENT;
        advance();
    } else {
        emitToken(TokenType::LPARENT);
        state = State::START;
    }
}

/**
 * State IN_PAREN_STAR_COMMENT - Di dalam komentar (*...*).
 *
 * Transisi:
 *   * -> SAW_STAR_IN_PAREN_COMMENT (mungkin akhir komentar *)
 *   l -> tetap di IN_PAREN_STAR_COMMENT (akumulasi)
 */
void Lexer::handleInParenStarComment() {
    char c = current();

    if (c == '*') { // * penutup?
        buffer += '*';
        state = State::SAW_STAR_IN_PAREN_COMMENT;
        advance();
    } else {
        buffer += c;
        advance();
    }
}

/**
 * State SAW_STAR_IN_PAREN_COMMENT - Di dalam komentar (*, melihat '*'.
 * Menentukan apakah '*' diikuti ')' (akhir komentar) atau bukan.
 *
 * Transisi:
 *   ) -> emit COMMENT (*), kembali ke START
 *   * -> tetap di SAW_STAR_IN_PAREN_COMMENT (akumulasi '*')
 *   l -> IN_PAREN_STAR_COMMENT (bukan akhir komentar)
 */
void Lexer::handleSawStarInParenComment() {
    char c = current();

    if (c == ')') { // penutup
        buffer += ')';
        emitToken(TokenType::COMMENT, buffer);
        state = State::START;
        advance();
    } else if (c == '*') { // ngulang terus ampe ')'
        buffer += '*';
        advance();
    } else { // balik ke comment state
        buffer += c;
        state = State::IN_PAREN_STAR_COMMENT;
        advance();
    }
}

/**
 * Handler akhir file (EOF).
 * Menangani token yang belum selesai ketika input habis.
 * Emit token yang valid atau error untuk token yang tidak lengkap.
 */
void Lexer::handleEof() {
    switch (state) {
        case State::START:
            break;

        case State::IN_IDENT: { // print ident terakhir
            TokenType type = lookupKeyword(buffer);
            if (type == TokenType::IDENT) {
                emitToken(TokenType::IDENT, buffer);
            } else {
                emitToken(type);
            }
            break;
        }

        case State::IN_NUMBER: // print int terakhir
            emitToken(TokenType::INTCON, buffer);
            break;

        case State::DOT_AFTER_NUM:
            emitToken(TokenType::INTCON, buffer);
            tokenStartLine = dotLine;
            tokenStartCol = dotCol;
            emitToken(TokenType::PERIOD);
            break;

        case State::IN_REAL: // print float terakhir
            // Bilangan riil terakhir
            emitToken(TokenType::REALCON, buffer);
            break;

        case State::IN_STRING: // string ga ketutup
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
        case State::SAW_STAR_IN_PAREN_COMMENT: // comment kebuka
            emitError("Unterminated comment at end of file");
            break;

        case State::SAW_LPAREN:
            emitToken(TokenType::LPARENT);
            break;
        
        case State::FOUND_EOF:
            break;
    }
}