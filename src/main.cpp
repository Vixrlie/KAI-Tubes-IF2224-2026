/**
 * main.cpp
 *
 * Entry point program Lexer untuk bahasa pemrograman Arion.
 *
 * Program membaca source code dari file input (.txt),
 * menjalankan lexical analysis menggunakan DFA,
 * dan menghasilkan daftar token ke terminal dan/atau file output (.txt).
 *
 * Usage:
 *   ./lexer <input.txt> [output.txt]
 *
 *   input.txt  : File source code yang akan dianalisis
 *   output.txt : (opsional) File output untuk menyimpan daftar token
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer/lexer.h"

/**
 * Menghitung baris akhir token.
 * Untuk token multiline (seperti komentar), baris akhir dihitung
 * berdasarkan jumlah newline dalam nilai token.
 */
int getEndLine(const Token &token)
{
    if (token.type == TokenType::COMMENT)
    {
        int newlines = 0;
        for (char c : token.value)
        {
            if (c == '\n')
                newlines++;
        }
        return token.line + newlines;
    }
    return token.line;
}

/**
 * Memformat daftar token menjadi string output.
 * Menyisipkan baris kosong di output ketika terdapat baris kosong
 * di source code (gap > 1 baris antar token berurutan).
 * Menggunakan baris akhir token sebelumnya untuk penanganan komentar multiline.
 */
std::string formatOutput(const std::vector<Token> &tokens)
{
    std::string output;
    int prevEndLine = 0;

    for (const auto &token : tokens)
    {
        // Sisipkan baris kosong jika ada gap baris di source code
        if (prevEndLine > 0 && token.line - prevEndLine > 1)
        {
            output += "\n";
        }
        output += formatToken(token) + "\n";
        prevEndLine = getEndLine(token);
    }

    return output;
}

int main(int argc, char *argv[])
{
    // Validasi argumen command-line
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input.txt> [output.txt]" << std::endl;
        return 1;
    }

    // Baca file input
    std::ifstream inputFile(argv[1]);
    if (!inputFile.is_open())
    {
        std::cerr << "Error: Cannot open input file '" << argv[1] << "'" << std::endl;
        return 1;
    }

    std::stringstream ss;
    ss << inputFile.rdbuf();
    std::string source = ss.str();
    inputFile.close();

    // Jalankan lexer
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    // Format output
    std::string output = formatOutput(tokens);

    // Cetak ke terminal
    std::cout << output;

    // Simpan ke file output jika diberikan
    if (argc >= 3)
    {
        std::ofstream outputFile(argv[2]);
        if (!outputFile.is_open())
        {
            std::cerr << "Error: Cannot open output file '" << argv[2] << "'" << std::endl;
            return 1;
        }
        outputFile << output;
        outputFile.close();
    }

    return lexer.hasErrors() ? 1 : 0;
}
