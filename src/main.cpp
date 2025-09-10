#include <Arduino.h>

#define ESP32_TX2 17 // TX2 (GPIO17) -> N58 RX
#define ESP32_RX2 16 // RX2 (GPIO16) <- N58 TX
#define NEOWAY_BAUD 115200
#define RESPONSE_TIMEOUT_MS 10000UL

HardwareSerial neoway(2); // UART2

// Read response from module up to a timeout (non-blocking, no delay())
String readResponse(unsigned long timeoutMs = RESPONSE_TIMEOUT_MS) {
  String resp;
  unsigned long start = millis();

  while (millis() - start < timeoutMs) {
    while (neoway.available()) {
      char c = (char)neoway.read();
      resp += c;
    }
    yield(); // keep system responsive
  }
  return resp;
}

// Send an "AT" command and print results to the USB Serial monitor
void sendAT() {
  Serial.println("-> Sending: AT");
  neoway.print("AT\r\n");

  String response = readResponse();
  if (response.length() == 0) {
    Serial.println("<- No response (timeout)");
  } else {
    Serial.println("<- Response:");
    Serial.println(response);
  }
}

// Buffer to collect one-line input from Serial Monitor
String serialLine = "";

void setup() {
  Serial.begin(115200);
  // Initialize UART2 to talk with Neoway N58 using onboard RX2/TX2 pins
  neoway.begin(NEOWAY_BAUD, SERIAL_8N1, ESP32_RX2, ESP32_TX2);

  Serial.println();
  Serial.println("Neoway N58 UART2 (ESP32) - AT sender");
  Serial.println("Using onboard RX2/TX2 pins: RX2=GPIO16, TX2=GPIO17");
  Serial.println("Type 'AT' and press Enter to send the AT command to the module.");
  Serial.println("Responses will be printed here. No delays are used in main loop.");
}

void loop() {
  // Read incoming bytes from USB Serial (user typing commands)
  while (Serial.available()) {
    char c = (char)Serial.read();
    // echo back
    Serial.write(c);

    if (c == '\r') continue; // ignore carriage return
    if (c == '\n') {
      String cmd = serialLine;
      cmd.trim();
      serialLine = "";

      if (cmd.length() == 0) {
        // ignore empty lines
      } else if (cmd.equalsIgnoreCase("AT")) {
        sendAT();
      } else {
        Serial.println("Only 'AT' is allowed in this sketch.");
      }
    } else {
      serialLine += c;
    }
  }

  // Forward any unsolicited data from Neoway to Serial Monitor
  while (neoway.available()) {
    Serial.write((char)neoway.read());
  }

  yield();
}
