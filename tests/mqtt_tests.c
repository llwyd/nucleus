#include "unity.h"
#include "mqtt_tests.h"
#include "pq_base.h"
#include "mqtt.h"

void test_MQTT_Init(void)
{
    mqtt_t m;
    char * client_name = "testclient";
    mqtt_msg_t pool[PQ_FULL_LEN];
    pq_t pq;

    uint8_t empty_array[MQTT_MSG_SIZE] = {0};
    memset(empty_array, 0x00, MQTT_MSG_SIZE);
    MQTT_Init(&m, 
            client_name, 
            NULL, 
            &pq,
            pool);

    TEST_ASSERT_EQUAL(&pq, m.pq);
    TEST_ASSERT_EQUAL(0u, m.pq->fill);
    TEST_ASSERT_EQUAL(PQ_DEFAULT_LEN, m.pq->max);

    TEST_ASSERT_EQUAL(NULL, m.subs);
    TEST_ASSERT_EQUAL(0u, m.state.subs);
    TEST_ASSERT_EQUAL(1u, m.state.seq_num);
    TEST_ASSERT_EQUAL(1u, m.state.rx_seq_num);
    TEST_ASSERT_EQUAL(MQTT_OK, m.state.status);
    TEST_ASSERT_EQUAL(client_name, m.client_name);
    TEST_ASSERT_EQUAL_STRING_ARRAY(client_name, m.client_name, strlen(client_name));
    for(uint32_t idx = 0; idx < pq.max; idx++)
    {
        TEST_ASSERT_EQUAL(&pool[idx], pq.heap[idx]);
        TEST_ASSERT_EQUAL(0u, pool[idx].seq_num);
        TEST_ASSERT_EQUAL(UINT32_MAX, pool[idx].key.key);
        TEST_ASSERT_EQUAL(0u, pool[idx].size);
        TEST_ASSERT_EQUAL(0u, pool[idx].timestamp);
        TEST_ASSERT_EQUAL(MQTT_NONE, pool[idx].type);
        TEST_ASSERT_EQUAL_MEMORY_ARRAY(pool[idx].msg, empty_array, sizeof(uint8_t), MQTT_MSG_SIZE);
    }
}

void test_MQTT_EncodeConnect(void)
{
    mqtt_t m;
    char * client_name = "testclient";
    mqtt_msg_t pool[PQ_FULL_LEN];
    pq_t pq;
    
    MQTT_Init(&m, 
            client_name, 
            NULL, 
            &pq,
            pool);


    TEST_ASSERT_EQUAL(0u, pq.fill);
    mqtt_msg_t * out = MQTT_Encode(&m,
            MQTT_CONNECT,
            NULL,
            0,
            NULL);

    /* Connect doesnt add to the PQ and is stored in the
     * cache */
    TEST_ASSERT(out != NULL);
    TEST_ASSERT_EQUAL(0u, pq.fill);
    TEST_ASSERT_EQUAL(out, PQ_Cache(&pq));

    //TEST_ASSERT_EQUAL(25u, out->size);
    TEST_ASSERT_EQUAL(pool[PQ_CACHE_IDX].msg, out->msg);
    mqtt_msg_t * msg = (mqtt_msg_t *)&pq.heap[PQ_CACHE_IDX]->key;
    TEST_ASSERT_EQUAL(msg->msg, out->msg);

    uint8_t expected_packet[] = {
        0x10,
        0x17,
        0x00,0x04,
        0x4d,0x51,0x54,0x54,
        0x05,
        0x02,
        0x00, MQTT_TIMEOUT,
        0x00,
        0x00, 0x0A,
        't','e','s','t','c','l','i','e','n','t'
    };
    
   // TEST_ASSERT_EQUAL_MEMORY_ARRAY(expected_packet, out->msg, sizeof(uint8_t), 23U);
}
void test_MQTT_ConAck(void)
{
#if 0
    mqtt_t m;
    char * client_name = "testclient";
    mqtt_msg_t pool[MQTT_FIFO_LEN];
    mqtt_fifo_t fifo;

    MQTT_Init(&m, 
            client_name, 
            NULL, 
            &fifo,
            pool);

    TEST_ASSERT_EQUAL(0u, m.fifo->base.fill);
    
    uint8_t * buffer = NULL;
    uint16_t packet_size = MQTT_Encode(&m, MQTT_CONNECT, &buffer, NULL, 0, NULL);

    //uint8_t recvd[] = {0x02, 0x02};
#endif
}

void test_MQTT_Subscribe(void)
{
    TEST_ASSERT_TRUE(0);
}

void test_MQTT_SubAck(void)
{
    TEST_ASSERT_TRUE(0);
}

void test_MQTT_Publish(void)
{
    TEST_ASSERT_TRUE(0);
}

void test_MQTT_PubAck(void)
{
    TEST_ASSERT_TRUE(0);
}

extern void MQTTTestSuite(void)
{
    RUN_TEST(test_MQTT_Init);
    RUN_TEST(test_MQTT_EncodeConnect);
    RUN_TEST(test_MQTT_ConAck);
    RUN_TEST(test_MQTT_Subscribe);
    RUN_TEST(test_MQTT_SubAck);
    RUN_TEST(test_MQTT_Publish);
    RUN_TEST(test_MQTT_PubAck);
}
