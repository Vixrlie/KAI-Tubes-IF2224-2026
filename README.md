# KAI-Tubes-IF2224-2026

## Deskripsi Umum Program
Proyek ini adalah implementasi frontend compiler untuk bahasa pemrograman **Arion** pada Tugas Besar IF2224 Teori Bahasa Formal dan Otomata. Implementasi mencakup:

- **Milestone 1:** lexical analysis untuk mengubah source code menjadi token.
- **Milestone 2:** syntax analysis dengan Recursive Descent Parser untuk membangun parse tree.
- **Milestone 3:** semantic analysis untuk membangun AST, melakukan pengecekan tipe dan scope, serta menghasilkan symbol table.

Program utama pada versi terbaru adalah `arion_parser`. Program ini dapat membaca source code Arion, token stream hasil Milestone 1, atau parse tree hasil Milestone 2. Output Milestone 3 berisi `tab`, `btab`, `atab`, parse tree, dan decorated AST.

## Fitur Program
- Lexer untuk source code Arion.
- Pembacaan token stream hasil Milestone 1.
- Recursive Descent Parser untuk grammar Arion.
- Pembacaan parse tree hasil Milestone 2.
- Konversi parse tree menjadi AST.
- Semantic analysis dengan pengecekan deklarasi, scope, tipe data, assignment, procedure/function call, control-flow condition, array access, record field access, dan function result.
- Symbol table `tab`, `btab`, dan `atab`.
- Decorated AST dengan anotasi tipe, lexical level, block index, dan index symbol table.
- Error handling untuk lexical error, syntax error, dan semantic error.
- Output dapat ditampilkan ke terminal dan/atau disimpan ke file.

## Requirement Menjalankan Program
- OS Windows/Linux/macOS.
- Compiler C++ dengan dukungan C++17, disarankan `g++`.
- `make` untuk build melalui Makefile.
- Shell/terminal untuk menjalankan command.

## Build dan Clean
Build program utama:

```bash
make
```

Build ulang dari awal:

```bash
make -B
```

Membersihkan hasil build:

```bash
make clean
```

Catatan:
- Makefile sudah menggunakan `-std=c++17`.
- Jika `make` belum tersedia di Windows, jalankan dari MSYS2/MinGW/Git Bash, atau gunakan `mingw32-make` sesuai environment.

## Cara Menjalankan Milestone 1
Milestone 1 adalah tahap lexical analysis. Jika executable standalone `lexer` tersedia, jalankan:

```bash
./lexer test/milestone-1/input-1.txt
```

Untuk menyimpan token stream ke file:

```bash
./lexer test/milestone-1/input-1.txt output-m1.txt
```

Pada Windows:

```bash
.\lexer.exe test\milestone-1\input-1.txt output-m1.txt
```

Format output lexer berupa token stream, misalnya:

```txt
programsy
ident (Hello)
semicolon
varsy
ident (a)
```

Pada versi Milestone 3, lexer juga dipakai otomatis oleh `arion_parser` saat input yang diberikan adalah source code Arion.

## Cara Menjalankan Milestone 2
Milestone 2 adalah tahap syntax analysis. Pada versi terbaru, parse tree tetap dicetak pada bagian `=== Parse Tree ===` dari output program utama.

Run dari source code Arion:

```bash
make run INPUT=test/milestone-2/input-1.txt
```

Run dari token stream hasil Milestone 1:

```bash
make run INPUT=test/milestone-1/output-1.txt
```

Simpan output ke file:

```bash
make run-output INPUT=test/milestone-2/input-1.txt OUTPUT=output-m2.txt
```

Output program versi terbaru tidak hanya parse tree, tetapi juga symbol table dan decorated AST karena pipeline sudah dilanjutkan sampai Milestone 3.

## Cara Menjalankan Milestone 3
Milestone 3 adalah tahap semantic analysis. Program menerima source code, token stream, atau parse tree sebagai input.

Run dari source code Arion:

```bash
make run INPUT=test/milestone-3/input-1.txt
```

Run dari parse tree hasil Milestone 2:

```bash
make run INPUT=test/milestone-2/output-1.txt
```

Simpan output ke file:

```bash
make run-output INPUT=test/milestone-3/input-1.txt OUTPUT=output-m3.txt
```

Output Milestone 3 memiliki bagian berikut:

```txt
=== tab ===
=== btab ===
=== atab ===
=== Parse Tree ===
=== Decorated AST ===
```

Jika ditemukan semantic error, program mengembalikan exit code `1` dan menampilkan daftar error, misalnya:

```txt
=== Semantic Errors ===
- Undefined identifier: x
```

## Format Input File `.txt`
Program utama `arion_parser` menerima tiga bentuk input:

1. Source code Arion biasa.
2. Token stream hasil Milestone 1.
3. Parse tree hasil Milestone 2.

Contoh source code:

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

Contoh token stream:

```txt
programsy
ident (Hello)
semicolon
varsy
ident (a)
comma
ident (b)
colon
ident (integer)
semicolon
```

Contoh parse tree:

```txt
<program>
├── <program-header>
│   ├── programsy
│   ├── ident(Hello)
│   └── semicolon
```

## Struktur Direktori Program
```text
KAI-Tubes-IF2224-2026/
├── Makefile
├── README.md
├── doc/
│   ├── laporan-milestone-1/
│   ├── laporan-milestone-2/
│   └── laporan-milestone-3/
├── src/
│   ├── main.cpp
│   ├── lexer/
│   │   ├── lexer.h
│   │   ├── lexer.cpp
│   │   ├── token.h
│   │   ├── token.cpp
│   │   ├── token_stream_reader.h
│   │   └── token_stream_reader.cpp
│   ├── parser/
│   │   ├── parser.h
│   │   ├── parser.cpp
│   │   ├── parse_tree_node.h
│   │   ├── parse_tree_node.cpp
│   │   ├── parse_tree_reader.h
│   │   ├── parse_tree_reader.cpp
│   │   ├── parse_tree_utils.h
│   │   └── parse_tree_utils.cpp
│   └── semantic/
│       ├── ast_builder.h
│       ├── ast_builder.cpp
│       ├── ast_formatter.h
│       ├── ast_formatter.cpp
│       ├── ast_node.h
│       ├── ast_node.cpp
│       ├── semantic_analyzer.h
│       ├── semantic_analyzer.cpp
│       ├── symbol_table.h
│       └── symbol_table.cpp
└── test/
    ├── milestone-1/
    ├── milestone-2/
    ├── milestone-3/
    └── semantic/
```

## Author Program
Tim **Kai**:
- Jonathan Kris Wicaksono (13524023)
- Vincent Rionarlie (13524031)
- Muhammad Aufar Rizqi Kusuma (13524061)
- Bryan Pratama Putra Hendra (13524067)

## Tabel Kontribusi
| No | Nama Lengkap | NIM | Deskripsi Pekerjaan | Persentase Kontribusi (%) |
|---|---|---|---|---:|
| 1 | Jonathan Kris Wicaksono | 13524023 | AST: merancang node hierarchy AST, Syntax-Directed Translation untuk konversi parse tree ke AST, dan AST formatter untuk output Decorated AST. | 25 |
| 2 | Vincent Rionarlie | 13524031 | Semantic Analyzer: implementasi visit functions untuk setiap node, type checking & compatibility rules, inferensi tipe, anotasi node AST, dan control flow validation. | 25 |
| 3 | Muhammad Aufar Rizqi Kusuma | 13524061 | Symbol Table: implementasi tab, btab, atab, manajemen scope push/pop, registrasi & lookup identifier, dan inisialisasi predefined identifier. | 25 |
| 4 | Bryan Pratama Putra Hendra | 13524067 | Laporan & README: penulisan laporan lengkap dan README, serta quality assurance. | 25 |