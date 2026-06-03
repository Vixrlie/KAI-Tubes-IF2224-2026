# KAI-Tubes-IF2224-2026

## Deskripsi Umum Program
Proyek ini adalah implementasi compiler untuk bahasa pemrograman **Arion** pada Tugas Besar IF2224 Teori Bahasa Formal dan Otomata. Implementasi mencakup seluruh pipeline compiler dari frontend hingga backend:

- **Milestone 1:** Lexical analysis untuk mengubah source code menjadi token stream.
- **Milestone 2:** Syntax analysis dengan Recursive Descent Parser untuk membangun parse tree.
- **Milestone 3:** Semantic analysis untuk membangun AST, melakukan pengecekan tipe dan scope, serta menghasilkan symbol table dan decorated AST.
- **Milestone 4:** Intermediate code generation untuk meratakan decorated AST menjadi Three-Address Code (TAC), serta interpreter berbasis stack machine untuk mengeksekusi TAC dan menghasilkan output final.

Program utama pada versi terbaru adalah `arion_parser`. Program ini dapat membaca source code Arion, token stream hasil Milestone 1, atau parse tree hasil Milestone 2. Output terbaru berisi `tab`, `btab`, `atab`, parse tree, decorated AST, intermediate code, dan interpreter output (atau interpreter errors jika terjadi runtime error).

## Fitur Program
- Lexer untuk source code Arion.
- Pembacaan token stream hasil Milestone 1.
- Recursive Descent Parser untuk grammar Arion.
- Pembacaan parse tree hasil Milestone 2.
- Konversi parse tree menjadi AST.
- Semantic analysis dengan pengecekan deklarasi, scope, tipe data, assignment, procedure/function call, control-flow condition, array access, record field access, dan function result.
- Symbol table `tab`, `btab`, dan `atab`.
- Decorated AST dengan anotasi tipe, lexical level, block index, dan index symbol table.
- Intermediate code generator dengan instruction set TAC (LIT, LOD, STO, INT, JMP, JPC, OPR, CAL, RET).
- Interpreter stack machine dengan siklus fetch-decode-execute, stack frame management (static link, dynamic link, return address), dan runtime security checks.
- Error handling untuk lexical error, syntax error, semantic error, dan runtime error (stack overflow, out-of-bounds, division by zero, invalid jump target, integer overflow).
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

## Cara Menjalankan Milestone 4
Milestone 4 adalah tahap intermediate code generation dan interpreter execution. Program menjalankan seluruh pipeline dari source code hingga output akhir.

Run dari source code Arion:

```bash
make run INPUT=test/milestone-4/input-m4-1.txt
```

Simpan output ke file:

```bash
make run-output INPUT=test/milestone-4/input-m4-1.txt OUTPUT=output-m4.txt
```

Output Milestone 4 memiliki bagian berikut:

```txt
=== tab ===
=== btab ===
=== atab ===
=== Parse Tree ===
=== Decorated AST ===
=== Intermediate Code ===
=== Interpreter Output ===
```

Jika terjadi runtime error, program mengembalikan exit code `1` dan menampilkan daftar error, misalnya:

```txt
=== Interpreter Errors ===
- Interpreter: division by zero
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
│   ├── laporan-milestone-3/
│   └── laporan-milestone-4/
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
│   ├── semantic/
│   │   ├── ast_builder.h
│   │   ├── ast_builder.cpp
│   │   ├── ast_formatter.h
│   │   ├── ast_formatter.cpp
│   │   ├── ast_node.h
│   │   ├── ast_node.cpp
│   │   ├── semantic_analyzer.h
│   │   ├── semantic_analyzer.cpp
│   │   ├── symbol_table.h
│   │   └── symbol_table.cpp
│   ├── codegen/
│   │   ├── intermediate_code.h
│   │   ├── intermediate_code.cpp
│   │   ├── runtime_value.h
│   │   └── runtime_value.cpp
│   └── interpreter/
│       ├── interpreter.h
│       └── interpreter.cpp
└── test/
    ├── milestone-1/
    ├── milestone-2/
    ├── milestone-3/
    ├── milestone-4/
    └── semantic/
```

## Author Program
Tim **Kai**:
- Jonathan Kris Wicaksono (13524023)
- Vincent Rionarlie (13524031)
- Muhammad Aufar Rizqi Kusuma (13524061)
- Bryan Pratama Putra Hendra (13524067)

## Tabel Kontribusi

### Milestone 1 — Lexical Analysis

| No | Nama Lengkap | NIM | Deskripsi Pekerjaan | Persentase Kontribusi (%) |
|---|---|---|---|---:|
| 1 | Jonathan Kris Wicaksono | 13524023 | Membuat konsep algoritma program, menyusun `lexer.h` serta `lexer.c`. | 25 |
| 2 | Vincent Rionarlie | 13524031 | Melakukan pembagian tugas, mendesain diagram transisi DFA. | 25 |
| 3 | Muhammad Aufar Rizqi Kusuma | 13524061 | Menyusun laporan akhir. | 25 |
| 4 | Bryan Pratama Putra Hendra | 13524067 | Mengkodekan `token.h` dan `token.c`. | 25 |

### Milestone 2 — Syntax Analysis

| No | Nama Lengkap | NIM | Deskripsi Pekerjaan | Persentase Kontribusi (%) |
|---|---|---|---|---:|
| 1 | Jonathan Kris Wicaksono | 13524023 | Merevisi pembacaan separator (spasi, new-line, dan tab). | 25 |
| 2 | Vincent Rionarlie | 13524031 | Menyusun laporan akhir. | 25 |
| 3 | Muhammad Aufar Rizqi Kusuma | 13524061 | Membuat struktur data keseluruhan untuk Parse Tree. | 25 |
| 4 | Bryan Pratama Putra Hendra | 13524067 | Membuat algoritma core dari parser. | 25 |

### Milestone 3 — Semantic Analysis

| No | Nama Lengkap | NIM | Deskripsi Pekerjaan | Persentase Kontribusi (%) |
|---|---|---|---|---:|
| 1 | Jonathan Kris Wicaksono | 13524023 | AST: merancang node hierarchy AST, Syntax-Directed Translation untuk konversi parse tree ke AST, dan AST formatter untuk output Decorated AST. | 25 |
| 2 | Vincent Rionarlie | 13524031 | Semantic Analyzer: implementasi visit functions untuk setiap node, type checking dan compatibility rules, inferensi tipe, anotasi node AST, dan control flow validation. | 25 |
| 3 | Muhammad Aufar Rizqi Kusuma | 13524061 | Symbol Table: implementasi `tab`, `btab`, `atab`, manajemen scope push/pop, registrasi dan lookup identifier, serta inisialisasi predefined identifier. | 25 |
| 4 | Bryan Pratama Putra Hendra | 13524067 | Laporan dan README: penulisan laporan lengkap dan README, penyusunan bagian teori, serta quality assurance. | 25 |

### Milestone 4 — Intermediate Code and Interpreter

| No | Nama Lengkap | NIM | Deskripsi Pekerjaan | Persentase Kontribusi (%) |
|---|---|---|---|---:|
| 1 | Bryan Pratama Putra Hendra | 13524067 | Revisi milestone sebelumnya dan Intermediate Code Generator (flattening Decorated AST ke TAC, instruksi dasar LIT/LOD/STO/INT). | 25 |
| 2 | Vincent Rionarlie | 13524031 | Interpreter dan memori eksekusi (VM stack machine, stack frame dengan SL/DL/RA, siklus Fetch-Decode-Execute). | 25 |
| 3 | Muhammad Aufar Rizqi Kusuma | 13524061 | Control flow, operasi, dan keamanan mesin (translasi IF/WHILE ke JMP/JPC, operasi OPR, proteksi stack overflow dan numerical overflow). | 25 |
| 4 | Jonathan Kris Wicaksono | 13524023 | Pengujian, laporan, dan rilis (penyusunan test case, penulisan laporan PDF, manajemen release GitHub beserta README). | 25 |
