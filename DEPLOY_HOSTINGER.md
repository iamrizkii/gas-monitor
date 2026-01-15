# Panduan Deploy Laravel ke Hostinger

## 📋 Persiapan

Sebelum deploy, pastikan:
1. Punya akun Hostinger dengan paket hosting
2. Domain sudah ada (atau pakai subdomain gratis Hostinger)
3. File project Laravel (`gas-monitor`) sudah siap

---

## 🚀 Langkah Deploy

### 1. Login ke Hostinger

1. Buka https://hpanel.hostinger.com
2. Login dengan akun Hostinger

### 2. Buat Database MySQL

1. Di hPanel, klik **Databases** → **MySQL Databases**
2. Buat database baru:
   - Database name: `gas_monitor`
   - Username: `gas_user`
   - Password: (buat password kuat)
3. Catat informasi database!

### 3. Upload File Laravel

**Opsi A: Via File Manager**
1. Di hPanel, klik **Files** → **File Manager**
2. Masuk ke folder `public_html`
3. Upload semua file dari `gas-monitor` (kecuali folder `vendor`)
4. Atau upload file ZIP lalu extract

**Opsi B: Via FTP**
1. Di hPanel, klik **Files** → **FTP Accounts**
2. Catat kredensial FTP
3. Upload via FileZilla atau FTP client lain

### 4. Konfigurasi .env

1. Di File Manager, buka file `.env`
2. Ubah konfigurasi:

```env
APP_NAME="Gas Monitor"
APP_ENV=production
APP_DEBUG=false
APP_URL=https://yourdomain.com

DB_CONNECTION=mysql
DB_HOST=localhost
DB_PORT=3306
DB_DATABASE=u123456789_gas_monitor
DB_USERNAME=u123456789_gas_user
DB_PASSWORD=yourpassword
```

### 5. Install Dependencies

1. Di hPanel, klik **Advanced** → **SSH Access**
2. Connect via SSH atau gunakan Terminal di hPanel
3. Jalankan:

```bash
cd public_html
composer install --no-dev --optimize-autoloader
php artisan key:generate
php artisan migrate --seed
php artisan config:cache
php artisan route:cache
```

### 6. Setup Public Folder

Laravel perlu redirect ke folder `public`. Di Hostinger:

1. Buat file `.htaccess` di root `public_html`:

```apache
<IfModule mod_rewrite.c>
    RewriteEngine On
    RewriteRule ^(.*)$ public/$1 [L]
</IfModule>
```

### 7. Test Akses

Buka browser: `https://yourdomain.com`

---

## ⚠️ Troubleshooting

| Masalah | Solusi |
|---------|--------|
| 500 Error | Cek file `.env` dan permissions |
| Database error | Pastikan kredensial DB benar |
| Blank page | Jalankan `php artisan config:clear` |
| Assets tidak load | Jalankan `php artisan storage:link` |

---

## 🔗 URL untuk ESP32

Setelah deploy, update `SERVER_URL` di kode ESP32:

```cpp
const char* SERVER_URL = "https://yourdomain.com";
```

---

## 📱 PWA Access

Setelah online, PWA bisa diakses dari mana saja:
- `https://yourdomain.com` (dari HP atau laptop manapun)
