# KAI-Tubes-IF2224-2026

## Deskripsi Umum Program
Proyek ini adalah implementasi **Lexical Analyzer (Lexer)** untuk bahasa pemrograman **Arion** pada Milestone 1 Tugas Besar IF2224 Teori Bahasa Formal dan Otomata. Program membaca source code dari berkas `.txt`, memproses karakter demi karakter menggunakan pendekatan **Deterministic Finite Automata (DFA)**, kemudian menghasilkan daftar token sesuai spesifikasi bahasa Arion.

## Fitur Program
- Tokenisasi source code Arion menjadi token stream.
- Mendukung 52 jenis token (literal, operator, delimiter, keyword, identifier, dan komentar).
- Mendukung komentar `{ ... }` dan `(* ... *)` termasuk multiline comment.
- Pengenalan keyword secara case-insensitive.
- Pelaporan lexical error tanpa menghentikan seluruh proses tokenisasi.
- Output dapat ditampilkan ke terminal dan/atau disimpan ke file output `.txt`.
- Menyisipkan baris kosong pada output untuk menjaga keterbacaan sesuai struktur baris source.

## Requirement Menjalankan Program
- OS Windows/Linux/macOS.
- Compiler C++ dengan dukungan C++17 (disarankan `g++`).
- Shell/terminal untuk build dan run.

## How To Run Program
### 1. Build
```bash
g++ -std=c++17 -Wall -Wextra -Isrc/lexer src/main.cpp src/lexer/lexer.cpp src/lexer/token.cpp -o lexer
```

### 2. Run (output ke terminal)
```bash
./lexer test/milestone-1/input-1.txt
```

### 3. Run (output ke file)
```bash
./lexer test/milestone-1/input-1.txt test/milestone-1/output-1.txt
```

Pada Windows PowerShell, gunakan `./lexer.exe` jika executable bernama `lexer.exe`.

## Format Input File `.txt`
Input berupa source code Arion biasa. Contoh:

```txt
program Hello;

var
	a, b: integer;

begin
	a := 5;
	b := a + 10;
	writeln('Result = ', b);
end.
```

Program akan menghasilkan token per baris, misalnya `programsy`, `ident (Hello)`, `semicolon`, dan seterusnya.

## Struktur Direktori Program
```text
KAI-Tubes-IF2224-2026/
├── Makefile
├── README.md
├── doc/
│   └── laporan-milestone-1/
│       ├── main.tex
│       ├── dafpus.bib
│       ├── sections/
│       │   ├── 01-pendahuluan.tex
│       │   ├── 02-dasar-teori.tex
│       │   ├── 03-implementasi.tex
│       │   ├── 04-eksperimen.tex
│       │   ├── 05-penutup.tex
│       │   └── 06-lampiran.tex
│       └── public/
├── src/
│   ├── main.cpp
│   └── lexer/
│       ├── lexer.h
│       ├── lexer.cpp
│       ├── token.h
│       └── token.cpp
└── test/
		├── milestone-1/
		│   ├── input-*.txt
		│   ├── expected-*.txt
		│   └── output-*.txt
		├── milestone-2/
		├── milestone-3/
		└── milestone-4/
```

## Author Program
Tim **Kai**:
- Jonathan Kris Wicaksono (13524023)
- Vincent Rionarlie (13524031)
- Muhammad Aufar Rizqi Kusuam (13524061)
- Bryan Pratama Putra Hendra (13524067)

## MIT License
Proyek ini menggunakan lisensi **MIT**.

Copyright (c) 2026 Kai

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.