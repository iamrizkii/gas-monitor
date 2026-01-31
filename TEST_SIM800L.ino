/*
 * TEST SIM800L Communication
 * Code sederhana untuk test apakah SIM800L bisa komunikasi dengan ESP32
 */

#define SIM800L_RX  16
#define SIM800L_TX  17

HardwareSerial SerialGSM(1);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== TEST SIM800L ===");
    Serial.println("Trying 9600 baud...");
    
    // Test dengan 9600 baud
    SerialGSM.begin(9600, SERIAL_8N1, SIM800L_RX, SIM800L_TX);
    delay(2000);
    
    // Kirim AT command
    SerialGSM.println("AT");
    delay(1000);
    
    // Baca response
    if (SerialGSM.available()) {
        Serial.println("Response dari SIM800L (9600 baud):");
        while (SerialGSM.available()) {
            Serial.write(SerialGSM.read());
        }
    } else {
        Serial.println("No response with 9600 baud");
        Serial.println("\nTrying 115200 baud...");
        
        // Coba 115200
        SerialGSM.end();
        delay(500);
        SerialGSM.begin(115200, SERIAL_8N1, SIM800L_RX, SIM800L_TX);
        delay(2000);
        
        SerialGSM.println("AT");
        delay(1000);
        
        if (SerialGSM.available()) {
            Serial.println("Response dari SIM800L (115200 baud):");
            while (SerialGSM.available()) {
                Serial.write(SerialGSM.read());
            }
        } else {
            Serial.println("No response with 115200 baud");
            Serial.println("\n✗ SIM800L tidak merespon!");
            Serial.println("\nKemungkinan:");
            Serial.println("1. TX/RX terbalik");
            Serial.println("2. GND tidak nyambung");
            Serial.println("3. SIM800L tidak dapat power");
            Serial.println("4. SIM800L rusak");
        }
    }
}

void loop() {
    // Forward Serial Monitor ke SIM800L
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        SerialGSM.println(cmd);
        Serial.print("Sent: ");
        Serial.println(cmd);
    }
    
    // Forward response SIM800L ke Serial Monitor
    if (SerialGSM.available()) {
        while (SerialGSM.available()) {
            Serial.write(SerialGSM.read());
        }
    }
}
