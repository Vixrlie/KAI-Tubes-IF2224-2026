/**
 * lexer.cpp
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
 * Mengkonsumsi karakter saat ini dan memperbarui posisi baris/kolom.
 * Newline memajukan baris dan mereset kolom ke 1.
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
 * Posisi token menggunakan tokenStartLine/tokenStartCol yang
 * di-set di awal pembacaan setiap token.
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
 * Melaporkan error leksikal.
 * Pesan error dicetak ke stderr dan token ERROR ditambahkan ke daftar.
 * Program tetap melanjutkan proses tokenisasi setelah error.
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
 * Mencari keyword berdasarkan identifier.
 * Perbandingan bersifat case-insensitive: identifier diubah ke lowercase
 * sebelum dicocokkan dengan tabel keyword.
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
 * Menjalankan DFA untuk menghasilkan seluruh token dari source code.
 * Loop utama membaca karakter satu per satu dan memanggil handler
 * sesuai dengan state DFA saat ini.
 */
std::vector<Token> Lexer::tokenize() {
    while (!isAtEnd()) {
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
            case State::IN_COMMENT:          handleInComment(); break;
            case State::SAW_LPAREN:          handleSawLparen(); break;
            case State::SAW_STAR_IN_COMMENT: handleSawStarInComment(); break;
        }
    }
    // Tangani token yang belum selesai di akhir file
    handleEof();
    return tokens;
}

/* ========== Handler State DFA ========== */

/**
 * State START - State awal DFA.
 * Membaca karakter pertama dari setiap token baru dan menentukan
 * transisi ke state yang sesuai.
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

    // Abaikan whitespace
    if (std::isspace(c)) {
        advance();
        return;
    }

    // Simpan posisi awal token
    tokenStartLine = line;
    tokenStartCol = col;

    if (std::isalpha(c)) {
        // Huruf: mulai identifier atau keyword
        buffer = c;
        state = State::IN_IDENT;
        advance();
    } else if (std::isdigit(c)) {
        // Digit: mulai bilangan integer
        buffer = c;
        state = State::IN_NUMBER;
        advance();
    } else if (c == '\'') {
        // Tanda kutip tunggal: mulai string/charcon
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
        // Awal komentar dengan kurung kurawal
        buffer = "{";
        state = State::IN_COMMENT;
        advance();
    } else if (c == '(') {
        // Bisa lparent atau awal komentar (*
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
        // Karakter tidak dikenali
        emitError("Unexpected character '" + std::string(1, c) + "'");
        advance();
    }
}

/**
 * State IN_IDENT - Membaca identifier atau keyword.
 * Identifier dimulai dengan huruf dan diikuti oleh huruf atau digit.
 *
 * Transisi:
 *   huruf/digit -> tetap di IN_IDENT (akumulasi ke buffer)
 *   lainnya     -> emit token (keyword atau ident), kembali ke START
 *                  (karakter saat ini di-reprocess)
 */
void Lexer::handleInIdent() {
    char c = current();

    if (std::isalnum(c)) {
        buffer += c;
        advance();
    } else {
        // Identifier selesai, cek apakah keyword
        TokenType type = lookupKeyword(buffer);
        if (type == TokenType::IDENT) {
            emitToken(TokenType::IDENT, buffer);
        } else {
            // Keyword tidak menyimpan value
            emitToken(type);
        }
        state = State::START;
        // Tidak advance: karakter saat ini di-reprocess di START
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

    if (std::isdigit(c)) {
        buffer += c;
        advance();
    } else if (c == '.') {
        // Simpan posisi titik untuk kemungkinan emit PERIOD
        dotLine = line;
        dotCol = col;
        state = State::DOT_AFTER_NUM;
        advance();
    } else {
        // Integer selesai
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

    if (std::isdigit(c)) {
        // Titik adalah pemisah desimal, lanjut ke bilangan riil
        buffer += '.';
        buffer += c;
        state = State::IN_REAL;
        advance();
    } else {
        // Titik bukan bagian bilangan, emit integer lalu period
        emitToken(TokenType::INTCON, buffer);

        // Emit period dengan posisi titik yang benar
        int savedLine = tokenStartLine;
        int savedCol = tokenStartCol;
        tokenStartLine = dotLine;
        tokenStartCol = dotCol;
        emitToken(TokenType::PERIOD);
        tokenStartLine = savedLine;
        tokenStartCol = savedCol;

        state = State::START;
        // Tidak advance: karakter saat ini di-reprocess
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
 * Mengakumulasi karakter ke buffer sampai menemukan kutip penutup.
 * Setelah selesai, token diklasifikasikan:
 *   - 1 karakter  -> CHARCON
 *   - selainnya   -> STRING
 *
 * Transisi:
 *   '       -> emit charcon/string, kembali ke START
 *   newline -> error (string tidak boleh lintas baris)
 *   lain    -> tetap di IN_STRING (akumulasi)
 */
void Lexer::handleInString() {
    char c = current();

    if (c == '\'') {
        // Kutip penutup ditemukan
        std::string value = "'" + buffer + "'";
        if (buffer.length() == 1) {
            emitToken(TokenType::CHARCON, value);
        } else {
            emitToken(TokenType::STRING, value);
        }
        state = State::START;
        advance();
    } else if (c == '\n') {
        // String tidak boleh mengandung newline
        emitError("Unterminated string literal");
        state = State::START;
        // Tidak advance: newline di-reprocess di START
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

    if (c == '=') {
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
 *   > -> emit NEQ (<>), kembali ke START
 *   = -> emit LEQ (<=), kembali ke START
 *   l -> emit LSS (<), kembali ke START (reprocess)
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
 *   = -> emit GEQ (>=), kembali ke START
 *   l -> emit GTR (>), kembali ke START (reprocess)
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
        // Tidak advance: karakter saat ini di-reprocess
    }
}

/**
 * State IN_COMMENT - Di dalam komentar.
 * Komentar bisa dimulai dengan '{' atau '(*' dan diakhiri dengan '}' atau '*)'.
 * Komentar boleh lintas baris (multiline).
 *
 * Transisi:
 *   } -> emit COMMENT, kembali ke START
 *   * -> SAW_STAR_IN_COMMENT (mungkin akhir komentar *)
 *   l -> tetap di IN_COMMENT (akumulasi)
 */
void Lexer::handleInComment() {
    char c = current();

    if (c == '}') {
        // Komentar berakhir dengan '}'
        buffer += '}';
        emitToken(TokenType::COMMENT, buffer);
        state = State::START;
        advance();
    } else if (c == '*') {
        // Mungkin awal dari penutup '*)'
        buffer += '*';
        state = State::SAW_STAR_IN_COMMENT;
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
 *   * -> IN_COMMENT (awal komentar)
 *   l -> emit LPARENT, kembali ke START (reprocess)
 */
void Lexer::handleSawLparen() {
    char c = current();

    if (c == '*') {
        // Awal komentar (*
        buffer = "(*";
        state = State::IN_COMMENT;
        advance();
    } else {
        emitToken(TokenType::LPARENT);
        state = State::START;
        // Tidak advance: karakter saat ini di-reprocess
    }
}

/**
 * State SAW_STAR_IN_COMMENT - Di dalam komentar, melihat '*'.
 * Menentukan apakah '*' diikuti ')' (akhir komentar) atau bukan.
 *
 * Transisi:
 *   ) -> emit COMMENT (*), kembali ke START
 *   } -> emit COMMENT (komentar juga bisa diakhiri '}')
 *   * -> tetap di SAW_STAR_IN_COMMENT (akumulasi '*')
 *   l -> IN_COMMENT (bukan akhir komentar)
 */
void Lexer::handleSawStarInComment() {
    char c = current();

    if (c == ')') {
        // Komentar berakhir dengan '*)'
        buffer += ')';
        emitToken(TokenType::COMMENT, buffer);
        state = State::START;
        advance();
    } else if (c == '}') {
        // Komentar juga bisa berakhir dengan '}'
        buffer += '}';
        emitToken(TokenType::COMMENT, buffer);
        state = State::START;
        advance();
    } else if (c == '*') {
        // '*' berturut, tetap menunggu ')'
        buffer += '*';
        advance();
    } else {
        // Bukan akhir komentar, kembali ke IN_COMMENT
        buffer += c;
        state = State::IN_COMMENT;
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
            // Tidak ada yang perlu dilakukan
            break;

        case State::IN_IDENT: {
            // Identifier/keyword terakhir
            TokenType type = lookupKeyword(buffer);
            if (type == TokenType::IDENT) {
                emitToken(TokenType::IDENT, buffer);
            } else {
                emitToken(type);
            }
            break;
        }

        case State::IN_NUMBER:
            // Integer terakhir
            emitToken(TokenType::INTCON, buffer);
            break;

        case State::DOT_AFTER_NUM:
            // Integer diikuti titik di akhir file
            emitToken(TokenType::INTCON, buffer);
            tokenStartLine = dotLine;
            tokenStartCol = dotCol;
            emitToken(TokenType::PERIOD);
            break;

        case State::IN_REAL:
            // Bilangan riil terakhir
            emitToken(TokenType::REALCON, buffer);
            break;

        case State::IN_STRING:
            // String tidak ditutup
            emitError("Unterminated string literal at end of file");
            break;

        case State::SAW_COLON:
            // ':' di akhir file adalah colon
            emitToken(TokenType::COLON);
            break;

        case State::SAW_LESS:
            // '<' di akhir file
            emitToken(TokenType::LSS);
            break;

        case State::SAW_GREATER:
            // '>' di akhir file
            emitToken(TokenType::GTR);
            break;

        case State::SAW_EQUAL:
            // '=' tunggal di akhir file
            emitError("Unexpected '=' at end of file (expected '==')");
            break;

        case State::IN_COMMENT:
        case State::SAW_STAR_IN_COMMENT:
            // Komentar tidak ditutup
            emitError("Unterminated comment at end of file");
            break;

        case State::SAW_LPAREN:
            // '(' di akhir file
            emitToken(TokenType::LPARENT);
            break;
    }
}
