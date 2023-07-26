#include "wifi.h"
#include "secret_keys.h"

extern bool WIFI_Init(void)
{
    bool success = false;
    printf("Initialising WIFI driver...");
    if(cyw43_arch_init_with_country(CYW43_COUNTRY_UK))
    {
        printf("Failed\n");
        goto cleanup;
    }
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0); 
    cyw43_arch_enable_sta_mode();
/*
    if(cyw43_arch_wifi_connect_async(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_MIXED_PSK))
    {
        printf("Failed\n");
        goto cleanup;
    }
*/
    printf("Success\n");
    success = true;
cleanup:
    return success;
}

extern void WIFI_TryConnect(void)
{
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0); 
    if(cyw43_arch_wifi_connect_async(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_MIXED_PSK))
    {
        printf("Failed to retry\n");
    }
}
