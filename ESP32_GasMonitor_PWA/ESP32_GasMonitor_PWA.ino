/*
 * ESP32 Gas Monitor - PWA Version
 * Pengganti Blynk dengan HTTP ke Laravel Server
 * 
 * PENTING: Ganti SERVER_IP dengan IP laptop Anda
 * Cek dengan: ipconfig di CMD
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

// ============== KONFIGURASI ==============
// Ganti dengan IP server Laravel Anda
const char* SERVER_IP = "172.20.10.8";    // IP Laptop Anda
const int SERVER_PORT = 8000;               // Port default Laragon

// WiFi credentials (sama seperti sebelumnya)
const char* ssid = "iki";
const char* pass = "12345670";

// ============== PIN DEFINITIONS ==============
const int sensorGas = 36;   // GPIO36 - Analog input sensor TGS2610
const int servoPin = 23;    // GPIO23 - Motor Servo
const int pinBuzz = 15;     // GPIO15 - Buzzer
const int pinRelay = 2;     // GPIO2  - Relay (Exhaust Fan)
const int pinValve = 22;    // GPIO22 - SSR (Solenoid Valve)

// ============== CONSTANTS ==============
#define REF_VOLTAGE    3.3
#define ADC_RESOLUTION 4096.0
#define SERVO_TUTUP    0
#define SERVO_BUKA     180
#define GAS_THRESHOLD  2.0    // Voltage threshold untuk deteksi gas

// ============== OBJECTS ==============
Servo servo1;
HTTPClient http;

// ============== VARIABLES ==============
bool statusNotif = false;
bool modeAuto = true;       // Default: Auto mode
bool valveOn = true;        // Default: Valve ON (gas bisa mengalir)
unsigned long lastSend = 0;
unsigned long lastCommandCheck = 0;

// ============== FUNCTION PROTOTYPES ==============
void connectWiFi();
void sendDataToServer(int gasPercent, float voltage, bool alert);
void checkServerCommands();
void executeCommand(String command);
void setValve(bool on);
void buzzfanOn();
void buzzfanOff();

// ============== SETUP ==============
void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Gas Monitor PWA Version ===");
    
    // Pin modes
    pinMode(pinBuzz, OUTPUT);
    pinMode(pinRelay, OUTPUT);
    pinMode(pinValve, OUTPUT);
    
    // Default states (aman)
    digitalWrite(pinBuzz, HIGH);   // Buzzer OFF (active low)
    digitalWrite(pinRelay, LOW);   // Fan OFF
    digitalWrite(pinValve, HIGH);  // Valve TUTUP (gas tidak mengalir saat bahaya)
    
    // Servo setup
    servo1.attach(servoPin);
    servo1.write(SERVO_TUTUP);
    
    // Connect WiFi
    connectWiFi();
    
    Serial.println("Setup complete!");
}

// ============== MAIN LOOP ==============
void loop() {
    // Pastikan WiFi terhubung
    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
    }
    
    // Baca sensor gas
    int adc_value = analogRead(sensorGas);
    float voltage_adc = ((float)adc_value * REF_VOLTAGE) / ADC_RESOLUTION;
    int persen = map(adc_value, 0, 4096, 0, 100);
    
    Serial.print("Gas: ");
    Serial.print(persen);
    Serial.print("% | Voltage: ");
    Serial.print(voltage_adc);
    Serial.print("V | Mode: ");
    Serial.println(modeAuto ? "AUTO" : "MANUAL");
    
    // Deteksi kebocoran gas
    bool isAlert = voltage_adc > GAS_THRESHOLD;
    
    if (isAlert) {
        // Ada kebocoran gas!
        buzzfanOn();
        
        if (modeAuto) {
            // Mode Auto: Otomatis tutup valve dan buka regulator
            digitalWrite(pinValve, LOW);    // Valve BUKA (matikan gas)
            servo1.write(SERVO_BUKA);       // Servo lepas regulator
            valveOn = false;
        }
        
        if (!statusNotif) {
            Serial.println("!!! KEBOCORAN GAS TERDETEKSI !!!");
        }
        statusNotif = true;
        
    } else {
        // Aman, tidak ada gas
        buzzfanOff();
        
        if (modeAuto) {
            // Mode Auto: Valve tetap ON (gas bisa mengalir)
            digitalWrite(pinValve, HIGH);   // Valve TUTUP (gas mengalir normal)
            servo1.write(SERVO_TUTUP);      // Servo posisi normal
            valveOn = true;
        }
        statusNotif = false;
    }
    
    // Kirim data ke server setiap 1 detik
    if (millis() - lastSend >= 1000) {
        sendDataToServer(persen, voltage_adc, isAlert);
        lastSend = millis();
    }
    
    // Cek command dari server setiap 1 detik
    if (millis() - lastCommandCheck >= 1000) {
        checkServerCommands();
        lastCommandCheck = millis();
    }
    
    delay(100);  // Small delay untuk stabilitas
}

// ============== WIFI CONNECTION ==============
void connectWiFi() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, pass);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi Connection Failed!");
    }
}

// ============== SEND DATA TO SERVER ==============
void sendDataToServer(int gasPercent, float voltage, bool alert) {
    if (WiFi.status() != WL_CONNECTED) return;
    
    String url = "http://" + String(SERVER_IP) + ":" + String(SERVER_PORT) + "/api/device/data";
    
    // Create JSON payload
    StaticJsonDocument<200> doc;
    doc["gas"] = gasPercent;
    doc["voltage"] = voltage;
    doc["alert"] = alert;
    
    String payload;
    serializeJson(doc, payload);
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Accept", "application/json");
    
    int httpCode = http.POST(payload);
    
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            String response = http.getString();
            
            // Parse response to get current mode
            StaticJsonDocument<200> resDoc;
            DeserializationError error = deserializeJson(resDoc, response);
            
            if (!error) {
                String serverMode = resDoc["mode"].as<String>();
                modeAuto = (serverMode == "auto");
            }
        }
    } else {
        Serial.print("HTTP Error: ");
        Serial.println(http.errorToString(httpCode));
    }
    
    http.end();
}

// ============== CHECK SERVER COMMANDS ==============
void checkServerCommands() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    String url = "http://" + String(SERVER_IP) + ":" + String(SERVER_PORT) + "/api/device/commands";
    
    http.begin(url);
    http.addHeader("Accept", "application/json");
    
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();
        
        // Parse JSON response
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, response);
        
        if (!error) {
            JsonArray commands = doc["commands"].as<JsonArray>();
            
            for (JsonObject cmd : commands) {
                int cmdId = cmd["id"];
                String command = cmd["command"].as<String>();
                
                Serial.print("Executing command: ");
                Serial.println(command);
                
                // Execute command
                executeCommand(command);
                
                // Acknowledge command
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
        Serial.println("Mode changed to AUTO");
        
    } else if (command == "mode_manual") {
        modeAuto = false;
        Serial.println("Mode changed to MANUAL");
        
    } else if (command == "valve_on") {
        if (!modeAuto) {  // Hanya bisa di mode manual
            setValve(true);
            Serial.println("Valve turned ON");
        }
        
    } else if (command == "valve_off") {
        if (!modeAuto) {  // Hanya bisa di mode manual
            setValve(false);
            Serial.println("Valve turned OFF");
        }
    }
}

// ============== ACKNOWLEDGE COMMAND ==============
void acknowledgeCommand(int commandId) {
    if (WiFi.status() != WL_CONNECTED) return;
    
    String url = "http://" + String(SERVER_IP) + ":" + String(SERVER_PORT) + "/api/device/command-ack";
    
    StaticJsonDocument<100> doc;
    doc["command_id"] = commandId;
    
    String payload;
    serializeJson(doc, payload);
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    http.POST(payload);
    http.end();
}

// ============== SET VALVE ==============
void setValve(bool on) {
    valveOn = on;
    if (on) {
        digitalWrite(pinValve, HIGH);   // Valve TUTUP (gas mengalir normal)
        servo1.write(SERVO_TUTUP);
    } else {
        digitalWrite(pinValve, LOW);    // Valve BUKA (gas diputus)
        servo1.write(SERVO_BUKA);
    }
}

// ============== BUZZER & FAN CONTROL ==============
void buzzfanOff() {
    digitalWrite(pinBuzz, HIGH);  // Buzzer OFF (active low)
    digitalWrite(pinRelay, LOW);  // Fan OFF
}

void buzzfanOn() {
    digitalWrite(pinBuzz, LOW);   // Buzzer ON (active low)
    digitalWrite(pinRelay, HIGH); // Fan ON
}
