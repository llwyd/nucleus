#include <stdlib.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <math.h>
#include "../common/comms.h"
#include <string.h>

static json_data_t timeData[ 3 ];
static uint8_t httpBuffer[ 512 ];

static void PopulateStruct( json_data_t * strct, char * name, int * data)
{
	memset( strct->name, 0, JSON_NAME_LEN );
	strcat( strct->name, name );
	strct->data.i = *data;	
}

int main (int argc, char ** argv)
{
	/* Retrieve system info */
	struct sysinfo info;
	sysinfo(&info);

	/* Convert into days, hours and minutes */
	long uptime = info.uptime;

	int days = (int)floor((float)uptime / ( 24.f * 3600.f ));
	uptime -= ( days * 3600 * 24) ;

	int hours = (int)floor(((float)uptime / 3600.f));
	uptime -= ( hours * 3600);

	int minutes = (int)floor(((float) uptime / 60.f ));

	PopulateStruct( &timeData[0], "Days", 		&days);
	PopulateStruct( &timeData[1], "Hours", 		&hours);
	PopulateStruct( &timeData[2], "Minutes", 	&minutes);

 	Comms_FormatData( httpBuffer, 512, timeData, 3 );

	printf("%d Days, %d hours and %d minutes\n",days, hours, minutes);
	return 0;
}

