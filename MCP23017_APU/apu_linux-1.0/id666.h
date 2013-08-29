#ifndef _id666_h__
#define _id666_h__


typedef struct
{
	char title[33]; // 32 in file
	char game_title[33]; // 32 in file
	char name_of_dumper[17]; // 16 in file
	char comments[33]; // 32 in file

	//unsigned char date[12]; // 11 in file
	char seconds_til_fadeout[4]; // 3 in file
	//unsigned char length_of_fadeout[6]; // 5 in file, milliseconds
	//unsigned char artist[33]; // 32 in file
	//unsigned char chn_dis;
	//unsigned char emulator_used; // 0 unknown, 1 zsnes, 2 snes9x

} id666_tag;

int read_id666(FILE *fptr, id666_tag *tag);

#endif // _id666_h__

