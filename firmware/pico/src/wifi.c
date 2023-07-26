#include "wifi.h"
//#include "secret_keys.h"

extern bool WIFI_Init(void)
{
    bool success = false;
    printf("Initialising WIFI driver...");
    if(cyw43_arch_init_with_country(CYW43_COUNTRY_UK))
    {
        printf("Failed\n");
        goto cleanup;
    }

    cyw43_arch_enable_sta_mode();

    if(cyw43_arch_wifi_connect_async(NULL, NULL, CYW43_AUTH_WPA2_MIXED_PSK))
    {
        printf("Failed\n");
        goto cleanup;
    }

    printf("Success\n");
    success = true;
cleanup:
    return success;
}
