/*
 * ESP32 Gas Monitor - Dual Mode (WiFi + GSM SIM800L)
 * 
 * Mode: WiFi sebagai koneksi utama, GSM sebagai backup
 * Jika WiFi gagal/mati → otomatis switch ke GSM
 * 
 * Hardware:
 * - ESP32 Dev Module
 * - SIM800L GSM Module
 * - Sensor TGS2610
 * - Motor Servo, Solenoid Valve, Buzzer, Relay
 */

// ============== LIBRARY ==============
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

// Library untuk SIM800L
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>

// ============== KONFIGURASI SERVER ==============
// GANTI dengan URL hosting Anda!
const char* SERVER_URL = "https://your-domain.hostinger.com";  // URL hosting

// ============== KONFIGURASI WIFI ==============
const char* WIFI_SSID = "SKRIPSI";
const char* WIFI_PASS = "Monitor2025";

// ============== KONFIGURASI GSM (Indosat) ==============
const char* APN = "indosatgprs";      // APN Indosat
const char* GSM_USER = "";             // Kosongkan
const char* GSM_PASS = "";             // Kosongkan

// ============== PIN SIM800L ==============
#define SIM800L_RX  16    // GPIO16 → SIM800L TXD
#define SIM800L_TX  17    // GPIO17 → SIM800L RXD
#define SIM800L_RST 4     // GPIO4  → SIM800L RST (optional)

// ============== PIN SENSOR & AKTUATOR ==============
const int sensorGas = 36;   // GPIO36 - Sensor TGS2610
const int servoPin = 23;    // GPIO23 - Motor Servo
const int pinBuzz = 15;     // GPIO15 - Buzzer
const int pinRelay = 2;     // GPIO2  - Relay (Fan)
const int pinValve = 22;    // GPIO22 - SSR (Valve)

// ============== CONSTANTS ==============
#define REF_VOLTAGE    3.3
#define ADC_RESOLUTION 4096.0
#define SERVO_TUTUP    0
#define SERVO_BUKA     180
#define GAS_THRESHOLD  2.0

// ============== OBJECTS ==============
Servo servo1;
HardwareSerial SerialGSM(1);  // Serial1 untuk SIM800L
TinyGsm modem(SerialGSM);
TinyGsmClient gsmClient(modem);
HTTPClient http;

// ============== CONNECTION MODE ==============
enum ConnectionMode {
    MODE_WIFI,
    MODE_GSM,
    MODE_NONE
};

ConnectionMode currentMode = MODE_NONE;

// ============== VARIABLES ==============
bool statusNotif = false;
bool modeAuto = true;
bool valveOn = true;
unsigned long lastSend = 0;
unsigned long lastCommandCheck = 0;
unsigned long lastConnectionCheck = 0;

// ============== FUNCTION PROTOTYPES ==============
bool connectWiFi();
bool connectGSM();
bool checkConnection();
void sendDataToServer(int gasPercent, float voltage, bool alert);
void checkServerCommands();
void executeCommand(String command);
void setValve(bool on);
void buzzfanOn();
void buzzfanOff();
String httpRequest(String endpoint, String method, String payload = "");

// ============== SETUP ==============
void setup() {
    Serial.begin(115200);
    Serial.println("\n=================================");
    Serial.println("Gas Monitor - Dual Mode (WiFi + GSM)");
    Serial.println("=================================");
    
    // Pin modes
    pinMode(pinBuzz, OUTPUT);
    pinMode(pinRelay, OUTPUT);
    pinMode(pinValve, OUTPUT);
    
    // Default states (aman)
    digitalWrite(pinBuzz, HIGH);   // Buzzer OFF
    digitalWrite(pinRelay, LOW);   // Fan OFF
    digitalWrite(pinValve, HIGH);  // Valve TUTUP
    
    // Servo setup
    servo1.attach(servoPin);
    servo1.write(SERVO_TUTUP);
    
    // Inisialisasi SIM800L Serial
    SerialGSM.begin(9600, SERIAL_8N1, SIM800L_RX, SIM800L_TX);
    delay(3000);  // Tunggu SIM800L boot
    
    // Coba koneksi
    if (!connectWiFi()) {
        Serial.println("WiFi gagal, mencoba GSM...");
        connectGSM();
    }
    
    Serial.println("Setup complete!");
}

// ============== MAIN LOOP ==============
void loop() {
    // Cek koneksi setiap 30 detik
    if (millis() - lastConnectionCheck >= 30000) {
        checkConnection();
        lastConnectionCheck = millis();
    }
    
    // Baca sensor gas
    int adc_value = analogRead(sensorGas);
    float voltage_adc = ((float)adc_value * REF_VOLTAGE) / ADC_RESOLUTION;
    int persen = map(adc_value, 0, 4096, 0, 100);
    
    Serial.print("Gas: ");
    Serial.print(persen);
    Serial.print("% | Mode: ");
    Serial.print(currentMode == MODE_WIFI ? "WiFi" : (currentMode == MODE_GSM ? "GSM" : "NONE"));
    Serial.println();
    
    // Deteksi kebocoran gas
    bool isAlert = voltage_adc > GAS_THRESHOLD;
    
    if (isAlert) {
        buzzfanOn();
        if (modeAuto) {
            digitalWrite(pinValve, LOW);
            servo1.write(SERVO_BUKA);
            valveOn = false;
        }
        if (!statusNotif) {
            Serial.println("!!! KEBOCORAN GAS TERDETEKSI !!!");
        }
        statusNotif = true;
    } else {
        buzzfanOff();
        if (modeAuto) {
            digitalWrite(pinValve, HIGH);
            servo1.write(SERVO_TUTUP);
            valveOn = true;
        }
        statusNotif = false;
    }
    
    // Kirim data ke server setiap 2 detik
    if (millis() - lastSend >= 2000) {
        if (currentMode != MODE_NONE) {
            sendDataToServer(persen, voltage_adc, isAlert);
        }
        lastSend = millis();
    }
    
    // Cek command dari server setiap 2 detik
    if (millis() - lastCommandCheck >= 2000) {
        if (currentMode != MODE_NONE) {
            checkServerCommands();
        }
        lastCommandCheck = millis();
    }
    
    delay(100);
}

// ============== CONNECT WIFI ==============
bool connectWiFi() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✓ WiFi Connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        currentMode = MODE_WIFI;
        return true;
    }
    
    Serial.println("\n✗ WiFi Failed!");
    return false;
}

// ============== CONNECT GSM ==============
bool connectGSM() {
    Serial.println("Initializing SIM800L...");
    
    // Reset modem (optional)
    if (SIM800L_RST > 0) {
        pinMode(SIM800L_RST, OUTPUT);
        digitalWrite(SIM800L_RST, LOW);
        delay(100);
        digitalWrite(SIM800L_RST, HIGH);
        delay(3000);
    }
    
    // Init modem
    Serial.println("Waiting for modem...");
    if (!modem.restart()) {
        Serial.println("✗ Modem restart failed!");
        return false;
    }
    
    String modemInfo = modem.getModemInfo();
    Serial.print("Modem: ");
    Serial.println(modemInfo);
    
    // Cek SIM card
    if (modem.getSimStatus() != 1) {
        Serial.println("✗ SIM card not detected!");
        return false;
    }
    Serial.println("✓ SIM card OK");
    
    // Tunggu registrasi jaringan
    Serial.print("Waiting for network...");
    if (!modem.waitForNetwork(60000L)) {
        Serial.println("\n✗ Network not available!");
        return false;
    }
    Serial.println("\n✓ Network registered");
    
    // Konek ke GPRS
    Serial.print("Connecting to GPRS (");
    Serial.print(APN);
    Serial.print(")...");
    
    if (!modem.gprsConnect(APN, GSM_USER, GSM_PASS)) {
        Serial.println("\n✗ GPRS connection failed!");
        return false;
    }
    
    Serial.println("\n✓ GPRS Connected!");
    Serial.print("Signal: ");
    Serial.print(modem.getSignalQuality());
    Serial.println("/31");
    
    currentMode = MODE_GSM;
    return true;
}

// ============== CHECK CONNECTION ==============
bool checkConnection() {
    // Cek koneksi saat ini
    if (currentMode == MODE_WIFI) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi lost! Trying to reconnect...");
            if (!connectWiFi()) {
                Serial.println("WiFi failed, switching to GSM...");
                return connectGSM();
            }
        }
        return true;
    } 
    else if (currentMode == MODE_GSM) {
        if (!modem.isGprsConnected()) {
            Serial.println("GSM lost! Trying WiFi first...");
            if (!connectWiFi()) {
                Serial.println("WiFi failed, reconnecting GSM...");
                return connectGSM();
            }
        }
        return true;
    }
    else {
        // Tidak ada koneksi, coba WiFi dulu
        if (!connectWiFi()) {
            return connectGSM();
        }
        return true;
    }
}

// ============== HTTP REQUEST ==============
String httpRequest(String endpoint, String method, String payload) {
    String url = String(SERVER_URL) + endpoint;
    String response = "";
    
    if (currentMode == MODE_WIFI) {
        // Pakai WiFi HTTPClient
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Accept", "application/json");
        
        int httpCode;
        if (method == "POST") {
            httpCode = http.POST(payload);
        } else {
            httpCode = http.GET();
        }
        
        if (httpCode > 0) {
            response = http.getString();
        }
        http.end();
    }
    else if (currentMode == MODE_GSM) {
        // Pakai TinyGSM HTTP
        // Untuk SIM800L, kita pakai AT command langsung
        // atau library TinyGSM HttpClient
        
        // Simplified: pakai TinyGsmClient dengan HTTPClient
        // Note: Ini memerlukan library tambahan untuk HTTPS
        // Untuk HTTP biasa:
        
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        
        int httpCode;
        if (method == "POST") {
            httpCode = http.POST(payload);
        } else {
            httpCode = http.GET();
        }
        
        if (httpCode > 0) {
            response = http.getString();
        }
        http.end();
    }
    
    return response;
}

// ============== SEND DATA TO SERVER ==============
void sendDataToServer(int gasPercent, float voltage, bool alert) {
    StaticJsonDocument<200> doc;
    doc["gas"] = gasPercent;
    doc["voltage"] = voltage;
    doc["alert"] = alert;
    doc["connection"] = (currentMode == MODE_WIFI) ? "wifi" : "gsm";
    
    String payload;
    serializeJson(doc, payload);
    
    String response = httpRequest("/api/device/data", "POST", payload);
    
    if (response.length() > 0) {
        StaticJsonDocument<200> resDoc;
        DeserializationError error = deserializeJson(resDoc, response);
        if (!error) {
            String serverMode = resDoc["mode"].as<String>();
            modeAuto = (serverMode == "auto");
        }
    }
}

// ============== CHECK SERVER COMMANDS ==============
void checkServerCommands() {
    String response = httpRequest("/api/device/commands", "GET");
    
    if (response.length() > 0) {
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, response);
        
        if (!error) {
            JsonArray commands = doc["commands"].as<JsonArray>();
            for (JsonObject cmd : commands) {
                int cmdId = cmd["id"];
                String command = cmd["command"].as<String>();
                
                Serial.print("Command: ");
                Serial.println(command);
                
                executeCommand(command);
                acknowledgeCommand(cmdId);
            }
        }
    }
}

// ============== EXECUTE COMMAND ==============
void executeCommand(String command) {
    if (command == "mode_auto") {
        modeAuto = true;
    } else if (command == "mode_manual") {
        modeAuto = false;
    } else if (command == "valve_on") {
        if (!modeAuto) setValve(true);
    } else if (command == "valve_off") {
        if (!modeAuto) setValve(false);
    }
}

// ============== ACKNOWLEDGE COMMAND ==============
void acknowledgeCommand(int commandId) {
    StaticJsonDocument<100> doc;
    doc["command_id"] = commandId;
    
    String payload;
    serializeJson(doc, payload);
    
    httpRequest("/api/device/command-ack", "POST", payload);
}

// ============== SET VALVE ==============
void setValve(bool on) {
    valveOn = on;
    if (on) {
        digitalWrite(pinValve, HIGH);
        servo1.write(SERVO_TUTUP);
    } else {
        digitalWrite(pinValve, LOW);
        servo1.write(SERVO_BUKA);
    }
}

// ============== BUZZER & FAN ==============
void buzzfanOff() {
    digitalWrite(pinBuzz, HIGH);
    digitalWrite(pinRelay, LOW);
}

void buzzfanOn() {
    digitalWrite(pinBuzz, LOW);
    digitalWrite(pinRelay, HIGH);
}
