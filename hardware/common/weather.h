#ifndef WEATHER_H
#define WEATHER_H


typedef struct weather_t
{
    float temperature;
    float humidity;
    uint8_t * description;
} weather_t;

void Weather_Init(uint8_t * loc, uint8_t * key);
void Weather_Update(void);
void Weather_GetData(weather_t *data);


#endif /* WEATHER_h */
