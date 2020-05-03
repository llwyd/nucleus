#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "driver/i2c_master.h"
#include "user_config.h"
#include <stdio.h>

//Define Timer
static  os_timer_t t;

// define station config struct
static struct station_config stat;

static char addr = 0x48;

static char data[2] = {0,0};
static char text[64] = {0};

static const float tempScaling = 0.0625f;

float Node_ConvertTemp( char *d )
{
	int rawTemp = ( (( uint16_t )d[ 0 ] << 4) | d[ 1 ] >> 4 );
	return ((float)rawTemp * tempScaling);
}

/* Timer */
void timer(void *arg)
{
	i2c_master_start();
	i2c_master_writeByte( 0x91 );
	if(i2c_master_checkAck())
	{
		data[0] = i2c_master_readByte();
		i2c_master_send_ack();
		data[1] = i2c_master_readByte();
		i2c_master_send_nack();
		i2c_master_stop();
		/* This stupid conversion is because float printing doesnt work properly */
		float temperature = Node_ConvertTemp(data);
		int temp = (int)(temperature*100);
		os_printf("Temperature: %d.%d\n",temp/100,temp%100);
	}
	else
	{
		os_printf("I2C Error\n");
	}
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
	
	/* Initialise I2C */
	i2c_master_gpio_init();
  	os_delay_us(10000);	
	//Setup/Connect WIFI
  	Node_WifiSetup();
  	//Timer Setup
  	os_timer_setfn(&t, (os_timer_func_t *)timer, NULL);
  	//Button polling rate
  	os_timer_arm(&t, 1000, 1);
}
