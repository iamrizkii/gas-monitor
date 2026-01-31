/*
 * AUTO DETECT SIM800L Baud Rate
 * Akan mencoba berbagai baud rate sampai ketemu yang cocok
 */

#define SIM800L_RX  16
#define SIM800L_TX  17

HardwareSerial SerialGSM(1);

// Daftar baud rate yang akan dicoba
int baudRates[] = {9600, 19200, 38400, 57600, 115200, 4800, 2400};
int numBaudRates = 7;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== AUTO DETECT SIM800L BAUD RATE ===\n");
    
    for (int i = 0; i < numBaudRates; i++) {
        int baud = baudRates[i];
        
        Serial.print("Testing ");
        Serial.print(baud);
        Serial.print(" baud... ");
        
        // Init serial dengan baud rate ini
        SerialGSM.end();
        delay(100);
        SerialGSM.begin(baud, SERIAL_8N1, SIM800L_RX, SIM800L_TX);
        delay(500);
        
        // Clear buffer
        while (SerialGSM.available()) {
            SerialGSM.read();
        }
        
        // Kirim AT command
        SerialGSM.println("AT");
        delay(1000);
        
        // Baca response
        String response = "";
        unsigned long timeout = millis();
        while (millis() - timeout < 2000) {
            if (SerialGSM.available()) {
                char c = SerialGSM.read();
                response += c;
            }
        }
        
        // Cek apakah response valid (mengandung "OK")
        if (response.indexOf("OK") >= 0) {
            Serial.println("✓ SUCCESS!");
            Serial.println("\n===================");
            Serial.print("BAUD RATE YANG BENAR: ");
            Serial.println(baud);
            Serial.println("===================\n");
            Serial.println("Response:");
            Serial.println(response);
            
            Serial.println("\n\nSekarang Anda bisa mengetik AT command:");
            Serial.println("Contoh: AT, ATI, AT+CSQ, AT+CPIN?");
            
            // Masuk ke mode interactive
            while(true) {
                // Forward Serial Monitor ke SIM800L
                if (Serial.available()) {
                    String cmd = Serial.readStringUntil('\n');
                    SerialGSM.println(cmd);
                }
                
                // Forward response SIM800L ke Serial Monitor
                if (SerialGSM.available()) {
                    while (SerialGSM.available()) {
                        Serial.write(SerialGSM.read());
                    }
                }
            }
            return;
        } else {
            Serial.println("No valid response");
        }
        
        delay(500);
    }
    
    Serial.println("\n✗ Tidak ketemu baud rate yang cocok!");
    Serial.println("\nKemungkinan:");
    Serial.println("1. TX/RX masih terbalik");
    Serial.println("2. GND tidak nyambung");
    Serial.println("3. SIM800L tidak dapat power cukup");
    Serial.println("4. SIM800L rusak");
}

void loop() {
    // Tidak digunakan
}
