#ifndef ESP8266_TINY_H
#define ESP8266_TINY_H

#include <SoftwareSerial.h>

// Add a '#define _SUPRESS_DEBUG' to your sketch before including this file if
// you really don't want debug output.
#ifdef _SUPRESS_DEBUG
#undef _WANT_DEBUG
#else
#define _WANT_DEBUG
#endif  // _SUPRESS_DEBUG

#if defined(__AVR_ATtiny85__)  // add more here; without hardware serial
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
  #undef _WANT_DEBUG
#else
  // All data sent and received will be written to hardware serial output.
  #ifdef _WANT_DEBUG
    #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__);
    #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__);
  #else
    #define DEBUG_PRINT(...)
    #define DEBUG_PRINTLN(...)
  #endif  // _WANT_DEBUG

#endif  // defined(__AVR_ATtiny85__)

class ESP8266_Tiny {
 public:
  // Don't forget: the ESP8266 uses 3.3V logic, so if your host chip isn't
  // already running at 3.3V, use Zener diodes or a level shifter.
  ESP8266_Tiny(int rxPin, int txPin);

  // Connect to an access point.
  bool connect(const char* ssid, const char* password);

  // Open a network connection.
  // 'channel' can be NULL if you aren't using a multiplexed connection.
  // 'host' should be an IP address.
  // 'port' is a network port number; I'm using char instead of int because it
  //        needs to be converted into characters anyway/
  bool openConnection(const char *channel, const char *host, const char *port,
      const char *protocol);

  // Close a network connection.
  // 'channel' can be NULL if you aren't using a multiplexed connection.
  bool closeConnection(const char *channel);

  // Start a TCP server on the specified port.
  bool startServer(const char *port);

  // Send some data on an established connection and expect a response. This
  // uses RAM so make sure the 'data' and 'response' buffers are small. The
  // return value is the number of bytes received.
  // 'channel' can be NULL if you aren't using a multiplexed connection.
  // 'data' is the data to send.
  // 'response' is where the response message should be copied.
  // 'response_len' is the size of the response buffer.
  int sendDataGetResponse(
      char *channel, const char *data, char *response, int response_len);

  // Convenience method for sending data without expecting a reply.
  int sendData(char *channel, const char *data) {
    return sendDataGetResponse(channel, data, NULL, 0);
  }

  // Read incoming data from an established connection. This uses RAM so make
  // sure the 'data' and 'response' buffers are small. The return value is the
  // number of bytes received.
  // 'channel' can be NULL if you aren't using a multiplexed connection.
  // 'response' is where the response message should be copied.
  // 'response_len' is the size of the response buffer.
  // 'deadline' is the absolute time, in milliseconds, to wait for incoming data.
  int readIncoming(
      char *channel, char *response, int response_len, long deadline);

  // Reset ESP8266; if 'echo' is false, local echo will be disabled. Note that
  // if you set echo=true and put a serial-to-USB adapter in parallel with the
  // TX pin from the ESP8266, you can 'eavesdrop' on everything sent or received
  // by the chip. This can be useful if you're trying to debug on an ATTiny that
  // doesn't have hardware serial output.
  bool reset(bool echo=false);

  // Some status/info commands. Right now they aren't useful if you don't have
  // debug output.
  bool getFirmwareInfo();
  bool getAccessPointInfo();
  bool getIPAddress();
  bool getStatus();

 private:
  // Read and discard anything coming from the ESP8266.
  void drain();

  // Get the next character from the ESP8266
  char next(long timeout=100);

  // Send some data, and read until you get 1 of the responses in 'expect'
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

  // Read data until a specified character is seen.
  bool readUntil(const char until, char *buffer, int buf_len,
                 long timeout=1000);

  SoftwareSerial _wifi;
};

#endif  // ESP8266_TINY_H
