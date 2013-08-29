#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include "apu.h"
#include "pspin.h"

#include "bootcode.h"
#include "dsploader.h"

extern int g_verbose; // from main.c
extern int g_debug; // from main.c
extern int g_progress; // from main.c
extern int g_exit_now; // from main.c
extern int g_playing; // from main.c

/* return false on timeout, otherwise true */
static int waitInport(int port, unsigned char data, int timeout_ms)
{
	struct timeval tv_before, tv_now;
	int elaps_milli;
	gettimeofday(&tv_before, NULL);

	
	while(ReadSPC700(port)!=data) {
		//usleep(1);
		gettimeofday(&tv_now, NULL);
		elaps_milli = (tv_now.tv_sec - tv_before.tv_sec) * 1000;
		elaps_milli += (tv_now.tv_usec-tv_before.tv_usec)/1000;
		if (elaps_milli > timeout_ms )
		{
			if (g_verbose) 
				printf("timeout after %d milli\n", elaps_milli);
			return 0;
		}
	}
	return 1;
}

int InitSPC700(void)
{
	if (!waitInport(0, 0xaa, 500)) {
		if (g_verbose) 
			printf("Should read 0xaa, but reads %02x\n", ReadSPC700(0));
		return -1;
	}
	if (!waitInport(1, 0xbb, 500)) {
		if (g_verbose) 
			printf("Should read 0xaa, but reads %02x\n", ReadSPC700(0));
		return -1;
	}

	WriteSPC700(1, 1);
	WriteSPC700(2, 2);
	WriteSPC700(3, 0);
	WriteSPC700(0, 0xCC);

	if (!waitInport(0, 0xcc, 500)) {
		if (g_verbose) 
			printf("Should read 0xcc, but reads %02x\n", ReadSPC700(0));
		return -1;
	}
	return 0;
}

int LoadAPU(FILE *fptr)
{
	int i=0, j=0, count=0;
	
	unsigned char spc_pcl;
	unsigned char spc_pch;
	unsigned char spc_a;
	unsigned char spc_x;
	unsigned char spc_y;
	unsigned char spc_sw;
	unsigned char spc_sp;

	//unsigned char bootbyte;
	
	unsigned char spcdata[65536]= {0};
	unsigned char spcram[64] = {0};
	unsigned char dspdata[128] = {0};
	
	int echosize, echoregion, bootptr, echoclear=2, readcount;
	
	fseek(fptr, 0x25, SEEK_SET);

	fread(&spc_pcl, 1, 1, fptr);
	fread(&spc_pch, 1, 1, fptr);
	fread(&spc_a, 1, 1, fptr);
	fread(&spc_x, 1, 1, fptr);
	fread(&spc_y, 1, 1, fptr);
	fread(&spc_sw, 1, 1, fptr);
	fread(&spc_sp, 1, 1, fptr);

	if (g_debug) {
		printf("PC: %02x%02x\n", spc_pch, spc_pcl);
		printf("A: %02X\n", spc_a);
		printf("X: %02X\n", spc_x);
		printf("Y: %02X\n", spc_y);
		printf("SW: %02X\n", spc_sw);
		printf("SP: %02X\n", spc_sp);
	}


	
	fseek(fptr, 0x100, SEEK_SET);
	fread(spcdata, 65536, 1, fptr);

	fseek(fptr, 0x10100, SEEK_SET);
	fread(dspdata, 128, 1, fptr);

//        for(i = 0;i<128;i++) printf("DSPDATA[%i]: %u\n",i,dspdata[i]);
	
	fseek(fptr, 0x101c0, SEEK_SET);
	fread(spcram, 64, 1, fptr);

	bootcode[0x19] = spcdata[0xf4];
	bootcode[0x1f] = spcdata[0xf5];
	bootcode[0x25] = spcdata[0xf6];
	bootcode[0x2b] = spcdata[0xf7];
	bootcode[0x01] = spcdata[0x00];
	bootcode[0x04] = spcdata[0x01];
	bootcode[0x07] = spcdata[0xfc];
	bootcode[0x0a] = spcdata[0xfb];
	bootcode[0x0d] = spcdata[0xfa];
	bootcode[0x10] = spcdata[0xf1];
	bootcode[0x38] = dspdata[0x6c];
	dspdata[0x6c] = 0x60;
	bootcode[0x3e] = dspdata[0x4c];
	dspdata[0x4c] = 0x00;
	bootcode[0x41] = spcdata[0xf2];
	bootcode[0x44] = spc_sp - 3;
	spcdata[0x100 + spc_sp - 0] = spc_pch;
	spcdata[0x100 + spc_sp - 1] = spc_pcl;
	spcdata[0x100 + spc_sp - 2] = spc_sw;
	bootcode[0x47] = spc_a;
	bootcode[0x49] = spc_y;
	bootcode[0x4b] = spc_x;

	count = 0;
	
	echoregion = dspdata[0x6d] * 256;
	echosize = dspdata[0x7d] * 2048;
	
	if (echosize==0) { echosize = 4; }

	for (j=255; j>=0; j--)
	{
		for (bootptr = 65471; bootptr >= 0x100; bootptr--)
		{
			if (	(bootptr > echoregion + echosize) ||
					(bootptr < echoregion) )
			{
				if (spcdata[bootptr] == j)
				{
					count++;
				}
				else
				{
					count = 0;
				}
				if (count == 77) { break; }
			}
			else
			{
				count = 0;
			}
		}
		if (count == 77) { break; }
	}

	if (bootptr == 0xff)
	{
		if (echosize < 77) {
			fprintf(stderr, "This spc file does not have sufficient ram to be loaded");
			return -1;
		}
		else {
			bootptr = echoregion;
		}
	}

	if (g_debug) { printf("Count: %d\n", count); }

	for (i=bootptr; i<=bootptr+count-1; i++)
	{
		spcdata[i] = bootcode[i - bootptr];
	}
	
	ResetAPU();

	InitSPC700();

	if (g_verbose) 
		printf("Sending dsp loader\n");

	if (g_exit_now || !g_playing) { ResetAPU(); return 0; }

	for (i=0; i<16; i++)
	{
		if (g_progress) {
			pspin_update();
		}
		WriteSPC700(1,dsploader[i]);
		WriteSPC700(0, i);
		if (!waitInport(0, i, 500)) {
			if (g_debug) {
				printf("Waited for %02x, but got %02x\n", i, ReadSPC700(0));
			}
			fprintf(stderr, "timeout 1\n"); return -1;
		}
	}
	if (g_verbose) 
		printf("\n");

	if (g_exit_now || !g_playing) { ResetAPU(); return 0; }

	WriteSPC700(1, 0);
	i = ReadSPC700(0);
	i+=2;
	WriteSPC700(0, i);

	if (!waitInport(0, i, 500)) {
		fprintf(stderr, "timeout 2\n"); return -1; 
	}

	if (g_verbose) 
		printf("Sending dsp data\n");

	for (i=0; i<128; i++)
	{
		if (g_exit_now || !g_playing) { ResetAPU(); return 0; }
		if (g_progress) {
			pspin_update();
		}
                
                WriteSPC700(1, dspdata[i]);
		WriteSPC700(0,i);

                if (!waitInport(0, i, 500)) {
			if (ReadSPC700(0)==0xaa)
			{
	//			fprintf(stderr, "ingored\n");
			}
			else
			{
				fprintf(stderr, "timeout 3\n"); return -1; 
			}
		}
	}
	if (g_verbose) 
		printf("\n");

	if (!waitInport(0, 0xaa, 500)) {
		fprintf(stderr, "timeout 4\n"); return -1; 
	}

	InitSPC700();
	if (g_verbose) 
		printf("Sending SPC data\n");

	for (i=2; i<=0xef; i++)
	{
		WriteSPC700(1, spcdata[i]);
		WriteSPC700(0, i-2);
		if (!waitInport(0, i-2, 500)) {
			fprintf(stderr, "timeout 5\n"); return -1; 
		}
		if (g_progress) 
			pspin_update();
		if (g_exit_now || !g_playing) { ResetAPU(); return 0; }
	}
	if (g_verbose) 
		printf("\n");
	
	
	WriteSPC700(1,1);
	WriteSPC700(2, 0);
	WriteSPC700(3, 1);
	i = ReadSPC700(0);
	i += 2;
	WriteSPC700(0, i);

	if (!waitInport(0, i, 500)) {
		fprintf(stderr, "timeout 5\n"); return -1; 
	}

	SetPort0(0);
	if (g_verbose) 
		printf("Uploading song\n");

	for (i=0x100; i <= 65471; i+= 16)
	{
		if (
			(i >= echoregion) && 
			(i<=(echoregion + echosize)) &&
			(bootptr != echoregion)
			)
		{
			for (j=0; j<16; j++)
			{
				if ( ((bootcode[0x38] & 0x20) == 0) && 
					(echoclear == 0)) {
					spcdata[i+j]=0;
				} else if (echoclear == 1) {
					spcdata[i+j]=0;
				}
			}
		}


		if (WriteSPC700Bytes(&spcdata[i], 16))
		{
                        printf("Transfer error\n");
			fprintf(stderr, "Transfer error\n");
			return -1;
		}
                else{
//                 printf(" OK # %i\n", i); 
                 }


		if (i % 256 == 0) 
                {
	          readcount += 256;
		  if (g_progress) 
                  {
		   pspin_update();
		  }                
		}

                //printf("g_exit_now: %i | g_playing: %i\n", g_exit_now,g_playing);
		if (g_exit_now || !g_playing) 
                { 
              //    printf("Exiting");
                  ResetAPU(); 
                  return 0; 
                }
                else
                {
                //printf("Continue");
                }
	}
//        printf("I'm HERE");
	for (i=65472; i<= 65535; i++)
	{
          //printf("I got here, yay!");

		if (spcdata[0xf1] & 0x80) {
			if (echosize + echoregion > 65472)
			{
				if ( ((bootcode[0x38]&0x20)==0) &&
					echoclear == 0)
				{
					spcram[i-65472] = 0;					
				} else if (echoclear == 1) {
					spcram[i-65472] = 0;
				}
			}

			if (WriteSPC700_WP0I(1, spcram[i-65472])==1) {
                                printf("some error\n");
 				fprintf(stderr, "some error\n");
				return -1;
			}
		}
		else {
			if (echosize + echoregion > 65472) {
				if ( ((bootcode[0x38]&0x20)==0) &&
						echoclear ==0) {
					spcdata[i]=0;
				} else if (echoclear == 1) {
					spcdata[i]=0;
				}
			}

			if (WriteSPC700_WP0I(1, spcram[i-65472])==1) {
                                printf("some error AGAIN\n");
				fprintf(stderr, "some error AGAIN\n");
				return -1;
			}
			
		}
		if (i % 256 == 0) {
			readcount += 256;
			if (g_progress) {
				pspin_update();
			}

                

		}
                
		if (g_exit_now || !g_playing) { ResetAPU(); return 0; }
	}
	if (g_verbose) 
		printf("\n");
	
	WriteSPC700(3, (bootptr & 0xff00)>>8 );
	WriteSPC700(2, (bootptr & 0xff));
	WriteSPC700(1, 0);
	WriteSPC700(0, 1);
	i = 0;
/*	if (!waitInport(0, 0x53, 500)) {
                printf("Should have been 0x53, Actually: %u\n",ReadSPC700(0));
		fprintf(stderr, "timeout 7\n");
		return -1;
	}*/
	
	WriteSPC700(0, spcdata[0xf4]);
	WriteSPC700(1, spcdata[0xf5]);
	WriteSPC700(2, spcdata[0xf6]);
	WriteSPC700(3, spcdata[0xf7]);

	if (g_debug) {
		printf("Boot ptr: %04X\n", bootptr);
		printf("Echo pointer: %04X\n", echoregion);
		printf("Echo size: %04X\n", echosize);
	}
	
	if (g_exit_now || !g_playing) { ResetAPU(); return 0; }
	return 0;
}


