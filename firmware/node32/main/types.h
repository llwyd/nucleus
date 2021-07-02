#ifndef _TYPES_H_
#define _TYPES_H_

#include <inttypes.h>
#include "bme280.h"

typedef enum sensing_type_t
{
    sensing_type_temperature,
    sensing_type_humidity,
    sensing_type_pressure,
} 
sensing_type_t;

typedef union
{
    double f;
    uint32_t ui;
    char * s;
} 
sensing_data_t;

typedef struct sensing_t
{
    char * mqtt_label;
    sensing_type_t type;
    sensing_data_t data;
}
sensing_t;

#endif /* _TYPES_H_*/
