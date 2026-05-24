#include <Arduino_JSON.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>

WebServer server(80);
const char *mdnsname = "esp32-wether";
const char *ssid = "Gelora";
const char *password = "maemosik";

String kodeLokasi = ""; // Variabel untuk menyimpan kode yang diterima
String cuacaLokasi = "";
String suhuLokasi = "";
String humLokasi = "";
String Lokasi = "";
String jam = "";
bool kodeDiterima = false; // Flag untuk menandakan kapan kode diterima

void HandleRoot() {
  // Baca file HTML dari LittleFS
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    server.send(500, "text/plain", "Gagal membuka index.html");
    return;
  }

  String html = file.readString();
  file.close();

  // Kirim konten HTML sebagai respons kepada klien
  server.send(200, "text/html", html);
}
void HandleKode() {
  if (server.hasArg("kode")) {
    kodeLokasi = server.arg("kode"); // Ambil kode yang dikirim dari AJAX
    APIdanKode();                    // Panggil fungsi untuk memproses kode
  }
}
void APIdanKode() {
  HTTPClient http;
  http.begin("https://api.bmkg.go.id/publik/prakiraan-cuaca?adm4=" +
             kodeLokasi);
  int httpResponCode = http.GET();
  Serial.print("HTTP Respon Code : ");
  Serial.println(httpResponCode);
  if (httpResponCode > 0) {
    String payload = http.getString();
    JSONVar myObject = JSON.parse(payload);
    if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing input failed!");
      return;
    }

    cuacaLokasi = String(
        (const char *)myObject["data"][0]["cuaca"][0][0]["weather_desc"]);

    Lokasi = String((const char *)myObject["lokasi"]["desa"]) + ", " +
             String((const char *)myObject["lokasi"]["kecamatan"]) + ", " +
             String((const char *)myObject["lokasi"]["kota"]) + ", " +
             String((const char *)myObject["lokasi"]["provinsi"]);
    jam = (const char *)myObject["data"][0]["cuaca"][0][0]["local_datetime"];
    suhuLokasi = String((int)myObject["data"][0]["cuaca"][0][0]["t"]);
    humLokasi = String((int)myObject["data"][0]["cuaca"][0][0]["hu"]);

    Serial.println("Lokasi     : " + Lokasi);
    Serial.println("Jam        : " + jam);
    Serial.println("Cuaca      : " + cuacaLokasi);
    Serial.println("Suhu       : " + suhuLokasi);
    Serial.println("Kelembaban : " + humLokasi);

  } else {
    Serial.println("error, ") + httpResponCode;
  }
}
void PostCuaca() {
  HTTPClient http;

  // Your Domain name with URL path or IP address with path
  http.begin("https://echo.free.beeceptor.com");

  // Specify content-type header
  http.addHeader("Content-Type", "application/json");
  // Data to send with HTTP POST
  String httpRequestData = "{\"Lokasi\":\"" + Lokasi + "\", \"Jam\":\"" + jam +
                           "\", \"cuaca\":\"" + cuacaLokasi +
                           "\", \"suhu\":\"" + suhuLokasi +
                           "\", \"kelembaban\":\"" + humLokasi + "\"}";

  // Send HTTP POST request
  int httpResponseCode = http.POST(httpRequestData);
  if (httpResponseCode > 0) {
    String postResponse = http.getString();
    Serial.println("Response from server: " + postResponse);
  } else {
    Serial.print("Error sending POST request");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void setup() {
  Serial.begin(115200);

  // Menghubungkan ke WiFi
  WiFi.begin(ssid, password); // nama wifi & password yang ada di warkop
                              // tempat saya mengerjakan tugas
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  if (MDNS.begin(mdnsname)) {
    MDNS.addService("http", "tcp", 80);
    Serial.print("mDNS: http://");
    Serial.print(mdnsname);
    Serial.println(".local");
  }
  Serial.println("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP()); // menampilkan gateaway jaringan wifi yang
                                  // tersambung ke esp32

  // Mengatur route untuk server
  server.on("/", HandleRoot);
  server.on("/kode", HTTP_POST, HandleKode);
  server.on("/cuaca", PostCuaca);

  // Memulai server
  server.begin();
}

void loop() {
  server.handleClient();
  if (kodeDiterima) {
    APIdanKode();         // Panggil fungsi untuk memproses kode dan API
    kodeDiterima = false; // Reset flag setelah kode diproses
  }
}