
#include <math.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include "aux.h"

static const uint8_t * db_path;

static unsigned long databaseSize = 0U;
static unsigned long uptime = 0U;
	
static struct sysinfo info;
static struct stat st;

static time_t currentAuxTime;

void Aux_Init( const uint8_t * path )
{
	db_path = path;
}

void Aux_Update(void)
{
	time(&currentAuxTime);

	stat( db_path, &st );

	databaseSize = st.st_size;

	/* Server uptime */
	sysinfo( &info );

	uptime = info.uptime;
}

