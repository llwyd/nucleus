#ifndef _DATA_QUEUE_H
#define _DATA_QUEUE_H

#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

typedef union dq_val_t
{
    double f;
    uint32_t ui;
    uint16_t us;
    char * s;
}
dq_val_t;

typedef enum dq_type_t
{
    dq_data_float,
    dq_data_uint32,
    dq_data_uint16,
    dq_data_str,
}
dq_type_t;

typedef enum dq_desc_t
{
    dq_desc_temperature,
    dq_desc_humidity,
    dq_desc_pressure,
    dq_desc_weather,
}
dq_desc_t;

typedef struct dq_data_t
{
    char * mqtt_label;
    dq_desc_t desc;
    dq_type_t type;
    dq_val_t data;
}
dq_data_t;

extern void DQ_AddDataToQueue( QueueHandle_t *q, void * data, dq_type_t type, dq_desc_t desc, char * label );

#endif  /* _DATA_QUEUE_H */
