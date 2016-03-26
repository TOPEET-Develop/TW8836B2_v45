/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/
#ifndef __UTIL_H__
#define __UTIL_H__

#define _between(x, a, b)	(a<=x && x<=b)
#define _swap(a, b)			{ a^=b; b^=a; a^=b; }



DWORD a2i(BYTE *str);
DWORD a2h(BYTE *str);

BYTE Asc1Bin(BYTE asc);
//BYTE Asc2Bin(BYTE *s);
WORD Asc4Bin(BYTE *s);
BYTE toupper(BYTE ch);
int stricmp(BYTE *ptr1, BYTE *ptr2);

BYTE TWitoa(short n, BYTE *buff);
BYTE TWhtos(BYTE n, BYTE *buff);
	#define NUM_RADIX				10
	#define HEX_RADIX				16
BYTE TWutoa(WORD value, BYTE *str, BYTE radix);
BYTE TWstrlen(BYTE *s);
BYTE *TWstrcpy(BYTE *d,BYTE *s);
BYTE *TWstrcat(BYTE *d,BYTE *s);
BYTE IsDigit(BYTE c);
short TWabsShort(short value);
BYTE SPI_cmd_protocol(BYTE max, ...);
void CheckRegister(BYTE page, BYTE reg, WORD line);


int TWmemcmp( const void * ptr1, const void * ptr2, WORD num );
void * TWmemcpy ( void * destination, const void * source, WORD num );
void * TWmemset ( void * ptr, int value, WORD num );

void I2CDeviceInitialize(BYTE *RegSet, BYTE delay);

#if defined(SUPPORT_FAST_INPUT_TOGGLE)
void WriteTW88Buff2Reg(struct REG_BUFF_INFO_s *p, BYTE *buff);
void ReadTW88Reg2Buff(struct REG_BUFF_INFO_s *p, BYTE *buff);
#endif

#endif // __UTIL_H__


