/**
 * token.h
 *
 * Definisi tipe token dan struktur data Token untuk bahasa pemrograman Arion.
 * File ini mendefinisikan seluruh 52 jenis token yang dikenali oleh lexer,
 * beserta fungsi utilitas untuk konversi dan format output.
 */

#ifndef TOKEN_H
#define TOKEN_H

#include <string>

/**
 * Enum TokenType
 * Merepresentasikan seluruh jenis token dalam bahasa Arion.
 * Sesuai dengan spesifikasi: 52 token + ERROR untuk penanganan kesalahan.
 */
enum class TokenType {
    // Literal (konstanta)
    INTCON,         // 1.  Konstanta integer: 1, 3, 48
    REALCON,        // 2.  Konstanta bilangan riil: 3.14, 26.7
    CHARCON,        // 3.  Konstanta karakter singular: 'j', 'k'
    STRING,         // 4.  Sekuens karakter: 'IRK', 'TBFO'

    // Operator logika
    NOTSY,          // 5.  NOT
    ANDSY,          // 12. AND
    ORSY,           // 13. OR

    // Operator aritmatika
    PLUS,           // 6.  +
    MINUS,          // 7.  -
    TIMES,          // 8.  *
    IDIV,           // 9.  div (pembagian integer)
    RDIV,           // 10. /   (pembagian riil)
    IMOD,           // 11. MOD

    // Operator perbandingan
    EQL,            // 14. ==
    NEQ,            // 15. <>
    GTR,            // 16. >
    GEQ,            // 17. >=
    LSS,            // 18. <
    LEQ,            // 19. <=

    // Delimiter
    LPARENT,        // 20. (
    RPARENT,        // 21. )
    LBRACK,         // 22. [
    RBRACK,         // 23. ]
    COMMA,          // 24. ,
    SEMICOLON,      // 25. ;
    PERIOD,         // 26. .
    COLON,          // 27. :
    BECOMES,        // 28. :=

    // Keyword deklarasi
    CONSTSY,        // 29. const
    TYPESY,         // 30. type
    VARSY,          // 31. var
    FUNCTIONSY,     // 32. function
    PROCEDURESY,    // 33. procedure
    ARRAYSY,        // 34. array
    RECORDSY,       // 35. record
    PROGRAMSY,      // 36. program

    // Identifier
    IDENT,          // 37. identifier

    // Keyword kontrol alur
    BEGINSY,        // 38. begin
    IFSY,           // 39. if
    CASESY,         // 40. case
    REPEATSY,       // 41. repeat
    WHILESY,        // 42. while
    FORSY,          // 43. for
    ENDSY,          // 44. end
    ELSESY,         // 45. else
    UNTILSY,        // 46. until
    OFSY,           // 47. of
    DOSY,           // 48. do
    TOSY,           // 49. to
    DOWNTOSY,       // 50. downto
    THENSY,         // 51. then

    // Komentar
    COMMENT,        // 52. { ... } atau (* ... *)

    // Error (untuk penanganan kesalahan)
    ERROR
};

/**
 * Struktur Token
 * Menyimpan informasi lengkap tentang sebuah token yang dihasilkan lexer.
 */
struct Token {
    TokenType type;     // Jenis token
    std::string value;  // Nilai token (untuk ident, intcon, realcon, charcon, string)
    int line;           // Nomor baris awal token (1-based)
    int column;         // Nomor kolom awal token (1-based)
};

/* Mengubah TokenType menjadi string untuk output */
std::string tokenTypeToString(TokenType type);

/* Mengecek apakah token memiliki nilai yang perlu ditampilkan */
bool tokenHasValue(TokenType type);

/* Memformat token untuk output sesuai spesifikasi */
std::string formatToken(const Token& token);

#endif // TOKEN_H
