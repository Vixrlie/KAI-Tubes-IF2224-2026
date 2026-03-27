/**
 * Deklarasi kelas Lexer untuk bahasa pemrograman Arion.
 * Lexer menggunakan Deterministic Finite Automata (DFA) untuk
 * membaca source code karakter demi karakter dan menghasilkan token.
 */

#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include "token.h"

/**
 * Enum State - State DFA untuk lexical analyzer.
 *
 * DFA terdiri dari 13 state yang merepresentasikan kondisi pembacaan:
 *   START                        : State awal, menunggu karakter baru
 *   IN_IDENT                     : Sedang membaca identifier/keyword
 *   IN_NUMBER                    : Sedang membaca digit integer
 *   DOT_AFTER_NUM                : Melihat '.' setelah digit (bisa real atau period)
 *   IN_REAL                      : Sedang membaca bagian desimal bilangan riil
 *   IN_STRING                    : Sedang membaca karakter di dalam tanda kutip
 *   SAW_COLON                    : Melihat ':' (bisa colon atau becomes)
 *   SAW_LESS                     : Melihat '<' (bisa lss, leq, atau neq)
 *   SAW_GREATER                  : Melihat '>' (bisa gtr atau geq)
 *   SAW_EQUAL                    : Melihat '=' pertama (mengharapkan '=' kedua)
 *   IN_CURLY_COMMENT             : Di dalam komentar curly
 *   SAW_LPAREN                   : Melihat '(' (bisa lparent atau awal komentar)
 *   IN_PAREN_STAR_COMMENT        : Di dalam komentar star
 *   SAW_STAR_IN_PAREN_COMMENT    : Di dalam komentar star, melihat '*' (bisa akhir komentar)
 */
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
    SAW_STAR_IN_PAREN_COMMENT
};

/**
 * Kelas Lexer
 *
 * Lexical analyzer berbasis DFA yang membaca source code karakter demi karakter.
 * Menghasilkan daftar token sesuai spesifikasi bahasa Arion.
 */
class Lexer {
public:
    /**
     * Konstruktor.
     * @param source String berisi seluruh source code yang akan di-tokenisasi
     */
    explicit Lexer(const std::string& source);

    /**
     * Menjalankan DFA dan menghasilkan seluruh token.
     * @return Vector berisi token-token hasil analisis leksikal
     */
    std::vector<Token> tokenize();

    /**
     * Mengecek apakah terdapat error selama proses tokenisasi.
     * @return true jika ada error, false jika tidak
     */
    bool hasErrors() const;

private:
    std::string source;         // Source code lengkap
    int pos;                    // Posisi karakter saat ini
    int line;                   // Nomor baris saat ini (1-based)
    int col;                    // Nomor kolom saat ini (1-based)
    State state;                // State DFA saat ini
    std::string buffer;         // Buffer akumulasi karakter untuk token
    int tokenStartLine;         // Baris awal token yang sedang diproses
    int tokenStartCol;          // Kolom awal token yang sedang diproses
    int dotLine;                // Baris posisi '.' (untuk DOT_AFTER_NUM)
    int dotCol;                 // Kolom posisi '.' (untuk DOT_AFTER_NUM)
    bool errorFlag;             // Flag adanya error
    std::vector<Token> tokens;  // Daftar token yang dihasilkan

    /* Mendapatkan karakter pada posisi saat ini */
    char current() const;

    /* Mengecek apakah sudah mencapai akhir input */
    bool isAtEnd() const;

    /* Mengkonsumsi karakter saat ini dan memajukan posisi */
    void advance();

    /* Menambahkan token ke daftar output */
    void emitToken(TokenType type, const std::string& value = "");

    /* Melaporkan error leksikal */
    void emitError(const std::string& message);

    /* Mencari keyword berdasarkan identifier (case-insensitive) */
    TokenType lookupKeyword(const std::string& identifier) const;

    /* Handler untuk setiap state DFA */
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

    /* Menangani akhir file untuk token yang belum selesai */
    void handleEof();
};

#endif // LEXER_H