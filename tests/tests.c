#include "unity.h"
#include "mqtt_tests.h"

void setUp( void )
{

}

void tearDown( void )
{
}

int main(void)
{
    UNITY_BEGIN();

    MQTTTestSuite();

    return UNITY_END();
}
