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
make
```

### 2. Run (output ke terminal)
```bash
make run INPUT=test/milestone-1/input-1.txt
```

### 3. Run (output ke file)
```bash
make run-output INPUT=test/milestone-1/input-1.txt OUTPUT=test/milestone-1/output-1.txt
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
- Source code Arion biasa, sesuai alur milestone 2.
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
- Muhammad Aufar Rizqi Kusuma (13524061)
- Bryan Pratama Putra Hendra (13524067)

## Tabel Kontribusi
| No | Nama Lengkap | NIM | Deskripsi Pekerjaan | Persentase Kontribusi (%) |
|---|---|---|---|---:|
| 1 | Jonathan Kris Wicaksono | 13524023 | Membuat konsep algoritma program, menyusun `lexer.h` serta `lexer.cpp` | 25 |
| 2 | Vincent Rionarlie | 13524031 | Melakukan pembagian tugas, mendesain diagram transisi DFA | 25 |
| 3 | Muhammad Aufar Rizqi Kusuma | 13524061 | Menyusun laporan akhir | 25 |
| 4 | Bryan Pratama Putra Hendra | 13524067 | Mengkodekan `token.h` dan `token.cpp` | 25 |