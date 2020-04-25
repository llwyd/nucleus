#include <stdlib.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <math.h>


int main (int argc, char ** argv)
{
	struct sysinfo info;
	sysinfo(&info);

	long uptime = info.uptime;
	int days = (int)floor((float)uptime / ( 24.f * 3600.f ));
	uptime -= ( days * 3600 * 24) ;

	int hours = (int)floor(((float)uptime / 3600.f));
	uptime -= ( hours * 3600);

	int minutes = (int)floor(((float) uptime / 60.f ));

	printf("Days: %d\n",days);
	printf("Hours: %d\n", hours);
	printf("Minutes: %d\n", minutes);
	return 0;
}

