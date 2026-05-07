# KAI-Tubes-IF2224-2026

## Deskripsi Umum Program
Proyek ini adalah implementasi **parser Recursive Descent** untuk bahasa pemrograman **Arion** pada Milestone 2 Tugas Besar IF2224 Teori Bahasa Formal dan Otomata. Program membaca source code Arion atau token stream hasil Milestone 1, lalu membangun **parse tree** sesuai grammar Arion dan menampilkan hasilnya.

## Fitur Program
- Lexer untuk source code Arion dan pembacaan token stream hasil milestone 1.
- Recursive Descent Parser untuk seluruh non-terminal grammar Arion.
- Membangun parse tree yang dicetak dengan struktur pohon.
- Lookahead satu token untuk pemilihan produksi tanpa backtracking.
- Pelaporan syntax error dengan informasi baris dan kolom.
- Menolak token `ERROR` dan `UNKNOWN` sejak awal parsing.
- Output dapat ditampilkan ke terminal dan/atau disimpan ke file keluaran.

## Requirement Menjalankan Program
- OS Windows/Linux/macOS.
- Compiler C++ dengan dukungan C++17 (disarankan `g++`).
- Shell/terminal untuk build dan run.

## Cara Menjalankan (Milestone 2)
### 1. Build
```bash
make
```

### 2. Run (output ke terminal)
```bash
make run INPUT=test/milestone-2/input-1.txt
```

### 3. Run (output ke file)
```bash
make run-output INPUT=test/milestone-2/input-1.txt OUTPUT=test/milestone-2/output-1.txt
```

### 4. Clean hasil build
```bash
make clean
```

Catatan:
- Makefile sudah mendukung Windows dan Unix-like shell.
- Jika `make` belum tersedia di Windows, jalankan dari MSYS2/MinGW/Git Bash, atau gunakan `mingw32-make` sesuai environment.

## Format Input File `.txt`
Program menerima dua bentuk input:
- Source code Arion biasa.
- Daftar token hasil milestone 1 dalam format `.txt`, misalnya `test/milestone-1/output-1.txt`.

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

Contoh input token stream:

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

## Struktur Direktori Program
```text
KAI-Tubes-IF2224-2026/
├── Makefile
├── README.md
├── doc/
│   ├── laporan-milestone-1/
│   └── laporan-milestone-2/
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
│   ├── lexer/
│   │   ├── lexer.h
│   │   ├── lexer.cpp
│   │   ├── token.h
│   │   └── token.cpp
│   └── parser/
│       ├── parser.h
│       ├── parser.cpp
│       ├── parse_tree_node.h
│       ├── parse_tree_node.cpp
│       ├── parse_tree_utils.h
│       └── parse_tree_utils.cpp
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
- Muhammad Aufar Rizqi Kusuma (13524061)
- Bryan Pratama Putra Hendra (13524067)

## Tabel Kontribusi
| No | Nama Lengkap | NIM | Deskripsi Pekerjaan | Persentase Kontribusi (%) |
|---|---|---|---|---:|
| 1 | Jonathan Kris Wicaksono | 13524023 | Implementasi parser recursive descent dan validasi token | 25 |
| 2 | Vincent Rionarlie | 13524031 | Desain grammar dan pemetaan produksi pada parser | 25 |
| 3 | Muhammad Aufar Rizqi Kusuma | 13524061 | Dokumentasi laporan milestone 2 dan pengujian | 25 |
| 4 | Bryan Pratama Putra Hendra | 13524067 | Implementasi parse tree dan utilitas traversal | 25 |