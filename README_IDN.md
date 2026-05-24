<div style="text-align: center;">

# ESP32 Demo HTTP Client & Prakiraan Cuaca BMKG

**English** · [Indonesia](README_IDN.md)

</div>

<p align="center">
  <a href="#detail-project">Detail Project</a> ·
  <a href="#fitur">Fitur</a> ·
  <a href="#keunggulan">Keunggulan</a> ·
  <a href="#kebutuhan-modul">Kebutuhan Modul</a> ·
  <a href="#katalog-fitur">Katalog Fitur</a> ·
  <a href="#tech-stack">Tech Stack</a> ·
  <a href="#build-dan-install">Build dan Install</a> ·
  <a href="#kontribusi">Kontribusi</a>
</p>

Aplikasi IoT interaktif yang dibuat untuk **ESP32** dengan memanfaatkan web server lokal, penyimpanan file lokal pada flash memory (**LittleFS**), dan API eksternal untuk mengambil prakiraan cuaca real-time di Indonesia. Aplikasi ini memungkinkan pengguna mencari desa, kecamatan, kota/kabupaten, atau provinsi di Indonesia secara dinamis, mengambil detail cuaca langsung dari **API BMKG (Badan Meteorologi, Klimatologi, dan Geofisika)**, serta mendemonstrasikan pengiriman data keluar menggunakan request **HTTP POST** ke web server remote.

---

## Fitur

* **Web Server Lokal:** Menyediakan antarmuka web pada jaringan lokal.
* **Integrasi API BMKG:** Mengambil data cuaca dari BMKG menggunakan API cuaca.
* **HTTP POST:** Mengirim data cuaca ke server remote.
* **mDNS:** Memudahkan akses ke antarmuka web menggunakan hostname lokal, misalnya `esp32-wether.local`.

---

## Alur Sistem & Arsitektur

Diagram di bawah ini menggambarkan bagaimana pengguna, ESP32, dan layanan API eksternal saling berinteraksi:

```mermaid
sequenceDiagram
    autonumber
    actor User
    participant Browser as Web Browser (LittleFS UI)
    participant ESP32 as ESP32 Web Server
    participant BMKG as BMKG API
    participant Beeceptor as Beeceptor (Echo Server)

    Note over Browser: Halaman dimuat & mengambil CSV Permendagri
    User->>Browser: Mencari & memilih lokasi hingga tingkat desa
    Browser->>ESP32: POST /kode (kode=location_code)
    activate ESP32
    ESP32->>BMKG: GET /publik/prakiraan-cuaca?adm4=code
    BMKG-->>ESP32: Mengembalikan payload JSON cuaca
    Note over ESP32: Parsing JSON (Lokasi, Waktu, Suhu, Kelembaban, Cuaca)
    ESP32-->>Serial Monitor: Menampilkan log cuaca
    deactivate ESP32

    User->>Browser: Klik kode lokasi (/cuaca link)
    Browser->>ESP32: GET /cuaca
    activate ESP32
    ESP32->>Beeceptor: POST payload cuaca dalam format JSON
    Beeceptor-->>ESP32: Mengembalikan echo response
    ESP32-->>Serial Monitor: Menampilkan response Beeceptor
    deactivate ESP32
````

---

## Detail Project

Proyek ini adalah kombinasi antara web server lokal dan API gateway yang berjalan pada ESP32:

1. **Menyediakan Portal Lokal:** ESP32 membuat web server lokal pada port 80 dan menampilkan halaman pencarian interaktif yang disimpan di `LittleFS`.

2. **Pencarian Wilayah Administratif di Sisi Client:** Halaman web secara dinamis mengunduh dan memproses file CSV wilayah administratif resmi Permendagri. Pengguna dapat mencari seluruh provinsi, kabupaten/kota, kecamatan, dan desa/kelurahan di Indonesia dengan fitur autocomplete berkecepatan tinggi.

3. **Integrasi Cuaca BMKG:** Ketika lokasi dipilih, kode administratif dikirim ke ESP32 melalui request `POST`. ESP32 kemudian melakukan query ke API resmi BMKG menggunakan kode tersebut, melakukan parsing terhadap payload JSON yang diterima, lalu menampilkan parameter real-time seperti status cuaca, suhu, kelembaban, dan timestamp ke Serial Monitor.

4. **Demonstrasi HTTP POST Keluar:** Ketika link lokasi yang dihasilkan diklik, ESP32 akan memformat detail cuaca yang sudah diambil menjadi payload JSON dan mengirimkannya melalui `HTTP POST` ke `https://echo.free.beeceptor.com`, kemudian menampilkan respons konfirmasi dari echo server pada koneksi serial.

---

## Keunggulan

* **Tanpa Beban Database pada Mikrokontroler:** Query database wilayah administratif yang besar, berisi puluhan ribu lokasi di Indonesia, dilakukan sepenuhnya di browser menggunakan parsing CSV. Hal ini menjaga konsumsi memori ESP32 tetap minimal.

* **Data API Pemerintah Real-Time:** Mengambil prakiraan cuaca resmi secara langsung dari badan meteorologi pemerintah, yaitu BMKG, secara real-time.

* **Model Jaringan Ganda:** ESP32 berfungsi sebagai **HTTP Server** untuk menangani aksi client lokal dan juga sebagai **HTTP Client** untuk melakukan query ke API BMKG serta mengirim metadata ke endpoint mock.

* **Penyimpanan Flash Lokal:** Asset HTML dipisahkan dengan rapi dari source code mikrokontroler dan disajikan langsung dari partisi flash menggunakan sistem file **LittleFS**.

---

## Kebutuhan Modul

Untuk membuat proyek ini, kamu membutuhkan komponen hardware dan dependency software berikut:

### Kebutuhan Hardware

| Modul / Part                | Deskripsi                                                | Fungsi                                                  |
| :-------------------------- | :------------------------------------------------------- | :------------------------------------------------------ |
| **ESP32 Development Board** | NodeMCU ESP32, ESP32-WROOM-32D, atau board setara.       | Mikrokontroler utama dengan modul Wi-Fi.                |
| **Kabel Micro-USB**         | Kabel USB-A ke Micro-USB yang mendukung transfer data.   | Untuk pemrograman dan komunikasi serial.                |
| **Jaringan Wi-Fi**          | Access point Wi-Fi lokal 2.4 GHz, tidak mendukung 5 GHz. | Akses internet untuk API BMKG dan hosting server lokal. |

### 📚 Software & Library IDE

* **Arduino IDE** atau VS Code dengan PlatformIO
* **ESP32 Board Package** versi 2.x atau 3.x
* **Arduino_JSON Library** oleh Arduino, untuk parsing JSON yang lebih stabil
* **ESP32 WebServer Library**, sudah tersedia pada ESP32 board package
* **HTTPClient Library**, sudah tersedia pada ESP32 board package
* **LittleFS Library**, library sistem file yang sudah terintegrasi di framework ESP32

---

## Katalog Fitur

### 🌐 Web Server & Search Interface (`data/index.html`)

* **Dynamic Autocomplete Suggestion:** Input pencarian langsung dicocokkan dengan data wilayah Permendagri.
* **Category Badges:** Badge visual yang rapi untuk membedakan desa/kelurahan, kecamatan, kabupaten/kota, dan provinsi.
* **Asynchronous API Posting:** Mengirim kode wilayah yang dipilih ke ESP32 menggunakan vanilla JavaScript `XMLHttpRequest` tanpa melakukan refresh halaman.

### ⚙️ Firmware ESP32 Controller (`http-post.ino`)

* **Fungsi Parser BMKG (`APIdanKode`):** Mengubah respons JSON mentah dari BMKG menjadi nilai string menggunakan `Arduino_JSON`.

* **Pengirim JSON POST (`PostCuaca`):** Mengemas detail cuaca ke dalam string JSON standar:

````json
  {
    "Lokasi": "Desa, Kecamatan, Kota, Provinsi",
    "Jam": "YYYY-MM-DD HH:MM:SS",
    "cuaca": "Deskripsi Cuaca",
    "suhu": "Suhu",
    "kelembaban": "Kelembaban"
}
````


* **Status Endpoint Handler (`HandleRoot`):** Membaca file GUI HTML dari penyimpanan lokal `LittleFS` dan mengirimkannya ke client.

---

## Tech Stack

* **Platform Firmware:** C++ dengan Arduino Core
* **Interface Penyimpanan:** LittleFS atau Lightweight Flash File System
* **Bahasa Frontend:** HTML5, CSS3, JavaScript ES6, Fetch API, AJAX
* **API & Sumber Data:**

  * [BMKG Public Weather API](https://api.bmkg.go.id)
  * [Database Wilayah Permendagri](https://github.com/kodewilayah/permendagri-72-2019)
  * [Beeceptor Mock Server](https://beeceptor.com)

---

## Build dan Install

### Step 1: Clone dan Siapkan Workspace

1. Clone repository ini ke komputer lokal.
2. Buka sketch `http-post.ino` di Arduino IDE.

### Step 2: Konfigurasi Kredensial Wi-Fi

Sebelum upload, ubah kredensial Wi-Fi di `http-post.ino` agar sesuai dengan jaringan lokal kamu:

````cpp
// Line 103
WiFi.begin("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD");
````

### Step 3: Install ESP32 LittleFS Tool

Karena UI HTML disimpan di direktori `data/`, kamu perlu menguploadnya ke flash memory ESP32.

* Install extension `arduino-littlefs-upload` dari Earl Philhower.
* Download file VSIX dari halaman [releases](https://github.com/earlephilhower/arduino-littlefs-upload/releases).
* Salin file VSIX ke `~/.arduinoIDE/plugins/` pada Mac dan Linux atau ke `C:\Users\<username>\.arduinoIDE\plugins\` pada Windows. Kamu mungkin perlu membuat folder tersebut secara manual terlebih dahulu.
* Restart Arduino IDE.

### Step 4: Upload Folder `data/` ke LittleFS

1. Hubungkan board ESP32 ke PC.
2. Buka Arduino IDE.
3. Pilih board ESP32 kamu, misalnya `ESP32 Dev Module`, lalu pilih COM/Serial port yang sesuai.
4. Tekan Ctrl+Shift+P atau Cmd+Shift+P pada Mac untuk membuka Command Palette.
5. Ketik `"Build LittleFS image in sketch directory"` lalu tekan Enter. Proses ini akan mengupload folder `data` ke partisi LittleFS ESP32.

### Step 4: Compile dan Upload

1. Hubungkan board ESP32 ke PC.
2. Pilih board ESP32 kamu, misalnya `ESP32 Dev Module`, lalu pilih COM/Serial port yang sesuai.
3. Verify dan compile sketch.
4. Klik **Upload** untuk menulis firmware ke ESP32.

### Step 5: Jalankan dan Uji

1. Buka **Serial Monitor** di Arduino IDE dan atur baud rate ke **`115200`**.
2. Reset board ESP32. ESP32 akan menampilkan IP address yang terhubung dan nama mDNS, misalnya:
   `Connected to WiFi. IP address: 192.168.1.15`
   dan
   `mDNS: http://esp32-wether.local`
3. Buka browser pada perangkat yang terhubung ke jaringan Wi-Fi yang sama, lalu akses `http://esp32-wether.local`.
4. Cari lokasi, pilih lokasi tersebut, lalu lihat Serial Monitor untuk memantau proses pengambilan dan output data cuaca BMKG.
5. Klik link yang dihasilkan untuk menjalankan demo POST.

---

## Kontribusi

Kontribusi membuat komunitas open-source menjadi tempat yang luar biasa untuk belajar, menginspirasi, dan berkarya. Setiap kontribusi yang kamu berikan akan sangat diapresiasi.

1. Fork proyek ini.
2. Buat branch fitur kamu: `git checkout -b feature/AmazingFeature`.
3. Commit perubahan kamu: `git commit -m 'Add some AmazingFeature'`.
4. Push ke branch tersebut: `git push origin feature/AmazingFeature`.
5. Buka Pull Request.

