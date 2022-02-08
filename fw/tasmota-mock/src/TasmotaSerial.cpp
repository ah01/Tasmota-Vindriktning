
#include <Arduino.h>
#include <TasmotaSerial.h>

TasmotaSerial::TasmotaSerial(int receive_pin, int transmit_pin, int hardware_fallback) {
  Serial.println("TasmotaSerial ctor");
}

TasmotaSerial::~TasmotaSerial(void) {
}

// bool TasmotaSerial::isValidGPIOpin(int pin) {
  
// }


bool TasmotaSerial::begin(uint32_t speed, uint32_t config) {
  
//  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  return true;
}


void TasmotaSerial::flush(void) {
    Serial2.flush();
}

int TasmotaSerial::peek(void) {
    return Serial2.peek();
}

int TasmotaSerial::read(void) {
  return Serial2.read();
}

size_t TasmotaSerial::read(char* buffer, size_t size) {
  return Serial2.read(buffer, size);
}

int TasmotaSerial::available(void) {
  return Serial2.available();
}

size_t TasmotaSerial::write(uint8_t b) {

  // Serial.print(">> ");
  // Serial.println(b, HEX);

  return Serial2.write(b);
}

bool TasmotaSerial::hardwareSerial() {
    return true;
}
