#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include "parport.h"
#include "apuplay.h"
#include "apu.h"
#include "id666.h"


int g_verbose = 0;
int g_playing = 1;
int g_progress = 1;
int g_debug = 1;
int g_exit_now = 0;

static void printTime(int seconds);

struct timeval last_int = {0, 0};

void signal_handler(int sig)
{

        printf("RECEIVED SIGNAL # %i\n",sig);
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
	printf("  -h       Prints this info\n");
}

int main(int argc, char **argv)
{
	int res, i;
	int reset_and_exit=0, status_line=0, loop=0, play_and_exit=0;
	char *filename;
	FILE *fptr=NULL;
	id666_tag tag;
	struct timeval tv_before, tv_now;
	
	signal(SIGINT, signal_handler);

	while((res =getopt(argc, argv, "rslvhx"))>=0)
	{
		switch(res)
		{
			case 'v':
				g_verbose = 1;
				break;
			case 's':
				status_line = 1;
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
			case '?':
				fprintf(stderr, "Unknown argument. try -h\n");
				return -1;
		}
	}

	
	
	if (reset_and_exit) 
	{
		if (g_verbose) { printf("Resetting APU\n"); }
		if (parport_init()<0) {
			printf("Failed to get io permissions. Are you root?\n");
			return 1;
		}
		if (!BidirAvailable())
		{
			if (!EnableBidir()) {
				printf("Failed to enter bidirectional mode. Try setting the parallel port mode to ECP, EPP or b\n");
				return 2;
			}
		}

		ResetAPU();
		return 0;
	}

	if (argc-optind<=0) {
		fprintf(stderr, "No file specified. Try -h\n");
		return -2;
	}


	res = parport_init();
	if (res<0) {
		printf("Failed to get io permissions. Are you root?\n");
		return 1;
	}
	
	if (!BidirAvailable())
	{
		printf("Bidir mode not available. Trying to enable it...\n");
		if (!EnableBidir())
		{
			printf("Failed to enter bidirectional mode. Try setting the parallel port mode to ECP, EPP or b\n");
			return 2;
		}
	}


	for (i = optind; i<argc; i++)
	{
		if (g_exit_now) { break; }

		filename = argv[i];
			
		fptr = fopen(filename, "r");
		if (fptr==NULL) { perror("fopen"); return 1; }

		read_id666(fptr, &tag);
	
		g_playing = 1;


                time_t start, end;
                start = time (NULL);

                printf("Now loading '%s'\n", filename);
		LoadAPU(fptr);

                end = time (NULL);
                printf("Took %ld seconds to load APU", (end - start) );

                if (!g_playing) { continue; } // next
		if (g_exit_now) { break; }

		printf("Title: %s\n", tag.title);
		printf("Game Title: %s\n", tag.game_title);
		printf("Dumper: %s\n", tag.name_of_dumper);
		printf("Comments: %s\n", tag.comments);
		printf("Seconds: %s\n", tag.seconds_til_fadeout);


		printf("\n");

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
			printf("Playing '%s'\n", tag.title);
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
							printf("Time: ");
							printTime(elaps_sec);
							printf(" [");
							printTime(num_sec - elaps_sec);
							printf("] of ");
							printTime(num_sec);
							printf(" \r");
						}
						else {
							printf("Time: ");
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
			
			ResetAPU();
			if (g_exit_now) { break; }
		}
		
	}
	
	ResetAPU();

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


