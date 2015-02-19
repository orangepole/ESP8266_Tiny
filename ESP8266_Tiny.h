#ifndef ESP8266_TINY_H
#define ESP8266_TINY_H

#include <SoftwareSerial.h>

#if defined(__AVR_ATtiny85__)  // add more here
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
  #undef _WANT_DEBUG
#else
  #define DEBUG_PRINT(...)   Serial.print(__VA_ARGS__);
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__);
  #define _WANT_DEBUG
#endif  // defined(__AVR_ATtiny85__)

class ESP8266_Tiny {
 public:
  ESP8266_Tiny(int rxPin, int txPin);

  bool connect(const char* ssid, const char* password);

  bool openConnection(const char *channel, const char *host, const char *port,
      const char *protocol);
  bool closeConnection(const char *channel);

  bool startServer(const char *port);

  int sendDataGetResponse(
      char *channel, const char *data, char *response, int response_len);
  int sendData(char *channel, const char *data) {
    return sendDataGetResponse(channel, data, NULL, 0);
  }

  bool getFirmwareInfo();
  bool getAccessPointInfo();
  bool getIPAddress();
  bool getStatus();
  bool reset(bool echo=false);

  int readIncoming(
      char *channel, char *response, int response_len, long deadline);

 private:
  void drain();
  char next(long timeout=100);
  bool sendExpect(const char **send, int send_parts, const char **expect,
                  int expect_parts, long timeout=1000);
  // Convenience method for 1-part commands.
  bool sendExpect(const char *send, const char *expect, long timeout=1000) {
    return sendExpect(&send, 1, &expect, 1, timeout);
  }
  bool sendExpect(const char *send, const char **expect, int expect_parts,
                  long timeout=1000) {
    return sendExpect(&send, 1, expect, expect_parts, timeout);
  }

  bool sendExpect(const char **send, int send_parts, const char *expect,
                  long timeout=1000) {
    return sendExpect(send, send_parts, &expect, 1, timeout);
  }
  bool readUntil(const char until, char *buffer, int buf_len,
                 long timeout=1000);
  SoftwareSerial _wifi;
};

#endif  // ESP8266_TINY_H
