# Gas Monitor PWA - Sistem Pendeteksi Kebocoran Gas LPG

Sistem monitoring dan kontrol alat pendeteksi kebocoran gas LPG berbasis PWA (Progressive Web App) menggunakan Laravel, menggantikan Blynk.

## 📋 Fitur

- ✅ Monitoring kadar gas secara real-time (gauge + chart)
- ✅ Mode Auto/Manual
- ✅ Kontrol valve ON/OFF
- ✅ Push notification saat kebocoran terdeteksi
- ✅ Installable di HP (PWA)
- ✅ Riwayat data gas (6H, 12H, 24H)

## 🔧 Prasyarat

1. **Laragon** (sudah terinstall dengan PHP 8.2+ dan MySQL)
2. **Composer** (biasanya sudah include di Laragon)
3. **Arduino IDE** (untuk upload kode ke ESP32)

## 🚀 Instalasi

### 1. Setup Database

Buka **HeidiSQL** (di Laragon → Database) dan buat database baru:

```sql
CREATE DATABASE gas_monitor;
```

### 2. Konfigurasi Environment

Copy file `.env.example` menjadi `.env`:

```bash
cd C:\laragon\www\amir\gas-monitor
copy .env.example .env
```

Edit file `.env` dan ubah konfigurasi database:

```env
APP_NAME="Gas Monitor"
APP_URL=http://gas-monitor.test

DB_CONNECTION=mysql
DB_HOST=127.0.0.1
DB_PORT=3306
DB_DATABASE=gas_monitor
DB_USERNAME=root
DB_PASSWORD=
```

### 3. Generate App Key

```bash
php artisan key:generate
```

### 4. Jalankan Migrasi & Seeder

```bash
php artisan migrate --seed
```

### 5. Akses Aplikasi

Buka browser dan akses:
- **Via Laragon**: http://gas-monitor.test
- **Via artisan**: `php artisan serve` → http://localhost:8000

## 📱 Setup ESP32

### 1. Install Library

Di Arduino IDE, install library berikut via Library Manager:
- **ArduinoJson** by Benoit Blanchon
- **ESP32Servo** by Kevin Harrington

### 2. Edit Kode ESP32

Buka file `ESP32_GasMonitor_PWA.ino` dan ubah:

```cpp
// Ganti dengan IP laptop Anda
const char* SERVER_IP = "192.168.1.100";  // Cek dengan: ipconfig
```

**Cara cek IP laptop:**
1. Buka CMD
2. Ketik `ipconfig`
3. Cari "IPv4 Address" di bagian WiFi (contoh: 192.168.1.100)

### 3. Upload ke ESP32

1. Pilih Board: **ESP32 Dev Module**
2. Pilih Port: **COM X** (sesuai ESP32 Anda)
3. Upload!

## 📡 Arsitektur Komunikasi

```
ESP32 (Sensor)          Laravel Server              PWA (HP)
     │                       │                         │
     ├──POST /api/device/data───>│                         │
     │   {gas, voltage, alert}   │                         │
     │                       │<──GET /api/readings/latest──┤
     │<──GET /api/device/commands│                         │
     │                       │<──POST /api/control/mode────┤
     │                       │<──POST /api/control/valve───┤
     │──POST /api/command-ack───>│                         │
```

## 🔌 Wiring ESP32

| Komponen | Pin ESP32 | Keterangan |
|----------|-----------|------------|
| Sensor TGS2610 | GPIO 36 | Analog input |
| Motor Servo | GPIO 23 | PWM output |
| Buzzer | GPIO 15 | Active LOW |
| Relay (Fan) | GPIO 2 | Output |
| SSR (Valve) | GPIO 22 | Output |

## 📂 Struktur Project

```
gas-monitor/
├── app/
│   ├── Models/
│   │   ├── GasReading.php
│   │   ├── DeviceCommand.php
│   │   └── DeviceSetting.php
│   └── Http/Controllers/
│       ├── Api/DeviceController.php
│       └── DashboardController.php
├── database/migrations/
├── resources/views/
│   └── dashboard.blade.php
├── public/
│   ├── manifest.json
│   ├── sw.js
│   └── icons/
├── routes/
│   ├── api.php
│   └── web.php
└── ESP32_GasMonitor_PWA.ino
```

## 🧪 Testing API

### Kirim Data (simulasi ESP32)
```bash
curl -X POST http://gas-monitor.test/api/device/data ^
  -H "Content-Type: application/json" ^
  -d "{\"gas\": 25, \"voltage\": 1.5, \"alert\": false}"
```

### Ambil Data Terbaru
```bash
curl http://gas-monitor.test/api/readings/latest
```

## ❓ Troubleshooting

### ESP32 tidak terhubung ke server
1. Pastikan ESP32 dan laptop di jaringan WiFi yang sama
2. Pastikan IP address benar di kode ESP32
3. Pastikan Laragon running (Apache hijau)

### PWA tidak bisa diinstall
1. Buka via HTTPS atau localhost
2. Pastikan manifest.json bisa diakses
3. Coba clear cache browser

### Push notification tidak muncul
1. Klik tombol 🔔 di dashboard
2. Izinkan notifikasi di browser
3. Pastikan browser support Push API

## 📝 License

MIT License - Free to use for educational purposes.

---

**Dibuat untuk Skripsi: Alat Pendeteksi Kebocoran Gas LPG Berbasis IoT**
