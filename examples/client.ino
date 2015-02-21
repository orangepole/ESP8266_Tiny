#include <SoftwareSerial.h>
#include <ESP8266_Tiny.h>

#if defined(__AVR_ATtiny85__)  // add more here
  #define SOFT_RX_PIN     1
  #define SOFT_TX_PIN     0
#else
  #define SOFT_RX_PIN     8
  #define SOFT_TX_PIN     9
#endif  // defined(__AVR_ATtiny85__)

// Make sure you replace these with values that match your environment.
#define ESP8266_SSID  ""
#define ESP8266_PASS  ""
#define SERVER_IP     ""
#define SERVER_PORT   ""

ESP8266_Tiny wifi(SOFT_RX_PIN, SOFT_TX_PIN);

void setup() {
#ifdef _WANT_DEBUG
  Serial.begin(115200);
#endif // _WANT_DEBUG
}

void loop() {
  if (ESP8266_SSID[0] == '\0' ||
      ESP8266_PASS[0] == '\0' || 
      SERVER_IP[0] == '\0' || 
      SERVER_PORT[0] == '\0') {
    DEBUG_PRINTLN("Please update the sketch with values that match your environment.");
    // do nothing.
    while (true) { }
  }
  wifi.reset();
  wifi.connect(ESP8266_SSID, ESP8266_PASS);
  delay(5000);
  wifi.openConnection(NULL, SERVER_IP, SERVER_PORT, "UDP");
  while (true) {
    char buffer[20];
    sprintf(buffer, "time is %ld", millis());
    // re-use buffer for response.
    if (wifi.sendDataGetResponse(NULL, buffer, buffer, 20) > 0) {
      DEBUG_PRINT("got response data: ");
      DEBUG_PRINTLN(buffer);
    }
    delay(5000);
  }
}

