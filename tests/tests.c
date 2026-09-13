#include "unity.h"
#include "mqtt_tests.h"
#include "base64_tests.h"

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
    BASE64TestSuite();

    return UNITY_END();
}
