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
#include <string.h>

/* Period in seconds to update the server */
#define HTTP_UPDATE_PERIOD ( 5 * 60 )
//#define HTTP_UPDATE_PERIOD ( 10 )

/* Define Timer */
static  os_timer_t t;

/*  define station config struct */
static struct station_config stat;

/* TCP Structs */
static struct espconn e;
static struct _esp_tcp tcp;
ip_addr_t tcpIP;

static char addr = 0x48;

static char data[2] = {0,0};
static char httpData[256] = {0};
static char httpRequest[512] ={0};

static const float tempScaling = 0.0625f;
static float temperature = 0.0f;
static float humidity = 0.0f;

static volatile timerCount = 0;

/*  TCP Receive Callback */
void ICACHE_FLASH_ATTR Node_Rx(void *arg,char*pdata,unsigned short len){
  os_printf("Data Received\n");
  os_printf("%s\n",pdata);
}

/*  TCP Sent Callback */
void ICACHE_FLASH_ATTR Node_Tx(void *arg){
  os_printf("Data Sent\n");
}

/*  TCP Connect Callback */
void ICACHE_FLASH_ATTR Node_Connect(void *arg){
  struct espconn *eDNS=(struct espconn *)arg;
  os_printf("Connection Successful!\n");
  //Register Rx and Tx callback functions
  espconn_regist_recvcb(eDNS,Node_Rx);
  espconn_regist_sentcb(eDNS,Node_Tx);
  //Send data
  espconn_send(&e,httpRequest,os_strlen(httpRequest));
}

/*  TCP Reconnect Callback */
void ICACHE_FLASH_ATTR Node_Reconnect(void  *arg, sint8 err){
  os_printf("Reconnecting...\n");
}

/*  TCP Disconnect Callback */
void ICACHE_FLASH_ATTR Node_Disconnect(void *arg){
  os_printf("Server Disconnected!\n");
}

/* Setup TCP Connection */
void ICACHE_FLASH_ATTR Node_TCPSetup( void )
{
	e.proto.tcp = &tcp;
	e.type = ESPCONN_TCP;
	e.state = ESPCONN_NONE;

	os_printf("Attempting TCP Connection\n");
	//Set the IP
	ipaddr_aton( domain,&e.proto.tcp->remote_ip);
	//Set the remote port
	e.proto.tcp->remote_port = port;
	//Set the local port
	e.proto.tcp->local_port = espconn_port();
	//Register Callback Functions
	espconn_regist_connectcb(	&e, Node_Connect); 
	espconn_regist_reconcb(		&e, Node_Reconnect);
	espconn_regist_disconcb(	&e, Node_Disconnect);  
	//Connect
	espconn_connect(&e);
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
	//Set reconnection policy
	wifi_station_set_reconnect_policy( true );
	//Connect to Wifi
	wifi_station_connect();
}

float Node_ConvertTemp( char *d )
{
	int rawTemp = ( (( uint16_t )d[ 0 ] << 4) | d[ 1 ] >> 4 );
	return ((float)rawTemp * tempScaling);
}

void Node_FormatTempData( const uint8_t * deviceID, float * temp, float * hum, uint8_t * buffer)
{
	int t = (int)(*temp*100);
	int h = (int)(*hum*100);
    os_sprintf(buffer,"{\"device_id\":\"%s\",\"temperature\": \"%d.%d\",\"humidity\":\"%d.%d\"}", deviceID, t/100,t%100,h/100,h%100);
}
void Node_Post( void )
{
	memset(httpRequest,0x00,512);
	os_sprintf(httpRequest, "POST %s HTTP/1.1\r\nHost: %s:%d\r\nContent-Type: application/json\r\nAccept: */*\r\nContent-Length: %d\r\nConnection:close\r\nUser-Agent: pi\r\n\r\n%s\r\n\r\n",path, domain, port, (int)strlen(httpData), httpData);
	//os_printf("%s\n",httpRequest);
	Node_TCPSetup();
}

/* Timer */
void timer(void *arg)
{
	if( timerCount > HTTP_UPDATE_PERIOD)
	{
		i2c_master_start();
		i2c_master_writeByte( (addr << 0x1) | 0x1 );
		if(i2c_master_checkAck())
		{
			data[0] = i2c_master_readByte();
			i2c_master_send_ack();
			data[1] = i2c_master_readByte();
			i2c_master_send_nack();
			i2c_master_stop();
			/* This stupid conversion is because float printing doesnt work properly */
			temperature = Node_ConvertTemp(data);
			memset(httpData,0x00,256);
			Node_FormatTempData(deviceUUID,&temperature,&humidity,httpData);
			Node_Post();
		}
		else
		{
			os_printf("I2C Error\n");
		}
		timerCount = 0;
	}
	else
	{
		timerCount++;
	}
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
