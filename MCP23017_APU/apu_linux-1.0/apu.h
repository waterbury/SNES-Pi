unsigned char ReadSPC700(int address);
int WriteSPC700(int address, unsigned char data);
unsigned char SetPort0 (unsigned char data);
int WriteSPC700_WP0I (int address, unsigned char data);
int WriteSPC700Bytes(unsigned char *data, int len);
int ResetAPU(void);
