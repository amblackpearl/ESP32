#include <WiFi.h>
#include <WebServer.h>
#include <Arduino_JSON.h>
#include <HTTPClient.h>

WebServer server(80);

String kodeLokasi = ""; // Variabel untuk menyimpan kode yang diterima
String cuacaLokasi = "";
String suhuLokasi= "";
String humLokasi = "";
String Lokasi = "";
String jam = "";
bool kodeDiterima = false; // Flag untuk menandakan kapan kode diterima

void HandleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html>
<html lang="id">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Pencarian Lokasi</title>
</head>
<style>
        body {
            font-family: 'Arial', sans-serif;
            background-color: #f4f4f9;
            margin: 0;
            padding: 20px;
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        h1 {
            color: #333;
            margin-bottom: 20px;
        }
        input[type="text"] {
            width: 100%;
            max-width: 600px;
            padding: 10px;
            border: 2px solid #007BFF;
            border-radius: 5px;
            font-size: 16px;
            transition: border 0.3s;
        }
        input[type="text"]:focus {
            border-color: #0056b3;
            outline: none;
        }
        #suggestions {
            width: 100%;
            max-width: 600px;
            background: white;
            border: 1px solid #ccc;
            border-radius: 5px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            margin-top: 5px;
            display: none; /* Hide by default */
        }
        .suggestion {
            padding: 10px;
            cursor: pointer;
            display: flex;
            flex-direction: column;
            border-bottom: 1px solid #eaeaea;
            transition: background 0.2s;
        }
        .suggestion:hover {
            background: #e6f7ff;
        }
        .suggestion-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            color: #007BFF;
            font-weight: bold;
        }
        .location-category {
            font-size: 0.9em;
            color: #666;
            font-weight: normal;
            margin-left: 10px;
            background-color: #e0f7fa;
            padding: 3px 5px;
            border-radius: 5px;
        }
        .location-detail {
            font-size: 0.9em;
            color: #555;
            margin-top: 5px;
        }
        #result {
            margin-top: 20px;
            padding: 15px;
            border: 1px solid #007BFF;
            border-radius: 5px;
            background: #e7f3ff;
            width: 100%;
            max-width: 600px;
            display: none; /* Hide by default */
        }
        #hasil {
            color: #007BFF;
            text-decoration: none;
        }
        #hasil:hover {
            text-decoration: underline;
        }
    </style>
<body>
    <h1>Cari Lokasi</h1>
    <input type="text" id="searchInput" placeholder="Masukkan nama lokasi..." onkeyup="showSuggestions()">
    <div id="suggestions"></div>
    <div id="result">
        <a id="hasil" href="/cuaca"></a> <!-- Link untuk hasil -->
    </div>

    <script>
        let locations = [];
        let districts = [];
        let subdistricts = [];
        let provinces = [];

        async function fetchData() {
            const response = await fetch('https://raw.githubusercontent.com/kodewilayah/permendagri-72-2019/refs/heads/main/dist/base.csv');
            const data = await response.text();
            parseCSV(data);
        }

        function parseCSV(data) {
            const rows = data.trim().split('\n').map(line => line.split(','));

            rows.forEach(row => {
                const code = row[0];
                const name = row[1];

                if (code.length === 2) {
                    provinces.push({ code, name });
                } else if (code.length === 5) {
                    districts.push({ code, name });
                } else if (code.length === 8) {
                    const districtCode = code.slice(0, 5);
                    const district = districts.find(d => d.code === districtCode);
                    subdistricts.push({ code, name, district: district ? district.name : 'Unknown' });
                } else if (code.length > 8) {
                    const subdistrictCode = code.slice(0, 8);
                    const subdistrict = subdistricts.find(s => s.code === subdistrictCode);
                    const districtCode = subdistrictCode.slice(0, 5);
                    const district = districts.find(d => d.code === districtCode);
                    const provinceCode = districtCode.slice(0, 2);
                    const province = provinces.find(p => p.code === provinceCode);

                    locations.push({
                        code,
                        name,
                        subdistrict: subdistrict ? subdistrict.name : 'Unknown',
                        district: district ? district.name : 'Unknown',
                        province: province ? province.name : 'Unknown',
                    });
                }
            });
        }

        function showSuggestions() {
            const input = document.getElementById('searchInput').value.toLowerCase();
            const suggestionsDiv = document.getElementById('suggestions');
            const resultDiv = document.getElementById('result');
            const hasilAnchor = document.getElementById('hasil');

            suggestionsDiv.innerHTML = ''; 
            resultDiv.style.display = 'none'; 
            hasilAnchor.textContent = ''; 

            if (input) {
                suggestionsDiv.style.display = 'block'; 

                const filteredSuggestions = [];

                provinces.forEach(province => {
                    if (province.name.toLowerCase().includes(input)) {
                        filteredSuggestions.push({
                            code: province.code,
                            name: province.name,
                            category: 'Provinsi',
                            details: ''
                        });
                    }
                });

                districts.forEach(district => {
                    if (district.name.toLowerCase().includes(input)) {
                        const province = provinces.find(p => p.code === district.code.slice(0, 2));
                        filteredSuggestions.push({
                            code: district.code,
                            name: district.name,
                            category: 'Kabupaten/Kota',
                            details: `Provinsi: ${province ? province.name : 'Unknown'}`
                        });
                    }
                });

                subdistricts.forEach(subdistrict => {
                    if (subdistrict.name.toLowerCase().includes(input)) {
                        const district = districts.find(d => d.code === subdistrict.code.slice(0, 5));
                        const province = provinces.find(p => p.code === district.code.slice(0, 2));
                        filteredSuggestions.push({
                            code: subdistrict.code,
                            name: subdistrict.name,
                            category: 'Kecamatan',
                            details: `Kabupaten/Kota: ${district ? district.name : 'Unknown'}, Provinsi: ${province ? province.name : 'Unknown'}`
                        });
                    }
                });

                locations.forEach(location => {
                    if (location.name.toLowerCase().includes(input)) {
                        const district = districts.find(d => d.code === location.code.slice(0, 5));
                        const province = provinces.find(p => p.code === district.code.slice(0, 2));
                        filteredSuggestions.push({
                            code: location.code,
                            name: location.name,
                            category: 'Desa/Kelurahan',
                            details: `Kecamatan: ${location.subdistrict}, Kabupaten/Kota: ${district ? district.name : 'Unknown'}, Provinsi: ${province ? province.name : 'Unknown'}`
                        });
                    }
                });

                filteredSuggestions.forEach(suggestion => {
                    const div = document.createElement('div');
                    div.classList.add('suggestion');

                    const header = document.createElement('div');
                    header.classList.add('suggestion-header');
                    header.innerHTML = `${suggestion.name} <span class="location-category">${suggestion.category}</span>`;
                    
                    const detailDiv = document.createElement('div');
                    detailDiv.classList.add('location-detail');
                    detailDiv.textContent = suggestion.details;
                    
                    div.onclick = () => {
                        showResult(suggestion.code);
                        suggestionsDiv.innerHTML = ''; 
                        suggestionsDiv.style.display = 'none'; 
                    };
                    
                    div.appendChild(header);
                    div.appendChild(detailDiv);
                    suggestionsDiv.appendChild(div);
                });
            } else {
                suggestionsDiv.style.display = 'none'; 
            }
        }

        function showResult(code) {
            const resultDiv = document.getElementById('result');
            const hasilAnchor = document.getElementById('hasil');

            resultDiv.style.display = 'block'; 
            hasilAnchor.textContent = code; 
            hasilAnchor.href = "/cuaca"; 

            sendToESP32(code); // Send the code to the ESP32 server
        }

        function sendToESP32(code) {
            const xhr = new XMLHttpRequest();
            xhr.open("POST", "/kode", true); // ESP32 will handle this route
            xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
            xhr.send("kode=" + encodeURIComponent(code)); // Send the code as part of POST data
        }

        window.onload = fetchData;
    </script>
</body>
</html>

  )rawliteral";
  // Kirim konten HTML sebagai respons kepada klien
  server.send(200, "text/html", html); // Mengirim HTML
}
 void HandleKode() {
  if (server.hasArg("kode")) {
    kodeLokasi = server.arg("kode"); // Ambil kode yang dikirim dari AJAX
    APIdanKode(); // Panggil fungsi untuk memproses kode
  }

}
void APIdanKode() {
  HTTPClient http;
    http.begin("https://api.bmkg.go.id/publik/prakiraan-cuaca?adm4=" + kodeLokasi);
    int httpResponCode = http.GET();
    Serial.print("HTTP Respon Code : ");
    Serial.println(httpResponCode);
    if(httpResponCode > 0){
      String payload = http.getString();
      JSONVar myObject = JSON.parse(payload);
      if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }

      cuacaLokasi = String((const char*)myObject["data"][0]["cuaca"][0][0]["weather_desc"]);

      Lokasi = String((const char*)myObject["lokasi"]["desa"]) + ", " + 
                    String((const char*)myObject["lokasi"]["kecamatan"]) + ", " + 
                    String((const char*)myObject["lokasi"]["kota"]) + ", " + 
                    String((const char*)myObject["lokasi"]["provinsi"]);
                    jam = (const char*)myObject["data"][0]["cuaca"][0][0]["local_datetime"];
suhuLokasi = String((int)myObject["data"][0]["cuaca"][0][0]["t"]);
humLokasi = String((int)myObject["data"][0]["cuaca"][0][0]["hu"]);

      Serial.println("Lokasi     : " + Lokasi);
      Serial.println("Jam        : " + jam);
      Serial.println("Cuaca      : " + cuacaLokasi);
      Serial.println("Suhu       : " + suhuLokasi);
      Serial.println("Kelembaban : " + humLokasi);


    }else{
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
      String httpRequestData = "{\"Lokasi\":\"" + Lokasi + "\", \"Jam\":\"" + jam + "\", \"cuaca\":\"" + cuacaLokasi + "\", \"suhu\":\"" + suhuLokasi + "\", \"kelembaban\":\"" + humLokasi + "\"}";
  
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
WiFi.begin("Gelora", "maemosik"); //nama wifi & password yang ada di warkop tempat saya mengerjakan tugas
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP()); //menampilkan gateaway jaringan wifi yang tersambung ke esp32

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
    APIdanKode(); // Panggil fungsi untuk memproses kode dan API
    kodeDiterima = false; // Reset flag setelah kode diproses
  }
  
}
