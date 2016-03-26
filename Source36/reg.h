/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/
//=============================================================================
//		TW8836 SFRs - compatible with DP80390XP
//=============================================================================
#ifndef __REGISTERS_H__
#define __REGISTERS_H__

/*
SFR Memory Map (TW8836)
===============
	0/8		1/9		2/A		3/B		4/C		5/D		6/E		7/F
	---		---		---		---		---		---		---		---
80:	P0		SP		DPL0	DPH0	DPL1	DPH1	DPS		PCON
88:	TCON	TMOD	TL0		TL1		TH0		TH1		CKCON
90:	P1		EXIF	(WTST)	(DPX0)			(DPX1)
98:	SCON0	SBUF0	BANK	   		CACHE	ACON	UART0FIFO UART1FIFO
A0:	P2
A8:	IE
B0:	P3
B8:	IP
C0:	SCON1	SBUF1	CCL1	CCH1	CCL2	CCH2	CCL3	CCH3
C8:	T2CON	T2IF	RLDL	RLDH	TL2		TH2		CCEN
D0:	PSW
D8:	WDCON
E0:	ACC				CAMC
E8:	EIE		STATUS	(MXAX) 	TA
F0:	B	
F8:	EIP				E2IF	E2IE	E2IP	E2IM	E2IT	__
------------------------------------------------------------------
Standard - standard 8051 register, not described in this document
bolded - new or modified nonstandard 8051 register, described in this document
( ) - reserved register, do not change
* Any reserved register bit shouldn.t be changed from reset value.

*/

/*
SFR F0: 01 BF BF BF BF BF BF BF - 00 BF EF 10 00 FF 00 BF 
SFR E0: 20 BF 01 BF BF BF BF BF - 04 04 00 00 BF BF BF BF 
SFR D0: 00 BF BF BF BF BF BF BF - 08 BF BF BF BF BF BF BF 
SFR C0: 51 00 00 00 00 00 00 55 - 00 00 2D FF 38 FF 00 BF 
SFR B0: 11 BF BF BF BF BF BF BF - 02 BF BF BF BF BF BF BF 
SFR A0: FF BF BF BF BF BF BF BF - 9B BF BF BF BF BF BF BF 
SFR 90: 5F 00 07 01 BF 00 00 90 - 54 75 00 00 01 02 80 00 
SFR 80: FF 8F B9 29 00 00 00 C0 - 55 66 FF F8 FF CE 00 BF 

*/

//===========================================
//				BYTE Registers
//===========================================
sfr SFR_P0			= 0x80;	/* Port 0                    */
	sbit P0_0  		= SFR_P0^0; //0x80;
	sbit P0_1  		= SFR_P0^1; //0x81;
	sbit P0_2  		= SFR_P0^2; //0x82;
	sbit P0_3  		= SFR_P0^3; //0x83;
	sbit P0_4  		= SFR_P0^4; //0x84;
	sbit P0_5  		= SFR_P0^5; //0x85;
	sbit P0_6  		= SFR_P0^6; //0x86;
	sbit P0_7  		= SFR_P0^7; //0x87;
sfr SFR_SP			= 0x81;	/* Stack Pointer             */
sfr SFR_DPL			= 0x82;	/* Data Pointer 0 Low byte   */
sfr SFR_DPL0		= 0x82;
sfr SFR_DPH			= 0x83;	/* Data Pointer 0 High byte  */
sfr SFR_DPH0		= 0x83;
sfr SFR_DPL1		= 0x84; /* TW88 :: Data Pointer 1 Low byte */
sfr SFR_DPH1		= 0x85; /* TW88 :: Data Pointer 1 High byte */
sfr SFR_DPS			= 0x86; /* TW88 :: Data Pointers Select register */
						    //[7:6] ID1,ID0
							//[5:5] TSL
							//[4:3] AU1,AU0
							//[0:0] SEL
sfr SFR_PCON		= 0x87;	/* TW88 :: Power Configuration       */
	//^0 PMM	1:Enabled Power Management Mode
	//^1 STOP	1:Enabled STOP mode
	//^2 SWB	1:Enabled SwitchBack
	//^4 PWE	1:
	//^6 SMOD1	Double baudrate for UART1.
	//^7 SMOD0	Double baudrate for UART0.

sfr SFR_TCON		= 0x88;	/* Timer 0,1 Configuration   */
	sbit SFRB_IT0   = SFR_TCON^0; //0x88;
	sbit SFRB_IE0   = SFR_TCON^1; //0x89;
	sbit SFRB_IT1   = SFR_TCON^2; //0x8A;
	sbit SFRB_IE1   = SFR_TCON^3; //0x8B;
	sbit SFRB_TR0   = SFR_TCON^4; //0x8C;
	sbit SFRB_TF0   = SFR_TCON^5; //0x8D;
	sbit SFRB_TR1   = SFR_TCON^6; //0x8E;
	sbit SFRB_TF1   = SFR_TCON^7; //0x8F;

sfr SFR_TMOD		= 0x89;	/* Timer 0,1 Mode            */
	//^0	T0M0    Timer0 mode
	//^1	T0M1	Timer0 mode
	//^2	CT0	   	0: pulse from internal oscillator
	//             	1: pulse from T0 pin on P3.4
	//^3	GATE0  	1:timer0 operates only if INT0 bit on P3.2 is set.
	//^4	T1M0	Timer1 mode
	//^5	T1M1	Timer1 mode
	//^6	CT1		0: pulse from internal oscillator
	//              1: pulse from T1 pin on P3.5	  
	//^7	GATE1  	1:timer1 operates only if INT1 bit on P3.3 is set.
	//
	//		<<timer mode>>
	//			mode0:	8bit timer, counter/32
	//			mode1:	16bit timer, counter
	//			mode2:	8bit auto-reload
	//			mode3:	8bit timer, counter


sfr SFR_TL0			= 0x8A;	/* Timer 0 Low byte counter  */
sfr SFR_TL1			= 0x8B;	/* Timer 1 Low byte counter  */
sfr SFR_TH0			= 0x8C;	/* Timer 0 High byte counter */
sfr SFR_TH1			= 0x8D;	/* Timer 1 High byte counter */
sfr SFR_CKCON		= 0x8E;	/* Clock Control */
	//^3	T0M  0:divided by 12, 1:divided by 4  
	//^4	T1M	 0:divided by 12, 1:divided by 4    
	//^6	WD0  Watchdog timer timeout period  
	//^7	WD1 
	//		WD[1:0]			# of clocks
	//			00	2**11	131072   
	//			01	2**14	1048576
	//			10	2**17	8388608
	//			11	2**20	67108864

sfr SFR_P1			= 0x90;	/* Port 1                    */
	sbit P1_0  		= SFR_P1^0; //0x90;
	sbit P1_1  		= SFR_P1^1; //0x91;
	sbit P1_2  		= SFR_P1^2; //0x92;
	sbit P1_3  		= SFR_P1^3; //0x93;
	sbit P1_4  		= SFR_P1^4; //0x94;
	sbit P1_5  		= SFR_P1^5; //0x95;
	sbit P1_6  		= SFR_P1^6; //0x96;
	sbit P1_7  		= SFR_P1^7; //0x97;
	sbit SFRB_T2    = SFR_P1^0; //0x90;
	sbit SFRB_T2EX  = SFR_P1^1;	//0x91;

sfr SFR_EXIF		= 0x91;	/* TW88 :: Extended Interrupt Priority register, aka. EIF */
sfr SFR_EIF			= 0x91;	/* External Interrupt Flag   */
    //sbit SFRB_EINT2F = SFR_EIF^0; EINT2 interrupt flag						        
    //sbit SFRB_EINT3F = SFR_EIF^1; EINT2 interrupt flag						        
    //sbit SFRB_EINT4F = SFR_EIF^2; EINT2 interrupt flag						        
    //sbit SFRB_EINT5F = SFR_EIF^3; EINT2 interrupt flag						        
    //sbit SFRB_EINT6F = SFR_EIF^4; EINT2 interrupt flag						        

sfr SFR_WTST		= 0x92;	/* TW88 :: Program Memory Wait States */
							/* DS80C390:: __sfr __at 0x92 P4CNT; */
sfr SFR_DPX			= 0x93;	/* TW88 :: Data Page Pointer 0       */
sfr     DPX			= 0x93;	/* i2c.src need it */
sfr SFR_DPP			= 0x93;
sfr SFR_DPX0		= 0x93;
sfr SFR_DPX1		= 0x95;	/* TW88 :: Data Page Pointer 1       */

sfr SFR_SCON		= 0x98;	/* Serial 0 Configuration    */
sfr SFR_SCON0		= 0x98;
	sbit SFRB_RI    = SFR_SCON^0; //0x98;
	sbit SFRB_TI    = SFR_SCON^1; //0x99;
	sbit SFRB_RB8   = SFR_SCON^2; //0x9A;
	sbit SFRB_TB8   = SFR_SCON^3; //0x9B;
	sbit SFRB_REN   = SFR_SCON^4; //0x9C;
	sbit SFRB_SM2   = SFR_SCON^5; //0x9D;	Multiprocessor
	sbit SFRB_SM1   = SFR_SCON^6; //0x9E;	SerialMode
	sbit SFRB_SM0   = SFR_SCON^7; //0x9F;	SerialMode
	//  SerialMode						BuadRate
	//		0	8bit shift register		Oscillator / 12. 
	//		1	8bit UART				Set by timer3. (worked with SMOD0)
	//		2	9bit UART				Oscillator / 64. (worked with SMOD0)
	//		3	9bit UART				Set by Tomer3. (worked with SMOD0)

sfr SFR_SBUF		= 0x99;	/* Serial 0 I/O Buffer       */
sfr SFR_SBUF0		= 0x99;
sfr TWBASE			= 0x9a;	/* TW88 :: TW88xx base address. Code Bank Address */
sfr	SFR_BANKREG		= 0x9A;	/*      ::aka BANKREG */
							/*      [7-0]	RG_PGMBASE	R/W	CodeBank Address	def:0 */
							/* DS80C390:: sfr SFR_ESP			= 0x9B;*/
sfr SFR_CACHE_EN	= 0x9C;	/* TW88 :: Cache Control */
							/*      [1]	PG_PWND		R/W PowerDown Cache */
							/*      [0]	CACHE_EN	R/W 1:Enable Cache 0:Disable Cache */

							/* DS80C390:: __sfr __at 0x9c AP */

sfr SFR_ACON		= 0x9D;	/* TW88 :: Address Control FLAT/LARGE*/
                            /*      [0] AM0 1:24 bit page address */
                            /*      [1] AM1 1:24 bit continus address */
sfr	SFR_UART0FIFO 	= 0x9E;	/* TW88 :: UART0 FIFO */
                            /*      [7] UART0FFEN UART0 FIFO enable  */ 
                            /*      [4] UART0FFOVF UART0 overflow status; write this bit with "1" to clear it */ 
                            /*      [3:0] UART0FFBYT UART0 FIFO valid byte number  */ 
sfr	SFR_UART1FIFO 	= 0x9F;	/* TW88 :: UART1 FIFO */
                            /*      [7] UART1FFEN UART1 FIFO enable  */ 
                            /*      [4] UART1FFOVF UART1 overflow status; write this bit with "1" to clear it */ 
                            /*      [3:0] UART1FFBYT UART1 FIFO valid byte number  */ 

sfr SFR_P2			= 0xA0;	/* Port 2                    */	
	sbit P2_0  		= SFR_P2^0; //0xA0; Chip Interrupt Status. Same as INT0.
	sbit P2_1  		= SFR_P2^1; //0xA1;	VBLANK Interrupt Status. Same as INT1.
	sbit P2_2  		= SFR_P2^2; //0xA2;	SPI-DMA Done Interrupt Status. Same as INT2.
	sbit P2_3  		= SFR_P2^3; //0xA3;	Touch/SAR ADC Ready Interrupt Status. Same as INT3.
	sbit P2_4  		= SFR_P2^4; //0xA4;	Touch Pen Interrupt Status. Same as INT6.
	sbit P2_5  		= SFR_P2^5; //0xA5;
	sbit P2_6  		= SFR_P2^6; //0xA6;
	sbit P2_7  		= SFR_P2^7; //0xA7;	Power Down Crystal. 
	                            // To power down crystal, set P2.7=0 and RRG0D4[7]=1
							/* DS80C390:: __sfr __at 0xa1 P5;*/
							/* DS80C390:: __sfr __at 0xa2 P5CNT; */
	
sfr SFR_IE			= 0xA8;	/* Interrupt Enable          */	
	sbit SFRB_EX0   = SFR_IE^0; //0xA8;
	sbit SFRB_ET0   = SFR_IE^1; //0xA9;
	sbit SFRB_EX1   = SFR_IE^2; //0xAA;
	sbit SFRB_ET1   = SFR_IE^3; //0xAB;
	sbit SFRB_ES    = SFR_IE^4; //0xAC;
	sbit SFRB_ES0   = SFR_IE^4; //0xAC;
	sbit SFRB_ET2   = SFR_IE^5; //0xAD;
	sbit SFRB_ES1   = SFR_IE^6; //0xAE;
	sbit SFRB_EA    = SFR_IE^7; //0xAF;

sfr SFR_P3			= 0xB0;	/* Port 3                    */
	sbit P3_0  		= SFR_P3^0; //0xB0;
	sbit P3_1  		= SFR_P3^1; //0xB1;
	sbit P3_2  		= SFR_P3^2; //0xB2;
	sbit P3_3  		= SFR_P3^3; //0xB3;
	sbit P3_4  		= SFR_P3^4; //0xB4;
	sbit P3_5  		= SFR_P3^5; //0xB5;
	sbit P3_6  		= SFR_P3^6; //0xB6;
	sbit P3_7  		= SFR_P3^7; //0xB7;

	sbit SFRB_RXD   = SFR_P3^0; //0xB0;
	sbit SFRB_TXD   = SFR_P3^1; //0xB1;
	sbit SFRB_INT0  = SFR_P3^2; //0xB2;
	sbit SFRB_INT1  = SFR_P3^3; //0xB3;
	sbit SFRB_T0    = SFR_P3^4; //0xB4;
	sbit SFRB_T1    = SFR_P3^5; //0xB5;
	sbit SFRB_WR    = SFR_P3^6; //0xB6;
	sbit SFRB_RD    = SFR_P3^7; //0xB7;

sfr SFR_IP			= 0xB8;	/* Interrupt Priority        */
	sbit SFRB_PX0   = SFR_IP^0; //0xB8;
	sbit SFRB_PT0   = SFR_IP^1; //0xB9;
	sbit SFRB_PX1   = SFR_IP^2; //0xBA;
	sbit SFRB_PT1   = SFR_IP^3; //0xBB;
	sbit SFRB_PS    = SFR_IP^4; //0xBC;
	sbit SFRB_PS0   = SFR_IP^4; //0xBC;
	sbit SFRB_PT2   = SFR_IP^5; //0xBD;
	sbit SFRB_PS1   = SFR_IP^6; //0xBE;

sfr SFR_SCON1		= 0xC0; /* TW88 :: Serial 1 Configuration    */
	sbit SFRB_RI1   = SFR_SCON1^0; //0xc0;
	sbit SFRB_TI1   = SFR_SCON1^1; //0xc1;
	sbit SFRB_RB18  = SFR_SCON1^2; //0xc2;
	sbit SFRB_TB18  = SFR_SCON1^3; //0xc3;
	sbit SFRB_REN1  = SFR_SCON1^4; //0xc4;
	sbit SFRB_SM12  = SFR_SCON1^5; //0xc5;	Multiprocessor
	sbit SFRB_SM11  = SFR_SCON1^6; //0xc6;  SerialMode
	sbit SFRB_SM10  = SFR_SCON1^7; //0xc7; 	SerialMode
	//  SerialMode						BuadRate
	//		0	8bit shift register		Oscillator / 12.
	//		1	8bit UART				Set by timer4. (worked with SMOD1)
	//		2	9bit UART				Oscillator / 64. (worked with SMOD1)
	//		3	9bit UART				Set by Tomer4. (worked with SMOD1)


sfr SFR_SBUF1		= 0xC1;	/* TW88 :: Serial 1 I/O Buffer       */

sfr	SFR_CCL1		= 0xC2; /* Comare/Capture/Reload 1 Low */
sfr	SFR_CCH1		= 0xC3; /* Comare/Capture/Reload 1 High*/
sfr	SFR_CCL2		= 0xC4; /* Comare/Capture/Reload 2 Low */
sfr	SFR_CCH2		= 0xC5;	/* Comare/Capture/Reload 2 High*/
sfr	SFR_CCL3		= 0xC6; /* Comare/Capture/Reload 3 Low */
sfr	SFR_CCH3		= 0xC7;	/* Comare/Capture/Reload 3 High*/

						/* DS80C390:: __sfr __at 0xc4 PMR; 		power managment */
						/* DS80C390:: __sfr __at 0xc6 MCON; 	memory control register */
						/* DS80C390:: __sfr __at 0xc7 TA; 		timed access register */

sfr SFR_T2CON		= 0xC8;	/* TW88 :: Timer 2 Configuration     */
    //		    
    //^0 T2I0  Timer2 Input selection.(only mode1)	
	//^1 T2I1
	//   mode1: Timer function input frequency = fdiv/12(T2PS=0) or fdiv/24(T2PS=1)	
	//^3 T2R0  Timer2 reload mode selection(only mode2)
	//^4 T2R1
	//   mode2: auto-reload upon timer 2 overflow.
	//^7 T2PS		
#if 0				
	//--- T2CON
	//sbit SFRB_CPRL2= SFR_T2CON^0; //0xC8;
	//sbit SFRB_CT2  = SFR_T2CON^1; //0xC9;
	//sbit SFRB_TR2   = SFR_T2CON^2; //0xCA;
	//sbit SFRB_EXEN2 = SFR_T2CON^3; //0xCB;
	//sbit SFRB_TCLK  = SFR_T2CON^4; //0xCC;
	//sbit SFRB_RCLK  = SFR_T2CON^5; //0xCD;
	//sbit SFRB_EXF2  = SFR_T2CON^6; //0xCE;
	//sbit SFRB_TF2   = SFR_T2CON^7; //0xCF;
#endif

sfr SFR_T2IF		= 0xC9;	/* TW88 :: Timer 2 Interrupt Flags   */
	//^0 TF2	Timer2 overflow flag.
	//          Set by a timer2 and must be cleared by software.
	//          TF2=1 causes an interrupt, if the timer2 interrupt is enabled by ET2 bit.
sfr SFR_RCRL		= 0xCA;	/* Timer 2 Reload Low byte   */		//SFR_CRCL
sfr SFR_RLDL		= 0xCA; 
sfr SFR_RCRH		= 0xCB;	/* Timer 2 Reload High byte  */		//SFR_CRCH
sfr SFR_RLDH		= 0xCB; 

sfr SFR_TL2			= 0xCC;	/* Timer 2 Low byte counter  */	
sfr SFR_TH2			= 0xCD;	/* Timer 2 High byte counter */

sfr SFR_CCEN		= 0xCE;	/* Comare/Capture/Reload Enable*/

sfr SFR_PSW			= 0xD0;	/* Program Status Word       */
	sbit SFRB_P     = SFR_PSW^0; //0xD0;
	sbit SFRB_OV    = SFR_PSW^2; //0xD2;
	sbit SFRB_RS0   = SFR_PSW^3; //0xD3;
	sbit SFRB_RS1   = SFR_PSW^4; //0xD4;
	sbit SFRB_F0    = SFR_PSW^5; //0xD5;
	sbit SFRB_AC    = SFR_PSW^6; //0xD6;
	sbit SFRB_CY    = SFR_PSW^7; //0xD7;


sfr SFR_WDCON		= 0xD8;	/* TW88 :: Watchdog configuration    */
	//--- WDCON
	sbit SFRB_RWT   = SFR_WDCON^0;	//0xd8;	Run Watchdog Timer
	sbit SFRB_EWT   = SFR_WDCON^1;	//0xd9;	Enable Watchdog Timer
	sbit SFRB_WTRF  = SFR_WDCON^2;	//0xda;	Watchdog Timer Reset Flag
	sbit SFRB_WDIF  = SFR_WDCON^3;	//0xdb;	Watchdog Interrupt Flag

sfr SFR_ACC			= 0xE0;	/* Accumulator               */
sfr SFR_CAMC		= 0xE2;	/* TW88 :: Chip Access Mode Control. [0]=1b:16bit mode */

sfr SFR_EIE			= 0xE8;	/* TW88 :: External Interrupt Enable. watchdog & 6,5,4,3,2 */
	sbit SFRB_EINT2 = SFR_EIE^0; //0xe8;	Enable EINT2
	sbit SFRB_EINT3 = SFR_EIE^1; //0xe9;	Enable EINT3
	sbit SFRB_EINT4 = SFR_EIE^2; //0xea;	Enable EINT4
	sbit SFRB_EINT5 = SFR_EIE^3; //0xeb;	Enable EINT5
	sbit SFRB_EINT6 = SFR_EIE^4; //0xec;	Enable EINT6
	sbit SFRB_EWDI  = SFR_EIE^5; //0xed;	Enable WatchDog Int


sfr SFR_STATUS		= 0xE9;	/* Status register           */
sfr	SFR_MXAX		= 0xEA;	/* TW88 :: MOVX @Ri High address. */
sfr SFR_TA			= 0xEB;	/* watchdog timed access register */
						
sfr SFR_B			= 0xF0;	/* B Working register        */

sfr SFR_EIP			= 0xF8;	/* TW88 :: External Interrupt Priority. watchdog & 6,5,4,3,2*/
	sbit SFRR_PINT2 = SFR_EIP^0; //0xf8;	Priority EINT2
	sbit SFRR_PINT3 = SFR_EIP^1; //0xf9;	Priority EINT3
	sbit SFRR_PINT4 = SFR_EIP^2; //0xfa;	Priority EINT4
	sbit SFRR_PINT5 = SFR_EIP^3; //0xfb;	Priority EINT5
	sbit SFRR_PINT6 = SFR_EIP^4; //0xfc;	Priority EINT6
	sbit SFRR_PWDI  = SFR_EIP^5; //0xfd;	Priority Watchdog Int


sfr SFR_E2IF		= 0xFA;	/* TW88 :: EINT 14~7 Flag */
sfr SFR_E2IE		= 0xFB;	/* TW88 :: EINT 14~7 Enable */
sfr SFR_E2IP		= 0xFC;	/* TW88 :: EINT 14-7 Priority */
sfr SFR_E2IM		= 0xFD;	/* TW88 :: EINT 14~7 Active Control. 1:Edge Active, 0:Level Active */
sfr SFR_E2IT		= 0xFE;	/* TW88 :: EINT 14~7 Edge/Level Polarity. */
							/*                  0: Low Level,  Falling Edge	*/
							/*                  1: High Level, Rising Edge */

#endif  //__REGISTERS_H__

/* DS80C320 Watchdog Timer from: 8052.com
Watchdog is a feeature to make your application is always running.
If the watchdog determines that your sofrware has crashed, it automacally reboots the 80C320.
The watchdog offers a feature that, instead of rebooting the system, will trigger an interrupt.
This is useful in situations where your software does not need to be completely reset to be 'revived'.

When the watchdog is enabled, the software must wirte a certain SFR on a regular 
bases to let the watchdog know that the program is still running correctly.
If the program does not write within a given perios of time, the watchdog 
assumes the program has crashed and effects a sysmte reboot(or trigger an interrupt).

The watchdog timeout value is set by CKCON.6(WD0) and CKCON.7(WD1).
The following configuration determine how ofter the software must 'kick' the watchdog to avoid a system reboot.
WD1 WD0	instuction cycles for
            interrupt   System Reboot
0   0		32,768		32,896				  32768 =  2**16
0   1		262,144							  40000h = 2**19
1   0		2,097,152						  200000h = 2**22
1   1		16,777,216						  1000000h = 2**25
It note that the watchdog will provoke an interrupt 128 instuction cycles before it initiates a reset.

on TW8836
=========
0 0  	2**11	131072
0 1		2**14	1048576
1 0		2**17	8388608
1 1		2**20	67108864
The Reset, is generated 512 clocks later regardless of the interrupt is used.
*/

