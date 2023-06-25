
#include "comms.h"
#include "weather.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static const uint8_t * location;
static const uint8_t * weather_key;

static uint8_t getPath[128];
static uint8_t weatherDesc[ 128 ];
static uint8_t weatherRaw[2048];

static weather_t weather;

void Weather_Init(const uint8_t * loc, const uint8_t * key)
{
    location = loc;
    weather_key = key;

	weather.description = weatherDesc;

    memset(getPath, 0x00, 128);
    snprintf(getPath, 128,"/data/2.5/weather?q=%s&appid=%s",location,weather_key);
}

void Weather_Update(void)
{
	memset(weatherDesc, 0x00, 128);
    Comms_Get( "api.openweathermap.org", "80", getPath, weatherRaw, 2048);
    Comms_ExtractTemp( weatherRaw, &weather.temperature );
    Comms_ExtractFloat( weatherRaw, &weather.humidity, "\"humidity\"" );
	Comms_ExtractValue( weatherRaw, weather.description, "\"description\":");

    memset(weatherRaw,0x00,2048);
}

float Weather_GetTemperature(void)
{
    return weather.temperature;
}

uint8_t * Weather_GetDescription(void)
{
	return weather.description;
}

