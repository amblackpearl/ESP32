jadi kode esp32 diatas ituu untuk melakukan http.GET request dan http.POST
untuk GET menggunakan API prakiraan cuaca BMKG dengan kode wilayah yang disesuaikan
karena ingin efisiensi saya membuat tampilan webserver yang bia mencari semua kode wilayah di seluruh indonesia termasuk desa/kelurahan, kecamatan, kota/kabupaten, dan provinsi
sehingga ketika sudah memilih kode wlayah di web server esp32 ini, nantinya data lokasi, cuaca, jam, suhu, dan kelembab an wilayah yang dipilih akan muncul di serial monitor
Selanjutnya ketika kode wilayah di webserver ditekan maka akan langsung mentriger esp32 untuk melakukan http.POST ke server
post yang dilakukan saya atur untuk mengirim data json, ketika sudah terkirim maka respon dari server dalam bentuk string akan dikirim ke serial monitor
