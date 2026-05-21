

# 

# **`Milestone 1`** **`Lexical Analysis`**

---

## `Release	: Kamis, 12 Maret 2026`

## `Deadline	: Kamis, 2 April 2026 pukul 23.59 WIB` 

## `Revisi	: -`

# 

# **`Daftar Revisi`** {#daftar-revisi}

\[DD-MM-YY\] Revisi X. Isi Revisi

# **`Daftar Isi`** {#daftar-isi}

[**Daftar Revisi	3**](#daftar-revisi)

[**Daftar Isi	4**](#daftar-isi)

[**I. Pendahuluan Tugas Besar	5**](#i.-pendahuluan-tugas-besar)

[**II. Teori Singkat	7**](#ii.-teori-singkat)

[**III. Spesifikasi Tugas Besar	8**](#iii.-spesifikasi-tugas-besar)

[A. Spesifikasi Program	8](#spesifikasi-program)

[B. Daftar Token Bahasa Pemrograman Arion	9](#daftar-token-bahasa-pemrograman-arion)

[C. Contoh Input/Output	12](#contoh-input/output)

[D. Diagram Transisi DFA	13](#diagram-transisi-dfa)

[E. Laporan	14](#laporan)

[**IV. Teknis Pengerjaan	15**](#iv.-teknis-pengerjaan)

[**VI. Pengumpulan	16**](#vi.-pengumpulan)

[**V. Penilaian	17**](#v.-penilaian)

[**Referensi	19**](#referensi)

[**Lampiran	20**](#lampiran)

# 

# **`I. Pendahuluan Tugas Besar`** {#i.-pendahuluan-tugas-besar}

Apa itu **Bahasa Pemrograman**? Bahasa pemrograman adalah cara agar kita dapat menuliskan program komputer tanpa perlu menghadapi sulitnya menulis dalam bahasa mesin. Bahasa mesin (biner 1 dan 0\) adalah satu-satunya bahasa yang dapat dipahami dan dijalankan oleh CPU, namun sangat sulit dibaca oleh manusia. Oleh karena itu, bahasa pemrograman sering kali dibedakan dengan istilah low-level atau high-level. High-level artinya bahasa lebih mendekati bahasa manusia dan sebaliknya low-level lebih mendekati bahasa mesin. Semakin mendekati bahasa manusia, maka akan semakin banyak proses yang diperlukan untuk mengubahnya menjadi kode mesin.

![][image1]  
Gambar 1\. Tipe-tipe bahasa pemrograman  
(Sumber: asisten)

Terdapat dua macam cara mengubah/memproses bahasa pemrograman menjadi machine code, yaitu **compiler** dan **interpreter**. Compiler adalah program yang menerjemahkan seluruh source code menjadi kode mesin (dalam bentuk file e.g. Object File) sebelum program dijalankan. Compiler secara umum terdiri dari 4 tahapan, mulai dari Lexical Analysis, Syntax Analysis, Semantic Analysis hingga Intermediate Code Generation. 

Di sisi lain adalah Interpreter yang memproses source code baris per baris secara langsung. Interpreter membaca, menerjemahkan, dan langsung mengeksekusi kode mesin baris per baris sehingga jika terdapat error pada baris ke 10, program akan tetap berjalan normal untuk 9 baris awal, dibandingkan dengan compiler yang akan gagal dijalankan jika terdeteksi terdapat error di dalam source code. Namun dibalik itu, interpreter sebenarnya memiliki tahapan yang sama seperti compiler.

Pada Tugas Besar Teori Bahasa Formal dan Automata, Anda diminta untuk membuat **Interpreter** untuk bahasa pemrograman  **Arion**, suatu bahasa pemrograman baru yang akan Anda kembangkan. Tugas besar ini dibagi menjadi **4 Milestone**, yaitu **Lexical Analysis, Syntax Analysis, Semantic Analysis, Intermediate Code Generation** dan **Interpreter.** Setiap Milestone akan memiliki pengumpulannya dan penilaiannya tersendiri.

Pada milestone pertama, Anda diminta untuk membuat tahapan Lexical Analysis dari suatu interpreter.

# **`II. Teori Singkat`** {#ii.-teori-singkat}

Analisis leksikal (lexical analysis) adalah tahap pertama dalam proses kompilasi atau interpretasi bahasa pemrograman. Pada tahap ini, *interpreter* membaca kode sumber mentah yang pada dasarnya hanyalah rangkaian karakter dan mengubahnya menjadi rangkaian satuan makna yang disebut token. **Token merupakan unit terkecil yang memiliki arti dalam sebuah program**, misalnya kata kunci (keyword), nama variabel (identifier), operator, angka, atau tanda baca.

Tujuan utama dari analisis leksikal adalah untuk mempermudah tahap berikutnya, yaitu parsing. Alih-alih berurusan langsung dengan karakter mentah, tahap parsing bekerja dengan kumpulan token yang sudah terstruktur, sehingga lebih mudah memahami susunan logis dari program. Pada proses ini, analisis leksikal juga membuang bagian yang tidak diperlukan seperti spasi, tab, dan komentar, serta dapat mendeteksi kesalahan seperti simbol yang tidak dikenal sejak awal.

Proses ini dilakukan oleh komponen yang disebut lexer. **Lexer membaca kode sumber dari kiri ke kanan dan menggunakan pola tertentu untuk mengenali jenis token yang berbeda-beda**. Ketika menemukan urutan karakter yang cocok dengan salah satu pola tersebut, lexer membuat sebuah token yang terdiri dari jenis token dan nilai token. Contoh hasil token:

* IDENT (a)  
* BECOMES  
* INTCON (5)

Agar proses pembacaan tersebut dapat dijalankan secara otomatis oleh komputer, mereka harus diimplementasikan ke dalam bentuk Finite Automata, khususnya **Deterministic Finite Automata (DFA)**.

# **`III. Spesifikasi Tugas Besar`** {#iii.-spesifikasi-tugas-besar}

Pada milestone 1, Anda diminta untuk membuat **lexer atau lexical analyzer** dalam **C/C++** yang menggunakan **Deterministic Finite Automata (DFA)** untuk menghasilkan **token** dari source code bahasa pemrograman Arion.

Deliverables:

* Source Code program,  
* Diagram Transisi DFA, dan  
* Laporan.

1. ## **Spesifikasi Program** {#spesifikasi-program}

Dalam milestone ini, Anda diminta untuk membuat **program Lexer** dalam bahasa pemrograman **C/C++ GNU**. Anda **DILARANG** **menggunakan library atau tools** apapun yang dapat membantu menghasilkan lexical analyzer. Anda wajib menggunakan **MAKEFILE** untuk mempermudah pengembangan dalam bahasa C/C++.

Program Lexer harus berfungsi selayaknya suatu **DFA**. Program Lexer **HANYA BOLEH** membaca **satu demi satu karakter** dari input source code dan memprosesnya sesuai dengan aturan pada DFA. Program harus dapat menerima **input source code** dalam bentuk **.txt**. Program kemudian memproses source code dengan **aturan DFA** dan mengeluarkan **output [daftar token](#daftar-token-bahasa-pemrograman-arion)** dalam format **.txt**.

| Masukan | Kode Sumber (.txt) |
| :---- | :---- |
| Keluaran | Daftar Token (.txt) |
| Otomata | Deterministic finite automata (DFA) |

**Catatan Penting:** Untuk mempermudah pengerjaan pada milestone-milestone selanjutnya, khususnya pada milestone 4 (pembuatan interpreter), SANGAT DISARANKAN agar program dikembangkan secara modular (terpisah antara komponen Lexer, Semantic, Syntax, dan Intermediate Code Generation). Hal ini supaya komponen dapat dimodifikasi sesuai keperluan tanpa mempengaruhi komponen lainnya, serta dapat dengan mudah dijalankan dalam satu pipeline: lexer \- syntax \- semantic \- intermediate code generation.

2. ## **Daftar Token Bahasa Pemrograman Arion** {#daftar-token-bahasa-pemrograman-arion}

	Lexer bertujuan untuk mengubah source code dari kumpulan karakter mentah menjadi unit-unit bermakna yang disebut **token**. Setiap token memiliki **jenis (type)** dan **nilai (value)** yang akan digunakan untuk tahap selanjutnya dalam proses kompilasi. Lexer diharapkan dapat menerima source code dan mengeluarkan daftar token yang dihasilkan. 

Berikut merupakan daftar token yang harus dapat dibaca oleh lexical analyzer:

| Daftar Token |  |  |  |
| ----- | ----- | ----- | ----- |
| No | Token | Definisi & Keterangan | Contoh Pemakaian |
| 1 | **intcon** | Konstanta integer | 1, 3, 48 |
| 2 | **realcon** | Konstanta bilangan riil | 3.14, 26.7 |
| 3 | **charcon** | Konstanta karakter singular, diapit dengan tanda petik tunggal (‘) | ‘j’, ‘k’, ‘t’ |
| 4 | **string** | Sekuens karakter, diapit dengan tanda petik tunggal (‘) | ‘IRK’, ‘TBFO’ |
| 5 | **notsy** | Operator logika NOT (negasi) | NOT |
| 6 | **plus** | Operator aritmatika pertambahan | \+ |
| 7 | **minus** | Operator aritmatika pengurangan | \- |
| 8 | **times** | Operator aritmatika perkalian | \* |
| 9 | **idiv** | Operator aritmatika pembagian Integer | div |
| 10 | **rdiv** | Operator aritmatika pembagian Riil | / |
| 11 | **imod** | Operator aritmatika modulo | MOD |
| 12 | **andsy** | Operator logika AND | AND |
| 13 | **orsy** | Operator logika OR | OR |
| 14 | **eql** | equal | \== |
| 15 | **neq** | Not equal | \<\> |
| 16 | **gtr** | Greater than | \> |
| 17 | **geq** | Greater than or equal | \>= |
| 18 | **lss** | Less than | \< |
| 19 | **leq** | Less than or equal | \<= |
| 20 | **lparent** | Left parentheses | ( |
| 21 | **rparent** | Right parantheses | ) |
| 22 | **lbrack** | Kurung siku kiri | \[ |
| 23 | **rbrack** | Kurung siku kanan | \] |
| 24 | **comma** | \- | , |
| 25 | **semicolon** | \- | ; |
| 26 | **period** | \- | . |
| 27 | **colon** | \- | : |
| 28 | **becomes** | Assignment operator | := |
| 29 | **constsy** | Deklarasi konstanta | const |
| 30 | **typesy** | Deklarasi tipe data | type  |
| 31 | **varsy** | Deklarasi variabel | var |
| 32 | **functionsy** | Deklarasi fungsi | function |
| 33 | **proceduresy** | Deklarasi prosedur | procedure |
| 34 | **arraysy** | Deklarasi array | array |
| 35 | **recordsy** | Deklarasi record | record |
| 36 | **programsy** | Deklarasi program | program |
| 37 | **ident** | Nama yang menunjukkan konstanta, tipe, variabel, prosedur, dan fungsi. Nama tersebut harus diawali dengan huruf, yang dapat diikuti oleh kombinasi huruf dan angka apa pun, dan berjumlah berapa pun. Bersifat case-insensitive | x, PI, MyInt, Integer, Real, Char, Stringh4g,identIniSangatPanjang24tetapiTetapValid |
| 38 | **beginsy** | \- | begin |
| 39 | **ifsy** | \- | if |
| 40 | **casesy** | \- | case |
| 41 | **repeatsy** | \- | repeat |
| 42 | **whilesy** | \- | while |
| 43 | **forsy** | \- | for |
| 44 | **endsy** | \- | end |
| 45 | **elsesy** | \- | else |
| 46 | **untilsy** | \- | until |
| 47 | **ofsy** | \- | of |
| 48 | **dosy** | \- | do |
| 49 | **tosy** | \- | to |
| 50 | **downtosy** | \- | downto |
| 51 | **thensy** | \- | then |
| 52 | **comment** | Dimulai dengan { atau (\* (bukan di dalam tanda kutip) dan diakhiri dengan } atau \*). | { Ini komentar }, (\* In komentar \*) |

3. ## **Contoh Input/Output** {#contoh-input/output}

Anda wajib membuat **sebuah DFA** yang dapat menentukan pola karakter dan menghasilkan token. Berikut adalah contoh format Input/Output dari program.

| Masukan (input) |
| :---- |
| program Hello; var   a, b: integer; begin   a := 5;   b := a \+ 10;   writeln('Result \= ', b); end. |
| Contoh visualisasi pemrosesan dalam program (State DFA belum tentu benar) |
| p \=\> State 1 r \=\> State 1 .  m \=\> State 2 \=\> Gotten: program . H \=\> State 3 e . ; \=\> char \=\> State 0 \=\> Gotten: ident(Hello) & semicolon . . . |
| Keluaran (output) |
| programsy ident (Hello) semicolon varsy ident (a) comma ident (b) colon ident (integer) semicolon beginsy ident (a) becomes intcon (5) semicolon  ident (b) becomes ident (a) plus intcon (10) semicolon ident (writeln) lparent string (‘Result \=’) comma ident (b) rparent semicolon endsy period |

4. ## **Diagram Transisi DFA** {#diagram-transisi-dfa}

Selain program, Anda wajib membuat **diagram transisi DFA** sesuai dengan **DFA** yang dibuat. Diagram wajib dibuat pada workspace berbasis web, seperti **draw io** (untuk workspace lain silahkan ditanyakan terlebih dahulu pada QNA). Pastikan pembuatan diagram transisi sesuai dengan aturan yang dipelajari di salinda perkuliahan.

**DILARANG** menggunakan library atau tools yang mengubah aturan DFA dalam program menjadi gambar diagram secara otomatis. Diagram transisi DFA harus dibuat secara manual. Hal ini dilakukan untuk memastikan bahwa mahasiswa memahami aturan DFA yang dibuatnya.

5. ## **Laporan** {#laporan}

   Anda wajib membuat laporan yang setidaknya berisi:  
* Cover dengan foto kelompok menggantikan logo ITB.  
* Teori Singkat   
  * Jelaskan teori-teori atau konsep yang digunakan (DFD, Lexer, Token, dsb)  
* Perancangan & Implementasi  
  * Cantumkan dan beri penjelasan diagram transisi DFA yang dibuat dan digunakan.  
  * Jelaskan implementasi program, terutama yang berhubungan dengan Lexical Analyzer dan DFA.  
* Pengujian  
  * Setidaknya 5 pengujian kasus unik.  
  * Setiap pengujian harus menunjukan hasil input/output program.  
  * Bukti berupa screenshot input/output program.  
* Kesimpulan dan Saran  
* Lampiran berupa:  
  * Link Repository Github.  
  * Link Workspace Diagram Transisi DFA  
  * Pembagian Tugas (menunjukan persentase kontribusi).  
* Referensi 

# 

# **`IV. Teknis Pengerjaan`** {#iv.-teknis-pengerjaan}

* Tugas Besar ini dikerjakan secara **berkelompok 4-5 orang** dan **tidak diperbolehkan lintas kelas**.   
* Setiap kelompok **WAJIB** mengisi [sheets pembagian kelompok](https://docs.google.com/spreadsheets/d/15ChahGWH1Zbv3Kq2e_TEq6e5GFXIxAp81pTZFxCTNBQ/edit?usp=sharing). Jika melewati deadline dan Anda tidak mengisi, maka akan dialokasikan secara acak.    
  * Deadline Pengisian adalah **Kamis, 26 Maret 2026 pukul 23.59 WIB.**

* Buatlah **Repository Github** dengan format **\[KODE\]-Tubes-IF2224-2026**, dengan kode adalah Kode Kelompok sesuai sheets pembagian kelompok.  
* Repository setidaknya mengandung:  
  * Folder **src** untuk menyimpan seluruh **source code**  
  * Folder **doc** untuk menyimpan **laporan**   
  * Folder **test** untuk menyimpan **semua input/output** yang dilakukan pada bagian pengujian laporan. Untuk membedakan dengan milestone lain, maka file dipisahkan ke dalam folder untuk setiap milestone.   
    Sebagai contoh: **\`test/milestone-1/output-1.txt\`**.  
  * **README** yang setidaknya mengandung:  
    * Identitas Kelompok  
    * Deskripsi Program  
    * Requirements  
    * Cara Instalasi dan Penggunaan Program  
    * Pembagian Tugas

* Jika ada pertanyaan lebih lanjut, silahkan ditanyakan pada [QNA](https://docs.google.com/spreadsheets/d/1sOf2QwQINQcqyy0lnMHJPAN6vPRQLd7jEntoPmNjWyk/edit?usp=sharing).

# 

# **`VI. Pengumpulan`** {#vi.-pengumpulan}

* **Pengumpulan bersifat Hard Deadline**

* Pada setiap Milestone, Anda **WAJIB**  membuat **Release.**   
  * Gunakan format tag berupa **v0.X.Y** dengan **X** adalah **nomor milestone** dan **Y** adalah **nomor pengumpulan**.  
  * Sebagai contoh, untuk milestone 1, release pertama, dan pengumpulan pertama adalah **v0.1.1**. Untuk pengumpulan selanjutnya karena revisi, gunakan tag **v0.1.2** dan seterusnya sesuai nomor pengumpulan. Untuk milestone selanjutnya, pengumpulan dimulai kembali dari satu yaitu **v0.2.1**, **V0.2.2**, dan seterusnya.

* **Laporan** dikumpulkan dengan cara **meletakkan** hasil **PDF** dalam **folder doc** dengan format **Laporan-X-\[KODE\].pdf**, dimana **X** adalah **nomor milestone.** Pastikan isi laporan sudah sesuai dengan [**teknis pengerjaan**](#iv.-teknis-pengerjaan).  
    
* Link Pengumpulan: [**Pengumpulan Milestone**](https://forms.gle/uWmAAKAqw3d3o72M6)  
* Deadline milestone: **Kamis, 02 April 2026 pukul 23.59 WIB**

# 

# **`V. Penilaian`** {#v.-penilaian}

| Program (70 poin) |  |  |  |
| ----- | ----- | ----- | ----- |
| No | Komponen | Poin | Syarat Poin Maksimal |
| 1\. | Kelengkapan token | 10 | Program mampu mengenali seluruh jenis token yang ditentukan dalam spesifikasi  |
| 2\. | Kesesuaian klasifikasi token | 10 | Setiap token diidentifikasi dan diklasifikasikan dengan benar sesuai kategori |
| 3\. | Struktur/modularitas program | 8 | Kode ditulis dengan struktur modular menggunakan fungsi atau kelas yang terpisah berdasarkan tanggung jawab. Alur eksekusi program mudah diikuti dan tidak redundan |
| 4\. | Error handling | 5 | Program dapat menangani error dengan baik (pesan informatif dan error tidak menyebabkan crash) |
| 5\. | Penerimaan input | 3 | Program dapat membaca file input (.txt) |
| 6\. | Penghasilan output | 2 | Program menghasilkan output daftar token secara lengkap dan terformat dengan jelas ke terminal |
| 7\. | Dokumentasi/komentar | 5 | Kode diberi komentar yang informatif dan tidak berlebihan |
| 8\. | Kesesuaian DFA  | 12 | Program berjalan selayaknya suatu DFA dan Source Code program sesuai dengan diagram transisi DFA  |
| 9\. | Pengujian | 15 | Program berhasil lolos seluruh kasus uji |

| Laporan & Diagram (30 poin) |  |  |  |
| ----- | ----- | ----- | ----- |
| No | Komponen | Poin | Syarat Poin Maksimal |
| 1\. | Penjelasan konsep | 10 | Penjelasan mengenai konsep lexer yang mencakup pengertian, fungsi, dan peran lexer dalam proses kompilasi, serta kaitannya dengan token dan DFA. |
| 2\. | Perancangan program | 5 | Penjelasan mengenai rancangan program yang mencakup teknologi yang digunakan, struktur program, fungsi/kelas utama, dan alur kerja program. Terdapat pula justifikasi atas pilihan implementasi |
| 3\. | Hasil implementasi | 2 | Dokumentasi mengenai hasil implementasi, dapat berupa tangkapan layar dengan penjelasan |
| 4\. | Test case | 3 | Dokumentasi mengenai pengujian mandiri (minimal 5 dan mencakup berbagai kasus) |
| 5\. | Diagram Transisi DFA | 10 | Diagram Transisi DFA digambar dengan benar dan lengkap |

# **`Referensi`** {#referensi}

“Compiler Design \- Lexical Analysis”  
[https://www.tutorialspoint.com/compiler\_design/compiler\_design\_lexical\_analysis.htm](https://www.tutorialspoint.com/compiler_design/compiler_design_lexical_analysis.htm) 

“Introduction to Automata Theory, Languages, and Computation 3rd Edition”   
[https://cdn-edunex.itb.ac.id/29161-Formal-Language-Theory-and-Automata/1629640939613\_Introduction-to-Automata-Theory,-Languages,-and-Computation-Edition-3.pdf](https://cdn-edunex.itb.ac.id/29161-Formal-Language-Theory-and-Automata/1629640939613_Introduction-to-Automata-Theory,-Languages,-and-Computation-Edition-3.pdf) 

“Compilers \- Principles, Techniques, and Tools (2006)”  
[https://repository.unikom.ac.id/48769/1/Compilers%20-%20Principles,%20Techniques,%20and%20Tools%20(2006).pdf](https://repository.unikom.ac.id/48769/1/Compilers%20-%20Principles,%20Techniques,%20and%20Tools%20\(2006\).pdf)

“An Introduction to Makefiles”   
[https://www.gnu.org/software/make/manual/html\_node/Introduction.html](https://www.gnu.org/software/make/manual/html_node/Introduction.html) 

# **`Lampiran`** {#lampiran}

Laman Pembagian Kelompok: [Pembagian Kelompok - Tubes IF2224 TBFO](https://docs.google.com/spreadsheets/d/15ChahGWH1Zbv3Kq2e_TEq6e5GFXIxAp81pTZFxCTNBQ/edit?usp=sharing)  
Laman QnA Tugas Besar: [QNA - Tubes IF2224 TBFO](https://docs.google.com/spreadsheets/d/1sOf2QwQINQcqyy0lnMHJPAN6vPRQLd7jEntoPmNjWyk/edit?gid=0#gid=0)  
From Pengumpulan: [Form Pengumpulan \- Tubes IF2224 TBFO](https://forms.gle/uWmAAKAqw3d3o72M6) 

[image1]: <data:image/png;base64,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>