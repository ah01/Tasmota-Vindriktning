/**
 * Mock of tasmota functions
 * 
 */

#include <Arduino.h>
#include <stdint.h>
#include <pgmspace.h>

#include <ext_printf.h>

#define D_CHECKSUM_FAILURE

enum XsnsFunctions {FUNC_SETTINGS_OVERRIDE, FUNC_PIN_STATE, FUNC_MODULE_INIT, FUNC_PRE_INIT, FUNC_INIT,
                    FUNC_LOOP, FUNC_EVERY_50_MSECOND, FUNC_EVERY_100_MSECOND, FUNC_EVERY_200_MSECOND, FUNC_EVERY_250_MSECOND, FUNC_EVERY_SECOND,
                    FUNC_SAVE_SETTINGS, FUNC_SAVE_AT_MIDNIGHT, FUNC_SAVE_BEFORE_RESTART,
                    FUNC_AFTER_TELEPERIOD, FUNC_JSON_APPEND, FUNC_WEB_SENSOR, FUNC_COMMAND, FUNC_COMMAND_SENSOR, FUNC_COMMAND_DRIVER,
                    FUNC_MQTT_SUBSCRIBE, FUNC_MQTT_INIT, FUNC_MQTT_DATA,
                    FUNC_SET_POWER, FUNC_SET_DEVICE_POWER, FUNC_SHOW_SENSOR, FUNC_ANY_KEY,
                    FUNC_ENERGY_EVERY_SECOND, FUNC_ENERGY_RESET,
                    FUNC_RULES_PROCESS, FUNC_TELEPERIOD_RULES_PROCESS, FUNC_SERIAL, FUNC_FREE_MEM, FUNC_BUTTON_PRESSED, FUNC_BUTTON_MULTI_PRESSED,
                    FUNC_WEB_ADD_BUTTON, FUNC_WEB_ADD_CONSOLE_BUTTON, FUNC_WEB_ADD_MANAGEMENT_BUTTON, FUNC_WEB_ADD_MAIN_BUTTON,
                    FUNC_WEB_GET_ARG, FUNC_WEB_ADD_HANDLER, FUNC_SET_CHANNELS, FUNC_SET_SCHEME, FUNC_HOTPLUG_SCAN,
                    FUNC_DEVICE_GROUP_ITEM };

enum LoggingLevels {LOG_LEVEL_NONE, LOG_LEVEL_ERROR, LOG_LEVEL_INFO, LOG_LEVEL_DEBUG, LOG_LEVEL_DEBUG_MORE};

//void AddLog(uint32_t loglevel, PGM_P formatP, ...) {
//}

void AddLog(uint32_t loglevel, PGM_P formatP, ...) {
//   uint32_t highest_loglevel = TasmotaGlobal.seriallog_level;
//   if (Settings->weblog_level > highest_loglevel) { highest_loglevel = Settings->weblog_level; }
//   if (Settings->mqttlog_level > highest_loglevel) { highest_loglevel = Settings->mqttlog_level; }
//   if (TasmotaGlobal.syslog_level > highest_loglevel) { highest_loglevel = TasmotaGlobal.syslog_level; }
//   if (TasmotaGlobal.templog_level > highest_loglevel) { highest_loglevel = TasmotaGlobal.templog_level; }
//   if (TasmotaGlobal.uptime < 3) { highest_loglevel = LOG_LEVEL_DEBUG_MORE; }  // Log all before setup correct log level

  // If no logging is requested then do not access heap to fight fragmentation
  //if ((loglevel <= highest_loglevel) && (TasmotaGlobal.masterlog_level <= highest_loglevel)) {
    va_list arg;
    va_start(arg, formatP);
    char* log_data = ext_vsnprintf_malloc_P(formatP, arg);
    va_end(arg);
    if (log_data == nullptr) { return; }

    //AddLogData(loglevel, log_data);

    Serial.print("LOG: ");
    Serial.println(log_data);

    free(log_data);
  //}
}




void AddLogBuffer(uint32_t loglevel, uint8_t *buffer, uint32_t count)
{
    // Serial.print("AddLogBuffer -> ");
    // for(uint32_t i = 0; i < count; i++)
    // {
    //     Serial.print(buffer[i], HEX);
    //     Serial.print(" ");
    // }

    // Serial.println();

   char hex_char[(count * 3) + 2];
   AddLog(loglevel, PSTR("DMP: %s"), ToHex_P(buffer, count, hex_char, sizeof(hex_char), ' '));
}



struct TG {
    uint8_t discovery_counter = 0;
} TasmotaGlobal;

int Pin(uint32_t gpio, uint32_t index = 0);
int Pin(uint32_t gpio, uint32_t index) {

    switch (gpio)
    {
    case GPIO_VINDRIKTNING_RX:
        return 16;
    
    case GPIO_VINDRIKTNING_TX:
        return 17;
        //return -1;

    case GPIO_VINDRIKTNING_FAN:
        return 12;
        //return -1;

    default:
        break;
    }

    return -1;
}

bool PinUsed(uint32_t gpio, uint32_t index = 0);

bool PinUsed(uint32_t gpio, uint32_t index) {
  return (Pin(gpio, index) >= 0);
}

void ClaimSerial(void) {}


#define TM_SERIAL_BAUDRATE 9600
//#define SERIAL_8N1



int ResponseAppend_P(const char* format, ...)  // Content send snprintf_P char data
{
    return 0;
}

int ResponseJsonEnd(void)
{
  return 0;
}
