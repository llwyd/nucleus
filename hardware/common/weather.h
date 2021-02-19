#ifndef WEATHER_H
#define WEATHER_H


typedef struct weather_t
{
    float temperature;
    float humidity;
    uint8_t * description;
} weather_t;

void Weather_Init(const uint8_t * loc, const uint8_t * key);
void Weather_Update(void);
float Weather_GetTemperature(void);

#endif /* WEATHER_h */
