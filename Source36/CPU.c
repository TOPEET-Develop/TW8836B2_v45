/**
 * @file
 * CPU.c 
 * @author Harry Han
 * @author YoungHwan Bae
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2014 Intersil Corporation
 * @section DESCRIPTION
 *	8051 MCU file
 *
*/
/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/
/*
Instruction
    http://www.keil.com/dd/docs/datashts/dcd/dp80390_instr.pdf
Register Bank
    http://www.keil.com/support/man/docs/c51/c51_le_regbankaccess.htm
    By default, all non-interrupt functions use register bank 0.  
*/
#include "config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "Global.h"
#include "printf.h"
#include "I2C.h"
#include "CPU.h"
#include "Remo.h"
#include "TouchKey.h"

/*
TW8836 MCU --DP80390XP compatible--

Cache 256 Bytes
XDATA   2KByte

                                CFFF+-----------+
                                    |  Chip     |
                                    |  Register |
    FF+---------------------+   0C00+-----------+
      | Internal |    SFR   |       |           |
      |   RAM    |          |       |  unused   |
    7F+----------+----------+   0800+-----------+
      |     Internal        |       |  Data     |
      |       RAM           |       |  Memory   |
    00+---------------------+   0000+-----------+
        IDATA        DATA


    00~07  Reg. Bank0
    08~0F  Reg. Bank1
    10~17  Reg. Bank2
    18~1F  Reg. Bank3
    20~27  Bit 00~3F
    28~2F  Bit 40~7F
    30~7F  General User RAM & Stack Space

Register Bank
    By default, register bank0 us used.
    Interrupt use register bank1,2,3.
    Tip.If Interrupt does not use, code can use these as data memory.
Bit memory
    Use SETB, CLR commands

SFR memory

TW8836B SFR  MAP------------
   -0-   -1-   -2-   -3-   -4-   -5-   -6-   -7-   
80:P0    SP    DPL0  DPH0  DPL1  DPH1  DPS   PCON  
88:TCON  TMOD  TL0   TL1   TH0   TH1   CKCON ..... 
90:P1    EXIF  _WTST _DPX0 ..... _DPX1 ..... ..... 
98:SCON0 SBUF0 BANK  ..... CACHE ACON  FIFO0 FIFO1 
A0:P2    ..... ..... ..... ..... ..... ..... ..... 
A8:IE    ..... ..... ..... ..... ..... ..... ..... 
B0:P3    ..... ..... ..... ..... ..... ..... ..... 
B8:IP    ..... ..... ..... ..... ..... ..... ..... 
C0:SCON1 SBUF1 CCL1  CCH1  CCL2  CCH2  CCL3  CCH3  
C8:T2CON T2IF  RLDL  RLDH  TL2   TH2   CCEN  ..... 
D0:PSW   ..... ..... ..... ..... ..... ..... ..... 
D8:WDCON ..... ..... ..... ..... ..... ..... ..... 
E0:ACC   ..... CAMC  ..... ..... ..... ..... ..... 
E8:EIE   STAT  _MXAX TA    ..... ..... ..... ..... 
F0:B     ..... ..... ..... ..... ..... ..... ..... 
F8:EIP   ..... E2IF  E2IE  E2IP  E2IM  E2IT  .....

StackPointer:
    It starts from 07h in startup.
    On start up code, it use below.
        IF PBPSTACK <> 0
        EXTRN DATA (?C_PBP)
                MOV     ?C_PBP,#LOW PBPSTACKTOP
        ENDIF
                MOV     SP,#?STACK-1
                LJMP    ?C_START
    After reset, the stack pointer is initialized to 07h. The stack will 
    start growing up from address 8h.
    The Keil C compiler uses internal DATA memory for your variables and also 
    allows you to use register banks 1, 2, and 3. If the stack pointer were not 
    adjusted, calls to functions and interrupts would overwrite your variables.
    Therefore, the last thing the startup code does is to set the stack pointer 
    to the end of all your internal DATA variables.

Interrupt
# bank name  enable prio   flag    Port func    description
-- -  ----   ----   ----   ----    ---- -----   -----------
0  1  EX0    IE.0   IP.0   IE0     P2.0 ext0_   chip interrupt. ->video ISR.
1  1  ET0    IE.1   IP.1   TF0          timer0_ time tick
2  1  EX1	 IE.2   IP.2   IE1     P2.1 ext1_   DE(data enable) end.
3  1  ET1	 IE.3   IP.3   TF1          timer1_ ->touch.
4  2  ES0    IE.4   IP.4   RI/TI        uart0_  uart0.
5  1  ET2	 IE.5   IP.5   T2IF         timer2_ ->remo sampling
6  1  ES1    IE.6   IP.6   RI1/TI1      uart1_
7  1  EINT2  EIE.0	EIP.0  EIF.0   P2.2	ext2_   DMA done.
8  1  EINT3  EIE.1	EIP.1  EIF.1   P2.3	ext3_   Touch_Ready interrupt.
9  1  EINT4  EIE.2	EIP.2  EIF.2  	    ext4_   xram 7FF access by I2C.
10 1  EINT5  EIE.3  EIP.3  EIF.3        ext5_   
11 1  EINT6  EIE.4	EIP.4  EIF.4   P2.4 ext6_   Touch_Pen interrupt.
12 3  EWDI	 EIE.5	EIP.5     	   WDIF watchdog_
13 1  EINT7  E2IE.0	E2IP.0 E2IF.0  P1.0 ext7_   Programable. P1.0 Pin60
14 1  EINT8  E2IE.1	E2IP.1 E2IF.1  P1.1 ext8_   Programable. P1.1 Pin61
15 1  EINT9  E2IE.2	E2IP.2 E2IF.2  P1.2 ext9_   Programable. P1.2 Pin62
16 1  EINT10 E2IE.3	E2IP.3 E2IF.3  P1.3 ext10_  Programable. P1.3 Pin63
17 1  EINT11 E2IE.4	E2IP.4 E2IF.4  P1.4 ext11_  Programable. P1.4 Pin87  ->remo activate
18 1  EINT12 E2IE.5	E2IP.5 E2IF.5  P1.5 ext12_  Programable. P1.5 Pin86
19 1  EINT13 E2IE.6	E2IP.6 E2IF.6  P1.6 ext13_  Programable. P1.6 Pin114
20 1  EINT14 E2IE.7	E2IP.7 E2IF.7  P1.7 ext14_  Programable. P1.7 Pin115

Interrupt Polling Sequence
EX0 ET0 EX1 ET1 ES ...
The "using" function attribute is used to select a register bank 

*/

//===== memory register ========
volatile BYTE	XDATA *DATA regTW88 = REG_START_ADDRESS;

DWORD ExtIntCount = 0;

//===== interrupt status ========
DATA	WORD	MCU_INT_STATUS=0;
DATA	BYTE	EXINT_STATUS = 0;

#ifdef DEBUG_ISR
XDATA WORD INTR_counter[21];
#endif
#ifdef DEBUG_UART
XDATA BYTE UART0_OVERFLOW_counter;
XDATA BYTE UART0_MAX_counter;
#endif


//==========================
// PROTOTYPE for Internal 
//==========================
//===== Remo ========
//TW8835 EVB uses P1_2(ext9_int), 
//TW8836 EVB uses P1_4(ext11_int).
void InitRemoTimer(void);




//*****************************************************************************
//*      Ext Int 0 Interrupt (Low / Falling) 	(Internal)	: <<Chip Status>>	P2.0
//*****************************************************************************

//===== interrupt status ========
DATA	BYTE	INT_STATUS=0;
DATA	BYTE	INT_MASK=0;
DATA	BYTE	INT_STATUS2=0;
DATA	BYTE	INT_STATUS3=0;	//for debug ext4_intr
DATA	BYTE	INT_STATUS_ACC=0;
DATA	BYTE	INT_STATUS2_ACC=0;
DATA	WORD	VH_Loss_Changed=0;

/**
* chip interrupt service routine.
*
* enable 	SFR_EX0
* port		P2.0
*
* Use ReadTW8A() and WriteTW8A().
* If use ReadTW88() and WriteTW88(), compiler omit PUSH/POP PSW.
*/
void ext0_int(void) interrupt 0 using 2
{
#ifdef DEBUG_ISR
	MCU_INT_STATUS |= 0x0001;
	INTR_counter[0]++;
#endif
    /* do not update if g_access was 0 */
	if(g_access==0)
		return;

    /* read register */
	INT_STATUS  = ReadTW8A(REG002) & 0xEF;
	INT_MASK    = ReadTW8A(REG003);
	INT_STATUS2 = ReadTW8A(REG004);
    /* clear */
	WriteTW8A(REG002, INT_STATUS );				//clear	
	WriteTW8A(REG004, INT_STATUS2 & 0x07);		//clear

	if(INT_STATUS & 0x02)						//keep 0x02.
		VH_Loss_Changed++;

    /* accumulate */
	INT_STATUS_ACC |= INT_STATUS;
	INT_STATUS2_ACC |= INT_STATUS2;
}


//*****************************************************************************
//*      Timer 0 Interrupt 									: <<System Timer>>                                               
//*****************************************************************************
//===== Timer0 =======
//NOTE: To read ISR related value that bigger than BYTE, use SFRB_ET0.
DATA 	BYTE   	tic01=0;				//unit 1msec
DATA 	WORD   	tic_pc=0;
DATA 	WORD   	tic_task=0;
DATA 	BYTE 	tic_timer=0;
DATA 	DWORD	SystemClock=0L;			//unit 10msec
DATA	DWORD	OsdTimerClock=0L;

/**
* timer0 interrupt service routine
*
* 1 msec timer tic
*  use 10us @27Mhz
*
* enable:	SFT_ET0
* port:		NONE
*/
void timer0_int(void) interrupt 1 using 1
{
#ifdef DEBUG_ISR
	MCU_INT_STATUS |= 0x0002;
	INTR_counter[1]++;
#endif

	tic01++;
	tic01 %= 10;		
	if ( tic01 == 0 ) {
		SystemClock++;
		if(OsdTimerClock)
			OsdTimerClock--;
	}
	tic_pc++;		//WORD size.
	tic_task++;		//WORD size.
	if(tic_timer)
		tic_timer--;
}
//*****************************************************************************
//*      Ext Int 1 Interrupt  (Low / Falling)			: <<DE End>>	P2.1
//*****************************************************************************
/*
* DE End interrupt service routine
*
* enable:	SFR_EX1
* port:		P2.1
*
* usage:
*	ext1_intr_flag =0; 		//clear
*	SFRB_EX1 = 1;			//enable
*	while(!ext1_intr_flag);	//wait
*
*/
BYTE ext1_intr_flag;

void ext1_int(void) interrupt 2 using 1
{
#ifdef DEBUG_ISR
	MCU_INT_STATUS |= 0x0004;
	INTR_counter[2]++;
#endif

	ext1_intr_flag = 1;
	SFRB_EX1 = 0; /* disable EX1 */
}

//****************************************************************************
//*      Timer 1 Interrupt : for Touch sensing & key sensing                                           
//****************************************************************************
WORD	CpuTouchX, CpuTouchY, CpuZ1, CpuZ2;
BYTE /*bit*/		CpuTouchPressed=0;
WORD	CpuAUX0 = 0;
WORD	CpuAUX1 = 0;
WORD	CpuAUX2 = 0;
WORD	CpuAUX3 = 0;
BYTE	CpuAUX0_Changed = 0;
BYTE	CpuAUX1_Changed = 0;
BYTE	CpuAUX2_Changed = 0;
BYTE	CpuAUX3_Changed = 0;
BYTE	CpuTouchStep=0, CpuTouchChanged=0;
WORD 	CpuTouchSkipCount=0;

#define ReadTscData(TscData)    \
    TscData = ReadTW8A(REG0B2); \
    TscData <<= 4;              \
    TscData += ReadTW8A(REG0B3) 
//---------------------
/**
* timer1 interrupt service routine for Touch
*
* every 500usec. use 82uS @ 27MHz
* @todo too big
*
* enable 	SFR_ET1
* port		NONE
*
*/
#if defined(SUPPORT_TOUCH)
void timer1_int(void) interrupt 3 using 1
{
	static WORD TX,TY;
	static WORD	temp;
	WORD TscData;
	WORD diff;

#ifdef DEBUG_ISR
	MCU_INT_STATUS |= 0x0008;
	INTR_counter[3]++;
#endif

	if(g_access==0)
		return;

 	if(CpuTouchSkipCount) {
		CpuTouchSkipCount--;
		return;
	}

	if ( CpuTouchStep == 0 ) {				// start Measure Z2
		//TSC MODE : 2 : Z2 measure
		ReadTscData(TscData);
		if ( TscData > 0x800 ) {					
			WriteTW8A(REG0B0, 0x01 );		// adc  mode Z1
			CpuTouchStep++;					// 1
			temp = 0;						// clear
			CpuZ2 = TscData;							
		}
		else {
			CpuTouchStep = 5;
		}
	}
	else if ( CpuTouchStep == 1 ) {			 // check Z1
		//TSC MODE : 1 : Z1 measure
		ReadTscData(TscData);
		if ( TscData > temp )	diff = TscData - temp;
		else					diff = temp - TscData;

		if ( diff < 10 ) {					// find stable value
			if ( TscData < 100 ) {			// no touch
				CpuTouchPressed = 0;
				CpuTouchStep = 5;
				CpuTouchChanged++;
			}
			else {
				WriteTW8A(REG0B0, 0 );		// adc start with mode XPOS
				CpuTouchStep++;				// restart Touch measurement
				temp = 0;					// clear
			}
		}
		else {
			temp = TscData;					// redo measure
			//--WriteTW8A(REG0B0, 0x01 );	// adc  mode Z1
		}
	}
	else if ( CpuTouchStep == 2 ) {			 // check XPOS
		//TSC MODE : 0 : X position measure
		ReadTscData(TscData);

		if ( TscData > temp )	diff = TscData - temp;
		else					diff = temp - TscData;
			
		if ( diff < 10 ) {					// find stable value
			WriteTW8A(REG0B0, 0x03 );		// adc start with mode YPOS
			CpuTouchStep++;
			temp = 0;						// clear
			TX = TscData;
		}
		else {
			temp = TscData;					// redo measure
			//--WriteTW8A(REG0B0, 0 );		// adc start with mode XPOS
		}
	}
	else if ( CpuTouchStep == 3 ) {	 		// check YPOS
		//TSC MODE : 3 : Y position measure
		ReadTscData(TscData);

		if ( TscData > temp )	diff = TscData - temp;
		else					diff = temp - TscData;

		if ( diff < 10 ) {					// find stable value
			WriteTW8A(REG0B0, 0x01 );		// adc start with mode Z1
			temp = 0;						// clear
			CpuTouchStep++;
			TY = TscData;
		}
		else {
			temp = TscData;					// redo measure
			//--WriteTW8A(REG0B0, 0x03 );	// adc start with mode YPOS
		}
	}
	else if ( CpuTouchStep == 4 ) {  		// check z1
		//TSC MODE : 1 : Z1 measure
		ReadTscData(TscData);

		if ( TscData > temp )	diff = TscData - temp;
		else				    diff = temp - TscData;

		if ( diff < 10 ) {					// find stable value
			if ( TscData < 100 ) {			// no touch, reset touch interrupt
				CpuTouchPressed = 0;
				CpuTouchChanged++;
			}
			else {
				//===================
				//
				//===================
				CpuTouchX = TX;
				CpuTouchY = TY;
				CpuZ1 = TscData;
				CpuTouchChanged++;
				CpuTouchPressed = 1;
			}
			CpuTouchStep++;
		}
		else {
			temp = TscData;					// redo measure
			//--WriteTW8A(REG0B0, 0x01 );	// adc start with mode Z
		}
	}
	else if ( CpuTouchStep == 5 ) {  		// Start AUX input check
		WriteTW8A(REG0B0, 0x07 );			// write Start, erase Ready, mode AUX3
		CpuTouchStep++;
		temp = 0;
	}
	else if ( CpuTouchStep == 6 ) { 
		//TSC MODE : 7 : AUX3 measure
		ReadTscData(TscData);

		if ( TscData > temp )	diff = TscData - temp;
		else					diff = temp - TscData;
			
		if ( diff < 10 ) {					// find stable value
			CpuAUX3 = TscData;
			CpuAUX3_Changed++;
			WriteTW8A(REG0B0, 2 );			// write Start, erase Ready, mode Z2
			temp = 0;
			CpuTouchStep = 0; 						
		}
		else {
			temp = TscData;					// redo measure
			//--WriteTW8A(REG0B0, 7 );		// write Start, erase Ready, mode AUX[3]
		}
	}
}
#else
/**
* If select only SUPPORT_KEYPAD, FW will use a Polling, and will not use a interrupt.
*/
void timer1_int(void) interrupt 3 using 1
{
#ifdef DEBUG_ISR
	MCU_INT_STATUS |= 0x0008;
	INTR_counter[3]++;
#endif

	return;
}
#endif //..SUPPORT_TOUCH..SUPPORT_KEYPAD
#undef ReadTscData


//*****************************************************************************
//      UART 0 Interrupt                                                   
//*****************************************************************************
DATA 	BYTE	RS_buf[RS_BUF_MAX];
DATA 	BYTE	RS_in=0;
DATA	BYTE	RS_out=0;
		bit		RS_Xbusy=0;

/**
* UART0 interrupt service routine
* 
* enable SFR_ES0
*/
void uart0_int(void) interrupt 4 using 2
{
	BYTE	count;

#ifdef DEBUG_ISR
	MCU_INT_STATUS |= 0x0010;
	INTR_counter[4]++;
#endif

	if( SFRB_RI ) {					//--- Receive interrupt ----
		SFRB_RI = 0;
		//if ( SFR_UART0FIFO & 0x80 ) {		// is it use fifo?
			count = SFR_UART0FIFO & 0x1F;
			if ( count & 0x10) {
				SFR_UART0FIFO = 0x90;		// overflowed, buffer clear
#ifdef DEBUG_UART
				UART0_OVERFLOW_counter++;
#endif
			}
			else {
#ifdef DEBUG_UART
				if(UART0_MAX_counter < count)
					UART0_MAX_counter = count;
#endif
				while (count) {
					RS_buf[RS_in++] = SFR_SBUF;
					if( RS_in>=RS_BUF_MAX ) RS_in = 0;
					count--;
				};
			}
		//}
		//else {
		//	RS_buf[RS_in++] = SFR_SBUF;
		//	if( RS_in>=RS_BUF_MAX ) RS_in = 0;
		//}
	}

	if( SFRB_TI ) {					//--- Transmit interrupt ----
		SFRB_TI = 0;
		RS_Xbusy=0;
	}
}

//****************************************************************************
//*      Timer 2 Interrupt 								: <<Remo Timer>>
//****************************************************************************
#define REMO_IN		PORT_REMO

#ifdef REMO_RC5
	bit         RemoPhase1, RemoPhase2;
	IDATA BYTE	RemoDataReady=0;
	IDATA BYTE  RemoSystemCode, RemoDataCode;
	IDATA DWORD RemoReceivedTime;
    #ifdef DEBUG_REMO
    	IDATA BYTE  RemoSystemCode0, RemoDataCode0;
    	IDATA BYTE  RemoSystemCode1, RemoDataCode1;
    	IDATA BYTE  RemoSystemCode2, RemoDataCode2;
    	IDATA BYTE  RemoCaptureDisable=0;
    #endif
#elif defined REMO_NEC
	bit			RemoPhase=0;
	DATA BYTE	RemoStep=0;
	DATA BYTE	RemoHcnt, RemoLcnt;
	DATA BYTE	RemoData[4];
    IDATA BYTE  RemoDataReady=0;
	IDATA BYTE  RemoNum, RemoBit;
    #ifdef DEBUG_REMO_NEC
    	DATA BYTE	DebugRemoStep;
    	DATA BYTE	DebugRemoHcnt;
    	DATA BYTE   DebugRemoLcnt;
    	DATA BYTE   DebugRemoNum;
    	DATA BYTE   DebugRemoBit;
    	DATA BYTE   DebugRemoPhase;
    	DATA BYTE   DebugRemoDataReady;
    #endif
#endif // REMO_RC5 REMO_NEC 
		bit		RM_get = 0;

DATA 	WORD   	RemoTic=0;

#ifdef REMO_RC5
#ifdef DEBUG_REMO
BYTE RemoCapture0[14+1];
BYTE RemoCapture1[14+1];
BYTE RemoCapture2[14+1];
#endif
/**
* timer2 interrupt service routine for remocon
*
* 
* enable	SFR_ET2
*			But, I am using EA, Do NOT USE SFR_ET2. 
* port		P1.4 
* 			On TW8836 EVB1.0, it uses INT11(PORT1_4) to activate timer2.
*/
void timer2_int(void) interrupt 5 using 1 // suggest: use register block 3
{
	BYTE	i;
	bit sample;

#ifdef DEBUG_ISR
	MCU_INT_STATUS |= 0x0020;
	INTR_counter[5]++;
#endif

	if(g_access==0) {
		SFR_T2IF &= 0xfc;			// Clear Interrupt Flag
		return;
	}
	SFR_T2IF &= 0xfc;				// Clear Interrupt Flag

	//read sample
	sample = REMO_IN;
	RemoTic++;

#ifdef DEBUG_REMO
	if(RemoTic==5) {
		if(RemoCaptureDisable==0)
			RemoCapture0[0] = 0x0F; //valid 0..4	
		if(RemoCaptureDisable==1)
			RemoCapture1[0] = 0x0F;	
		if(RemoCaptureDisable==2)
			RemoCapture2[0] = 0x0F;	
	}

	i = (RemoTic & 0x07);
	if(RemoTic <= 8*14) {
		if(sample) {
			if(RemoCaptureDisable==0)
				RemoCapture0[RemoTic >> 3] |= (1 << i);	
			if(RemoCaptureDisable==1)
				RemoCapture1[RemoTic >> 3] |= (1 << i);	
			if(RemoCaptureDisable==2)
				RemoCapture2[RemoTic >> 3] |= (1 << i);	
		}
	}
#endif

	//---------------------------
	// Note. We starts from RemoTic 4 with RemoPhase1 0.
	//       At frist time, The RemoTic will be 5, and RemoPhase2 has to be 1.
	//---------------------------
	i = RemoTic & 0x07;
	if     ( i==1 ) RemoPhase1 = sample; //REMO_IN;
	else if( i==5 )	RemoPhase2 = sample; //REMO_IN;
	//----- Received 1 Bit -----
	else if( i==0 ) {	//every 8 RemoTic
		if( RemoPhase1==RemoPhase2 ) {	// error
			SFRB_ET2=0;

			ClearRemoTimer();			
			EnableRemoInt();

			return;
		}
		if( RemoTic<=(8*8) ) {				// SystemCode. Start1+Start2 + Toggle + 5 BIT ADDRESS
			RemoSystemCode <<=1;
			if( RemoPhase1==1 && RemoPhase2==0 )
				RemoSystemCode |=1;
		}
		else {								// DataCode.  6 BIT COMMAND
			RemoDataCode <<=1;
			if( RemoPhase1==1 && RemoPhase2==0 )
				RemoDataCode |=1;
		}
		//----- Received 1 Packet -----
		if( RemoTic >= (8*14) ) {
			RemoDataReady++;				// new key
#ifdef DEBUG_REMO
			if(RemoCaptureDisable==0) {  RemoSystemCode0 = RemoSystemCode; RemoDataCode0 = RemoDataCode; }
			if(RemoCaptureDisable==1) {  RemoSystemCode1 = RemoSystemCode; RemoDataCode1 = RemoDataCode; }
			if(RemoCaptureDisable==2) {  RemoSystemCode2 = RemoSystemCode; RemoDataCode2 = RemoDataCode; }
			RemoCaptureDisable++;
#endif
			RemoReceivedTime = SystemClock;

			ClearRemoTimer();				
		}
	}
}
#endif //..REMO_RC5
#ifdef REMO_NEC
void timer2_int(void) interrupt 5 using 1	// suggest: use register block 3			
{
	SFR_T2IF &= 0xfc;			// Clear Interrupt Flag

	RemoTic++;

	if( RemoDataReady )		
		return;

	switch( RemoStep ) {
	case 0:
		//wait 9ms.
		if( REMO_IN==0 ) {
			RemoLcnt++;
			if( RemoLcnt==0xff ) //wait 42.38...samples
				goto RemoError;
		}
		else {
			RemoHcnt = 0;
			RemoStep++;
		}
		break;

	case 1:
		//wait 4.5ms for normal........ wait 24.106 samples
		//wait 2.25ms for repeat code.. wait 12.053 samples
		if( REMO_IN==1 ) {
			RemoHcnt++;
			if( RemoHcnt==0xff )  //
				goto RemoError;
		}
		else {
			if( RemoLcnt>=15*3 && RemoLcnt<=17*3 ) {	//target 16*3 =	48
				
				if( RemoHcnt>=3*3 && RemoHcnt<=5*3 ) {	//target 4*3 = 12
					RemoStep = 3;
					#ifdef DEBUG_REMO_NEC
					if(DebugRemoStep==0) {
						DebugRemoStep = RemoStep;
						DebugRemoLcnt = RemoLcnt;
						DebugRemoHcnt = RemoHcnt;
					}
					#endif
					RemoDataReady = 2;					 //auto repeat..
					break;
				}
				else if( RemoHcnt>=7*3 && RemoHcnt<=9*3 ) {	//target 8*3 = 24
					RemoStep++;								//move to RemoStep 2.
					#ifdef DEBUG_REMO_NEC
					if(DebugRemoStep==0) {
						DebugRemoStep = RemoStep;
						DebugRemoLcnt = RemoLcnt;
						DebugRemoHcnt = RemoHcnt;
					}
					#endif
					RemoPhase = 0;
					RemoLcnt = 0;
					RemoNum  = 0;
					RemoBit  = 0;

					break;
				}
			}
			else goto RemoError;
		}
		break;

	case 2:
		if( RemoPhase==0 ) {
			if( REMO_IN==0 )					// Phase=0  Input=0
				RemoLcnt++;
			else {								// Phase=0  Input=1
				RemoPhase = 1;
				RemoHcnt = 0;
			}
		}
		else {								
			if( REMO_IN==1 )					// Phase=1  Input=1
				RemoHcnt++;
			else {								// Phase=1  Input=0
				RemoPhase = 0;
				if( RemoLcnt>=1 && RemoLcnt<=5 ) {
					if( RemoHcnt<=2*3 ) 			// bit 0
						RemoData[RemoNum] <<= 1;
					else if( RemoHcnt<=4*3 ) {		// bit 1
						RemoData[RemoNum] <<= 1;
						RemoData[RemoNum]++;
					}
					else goto RemoError;

					if( ++RemoBit>=8 ) {
						RemoBit = 0;
						if( ++RemoNum>=4 ) {
							RemoDataReady = 1;
							RemoStep++;
						}
					}
					RemoLcnt = 0;

				}
				else goto RemoError;
			}
		}
		break;

	case 3:
		break;
	
	}
	return;

RemoError:
	ClearRemoTimer();
	EnableRemoInt();

}
#endif	//..REMO_NEC



//*****************************************************************************
//      UART 1 Interrupt                                                   
//*****************************************************************************
#ifdef SUPPORT_UART1

DATA 	BYTE	RS1_buf[RS_BUF_MAX];
DATA 	BYTE	RS1_in=0, RS1_out=0;
		bit		RS1_Xbusy=0;

/**
* UART1 interrupt service routine
* 
* If REG04B[6]=1, pin #120,#121 is assigned to #60,#61.
* enable SFR_ES1
*
*/
void uart1_int(void) interrupt 6 using 1
{
	BYTE	count;

#ifdef DEBUG_ISR
	MCU_INT_STATUS |= 0x0040;
	INTR_counter[6]++;
#endif

	if( SFRB_RI1 ) {						//--- Receive interrupt ----
		SFRB_RI1 = 0;
		//if ( SFR_UART1FIFO & 0x80 ) {		// is it use fifo?
			count = SFR_UART1FIFO & 0x1F;
			if ( count & 0x10) {
				SFR_UART1FIFO = 0x90;		// overflowed, buffer clear
			}
			else {
				while (count) {
					RS1_buf[RS1_in++] = SFR_SBUF1;
					if( RS1_in>=RS_BUF_MAX ) RS1_in = 0;
					count--;
				};
			}
		//}
		//else {
		//	RS1_buf[RS1_in++] = SFR_SBUF1;
		//	if( RS1_in >= RS_BUF_MAX ) RS1_in = 0;
		//}
	}

	if( SFRB_TI1 ) {					//--- Transmit interrupt ----
		SFRB_TI1 = 0;
		RS1_Xbusy=0;
	}
}
#endif

//*****************************************************************************
//*      Ext Int 2 Interrupt  (Low)			 		: <<DMA Done>>		P2.2	EXIF0
//*****************************************************************************
/*
*
* enable	SFR_EX1
*/
void ext2_int(void) interrupt 7 using 1
{
	BYTE val;

#ifdef DEBUG_ISR
	MCU_INT_STATUS |= 0x0080;
	INTR_counter[7]++;
#endif
	if(g_access==0)
		return;

	val = ReadTW8A(REG002);
	WriteTW8A(REG002, 0x80);	// Clear Int		
}

//*****************************************************************************
//*      Ext Int 3 Interrupt (Low)						: <<Touch Ready>>	P2.3	EXIF1
// enable REG0B1[7]=0 (default)
// status REG0B0[3]
// clear  REG0B0[3]=1
//*****************************************************************************
void ext3_int(void) interrupt 8 using 1
{
#ifdef DEBUG_ISR
	MCU_INT_STATUS |= 0x0100;
	INTR_counter[8]++;
#endif
}
//*****************************************************************************
//*      Ext Int 4 Interrupt (Low)						: XRAM 0x7FF access by I2C, reserved		EXIF2
//*****************************************************************************
/*
*
*	SFR_EINT4
*/
void ext4_int(void) interrupt 9 using 1
{
#ifdef DEBUG_ISR
	MCU_INT_STATUS |= 0x0200;
	INTR_counter[9]++;
#endif
	ExtIntCount++;
	INT_STATUS3 = ReadTW8A(REG002) & 0x10;
	WriteTW8A(REG002, INT_STATUS3);		//clear	

	//If you want to test, use SFR_EINT4 and skip "SFR_EINT4=1".
}
//*****************************************************************************
//*      Ext Int 5 Interrupt (Falling)					: reserved
//*****************************************************************************
void ext5_int(void) interrupt 10 using 1
{
#ifdef DEBUG_ISR
	MCU_INT_STATUS |= 0x0400;
	INTR_counter[10]++;
#endif
}
//*****************************************************************************
//*      Ext Int 6 Interrupt (Falling)					: <<Pen Int>> P2.4
//
// enable REG0B1[6]=0 (default)
// status REG0B0[4]
// clear  REG0B0[4]=1
// 
//*****************************************************************************
void ext6_int(void) interrupt 11 using 1
{
#ifdef DEBUG_ISR
	MCU_INT_STATUS |= 0x0800;
	INTR_counter[11]++;
#endif
}
//*****************************************************************************
//*      Watchdog Interrupt								: <<Watchdog>>
//*****************************************************************************
//===== WatchDog ======
#ifdef DEBUG_WATCHDOG
		bit		F_watch=0;
#endif


#pragma SAVE
#pragma OPTIMIZE(8,SPEED)
/**
* watchdog interrupt service routine
* 
*#ifdef DEBUG_WATCHDOG
* only for debug.
*#else
* clear CACHE
*#endif
*/
void watchdog_int(void) interrupt 12 using 3
{
#ifdef DEBUG_ISR
	MCU_INT_STATUS |= 0x1000;
	INTR_counter[12]++;
#endif

#ifdef DEBUG_WATCHDOG
	F_watch = 1;        //to indicate this interrupt was happen.
                        //and write RWT.
	SFRB_EA=0; 			//Disable Interrupt
	SFR_TA = 0xaa;
	SFR_TA = 0x55;
	SFR_WDCON = 0x03;	// - - - - WDIF WTRF EWT RWT..it to clear WDIF.
	SFRB_EA=1; 			//Enable Interrupt
#else
    /*
        disable cache & do not write RWT.
        system will be reset after 512 clock cycle base on 32KHz.
    */
    SFR_CACHE_EN=0;
#endif
}
#pragma RESTORE


//*****************************************************************************
//*      Ext Int 7 Interrupt (Programable)				: <<INT 7>>		P1.0
//*****************************************************************************
void ext7_int(void) interrupt 13 using 1
{
	EXINT_STATUS |= 1;
	ExtIntCount++;
#ifdef DEBUG_ISR
	INTR_counter[13]++;
#endif

	SFR_E2IF  = 0x01;		// Clear Flag, if Edge triggered
}
//*****************************************************************************
//*      Ext Int 8 Interrupt (Programable)				: <<INT 8>>		P1.1
//*****************************************************************************
void ext8_int(void) interrupt 14 using 1
{
	EXINT_STATUS |= 2;
#ifdef DEBUG_ISR
	INTR_counter[14]++;
#endif

	SFR_E2IF  = 0x02;		// Clear Flag, if Edge triggered
}
//*****************************************************************************
//*      Ext Int 9 Interrupt (Programable)				: <<INT 9>>		P1.2  
//*****************************************************************************
//On TW8836 EVB, pin62(P1.2) is assigned to DTVCLK2.
//and, remocon is assigned on P1.4
void ext9_int(void) interrupt 15 using 1
{
	EXINT_STATUS |= 4;
	ExtIntCount++;
#ifdef DEBUG_ISR
	INTR_counter[15]++;
#endif

	SFR_E2IF  = 0x04;		// Clear Flag, if Edge triggered
}
//*****************************************************************************
//*      Ext Int 10 Interrupt (Programable)				: <<INT 10>>	P1.3
//*****************************************************************************
/**
* ext10 interrupt service routine
* 
*/
void ext10_int(void) interrupt 16 using 1
{
	EXINT_STATUS |= 8;
	ExtIntCount++;
#ifdef DEBUG_ISR
	INTR_counter[16]++;
#endif

	SFR_E2IF  = 0x08;		// Clear Flag, if Edge triggered

}

//*****************************************************************************
//*      Ext Int 11 Interrupt (Programable)				: <<INT 11>		P1.4  Remocon
//*****************************************************************************
void ext11_int(void) interrupt 17 using 1
{
	EXINT_STATUS |= 0x10;
	ExtIntCount++;

#ifdef DEBUG_ISR
	INTR_counter[17]++;
#endif

	DisableRemoInt();
	SFR_E2IF  = 0x10;		// Clear Flag, if Edge triggered

	InitRemoTimer();
}
//*****************************************************************************
//*      Ext Int 12 Interrupt (Programable)				: <<INT 12>>	P1.5
//*****************************************************************************
// On TW8835, P1.5 was for PowerDown.
void ext12_int(void) interrupt 18 using 1
{
	EXINT_STATUS |= 0x20;
#ifdef DEBUG_ISR
	INTR_counter[18]++;
#endif

	SFR_E2IF  = 0x20;		// Clear Flag, if Edge triggered
}
//*****************************************************************************
//*      Ext Int 13 Interrupt (Programable)				: <<INT 13>>	P1.6
//*****************************************************************************
void ext13_int(void) interrupt 19 using 1
{
	EXINT_STATUS |= 0x40;
#ifdef DEBUG_ISR
	INTR_counter[19]++;
#endif

	SFR_E2IF  = 0x40;		// Clear Flag, if Edge triggered
}

//*****************************************************************************
//*      Ext Int 14 Interrupt (Programable)					: <<INT 14>>	P1.7
//*****************************************************************************
void ext14_int(void) interrupt 20 using 1
{
	EXINT_STATUS |= 0x80;
#ifdef DEBUG_ISR
	INTR_counter[20]++;
#endif

	SFR_E2IF  = 0x80;		// Clear Flag, if Edge triggered
}

//*****************************************************************************


//=============================================================================
//		Serial RX Check 												   
//=============================================================================
/**
* check Serial RX
*
* @return
*	1:something to read.  0:empty.
*	
*/
BYTE RS_ready(void)
{
	if( RS_in == RS_out ) return 0;
	else return 1;
}

#ifdef SUPPORT_UART1
BYTE RS1_ready(void)
{
	if( RS1_in == RS1_out ) return 0;
	else return 1;
}
#endif

//=============================================================================
//		Serial RX														   
//=============================================================================
/**
* get Rx data
*/
BYTE RS_rx(void)
{
	BYTE	ret;

	SFRB_ES = 0;

	ret = RS_buf[RS_out];
	RS_out++;
	if(RS_out >= RS_BUF_MAX) 
		RS_out = 0;

	SFRB_ES = 1;

	return ret;
}
#ifdef SUPPORT_UART1
BYTE RS1_rx(void)
{
	BYTE	ret;

	SFRB_ES1 = 0;
	ret = RS1_buf[RS1_out];
	RS1_out++;
	if(RS1_out >= RS_BUF_MAX) 
		RS1_out = 0;
	SFRB_ES1 = 1;

	return ret;
}
#endif

/**
* unget Rx data
*/
void RS_ungetch(BYTE ch)
{
	SFRB_ES = 0;
	RS_buf[RS_in++] = ch;
	if( RS_in>=RS_BUF_MAX ) RS_in = 0;
	SFRB_ES = 1;
}
#ifdef SUPPORT_UART1
void RS1_ungetch(BYTE ch)
{
	SFRB_ES1 = 0;
	RS1_buf[RS1_in++] = ch;
	if( RS1_in >=RS_BUF_MAX ) RS1_in = 0;
	SFRB_ES1 = 1;
}
#endif

//=============================================================================
//		Serial TX														   
//=============================================================================
/**
* send Tx
*/
void RS_tx(BYTE tx_buf)
{
	while(RS_Xbusy);

	SFRB_ES = 0;
	SFR_SBUF = tx_buf;
	RS_Xbusy=1;
	SFRB_ES = 1;
}
#ifdef SUPPORT_UART1
void RS1_tx(BYTE tx_buf)
{
	while(RS1_Xbusy);

	SFRB_ES1 = 0;
	SFR_SBUF1 = tx_buf;
	RS1_Xbusy=1;
	SFRB_ES1 = 1;
}
#endif

#ifdef DEBUG_UART
void DEBUG_dump_uart0(void)
{
	BYTE i;
	if(UART0_OVERFLOW_counter) {
		Printf("\n\rUART0 Overflow:%bd",UART0_OVERFLOW_counter);
		UART0_OVERFLOW_counter = 0;  //clear
	}
	if(UART0_MAX_counter) {
		Printf("\n\rUART0 Max:%bd",UART0_MAX_counter);
		UART0_MAX_counter = 0;  //clear
	}
	Printf("\n\rRS_buf[%bd], RS_in:%bd RS_out:%bd",RS_BUF_MAX,RS_in,RS_out);
	for(i=0; i <  RS_BUF_MAX; i++)
		Printf(" %02bx", RS_buf[i]);
}
#endif


//=============================================================================
//														   
//=============================================================================
/**
* 1ms delay
*
* tic_pc will be increased every 1 timer0 interrupt that based 1ms.
* tic_pc: 0~0xffff and will not increased after 0xffff.
* @param cnt_1ms have to less then 65536. max 65sec delay
*
* tic_pc uses WORD size. It is not an automic. It needs SFRB_EA.
*/
void delay1ms(WORD cnt_1ms)
{
	volatile WORD temp_tic_pc;
	SFRB_ET0 = 0;
	tic_pc = 0;								//clear
	SFRB_ET0 = 1;
	do {
		SFRB_ET0=0;
		temp_tic_pc = tic_pc;				//read
		SFRB_ET0=1;
	} while(temp_tic_pc < cnt_1ms);			//compare
}

/**
* 1sec delay
*/
void delay1s(WORD cnt_1s, WORD line)
{
	WORD i;
	Printf("\n\rWait%ds @%d",cnt_1s,line);
	for(i=0; i < cnt_1s; i++) {
		delay1ms(1000);
	}
}

//=============================================================================
//                            Watchdog                                                   
//=============================================================================
/*
watchdog programming

1. Select interval value WD1,WD0.
2. Enable watchdog
    WDCON.1(EWT):Enable Watchdog Timer.
3. Reset Watchdog before it reach 
    WDCON.0(RWT):Reset Watchdog Timer.


other watchdog control bits
WDCON.3(WDIF):Watchdog Interrupt flag. This bit is set 512 machine cycle before the watchdog do a system reset.
WDCON.2(WTRF):Watchdog Reset Has Ocurred. This bit is set when watchdog reset has occured.
            If it was clear, the last reset was not caused by Watchdog.

Watchdog Interrupt
EIE.5(EWDI):Enable Watchdog Interrupt. 
    When this bit is set, an interrupt through vector 63h will be triggered when WDCON.3.
EIP.5(PWDI):Watchdog Interrupt Priority.
    When this bit i set, the watchdog interrupt is assigned high priority.
    When this bit is clear the watchdog interrupt is assigned low priority.

Watchdog System Reset
WDCON.2(WTRF) will be 1.
Other WDCON bits becomes 0.
*/
#pragma SAVE
#pragma OPTIMIZE(8,SIZE)

#if defined(SUPPORT_WATCHDOG) || defined(DEBUG_WATCHDOG)
/**
* restart watchdog
*
*       MOV TA,#0AAh    ;Execute the Timed Access Protection.
*       MOV TA,#055h    ;code to open the Timed Access window.
*       SETB    RWT     ;Reset the watchdog timer by setting the RWT bit.
*/
void RestartWatchdog(void)
{
	SFRB_EWDI = 0;		// Disable WDT Interrupt

	SFRB_EA=0; 			//Disable Interrupt
	SFR_TA = 0xaa;
	SFR_TA = 0x55;
	SFR_WDCON = 0x03;	// - - - - WDIF WTRF EWT RWT.  Reset Watchdog
	SFRB_EA=1; 			//Enable Interrupt
	
#ifdef DEBUG_WATCHDOG
	F_watch = 0;
#else
    if(SFR_CACHE_EN==0)
        SFR_CACHE_EN=1;
#endif
	SFRB_EWDI = 1;		// Enable WDT Interrupt (disable for test)
}

//extern void EE_Increase_Counter_Watchdog(void);

/**
* enable Watchdog
*
* 27MHz MCU clock with EnableWatchdog(1),
*   type "delay 4340" on TW-Terminal.
*/
void EnableWatchdog(BYTE mode)
{
	Printf("\n\rEnableWatchdog(%bd)",mode);
	SFRB_EWDI = 0;		// Disable WDT Interrupt

	/* SFR_CKCON (0x8E) : base 32kHz clock.
	mode WD[1:0]	CLK(on TW8836B)	CLK(on FPGA)	Delay
	3	00			2^11			2^17			70mSec
	2	01			2^14			2^20			540msec
	1	10			2^17			2^23			4 sec
	0	11			2^20			2^26			32 sec
	*/
	SFR_CKCON &= 0x3F;
	switch(mode) {
	case 0: 	SFR_CKCON |= 0xc0;	break;	
	case 1: 	SFR_CKCON |= 0x80;	break;	
	case 2: 	SFR_CKCON |= 0x40;	break;	
	case 3:		SFR_CKCON |= 0x00;	break;	
	default: 	SFR_CKCON |= 0xc0;	break;
	}
	SFRB_EA=0; 			//Disable Interrupt
	SFR_TA = 0xaa;
	SFR_TA = 0x55;
	SFR_WDCON = 0x03;	// - - - - WDIF WTRF EWT RWT.
	SFRB_EA=1; 			//Enable Interrupt

	SFR_EIP |= 0x20;	// BK160208:We have 512 counter, is it enough ?
	SFRB_EWDI = 1;		// Enable WDT Interrupt (disable for test)

#ifdef DEBUG_WATCHDOG
	F_watch = 1; //for first  wdt_last value
#endif
}

/*
Note: See the OK and NG code. If you use OPTIMIZE(9,), the compiler made a NG code.
OK:
	MOV     SFR_TA,#0AAH
	MOV     SFR_TA,#055H
	MOV     SFR_WDCON,#00H
	SETB    SFR_EWDI

NG:
	MOV     SFR_TA,#0AAH
	MOV     SFR_TA,#055H
	CLR     A
	MOV     SFR_WDCON,A
	CLR     SFR_EWDI
*/

/**
* disable watchdog
*/
void DisableWatchdog(void)
{
	SFRB_EWDI = 0;		// Disable WDT Interrupt

	SFRB_EA=0; 			//Disable Interrupt
	SFR_TA = 0xaa;
	SFR_TA = 0x55;
	SFR_WDCON = 0x00;	// - - - - WDIF WTRF EWT RWT
	SFRB_EA=1; 			//Enable Interrupt

	Puts("DisableWatchdog\n");
}
#pragma RESTORE
#endif //..defined(SUPPORT_WATCHDOG) || defined(DEBUG_WATCHDOG)

/**
* enable external interrupt
*/
void EnableExtInterrupt(BYTE intrn)
{
	SFR_E2IE |= (1 << intrn);	
}
/**
* disable externel interrupt
*/
void DisableExtInterrupt(BYTE intrn)
{
	SFR_E2IE &= ~(1 << intrn);	
}


//=============================================================================
//			Remocon
//=============================================================================



//On TW8835, it was use P1_2<<INT9>>.
//On TW8836, FW using P1.4<<INT11>>.
#ifdef REMO_RC5

/**
* init remocon timer
*/
void InitRemoTimer(void)
{
#ifdef DEBUG_REMO
	BYTE i;
#endif

	ClearRemoTimer();			//T2CON = 0x00;				// 
	 
	SFRB_ET2  = 0;				// Disable Timer2 Interrupt
	SFR_T2IF = 0x00;			// Clear Flag

								// Reload Value
	SFR_RCRH = SFR_TH2 = 0xff;	// 0xFF2E = 0x10000-0xD2 = 0x10000-210. 
	SFR_RCRL = SFR_TL2 = 0x2e;	// it means 210 usec interval. 

	SFR_RCRL = SFR_TL2 = 0x2D;		
								//RC5 uses 14 bytes.
								//RC5 spec uses a 24892us for 14 byte.
								//TW8835 use a 8 sampling per one bit 
								//If remocon use a 24892us for 14 Bytes, we have to assign 222us interval.
								// 
								//When I measure a data1 signal from the first falling edge to the last up edge, it was 22800us.
								//and it starts from first falling edge.
								// 22800us = (14*8-4) * interval. 
								// the best interval value is 211us.


	SFR_T2CON = 0x12;			// 0001 0010 
								// |||| |||+-- T2I0 \ Timer2 Input Selection 
								// ||||	||+--- T2I1 / 00=No,  01=Timer,  10=Counter, 11=Gate
								// ||||	|+---- T2CM:  Compare mode
								// ||||	+----- T2R0 \ Timer2 Reload Mode 
								// |||+------- T2R1	/ 00=No,  01=No,     10=Auto,    11=pin T2EX
								// ||+-------- ---
								// |+--------- I3FR: Timer2 Compare0 Interrupt Edge...
								// +---------- T2PS: Timer2 Prescaler

	//start from...
	RemoTic = 4;
	RemoPhase1 = 1;

#ifdef DEBUG_REMO
	if(RemoCaptureDisable==0) {
		for(i=0; i <= 14; i++) {
			RemoCapture0[i]=0x00;
			RemoCapture1[i]=0x00;
			RemoCapture2[i]=0x00;
		}
	}
#endif
	//BKFYI.
	//the next timer2 ISR will be RemoTic 5, and ISR will capture the sample RemoPhase2.
	//The RemoPhase2 have to be 0 in our system(Active Low). 

	RemoSystemCode = 0;
	RemoDataCode = 0;

	SFRB_ET2  = 1;					// Enable Timer 2 Interrupt
}

#elif defined REMO_NEC

void InitRemoTimer(void)
{
	WORD temp;

	ClearRemoTimer();			//T2CON = 0x00;	
	 
	ET2  = 0;					// Disable Timer2 Interrupt
	T2IF = 0x00;				// Clear Flag

								//need 186.667us 
	temp = 0x10000 - 187; 		//or 186	0x10000- BA. 186.667uS  

	CRCH = TH2 = temp>>8;
	CRCL = TL2 = (BYTE)(temp & 0xff);

	T2CON = 0x12;				// 0001 0010 
								// |||| |||+-- T2I0 \ Timer2 Input Selection 
								// ||||	||+--- T2I1 / 00=No,  01=Timer,  10=Counter, 11=Gate
								// ||||	|+---- T2CM:  Compare mode
								// ||||	+----- T2R0 \ Timer2 Reload Mode 
								// |||+------- T2R1	/ 00=No,  01=No,     10=Auto,    11=pin T2EX
								// ||+-------- ---
								// |+--------- I3FR: Timer2 Compare0 Interrupt Edge...
								// +---------- T2PS: Timer2 Prescaler


	RemoTic = 0;		//tm01 = 0;
	RemoStep  = 0;
	RemoPhase = 0;
	RemoHcnt  = 0;
	RemoLcnt  = 0;

	ET2  = 1;					// Enable Timer 2 Interrupt
}
#endif

//=============================================================================
//                            Initialize CPU                                                   
//=============================================================================
/**
* initialize MCU(DP80390)
*/
void init_cpu(BYTE fWatchDog)
{
#ifdef DEBUG_ISR
	BYTE i;
#endif

	TWBASE = 0x00;					// Base address of TW88xx
	SFR_CAMC = 1;					// Chip Access Mode Control. E2[0]=1b:16bit mode

    if (!fWatchDog) 
	{
    	//---------- Initialize Timer Divider ---------
    	WriteTW88(REG4E2, 0x69);		// Timer0 Divider : system tic 0. 
    	WriteTW88(REG4E3, 0x78);		// 27M/27000 = 1msec
    
    	WriteTW88(REG4E4, 0x01);		// Timer1 Divider : for Touch
    	WriteTW88(REG4E5, 0x0e);		// 27M/270 = 10usec	
    
    	WriteTW88(REG4E6, 0);			// Timer2 Divider : remo timer
    	WriteTW88(REG4E7, 0x1b);		// 27M/27 = 1usec
    
    	WriteTW88(REG4E8, 0);			// Timer3 Divider : baudrate for UART0
    	WriteTW88(REG4E9, 0x0c);		// (22.1184M *2 /32) / 12 = 115200bps on SM0=1 & SMOD0=1	
    
    	WriteTW88(REG4EA, 0);			// Timer4 Divider : baudrate for UART1
    	WriteTW88(REG4EB, 0x0c);		// (22.1184M *2 /32) / 12 = 115200bps on SM1=1 & SMOD1=1	
    
    	/*If you want the UART1 swap, add below code. */
    	/* WriteTW88(REG04B, ReadTW88(REG04B) | 0x40);	*/
    }

#ifdef DEBUG_ISR
	for (i=0; i < 21; i++)
		INTR_counter[i]=0;
#endif

	//---------- Initialize interrupt -------------
	SFR_CKCON = 0x00;		// Clock control register			
							// 0000 0000
							// |||| |||+-- MD0 \.
							// |||| ||+--- MD1 	> MD[2:0] Stretch RD/WR timing
							// |||| |+---- MD2 /
							// |||| +----- T0M:  Timer0 Pre-Divider 0=div by 12,  1=div by 4
							// |||+------- T1M:  Timer1 Pre-Divider 0=div by 12,  1=div by 4
							// ||+-------- ---
							// |+--------- WD0 \ Watchdong Timeout Period
							// +---------- WD1 / 00=2^17,  01=2^20,  10=2^23,  11=2^26

    SFR_TMOD = 0x66;		// 0110 0110
							// |||| ||||   << Timer 0 >>
							// |||| |||+-- M0 \  00= 8bit timer,counter /32  01= 16bit timer,counter
							// |||| ||+--- M1 /  10= 8bit auto reload        11= 8bit timer,counter
							// |||| |+---- CT:   0=Timer Mode,    1=Counter Mode
							// |||| +----- GATE: 0=GATE not used, 1=GATE used
							// ||||        << Timer 1 >>
							// |||+------- M0 \  00= 8bit timer,counter /32  01= 16bit timer,counter
							// ||+-------- M1 /  10= 8bit auto reload        11= 8bit timer,counter
							// |+--------- CT:   0=Timer Mode,    1=Counter Mode
							// +---------- GATE: 0=GATE not used, 1=GATE used

    SFR_TCON = 0x55;		// 0101 0101
							// |||| |||+-- IT0:  INT0 Trigger 0=level, 1=edge
							// |||| ||+--- IE0:  INT0 Interrupt Flag
							// |||| |+---- IT1:  INT1 Trigger 0=level, 1=edge
							// |||| +----- IE1:  INT1 Interrupt Flag
							// |||+------- TR0:  Timer0 Run
							// ||+-------- TF0:  Timer0 Flag
							// |+--------- TR1:  Timer1 Run
							// +---------- TF0:  Timer1 Flag

 	SFR_TH0 = 0xff;			// 0xFFFF = 0x10000-1 = 1 msec
	SFR_TL0 = 0xff;			//

							// for TOUCH SAR sensing timer
	SFR_TH1 = 206;			// 
							// TH1 = 156. 1ms
							// TH1 = 206. 0.5ms = 50*10usec

	SFR_T2CON = 0x12;		// 0001 0010 
							// |||| |||+-- T2I0 \ Timer2 Input Selection 
							// ||||	||+--- T2I1 / 00=No,  01=Timer,  10=Counter, 11=Gate
							// ||||	|+---- T2CM:  Compare mode
							// ||||	+----- T2R0 \ Timer2 Reload Mode 
							// |||+------- T2R1	/ 00=No,  01=No,     10=Auto,    11=pin T2EX
							// ||+-------- ---
							// |+--------- I3FR: Timer2 Compare0 Interrupt Edge...
							// +---------- T2PS: Timer2 Prescaler
    		
	SFR_TH2 = 0xff;	        // 0xFF2E = 0x10000-0xD2 = 0x10000-210. 
	SFR_TL2 = 0x2e;	        // it means 210 usec interval. 

	SFR_PCON = 0xc0;		// 1100 0000
							// |||| |||+-- PMM:  Power Management Mode 0=Disable,  1=Enable
							// |||| ||+--- STOP: Stop Mode             0=Disable,  1=Enable
							// |||| |+---- SWB:  Switch Back from STOP 0=Disable,  1=Enable
							// |||| +----- ---
							// |||+------- PWE:	 (Program write Enable)
							// ||+-------- ---
							// |+--------- SMOD1:UART1 Double baudrate bit
							// +---------- SMOD0:UART0 Double baudrate bit

	SFR_SCON = 0x50;		// 0101 0000
							// |||| |||+-- RI:   Receive Interrupt Flag
							// |||| ||+--- TI:   Transmit Interrupt Flag
							// |||| |+---- RB08: 9th RX data
							// |||| +----- TB08: 9th TX data
							// |||+------- REN:	 Enable Serial Reception
							// ||+-------- SMO2: Enable Multiprocessor communication
							// |+--------- SM01 \   Baudrate Mode
							// +---------- SM00 / 00=f/12,  01=8bit var,  10=9bit,f/32,f/64,  11=9bit var

	SFR_SCON1 = 0x50;		// 0101 0000
							// |||| |||+-- RI:   Receive Interrupt Flag
							// |||| ||+--- TI:   Transmit Interrupt Flag
							// |||| |+---- RB08: 9th RX data
							// |||| +----- TB08: 9th TX data
							// |||+------- REN:	 Enable Serial Reception
							// ||+-------- SMO2: Enable Multiprocessor communication
							// |+--------- SM11 \   Baudrate Mode
							// +---------- SM10 / 00=f/12,  01=8bit var,  10=9bit,f/32,f/64,  11=9bit var

	//---------- Enable Individual Interrupt ----------
	SFR_IP	 = 0x10;		// 0001 0000 interrupt priority
                            // |||| |||+-- PX0 EX0
                            // |||| ||+--- RT0 ET0
                            // |||| |+---- PX1 EX1
                            // |||| +----- PT1 ET1
                            // |||+------- PS  ES (use register bank 2)
                            // ||+-------- PT2 ET2
                            // |+--------- PS1 ES1
                            // +---------- unused 					

	SFR_IE	 = 0x12;		// 1001 0010 interrupt enable
                            // |||| |||+ EX0    : Chip Interrupt. I will enable it later.
                            // |||| ||+- ET0    : Timer0    System Tic
                            // |||| |+-- EX1    : DE End
                            // |||| +--- ET1    : timer1 - touch
                            // |||+----- ES     : UART0
                            // ||+------ ET2    : timer2 - remocon
                            // |+------- ES1    : UART1
                            // +-------- EA     : Global Interrupt. I will enable it below.

#ifdef SUPPORT_UART1
	SFRB_ES1  = 1;			// UART1  	: External MCU
#endif

	//---------- Serial FIFO ----------
	SFR_UART0FIFO = 0x80;	//          : UART0 FIFO

#ifdef SUPPORT_UART1
	SFR_UART1FIFO = 0x80;	//          : UART1 FIFO
#endif

	//---------- Enable global Interrupt ----------
	SFRB_EA   = 1;			// Global Interrupt


	//---------- Extended Interrupt -------------------
	SFR_EIE	 = 0x04;		// 0000 0100 extended interrupt enable
                            // |||| |||+ EINT2  : SPI-DMA done
                            // |||| ||+- EINT3  : Touch Ready
                            // |||| |+-- EINT4  : SW 7FF 
                            // |||| +--- EINT5  : reserved
                            // |||+----- EINT6  : Pen
                            // ||+------ EWDI   : Watchdog
                            // |+------- reserved
                            // +-------- reserved
    SFR_EIP  = 0x00;
                            /* If Watchdog, EIE.5 & EIP.5 will be 1. */
	//---------- Extended2 Interrupt 7~14 Config. ---------------
                            // xxxx xxxx
                            // |||| |||+- EINT7
                            // |||| ||+-- EINT8
                            // |||| |+--- EINT9  :Remocon. SW will enable 9 later.
                            // |||| +---- EINT10 :MODEL_TW8835_EXTI2C. VideoISR.
                            // |||+------ EINT11
                            // ||+------- EINT12
                            // |+-------- EINT13
                            // +--------- EINT14
							// 
    //SFR_E2IF  = 0x00;		// (0xFA)	0000 0000 : Interrupt Flag         
	SFR_E2IE  = 0x00;		// (0xFB)	0000 0000 : Interrupt Enable.
	SFR_E2IP  = 0x00;		// (0xFC)	0000 0000 : Interrupt Priority
	SFR_E2IM  = 0xff;		// (0xFD)	0000 0000 : Interrupt Mode(0=Level, 1=Edge)
	SFR_E2IT  = 0x00;		// (0xFE)	0000 0000 : Level/Edge Polarity  (0=Low/Falling, 1=High/Rising)
							/*                      0: Low Level,  Falling Edge	*/
							/*                      1: High Level, Rising Edge */

	Puts("\n\n");
	Puts("init_cpu");

	if (SFR_WDCON & 0x04) 
        Puts(" WTRF");   //<--SFRB_WTRF

	Puts("\n");

	/* BKFYI:
		TW8836B2 always use 16bit access.
		If you want to check, use below code.

	if(SFR_CAMC==1)
		Puts("16Bit Access");
	*/

	//------- Remote Controller (EINT11, Timer2) --------

	//ClearRemoTimer. RemoINTR(EINT9) will be activateed in RemoTimer		

	/* BKFYI:
		The cache enable routine is moved to InitCore.
	*/
}

//=============================================================================
//		Time
//=============================================================================
#ifdef SUPPORT_FOSD_MENU
DATA	BYTE	SleepTimer;
DATA	WORD	SleepTime;
#define _24H_SECS			86400L			// 24*60*60

WORD DiffTime_ms( WORD stime, WORD etime )
{
	if( etime < stime ) {			// resetted
		return etime + (6000 - stime);
	}
	else {
		return etime - stime;
	}
}

WORD GetTime_ms(void)
{
	WORD tms;

	tms = tic01;
	tms += ( SystemClock * 1000 );
	return tms;	// in ms
}
BYTE GetTime_H(void)
{
	return ( SystemClock / 60 / 60 ) % 24 ;
}
BYTE GetTime_M(void)
{
	return ( SystemClock / 60  ) % 60 ;
}
BYTE GetSleepTimer(void)
{
	WORD val;

	val = SleepTimer;
	if( val )	{	// already set, display rest of time
		val = ( SleepTime >> 8 ) * 60 + ( SleepTime & 0xff );
		val -= ( GetTime_H() * 60 + GetTime_M() );
	}
	return (BYTE)val;
}
void SetSleepTimer(BYTE stime)
{
	SleepTimer = stime;
	if( SleepTimer==0 )
		SleepTime = 0xffff;
	else {
		SleepTime = GetTime_H() + ( GetTime_M() + SleepTimer ) / 60;
		SleepTime = ( SleepTime << 8 ) | ( ( GetTime_M() + SleepTimer ) % 60 );
	}

	#ifdef DEBUG_TIME
	dPrintf("\n\rSetSleepTimer(SleepTime:0x%bx)", SleepTime);
	#endif
}
#endif

