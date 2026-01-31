/*
 * ESP32 Gas Monitor - WiFi Online Version
 * 
 * Koneksi ke server HOSTING (gas-monitor.site)
 * Bisa diakses dari HP mana saja via PWA
 * 
 * Hardware:
 * - ESP32 Dev Module
 * - Sensor TGS2610
 * - Motor Servo, Solenoid Valve, Buzzer, Relay
 */

// ============== LIBRARY ==============
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>  // Untuk HTTPS
#include <ArduinoJson.h>
#include <ESP32Servo.h>

// ============== KONFIGURASI SERVER ==============
const char* SERVER_HOST = "gas-monitor.site";  // Domain hosting Anda
const int SERVER_PORT = 80;                     // Port HTTP

// ============== KONFIGURASI WiFi ==============
// GANTI dengan WiFi Anda!
const char* ssid = "NAMA_WIFI_ANDA";      // Contoh: "Rumah_WiFi"
const char* pass = "PASSWORD_WIFI_ANDA";  // Contoh: "password123"

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

// ============== VARIABLES ==============
bool isConnected = false;
bool statusNotif = false;
bool modeAuto = true;
bool valveOn = true;
unsigned long lastSend = 0;
unsigned long lastCommandCheck = 0;
unsigned long lastReconnect = 0;

// ============== FUNCTION PROTOTYPES ==============
bool connectWiFi();
bool sendDataToServer(int gasPercent, float voltage, bool alert);
void checkServerCommands();
void executeCommand(String command);
void acknowledgeCommand(int commandId);
void setValve(bool on);
void buzzfanOn();
void buzzfanOff();

// ============== SETUP ==============
void setup() {
    Serial.begin(115200);
    Serial.println("\n=====================================");
    Serial.println("Gas Monitor - WiFi Online Version");
    Serial.println("Server: gas-monitor.site");
    Serial.println("=====================================");
    
    // Pin modes
    pinMode(pinBuzz, OUTPUT);
    pinMode(pinRelay, OUTPUT);
    pinMode(pinValve, OUTPUT);
    
    // Default states (aman)
    digitalWrite(pinBuzz, LOW);    // Buzzer OFF (Silent)
    digitalWrite(pinRelay, LOW);   // Fan OFF
    digitalWrite(pinValve, HIGH);  // Valve TUTUP
    
    // Servo setup
    servo1.attach(servoPin);
    servo1.write(SERVO_TUTUP);
    
    // Konek ke WiFi
    connectWiFi();
    
    Serial.println("Setup complete!");
}

// ============== MAIN LOOP ==============
void loop() {
    // Cek koneksi WiFi setiap 30 detik
    if (!isConnected && millis() - lastReconnect >= 30000) {
        connectWiFi();
        lastReconnect = millis();
    }
    
    // Baca sensor gas
    int adc_value = analogRead(sensorGas);
    float voltage_adc = ((float)adc_value * REF_VOLTAGE) / ADC_RESOLUTION;
    int persen = map(adc_value, 0, 4096, 0, 100);
    
    Serial.print("Gas: ");
    Serial.print(persen);
    Serial.print("% | Voltage: ");
    Serial.print(voltage_adc);
    Serial.print("V | WiFi: ");
    Serial.println(isConnected ? "Connected" : "Disconnected");
    
    // Deteksi kebocoran gas
    bool isAlert = voltage_adc > GAS_THRESHOLD;
    
    if (isAlert) {
        buzzfanOn();
        if (modeAuto) {
            digitalWrite(pinValve, LOW);
            // servo1.write(SERVO_BUKA);  // Servo tidak bergerak di auto mode
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
            // servo1.write(SERVO_TUTUP);  // Servo tidak bergerak di auto mode
            valveOn = true;
        }
        statusNotif = false;
    }
    
    // Kirim data ke server setiap 1 detik
    if (millis() - lastSend >= 1000) {
        if (isConnected) {
            sendDataToServer(persen, voltage_adc, isAlert);
        }
        lastSend = millis();
    }
    
    // Cek command dari server setiap 1 detik
    if (millis() - lastCommandCheck >= 1000) {
        if (isConnected) {
            checkServerCommands();
        }
        lastCommandCheck = millis();
    }
    
    delay(100);
}

// ============== CONNECT WiFi ==============
bool connectWiFi() {
    Serial.println("\n--- Connecting to WiFi ---");
    Serial.print("SSID: ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, pass);
    
    int attempts = 0;
    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(1000);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✓ WiFi Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Signal Strength: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        isConnected = true;
        return true;
    } else {
        Serial.println("\n✗ WiFi Connection Failed!");
        isConnected = false;
        return false;
    }
}

// ============== SEND DATA TO SERVER ==============
bool sendDataToServer(int gasPercent, float voltage, bool alert) {
    if (WiFi.status() != WL_CONNECTED) {
        isConnected = false;
        return false;
    }
    
    WiFiClientSecure client;
    client.setInsecure();  // Skip SSL certificate verification (untuk hosting)
    
    HTTPClient http;
    
    // Buat URL lengkap dengan HTTPS
    String url = "https://" + String(SERVER_HOST) + "/api/device/data";
    
    // Buat JSON payload
    StaticJsonDocument<200> doc;
    doc["gas"] = gasPercent;
    doc["voltage"] = voltage;
    doc["alert"] = alert;
    doc["connection"] = "wifi";
    
    String payload;
    serializeJson(doc, payload);
    
    // Kirim HTTP POST
    http.begin(client, url);  // Pakai secure client
    http.addHeader("Content-Type", "application/json");
    
    int httpCode = http.POST(payload);
    
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            String response = http.getString();
            
            // Parse response untuk update mode
            StaticJsonDocument<200> resDoc;
            DeserializationError error = deserializeJson(resDoc, response);
            if (!error) {
                String serverMode = resDoc["mode"].as<String>();
                modeAuto = (serverMode == "auto");
            }
            
            Serial.println("✓ Data sent successfully");
        } else {
            Serial.print("✗ HTTP Response: ");
            Serial.print(httpCode);
            Serial.print(" - ");
            Serial.println(http.getString());
        }
        http.end();
        return true;
    } else {
        Serial.print("✗ HTTP Error: ");
        Serial.println(httpCode);
        http.end();
        return false;
    }
}

// ============== CHECK SERVER COMMANDS ==============
void checkServerCommands() {
    if (WiFi.status() != WL_CONNECTED) {
        isConnected = false;
        return;
    }
    
    WiFiClientSecure client;
    client.setInsecure();
    
    HTTPClient http;
    String url = "https://" + String(SERVER_HOST) + "/api/device/commands";
    
    http.begin(client, url);
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();
        
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, response);
        
        if (!error) {
            JsonArray commands = doc["commands"].as<JsonArray>();
            for (JsonObject cmd : commands) {
                int cmdId = cmd["id"];
                String command = cmd["command"].as<String>();
                
                Serial.print("Command received: ");
                Serial.println(command);
                
                executeCommand(command);
                acknowledgeCommand(cmdId);
            }
        }
    }
    
    http.end();
}

// ============== EXECUTE COMMAND ==============
void executeCommand(String command) {
    if (command == "mode_auto") {
        modeAuto = true;
        Serial.println("Mode: AUTO");
    } else if (command == "mode_manual") {
        modeAuto = false;
        Serial.println("Mode: MANUAL");
    } else if (command == "valve_on") {
        if (!modeAuto) {
            setValve(true);
            Serial.println("Valve: ON");
        }
    } else if (command == "valve_off") {
        if (!modeAuto) {
            setValve(false);
            Serial.println("Valve: OFF");
        }
    }
}

// ============== ACKNOWLEDGE COMMAND ==============
void acknowledgeCommand(int commandId) {
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }
    
    WiFiClientSecure client;
    client.setInsecure();
    
    HTTPClient http;
    String url = "https://" + String(SERVER_HOST) + "/api/device/command-ack";
    
    StaticJsonDocument<100> doc;
    doc["command_id"] = commandId;
    
    String payload;
    serializeJson(doc, payload);
    
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    http.POST(payload);
    http.end();
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
    digitalWrite(pinBuzz, LOW);    // Buzzer OFF (Silent)
    digitalWrite(pinRelay, LOW);   // Fan OFF
}

void buzzfanOn() {
    digitalWrite(pinBuzz, HIGH);   // Buzzer ON (Beep)
    digitalWrite(pinRelay, HIGH);  // Fan ON
}
