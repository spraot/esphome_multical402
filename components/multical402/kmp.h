#pragma once

#include "esphome/core/log.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace multical402 {

static const char *const KMP_TAG = "multical402";

// Kamstrup timeout after transmit
static const unsigned char KMP_TIMEOUT_MS = 250;

enum DestinationAddress : unsigned char {
  HEAT_METER = 0x3f,
  LOGGER_TOP = 0x7f,
  LOGGER_BASE = 0xbf,
};

// Kamstrup Multical 402/403/602
// The registers we want to get out of the meter
static const unsigned int registerIds[] = {
    0x003C,  // 60 - Heat energy - Standard resolution (1 kWh)
    0x0050,  // 80 - Current power
    0x0056,  // 86 - Current forward temperature
    0x0057,  // 87 - Current return temperature
    0x0059,  // 89 - Current differential temperature
    0x004A,  // 74 - Current water flow
    0x0044,  // 68 - Volume register V1
    0x010A,  // 266 - Heat energy - Multical 403 High resolution (1 Wh)
    0x009B   // 155 - Heat energy - Multical 602 High resolution (0.01 kWh)
};

class KMP {
 public:
  KMP(uart::UARTDevice *uart) : uart_(uart) {}

  float HeatEnergy() { return this->Read(registerIds[0]); }
  float CurrentPower() { return this->Read(registerIds[1]); }
  float CurrentForwardTemperature() { return this->Read(registerIds[2]); }
  float CurrentReturnTemperature() { return this->Read(registerIds[3]); }
  float CurrentDifferentialTemperature() { return this->Read(registerIds[4]); }
  float CurrentWaterFlow() { return this->Read(registerIds[5]); }
  float Volume() { return this->Read(registerIds[6]); }
  float HeatEnergy_hires_403() { return this->Read(registerIds[7]); }
  float HeatEnergy_high() { return this->Read(registerIds[8]); }

 private:
  uart::UARTDevice *uart_;

  // kamReadReg - read a Kamstrup register
  float Read(unsigned int registerId) {
    char recvmsg[40];  // buffer of bytes to hold the received data
    float rval = -1;   // this will hold the final value

    // prepare message to send and send it
    char sendmsg[] = {HEAT_METER, 0x10, 0x01, static_cast<char>(registerId >> 8),
                      static_cast<char>(registerId & 0xff)};
    this->Send(sendmsg, 5);

    // listen if we get an answer
    unsigned short rxnum = this->Receive(recvmsg);

    if (rxnum != 0) {
      rval = this->Decode(registerId, recvmsg);
    }

    return rval;
  }

  // kamSend - send data to Kamstrup meter
  void Send(char const *msg, int msgsize) {
    // append checksum bytes to message
    char newmsg[msgsize + 2];
    for (int i = 0; i < msgsize; i++) {
      newmsg[i] = msg[i];
    }
    newmsg[msgsize++] = 0x00;
    newmsg[msgsize++] = 0x00;
    int c = this->crc_1021(newmsg, msgsize);
    newmsg[msgsize - 2] = (c >> 8);
    newmsg[msgsize - 1] = c & 0xff;

    // build final transmit message - escape various bytes
    unsigned char txmsg[20] = {0x80};  // prefix
    unsigned int txsize = 1;
    for (int i = 0; i < msgsize; i++) {
      if (newmsg[i] == 0x06 or newmsg[i] == 0x0d or newmsg[i] == 0x1b or newmsg[i] == 0x40 or
          newmsg[i] == 0x80) {
        txmsg[txsize++] = 0x1b;
        txmsg[txsize++] = newmsg[i] ^ 0xff;
      } else {
        txmsg[txsize++] = newmsg[i];
      }
    }
    txmsg[txsize++] = 0x0d;  // EOL

    uart_->write_array(txmsg, txsize);
  }

  // kamReceive - receive bytes from Kamstrup meter
  unsigned short Receive(char recvmsg[]) {
    char rxdata[50];  // buffer to hold received data
    unsigned long rxindex = 0;
    unsigned long starttime = millis();

    uart_->flush();  // flush serial buffer - might contain noise

    char r = 0;

    // loop until EOL received or timeout
    while (r != 0x0d) {
      if (millis() - starttime > KMP_TIMEOUT_MS) {
        ESP_LOGW(KMP_TAG, "Timed out listening for data");
        return 0;
      }

      if (uart_->available()) {
        r = uart_->read();
        if (r != 0x40) {  // don't append if we see the start marker
          rxdata[rxindex] = r;
          rxindex++;
        }
      }
    }

    // remove escape markers from received data
    unsigned short j = 0;
    for (unsigned short i = 0; i < rxindex - 1; i++) {
      if (rxdata[i] == 0x1b) {
        char v = rxdata[i + 1] ^ 0xff;
        if (v != 0x06 and v != 0x0d and v != 0x1b and v != 0x40 and v != 0x80) {
          ESP_LOGW(KMP_TAG, "Missing escape %X", v);
        }
        recvmsg[j] = v;
        i++;  // skip
      } else {
        recvmsg[j] = rxdata[i];
      }
      j++;
    }

    // check CRC
    if (this->crc_1021(recvmsg, j)) {
      ESP_LOGW(KMP_TAG, "CRC error: ");
      return 0;
    }

    return j;
  }

  // kamDecode - decodes received data
  float Decode(const unsigned int registerId, const char *msg) {
    // skip if message is not valid
    if (msg[0] != 0x3f or msg[1] != 0x10) {
      return false;
    }
    if (msg[2] != (registerId >> 8) or msg[3] != (registerId & 0xff)) {
      return false;
    }

    // decode the mantissa
    long x = 0;
    for (int i = 0; i < msg[5]; i++) {
      x <<= 8;
      x |= msg[i + 7];
    }

    // decode the exponent
    int i = msg[6] & 0x3f;
    if (msg[6] & 0x40) {
      i = -i;
    }
    float ifl = pow(10, i);
    if (msg[6] & 0x80) {
      ifl = -ifl;
    }

    return (float) (x * ifl);
  }

  // crc_1021 - calculate crc16
  long crc_1021(char const *inmsg, unsigned int len) {
    long creg = 0x0000;
    for (unsigned int i = 0; i < len; i++) {
      int mask = 0x80;
      while (mask > 0) {
        creg <<= 1;
        if (inmsg[i] & mask) {
          creg |= 1;
        }
        mask >>= 1;
        if (creg & 0x10000) {
          creg &= 0xffff;
          creg ^= 0x1021;
        }
      }
    }
    return creg;
  }
};

}  // namespace multical402
}  // namespace esphome
