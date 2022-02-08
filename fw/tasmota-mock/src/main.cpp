//#include <Arduino.h>


#define RXD2 16
#define TXD2 17


#define USE_VINDRIKTNING

#define GPIO_VINDRIKTNING_RX 1
#define GPIO_VINDRIKTNING_TX 2
#define GPIO_VINDRIKTNING_FAN 3

#define LOG_LN(msg) Serial.println(msg);


#include <tasmota.h>

#include "xsns_91_vindriktning.ino.h"

static uint sec_counter = 0;
static unsigned long last_sec;

void setup()
{
    Serial.begin(115200);
    Serial.println("\nSTART");

    Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
}

void loop()
{

    if (millis() - last_sec > 1000)
    {
        last_sec = millis();
        

        if (sec_counter == 0)
        {
            LOG_LN("FUNC_INIT")
            Xsns91(FUNC_INIT);
        }
        else
        {
            LOG_LN("")
            Xsns91(FUNC_EVERY_SECOND);
        }
        sec_counter++;
    }
}
