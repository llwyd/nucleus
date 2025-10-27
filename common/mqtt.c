#include "mqtt.h"

#include <time.h>
#include <assert.h>

#define MQTT_PORT ( "1883" )
#define BUFFER_SIZE (128)
#define ID_BUFFER_SIZE (32)

#define MQTT_CONNACK_CODE   ( 0x20 )
#define MQTT_SUBACK_CODE    ( 0x90 )

#define MQTT_PUBLISH_CODE   ( 0x30 )
#define MQTT_PUBACK_CODE    ( 0x40 )
#define MQTT_PINGRESP_CODE  ( 0xD0 )
#define MQTT_DISCONNECT_CODE  ( 0x00 )

typedef struct
{
    mqtt_msg_type_t msg_type;
    uint16_t seq_num;
}
mqtt_resp_t;

typedef struct mqtt_pub_t
{
    char * name;
    char * data;
} mqtt_pub_t;

typedef struct msg_id_t
{
    unsigned char read_index;
    unsigned char write_index;
    unsigned char fill;
    uint16_t id[ID_BUFFER_SIZE];
} msg_id_t;


static void IncrementSeqNum(mqtt_t * mqtt)
{
    mqtt->state.seq_num++;
    if(mqtt->state.seq_num == 0U)
    {
        uint32_t seq_num = 1u;
        const uint32_t fill = mqtt->pq->fill;
        for(uint32_t idx = 0; idx < fill; idx++)
        {
            (void)PQ_DecreaseKey(mqtt->pq, idx, seq_num);
            seq_num++;
        }

        mqtt->state.seq_num = seq_num;
    }
}

extern void MQTT_Init( mqtt_t * mqtt,
                char * client_name,
                mqtt_subs_t * subs,
                pq_t * pq,
                mqtt_msg_t * pool
                )
{
    assert(mqtt != NULL);
    assert(client_name != NULL);

    memset(mqtt, 0x00, sizeof(mqtt_t));

    mqtt->pq = pq;
    mqtt->subs = subs;
    mqtt->client_name = client_name;    
    mqtt->state.subs = 0u;
    mqtt->state.seq_num = 1u;
    mqtt->state.status = MQTT_OK;
    mqtt->state.rx_seq_num = 1u;
    PQ_Init(mqtt->pq, &pool->key, sizeof(mqtt_msg_t));

    for(uint32_t idx = 0; idx < PQ_FULL_LEN; idx++)
    {
        pool[idx].size = 0u;
        pool[idx].seq_num = 0u;
        pool[idx].type = MQTT_NONE;
        pool[idx].timestamp = 0u;
        memset(pool[idx].msg, 0x00, MQTT_MSG_SIZE); 
    }
}

extern mqtt_msg_t * MQTT_Encode(mqtt_t * mqtt,
        mqtt_msg_type_t type,
        uint8_t * data_buffer, 
        uint16_t data_len,
        mqtt_msg_params_t * params
        )
{
    (void)mqtt;
    (void)data_buffer;
    (void)data_len;
    uint16_t full_packet_size = 0;
    mqtt_msg_t * ret = NULL;
    printf("\tPQ: Fill: %lu / %lu\n", mqtt->pq->fill, mqtt->pq->max);
    if(PQ_IsFull(mqtt->pq))
    {
        if (type != MQTT_CONNECT)
        {
            printf("\tMQTT: Buffer full, exiting\n");
            goto cleanup;
        }
    }

    switch(type)
    {
        case MQTT_CONNECT:
        {
            /* Reset the state */
            PQ_Flush(mqtt->pq);

            /* Store message but dont actually place in the PQ */
            mqtt_msg_t * buffer = (mqtt_msg_t *)PQ_Cache(mqtt->pq);
            memset(buffer->msg,0x00, MQTT_MSG_SIZE);
            /* Message Template for mqtt connect */
            uint8_t mqtt_template_connect[] =
            {
                        0x10,                                   /* connect */
                        0x00,                                   /* payload size */
                        0x00, 0x04,                             /* Protocol name size */
                        'M', 'Q', 'T', 'T',                     /* MQTT */
                        0x05,                                   /* Version MQTT v5 */
                        0x02,                                   /* Clean Session, No Will */
                        0x00, MQTT_TIMEOUT,                     /* Keep alive timeout */
                        0x05,                                   /* Properties Length */
                        0x27,                                   /* Max packet size */
                        0x00, 0x00, 0x00, 0x80,                 /* 128 max packet size */
            };
            uint8_t * msg_ptr = buffer->msg;
            uint16_t packet_size = (uint16_t)sizeof( mqtt_template_connect );
            uint16_t name_size = strlen( mqtt->client_name );

            memcpy( msg_ptr, mqtt_template_connect, packet_size );
            msg_ptr+= packet_size;
            msg_ptr++;
            *msg_ptr++ = (uint8_t)(name_size & 0xFF);
            memcpy(msg_ptr, mqtt->client_name, name_size);
                        
            uint16_t total_packet_size = packet_size + name_size;
            buffer->msg[1] = (uint8_t)(total_packet_size&0xFF);
            full_packet_size = total_packet_size + 2;
            buffer->type = MQTT_CONNECT; 
            buffer->seq_num = 0u;
            buffer->size = full_packet_size;
            
            mqtt->state.subs = 0u;
            mqtt->state.seq_num = 1u;
            ret = buffer;
            break;
        }
        case MQTT_SUBSCRIBE:
        {
            const uint16_t seq_num = mqtt->state.seq_num;

            mqtt_msg_t * buffer = (mqtt_msg_t *)PQ_Push(mqtt->pq, seq_num);
            buffer->seq_num = seq_num;
            memset(buffer->msg,0x00, MQTT_MSG_SIZE);
            const unsigned char mqtt_template_subscribe[] =
            {
                0x82,
                0x00,   // length of message
            };
            uint8_t * msg_ptr = buffer->msg;
            uint16_t packet_size = (uint16_t)sizeof( mqtt_template_subscribe );
            memcpy( msg_ptr, mqtt_template_subscribe, packet_size );
            msg_ptr+= packet_size;
            
            *msg_ptr++ = (uint8_t)((seq_num >> 8U)&0xFF);
            *msg_ptr++ = (uint8_t)(seq_num&0xFF);        
            *msg_ptr++ = 0u;
            
            msg_ptr++;
            *msg_ptr++ = (uint8_t)(data_len & 0xFF);
            memcpy(msg_ptr, data_buffer, data_len); 
            msg_ptr+=data_len;
            *msg_ptr |= (0x1);
                    
            uint16_t total_packet_size = packet_size + data_len + 1 + 2 + 1;
            buffer->msg[1] = (uint8_t)(total_packet_size&0xFF);
            full_packet_size = total_packet_size + 2;
            
            buffer->type = MQTT_SUBSCRIBE;
            buffer->size = full_packet_size;
            buffer->timestamp = params->timestamp; 
            
            ret = buffer;
            IncrementSeqNum(mqtt);
            break;
        }
        case MQTT_PUBLISH:
        {
            assert(params != NULL);
            assert(params->qos < 3u);
            const uint16_t seq_num = mqtt->state.seq_num;

            mqtt_msg_t * buffer = NULL;
            if(params->qos > 0u)
            {
                buffer = (mqtt_msg_t *)PQ_Push(mqtt->pq, seq_num);
            }
            else
            {
                buffer = (mqtt_msg_t *)PQ_Cache(mqtt->pq);
            }
            buffer->seq_num = seq_num;
            memset(buffer->msg,0x00, MQTT_MSG_SIZE);
            uint8_t * msg_ptr = buffer->msg;
            uint16_t topic_size = strlen((char*)params->topic);
            if(!params->global)
            {
                topic_size += strlen((char*)mqtt->client_name);
                topic_size += 1;
            }
            *msg_ptr = MQTT_PUBLISH_CODE;
            if( params->qos > 0u )
            {
                *msg_ptr |= (1u << params->qos);
            }
            msg_ptr++;
            msg_ptr++; /* This is where message length would go */ 
            msg_ptr++; /* MSB topic length */
            *msg_ptr++ = (uint8_t)(topic_size & 0xFF);
            memcpy(msg_ptr, params->topic, strlen((char*)params->topic));
            msg_ptr+=strlen((char*)params->topic);
          
            /* Append client name to topic if not global transmission */
            if(!params->global)
            {
                *msg_ptr++ = (uint8_t)'/';
                memcpy(msg_ptr, mqtt->client_name, strlen((char*)mqtt->client_name));
                msg_ptr+=strlen((char*)mqtt->client_name);
            }

            if(params->qos > 0u)
            {
                *msg_ptr++ = (uint8_t)((seq_num >> 8U)&0xFF);
                *msg_ptr++ = (uint8_t)(seq_num&0xFF);        
            }
            *msg_ptr++ = 0u;
            
            memcpy(msg_ptr, data_buffer, data_len);
    
            /* Topic len + topic + data + msg id */
            uint8_t total_packet_size = (uint8_t)strlen((char*)params->topic) + data_len + 2 + 1;

            if(params->qos > 0u)
            {
                total_packet_size += 2u;
            }

            if(!params->global)
            {
                total_packet_size += (uint8_t)strlen((char*)mqtt->client_name) + 1;
            }
            buffer->msg[1] = (uint8_t)(total_packet_size&0xFF);
            /* header and size byte */
            full_packet_size = total_packet_size + 2;
            
            buffer->type = MQTT_PUBLISH;
            buffer->seq_num = mqtt->state.seq_num;
            buffer->size = full_packet_size;
            buffer->timestamp = params->timestamp; 
            printf("\tseqnum: %u\n",buffer->seq_num);
            
            if(params->qos > 0u)
            {
                IncrementSeqNum(mqtt);
            } 
            /* If qos is 0, then we can immediately dequeue */
            ret = buffer;
            break;
        }
        case MQTT_PUBACK:
        {
            /* Store message but dont actually place in the PQ */
            mqtt_msg_t * buffer = (mqtt_msg_t *)PQ_Cache(mqtt->pq);
            memset(buffer->msg,0x00, MQTT_MSG_SIZE);
            
            uint8_t * msg_ptr = buffer->msg;
            *msg_ptr++ = MQTT_PUBACK_CODE;
            *msg_ptr++ = 0x4; /* Remaining Length */
            *msg_ptr++ = (uint8_t)((mqtt->state.rx_seq_num >> 8U)&0xFF);
            *msg_ptr++ = (uint8_t)(mqtt->state.rx_seq_num&0xFF);        
            *msg_ptr++ = 0x00; /* Reason code */
            *msg_ptr++ = 0x00; /* Remaining properties */
            
            full_packet_size = 6u;
            
            buffer->type = MQTT_PUBACK;
            buffer->seq_num = mqtt->state.rx_seq_num;
            buffer->size = full_packet_size;
            buffer->timestamp = params->timestamp;
            printf("\t Sending PUBACK for %u\n", buffer->seq_num);
            ret = buffer;
            break;
        }
        default:
        {
            assert(false);
            break;
        }
    }
    printf("\tMQTT: PQ fill: %lu\n", mqtt->pq->fill);
cleanup:
    return ret;
}

extern bool MQTT_Decode( mqtt_t * mqtt, uint8_t * buffer, uint16_t len)
{
    bool success = false;
    uint8_t return_code = ( buffer[0] & 0xF0 );
    uint8_t qos = (buffer[0] >> 1u) & 0x03;
    uint8_t msg_length = buffer[1];
    mqtt->state.status = MQTT_OK;
    (void)msg_length;
    (void)len;
    
    switch( return_code )
    {
        case MQTT_CONNACK_CODE:
        {
            printf("\tCONNACK Received\n");
            /* Pop the previous message and check whether the last
             * was a connect packet */
            mqtt_msg_t * b = (mqtt_msg_t *)PQ_Cache(mqtt->pq);

            bool resp_type = (b->type == MQTT_CONNECT);
            bool conn_success = (buffer[3] == 0x00);
            /*TODO handle other params, dont care for now */

            success = resp_type && conn_success;
            break;
        }
        case MQTT_SUBACK_CODE:
        {
            printf("\tSUBACK Received\n");
            /* Pop the previous message and check whether the last
             * was a connect packet */
            mqtt_msg_t * b = (mqtt_msg_t *)PQ_Pop(mqtt->pq);

            bool resp_type = (b->type == MQTT_SUBSCRIBE);
            uint16_t seq_num = (buffer[2] << 8u) | (buffer[3]);
            mqtt->state.rx_seq_num = seq_num;
            /* jump over properties, dont care */
            uint16_t jump_idx = buffer[4];
            bool sub_success = (buffer[5 + jump_idx] <= 0x02);
            bool seq_success = (seq_num == b->seq_num);

            success = resp_type && sub_success && seq_success;
            if(success)
            {
                mqtt->state.subs += 1u;
            }
            break;
        }
        case MQTT_PUBACK_CODE:
        {
            printf("\tPUBACK Received\n");
            uint16_t seq_num = (buffer[2] << 8u) | (buffer[3]);
            mqtt->state.rx_seq_num = seq_num;
            
            /* Iterate through the PQ to find the message */
            const uint32_t fill = mqtt->pq->fill;
            mqtt_msg_t * b = NULL;
            for(uint32_t idx = 0; idx < fill; idx++)
            {
                mqtt_msg_t * c = (mqtt_msg_t *)PQ_Peek(mqtt->pq, idx);
                if(c->seq_num == seq_num)
                {
                    printf("\tPacket found %u\n", c->seq_num);
                    c = (mqtt_msg_t *)PQ_DecreaseKey(mqtt->pq, idx, 0u);
                    b = (mqtt_msg_t *)PQ_Pop(mqtt->pq);
                    assert(b == c);
                    break;
                }
            }
            /* Pop the previous message and check whether the last
             * was a connect packet */
            if(b != NULL)
            {
                bool resp_type = (b->type == MQTT_PUBLISH);

                bool sub_success = ((buffer[4]&0x0F) == 0x00);
                bool seq_success = (seq_num == b->seq_num);
                bool key_success = (0u == b->key.key);
                printf("\tseqnum: %u\n", seq_num);
                success = resp_type && sub_success && seq_success && key_success;
            }
            else
            {
                printf("\tPacket NOT found");
                success = false;
            }
            break;
        }
        case MQTT_PUBLISH_CODE:
        {
            printf("\tPUBLISH Received\n");
            uint16_t topic_len = (buffer[2] << 8) | buffer[3];
            /*if qos =1 or 2, then msg id here */
            uint8_t * const topic = &buffer[4];
            printf("\ttopic: %s\n", topic);
            printf("\t  qos: %u\n", qos);
            uint8_t prop_len = *(topic+topic_len);
            uint8_t * msg_data = topic+topic_len+prop_len+1;
            for( uint8_t i = 0; i < mqtt->subs->num_subs ; i++ )
            {
                uint8_t * sub_topic = (uint8_t*)mqtt->subs->subs[i].name;
                const uint16_t sub_topic_len = strlen((char*)sub_topic);
                const bool global = mqtt->subs->subs[i].global;
                if( memcmp(sub_topic, topic ,sub_topic_len) == 0)
                {
                    if(global)
                    {
                        success = mqtt->subs->subs[i].callback_fn(msg_data);
                    }
                    else
                    {
                        uint8_t * client_name = (uint8_t*)mqtt->client_name;
                        uint16_t client_len = strlen((char*)client_name);
                        
                        uint16_t offset = (topic_len > client_len) ? (uint16_t)(topic_len - client_len) : 0u;
                        uint8_t * const topic_client = topic+offset;
                        if( memcmp(topic_client, client_name, client_len) == 0)
                        {
                            success = mqtt->subs->subs[i].callback_fn(msg_data);
                        }
                    }

                    if(success)
                    {
                        if(qos > 0)
                        {
                            mqtt->state.status = MQTT_SEND_ACK;
                        }
                    }
                }
            }
            break;
        }
        default:
        {
            assert(false);
            success = false;
            break;
        }
    }
    printf("\tMQTT: PQ fill: %lu\n", mqtt->pq->fill);
    return success;
}


extern void MQTT_Tick(mqtt_t * mqtt)
{
    (void)mqtt;
    /* Resend QOS packets if timeout */
}

extern bool MQTT_AllSubscribed( mqtt_t * mqtt )
{
    return ( mqtt->state.subs == mqtt->subs->num_subs );
}

extern uint32_t MQTT_CheckTimeouts(mqtt_t * mqtt)
{
    (void)mqtt;
    uint32_t timeouts = 0u;

    return timeouts;
}

extern mqtt_msg_t * MQTT_Peek(mqtt_t * mqtt, uint32_t index)
{
    return (mqtt_msg_t *)PQ_Peek(mqtt->pq, index);
}

extern mqtt_status_t MQTT_GetStatus(mqtt_t * mqtt)
{
    return mqtt->state.status;
}

