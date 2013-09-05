/* hwapu - SPC music playback tools for real snes apu
 * Copyright (C) 2004-2005  Raphael Assenat <raph@raphnet.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
//#include "parport.h"
#include "apuplay.h"
#include "apu.h"
#include "id666.h"

#include "apu_ppio.h"
#include "apu_ppdev.h"

#ifdef DJGPP
/* todo: use conio */
#define BOLD()
#define NORMAL()

#else
/* use ansi codes */
#define BOLD() printf("%c[36m", 27);
#define NORMAL() printf("%c[0m", 27);
#endif

int g_verbose = 0;
int g_playing = 1;
int g_progress = 1;
int g_debug = 0;
int g_exit_now = 0;
int g_use_embedded = 0;

static void printTime(int seconds);

static APU_ops *apu_ops;

struct timeval last_int = {0, 0};

void signal_handler(int sig)
{
	struct timeval tv_now;
	int elaps_milli;
	static int first=1;
	
	g_playing = 0;

	gettimeofday(&tv_now, NULL);	

	if (first)
	{
		first = 0;
	}
	else
	{
		elaps_milli = (tv_now.tv_sec - last_int.tv_sec)*1000;
		elaps_milli += (tv_now.tv_usec - last_int.tv_usec)/1000;

		if (elaps_milli < 1500) {
			g_exit_now = 1;
		}
	}
	
	memcpy(&last_int, &tv_now, sizeof(struct timeval));
}

void printhelp(void)
{
	printf("apuplay version %s\n\n", VERSION_STR);
	printf("Usage: ./apuplay [options] spc_file\n\n");
	printf("Supported options:\n\n");
	printf("  -v       Verbose\n");
	printf("  -l       Endless loop mode. Ignore ID666 tag time\n");
	printf("  -s       Display a status line\n");
	printf("  -x       Send the song to the APU and exit. Use -r to stop\n");
	printf("  -r       Just reset the APU and exit. This will stop\n");
	printf("           the current tune.\n");
	printf("  -e       Use a different apu loading algorithm (embedded version)\n");
	printf("           which needs less memory but does more file io. (not\n");
	printf("           significant on a PC. It was used to develop the code\n");
	printf("           which I use on the portable APU player.\n");
	printf("  -d       Debug mode. Adds a lot of verbose output.\n");
#ifdef PPDEV_SUPPORTED
	printf("  -p dev   Use ppdev instead of direct I/O\n");
#endif
#ifdef PPIO_SUPPORTED
	printf("  -i addr  Use direct io with parallel port at given address\n");
#endif
	printf("  -h       Prints this info\n");
}

int main(int argc, char **argv)
{
	int res, i;
	int use_ppdev=0;
	int use_ppio=0;
	int io_specified=0;
	int reset_and_exit=0, status_line=1, loop=0, play_and_exit=0;
	char *filename;
	FILE *fptr=NULL;
	id666_tag tag;
	struct timeval tv_before, tv_now;
	
	signal(SIGINT, signal_handler);

	while((res =getopt(argc, argv, 

					"rslvhxed"
#ifdef PPDEV_SUPPORTED
					"p"
#endif
#ifdef PPIO_SUPPORTED
					"i"
#endif
					))>=0)
	{
		switch(res)
		{
			case 'd':
				g_debug = 1;
				break;
			case 'e':
				g_use_embedded = 1;
				break;
			case 'v':
				g_verbose = 1;
				break;
			case 's':
				status_line = 0;
				break;
			case 'l':
				loop = 1;
				break;
			case 'r':
				reset_and_exit = 1;
				break;
			case 'h':
				printhelp();
				return 0;
			case 'x':
				play_and_exit = 1;
				break;
#ifdef PPDEV_SUPPORTED
			case 'p':
				use_ppdev = 1;
				io_specified = 1;
				break;
#endif
#ifdef PPIO_SUPPORTED
			case 'i':
				use_ppio = 1;
				io_specified = 1;
				break;
#endif
			case '?':
				fprintf(stderr, "Unknown argument. try -h\n");
				return -1;
		}
	}

	
	if (argc-optind<=0 && !reset_and_exit) {
		fprintf(stderr, "No file specified. Try -h\n");
		return -2;
	}

#ifdef PPDEV_SUPPORTED
	if (!io_specified) {
		use_ppdev = 1;
		io_specified = 1;
	}
#endif

#ifdef PPIO_SUPPORTED
	if (!io_specified) {
		use_ppio = 1;
		io_specified = 1;
	}
#endif

	if (!io_specified) {
		fprintf(stderr, "No io layer for apu compiled\n");
		return -3;
	}
	
#ifdef PPDEV_SUPPORTED
	if (use_ppdev) {
		apu_ops = apu_ppdev_getOps();
	} 
	else 
#endif

#ifdef PPIO_SUPPORTED
	if (use_ppio)
	{
		apu_ops = apu_ppio_getOps();
	}
#endif


	apu_setOps(apu_ops);

	/* initialize the interface with the module.
	 * (Open device, get io permissions, etc...) */
	if (apu_ops->init("")<0) {
		return 1;
	}
	
	if (reset_and_exit) 
	{
		if (g_verbose) { printf("Resetting APU\n"); }					
		apu_reset();

		return 0;
	}



	for (i = optind; i<argc; i++)
	{
		if (g_exit_now) { break; }

		filename = argv[i];
			
		fptr = fopen(filename, "rb");
		if (fptr==NULL) { perror("fopen"); return 1; }

		read_id666(fptr, &tag);
	
		g_playing = 1;

                time_t start, end;
                start = time (NULL);

		printf("Now loading '%s'", filename);
		if (g_use_embedded) {
			printf(" using 'embedded' algo\n");
			if (LoadAPU_embedded(fptr)<0) { break; }
		} else  {
			printf("  \n");
			if (LoadAPU(fptr)<0) { break; }
		}

                end = time (NULL);
                printf("Took %ld seconds to load APU.\n", (end - start) );


		if (!g_playing) { continue; } // next
		if (g_exit_now) { break; }

		BOLD(); printf("Title: "); NORMAL();
		printf("%s\n", tag.title);
		BOLD(); printf("Game Title: "); NORMAL();
		printf("%s\n", tag.game_title);
		BOLD(); printf("Dumper: "); NORMAL();
		printf("%s\n", tag.name_of_dumper);
		BOLD(); printf("Comments: "); NORMAL();
		printf("%s\n", tag.comments);
		BOLD(); printf("Seconds: "); NORMAL();
		printf("%s\n", tag.seconds_til_fadeout);

		fclose(fptr);

		if (play_and_exit) {
			return 0;
		}

		gettimeofday(&tv_before, NULL);
		
		{
			int elaps_sec;
			int num_sec = atoi(tag.seconds_til_fadeout);
			int last_elaps_sec=-1;
	
			if (num_sec<1 || num_sec>999) {
				num_sec = 150;
			}
			if (strlen(tag.title)==0) {
				strncpy(tag.title, filename, 32);
			}
			
			if (g_exit_now) { break; }
			while (g_playing)
			{
				gettimeofday(&tv_now, NULL);
				elaps_sec = tv_now.tv_sec - tv_before.tv_sec;
				if (elaps_sec > num_sec) { break; }
				
				if (status_line)
				{
					if (last_elaps_sec != elaps_sec)
					{
						if (!loop) {
							BOLD(); printf("Time: "); NORMAL();
							printTime(elaps_sec);
							printf(" [");
							printTime(num_sec - elaps_sec);
							printf("] of ");
							printTime(num_sec);
							printf(" \r");
						}
						else {
							BOLD(); printf("Time: "); NORMAL();
							printTime(elaps_sec);
							printf(" \r");
						}
					}
					last_elaps_sec = elaps_sec;
					fflush(stdout);
				}
						
				usleep(7500); // update every 75 ms
			}
			if (g_playing)
				printf("\nFinished playing.\n");
			
			apu_reset();
			if (g_exit_now) { break; }
		}
		
	}
	
	apu_reset();

	return 0;
}

static void printTime(int seconds)
{
	int hour=0, min=0, sec=0;

	if (seconds>=3600) {
		hour = seconds/3600;
		seconds -= hour*3600;;
	}
	if (seconds>=60) {
		min = seconds/60;
		seconds -= min*60;
	}
	sec = seconds;

	printf("%02d:%02d:%02d", hour, min, sec);
}


