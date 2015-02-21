#include <Arduino.h>
#include <ESP8266_Tiny.h>

#define _BUFFER_SIZE 20

ESP8266_Tiny::ESP8266_Tiny(int rxPin, int txPin) : _wifi(rxPin, txPin) {
  _wifi.begin(9600);
}

bool ESP8266_Tiny::connect(const char* ssid, const char* password) {
  const char *connectCmd[] = {"AT+CWJAP=\"", ssid, "\",\"",
                              password, "\""};
  const char *modeExpect[] = { "OK\r", "no change\r" };
  // TODO: consider breaking the mode command out into its own method.
  return sendExpect("AT+CWMODE=1" , modeExpect, 2) &&
         sendExpect(connectCmd, 5, "OK\r", 10000);
}

bool ESP8266_Tiny::openConnection(
    const char *channel, const char *host, const char *port,
    const char *protocol) {
  const char *startCmd[] = {
        "AT+CIPSTART=", channel, channel != NULL? "," : NULL, "\"",
        protocol, "\",\"", host, "\",", port };
  return sendExpect(startCmd, 9, ">\r");
}

bool ESP8266_Tiny::closeConnection(const char *channel) {
  const char *stopCmd[] = {
        "AT+CIPCLOSE", channel != NULL ? "=" : NULL, channel };
  return sendExpect(stopCmd, 3 , "OK\r");
}

bool ESP8266_Tiny::startServer(const char* port) {
  const char *serverCmd[] = { "AT+CIPSERVER=1," , port };
  sendExpect("AT+CIPMUX=1", "OK\r");
  return sendExpect(serverCmd, 2, "OK\r");
}

int ESP8266_Tiny::sendDataGetResponse(
    char *channel, const char *data, char *response, int response_len) {
  drain();
  char dataLen[5];
  // Add 2 to data length to account for '\r\n'
  itoa(strlen(data) + 2, dataLen, 10);
  const char *sendCmd[] = {
      "AT+CIPSEND=", channel, channel != NULL ? "," : NULL,
      dataLen, "\r\n", data, "\r\n" };

  long deadline = millis() + 10000;
  for (int i = 0; i < 7; i++) {
    if (NULL != sendCmd[i]) {
      _wifi.print(sendCmd[i]);
      DEBUG_PRINT(sendCmd[i]);
    }
  }
  DEBUG_PRINT("\n");
  _wifi.flush();
  if (NULL == response) {
    return 0;
  }
  response[0] = '\0';
  return readIncoming(channel, response, response_len, deadline);
}

int ESP8266_Tiny::readIncoming(
    char *channel, char *response, int response_len, long deadline) {
  int match = 0;
  int length;
  // response is +IPD,<len>:<data>
  // TODO: consider using readUntil for some of this.
  while (millis() < deadline) {
    if (next() == 'I' &&
        next() == 'P' &&
        next() == 'D' &&
        next() == ',') {
      if (NULL != channel) {
        *channel = next();
        if (next() != ',') {
          return -1;
        }
        channel[1] = '\0';
      }
      // TODO: we can't receive very much; consider chunking?
      char dataLen[4];
      int idx = 0;
      while (idx < 3) {
        char c = next();
        if (c == ':') break;
        dataLen[idx++] = c;
      }
      dataLen[idx] = '\0';
      length = atoi(dataLen);
      if (length < 0 || length > response_len -1) {
        return -1;
      }
      for (int i = 0; i < length; ++i) {
        response[i] = next();
      }
      response[length] = '\0';
      return length;
    }
  }
  return -1;
}

bool ESP8266_Tiny::reset(bool echo) {
  bool result = sendExpect("AT+RST", "ready\r");
  if (result && !echo) {
    return sendExpect("ATE0", "OK\r");
  }
  return result;
}

// For all of these, add some kind of response buffer.
bool ESP8266_Tiny::getFirmwareInfo() {
  return sendExpect("AT+GMR", "OK\r");
}

bool ESP8266_Tiny::getAccessPointInfo() {
  return sendExpect("AT+CWJAP?",  "OK\r");
}

bool ESP8266_Tiny::getIPAddress() {
  return sendExpect("AT+CIFSR", "OK\r");
}

bool ESP8266_Tiny::getStatus() {
  return sendExpect("AT+CIPSTATUS", "OK\r");
}

void ESP8266_Tiny::drain() {
  while (_wifi.available()) {
    _wifi.read();
  }
}

char ESP8266_Tiny::next(long timeout) {
  long deadline = millis() + timeout;
  while (!_wifi.available() && millis() < deadline) { }
  return _wifi.read();
}

bool ESP8266_Tiny::sendExpect(
    const char **send, int send_parts, const char **expect, int expect_parts,
    long timeout) {
  drain();
#ifdef _WANT_DEBUG
  DEBUG_PRINT("sending ");
  for (int i = 0; i < send_parts; i++) {
    if (NULL != send[i]) {
      DEBUG_PRINT(send[i]);
    }
  }
  DEBUG_PRINT("\n");
#endif  // _WANT_DEBUG
  long deadline = millis() + timeout;
  for (int i = 0; i < send_parts; i++) {
    if (NULL != send[i]) {
      _wifi.write(send[i]);
    }
  }
  // All commands need a '\r\n' at the end; add it here instead of making all
  // of the callers add it to their send strings.
  _wifi.write("\r\n");

  // Don't flush; some commands respond so quickly that we lose some of the
  // response.
  char buffer[_BUFFER_SIZE];
  while (millis() < deadline) {
    if (readUntil('\n', buffer, _BUFFER_SIZE, timeout)) {
      for (int i = 0; i < expect_parts; ++i) {
        if (strcmp(buffer, expect[i]) == 0) {
          return true;
        }
      }
    }
  }
  return false;
}

bool ESP8266_Tiny::readUntil(
    const char until, char *buffer, int buf_len, long timeout) {
  int idx = 0;
  long deadline = millis() + timeout;
  bool found = false;
  while (millis() < deadline && idx < buf_len -1) {
    if (_wifi.available()) {
      char c = _wifi.read();
      if (c != until) {
        buffer[idx++] = c;
      } else {
        found = true;
        break;
      }
    }
  }
  buffer[idx] = '\0';
  DEBUG_PRINT("read ");
  DEBUG_PRINTLN(buffer);
  return found;
}
