#include <stdlib.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <math.h>
#include "../common/comms.h"
#include <string.h>
#include <unistd.h>

static json_data_t timeData[ 4 ];
static uint8_t httpBuffer[ 512 ];

/* TODO -> mode this to comms, and add enum */
static void PopulateStruct( json_data_t * strct, char * name, int * data)
{
	memset( strct->name, 0, JSON_NAME_LEN );
	strcat( strct->name, name );
	strct->data.i = *data;	
}

int main (int argc, char ** argv)
{
	char * databasePath;
	int inputFlags;

	while((inputFlags = getopt( argc, argv, "f:" )) != -1)
	{
		switch(inputFlags)
		{
			case 'f':
				databasePath = optarg;
				break;
		}
	}

	/* Retrieve system info */
	struct sysinfo info;
	struct stat st;
	sysinfo(&info);
	if(databasePath != NULL)
	{
		stat(databasePath, &st);
	}

	/* Convert into days, hours and minutes */
	long uptime = info.uptime;

	int days = (int)floor((float)uptime / ( 24.f * 3600.f ));
	uptime -= ( days * 3600 * 24) ;

	int hours = (int)floor(((float)uptime / 3600.f));
	uptime -= ( hours * 3600);

	int minutes = (int)floor(((float) uptime / 60.f ));

	PopulateStruct( &timeData[0], "days", 			&days);
	PopulateStruct( &timeData[1], "hours", 			&hours);
	PopulateStruct( &timeData[2], "minutes", 		&minutes);

	/* Retrieve database size information */
	int size = 0;
	if(databasePath != NULL)
	{
		size = st.st_size;
	}
	printf("Database Size: %d Bytes\n",size);	
	PopulateStruct( &timeData[3], "database_size", 	&size);
 	
	Comms_FormatData( httpBuffer, 512, timeData, 4 );

	Comms_Post( "0.0.0.0", "80", "/stats", httpBuffer);

	printf("%d Days, %d hours and %d minutes\n",days, hours, minutes);
	return 0;
}

