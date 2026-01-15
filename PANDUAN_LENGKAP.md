# Panduan Lengkap Gas Monitor PWA

## Daftar Isi
1. [Test Lokal (Tanpa Hosting)](#-cara-test-lokal-tanpa-hosting)
2. [Checklist Setelah Alat Jadi](#-checklist-lengkap-untuk-amir-setelah-alat-jadi)
3. [Wiring Diagram](#-wiring-diagram)
4. [Troubleshooting](#-troubleshooting)

---

# ✅ Cara Test Lokal (Tanpa Hosting)

Test semua fitur di laptop dulu sebelum deploy ke hosting.

## Persiapan

### 1. Pastikan Database Sudah Ready
```bash
cd C:\laragon\www\amir\gas-monitor
php artisan migrate --seed
```

### 2. Jalankan Server Laravel
```bash
php artisan serve --host=0.0.0.0 --port=8000
```

### 3. Cek IP Laptop
```bash
ipconfig
```
Catat **IPv4 Address** (contoh: `172.20.10.8`)

### 4. Edit Kode ESP32

Buka file: `ESP32_GasMonitor_PWA.ino`

Ubah bagian ini:
```cpp
const char* SERVER_IP = "172.20.10.8";  // Ganti dengan IP laptop
const char* ssid = "SKRIPSI";            // Nama WiFi
const char* pass = "Monitor2025";        // Password WiFi
```

### 5. Upload ke ESP32
- Buka Arduino IDE
- Pilih Board: **ESP32 Dev Module**
- Pilih Port: **COM X**
- Klik Upload

### 6. Test PWA
- Laptop: `http://localhost:8000`
- HP: `http://172.20.10.8:8000` (HP & laptop harus di WiFi sama)

---

## Test Simulasi (Tanpa Alat)

Jika alat belum jadi, bisa test PWA dengan simulasi data via PowerShell:

```powershell
# Gas aman (10%)
Invoke-WebRequest -Uri "http://localhost:8000/api/device/data" -Method POST -ContentType "application/json" -Body '{"gas": 10, "voltage": 0.8, "alert": false}'

# Gas bahaya (85%) + Alert
Invoke-WebRequest -Uri "http://localhost:8000/api/device/data" -Method POST -ContentType "application/json" -Body '{"gas": 85, "voltage": 2.5, "alert": true}'
```

---

# 📋 Checklist Lengkap untuk Amir (Setelah Alat Jadi)

## ⚡ FASE 1: Test Lokal (WiFi Only)

### Hardware
- [ ] Wiring sensor TGS2610 ke ESP32
- [ ] Wiring servo, buzzer, relay, solenoid valve
- [ ] Sambungkan ESP32 ke laptop via USB

### Software
- [ ] Install library di Arduino IDE:
  - [ ] ArduinoJson
  - [ ] ESP32Servo
- [ ] Edit `ESP32_GasMonitor_PWA.ino` (ganti IP laptop)
- [ ] Upload kode ke ESP32
- [ ] Jalankan `php artisan serve --host=0.0.0.0 --port=8000`

### Testing Lokal
- [ ] Buka Serial Monitor, pastikan ESP32 konek WiFi
- [ ] Buka PWA, pastikan gauge menampilkan data
- [ ] Test Mode Auto/Manual
- [ ] Test toggle Komponen ON/OFF (di mode Manual)
- [ ] Test alert dengan mendekatkan gas ke sensor
- [ ] Test notifikasi di HP

---

## ⚡ FASE 2: Deploy Hosting

### Deploy Laravel
- [ ] Upload folder `gas-monitor` ke Hostinger
- [ ] Buat database MySQL di Hostinger
- [ ] Edit `.env` dengan kredensial Hostinger
- [ ] Jalankan migration: `php artisan migrate --seed`
- [ ] Catat URL domain (contoh: `https://gas-monitor.site`)

### Update Kode ESP32
- [ ] Install library tambahan: **TinyGSM**
- [ ] Buka file `ESP32_GasMonitor_DualMode.ino`
- [ ] Ganti `SERVER_URL` dengan URL hosting
- [ ] Upload ulang ke ESP32

### Testing Online
- [ ] Buka PWA dari HP (via 4G, bukan WiFi)
- [ ] Pastikan data dari ESP32 sampai ke PWA

---

## ⚡ FASE 3: Setup SIM800L (GSM Backup)

### Hardware Tambahan
- [ ] Wiring SIM800L ke ESP32 (lihat tabel di bawah)
- [ ] Pasang antena GSM
- [ ] Masukkan SIM card Indosat dengan kuota data

### Testing Dual Mode
- [ ] Upload `ESP32_GasMonitor_DualMode.ino`
- [ ] Buka Serial Monitor
- [ ] Test 1: WiFi ON → ESP32 konek via WiFi ✓
- [ ] Test 2: Matikan WiFi → ESP32 switch ke GSM ✓
- [ ] Test 3: Nyalakan WiFi lagi → ESP32 kembali ke WiFi ✓

---

# 🔌 Wiring Diagram

## ESP32 + Sensor + Aktuator

| Komponen | Pin ESP32 | Keterangan |
|----------|-----------|------------|
| Sensor TGS2610 | GPIO36 | Analog input |
| Motor Servo | GPIO23 | PWM |
| Buzzer | GPIO15 | Active LOW |
| Relay (Fan) | GPIO2 | Output |
| SSR (Valve) | GPIO22 | Output |

## SIM800L (Untuk Dual Mode)

| SIM800L | ESP32 | ⚠️ Penting |
|---------|-------|-----------|
| VCC | **4.2V Stepdown** | JANGAN pakai 5V! |
| GND | GND | |
| TXD | GPIO16 | |
| RXD | GPIO17 | |
| RST | GPIO4 | Optional |

---

# 🔧 Troubleshooting

| Masalah | Solusi |
|---------|--------|
| ESP32 tidak konek WiFi | Cek nama & password WiFi di kode |
| PWA tidak dapat data | Pastikan IP laptop benar, server jalan |
| Gauge tidak berubah | Refresh browser, cek Serial Monitor |
| Notifikasi tidak bunyi | Klik 🔔, izinkan notifikasi di browser |
| SIM800L tidak konek | Cek wiring VCC (harus 4.2V), cek SIM card |
| GSM registrasi gagal | Pastikan SIM aktif & ada kuota |

---

# 📁 Daftar File Penting

| File | Fungsi |
|------|--------|
| `ESP32_GasMonitor_PWA.ino` | Kode ESP32 (WiFi only) - untuk test lokal |
| `ESP32_GasMonitor_DualMode.ino` | Kode ESP32 (WiFi + GSM) - untuk produksi |
| `DEPLOY_HOSTINGER.md` | Panduan deploy ke Hostinger |
| `README.md` | Dokumentasi project |

---

# ✅ Checklist Final

Sebelum demo/presentasi, pastikan:

- [ ] PWA bisa diakses dari internet
- [ ] ESP32 bisa konek WiFi DAN GSM
- [ ] Sensor mendeteksi gas dengan benar
- [ ] Buzzer & fan aktif saat alert
- [ ] Servo & valve bekerja di mode Auto
- [ ] Notifikasi muncul di HP saat alert
- [ ] PWA bisa diinstall di HP
