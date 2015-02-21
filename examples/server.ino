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
      SERVER_PORT[0] == '\0') {
    DEBUG_PRINTLN("Please update the sketch with values that match your environment.");
    // do nothing.
    while (true) { }
  }
  wifi.reset();
  wifi.connect(ESP8266_SSID, ESP8266_PASS);
  // getIPAddress() will print the address to the debug stream. It might be hard to 
  // figure out what your IP address is here if you're using an ATTiny or some other
  // chip that doesn't support hardware serial; some options are:
  // 1) Check your router/DHCP server and look for a recently connected WiFi device
  // 2) Connect a serial-to-USB adapter to the ESP8266 with ESP8266 TX -> adapter RX and
  //    GND to GND, run your favorite terminal program connected to the adapter port, 
  //    and you'll see everything the ESP8266 sends.
  wifi.getIPAddress();
  wifi.startServer(SERVER_PORT);
  char incoming[20];
  while (true) {
    char channel[2];
    int length = wifi.readIncoming(channel, incoming, 20, millis() + 10000);
    if (length > 0) {
      DEBUG_PRINT("got ");
      DEBUG_PRINT(length);
      DEBUG_PRINT(" bytes incoming on channel ");
      DEBUG_PRINT(channel);
      DEBUG_PRINT(" : ");
      DEBUG_PRINTLN(incoming);
      wifi.sendData(channel, "ACK");
    } 
  }
}

