#include <stdio.h>
#include "pspin.h"

/* progress spinner */

static int pspin_step=0;


void pspin_update(void)
{
	switch(pspin_step)
	{
		default:
		case 4:
			pspin_step = 0;
		case 0:
			printf("|\b"); break;
		case 1:
			printf("/\b"); break;
		case 2:
			printf("-\b"); break;
		case 3:
			printf("\\\b"); break;
	}
	fflush(stdout);
	pspin_step++;
}


