/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/
#ifndef __PRINTF_H__
#define __PRINTF_H__

#define _outchar(c) while(1) { if( !RS_Xbusy ) { SFRB_ES=0; SFR_SBUF = c; RS_Xbusy=1; SFRB_ES=1; break; } }

#define Putch(c) _outchar(c)

void Printf ( const char *fmt, ... );
void Puts ( char *ptr );

#ifdef SUPPORT_UART1
#define _outchar1(c) while(1) { if( !RS1_Xbusy ) { SFRB_ES1=0; SFR_SBUF1 = c; RS1_Xbusy=1; SFRB_ES1=1; break; } }
#define Putch1(c) _outchar1(c)

void Printf1 ( const char *fmt, ... );
void Puts1 ( char *ptr );
#endif

//========= Debugging String Display Functions =================
// 'nullFn' has to be defined in Compiler Option as nullFn="//" 
// to prevent compile error in No DEBUG option
//==============================================================
#ifdef DEBUG
 void dPrintf( const char *fmt, ... );
 void dPuts( char *ptr );
 void wPrintf( const char *fmt, ... );
 void wPuts( char *ptr );
 void ePrintf( const char *fmt, ... );
 void ePuts( char *ptr );

 #ifdef DEBUG_PC
 #define pcPrintf	dPrintf
 #else 
 #define pcPrintf	nullFn
 #endif

 #ifdef DEBUG_OSD
 #define osdPrintf	dPrintf
 #else 
 #define osdPrintf	nullFn
 #endif
 
 #ifdef DEBUG_TW
 #define twPrintf	dPrintf
 #else 
 #define twPrintf	nullFn
 #endif

#else
 #define dPrintf	nullFn
 #define dPuts		nullFn
 #define wPrintf	nullFn
 #define wPuts		nullFn
 #define ePrintf	nullFn
 #define ePuts		nullFn

 #define pcPrintf	nullFn
 #define osdPrintf	nullFn
 #define twPrintf	nullFn
#endif	// DEBUG

BYTE Getch(void);
BYTE Getche(void);

void	DebugRegister( WORD reg );

void PrintSystemClockMsg(char *msg);

#endif	//__PRINTF_H__
