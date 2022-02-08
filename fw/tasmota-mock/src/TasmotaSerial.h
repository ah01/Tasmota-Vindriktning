
#ifndef TasmotaSerial_h
#define TasmotaSerial_h
/*********************************************************************************************\
 * TasmotaSerial supports up to 115200 baud with default buffer size of 64 bytes using optional no iram
 *
 * Based on EspSoftwareSerial v3.4.3 by Peter Lerup (https://github.com/plerup/espsoftwareserial)
\*********************************************************************************************/

#define TM_SERIAL_BAUDRATE           9600   // Default baudrate
#define TM_SERIAL_BUFFER_SIZE        64     // Receive buffer size

#include <inttypes.h>
#include <Stream.h>

// #ifdef ESP32
// #include <HardwareSerial.h>
// #endif

class TasmotaSerial : public Stream {
  public:
    TasmotaSerial(int receive_pin, int transmit_pin, int hardware_fallback = 0);
    virtual ~TasmotaSerial();

    bool begin(uint32_t speed = TM_SERIAL_BAUDRATE, uint32_t config = SERIAL_8N1);
    bool hardwareSerial(void);
    int peek(void);

    size_t write(uint8_t byte) override;
    int read(void) override;
    size_t read(char* buffer, size_t size);
    int available(void) override;
    void flush(void) override;

//    void rxRead(void);

//    uint32_t getLoopReadMetric(void) const { return m_bit_follow_metric; }

// #ifdef ESP32
//     uint32_t getUart(void) const { return m_uart; }
// #endif

    using Print::write;

};

#endif  // TasmotaSerial_h
