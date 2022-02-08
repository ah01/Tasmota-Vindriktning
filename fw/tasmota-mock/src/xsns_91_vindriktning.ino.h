/*
  xsns_91_vindriktning.ino - IKEA vindriktning particle concentration sensor support for Tasmota

  Copyright (C) 2021  Marcel Ritter and Theo Arends

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef USE_VINDRIKTNING
/*********************************************************************************************\
 * IKEA VINDRIKTNING PM2.5 particle concentration sensor
 *
 * This sensor uses a subset of the PM1006K LED particle sensor
 * To use Tasmota the user needs to add an ESP8266 or ESP32
\*********************************************************************************************/

#define XSNS_91                   91

//#define VINDRIKTNING_SHOW_PM1         // Display undocumented/supposed PM1.0 values
//#define VINDRIKTNING_SHOW_PM10        // Display undocumented/supposed PM10 values

#include <TasmotaSerial.h>

#define VINDRIKTNING_DATASET_SIZE 20

uint8_t const VINDRIKTNING_PM1006_READ_CMD[] = { 0x11, 0x02, 0x0B, 0x01, 0xE1 };

// How long should be fan ON before first measurement
constexpr uint16_t VINDRIKTNING_T_FAN = 10000;

// Pause between measurements
constexpr uint16_t VINDRIKTNING_T_MEASUREMENT_PAUSE = 20000;

// Measurement timeout
constexpr uint16_t VINDRIKTNING_T_RX_TIMEOUT = 2000;



enum VindriktningState {
  VINDRIKTNING_STATE_DISABLED,
  VINDRIKTNING_STATE_INIT,
  
  VINDRIKTNING_STATE_RX_ONLY, // 

  VINDRIKTNING_STATE_FAN_ON,
  VINDRIKTNING_STATE_TX,
  VINDRIKTNING_STATE_RX,
  VINDRIKTNING_STATE_RX_WAIT,
  VINDRIKTNING_STATE_TX_DELAY,
  VINDRIKTNING_STATE_TX_TIMEOUT,
  VINDRIKTNING_STATE_FAN_OFF
  
};

struct VINDRIKTNING 
{

#ifdef VINDRIKTNING_SHOW_PM1
  uint16_t pm1_0 = 0;
#endif  // VINDRIKTNING_SHOW_PM1
  uint16_t pm2_5 = 0;
#ifdef VINDRIKTNING_SHOW_PM10
  uint16_t pm10 = 0;
#endif  // VINDRIKTNING_SHOW_PM10


  uint8_t state = VINDRIKTNING_STATE_DISABLED;
  unsigned long state_eneter_millis = 0;

  uint8_t valid = 0; // [sec] > 0 -> data is valid, set to 60 when data are read and decrement every sec.

  int fanPin = -1;

  bool discovery_triggered = false;

  TasmotaSerial *serial = NULL;

} Vindriktning;

// --- helpers ----------------------------------------------------------------

void VindriktningChangeState(uint8_t newSate)
{
  //Serial.print("New state: ");
  //Serial.println(newSate);

  Vindriktning.state = newSate;
  Vindriktning.state_eneter_millis = millis();
}

uint32_t VindriktningTimeInState()
{
  auto t = millis() - Vindriktning.state_eneter_millis;
  return t;
}

void VindriktningFanSet(bool on)
{
  if (Vindriktning.fanPin == -1)
    return;

  if (on)
    AddLog(LOG_LEVEL_DEBUG, PSTR("VINDRIKTNING: Fan ON"));
  else
    AddLog(LOG_LEVEL_DEBUG, PSTR("VINDRIKTNING: Fan OFF"));

  digitalWrite(Vindriktning.fanPin, on);
}


// --- xxx ----------------------------------------------------------------

bool VindriktningReadData(void) 
{
  
  if (!Vindriktning.serial->available()) {
    return false;
  }
  while ((Vindriktning.serial->peek() != 0x16) && Vindriktning.serial->available()) {
    Vindriktning.serial->read();
  }
  if (Vindriktning.serial->available() < VINDRIKTNING_DATASET_SIZE) {
    return false;
  }

  uint8_t buffer[VINDRIKTNING_DATASET_SIZE];
  Vindriktning.serial->readBytes(buffer, VINDRIKTNING_DATASET_SIZE);
  Vindriktning.serial->flush();  // Make room for another burst

  AddLogBuffer(LOG_LEVEL_DEBUG_MORE, buffer, VINDRIKTNING_DATASET_SIZE);

  uint8_t crc = 0;
  for (uint32_t i = 0; i < VINDRIKTNING_DATASET_SIZE; i++) {
    crc += buffer[i];
  }
  if (crc != 0) {
    AddLog(LOG_LEVEL_DEBUG, PSTR("VDN: " D_CHECKSUM_FAILURE));
    return false;
  }

  // sample data:
  //  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
  // 16 11 0b 00 00 00 0c 00 00 03 cb 00 00 00 0c 01 00 00 00 e7
  //               |pm2_5|     |pm1_0|     |pm10 |        | CRC |
  Vindriktning.pm2_5 = (buffer[5] << 8) | buffer[6];
#ifdef VINDRIKTNING_SHOW_PM1
  Vindriktning.pm1_0 = (buffer[9] << 8) | buffer[10];
#endif  // VINDRIKTNING_SHOW_PM1
#ifdef VINDRIKTNING_SHOW_PM10
  Vindriktning.pm10 = (buffer[13] << 8) | buffer[14];
#endif  // VINDRIKTNING_SHOW_PM10

  if (!Vindriktning.discovery_triggered) {
    TasmotaGlobal.discovery_counter = 1;      // force TasDiscovery()
    Vindriktning.discovery_triggered = true;
  }
  
  return true;
}

/*********************************************************************************************/

void VindriktningSecond(void) {                // Every second

  if (Vindriktning.state == VINDRIKTNING_STATE_DISABLED)
    return;

  if (Vindriktning.valid > 0)
  {
    if (Vindriktning.valid == 1)
    {
      AddLog(LOG_LEVEL_DEBUG, PSTR("VINDRIKTNING: Measurement too old"));
    }
    Vindriktning.valid--;
  }

  switch (Vindriktning.state)
  {
    case VINDRIKTNING_STATE_INIT:
    {
      AddLog(LOG_LEVEL_DEBUG, PSTR("VINDRIKTNING: Start measurement loop"));
      if (Vindriktning.fanPin != -1)
      {
        VindriktningFanSet(true);
        VindriktningChangeState(VINDRIKTNING_STATE_FAN_ON);
      }
      else
      {
        VindriktningChangeState(VINDRIKTNING_STATE_TX);
      }
      break;
    }

    case VINDRIKTNING_STATE_RX_ONLY:
    {
      if (VindriktningReadData()) {
        Vindriktning.valid = 60;
      }
      break;
    }

    case VINDRIKTNING_STATE_FAN_ON:
    {
      if (VindriktningTimeInState() > VINDRIKTNING_T_FAN)
      {
        VindriktningChangeState(VINDRIKTNING_STATE_TX);
      }
      break;
    }

    case VINDRIKTNING_STATE_TX: // Tx measurement request to PM1006
    {
      AddLog(LOG_LEVEL_DEBUG, PSTR("VINDRIKTNING: Send request"));
      Vindriktning.serial->write(VINDRIKTNING_PM1006_READ_CMD, 5);
      VindriktningChangeState(VINDRIKTNING_STATE_RX_WAIT);
      break;
    }

    case VINDRIKTNING_STATE_RX_WAIT: // Wait for responce from PM1006
    {
      if (VindriktningTimeInState() > 5000) // timeout
      {
        AddLog(LOG_LEVEL_ERROR, PSTR("VINDRIKTNING: Rx timeout"));
        VindriktningChangeState(VINDRIKTNING_STATE_INIT); // start over
      }

      if (VindriktningReadData())
      {
        Vindriktning.valid = 60;
        AddLog(LOG_LEVEL_DEBUG, PSTR("VINDRIKTNING: Measurement done (value: %i)"), Vindriktning.pm2_5);
        VindriktningFanSet(false);
        VindriktningChangeState(VINDRIKTNING_STATE_FAN_OFF);
      }

      break;
    }

    case VINDRIKTNING_STATE_FAN_OFF: // Wait for responce from PM1006
    {
      if (VindriktningTimeInState() > VINDRIKTNING_T_MEASUREMENT_PAUSE) // timeout
      {
        VindriktningChangeState(VINDRIKTNING_STATE_INIT);
      }
      break;
    }

  }
}

/*********************************************************************************************/

void VindriktningInit(void) {
  
  // original:
  /*
  if (PinUsed(GPIO_VINDRIKTNING_RX)) {
    VindriktningSerial = new TasmotaSerial(Pin(GPIO_VINDRIKTNING_RX), -1, 1);
    if (VindriktningSerial->begin(9600)) {
      if (VindriktningSerial->hardwareSerial()) { ClaimSerial(); }
      Vindriktning.type = 1;
    }
  }
  */
  // 

  // RX PIN
  if (PinUsed(GPIO_VINDRIKTNING_RX)) 
  {
    // TX PIN
    int tx_pin = -1;
    int rx_pin = Pin(GPIO_VINDRIKTNING_RX);
    

    if (PinUsed(GPIO_VINDRIKTNING_TX))
    {
      tx_pin = Pin(GPIO_VINDRIKTNING_TX);
    } 
    
    AddLog(LOG_LEVEL_DEBUG, PSTR("VINDRIKTNING: PM1006 pins: rx: %i, tx: %i"), rx_pin, tx_pin);

    Vindriktning.serial = new TasmotaSerial(rx_pin, tx_pin, 1);

    if (Vindriktning.serial->begin(9600)) {
      if (Vindriktning.serial->hardwareSerial()) { 
        ClaimSerial(); 
      }
    }

    // FAN PIN
    if (PinUsed(GPIO_VINDRIKTNING_FAN))
    {
      Vindriktning.fanPin = Pin(GPIO_VINDRIKTNING_FAN);
      pinMode(Vindriktning.fanPin, OUTPUT);
    }

    AddLog(LOG_LEVEL_DEBUG, PSTR("VINDRIKTNING: Fan pin: %i"), Vindriktning.fanPin);
    
    if (tx_pin != -1)
    {
      VindriktningChangeState(VINDRIKTNING_STATE_INIT);
    }
    else
    {
      AddLog(LOG_LEVEL_DEBUG, PSTR("VINDRIKTNING: Tx pin not defined, RX only mode"));
      VindriktningChangeState(VINDRIKTNING_STATE_RX_ONLY);
    }

  }
  else 
  {
    AddLog(LOG_LEVEL_DEBUG, PSTR("VINDRIKTNING: Tx pin not defined, disabled"));
  }
}

#ifdef USE_WEBSERVER
const char HTTP_VINDRIKTNING_SNS[] PROGMEM =
  "{s}VINDRIKTNING " D_ENVIRONMENTAL_CONCENTRATION " %s " D_UNIT_MICROMETER "{m}%d " D_UNIT_MICROGRAM_PER_CUBIC_METER "{e}";      // {s} = <tr><th>, {m} = </th><td>, {e} = </td></tr>
#endif  // USE_WEBSERVER

void VindriktningShow(bool json) {
  if (Vindriktning.valid) {
    if (json) {
      ResponseAppend_P(PSTR(",\"VINDRIKTNING\":{"));
#ifdef VINDRIKTNING_SHOW_PM1
      ResponseAppend_P(PSTR("\"PM1\":%d,"), Vindriktning.pm1_0);
#endif  // VINDRIKTNING_SHOW_PM1
      ResponseAppend_P(PSTR("\"PM2.5\":%d"), Vindriktning.pm2_5);
#ifdef VINDRIKTNING_SHOW_PM10
      ResponseAppend_P(PSTR(",\"PM10\":%d"), Vindriktning.pm10);
#endif  // VINDRIKTNING_SHOW_PM10
      ResponseJsonEnd();
#ifdef USE_DOMOTICZ
      if (0 == TasmotaGlobal.tele_period) {
#ifdef VINDRIKTNING_SHOW_PM1
        DomoticzSensor(DZ_COUNT, Vindriktning.pm1_0);	   // PM1.0
#endif  // VINDRIKTNING_SHOW_PM1
        DomoticzSensor(DZ_VOLTAGE, Vindriktning.pm2_5);	 // PM2.5
#ifdef VINDRIKTNING_SHOW_PM10
        DomoticzSensor(DZ_CURRENT, Vindriktning.pm10);	 // PM10
#endif  // VINDRIKTNING_SHOW_PM10
      }
#endif  // USE_DOMOTICZ
#ifdef USE_WEBSERVER
    } else {
#ifdef VINDRIKTNING_SHOW_PM1
        WSContentSend_PD(HTTP_VINDRIKTNING_SNS, "1", Vindriktning.pm1_0);
#endif  // VINDRIKTNING_SHOW_PM1
        WSContentSend_PD(HTTP_VINDRIKTNING_SNS, "2.5", Vindriktning.pm2_5);
#ifdef VINDRIKTNING_SHOW_PM10
        WSContentSend_PD(HTTP_VINDRIKTNING_SNS, "10", Vindriktning.pm10);
#endif  // VINDRIKTNING_SHOW_PM10
#endif  // USE_WEBSERVER
    }
  }
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xsns91(uint8_t function) {
  bool result = false;

  switch (function) 
  {
    case FUNC_EVERY_SECOND:
      VindriktningSecond();
      break;

    case FUNC_JSON_APPEND:
      VindriktningShow(1);
      break;

#ifdef USE_WEBSERVER
    case FUNC_WEB_SENSOR:
      VindriktningShow(0);
      break;
#endif  // USE_WEBSERVER

    case FUNC_INIT:
      VindriktningInit();
      break;
  }

  return result;
}

#endif  // USE_VINDRIKTNING