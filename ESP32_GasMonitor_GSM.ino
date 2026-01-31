/*
 * ESP32 Gas Monitor - GSM Only (SIM800L)
 * 
 * Koneksi internet HANYA via SIM800L GSM
 * Tidak menggunakan WiFi sama sekali
 * 
 * Hardware:
 * - ESP32 Dev Module
 * - SIM800L GSM Module + Antena
 * - SIM Card Indosat dengan kuota data
 * - Sensor TGS2610
 * - Motor Servo, Solenoid Valve, Buzzer, Relay
 */

// ============== LIBRARY ==============
#include <ArduinoJson.h>
#include <ESP32Servo.h>

// Library untuk SIM800L
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>

// ============== KONFIGURASI SERVER ==============
// GANTI dengan URL hosting Anda!
const char* SERVER_HOST = "gas-monitor.site";  // Domain hosting
const int SERVER_PORT = 80;                     // Port HTTP standard

// ============== KONFIGURASI GSM (Tri / 3) ==============
const char* APN = "3gprs";             // APN untuk Kartu 3 (Tri)
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
TinyGsmClient client(modem);

// ============== VARIABLES ==============
bool isConnected = false;
bool statusNotif = false;
bool modeAuto = true;
bool valveOn = true;
unsigned long lastSend = 0;
unsigned long lastCommandCheck = 0;
unsigned long lastReconnect = 0;

// ============== FUNCTION PROTOTYPES ==============
bool connectGSM();
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
    Serial.println("\n=================================");
    Serial.println("Gas Monitor - GSM Only (SIM800L)");
    Serial.println("=================================");
    
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
    
    // Inisialisasi SIM800L Serial (trying 115200 baud)
    SerialGSM.begin(115200, SERIAL_8N1, SIM800L_RX, SIM800L_TX);
    delay(3000);  // Tunggu SIM800L boot
    
    // Konek ke GSM
    connectGSM();
    
    Serial.println("Setup complete!");
}

// ============== MAIN LOOP ==============
void loop() {
    // Cek koneksi setiap 30 detik
    if (!isConnected && millis() - lastReconnect >= 30000) {
        connectGSM();
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
    Serial.print("V | GSM: ");
    Serial.println(isConnected ? "Connected" : "Disconnected");
    
    // Deteksi kebocoran gas
    bool isAlert = voltage_adc > GAS_THRESHOLD;
    
    if (isAlert) {
        buzzfanOn();
        if (modeAuto) {
            digitalWrite(pinValve, LOW);
            // servo1.write(SERVO_BUKA);
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
            // servo1.write(SERVO_TUTUP);
            valveOn = true;
        }
        statusNotif = false;
    }
    
    // Kirim data ke server setiap 3 detik (GSM lebih lambat)
    if (millis() - lastSend >= 3000) {
        if (isConnected) {
            sendDataToServer(persen, voltage_adc, isAlert);
        }
        lastSend = millis();
    }
    
    // Cek command dari server setiap 3 detik
    if (millis() - lastCommandCheck >= 3000) {
        if (isConnected) {
            checkServerCommands();
        }
        lastCommandCheck = millis();
    }
    
    delay(100);
}

// ============== CONNECT GSM ==============
bool connectGSM() {
    Serial.println("\n--- Connecting to GSM Network ---");
    
    // Reset modem (optional)
    if (SIM800L_RST > 0) {
        pinMode(SIM800L_RST, OUTPUT);
        digitalWrite(SIM800L_RST, LOW);
        delay(100);
        digitalWrite(SIM800L_RST, HIGH);
        delay(3000);
    }
    
    // Init modem
    Serial.println("Restarting modem...");
    if (!modem.restart()) {
        Serial.println("✗ Modem restart failed!");
        isConnected = false;
        return false;
    }
    
    String modemInfo = modem.getModemInfo();
    Serial.print("Modem: ");
    Serial.println(modemInfo);
    
    // Cek SIM card
    int simStatus = modem.getSimStatus();
    if (simStatus != 1 && simStatus != 3) {
        Serial.println("✗ SIM card not detected!");
        isConnected = false;
        return false;
    }
    Serial.println("✓ SIM card OK");
    
    // Tunggu registrasi jaringan
    Serial.print("Waiting for network");
    for (int i = 0; i < 60; i++) {
        if (modem.isNetworkConnected()) {
            break;
        }
        Serial.print(".");
        delay(1000);
    }
    
    if (!modem.isNetworkConnected()) {
        Serial.println("\n✗ Network not available!");
        isConnected = false;
        return false;
    }
    Serial.println("\n✓ Network registered");
    
    // Cek signal
    int signal = modem.getSignalQuality();
    Serial.print("Signal: ");
    Serial.print(signal);
    Serial.println("/31");
    
    // Konek ke GPRS
    Serial.print("Connecting to GPRS (");
    Serial.print(APN);
    Serial.print(")");
    
    for (int i = 0; i < 3; i++) {
        if (modem.gprsConnect(APN, GSM_USER, GSM_PASS)) {
            break;
        }
        Serial.print(".");
        delay(2000);
    }
    
    if (!modem.isGprsConnected()) {
        Serial.println("\n✗ GPRS connection failed!");
        isConnected = false;
        return false;
    }
    
    Serial.println("\n✓ GPRS Connected!");
    Serial.print("IP: ");
    Serial.println(modem.localIP());
    
    isConnected = true;
    return true;
}

// ============== SEND DATA TO SERVER ==============
bool sendDataToServer(int gasPercent, float voltage, bool alert) {
    Serial.println("Sending data to server...");
    
    // Buat JSON payload
    StaticJsonDocument<200> doc;
    doc["gas"] = gasPercent;
    doc["voltage"] = voltage;
    doc["alert"] = alert;
    doc["connection"] = "gsm";
    
    String payload;
    serializeJson(doc, payload);
    
    // HTTP POST request
    if (!client.connect(SERVER_HOST, SERVER_PORT)) {
        Serial.println("✗ Connection to server failed!");
        isConnected = false;
        return false;
    }
    
    // Send HTTP POST
    client.print("POST /api/device/data HTTP/1.1\r\n");
    client.print("Host: ");
    client.print(SERVER_HOST);
    client.print("\r\n");
    client.print("Content-Type: application/json\r\n");
    client.print("Content-Length: ");
    client.print(payload.length());
    client.print("\r\n");
    client.print("Connection: close\r\n\r\n");
    client.print(payload);
    
    // Wait for response
    unsigned long timeout = millis();
    while (client.connected() && millis() - timeout < 10000) {
        if (client.available()) {
            String line = client.readStringUntil('\n');
            if (line.startsWith("{")) {
                // Parse JSON response
                StaticJsonDocument<200> resDoc;
                DeserializationError error = deserializeJson(resDoc, line);
                if (!error) {
                    String serverMode = resDoc["mode"].as<String>();
                    modeAuto = (serverMode == "auto");
                    Serial.println("✓ Data sent successfully");
                }
                break;
            }
        }
    }
    
    client.stop();
    return true;
}

// ============== CHECK SERVER COMMANDS ==============
void checkServerCommands() {
    if (!client.connect(SERVER_HOST, SERVER_PORT)) {
        isConnected = false;
        return;
    }
    
    // Send HTTP GET
    client.print("GET /api/device/commands HTTP/1.1\r\n");
    client.print("Host: ");
    client.print(SERVER_HOST);
    client.print("\r\n");
    client.print("Connection: close\r\n\r\n");
    
    // Wait for response
    String response = "";
    unsigned long timeout = millis();
    bool bodyStarted = false;
    
    while (client.connected() && millis() - timeout < 10000) {
        if (client.available()) {
            String line = client.readStringUntil('\n');
            if (bodyStarted && line.startsWith("{")) {
                response = line;
                break;
            }
            if (line == "\r") {
                bodyStarted = true;
            }
        }
    }
    
    client.stop();
    
    if (response.length() > 0) {
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
    if (!client.connect(SERVER_HOST, SERVER_PORT)) {
        return;
    }
    
    StaticJsonDocument<100> doc;
    doc["command_id"] = commandId;
    
    String payload;
    serializeJson(doc, payload);
    
    client.print("POST /api/device/command-ack HTTP/1.1\r\n");
    client.print("Host: ");
    client.print(SERVER_HOST);
    client.print("\r\n");
    client.print("Content-Type: application/json\r\n");
    client.print("Content-Length: ");
    client.print(payload.length());
    client.print("\r\n");
    client.print("Connection: close\r\n\r\n");
    client.print(payload);
    
    delay(500);
    client.stop();
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
    digitalWrite(pinBuzz, LOW);
    digitalWrite(pinRelay, LOW);
}

void buzzfanOn() {
    digitalWrite(pinBuzz, HIGH);
    digitalWrite(pinRelay, HIGH);
}
