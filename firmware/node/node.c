#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "user_config.h"

//Define Timer
static volatile os_timer_t t;

// define station config struct
static struct station_config stat;

/* Timer */
void timer(void *arg)
{
    	os_printf("Hello, World\n");
}

/* Setup WiFi connection */
void ICACHE_FLASH_ATTR Node_WifiSetup()
{
  	//Set Station Mode
  	wifi_set_opmode (STATION_MODE);
  	//Copy info from user_config.h into struct
	os_memcpy(&stat.ssid,ssid,32);
	os_memcpy(&stat.password,password,64);
	//Set config
	wifi_station_set_config(&stat);
	//Set HostName
	wifi_station_set_hostname("Sensor Node\n");
	//Connect to Wifi
	wifi_station_connect();
}

/* main function */
void ICACHE_FLASH_ATTR user_init()
{
  	// init gpio
  	gpio_init();
	// Allow uart output for debug
	uart_div_modify(0, UART_CLK_FREQ / 115200);
  	//Setup/Connect WIFI
  	Node_WifiSetup();
  	//Timer Setup
  	os_timer_setfn(&t, (os_timer_func_t *)timer, NULL);
  	//Button polling rate
  	os_timer_arm(&t, 1000, 1);
}
