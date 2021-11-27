#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "sdkconfig.h"
#include "dataqueue.h"

#include "assert.h"

extern void DQ_AddDataToQueue( QueueHandle_t *q, void * data, dq_type_t type, dq_desc_t desc, char * label )
{
    assert( q != NULL );
    assert( data != NULL );
    assert( label != NULL );

    dq_data_t queueData;

    queueData.type  = type;
    queueData.mqtt_label = label;
    queueData.desc  = desc;

    switch( type )
    { 
        case dq_data_float:
        {
            float * d = data;
            queueData.data.f = *d;
            break;
        }
        case dq_data_uint32:
        {
            queueData.data.ui = *( (uint32_t * ) data );
            break;
        }
        case dq_data_uint16:
        {
            queueData.data.us = *( (uint16_t * ) data );
            break;
        }
        case dq_data_str:
        {
            queueData.data.s = (char * ) data ;
            break;
        }
        default:
        {
            assert( false );
        }
    }

    xQueueSend( *q, ( void *)&queueData, (TickType_t) 0U);
}
