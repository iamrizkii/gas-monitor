/*
 * RAW DATA TEST - Cek apakah ada data masuk dari SIM800L
 */

#define SIM800L_RX  16
#define SIM800L_TX  17

HardwareSerial SerialGSM(1);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== RAW DATA TEST ===");
    Serial.println("Mencoba berbagai baud rate dan menampilkan RAW data...\n");
}

void loop() {
    int baudRates[] = {9600, 115200, 57600, 38400, 19200};
    
    for (int i = 0; i < 5; i++) {
        int baud = baudRates[i];
        
        Serial.print("\n--- Testing ");
        Serial.print(baud);
        Serial.println(" baud ---");
        
        SerialGSM.end();
        delay(100);
        SerialGSM.begin(baud, SERIAL_8N1, SIM800L_RX, SIM800L_TX);
        delay(500);
        
        // Clear buffer
        while (SerialGSM.available()) {
            SerialGSM.read();
        }
        
        // Kirim AT
        Serial.println("Sending: AT");
        SerialGSM.println("AT");
        
        // Tunggu dan tampilkan semua data yang masuk
        delay(1000);
        
        if (SerialGSM.available()) {
            Serial.print("Received (");
            Serial.print(SerialGSM.available());
            Serial.println(" bytes):");
            
            while (SerialGSM.available()) {
                int b = SerialGSM.read();
                
                // Tampilkan dalam HEX dan ASCII
                Serial.print("0x");
                if (b < 16) Serial.print("0");
                Serial.print(b, HEX);
                Serial.print(" (");
                
                if (b >= 32 && b <= 126) {
                    Serial.print((char)b);
                } else {
                    Serial.print(".");
                }
                
                Serial.print(") ");
            }
            Serial.println();
        } else {
            Serial.println("✗ No data received!");
        }
        
        delay(2000);
    }
    
    Serial.println("\n\n=== CEK HARDWARE ===");
    Serial.println("1. LED SIM800L nyala? (harus nyala atau kedip)");
    Serial.println("2. Voltage VCC SIM800L = 4.0-4.2V?");
    Serial.println("3. GND ESP32 dan SIM800L nyambung?");
    Serial.println("4. Kabel TX/RX tidak putus?");
    Serial.println("\nTest akan diulang dalam 10 detik...\n");
    
    delay(10000);
}
