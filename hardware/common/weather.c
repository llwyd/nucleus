
#include "comms.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static uint8_t * location;
static uint8_t * weather_key;

static uint8_t getPath[64];
static uint8_t weatherRaw[512];

static weather_t weather;

void Weather_Init(uint8_t * loc, uint8_t * key)
{
    location = loc;
    weather_key = key;

    memset(getPath, 0x00, 64);
    snprintf(getPath,HTTP_BUFFER_SIZE,"/data/2.5/weather?q=%s&appid=%s",location,weather_key);
}

void Weather_Update(void)
{
    Comms_Get( "api.openweathermap.org", "80", getPath, weatherRaw, 512 );
    Comms_ExtractTemp( httpRcv, &data->outsideTemperature );
    Comms_ExtractFloat( httpRcv, &data->outsideHumidity, "\"humidity\"" );
    printf("%s\n",weatherRaw);
    memset(weatherRaw,0x00,512);
}

void Weather_GetData(weather_t *data)
{
    *data = weather;
}

