#ifndef _apu_play_h__
#define _apu_play_h__

/* returns 0 if ok, otherwise false */
int InitSPC700(void);

int LoadAPU(FILE *fptr);

#endif // _apu_play_h__

