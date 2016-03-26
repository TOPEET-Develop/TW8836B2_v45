/**
 * @file
 * junk.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	junk yard
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
#if 0
/*BKFYI. 
If you want to execute the code at the end of code area, add this dummy code.
It will add a big blank code at the front of code area.
*/
code BYTE dummy_code[1023*5] = {
};
#endif

#if 0 //==============================
#ifdef I2C_ASSEMBLER
static void I2CWriteData(BYTE value)
{
	//BYTE error;

	I2CD = value;

#pragma asm
;----------------
;
;----------------
;	clr EA

	clr	I2C_SCL
	mov	c, I2CD7
	mov I2C_SDA, c
	lcall ddH0	;;;
	setb I2C_SCL
	lcall ddH

	clr	I2C_SCL
	mov	c, I2CD6
	mov I2C_SDA, c
	lcall ddH0	;;;
	setb I2C_SCL
	lcall ddH

	clr	I2C_SCL
	mov	c, I2CD5
	mov I2C_SDA, c
	lcall ddH0	;;;
	setb I2C_SCL
	lcall ddH

	clr	I2C_SCL
	mov	c, I2CD4
	mov I2C_SDA, c
	lcall ddH0	;;;
	setb I2C_SCL
	lcall ddH

	clr	I2C_SCL
	mov	c, I2CD3
	mov I2C_SDA, c
	lcall ddH0	;;;
	setb I2C_SCL
	lcall ddH

	clr	I2C_SCL
	mov	c, I2CD2
	mov I2C_SDA, c
	lcall ddH0	;;;
	setb I2C_SCL
	lcall ddH

	clr	I2C_SCL
	mov	c, I2CD1
	mov I2C_SDA, c
	lcall ddH0	;;;
	setb I2C_SCL
	lcall ddH

	clr	I2C_SCL
	mov	c, I2CD0
	mov I2C_SDA, c
	lcall ddH0	;;;
	setb I2C_SCL
	lcall ddH

;----------------
;
;----------------
;	setb EA

#pragma endasm

	I2C_SCL=0;	ddH();
	I2C_SDA = 1;	//listen for ACK

	I2C_SCL=1;	ddH();
	I2C_SCL=0;
}
#endif //..#ifdef I2C_ASSEMBLER
#ifdef I2C_ASSEMBLER
static BYTE I2CReadData(BYTE fLast)
{
	I2C_SDA = 1;

#pragma asm

;	clr		EA

	clr		I2C_SCL
	lcall	ddH
	setb	I2C_SCL
	lcall	ddH
	mov		c, I2C_SDA
	mov		I2CD7, c

	clr		I2C_SCL
	lcall	ddH
	setb	I2C_SCL
	lcall	ddH
	mov		c, I2C_SDA
	mov		I2CD6, c

	clr		I2C_SCL
	lcall	ddH
	setb	I2C_SCL
	lcall	ddH
	mov		c, I2C_SDA
	mov		I2CD5, c

	clr		I2C_SCL
	lcall	ddH
	setb	I2C_SCL
	lcall	ddH
	mov		c, I2C_SDA
	mov		I2CD4, c

	clr		I2C_SCL
	lcall	ddH
	setb	I2C_SCL
	lcall	ddH
	mov		c, I2C_SDA
	mov		I2CD3, c

	clr		I2C_SCL
	lcall	ddH
	setb	I2C_SCL
	lcall	ddH
	mov		c, I2C_SDA
	mov		I2CD2, c

	clr		I2C_SCL
	lcall	ddH
	setb	I2C_SCL
	lcall	ddH
	mov		c, I2C_SDA
	mov		I2CD1, c

	clr		I2C_SCL
	lcall	ddH
	setb	I2C_SCL
	lcall	ddH
	mov		c, I2C_SDA
	mov		I2CD0, c

;	setb	EA

#pragma endasm

	I2C_SCL=1;	ddH();
	I2C_SCL=0;

	//return I2CD;
}
#endif //..#ifdef I2C_ASSEMBLER
#endif //======================================================


//depaned on timer2 reload value

#if 0
static void timer2_i2c_dd(BYTE delay)
{
	//set value
	timer2_tic = delay;
	//wait
	while(timer2_tic);
}
static void timer2_i2c_onoff(BYTE fOn)
{
	if(fOn)	{
		DisableRemoInt();

		SFR_ET2  = 0;				// Disable Timer2 Interrupt
		timer2_owner = 1;
		SFR_T2IF = 0x00;			// Clear Flag
									// Reload Value
		//SFR_CRCH = SFR_TH2 = 0xff;	// 0xFFFD = 0x10000-0x03 = 0x10000-3. 
		//SFR_CRCL = SFR_TL2 = 0xfd;	// it means 3 usec interval. 333kHz. 
		//SFR_CRCH = SFR_TH2 = 0xff;	// 0xFFEC = 0x10000-0x14 = 0x10000-20. 
		//SFR_CRCL = SFR_TL2 = 0xEC;	// it means 20 usec interval. 50kHz. 
		//SFR_CRCH = SFR_TH2 = 0xff;	// 0xFFF6 = 0x10000-0x0A = 0x10000-10. 
		//SFR_CRCL = SFR_TL2 = 0xF6;	// it means 10 usec interval. 100kHz. 
		SFR_CRCH = SFR_TH2 = 0xff;	// 0xFFFF = 0x10000-1. 
		SFR_CRCL = SFR_TL2 = 0xFF;	// it means 1 usec interval. 1000kHz. 
	
		SFR_T2CON = 0x12;			// 0001 0010 
									// |||| |||+-- T2I0 \ Timer2 Input Selection 
									// ||||	||+--- T2I1 / 00=No,  01=Timer,  10=Counter, 11=Gate
									// ||||	|+---- T2CM:  Compare mode
									// ||||	+----- T2R0 \ Timer2 Reload Mode 
									// |||+------- T2R1	/ 00=No,  01=No,     10=Auto,    11=pin T2EX
									// ||+-------- ---
									// |+--------- I3FR: Timer2 Compare0 Interrupt Edge...
									// +---------- T2PS: Timer2 Prescaler
	
		SFR_ET2  = 1;				// Enable Timer 2 Interrupt
	}
	else {
		SFR_ET2  = 0;				// Disable Timer2 Interrupt
		timer2_owner = 0;
		//give it back to remocon.
		EnableRemoInt();
	}
}
#endif


//===================JUNK CODE============================

#ifdef UNCALLED_CODE
void HDMIPowerDown(void)
{
	ReadI2CByte(I2CID_EP9351,EP9351_General_Control_0, ReadI2C(I2CID_EP9351,EP9351_General_Control_0) |0x04);
	delay1ms(500);
}
#endif

// call from CheckAndSetHDMI
//BKTODO:Remove it.
#if 0
BYTE CheckAndSet_EP9351(void)
{
	volatile BYTE vTemp;
	BYTE TempByte[20];
	BYTE i;

#if 0
	//Hot Boot needs Soft Reset.  TODO:Need Verify. I don't know why it need.
	bTemp = ReadI2C(I2CID_EP9351, EP9351_Status_Register_0 );
	if ( (bTemp & 0x1C) != 0x1C ) {
		Printf("\n\rEP9351 $%bx read:%bx. Do SWReset",EP9351_Status_Register_0,bTemp);
		bTemp = ReadI2C(I2CID_EP9351,EP9351_General_Control_0);
		bTemp |= EP9351_General_Control_0__PWR_DWN;
		ReadI2CByte(I2CID_EP9351,EP9351_General_Control_0, bTemp);		 		// set to 0x40, Soft reset
		delay1ms(500);
		bTemp &= ~EP9351_General_Control_0__PWR_DWN;
		ReadI2CByte(I2CID_EP9351,EP9351_General_Control_0, bTemp);				// set to NORMAL
		Printf("=>read:%bx",ReadI2C(I2CID_EP9351, EP9351_Status_Register_0 ));
	}
#endif
	ReadI2CByte(I2CID_EP9351, EP9351_General_Control_1, ReadI2C(I2CID_EP9351,EP9351_General_Control_1) );		// make Positive polarity always
	ReadI2CByte(I2CID_EP9351, EP9351_General_Control_9, 0x01);			   	// enable EQ_GAIN

	for(i=0; i < 100; i++) {
		delay1ms(10);
		vTemp = ReadI2C(I2CID_EP9351,EP9351_Status_Register_0);
		if(vTemp)
			Printf("\n\r%bd:check status : %bx",i,vTemp);

		vTemp = ReadI2C(I2CID_EP9351,EP9351_HDMI_INT);
		if((vTemp & EP9351_HDMI_INT__AVI)) {
			//Printf("\n\rcheck INT end : %bd",i);
			break;
		}
	}
	if(i==100) {
		Printf("\n\rCheckAndSet_EP9351 FAIL");
		return ERR_FAIL;	//NO AVI_F
	}


	Printf("\n\rInit_HDMI read %bx:%bx @%bd",EP9351_HDMI_INT,vTemp,i);

	//read AVI InfoFrame at 0x2A
	ReadI2C(I2CID_EP9351, EP9351_AVI_InfoFrame, TempByte, 15);
	DBG_PrintAviInfoFrame();

	//color convert to RGB
	Puts("\n\rInput HDMI format ");
	i = (TempByte[2] & 0x60) >> 5;
	if(i == 0) 	{
		Puts("RGB");
		ReadI2CByte(I2CID_EP9351, EP9351_General_Control_2, 0x00 ); //0x42
	}
	else if (i==1)  	{
		Puts("YUV(422)");
		ReadI2CByte(I2CID_EP9351, EP9351_General_Control_2, 0x50 );
	}
	else if (i==2)  	{
		Puts("YUV(444)");
		ReadI2CByte(I2CID_EP9351, EP9351_General_Control_2, 0x10 );
	}

	//dump control register
	Puts("\n\rEP9351 General Control:");
	for(i=0; i < 10; i++) {
		Printf("%2bx ",ReadI2C(I2CID_EP9351,0x40+i));
	}

	ReadI2CByte(I2CID_EP9351,EP9351_General_Control_9, 0x09);			   	// enable EQ_GAIN. use 40uA PUMP.
	Printf( "=>%02bx", ReadI2C(I2CID_EP9351,EP9351_General_Control_9) );


	return 0;
}
#endif

#if 0
BYTE Init_HDMI_EP9X53__OLD(void)
{	
	BYTE i, cVal[15];
	volatile BYTE value;
	BYTE j;


#if 1	//Hot Boot need it. BK120319
	i = ReadI2C(I2CID_EP9351, EP9351_Status_Register_0 );
//	if ( (i & 0x1C) != 0x1C ) {
		ReadI2CByte(I2CID_EP9351,EP9351_General_Control_0, EP9351_General_Control_0__PWR_DWN);		 		// set to 0x40, Soft reset
		delay1ms(500);
		ReadI2CByte(I2CID_EP9351,EP9351_General_Control_0, 0x00);				// set to NORMAL
//	}
	delay1ms(500);

	//while ( ReadI2C(I2CID_EP9351,EP9351_General_Control_0) ) ;			// wait till correct val
	ReadI2CByte(I2CID_EP9351, EP9351_General_Control_1, ReadI2C(I2CID_EP9351,EP9351_General_Control_1) );		// make Positive polarity always
	ReadI2CByte(I2CID_EP9351, EP9351_General_Control_9, 0x01);			   	// enable EQ_GAIN

	cVal[13] = ReadI2C(I2CID_EP9351,EP9351_HDMI_INT);	  				//$29 interrupt flag
	cVal[14] = ReadI2C(I2CID_EP9351,EP9351_Status_Register_0);	  		//$3C	status0
	ReadI2C(I2CID_EP9351, EP9351_Timing_Registers, cVal, 13);		//$3B
	DBG_PrintTimingRegister();	//HDMI_DumpTimingRegister(cVal);
	Printf("\n\rR29:%bx,R3C:%bx",cVal[13],cVal[14]);
#endif
	for(i=0; i < 100; i++) {
		delay1ms(10);
		value = ReadI2C(I2CID_EP9351,EP9351_Status_Register_0);
		if(value)
			Printf("\n\r%bd:check status : %bx",i,value);

		value = ReadI2C(I2CID_EP9351,EP9351_HDMI_INT);
		if((value & EP9351_HDMI_INT__AVI)) {
			//Printf("\n\rcheck INT end : %bd",i);
			break;
		}
	}
	if(i==100) {
		return ERR_FAIL;	//NO AVI_F
	}

	Printf("\n\rInit_HDMI read %bx:%bx @%bd",EP9351_HDMI_INT,value,i);

	//read AVI InfoFrame at 0x2A
	ReadI2C(I2CID_EP9351, EP9351_AVI_InfoFrame, cVal, 15);
	DBG_PrintAviInfoFrame();

	//color convert to RGB
	Puts("\n\rInput HDMI format ");
	i = (cVal[2] & 0x60) >> 5;
	if(i == 0) 	{
		Puts("RGB");
		ReadI2CByte(I2CID_EP9351, EP9351_General_Control_2, 0x00 ); //0x42
	}
	else if (i==1)  	{
		Puts("YUV(422)");
		ReadI2CByte(I2CID_EP9351, EP9351_General_Control_2, 0x50 );
	}
	else if (i==2)  	{
		Puts("YUV(444)");
		ReadI2CByte(I2CID_EP9351, EP9351_General_Control_2, 0x10 );
	}

	//dump control register
	Puts("\n\rEP9351 General Control:");
	for(i=0; i < 10; i++) {
		Printf("%2bx ",ReadI2C(I2CID_EP9351,0x40+i));
	}

	ReadI2CByte(I2CID_EP9351,EP9351_General_Control_9, 0x09);			   	// enable EQ_GAIN. use 40uA PUMP.
	Printf( "=>%02bx", ReadI2C(I2CID_EP9351,EP9351_General_Control_9) );

	return ERR_SUCCESS;
}
#endif

//----------------------------------------------------------------------------------------------------------------------
#if 0
void EP9351_Interrupt()
{
	// Interrupt Flags Register
	Event_HDMI_Info = 1;	
}
#endif

#if 0
BYTE EP9351_Task(void)
{
	status0 = ReadI2C(I2CID_EP9351, EP9351_Status_Register_0);
	status1 = ReadI2C(I2CID_EP9351, EP9351_Status_Register_1);

	if((status1 & 0xC0 == 0xC0) && !(is_powerdown)) {
		if(!is_Valid_Signal) {
			//No Signal -> Signal Valid
			is_Valid_Signal = 1;

			//VideoMuteDisable

			if(status0 & EP9351_Status_Register_0__HDMI) {
			}
			else {
			}

		}
	}
	else {
		if(is_Valid_Signal) {
			is_Valid_Signal = 0;
		}
	}

	if(is_NoSignal_Reset) {
	}

	if(is_HDMI) {
		ReadInterruptFlags();
	}

}
#endif


//-----------------------------------------------------------------------------
//desc
//parameter
//	number of bytes
//	read bytes
//	register
//	write cmd data
//return
#if 0 //BK111118
BYTE SPI_cmd_protocol(BYTE max, ...)
{
	va_list ap;
	BYTE page;
	BYTE temp;
	BYTE r_cnt, w_cnt, i;
	BYTE w_cmd[5];
	BYTE ret;

	//-------------------
	ret=0xff;
	if(max < 2)
		return 0xff;
	
	va_start(ap, max);

	w_cnt = max-1;
	r_cnt = va_arg(ap, BYTE);		//r_cnt
	for(i=0; i < w_cnt; i++) {
		w_cmd[i]=va_arg(ap, BYTE);		//reg
	}
	Printf("\n\rSPICMD[%bd] r:%bd w:%bd reg:%bx",max, r_cnt, w_cnt-1, w_cmd[0]);

	ReadTW88Page(page);		//save
	WriteTW88Page(PAGE4_SPI );

	WriteTW88(REG4C3, 0x40+w_cnt);
	Write2TW88(REG4C6,REG4C7, 0x04d0);			// DMA Page & Index. indecate DMA read/write Buffer at 0x04D0.
	WriteTW88(REG4DA,0 );					// DMA Length high
	Write2TW88(REG4C8,REG4C9, r_cnt);			// DMA Length middle & low

	//write 
	for(i=0; i < w_cnt; i++)
		WriteTW88(REG4CA+i, w_cmd[i] );		// write cmd1
	if(r_cnt) {
		WriteTW88(REG4C4, 0x01 );				// start
		//delay1ms(2);
		for(i=0; i < 100; i++)
			_nop_();

	}
	else {	
		WriteTW88(REG4C4, 0x07 );				// start, with write, with busycheck
	}

	//read
	if(r_cnt)	Puts("\tREAD:");	
	for(i=0; i < r_cnt; i++) {
		temp = ReadTW88(REG4D0+i );
		Printf("%02bx ",temp);
		if(i==0)
			ret = temp;
	}							

	WriteTW88Page(page );	//restore
	//-------------------
	va_end(ap);

	return ret;
}
#endif
#if 0 //test CRC8

//#define GP  0x107   /* x^8 + x^2 + x + 1 CRC-8-CCITT */
//#define DI  0x07

//#define GP  0x131   /* x^8 + x^5 + x^4 + 1 CRC-8 Dallas/Maxim */
//#define DI  0x31

#define GP  0x1D5   /* x^8 + x^7 + x^6 + x^4 + x^2 + 1 CRC-8 */
#define DI  0xD5

//#define GP  0x19B   /* x^8 + x^7 + x^4 + x^3 + x + 1 CRC-8-WCDMA */
//#define DI  0x9B


static unsigned char crc8_table[256];     /* 8-bit table */
/*
* Should be called before any other crc function.  
*/
static void init_crc8()
{
	int i,j;
	unsigned char crc;
	
	for (i=0; i<256; i++) {
		crc = i;
		for (j=0; j<8; j++)
			crc = (crc << 1) ^ ((crc & 0x80) ? DI : 0);
		crc8_table[i] = crc & 0xFF;
	}
	//for(i=0; i < 16; i++) {
	//	Printf("\n\r%x:",i);
	//	for(j=0; j <16; j++) {
	//		Printf(" %02bx", crc8_table[i*16+j]); 
	//	}
	//}
	//Puts("\n\r");

}

void crc8(unsigned char *crc, unsigned char m)
     /*
      * For a byte array whose accumulated crc value is stored in *crc, computes
      * resultant crc obtained by appending m to the byte array
      */
{
	*crc = crc8_table[(*crc) ^ m];
	*crc &= 0xFF;
}


void TestCrC8(void)
{
	BYTE crc;

	init_crc8();
	crc = 0;
	crc8(&crc, 0xF6	);
	crc8(&crc, 0x8B	);
	crc8(&crc, 0x3D	);
	crc8(&crc, 0x11	);
	crc8(&crc, 0x5D	);
	crc8(&crc, 0xB6	);
	crc8(&crc, 0x7B	);

	Printf("\n\rCRC8 %bx",crc);
}
#endif //test CRC8

//=============================================================================
//                                                                           
//=============================================================================
/*
#ifdef DEBUG
BYTE Getch(void)
{
	while(!RS_ready());
	return RS_rx();
}
#endif
*/
/*
BYTE Getche(void)
{
	BYTE ch;

	while(!RS_ready());
	ch = RS_rx();
	RS_tx(ch);

	return ch;
}
*/
#if 0
	Puts("\n\rINTR ");
	if(SFR_EX0)			Puts(" 0:ext0");
	if(SFR_ET0)			Puts(" 1:timer0");
	if(SFR_EX1)			Puts(" 2:ext1");
	if(SFR_ET1)			Puts(" 3:timer1");
	if(SFR_ES)			Puts(" 4:uart0");

	if(SFR_ET2)			Puts(" 5:timer2");
	if(SFR_ES1)			Puts(" 6:uart1");

	if(SFR_EINT2)		Puts(" 7:ext2");
	if(SFR_EINT3)		Puts(" 8:ext3");
	if(SFR_EINT4)		Puts(" 9:ext4");
	if(SFR_EINT5)		Puts(" 10:ext5");
	if(SFR_EINT6)		Puts(" 11:ext6");
	if(SFR_EWDI)		Puts(" 12:watchdog");
	if(SFR_E2IE & 0x01)	Puts(" 13:ext7");
	if(SFR_E2IE & 0x02)	Puts(" 14:ext8");
	if(SFR_E2IE & 0x04)	Puts(" 15:ext9");
	if(SFR_E2IE & 0x08)	Puts(" 16:ext10");
	if(SFR_E2IE & 0x10)	Puts(" 17:ext11");
	if(SFR_E2IE & 0x20)	Puts(" 18:ext12");
	if(SFR_E2IE & 0x40)	Puts(" 19:ext13");
	if(SFR_E2IE & 0x80)	Puts(" 20:ext14");
#endif


#if 0 //#ifdef MODEL_TW8835_EXTI2C
	//base 27MHz MCU clock.	1 clk cycle: 37nSec
	//SCLK:367kHz. almost 400kHz. one I2C read use 196uS.
	#define I2CDelay_1		_nop_(); _nop_()
	#define I2CDelay_3		dd(1*5)			//
	#define I2CDelay_4		dd(1*5)			//need 100
	#define I2CDelay_5		_nop_(); _nop_()
	#define I2CDelay_6		_nop_(); _nop_()
	#define I2CDelay_7		dd(1*5)			//need 100
	#define I2CDelay_8		dd(2*5)			//need 200
	#define I2CDelay_9		_nop_()
	#define I2CDelay_ACK	dd(1*5)
#endif
#if 0
	//base 72MHz MCU clock.	1 Clk cycle: 13.89nS
	//SCLK:???kHz. one I2C read use ???uS.
#define I2CDelay_1		N_O_P_20 	//6
#define I2CDelay_3		N_O_P_100	//??
#define I2CDelay_4		N_O_P_50	//27
#define I2CDelay_5		N_O_P_10 	//4		  5:NG
#define I2CDelay_6		N_O_P_20 	//6
#define I2CDelay_7		N_O_P_50	//27
#define I2CDelay_8		N_O_P_100	//53
#define I2CDelay_9		N_O_P_5 //_nop_()
#define I2CDelay_ACK	N_O_P_100
#endif
#if 0
	//base 72MHz MCU clock.	1 Clk cycle: 13.88nS
	//SCLK:???kHz. one I2C read use ???uS.
#define I2CDelay_1		dd(1)
#define I2CDelay_2		dd(10)
#define I2CDelay_3		dd(5)
#define I2CDelay_4		dd(5)
#define I2CDelay_5		dd(5)
#define I2CDelay_6		
#define I2CDelay_7		dd(3)
#define I2CDelay_8		dd(10)
#define I2CDelay_9		dd(10)
#define I2CDelay_ACK	dd(10)
#endif

//=============================================================================
// REMOVED
//=============================================================================
#if 0
//new 110909
void LoDecoderMode(BYTE mode)
{
	BYTE value;
	WriteTW88Page(PAGE1_DECODER);
	value = ReadTW88(REG102) & 0xCF;
	value |= (mode << 4);
	WriteTW88(REG102, value);
}

void InMuxPowerDown(BYTE Y, BYTE C, BYTE V)
{
	BYTE value;
	WriteTW88Page(PAGE1_DECODER);
	value = ReadTW88(REG106) & 0xF8;
	if(Y)	value |= 0x04;
	if(C)	value |= 0x02;
	if(V)	value |= 0x01;
	WriteTW88(REG106,value);		
}
//assume R102[6]=1;
void InMuxInput(BYTE Y, BYTE C, BYTE V)
{
	BYTE value;
	WriteTW88Page(PAGE1_DECODER);
	value = ReadTW88(REG102) & ~0x8F;
	value |= (Y << 2);
	if(C >= 2)		value |= 0x80;
	if(C & 0x01) 	value |= 0x02;
	value |= V;
	WriteTW88(REG102,value);
	
}
void AFESelectDecoderAndClock(BYTE fLoDecoder)
{
	BYTE r105,r1c0;

	WriteTW88Page(PAGE1_DECODER);
	r105 = ReadTW88(REG105) & 0xFE;
	r1c0 = ReadTW88(REG1C0) & 0xFE;
	if(fLoDecoder) {
		r105 |= 0x01;	//set decoder
		r1c0 |= 0x01;	//decoder use 27MHz
	}
	WriteTW88(REG105,r105);
	WriteTW88(REG1C0,r1c0);
	 
}

//	InputPort		PowerDown	AntiAliasingFilter
//	----			----		----
//Y	R102[3:2]		R106[2]		R105[3]
//C	R102[7]R102[1]	R106[1]		R105[2]
//V	R102[0]			R106[0]		R105[1]
//
// input		InputPort	PowerDown	AFE Path	ADC Clock	PGA select
// -----		---------	---------	----		---- 		----	
// CVBS			Y0			C&V			LoDecoder	27M			LowSpeed
// SVIDEO		Y1,C0		V			LoDecoder	27M			LowSpeed
// Component	Y2,C1,V0				HiDecoder	LLPLL		HighSpeed
// aPC			Y2,C1,V0				HiDecoder	LLPLL		HighSpeed
void AFESetInMuxInput_____TEST(BYTE InputMode)
{
	BYTE use_highspeed_path;

	use_highspeed_path = 0;	
	switch(InputMode) {
	case INPUT_CVBS:
		InMuxInput(0,0,0);
		InMuxPowerDown(0,1,1);
		LoDecoderMode(0);
		break;
	case INPUT_SVIDEO:
		InMuxInput(1,0,0);
		InMuxPowerDown(0,0,1);
		LoDecoderMode(1);
		break;
	case INPUT_COMP:
		InMuxInput(2,1,0);
		InMuxPowerDown(0,0,0);
		LoDecoderMode(0);
		use_highspeed_path = 1;
		break;
	case INPUT_PC:
		InMuxInput(2,1,0);
		InMuxPowerDown(0,0,0);
		LoDecoderMode(0);
		use_highspeed_path = 1;
		break;
	case INPUT_DVI:
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
		InMuxInput(0,0,0);
		InMuxPowerDown(0,0,0);
		LoDecoderMode(0);
		break;
	case INPUT_BT656:
		InMuxInput(0,0,0);
		InMuxPowerDown(0,1,1);
		LoDecoderMode(0);
		break;
	}
	AFESelectDecoderAndClock(!use_highspeed_path);
}
#endif
#if 0
void AFESetInMuxYOUT()
{}
void AFEClampStartEnd(BYTE start, BYTE end)
{}
void AFEClampPosition(BYTE position)
{}
void AFEClampSetMode(BYTE mode)
{}
#endif

//===========================================
// TW9900
//===========================================
//---------------------------------------------
//		BYTE	CheckTW9900VDLOSS( BYTE n )
//---------------------------------------------
#ifdef USE_TW9900
BYTE	CheckTW9900VDLOSS( BYTE n )
{..}

//---------------------------------------------
//		BYTE	CheckTW9900STD( BYTE n )
//---------------------------------------------
BYTE	CheckTW9900STD( BYTE n )
{..}
#endif

//====================================
// VADC.C
//====================================
//The unit is sysclk
//480i(SOY) = 140 (92~213)
//576i(SOY) = 140 (92~213)
//480p(SOY) = 52  (30~83)
//576p(SOY) = 58  (30~83)
//1080i(SOY)= 24  (14~44)
//720p(SOY) = 38  (18~67)
//H/Vsync = 0
#if 0
//								 720    720    720    720 	1920   1280   1920
//							     480i,  576i,  480p, 576p,  1080i, 720p,  1080p
code	BYTE	ClampPos[]   = { 140,   140,   52,    58,    24,    38,    24 };
#endif



//for bank issue
#if 0
BYTE GetInput_aRGBMode(void)
{...}
#endif

#if 0
//!void VAdcAdjustPhase(BYTE mode)
//!{
//!	//WriteTW88Page(PAGE1_VADC );
//!	if(Input_aRGBMode >= EE_YUVDATA_START) {
//!	}
//!	else {
//!		if(mode==10) VAdcSetPhase(3, 0);	//WriteTW88(REG1C5, 3); 
//!		if(mode==18) VAdcSetPhase(17, 0);	 
//!	}
//!}
#endif

//!#if !defined(SUPPORT_COMPONENT) && !defined(SUPPORT_PC)
//!//----------------------------
//!//Trick for Bank Code Segment
//!//----------------------------
//CODE BYTE DUMMY_VADC_CODE;
//void Dummy_VADC_func(void)
//{
//	BYTE temp;
//	temp = DUMMY_VADC_CODE;
//}
//!#else //..!defined(SUPPORT_COMPONENT) && !defined(SUPPORT_PC)
//! other real code
//!#endif


							//       1      2       3      4      5      6      7      8      9     10
							//   	480i,  576i,   480p, 576p,1080i50,1080i60,720p50,720p60,1080p5,1080p6
#if 0
//scaled
code	WORD	YUVDividerPLL[] = { 858,   864,   858,   864,   2460,  2200,  1980,  1650,  2640,  2200 };
code	WORD	YUVVtotal[]     = { 262,   312,   525,   625,   562,   562,   750,   750,   1124,  1124 };

code	BYTE	YUVClampPos[]   = { 128,   128,   64,    58,    40,    32,    38,    38,    14,    14 };		// 0x1D7

code	WORD	YUVCropH[]      = { 720,   720,   720,   720,   1920,  1920,  1280,  1280,  1920,  1920 };
code	WORD	YUVCropV[]      = { 240,   288,   480,   576,   540,   540,   720,   720,   1080,  1080 };
code	WORD	YUVDisplayH[]   = { 700,   700,   700,   700,   1880,  1880,  1260,  1260,  1880,  1880 };		// 0x042[3:0],0x046
code	WORD	YUVDisplayV[]   = { 230,   278,   460,   556,   520,   520,   696,   696,   1040,  1040 };		// 0x042[6:4],0x044

code	WORD	YUVStartH[]     = { 112,   126,   114,   123,   230,   233,   293,   293,   233,   233 };		// 0x040[7:6],0x045 InputCrop
code	WORD	YUVStartV[]     = { 1,     1,     2,     2,     2,     2,     2,     2,     2,     2 };			// 0x043 InputCrop
code	BYTE	YUVOffsetH[]    = { 5,     4,     10,    6,     40,    40,    20,    20,    30,    30 };
code	BYTE	YUVOffsetV[]    = { 48,    48,    48,    48,    28,    26,    24,    25,    26,    26 };		// use as V-DE 0x215	
code	BYTE	YUVScaleVoff[]  = { 128,   128,   0,     0,     128,   128,   0,     0,     0,     0 };

code	WORD	MYStartH[]      = { 121,   131,   121,   131,   
	235-44,   
	235-44,   
	299-40,   
	299-40,   
	235-44,   
	235-44 };		// 0x040[7:6],0x045 InputCrop
code	WORD	MYStartV[]      = { 19,   21,   38,   44,   
	20,	20,   
	25, 25,   
	41, 41 };	

#endif //.#else
#if 0
							//   	480i,  576i,   480p, 576p,1080i50,1080i60,720p50,720p60,1080p5,1080p6
//total scan pixel & line
code	WORD	YUVDividerPLL[] = { 858,   864,   858,   864,   2460,  2200,  1980,  1650,  2640,  2200 };		//total horizontal pixels
code	WORD	YUVVtotal[]     = { 262,   312,   525,   625,   562,   562,   750,   750,   1124,  1124 };		//total vertical scan line

code	BYTE	YUVClampPos[]   = { 140,   140,   52,    58,    24,    32,    38,    38,    14,    14 };		//R1D7. clamp position offset.

//reduced resolution.
code	WORD	YUVDisplayH[]   = { 720,   720,   720,   720,   1920,  1920,  1280,  1280,  1920,  1920 };		// R042[3:0]R046[7:0] for overscan
code	WORD	YUVDisplayV[]   = { 240,   288,   480,   576,   540,   540,   720,   720,   1080,  1080 };		// R042[6:4]R044[7:0] for overscan

//resolution
code	WORD	YUVCropH[]      = { 720,   720,   720,   720,   1920,  1920,  1280,  1280,  1920,  1920 };		// horizontal resolution
code	WORD	YUVCropV[]      = { 240,   288,   480,   576,   540,   540,   720,   720,   1080,  1080 };		// vertical resolution

code	WORD	YUVStartH[]     = { 121-16,131-16,121-16,131-16,235-16,235-16,299-16,299-16,   235-16,235-16 };		// 0x040[7:6],0x045 InputCrop
code	WORD	YUVStartV[]     = { 1,     1,     2,     2,     2,     2,     2,     2,     2,     2 };			// 0x043 InputCrop

code	BYTE	YUVOffsetH[]    = { 5,     4,     10,    6,     40,    40,    20,    20,    30,    30 };
code	BYTE	YUVOffsetV[]    = { 42,    40,    40,    38,    20,    20,    18,    18,    10,    10 };		// use as V-DE 0x215	
//=>VDE value
							//   	480i,  576i,   480p, 576p,1080i50,1080i60,720p50,720p60,1080p5,1080p6
code	BYTE  YUV_VDE_NOSCALE[] = { 21,    24,    40,    46,    22,    22,    27,    27,    22,    22 };		// use as V-DE 0x215	



code	BYTE	YUVScaleVoff[]  = { 128,   128,   0,     0,     128,   128,   0,     0,     0,     0 };


code	WORD	MYStartH[]      = { 121,   131,   121,   131,   
	235-44,   
	235-44,   
	299-40,   
	299-40,   
	235-44,   
	235-44 };		// 0x040[7:6],0x045 InputCrop
code	WORD	MYStartV[]      = { 19,   21,   38,   44,   
	20,	20,   
	25, 25,   
	41, 41 };	
#endif

//=============================================================================
// REMOVED
//=============================================================================

//void SPI_cmd_protocol(BYTE max, ...)
#if 0
BYTE SPI_QUADInit_Test(void)
{
	BYTE ret;
	BYTE temp;
	ret = SFLASH_VENDOR_MICRON;

	SPI_cmd_protocol(2,	3, 0x9f);
	SPI_cmd_protocol(2,	1,5);
	SPI_cmd_protocol(5,	8,3,0,0,0);	 //BUGBUG......

	temp=SPI_cmd_protocol(2,	1,0x85);
	if((temp&0xF0)==0x60)
		Puts("\n\rOK.6 dummy clock @V");
	else
		Puts("\n\rFAIL.6 dummy clock @V");

	temp=SPI_cmd_protocol(2,	1,0x65);
	if(temp&0x80)
		Puts("\n\rFAIL.QuadIO@VE");
	else
		Puts("\n\rOK.QuadIO@VE");

	SPI_cmd_protocol(2,	2,0xB5);

	WriteTW88Page(PAGE4_SPI);
	temp = ReadTW88(REG4D0);
	if((temp&0xF0)==0x60)
		Puts("\n\rOK.6 dummy clock @NV");

 	temp = ReadTW88(REG4D1);
	if(temp&0x04)
		Puts("\n\rFAIL. QuadIO@NV");
	else
		Puts("\n\rOK.QuadIO@NV");
#if 0
	temp = SPI_cmd_protocol(2,	1,0xB5);			//read NV cof reg
	SPI_cmd_protocol(2,	0,6);						//write enable
	SPI_cmd_protocol(3,	0,0xB1, temp & ~0x08);		//update NV cof reg with Quid Input
	SPI_cmd_protocol(2,	1,0xB5);					//read NV cof reg
#endif

#if 0
	temp = SPI_cmd_protocol(2,	1,0x85);			//read V cof reg
	SPI_cmd_protocol(2,	0,6);						//write enable
	SPI_cmd_protocol(3,	0,0x81, temp & ~0x08);		//update V cof reg with Quid Input
	SPI_cmd_protocol(2,	1,0x85);					//read V cof reg
#endif

#if 0
	temp = SPI_cmd_protocol(2,	1,0x65);			//read VE cof reg
	SPI_cmd_protocol(2,	0,6);						//write enable
	SPI_cmd_protocol(2,	0,0x61, temp & ~0x80);		//update VE cof reg with Quid Input
	SPI_cmd_protocol(2,	1,0x65);					//read VE cof reg
#endif
	return 0;
}
#endif

//=============================================================================
//=============================================================================
// REMOVED
//=============================================================================
//=============================================================================
#if 0 //BK111013
//BKTODO111013 I don't know why it use a fixed value ????
void ScalerSetDeOnFreerun(void)
{
	BYTE fOn=OFF;
	BYTE HDE_value = 40;
	BYTE VDE_value = 48;
	WORD VTotal;

	switch(InputMain) {
	case INPUT_CVBS:
	case INPUT_SVIDEO:
		break;
	case INPUT_COMP:
	//V-DE:0
//!		wTemp = ScalerCalcHDE();
//!		ScalerWriteHDEReg(wTemp, PANEL_H);
//!		wTemp = FPGA_GetVDE();
//!		if(wTemp < 1)
//!			wTemp = 1;
//!		ScalerSetVDEPosHeight(wTemp,  PANEL_V);
		HDE_value = 40;
		VDE_value = 48;
		HDE_value=ScalerReadHDEReg();
		VDE_value=ScalerReadVDEReg();
		fOn = ON;
		break;
	case INPUT_PC:
	//V-DE:0
//!		wTemp = ScalerCalcHDE();
//!		ScalerWriteHDEReg(wTemp, PANEL_H);
//!		//----------------------
//!		//wTemp = FPGA_GetVDE();
//!		//if(wTemp<1)	 //temp
//!		//	wTemp=1;
//!		//----------------------
//!		wTemp = ScalerCalcVDE();
//!		ScalerSetVDEPosHeight(wTemp,  PANEL_V);
		HDE_value = 40;
		VDE_value = 48;
		fOn = ON;
		break;
	case INPUT_DVI:
//!		wTemp = ScalerCalcHDE();
//!		ScalerWriteHDEReg(wTemp, PANEL_H);
//!		wTemp = FPGA_GetVDE();
//!		ScalerSetVDEPosHeight(wTemp,  PANEL_V);
		HDE_value = 40;
		VDE_value = 48;
		fOn = ON;
		break;
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
//!		wTemp = ScalerCalcHDE();
//!		ScalerWriteHDEReg(wTemp, PANEL_H);
//!		wTemp = FPGA_GetVDE();
//!	wTemp = 35;
//!		ScalerSetVDEPosHeight(wTemp,  PANEL_V);
		HDE_value = 40;
		VDE_value = 48;
		fOn = ON;
		break;
	case INPUT_BT656:
//!		wTemp = ScalerCalcHDE();
//!		ScalerWriteHDEReg(wTemp, PANEL_H);
//!		wTemp = FPGA_GetVDE();
//!		if(wTemp < 3)
//!			wTemp = 2;
//!		ScalerSetVDEPosHeight(wTemp,  PANEL_V);
		HDE_value = 40;
		VDE_value = 48;
		fOn = ON;
		break;
	default:
		break;
	}

	dPrintf("\n\rScalerSetDeOnFreerun HDE:%bd VDE:%bd",	ScalerReadHDEReg(),ScalerReadVDEReg());



	if(fOn) {
	
		VTotal = ScalerReadFreerunVtotal();
		if(VDE_value >= (VTotal - PANEL_V)) {
			VDE_value = VTotal - PANEL_V -1;
		}
		WaitVBlank(1); 	//InitLogo1 need it

		ScalerWriteHDEReg(HDE_value);
		ScalerWriteVDEReg(VDE_value);
		SpiOsdSetDeValue();	//InitLogo1 need it
	}
}
#endif


#if 0
//use x100 value for floating point
void ScalerSetHScale100(WORD Length)	
{
	DWORD	temp;

	WriteTW88Page(PAGE2_SCALER);

	if(PANEL_H >= (Length/100)) { 					
		//UP SCALE
		temp = (DWORD)Length * 0x2000L;
		temp /= 100;
		temp /= PANEL_H;
		ScalerWriteXUpReg(temp);				//set up scale
		ScalerWriteXDownReg(0x0400);			//clear down scale
		dPrintf("\n\rScalerSetHScale100(%d) UP:0x0400 DN:0x%04lx",Length, temp);
	}
	else {										
		//DOWN SCALE
		temp = (DWORD)Length * 0x0400L;						
		temp /= 100;
		temp /= PANEL_H;
		ScalerWriteXUpReg(0x2000);			//clear up scale
		ScalerWriteXDownReg(temp);			//set down scale
		dPrintf("\n\rScalerSetHScale100(%d) UP:0x%04lx DN:0x2000",Length, temp);
	}
}
#endif

#ifdef UNCALLED_SEGMENT
WORD GetHScaledRatio(WORD length)
{...}
#endif

#if 0
//use x100 for floating point
void ScalerSetVScale100(WORD Length)
{
	DWORD	temp;

	WriteTW88Page(PAGE2_SCALER);

	temp = Length * 0x2000L;
	temp += (PANEL_V / 2);
	temp /= PANEL_V;
	temp /= 100;
	dPrintf("\n\rScalerSetVScale(%d) 0x%04lx",Length, temp);

	ScalerWriteVScaleReg(temp);
}
#endif

//BKTODO: It comes from TW8823.
//	use ScalerSetVScale() & ScalerSetVDEPosHeight() with GetVScaledRatio()
//offset for V back porch
#ifdef UNCALLED_SEGMENT
void ScalerSetVScaleWithOffset(WORD Length, BYTE offset)
{
	DWORD	temp;

	WriteTW88Page(PAGE2_SCALER);

	temp = Length * 0x2000L;
	temp /= PANEL_V;
	temp += offset;

	dPrintf("\n\rScalerSetVScale(%d,%bd) 0x%04lx",Length, offset, temp);

	ScalerWriteVScaleReg(temp);
}
#endif

#if 0
WORD GetVScaledRatio(WORD length)
{
	DWORD dTemp;
	WORD wTemp;
	WORD wResult;
	WORD wTest;

	dPrintf("\n\rGetVScaledRatio(%d)",length);

	dTemp = length * 0x2000L;
	wTemp = ScalerReadVScaleReg();
	dPrintf(" ratio:	8192/%d",wTemp);

	wResult = (WORD)(dTemp / wTemp);
	dTemp += 0x1000L; //add (0x2000/2)  ..roundup..
	wTest = (WORD)(dTemp / wTemp);

	dPrintf(" result:%d test:%d",wResult,wTest);
	return wResult;
}
#endif


//old name: WORD FPGA_GetHDE(void)
#if 0
WORD ScalerCalcHDE___OLD(void)
{
	WORD wTemp;
	BYTE PCLKO;

	WriteTW88Page(PAGE2_SCALER );
	wTemp = ReadTW88(REG20b);
	PCLKO = ReadTW88(REG20d) & 0x03;
	if(PCLKO==3)
		PCLKO = 2;

#if 0
	return wTemp+32;
#else //new 110624
	return wTemp+33 - PCLKO;
#endif
}
#endif

//Note: it is available after meas.

/*
VStart = REG(0x536[7:0],0x537[7:0])
VPulse = REG(0x52a[7:0],0x52b[7:0])
VPol = REG(0x041[3:3])
VScale = REG(0x206[7:0],0x205[7:0])

result = ((VStart - (VPulse * VPol)) * 8192 / VScale) + 1
*/

#if 0
WORD ScalerCalcVDE___OLD(void)
{
	BYTE VPol;
	WORD VStart,VPulse,VScale;
	WORD wResult;

	WriteTW88Page(PAGE5_MEAS);
	Read2TW88(REG536,REG537, VStart);
	Read2TW88(REG52A,REG52B, VPulse);

	WriteTW88Page(PAGE0_INPUT);
	VPol = ReadTW88(REG041) & 0x08 ? 1: 0;
	
	VScale = ScalerReadVScaleReg();

	wResult = ((DWORD)(VStart - (VPulse * VPol)) * 8192 / VScale) + 1;
	return wResult;
}
#endif

#if 0
WORD GetCalcVDEStart(void)		??sameas ScalerCalcVDE
{
	WORD VStart,VPulse,VScale;
	BYTE VPol;
	WORD wResult;
	DWORD dTemp;

	dPrintf("\n\rGetVScaledRatio()");

	WriteTW88Page(PAGE2_SCALER);
	Read2TW88(REG206,REG207, VScale);

	WriteTW88Page(PAGE5_MEAS);
	Read2TW88(REG536,REG537, VStart);
	Read2TW88(REG52a,REG52B, VPulse);

	
	WriteTW88Page(PAGE0_INPUT);
	VPol = ReadTW88(REG041) & 0x08 ? 1: 0;

	dPrintf("VStart:%d VPulse:%d VPol:%bd VScale:%d ",VStart,VPulse,VPol,VScale);

	//wResult = ((VStart - (VPulse*VPol)) * 8192 / VScale) + 1;
	dTemp = VStart - (VPulse*VPol);
	dTemp *= 8192;
	dTemp /= VScale;
	dTemp += 1;
	wResult = (WORD)dTemp;
	dPrintf(" result:%d",wResult);
	
	return wResult;
}
#endif

//===================================================================
//
//===================================================================

#ifdef UNCALLED_SEGMENT
//parameter
//	type - input
//			0:CVBS+NTSC
//			1:CVBS+PAL
//			2:RGB + SVGA
void ScalerSetDefault(BYTE type)
{
	if(type==0) {
		//CVBS+NTSC		720x480
		ScalerSetHScale(720);		//input:720. line_buff:720 output:Panel:800 
		ScalerSetVScale(240-15);	//??-15	ScalerSetVScale(240);	//480 with interlaced.

		ScalerSetLineBuffer(0x62, 720);	 	//98, 720
		ScalerWriteOutputHBlank(2);

		ScalerWriteHDEReg(0x84);			// position:132, size:0x320=800(PANEL_H)
		ScalerWriteVDEReg(0x30);			//ScalerSetVDEPosHeight(0x2c, PANEL_V);		// 44, 0x1e0=480(PANEL_V)
		//ScalerSetOutputWidthAndHeight(PANEL_H,PANEL_V);
		ScalerSetHSyncPosLen(0, 1);				//ScalerSetHSyncPosLen(0, 4);				//position:0 len:4
		ScalerSetVSyncPosLen(0,5);
		ScalerSetOutputFixedVline(OFF /*,0*/);			//off
		ScalerSetVDEMask(0,0);
	}
	else if(type==1) {
		//CVBS+PAL		720x576

		//need a default settings

		ScalerSetHScale(720);
		ScalerSetVScale(288);	//576 with interlaced.

		ScalerSetLineBuffer(0x62, 720);	 	//98, 720
		ScalerWriteOutputHBlank(2);

		ScalerWriteHDEReg(0x84);					//
		ScalerWriteVDEReg(0x30);					//28
		//ScalerSetOutputWidthAndHeight(PANEL_H,PANEL_V);
		ScalerSetHSyncPosLen(0, 1);				//ScalerSetHSyncPosLen(0, 4);				//position:0 len:4
		ScalerSetVSyncPosLen(0,5);
		ScalerSetOutputFixedVline(OFF /*,0*/);			//off
		ScalerSetVDEMask(0,0);
	}
	else /* if(type==2) */ {	//RGB + SVGA

		//need a default settings

		ScalerSetHScale(PANEL_H);
		ScalerSetVScale(PANEL_V);

		ScalerSetLineBuffer(0x32, PANEL_H);	// 50, 800
		ScalerWriteOutputHBlank(2);

		ScalerWriteHDEReg(0x53);					//83
		ScalerWriteVDEReg(0x1c);					//28
		//ScalerSetOutputWidthAndHeight(PANEL_H,PANEL_V);
		ScalerSetHSyncPosLen(0, 1);				//ScalerSetHSyncPosLen(0, 4);				//position:0 len:4
		ScalerSetVSyncPosLen(0, 1);
	}
}
#endif


//R202

//R207, R208

//called from measure
//BKTODO - it is a TW8823 version
//parameter
//	length: HAN(HorizontalActiveNumber)
#if 0
void	SetHScaleFull( WORD Length, WORD VPeriod, DWORD VPeriod27 )
{
	dPrintf( "\n\rSetHScaleFull(Length:%d, VPeriod:%d, VPeriod27:%ld)", Length, VPeriod, VPeriod27 );
}
#endif


//=============================================================================
//	YPbPr Table
//=============================================================================
//								 720    720    720    720 	1920   1280   1920
//							     480i,  576i,  480p, 576p,  1080i, 720p,  1080p
//code	BYTE	ClampPos[]   = { 140,   140,   52,    58,    24,    38,    24 };		//BKTODO:I need it for aPC also.

/*

SDTV 480i/60M
	 576i/50	
	 480p SMPTE 267M-1995
HDTV 1080i/60M
	 1080i/50
	 720p/60M
	 720p/50
	 1080p = SMPTE 274M-1995 1080p/24 & 1080p/24M
	                         1080p/50 1080p/60M


			scan lines	 field1 field2	 half
480i/60M	525			 23~262 285~524	 142x
576i/50		625			 23~310 335~622
1080i		1125
720p		750

standard
480i/60M	SMPTE 170M-1994.
			ITU-R BT.601-4
			SMPTE 125M-1995
			SMPTE 259M-1997
*/

//=============================================================================
//JUNK
//=============================================================================
//test 2BPP intersil
//!code WORD consolas16x26_606C90_2BPP[4] = {
//!	0xF7DE,0x0000,0x5AAB,0xC000
//!}; 
//!void FOsdIntersil(BYTE winno)
//!{
//!	BYTE palette;
//!	DECLARE_LOCAL_page
//!	BYTE i;
//!
//!	ReadTW88Page(page);
//!
//!	WaitVBlank(1);
//!	FOsdWinEnable(winno,OFF);	//winno disable
//!
//!	FOsdWinScreenXY(winno, 0,26);	//start 0x,0x  4 colums 1 line
//!	FOsdWinScreenWH(winno, 4, 1);	//start 0x,0x  4 colums 1 line
//! 	FOsdWinZoom(winno, 0, 0);			//zoom 1,1
//!	FOsdWinMulticolor(winno, ON);
//!
//!	WriteTW88Page(PAGE3_FOSD );
//!
//!	palette = 40;
//!	FOsdSetPaletteColorArray(palette,consolas16x26_606C90_2BPP,4, 0);
//!	FOsdRamSetAddrAttr(120, palette>>2);	//addr,palette,mode
//!	for(i=0; i < 4; i++) {
//!		WriteTW88(REG307, BPP2_START+4 + i*2);	//intersil icon
//!	}
//!
//!	FOsdWinEnable(winno,ON);		//winno enable
//!	WriteTW88Page(page);
//!}


//=============================================================================
// 
//=============================================================================
//!#if 0
//!void FOsdRam_Set(WORD index, BYTE ch, BYTE attr, BYTE len)
//!{
//!	BYTE i,bTemp;
//!	WORD addr;
//!	
//!	WriteTW88(REG304, ReadTW88(REG304) & ~0x0D);
//!	for(i=0; i < len; i++) {
//!		addr = index + i;
//!		bTemp = ReadTW88(REG305) & 0xFE;
//!		if(addr > 0x100)
//!			bTemp |= 0x01;
//!		WriteTW88(REG305, bTemp);
//!
//!		WriteTW88(REG306, (BYTE)addr);	//FOsdRamSetAddress(OsdRamAddr);
//!		WriteTW88(REG307, ch);
//!
//!		WriteTW88(REG306, (BYTE)addr);	//FOsdRamSetAddress(OsdRamAddr);
//!		WriteTW88(REG308, attr );	
//!	}
//!}
//!
//!void FOsdRam_Clear(WORD index, BYTE bg_color_index, BYTE len)
//!{
//!	FOsdRam_Set(index,FOSD_ASCII_BLANK,bg_color_index,len);
//!}
//!#endif

//!struct FWIN_s {
//!	BYTE no;
//!	BYTE col, row;
//!	BYTE attr;
//!	WORD start;
//!	WORD addr;
//!} fwin;

//assume bank3. AutoInc
//!#if 0
//!static void FOsdRam_goto(BYTE win, BYTE w,BYTE y)
//!{
//!	WORD addr;
//!	BYTE bTemp;
//!
//!	addr = fwin.start+fwin.col*y+w;
//!	fwin.addr = addr;
//!
//!	FOsdRamSetAddress(addr);
//!}
//!
//!static void FOsdRam_setAttr(BYTE attr, BYTE cnt, BYTE auto_mode )
//!{
//!	WORD addr;
//!	BYTE i,j;
//!
//!	addr = fwin.addr;
//!	fwin.attr = attr;
//!
//!	FOsdSetAccessMode(FOSD_OSDRAM_WRITE_AUTO);	//WriteTW88(REG304, (ReadTW88(REG304)&0xF3)|0x04); // Auto addr increment with D or A
//!	FOsdRamSetAddress(addr);
//!
//!	if(cnt) {
//!		for (i=0; i<(cnt/8); i++) {
//!			for ( j=0; j<8; j++ )
//!				WriteTW88(REG308, attr);
//!			delay1ms(1);
//!		}
//!		for ( j=0; j<(cnt%8); j++ )
//!			WriteTW88(REG308, attr);
//!	}
//!	else {
//!		WriteTW88(REG308, attr);
//!	}
//!
//!	if(auto_mode==3) {
//!	    //reset addr for data
//!		WriteTW88(REG304, ReadTW88(REG304) | 0x0C); // Auto addr increment with wit previous attr
//!		FOsdRamSetAddress(addr); 
//!	}
//!}
//!#endif
//!
//!//assume bank3.
//!#if 0
//!static void FOsdRam_putch(WORD ch)
//!{
//!   	if(ch < 0x100)
//!		WriteTW88(REG304,ReadTW88(REG304) & ~0x20);	//lower	
//!	else
//!		WriteTW88(REG304,ReadTW88(REG304) | 0x20);	//UP256
//!	WriteTW88(REG307, (BYTE)ch);	
//!}
//!#endif
//!
//!#if 0
//!static void FOsdRam_puts(WORD *s)
//!{
//!	BYTE hi = 0;
//!
//!   	if(*s < 0x100) {
//!		WriteTW88(REG304,ReadTW88(REG304) & ~0x20);	//lower	
//!	}
//!	while(*s) {
//!		if(hi==0 && *s >= 0x100) {
//!			hi=1;
//!			WriteTW88(REG304,ReadTW88(REG304) | 0x20); //UP256
//!		}
//!		WriteTW88(REG307,(BYTE)*s++);
//!	}
//!}
//!#endif


//!#if 0
//!void FOsdDownloadFontCode( void )
//!{
//!#ifdef SUPPORT_UDFONT
//!BYTE	i, j
//!#endif
//!BYTE    page;
//!
//!	ReadTW88Page(page);
//!	WaitVBlank(1);	
//!	McuSpiClkToPclk(0x02);	//with divider 1=1.5(72MHz). try 2
//!
//!	WriteTW88Page(PAGE3_FOSD );
//!
//!	WriteTW88(REG350, 0x09 );					// default FONT height: 18 = 9*2
//!
//!	WriteTW88(REG300, ReadTW88(REG300) & 0xFD ); // turn OFF bypass for Font RAM
//!	WriteTW88(REG309, 0x00 ); //Font Addr
//!
//!	FOsdSetAccessMode(FOSD_ACCESS_FONTRAM);	
//!
//!=======================
//!#ifdef SUPPORT_UDFONT
//!	FOsdFontWrite(0x00,&ROMFONTDATA[0][0], 27, 0xA0);
//!	FOsdFontWrite(0xA0,&RAMFONTDATA[0][0], 27, 0x22);
//!	FOsdFontWrite(0xC2,&RAMFONTDATA[0x82][0], 27, 0x60-0x22);
//!#endif
//!========================
//!
//!
//!#ifdef SUPPORT_UDFONT
//!	i = 0;
//!	for ( i=0; i<0xA0; i++ ) {
//!		WriteTW88(REG309, i);
//!
//!		for ( j = 0; j<27; j++ ) {
//!			WriteTW88(REG30A, ROMFONTDATA[i][j] );
//!		}
//!	}
//!	for ( i=0; i<0x22; i++ ) {
//!		WriteTW88(REG309, i+0xa0);
//!
//!		for ( j = 0; j<27; j++ ) {
//!			WriteTW88(REG30A, RAMFONTDATA[i][j] );
//!		}
//!	}
//!	for ( i=0; i<(0x60-0x22); i++ ) {
//!		WriteTW88(REG309, i+0x22+0xa0);
//!
//!		for ( j = 0; j<27; j++ ) {
//!			WriteTW88(REG30A, RAMFONTDATA[i+0x82][j] );
//!		}
//!	}
//!#endif
//!	FOsdSetAccessMode(FOSD_ACCESS_OSDRAM);	
//!
//!	WriteTW88(REG30B, 0xF0 );	  					// 2bit color font start
//!	WriteTW88(REG_FOSD_MADD3, 0xF0 );
//!	WriteTW88(REG_FOSD_MADD4, 0xF0 );
//!
//!	McuSpiClkRestore();
//!	WriteTW88Page(page );
//!}
//!#endif
//!
//!#if 0
//!void FOsdDownloadFont2Code( void )
//!{
//!BYTE	i, j, page;
//!
//!	ReadTW88Page(page);
//!	WaitVBlank(1);	
//!	McuSpiClkToPclk(0x02);	//with divider 1=1.5(72MHz). try 2
//!
//!	WriteTW88Page(PAGE3_FOSD );
//!
//!	WriteTW88(REG_FOSD_CHEIGHT, 0x09 );					// default FONT height: 18 = 9*2
//!
//!	WriteTW88(REG300, ReadTW88(REG300) & 0xFD ); // turn OFF bypass for Font RAM
//!	WriteTW88(REG309, 0x00 ); //Font Addr
//!
//!	i = 0;
//!	FOsdSetAccessMode(FOSD_ACCESS_FONTRAM);	
//!#ifdef SUPPORT_UDFONT
//!	for ( i=0; i<0xA0; i++ ) {
//!		WriteTW88(REG309, i);
//!
//!		for ( j = 0; j<27; j++ ) {
//!			WriteTW88(REG30A, RAMFONTDATA[i][j] );
//!		}
//!	}
//!#endif
//!	FOsdSetAccessMode(FOSD_ACCESS_OSDRAM);
//!
//!	WriteTW88(REG30B, 0xF0 );	  					// 2bit color font start
//!	WriteTW88(REG_FOSD_MADD3, 0xF0 );
//!	WriteTW88(REG_FOSD_MADD4, 0xF0 );
//!
//!	McuSpiClkRestore();
//!	WriteTW88Page(page );
//!}
//!#endif
//!
//!
//!
//!
//!
//!
//!//with Attr
//!//without Attr
//!//void FOsdRamWriteStr(WORD addr, BYTE *str, BYTE len)
//!//{
//!//}
//!
//!//void FontOsdWinChangeBackColor(BYTE index, WORD color)
//!//{
//!//}
//!
//!
//!//bank issue
//!//ex:
//!//	for(i=0; i < 8; i++)
//!//		FontOsdBpp3Alpha_setLutOffset(i,your_table[i]);	
//!#if 0
//!void FontOsdBpp3Alpha_setLutOffset(BYTE i, BYTE order)
//!{
//!	BPP3_alpha_lut_offset[i] = order;
//!}
//!#endif
//!
//!#ifdef UNCALLED_SEGMENT_CODE
//!void FontOsdWinAlphaGroup(BYTE winno, BYTE level)
//!{
//!}
//!#endif
//!
//!
//!
//!#if 0
//!void FOsdWriteAllPalette(WORD color)
//!{
//!	BYTE i;
//!	BYTE r30c;
//!	BYTE page;
//!
//!	ReadTW88Page(page);
//!
//!	McuSpiClkToPclk(CLKPLL_DIV_2P0);
//!
//!	WriteTW88Page(PAGE3_FOSD );
//!	r30c = ReadTW88(REG30C) & 0xC0;
//!	for(i=0; i < 64; i++) {
//!		WriteTW88(REG30C, r30c | i );
//!		WriteTW88(REG30D, (BYTE)(color>>8));
//!		WriteTW88(REG30E, (BYTE)color);
//!	}
//!	
//!	McuSpiClkRestore();
//!		
//!	WriteTW88Page(page);
//!}
//!#endif

//void SPI_ReadData2xdata     ( DWORD spiaddr, BYTE *ptr, DWORD cnt );
//==>SpiFlashDmaRead(DMA_DEST_MCU_XMEM,(WORD)(BYTE *)ptr, DWORD spiaddr, DWORD cnt);
#if 0
// TODO: pls use, (dest,src,len)
void SPI_ReadData2xdata( DWORD spiaddr, BYTE *ptr, DWORD cnt )
{
	WORD xaddr;

	xaddr = (WORD)ptr;

	WriteTW88Page(PAGE4_SPI );					// Set Page=4

#ifdef FAST_SPIFLASH
	WriteTW88(REG4C3, 0xC0 | CMD_x_BYTES);	//DMAMODE_R_XDATA );		// Mode = SPI -> incremental xdata
	WriteTW88(REG4CA, CMD_x_READ );			// Read Command

	WriteTW88(REG4CB, spiaddr>>16 );			// SPI address
	WriteTW88(REG4CC, spiaddr>>8 );				// SPI address
	WriteTW88(REG4CD, spiaddr );				// SPI address

 	WriteTW88(REG4C6, xaddr>>8 );				// Buffer address
	WriteTW88(REG4C7, xaddr );					// Buffer address

	WriteTW88(REG4DA, cnt>>16 );				// Read count
 	WriteTW88(REG4C8, cnt>>8 );					// Read count
	WriteTW88(REG4C9, cnt );					// Read count

	WriteTW88(REG4C4, 0x01);					//DMA-Read start
#else

	

	//dPrintf("\n\rSPI_ReadData2xdata(%lx,,%lx)",spiaddr,cnt);
	//dPrintf(" CMD:%02bx cmdlen:%bd",CMD_x_READ,CMD_x_BYTES);

	SpiFlashDmaDestType(DMA_DEST_MCU_XMEM,0);
	SpiFlashCmd(SPICMD_x_READ, SPICMD_x_BYTES);
	SpiFlashSetAddress2Hw(spiaddr);
	SpiFlashDmaBuffAddr(xaddr);
	SpiFlashDmaReadLen(cnt);
	SpiFlashDmaStart(SPIDMA_READ,0, __LINE__);
#endif
}
#endif

//=============================================================================
//
//=============================================================================

//void SPI_quadio(void)
//{
//	SPI_WriteEnable();
//  
//	WriteTW88Page(PAGE4_SPI );			// Set Page=4
//
//	WriteTW88(REG4C3, 0x42 );			// Mode = command write, Len=2
//	WriteTW88(REG4CA, 0x01 );			// SPI Command = WRITE_ENABLE
// 	WriteTW88(REG4C8, 0x40 );			// Read count
//	WriteTW88(REG4C4, 0x03 );			// DMA-Write start
//}


//=============================================================================
//		SPI DMA (SPI --> Fixed Register)
//=============================================================================
#if 0
void SPI_ReadData2Reg( WORD index, DWORD spiaddr, DWORD size )
{
	WriteTW88Page(PAGE4_SPI);
#if 0
	WriteTW88(REG403, DMAMODE_RW_FIX_REG);		// Mode = SPI -> fixed register

	WriteTW88(REG40a, SPICMD_x_READ );			// Read Command

	WriteTW88(REG40b, spiaddr>>16 );			// SPI address
	WriteTW88(REG40c, spiaddr>>8 );				// SPI address
	WriteTW88(REG40d, spiaddr );				// SPI address

	WriteTW88(REG406, index>>8 );				// Buffer address
	WriteTW88(REG407, index );					// Buffer address

	WriteTW88(REG41a, size>>16 );					// Read count
	WriteTW88(REG408, size>>8 );					// Read count
	WriteTW88(REG409, size );						// Read count
	
	WriteTW88(REG404, 0x01 );					// DMA-Read start
#else
//	SpiFlashDmaDestType(dest_type,0);
//BUGBUG: only support slow & fast.
	SpiFlashCmd(SPICMD_x_READ, SPICMD_x_BYTES);
	SpiFlashSetAddress2Hw(spiaddr);
	SpiFlashDmaBuffAddr(index);
	SpiFlashDmaReadLen(size);						
	SpiFlashDmaStart(SPIDMA_READ, SPIDMA_BUSYCHECK, __LINE__);
#endif
}
#endif
//=============================================================================
//		SPI DMA (SPI --> Incremental Register)
//=============================================================================
/*
void SPI_ReadData2RegInc( WORD index, DWORD spiaddr, DWORD cnt )
{
	WriteTW88Page(PAGE4_SPI );				// Set Page=5

	WriteTW88(REG403, DMAMODE_RW_INC_REG );		// Mode = SPI -> incremental register
	WriteTW88(REG40a, SPICMD_x_READ );				// Read Command
	WriteTW88(REG40b, spiaddr>>16 );				// SPI address
	WriteTW88(REG40c, spiaddr>>8 );				// SPI address
	WriteTW88(REG40d, spiaddr );					// SPI address
 	WriteTW88(REG406, index>>8 );				// Buffer address
	WriteTW88(REG407, index );					// Buffer address
	WriteTW88(REG41a, cnt>>16 );					// Read count
 	WriteTW88(REG408, cnt>>8 );					// Read count
	WriteTW88(REG409, cnt );						// Read count

	WriteTW88(REG404, 0x01 );					// DMA-Read start
}
*/

//=============================================================================
//		SPI DMA (SPI --> Incremental XDATA)
//=============================================================================
//#include <intrins.h>
//	_nop_();

//---------------------------------------------
//description
//	input data format selection
//	if input is PC(aRGB),DVI,HDMI, you have to set.
//@param
//	0:YCbCr 1:RGB
//
//CVBS:0x40
//SVIDEO:0x54. IFSET:SVIDEO, YSEL:YIN1
//---------------------------------------------
#if 0
void DecoderSetPath(BYTE path)
{
	WriteTW88Page(PAGE1_DECODER );	
	WriteTW88(REG102, path );   		
}
#endif

//---------------------------------------------
//
//@param
//	input_mode	0:RGB mode, 1:decoder mode
//register
//	R105
//---------------------------------------------
#if 0
void DecoderSetAFE(BYTE input_mode)
{
	WriteTW88Page(PAGE1_DECODER );	
	if(input_mode==0) {
		WriteTW88(REG105, (ReadTW88(REG105) & 0xF0) | 0x04);	//? C is for decoder, not RGB	
	}
	else {
		WriteTW88(REG105, (ReadTW88(REG105) | 0x0F));	
	}
}
#endif



//=============================================================================
// REMOVED
//=============================================================================

//===========================================
// ADC(AUX)
//===========================================
//internal
//note: NO PAGE change. Parent have to take care.
#ifdef SUPPORT_ANALOG_SENSOR
#ifdef SUPPORT_TOUCH
//desc: Read ADC value
//@param
//	0:X position
//	1:Z1 position
//	2:Z2 position
//	3:Y position
//	4: Aux0 value
//	5: AUX1 value
//	6: AUX2 value
//	7: AUX3 value
//register
//	R0B0
//	R0B2[7:0]R0B3[3:0]	TSC ADC Data
//internal
#ifdef TEST_BK111109
#endif
#endif
#endif


#ifdef SUPPORT_TOUCH
#ifdef TEST_BK111109
//??//desc:Read Z (presure) data.
//??//return
//??//	presure data
//??//	if fail, return 0xFF;
//??//Note:assuem page 0.
//??BYTE _TscGetZ(void)
//??{
//??	WORD z1, z2;
//??	z1 = _TscGetAdcDataOutput(ADC_MODE_Z1);
//??	if(z1 == 0) return (255);
//??	
//??	z2 = _TscGetAdcDataOutput(ADC_MODE_Z2);
//??
//??	dPrintf("\n\rTouch Z1:%d, Z2:%d", z1, z2 );
//??	Z1 = z1;
//??	Z2 = z2;
//??	return (z2 - z1);		
//??}
//??//desc:Read X position value
//??//global
//??//	TouchX : Latest pressed X position value
//??//Note:assuem page 0.
//??void _TscGetX(void)
//??{
//??	TouchX = _TscGetAdcDataOutput(ADC_MODE_X);
//??	//dPrintf("\n\rTouch X:%d", TouchX );
//??}
//??//desc:Read Y position value
//??//global
//??//	TouchX : Latest pressed Y position value
//??//Note:assuem page 0.
//??void _TscGetY(void)
//??{
//??	TouchY = _TscGetAdcDataOutput(ADC_MODE_Y);
//??	//dPrintf("\n\rTouch Y:%d", TouchY );
//??}
#endif
#endif //..

//=============================================================================
//		AUX 
//=============================================================================
#if 0 
//#ifdef SUPPORT_KEYPAD
void InitTscAdc(BYTE mode)
{
	 WriteTW88Page(PAGE0_TOUCH );
	 WriteTW88(REG0B0, mode );				// 0000-0xxx & mode
	 if(mode & 0x04) {
	 	//aux
		WriteTW88(REG0B1, 0x00 );			// enable the ready interrupt
		WriteTW88(REG0B4, 0x08 | 0x04 );	// div32.Continuous sampling for TSC_ADC regardless of the START command 
	 }
	 else {
	 	//touch
		WriteTW88(REG0B1, 0x80 );			// disable the ready interrupt
		WriteTW88(REG0B4, 0x08 );			// div2. Continuous sampling for TSC_ADC regardless of the START command
	 }
}
#endif

//global --BKTODO:see InitTouch also..
//@param
//	auxn: 0~3
#if 0
//#ifdef SUPPORT_TOUCH
//#ifdef SUPPORT_KEYPAD
void InitAUX( BYTE auxn )
{
#if 0
	 WriteTW88Page(PAGE0_TOUCH );
	 WriteTW88(REG0B0, 0x04 | auxn);			//  
	 WriteTW88(REG0B1, 0x00 );				// enable Ready Interrupt
	 WriteTW88(REG0B4, 0x08 | 0x04 );		// continuous sampling. div32  
#endif
	//init AUX0
	InitTscAdc(auxn | 0x04);	
}
#endif


#ifdef SUPPORT_TOUCH
//desc
//	Check the touch presure (Z value) and update TouchX and TouchY.
//update
//	TouchPressed
//	TouchDown
//	TouchX & TouchY
#ifdef TEST_BK111109
#endif //..TEST_BK111109
#endif

#if 0
void UpdateTouchCalibXY(BYTE index,WORD x, WORD y)
{
	TouchCalibX[index] = x;
	TouchCalibY[index] = y;
}
#endif
//WORD TouchGetCalibedX(BYTE index) { return TouchCalibX[index]; }
//WORD TouchGetCalibedY(BYTE index) { return TouchCalibY[index]; }
//void TouchSetCalibedXY(BYTE index, WORD x, WORD y) 
//{
//	TouchCalibX[index] = x;
//	TouchCalibY[index] = y;
//}

//=============================================================================
//		SenseTouch 
//=============================================================================
//return
//	1:Yes. return the position info
//	0:No input.
#ifdef SUPPORT_TOUCH
//return
//	1: success
//	0: fail
#ifdef TEST_BK111109
//!BYTE SenseTouch( WORD *xpos, WORD *ypos)
//!{
//!	bit TouchPressedOld;
//!
//!	TouchPressedOld = TouchPressed;
//!
//!	CheckTouch();
//!	if (!TouchPressed ) {
//!		if(TouchPressedOld)
//!			dTscPuts("\n\rTsc UP");
//!		return(0);
//!	}
//!	//
//!	//
//!	//
//!	if ( TouchPressedOld ) {   		// before it pressed with start
//!		_TscGetScreenPos();
//!		*xpos = PosX;
//!		*ypos = PosY;
//!		dPuts("\n\rSenseTouch:");
//!		if(TouchPressedOld)	dPuts(" OLD ");
//!		if(TouchPressed) dPuts(" NEW");
//!		if(TouchDown) dPuts(" DN"); 
//!		return (1);
//!	}
//!	else {
//!		_TscGetScreenPos();
//!		*xpos = PosX;
//!		*ypos = PosY;
//!		dPuts("\n\rSenseTouch:");
//!		if(TouchPressedOld)	dPuts(" OLD ");
//!		if(TouchPressed) dPuts(" NEW"); 
//!		if(TouchDown) dPuts(" DN"); 
//!		return (1);
//!	}
//!	return ( 0 );
//!}
#endif //..TEST_BK111109
#endif

//=============================================================================
//		GetTouch 
//=============================================================================
#ifdef SUPPORT_TOUCH
//!void GetTouch( void )
//!{
//!	WORD movX, movY; //move value
//!	BYTE dirX, dirY; //move direction
//!	BYTE TC;		 //Touch change counter
//!	bit	 TP;		 //pressed status
//!WORD	/*pressEndTime,*/ tsc_dt;
//!
//!	//update value 
//!	EA = 0;
//!	TC = CpuTouchChanged;
//!	TP = CpuTouchPressed;
//!	TouchX = CpuTouchX;
//!	TouchY = CpuTouchY;
//!	EA = 1;
//!
//!	if ( TouchChangedOld == TC ) return;			// no measurement
//!	TouchChangedOld = TC;
//!
//!	if ( TouchPressedOld ) {   		// before it pressed with start
//!		//pressed->
//!		if ( TP ) {
//!			//pressed->pressed
//!			//pressEndTime = tic01;  
//!			TscTimeEnd = SystemClock;
//!			_TscGetScreenPos();
//!			//direction. 
//!			if ( OldPosX > PosX ) {
//!				dirX = 0;					// decrease
//!				movX = OldPosX - PosX;
//!			}
//!			else {
//!				dirX = 1;					// increase
//!				movX = PosX - OldPosX;
//!			}
//!			if ( OldPosY > PosY ) {
//!				dirY = 0;					// decrease
//!				movY = OldPosY - PosY;
//!			}
//!			else {
//!				dirY = 1;					// increase
//!				movY = PosY - OldPosY;
//!			}
//!			if ( TouchStatus > TOUCHMOVE ) {
//!#if 1	 //BK111109
//!				//update direction
//!				if ( movX > movY ) {
//!					if (( movX / movY ) > 2) {
//!						if (dirX)
//!							TouchStatus = TOUCHMOVERIGHT;
//!						else
//!							TouchStatus = TOUCHMOVELEFT;
//!					}
//!				}
//!				else {
//!					if (( movY / movX ) > 2) {
//!						if (dirY)
//!							TouchStatus = TOUCHMOVEDOWN;
//!						else
//!							TouchStatus = TOUCHMOVEUP;
//!					}
//!				}
//!#endif
//!				OldPosX = PosX;
//!				OldPosY = PosY;
//!			}
//!			else 
//!			if (( movX > TSC_MOVE_MIN ) || ( movY > TSC_MOVE_MIN )) {
//!				if ( movX > movY ) {
//!					if (( movX / movY ) > 2) {
//!						if (dirX)
//!							TouchStatus = TOUCHMOVERIGHT;
//!						else
//!							TouchStatus = TOUCHMOVELEFT;
//!					}
//!				}
//!				else {
//!					if (( movY / movX ) > 2) {
//!						if (dirY)
//!							TouchStatus = TOUCHMOVEDOWN;
//!						else
//!							TouchStatus = TOUCHMOVEUP;
//!					}
//!				}
//!				OldPosX = PosX;
//!				OldPosY = PosY;
//!			}
//!			//TscPrintf("\n\rTouch Postion: xpos=%d, ypos=%d, endTime:%d", PosX, PosY, pressEndTime );
//!//			TscPrintf("\rTouch Postion: xpos=%d ypos=%d %dx%d  z1:%d endTime:%d", PosX, PosY, TouchX, TouchY, Z1, pressEndTime );
//!//			TscPrintf("\n\rTouch Postion: xpos=%d ypos=%d %dx%d  z1:%d endTime:%d", PosX, PosY, TouchX, TouchY, Z1, pressEndTime );
//!//			dTscPrintf("\n\rTouch Postion: xpos=%d ypos=%d %dx%d  z:%d(0x%x-0x%x) endTime:%ld", PosX, PosY, TouchX, TouchY, Z2-Z1,Z2,Z1, TscTimeEnd - TscTimeStart );
//!//			dTscPrintf("\n\rTouch Postion: xpos=%d ypos=%d %dx%d  z:%d(0x%x-0x%x) endTime:%ld", PosX, PosY, TouchX, TouchY, CpuZ2-CpuZ1,CpuZ2,CpuZ1, TscTimeEnd - TscTimeStart );
//!//			dTscPrintf("\n\rTouch Postion: xpos=%d ypos=%d %dx%d  z:%d endTime:%ld", PosX, PosY, TouchX, TouchY, CpuZ1, TscTimeEnd - TscTimeStart );
//!			dTscPrintf("\n\rTouch Position:xypos=%dx%d TouchXY=%dx%d  z:%d(0x%x-0x%x) endTime:%ld", PosX, PosY, TouchX, TouchY, CpuZ2-CpuZ1,CpuZ2,CpuZ1, TscTimeEnd - TscTimeStart );
//!			PrintTouchStatus(TouchStatus,0,0,0);	
//!			//OsdWinScreen( 0, PosX, PosY, 10, 10 );
//!		}
//!		else {
//!			//pressed->detached
//!			//TouchStatus
//!			//	TOUCHMOVE|TOUCHMOVED=>TOUCHEND	
//!			//	TOUCHCLICK=>TOUCHDOUBLECLICK
//!			//	?=>TOUCHCLICK
//!
//!			//use previous TscTimeEnd that is saved at the last pressed state.
//!
//!			dTscPrintf("\n\rTouchEndTime:%ld", TscTimeEnd);
//!			if(TscTimeEnd < TscTimeStart) {
//!				//overflow
//!				tsc_dt = 0xFFFFFFFF - TscTimeStart + TscTimeEnd;
//!			}
//!			else
//!				tsc_dt = TscTimeEnd - TscTimeStart;
//!			dTscPrintf("  Touch duration:%d", tsc_dt); //max 65536ms.
//!
//!			if ( OldPosX > StartX ) {
//!				dirX = 0;					// decrease
//!				movX = OldPosX - StartX;
//!			}
//!			else {
//!				dirX = 1;					// increase
//!				movX = StartX - OldPosX;
//!			}
//!			if ( OldPosY > StartY ) {
//!				dirY = 0;					// decrease
//!				movY = OldPosY - StartY;
//!			}
//!			else {
//!				dirY = 1;					// increase
//!				movY = StartY - OldPosY;
//!			}
//!			veloX = 1000;
//!			veloX *= movX;
//!			veloX /= tsc_dt;
//!			veloY = 1000;
//!			veloY *= movY;
//!			veloY /= tsc_dt;
//!			dTscPrintf("\n\rVelocity X:%ld Y:%ld", veloX,veloY );
//!
//!#if 0 //BK111108
//!			if (( TouchStatus >= TOUCHMOVE) || ( TouchStatus == TOUCHMOVED ))
//!				TouchStatus = TOUCHEND;
//!#else
//!			if (TouchStatus >= TOUCHMOVE) {
//!				TouchStatus = TOUCHMOVED;
//!			}
//!#endif
//!			else	{
//!				if((TscTimeLastEnd + 10) > TscTimeStart) {
//!					TouchStatus = TOUCHCLICK;
//!				}
//!				else if((TscTimeLastEnd + 100) > TscTimeStart) {
//!					TouchStatus = TOUCHDOUBLECLICK;
//!				}
//!				else {
//!					if(tsc_dt > 1000)	TouchStatus = TOUCHLONGCLICK;		//more then 10sec, it is a special..
//!					else	TouchStatus = TOUCHCLICK;
//!				}
//!			}
//!			//EndX = OldPosX;
//!			//EndY = OldPosY;
//!			//dTscPrintf("\n\rTouch Postion: xpos=%d, ypos=%d", OldPosX, OldPosY);
//!			//dTscPrintf("\n\rTouch Move: movx=(%bd)%d, movy=(%bd)%d", dirX, movX, dirY, movY);
//!			//dTscPrintf("\n\rTouch Velocity: vx=%ld, vy=%ld", veloX, veloY );
//!			//dTscPrintf("\n\rTouchEndTime:%d", pressEndTime);
//!			LastTouchStatus = TouchStatus;
//!
//!
//!			//dTscPrintf("\n\rTouch End:  OLDxypos=%dx%d TouchXY=%dx%d  z:%d(0x%x-0x%x) endTime:%ld", OldPosX, OldPosY, TouchX, TouchY, CpuZ2-CpuZ1,CpuZ2,CpuZ1, TscTimeEnd - TscTimeStart );
//!			//dTscPrintf("\n\rTouch End:  OLDxypos=%dx%d Move: movx=%s%d, movy=%s%d Velocity: vx=%ld, vy=%ld TouchEndTime:%d", 
//!			//	OldPosX, OldPosY, 
//!			//	dirX ? "+" : "-", movX, 
//!			//	dirY ? "+" : "-", movY, 
//!			//	veloX, veloY, 
//!			//	pressEndTime );
//! 			dTscPrintf("\n\rTouch End:  OLDxypos=%dx%d Move: movx=%s%d, movy=%s%d Velocity: vx=%ld, vy=%ld", 
//!				OldPosX, OldPosY, 
//!				dirX ? "+" : "-", movX, 
//!				dirY ? "+" : "-", movY, 
//!				veloX, veloY);
//!			PrintTouchStatus(TouchStatus,0,0,0);
//!		}
//!	}
//!	else {
//!		//normal ->
//!		if ( TP ) {
//!			//OLD: normal->pressed
//!			//if((TouchStatus==TOUCHMOVED) & ((TscTimeLastEnd + 10) > TscTimeStart)) {
//!			if(((TouchStatus==TOUCHMOVED) || (TouchStatus==TOUCHEND && LastTouchStatus==TOUCHMOVED) )
//!			& ((TscTimeLastEnd + 10) > SystemClock)) {
//!				TouchStatus = TOUCHMOVE;	
//!
//!
//!				_TscGetScreenPos();
//!				/*OldPosX = StartX = PosX; */
//!				/*OldPosY = StartY = PosY; */
//!				dTscPrintf("\n\rTouch MOVE_AGAIN:xypos=%dx%d TouchXY=%dx%d  z:%d(0x%x-0x%x) endTime:%ld", PosX, PosY, TouchX, TouchY, CpuZ2-CpuZ1,CpuZ2,CpuZ1, TscTimeEnd - TscTimeStart );
//!			}
//!			else if ((TscTimeEnd+10) > SystemClock && TouchStatus==TOUCHMOVED ) {
//!				dTscPrintf("\n\rTouch MOVE_AGAIN: dt:%ld",SystemClock- TscTimeEnd);
//!				TouchStatus = TOUCHMOVE;
//!			}
//!			else {
//!				//normal->pressed
//!				TouchStatus = TOUCHPRESS;
//!				/*pressEndTime =*/ /*pressStartTime = tic01; */
//!				TscTimeLastEnd = TscTimeEnd;
//!				TscTimeStart = /*TscTimeEnd =*/ SystemClock;
//!				_TscGetScreenPos();
//!				OldPosX = StartX = PosX;
//!				OldPosY = StartY = PosY;
//!	//			TscPrintf("\n\rTouch Start with: xpos=%d, ypos=%d, startTime:%d", StartX, StartY, pressStartTime );
//!	//			TscPrintf("\rTouch Start : xpos=%d ypos=%d %dx%d  z1:%d startTime:%d", StartX, StartY, TouchX, TouchY, Z1, pressStartTime );
//!	//			TscPrintf("\rTouch Start : xpos=%d ypos=%d %dx%d  z1:%d startTime:%d", StartX, StartY, TouchX, TouchY, Z1, pressStartTime );
//!	//			dTscPrintf("\n\rTouch Start : xpos=%d ypos=%d %dx%d  z:%d startTime:%d", StartX, StartY, TouchX, TouchY, CpuZ1, pressStartTime );
//!				//OsdWinScreen( 0, PosX, PosY, 10, 10 );
//!				dTscPrintf("\n\rTouch Start:   xypos=%dx%d TouchXY=%dx%d  z:%d(0x%x-0x%x) endTime:%ld", PosX, PosY, TouchX, TouchY, CpuZ2-CpuZ1,CpuZ2,CpuZ1, TscTimeEnd - TscTimeStart );
//!			}
//!		}
//!		else {
//!		}
//!	}
//!	TouchPressedOld = TP;
//!}
//!
#endif	//..SUPPORT_TOUCH

//=============================================================================
//		GetVeloX 
//		GetVeloY 
//=============================================================================
#ifdef UNCALLED_SEGMENT
WORD GetVeloX( void )
{...}
WORD GetVeloY( void )
{...}
#endif

//=============================================================================
//		CheckTouchStatus 
//=============================================================================
#ifdef TEST_BK111109
//!BYTE CheckTouchStatus( WORD *xpos, WORD *ypos)
//!{
//!	static BYTE old_TouchStatus;
//!
//!	GetTouch();
//!	*xpos = OldPosX;
//!	*ypos = OldPosY;
//!	if(old_TouchStatus != TouchStatus) {
//!		PrintTouchStatus(TouchStatus,OldPosX,OldPosY, 1);
//!#if 0
//!		switch(TouchStatus) {
//!		case TOUCHPRESS: 		dPrintf("\n\rTOUCHPRESS xpos=%d ypos=%d",OldPosX,OldPosY); break;
//!		case TOUCHMOVED: 		dPuts  ("\n\rTOUCHMOVED"); break;
//!		case TOUCHEND: 			dPuts  ("\n\rTOUCHEND"); break;
//!		case TOUCHCLICK: 		dPrintf("\n\rTOUCHCLICK xpos=%d ypos=%d",OldPosX,OldPosY); break;
//!		case TOUCHDOUBLECLICK: 	dPrintf("\n\rTOUCHDOUBLECLICK xpos=%d ypos=%d",OldPosX,OldPosY); break;
//!		case TOUCHMOVE: 		dPuts  ("\n\rTOUCHMOVE"); break;
//!		case TOUCHMOVEUP: 		dPuts  ("\n\rTOUCHMOVEUP"); break;
//!		case TOUCHMOVEDOWN: 	dPuts  ("\n\rTOUCHMOVEDOWN"); break;
//!		case TOUCHMOVELEFT: 	dPuts  ("\n\rTOUCHMOVELEFT"); break;
//!		case TOUCHMOVERIGHT: 	dPuts  ("\n\rTOUCHMOVERIGHT"); break;
//!		default: 				dPrintf("\n\rTOUCH UNKNOWN:%bd", TouchStatus); break;
//!		}
//!#endif
//!		old_TouchStatus = TouchStatus;
//!	}
//!	return( TouchStatus );
//!}
#endif

#if 0  //remove 110207
//LutOffset. 0, 64, 126,...255
//size 256 table with 8bpp will be 256*4 = 0x400
//BK110207: LUT 256->512. Need 9bit
void SpiOsdLoadLUTS(WORD LutOffset, WORD size, DWORD	address )
{
}
#endif

//#include "Data\DataInitCVBSFull.inc"
#if 0
code BYTE default_register[] = {
// cmd  reg		mask	value
	w, 0xFF, 0x00,
	r,	0x02,	0x00,	0x00,
	r,	0x03,	0x00,	0xFF,
	r,	0x04,	0xF0,	0x00,
	r,	0x06,	0x00,	0x00,
	r,	0x07,	0x00,	0x00,
};
void CheckHWDefaultRegister()
{
}
#endif


#if 0
code BYTE DefPage0Regs[256] = {
    /*         0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
	/*000:*/0x78,0x00,0xC2,0xFF,0x01,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*010:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*020:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*030:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*040:*/0x00,0x00,0x02,0x20,0xF0,0x20,0xD0,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,
	/*050:*/0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0xBF,0xBF,0xBF,0xBF,0x00,0x00,0x00,0x00,
	/*060:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x20,0xF0,0x20,0xD0,0x10,0x10,
	/*070:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*080:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*090:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x00,0xFF,0x00,
	/*0A0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x99,
	/*0B0:*/0x87,0x00,0x00,0x00,0x00,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*0C0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0D0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x80,0x55,0x00,0x80,0x00,0x80,
	/*0E0:*/0xF2,0x77,0x04,0x40,0x84,0x00,0x20,0x00,0xF2,0x0A,0x04,0x40,0x20,0x80,0x20,0x00,
	/*0F0:*/0xC8,0x02,0x00,0x00,0x00,0x00,0x00,0x16,0x01,0x20,0x00,0x40,0xB0,0x11,0x00,0x00 };
//REG006 makes hang. 120628 it was 0xFF
//REG0D4[0] makes reset.
code BYTE RegMask_W_Page0[256] = {
	/*000:*/0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x7F,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
	/*010:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
	/*020:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*030:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*040:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*050:*/0xFE,0x37,0x7F,0x0F,0xFF,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
	/*060:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*070:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*080:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*090:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0A0:*/0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0B0:*/0xFF,0xFF,0x00,0xF0,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0C0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0D0:*/0x00,0x00,0x00,0x00,0xFE,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*0E0:*/0xFF,0xFF,0xCF,0xFF,0xFF,0xFF,0xFF,0x00,0xFF,0xFF,0xCF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*0F0:*/0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00 };
code BYTE RegMask_R_Page0[256] = {
	/*000:*/0xFF,0x00,0xFF,0xFF,0xFF,0xFF,0x7F,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
	/*010:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
	/*020:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*030:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*040:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x07,0x00,0x00,0x00,0x00,0x00,
	/*050:*/0xFE,0x37,0x7F,0x0F,0xFF,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
	/*060:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*070:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*080:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*090:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*0A0:*/0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0B0:*/0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0C0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0D0:*/0x00,0x00,0x00,0x00,0xFF,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*0E0:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*0F0:*/0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00 };


code BYTE DefPage1Regs[256] = {
    /*         0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
	/*100:*/0x00,0x93,0x40,0x04,0x00,0x00,0x00,0x12,0x12,0x20,0x0A,0xD0,0xCC,0x00,0x11,0x00,
	/*110:*/0x00,0x5C,0x11,0x80,0x80,0x00,0x00,0x80,0x44,0x58,0x10,0x00,0x27,0x7F,0x00,0x00,
	/*120:*/0x50,0x22,0xF0,0xD8,0xBC,0xB8,0x44,0x38,0x00,0x00,0x78,0x44,0x30,0x14,0xA5,0xE0,
	/*130:*/0xD0,0x00,0x00,0x05,0x1A,0x00,0xE3,0x28,0xAF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*140:*/0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*150:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*160:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*170:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*180:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*190:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*1A0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*1B0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*1C0:*/0x00,0xC7,0x01,0x03,0x5A,0x00,0x20,0x04,0x00,0x06,0x06,0x30,0x00,0x54,0x00,0x00,
	/*1D0:*/0x00,0xF0,0xF0,0xF0,0x00,0x00,0x10,0x70,0x00,0x04,0x80,0x80,0x20,0x00,0x00,0x00,
	/*1E0:*/0x00,0x05,0xD9,0x07,0x33,0x31,0x00,0x2A,0x01,0x00,0x03,0x00,0x00,0x00,0x00,0x00,
	/*1F0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
code BYTE RegMask_W_Page1[256] = {
	/*000:*/0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,
	/*010:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xE0,0x00,0x0F,0xFF,0x0F,0xFF,
	/*020:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*030:*/0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*040:*/0xC0,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*050:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*060:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*070:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*080:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*090:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0A0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0B0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0C0:*/0xFF,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,
	/*0D0:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,
	/*0E0:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,
	/*0F0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
code BYTE RegMask_R_Page1[256] = {
	/*000:*/0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,
	/*010:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*020:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*030:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*040:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*050:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*060:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*070:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*080:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*090:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0A0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0B0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0C0:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,
	/*0D0:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,
	/*0E0:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,
	/*0F0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };


code BYTE DefPage2Regs[256] = {
    /*         0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
	/*200:*/0x00,0x00,0x20,0x00,0x20,0x00,0x20,0x80,0x10,0x00,0x04,0x30,0xD0,0x00,0x20,0x00,
	/*210:*/0x10,0x00,0x03,0x10,0x20,0x20,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x06,0x00,0x00,
	/*220:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*230:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*240:*/0x10,0x00,0x01,0x00,0x00,0x01,0x00,0x00,0x01,0x10,0x00,0x00,0x10,0x80,0x00,0x00,
	/*250:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*260:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*270:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*280:*/0x20,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x30,0x00,0xBF,0xBF,0xBF,
	/*290:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*2A0:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*2B0:*/0x10,0x40,0x40,0x08,0x10,0x0D,0x67,0x94,0x2A,0xD0,0x02,0x18,0xBF,0xBF,0x00,0x00,
	/*2C0:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*2D0:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*2E0:*/0x00,0x00,0x00,0x00,0x00,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*2F0:*/0x00,0x00,0x00,0x00,0x00,0x00,0xBF,0xBF,0x00,0x80,0xBF,0xBF,0xBF,0xBF,0xBF,0x00 };
code BYTE RegMask_W_Page2[256] = {
	/*000:*/0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*010:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*020:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*030:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*040:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*050:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*060:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*070:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*080:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x80,0x00,0x00,0x00,
	/*090:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0A0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0B0:*/0xFF,0xFF,0xFF,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,
	/*0C0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0D0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0E0:*/0xFF,0xFF,0x03,0xFF,0x77,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0F0:*/0xFF,0xFF,0x7F,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00 };
code BYTE RegMask_R_Page2[256] = {
	/*000:*/0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*010:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*020:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*030:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*040:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*050:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*060:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*070:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*080:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x80,0x00,0x00,0x00,
	/*090:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0A0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0B0:*/0xFF,0xFF,0xFF,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,
	/*0C0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0D0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0E0:*/0xFF,0xFF,0x03,0xFF,0x77,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0F0:*/0xFF,0xFF,0x7F,0xFF,0xFF,0xFF,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00 };


code BYTE DefPage3Regs[256] = {
    /*         0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
	/*300:*/0x00,0x06,0x06,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x31,0x00,0x00,0x00,0xBF,
	/*310:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*320:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*330:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*340:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*350:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*360:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*370:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*380:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*390:*/0x12,0x1B,0x00,0x71,0xB1,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*3A0:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*3B0:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*3C0:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*3D0:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*3E0:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*3F0:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0x00 };
code BYTE RegMask_W_Page3[256] = {
	/*000:*/0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*010:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*020:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*030:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*040:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*050:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*060:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*070:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*080:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*090:*/0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0A0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0B0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0C0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0D0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0E0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0F0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
code BYTE RegMask_R_Page3[256] = {
	/*000:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*010:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*020:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*030:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*040:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*050:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*060:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*070:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*080:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*090:*/0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0A0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0B0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0C0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0D0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0E0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0F0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };


code BYTE DefPage4Regs[256] = {
    /*         0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
	/*400:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x45,
	/*410:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*420:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*430:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*440:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*450:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*460:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*470:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*480:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*490:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*4A0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*4B0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*4C0:*/0x00,0x00,0x00,0x40,0x80,0x80,0x04,0x90,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,
	/*4D0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x08,0x00,0x00,0x00,0x00,0x00,0x00,
	/*4E0:*/0x00,0x06,0x69,0x78,0x01,0x0E,0x00,0x1B,0x00,0x0C,0x00,0x18,0x01,0x00,0x00,0x00,
	/*4F0:*/0x00,0x00,0x00,0x40,0x80,0x00,0x04,0x90,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
//REG4C2 makes hang. it was 0x01
code BYTE RegMask_W_Page4[256] = {
	/*000:*/0xFF,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,
	/*010:*/0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*020:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*030:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*040:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*050:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*060:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*070:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*080:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*090:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*0A0:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*0B0:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*0C0:*/0x87,0x01,0x00,0xFF,0x07,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*0D0:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x0F,0xFF,0xFF,0x1F,0x10,
	/*0E0:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,
	/*0F0:*/0x07,0x01,0x10,0xFF,0x07,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };
code BYTE RegMask_R_Page4[256] = {
	/*000:*/0xFF,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,
	/*010:*/0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*020:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*030:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*040:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*050:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*060:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*070:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*080:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*090:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*0A0:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*0B0:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	/*0C0:*/0x87,0x01,0x03,0xFF,0xC7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*0D0:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x0F,0xFF,0xFF,0x1F,0x10,
	/*0E0:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,
	/*0F0:*/0x07,0x01,0x10,0xFF,0x07,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };


code BYTE DefPage5Regs[256] = {
    /*         0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
	/*500:*/0x00,0x20,0x01,0xE0,0x00,0x20,0x00,0xDA,0x00,0x00,0x00,0x8C,0x00,0xBF,0xBF,0xBF,
	/*510:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x80,0x80,0x0C,
	/*520:*/0x80,0x80,0x00,0x00,0x10,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*530:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*540:*/0xFF,0x00,0xFF,0x08,0x3D,0x5F,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*550:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*560:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*570:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*580:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*590:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*5A0:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*5B0:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*5C0:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*5D0:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*5E0:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,
	/*5F0:*/0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0x00	 };
code BYTE RegMask_W_Page5[256] = {
	/*000:*/0x07,0xFF,0x0F,0xFF,0x07,0xFF,0x07,0xFF,0x0F,0x7F,0x8F,0xFF,0x00,0x00,0x00,0x00,
	/*010:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,
	/*020:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*030:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*040:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*050:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*060:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*070:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*080:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*090:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0A0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0B0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0C0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0D0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0E0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0F0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
code BYTE RegMask_R_Page5[256] = {
	/*000:*/0x07,0xFF,0x0F,0xFF,0x07,0xFF,0x07,0xFF,0x0F,0x7F,0x8F,0xFF,0x00,0x00,0x00,0x00,
	/*010:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*020:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/*030:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,
	/*040:*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*050:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*060:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*070:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*080:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*090:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0A0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0B0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0C0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0D0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0E0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*0F0:*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
#endif


#if 0
//-----------------------------------------------------------------------------
/**
* Description
* 	reset RLC register.
*	If write "1" when RLE is working, it makes system hang.
* @param
* @return
*/
void SpiOsdResetRLC(BYTE winno, BYTE reset)
{

	WriteTW88Page(PAGE4_SOSD );
	if(winno==0) {
		if(reset)	WriteTW88(REG404, ReadTW88(REG404) | 0x02);
		else		WriteTW88(REG404, ReadTW88(REG404) & 0xFD);
		if(reset)	WriteTW88(REG406, ReadTW88(REG406) | 0x02);
		else		WriteTW88(REG406, ReadTW88(REG406) & 0xFD);
	}
	else
	if(winno==1 || winno==2) {
		if(reset)	WriteTW88(REG406, ReadTW88(REG406) | 0x02);
		else		WriteTW88(REG406, ReadTW88(REG406) & 0xFD);
	}
	else 
	{
		if(reset)	WriteTW88(REG404, ReadTW88(REG404) | 0x02);
		else		WriteTW88(REG404, ReadTW88(REG404) & 0xFD);
	}
}
#endif


//=============================================================================
// test routine
//=============================================================================
#if 0
BYTE nStartPosX;
BYTE nStartPosY;
BYTE nCarIndex=0;
BYTE nMoving=0;
#define WIN_GRID_IMG		1
#define _BLOCK_OFFSET		0x3B100L
#define _IMG_OFFSET			0xF00
#define _CNT_MAX			62
#define TEST_GRID_LUT_LEN		0x040L
#define TEST_GRID_START			0x800000	//from 8MByte. use 16MByte(128MBit) SPIFLASH

#define TEST_GRID_H			760
#define TEST_GRID_V			480

DWORD lGridStartAddr;
DWORD lOffsetStart;

FAR CONST BYTE test_grid_rle[5][_CNT_MAX] = {
//###################################################################################################

	//01. Velfire/Alphard
	//RLC 00,	01,   02,   03,   04,   05,   06,   07,   08,   09,
	{	0x4c, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC00 - 09
		0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC10 - 19
		0x4a, 0x4a,	0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC20 - 29
		0x49, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC30 - 39
		0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC40 - 49
		0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC50 - 59
		0x4a, 0x4a},													//RLC60 - 61

	//02. Prius-alpha
	//RLC 00,	01,   02,   03,   04,   05,   06,   07,   08,   09,
	{	0x4b, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC00 - 09
		0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC10 - 19
		0x4a, 0x4a,	0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC20 - 29
		0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC30 - 39
		0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC40 - 49
		0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC50 - 59
		0x4a, 0x4a},													//RLC60 - 61

	//03. Prius
	//RLC 00,	01,   02,   03,   04,   05,   06,   07,   08,   09,
	{	0x4b, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC00 - 09
		0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC10 - 19
		0x4a, 0x4a,	0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC20 - 29
		0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC30 - 39
		0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC40 - 49
		0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC50 - 59
		0x4a, 0x4a},													//RLC60 - 61

	//04. Delica
	//RLC 00,	01,   02,   03,   04,   05,   06,   07,   08,   09,
	{	0x4b, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC00 - 09
		0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC10 - 19
		0x4a, 0x4a,	0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC20 - 29
		0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC30 - 39
		0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC40 - 49
		0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC50 - 59
		0x4a, 0x4a},													//RLC60 - 61

	//xx. Test modes...
	//RLC 00,	01,   02,   03,   04,   05,   06,   07,   08,   09,
	{	0x4b, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49,		//RLC00 - 09
		0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49,		//RLC10 - 19
		0x49, 0x49,	0x49, 0x49, 0x49, 0x49, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC20 - 29
		0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49,		//RLC30 - 39
		0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49,		//RLC40 - 49
		0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x4a, 0x4a, 0x4a, 0x4a,		//RLC50 - 59
		0x4a, 0x49},													//RLC60 - 61
};



void DParkGridAction(BYTE n)
{
	DWORD	offset;
	BYTE rle_byte;
	DECLARE_LOCAL_page;

	ReadTW88Page(page);

	offset = (_IMG_OFFSET * (DWORD)n) + lOffsetStart;
	rle_byte = test_grid_rle[nCarIndex][n];



	SpiOsdWinImageLoc( WIN_GRID_IMG, offset );
	SpiOsdWinImageSizeWH(WIN_GRID_IMG, TEST_GRID_H, TEST_GRID_V );
	SpiOsdWinScreen( WIN_GRID_IMG, nStartPosX, nStartPosY , TEST_GRID_H, TEST_GRID_V ); //noisy on bottom area when nStartPosY has a 5.
	//SpiOsdWinScreen( WIN_GRID_IMG, nStartPosX, nStartPosY , TEST_GRID_H, TEST_GRID_V - nStartPosY ); //fix the noise issue.
	SpiOsdWinPixelAlpha( WIN_GRID_IMG, ON );
	SpiOsdWinPixelWidth(WIN_GRID_IMG,rle_byte >> 4);	//all 4BPP.
	SpiOsdWinLutOffset(WIN_GRID_IMG,0);	 	//all 0
	SpiOsdWinBuffEnable( WIN_GRID_IMG, ON );



	WaitVBlank(1);
//PORT_DEBUG=0;
	SpiOsdRlcReg( WIN_GRID_IMG, rle_byte >> 4, rle_byte & 0x0F);
	SpiOsdLoadLUT(WIN_GRID_IMG, 1, 0, TEST_GRID_LUT_LEN, lGridStartAddr,0xFF); //Load Palette : type, LUT offset, size, address
	SOsdWinBuffWrite2Hw(WIN_GRID_IMG, WIN_GRID_IMG);
	WriteTW88Page( PAGE4_SOSD );
	WriteTW88( REG410, 0xc3 );    		// LUT Write Mode, En & byte ptr inc.
	//assume LUT is under 256
	//addr							data
	WriteTW88(REG411, 0x00);			WriteTW88(REG412, 0x7F);
	/*WriteTW88(REG411, 0x01);*/		WriteTW88(REG412, 0x3F);
	/*WriteTW88(REG411, 0x02);*/		WriteTW88(REG412, 0x3F);
	/*WriteTW88(REG411, 0x03);*/		WriteTW88(REG412, 0x3F);
	/*WriteTW88(REG411, 0x04);*/		WriteTW88(REG412, 0x3F);
	/*WriteTW88(REG411, 0x05);*/		WriteTW88(REG412, 0x3F);
	/*WriteTW88(REG411, 0x06);*/		WriteTW88(REG412, 0x3F);
	/*WriteTW88(REG411, 0x07);*/		WriteTW88(REG412, 0x3F);
	/*WriteTW88(REG411, 0x08);*/		WriteTW88(REG412, 0x3F);
	/*WriteTW88(REG411, 0x09);*/		WriteTW88(REG412, 0x3F);
	/*WriteTW88(REG411, 0x0a);*/		WriteTW88(REG412, 0x3F);
	/*WriteTW88(REG411, 0x0b);*/		WriteTW88(REG412, 0x3F);
	/*WriteTW88(REG411, 0x0c);*/		WriteTW88(REG412, 0x3F);
	/*WriteTW88(REG411, 0x0d);*/		WriteTW88(REG412, 0x3F);
	/*WriteTW88(REG411, 0x0e);*/		WriteTW88(REG412, 0x3F);
	/*WriteTW88(REG411, 0x0f);*/		WriteTW88(REG412, 0x3F);
//PORT_DEBUG=1;
	dPuts("\n\rFinished LUT GRID DMA");
	WriteTW88Page(page);	
}

void TestDParkGridAction(void)
{
	BYTE n;
	DECLARE_LOCAL_page;

	ReadTW88Page(page);
	
	SOsdWinBuffClean(0);
	//init DE
	SpiOsdSetDeValue();
	SpiOsdWinHWOffAll(0); //without wait
	SpiOsdEnable(ON);


	nStartPosX = 0;
	nStartPosY = 0;
	while(!RS_ready()) {			
		for(nCarIndex=0; nCarIndex < 4; nCarIndex++) {

			lGridStartAddr = TEST_GRID_START+ _BLOCK_OFFSET * nCarIndex;
			lOffsetStart = lGridStartAddr + TEST_GRID_LUT_LEN;

			for(n=0; n < _CNT_MAX; n++) {
				DParkGridAction(n);
				if(n) delay1ms(100);
				else  delay1ms(500);

			}
		}
		nStartPosX += 50;
		nStartPosX %= 400;
		nStartPosY += 50;
		nStartPosY %= 240;
	}



	WaitVBlank(1);
	SpiOsdWinBuffEnable( WIN_GRID_IMG, OFF ); //disable window.
	SpiOsdEnable(OFF);
	WriteTW88Page(page);	
}

void TestAutoDParkGridAction(BYTE positionX, BYTE positionY)
{
	BYTE n;
	DECLARE_LOCAL_page
	//BYTE ch;

	ReadTW88Page(page);
	
	SOsd_CleanReg();
	//init DE
	SpiOsdSetDeValue();
	SpiOsdWinHWOffAll(0); //without wait
	SpiOsdEnable(ON);

	nStartPosX = positionX;
	nStartPosY = positionY;

	//Printf("\n\rto quit, press 'q'");
	while(!RS_ready()) {
		for(nCarIndex=0; nCarIndex < 4; nCarIndex++) {

			lGridStartAddr = TEST_GRID_START+ _BLOCK_OFFSET * nCarIndex;
			lOffsetStart = lGridStartAddr + TEST_GRID_LUT_LEN;

			for(n=0; n < 58 /*_CNT_MAX*/; n++) {
				//while(!RS_ready());
				//ch = RS_rx();
				//if(ch=='q') 
				//	break;
				DParkGridAction(n);
			}
			
		}
		//if(ch=='q')
		//	break;
	}
	RS_rx();	//clear
	WaitVBlank(1);
	SpiOsdWinBuffEnable( WIN_GRID_IMG, OFF ); //disable window.
	SpiOsdEnable(OFF);
	WriteTW88Page(page);	
}
void TestStepDParkGridAction(BYTE positionX, BYTE positionY)
{
	DECLARE_LOCAL_page

	ReadTW88Page(page);
	
	SOsd_CleanReg();
	//init DE
	SpiOsdSetDeValue();
	SpiOsdWinHWOffAll(0); //without wait
	SpiOsdEnable(ON);

	nStartPosX = positionX;
	nStartPosY = positionY;

	lGridStartAddr = TEST_GRID_START+ _BLOCK_OFFSET * nCarIndex;
	lOffsetStart = lGridStartAddr + TEST_GRID_LUT_LEN;
	Printf("\n\rCar:%bd, move:%bd", nCarIndex, nMoving);
	DParkGridAction(nMoving);
	nMoving++;
	if(nMoving==58) {
		nMoving=0;
		nCarIndex++;
		if(nCarIndex==5)
			nCarIndex=0;
	}
	if(nMoving==1 &&  nCarIndex==4) {
		nCarIndex = 0;
		nMoving = 0;
	}
}
#endif //..test routine

//-----------------------------------------------------------------------------
/**
* Description
*	download LUT by IO (without DMA).
*	It is only for old image files that was use on TW8832.
*
*	TW8836 has two LUTs.
*	LUTB(LUT group B) has 256 palettes, We can use DMA on LUTTYPE_ADDR.
*	LUTA(LUT group A) has 512 palettes. If use LUTTYPE_ADDR, we must use a IO method.
*
*	This function needs a buffer because SPI.c is located on Code Bank 0.
*	TW8836 has only 2K data memory. It is a really painfull solution.
*
*
* @param
* @param	type
*	- 1:Byte pointer - LUTS type
*	- 0:Address pointer - LUT type
* @param
* @param
* @param
* @return
*/
#if 0  //move to SPI.c and remove temp_SPI_Buffer[64] to save DataSegment.
BYTE temp_SPI_Buffer[64];	//only for SpiOsdIoLoadLUT.
void SpiOsdIoLoadLUT(BYTE _winno, BYTE type, WORD LutOffset, WORD size, DWORD spiaddr, alpha)
{
	BYTE i,j,k;
	BYTE R410_data;
#ifdef DEBUG_OSD
	dPrintf("\n\rSpiOsdIoLoadLUT%s LutLoc:%d size:%d 0x%06lx", type ? "S":" ", LutOffset, size, spiaddr);
#endif

#if defined(TW8836_CHIPDEBUG)
#else
//	McuSpiClkToPclk(CLKPLL_DIV_3P0);
#endif

	WriteTW88Page(PAGE4_SOSD );

	//--- SPI-OSD config
	if(type==LUTTYPE_ADDR)	R410_data = 0xC0;			// LUT Write Mode, En & address ptr inc.
	else					R410_data = 0xA0;			// LUT Write Mode, En & byte ptr inc.
	if(LutOffset >> 8)
		R410_data |= 0x08;   //BK130121 bugfix
	if(_winno==1 || _winno==2)
		R410_data |= 0x04;		

	if(type==LUTTYPE_ADDR) {
		//
		//ignore size. it is always 0x400.(256*4)
		//BKTODO130124. If it is a LUTB,...
		//		
		for(i=0; i < 4; i++) {	 
			WriteTW88(REG410, R410_data | i );	//assign byte ptr	
			WriteTW88(REG411, (BYTE)LutOffset);	//reset address ptr.
			for(j=0; j<(256/64);j++) {
				SpiFlashDmaRead2XMem(temp_SPI_Buffer,spiaddr + i*256 + j*64,64);	 //BUGBUG120606 BANK issue
				for(k=0; k < 64; k++) {
					WriteTW88(REG412, temp_SPI_Buffer[k]);		//write data
				}
			}
		}
	}
	else {
		WriteTW88(REG410, R410_data);			//assign byte ptr. always start from 0.
		WriteTW88(REG411, (BYTE)LutOffset);		//reset address ptr.

		for(i=0; i < (size / 64); i++ ) {		//min size is a 64(16*4)
			SpiFlashDmaRead2XMem(temp_SPI_Buffer,spiaddr + i*64,64);
			for(k=0; k < 64; k++) {
				WriteTW88(REG412, temp_SPI_Buffer[k]);		//write data
			}
		}
	}
	//pixel alpha
	if(alpha!=0xFF) {
		SpiOsdPixelAlphaAttr(_winno,LutOffset+alpha, 0x7F);	
	}



#if defined(TW8836_CHIPDEBUG)
#else
//	McuSpiClkRestore();
#endif
}
#endif

/*
@param input
	0:SOG Slicer
	1:HSYNC_IN - we are not using.
@param pol
	0: always 0. because we using it only for SOG Slicer.
SyncSeperator(BYTE input, BYTE pol)
{	}
*/

/*
LINUX TABLE
# 640x400 @ 85Hz (VESA) hsync: 37.9kHz
ModeLine "640x400"    31.5    640  672  736  832  400  401  404  445 -hsync +vsync
# 720x400 @ 85Hz (VESA) hsync: 37.9kHz
ModeLine "720x400"    35.5    720  756  828  936  400  401  404  446 -hsync +vsync

# 640x480 @ 60Hz (Industry standard) hsync: 31.5kHz
ModeLine "640x480"    25.2    640  656  752  800  480  490  492  525 -hsync -vsync
									16  112             10   12
# 640x480 @ 100Hz hsync: 50.9kHz
Modeline "640x480"    43.163  640  680  744  848  480  481  484  509 +hsync +vsync
# 640x480 @ 72Hz (VESA) hsync: 37.9kHz
ModeLine "640x480"    31.5    640  664  704  832  480  489  491  520 -hsync -vsync
# 640x480 @ 75Hz (VESA) hsync: 37.5kHz
ModeLine "640x480"    31.5    640  656  720  840  480  481  484  500 -hsync -vsync
# 640x480 @ 85Hz (VESA) hsync: 43.3kHz
ModeLine "640x480"    36.0    640  696  752  832  480  481  484  509 -hsync -vsync
# 768x576 @ 60 Hz (GTF) hsync: 35.82 kHz; pclk: 34.96 MHz
ModeLine "768x576"    34.96   768  792  872  976  576  577  580  597 -hsync +vsync
# 768x576 @ 72 Hz (GTF) hsync: 43.27 kHz; pclk: 42.93 MHz
ModeLine "768x576"    42.93   768  800  880  992  576  577  580  601 -hsync +vsync
# 768x576 @ 75 Hz (GTF) hsync: 45.15 kHz; pclk: 45.51 MHz
ModeLine "768x576"    45.51   768  808  888  1008 576  577  580  602 -hsync +vsync
# 768x576 @ 85 Hz (GTF) hsync: 51.42 kHz; pclk: 51.84 MHz
ModeLine "768x576"    51.84   768  808  888  1008 576  577  580  605 -hsync +vsync
# 768x576 @ 100 Hz (GTF) hsync: 61.10 kHz; pclk: 62.57 MHz
ModeLine "768x576"    62.57   768  816  896  1024  576 577  580  611 -hsync +vsync
# 800x600 @ 56Hz (VESA) hsync: 35.2kHz
ModeLine "800x600"    36.0    800  824  896 1024  600  601  603  625 +hsync +vsync
# 800x600 @ 60Hz (VESA) hsync: 37.9kHz
ModeLine "800x600"    40.0    800  840  968 1056  600  601  605  628 +hsync +vsync
# 800x600 @ 72Hz (VESA) hsync: 48.1kHz
ModeLine "800x600"    50.0    800  856  976 1040  600  637  643  666 +hsync +vsync
# 800x600 @ 75Hz (VESA) hsync: 46.9kHz
ModeLine "800x600"    49.5    800  816  896 1056  600  601  604  625 +hsync +vsync
# 800x600 @ 85Hz (VESA) hsync: 53.7kHz
ModeLine "800x600"    56.3    800  832  896 1048  600  601  604  631 +hsync +vsync
# 800x600 @ 100Hz hsync: 63.6kHz
Modeline "800x600"    68.179  800  848  936 1072  600  601  604  636 +hsync +vsync
# 1024x600 @ 60 Hz (GTF) hsync: 37.32 kHz; pclk: 48.96 MHz
ModeLine "1024x600"   48.96   1024 1064 1168 1312  600 601  604  622 -hsync +vsync
# 1024x768i @ 43Hz (industry standard) hsync: 35.5kHz
ModeLine "1024x768"   44.9   1024 1032 1208 1264  768  768  776  817 +hsync +vsync Interlace
# 1024x768 @ 60Hz (VESA) hsync: 48.4kHz
ModeLine "1024x768"   65.0   1024 1048 1184 1344  768  771  777  806 -hsync -vsync
# 1024x768 @ 70Hz (VESA) hsync: 56.5kHz
ModeLine "1024x768"   75.0   1024 1048 1184 1328  768  771  777  806 -hsync -vsync
# 1024x768 @ 75Hz (VESA) hsync: 60.0kHz
ModeLine "1024x768"   78.8   1024 1040 1136 1312  768  769  772  800 +hsync +vsync
# 1024x768 @ 85Hz (VESA) hsync: 68.7kHz
ModeLine "1024x768"   94.5   1024 1072 1168 1376  768  769  772  808 +hsync +vsync
# 1024x768 @ 100Hz hsync: 81.4kHz
Modeline "1024x768" 113.309  1024 1096 1208 1392  768  769  772  814 +hsync +vsync
# 1024x768 @ 120Hz hsync: 98.8kHz
Modeline "1024x768" 139.054  1024 1104 1216 1408  768  769  772  823 +hsync +vsync
# 1152x864 @ 60Hz hsync: 53.7kHz
Modeline "1152x864"  81.642  1152 1216 1336 1520  864  865  868  895 +hsync +vsync
# 1152x864 @ 75Hz (VESA) hsync: 67.5kHz
ModeLine "1152x864" 108.0    1152 1216 1344 1600  864  865  868  900 +hsync +vsync
# 1152x864 @ 85Hz hsync: 77kHz
Modeline "1152x864" 119.651  1152 1224 1352 1552  864  865  868  907 +hsync +vsync
# 1152x864 @ 100Hz hsync: 91.5kHz
Modeline "1152x864" 143.472  1152 1232 1360 1568  864  865  868  915 +hsync +vsync
# 1280x768 @ 60 Hz (GTF) hsync: 47.70 kHz; pclk: 80.14 MHz
ModeLine "1280x768"  80.14   1280 1344 1480 1680  768  769  772  795 -hsync +vsync
# 1280x800 @ 60 Hz (GTF) hsync: 49.68 kHz; pclk: 83.46 MHz
ModeLine "1280x800"  83.46   1280 1344 1480 1680  800  801  804  828 -hsync +vsync
# 1280x960 @ 60Hz (VESA) hsync: 60.0kHz
ModeLine "1280x960"  108.0   1280 1376 1488 1800  960  961  964 1000 +hsync +vsync
# 1280x960 @ 75Hz hsync: 75.1kHz
Modeline "1280x960" 129.859  1280 1368 1504 1728  960  961  964 1002 +hsync +vsync
# 1280x960 @ 85Hz (VESA) hsync: 85.9kHz
ModeLine "1280x960"  148.5   1280 1344 1504 1728  960  961  964 1011 +hsync +vsync
# 1280x960 @ 100Hz hsync: 101.7kHz
Modeline "1280x960" 178.992  1280 1376 1520 1760  960  961  964 1017 +hsync +vsync
# 1280x1024 @ 60Hz (VESA) hsync: 64.0kHz
ModeLine "1280x1024" 108.0   1280 1328 1440 1688 1024 1025 1028 1066 +hsync +vsync
# 1280x1024 @ 75Hz (VESA) hsync: 80.0kHz
ModeLine "1280x1024" 135.0   1280 1296 1440 1688 1024 1025 1028 1066 +hsync +vsync
# 1280x1024 @ 85Hz (VESA) hsync: 91.1kHz
ModeLine "1280x1024" 157.5   1280 1344 1504 1728 1024 1025 1028 1072 +hsync +vsync
# 1280x1024 @ 100Hz hsync: 108.5kHz
Modeline "1280x1024" 190.96  1280 1376 1520 1760 1024 1025 1028 1085 +hsync +vsync
# 1280x1024 @ 120Hz hsync: 131.6kHz
Modeline "1280x1024" 233.793 1280 1384 1528 1776 1024 1025 1028 1097 +hsync +vsync
# 1368x768 @ 60 Hz (GTF) hsync: 47.70 kHz; pclk: 85.86 MHz
ModeLine "1368x768"  85.86   1368 1440 1584 1800 768  769  772  795  -hsync +vsync
# 1400x1050 @ 60Hz M9 Laptop mode, hsync: 65kHz
ModeLine "1400x1050" 122.00  1400 1488 1640 1880 1050 1052 1064 1082 +hsync +vsync
# 1400x1050 @ 72 Hz (GTF) hsync: 78.77 kHz; pclk: 149.34 MHz
ModeLine "1400x1050" 149.34  1400 1496 1648 1896 1050 1051 1054 1094 -hsync +vsync
# 1400x1050 @ 75 Hz (GTF) hsync: 82.20 kHz; pclk: 155.85 MHz
ModeLine "1400x1050" 155.85  1400 1496 1648 1896 1050 1051 1054 1096 -hsync +vsync
# 1400x1050 @ 85 Hz (GTF) hsync: 93.76 kHz; pclk: 179.26 MHz
ModeLine "1400x1050" 179.26  1400 1504 1656 1912 1050 1051 1054 1103 -hsync +vsync
# 1400x1050 @ 100 Hz (GTF) hsync: 111.20 kHz; pclk: 214.39 MHz
ModeLine "1400x1050" 214.39  1400 1512 1664 1928 1050 1051 1054 1112 -hsync +vsync
# 1440x900 @ 60 Hz  (GTF) hsync: 55.92 kHz; pclk: 106.47 MHz
ModeLine "1440x900"  106.47  1440 1520 1672 1904 900  901  904  932  -hsync +vsync
# 1440x1050 @ 60 Hz (GTF) hsync: 65.22 kHz; pclk: 126.27 MHz
ModeLine "1440x1050" 126.27  1440 1536 1688 1936 1050 1051 1054 1087 -hsync +vsync
# 1600x1000 @ 60Hz hsync: 62.1kHz
Modeline "1600x1000" 133.142 1600 1704 1872 2144 1000 1001 1004 1035 +hsync +vsync
# 1600x1000 @ 75Hz hsync: 78.3kHz
Modeline "1600x1000" 169.128 1600 1704 1880 2160 1000 1001 1004 1044 +hsync +vsync
# 1600x1000 @ 85Hz hsync: 89.2kHz
Modeline "1600x1000" 194.202 1600 1712 1888 2176 1000 1001 1004 1050 +hsync +vsync
# 1600x1000 @ 100Hz hsync: 105.9kHz
Modeline "1600x1000" 232.133 1600 1720 1896 2192 1000 1001 1004 1059 +hsync +vsync
# 1600x1024 @ 60Hz hsync: 63.6kHz
Modeline "1600x1024" 136.385 1600 1704 1872 2144 1024 1027 1030 1060 +hsync +vsync
# 1600x1024 @ 75Hz hsync: 80.2kHz
Modeline "1600x1024" 174.416 1600 1712 1888 2176 1024 1025 1028 1069 +hsync +vsync
# 1600x1024 @ 76Hz hsync: 81.3kHz
Modeline "1600x1024" 170.450 1600 1632 1792 2096 1024 1027 1030 1070 +hsync +vsync
# 1600x1024 @ 85Hz hsync: 91.4kHz
Modeline "1600x1024" 198.832 1600 1712 1888 2176 1024 1027 1030 1075 +hsync +vsync
# 1600x1200 @ 60Hz (VESA) hsync: 75.0kHz
ModeLine "1600x1200" 162.0   1600 1664 1856 2160 1200 1201 1204 1250 +hsync +vsync
# 1600x1200 @ 65Hz (VESA) hsync: 81.3kHz
ModeLine "1600x1200" 175.5   1600 1664 1856 2160 1200 1201 1204 1250 +hsync +vsync
# 1600x1200 @ 70Hz (VESA) hsync: 87.5kHz
ModeLine "1600x1200" 189.0   1600 1664 1856 2160 1200 1201 1204 1250 +hsync +vsync
# 1600x1200 @ 75Hz (VESA) hsync: 93.8kHz
ModeLine "1600x1200" 202.5   1600 1664 1856 2160 1200 1201 1204 1250 +hsync +vsync
# 1600x1200 @ 85Hz (VESA) hsync: 106.3kHz
ModeLine "1600x1200" 229.5   1600 1664 1856 2160 1200 1201 1204 1250 +hsync +vsync
# 1600x1200 @ 100 Hz (GTF) hsync: 127.10 kHz; pclk: 280.64 MHz
ModeLine "1600x1200" 280.64  1600 1728 1904 2208 1200 1201 1204 1271 -hsync +vsync
# 1680x1050 @ 60 Hz  (GTF) hsync: 65.22 kHz; pclk: 147.14 MHz
ModeLine "1680x1050" 147.14  1680 1784 1968 2256 1050 1051 1054 1087 -hsync +vsync
# 1792x1344 @ 60Hz (VESA) hsync: 83.6kHz
ModeLine "1792x1344" 204.8   1792 1920 2120 2448 1344 1345 1348 1394 -hsync +vsync
# 1792x1344 @ 75Hz (VESA) hsync: 106.3kHz
ModeLine "1792x1344" 261.0   1792 1888 2104 2456 1344 1345 1348 1417 -hsync +vsync
# 1800x1440 (unchecked)
ModeLine "1800x1440" 230     1800 1896 2088 2392 1440 1441 1444 1490 +hsync +vsync
# 1856x1392 @ 60Hz (VESA) hsync: 86.3kHz
ModeLine "1856x1392" 218.3   1856 1952 2176 2528 1392 1393 1396 1439 -hsync +vsync
# 1856x1392 @ 75Hz (VESA) hsync: 112.5kHz
ModeLine "1856x1392" 288.0   1856 1984 2208 2560 1392 1393 1396 1500 -hsync +vsync
# 1920x1080 @ 60Hz hsync: 67.1kHz
Modeline "1920x1080" 172.798 1920 2040 2248 2576 1080 1081 1084 1118 -hsync -vsync
# 1920x1080 @ 75Hz hsync: 81.1kHz
Modeline "1920x1080" 211.436 1920 2056 2264 2608 1080 1081 1084 1126 +hsync +vsync
# 1920x1200 @ 60Hz hsync: 74.5kHz
Modeline "1920x1200" 193.156 1920 2048 2256 2592 1200 1201 1203 1242 +hsync +vsync
# 1920x1200 @ 75Hz hsync: 94kHz
Modeline "1920x1200" 246.590 1920 2064 2272 2624 1200 1201 1203 1253 +hsync +vsync
# 1920x1440 @ 60Hz (VESA) hsync: 90.0kHz
ModeLine "1920x1440" 234.0   1920 2048 2256 2600 1440 1441 1444 1500 -hsync +vsync
# 1920x1440 @ 75Hz (VESA) hsync: 112.5kHz
ModeLine "1920x1440" 297.0   1920 2064 2288 2640 1440 1441 1444 1500 -hsync +vsync
# 1920x2400 @ 25Hz for IBM T221, VS VP2290 and compatible, hsync: 60.8kHz
Modeline "1920x2400" 124.62  1920 1928 1980 2048 2400 2401 2403 2434 +hsync +vsync
# 1920x2400 @ 30Hz for IBM T221, VS VP2290 and compatible, hsync: 73.0kHz
Modeline "1920x2400" 149.25  1920 1928 1982 2044 2400 2402 2404 2434 +hsync +vsync
# 2048x1536 @ 60Hz hsync: 95.3kHz
Modeline "2048x1536" 266.952 2048 2200 2424 2800 1536 1537 1540 1589 +hsync +vsync
*/
//NeedClock ?
//LLPLL
//PCLKO
//-----------------------------------------------------------------------------
/**
* set High speed clock. only for test.
*/
#if 0
//void ClockHigh(void)
//{
//	dPrintf("\n\rHigh");	
//
//	WriteTW88Page(PAGE0_SSPLL );
//	WriteTW88(REG0F6, 0x00 );	// PCLK div by 1
//
//	WriteTW88Page(PAGE2_SCALER );
//	WriteTW88(REG20D, 0x81 );	// PCLKO div by 2
//
//	WriteTW88Page(PAGE4_CLOCK);
//	WriteTW88(REG4E1, 0xe0 );	// Source=PCLK, Delay=1, Edge=1
//	PllClkSetSpiInputClockLatch(3);	//just for test.
//
//	SPI_SetReadModeByRegister(0x05);	// SPI mode QuadIO, Match DMA mode with SPI-read
//}

//-----------------------------------------------------------------------------
/**
* set Low speed clock. only for test.
*/
//void ClockLow(void)
//{
//	dPrintf("\n\rLow");	
//
//	WriteTW88Page(PAGE0_SSPLL );
//	WriteTW88(REG0F6, 0x00 );	// PCLK div by 1
//
//	WriteTW88Page(PAGE2_SCALER );
//	WriteTW88(REG20D, 0x80 );	// PCLKO div by 1
//
//	WriteTW88Page(PAGE4_CLOCK);
//	WriteTW88(REG4E1, 0x20 );	// Source=PCLK, Delay=0, Edge=0
//
//	SPI_SetReadModeByRegister(0x05);	// SPI mode QuadIO, Match DMA mode with SPI-read
//}

//-----------------------------------------------------------------------------
/**
* set 27MHz clock. only for test.
*/
//void Clock27(void)
//{
//	dPrintf("\n\r27MHz");	
//
//	WriteTW88Page(PAGE0_SSPLL );
//	WriteTW88(REG0F6, 0x00 );	// PCLK div by 1
//
//	WriteTW88Page(PAGE2_SCALER );
//	WriteTW88(REG20D, 0x80 );	// PCLKO div by 1
//
//	WriteTW88Page(PAGE4_CLOCK);
//	WriteTW88(REG4E1, 0x00 );	// Source=27M
//
//	SPI_SetReadModeByRegister(0x05);	// SPI mode QuadIO, Match DMA mode with SPI-read
//}
#endif

#ifdef UNCALLED_SEGMENT
//---------------------------------------
//R1C2[5:4] - VCO range
//				0 = 5  ~ 27MHz
//				1 = 10 ~ 54MHz
//				2 = 20 ~ 108MHz
//				3 = 40 ~ 216MHz
//R1C2[2:0] - Charge pump
//				0 = 1.5uA
//				1 = 2.5uA
//				2 = 5.0uA
//				3 = 10uA
//				4 = 20uA
//				5 = 40uA
//				6 = 80uA
//				7 = 160uA
//----------------------------------------

//-----------------------------------------------------------------------------
//parmeter
//	_IPF: Input PixelClock Frequency= Htotal*Vtotal*Hz
//
//
//BYTE SetVCORange(DWORD _IPF)
//
//need pixel clock & POST divider value.
//
//pixel clock = Htotal * Vtotal * Freq
//HFreq = Vtotal * Freq
//pixel clock = Htotal * HFreq
//
//-----------------------------------------------------------------------------
//==>LLPLLSetVcoRange
BYTE aRGB_SetVcoRange(DWORD _IPF)
{
	BYTE VCO_CURR, value, chged=0;
	WORD val;
	
	val = _IPF / 1000000L;

	dPrintf("\n\raRGB_SetVcoRange _IPF:%lx val:%dMHz",_IPF,val);  
												//   +------BUG
												//    ??pump value		
	if     ( val < 15 )		VCO_CURR = 0x01;	// 00 000
	else if( val < 34 )		VCO_CURR = 0x01;	// 00 000
	else if( val < 45 )		VCO_CURR = 0x11;	// 01 000
	else if( val < 63 )		VCO_CURR = 0x11;	// 01 000
	else if( val < 70 )		VCO_CURR = 0x21;	// 10 000
	else if( val < 80 )		VCO_CURR = 0x21;	// 10 000
	else if( val <100 )		VCO_CURR = 0x21;	// 10 000
	else if( val <110 )		VCO_CURR = 0x21;	// 10 000
	else					VCO_CURR = 0x31;	// 11 000
	VCO_CURR |= 0xC0;	//POST div 1
	
	WriteTW88Page(PAGE1_VADC);
	value = ReadTW88(REG1C2);
	if( VCO_CURR != value) {
		chged = 1;
		dPrintf(" R1C2:%bx->%bx", value, VCO_CURR );
		WriteTW88(REG1C2, VCO_CURR);			// VADC_VCOCURR
		delay1ms(1);					// time to stabilize
	}


//	#ifdef DEBUG_PC_MEAS
//	dPrintf("\r\nSetVCO=%02bx, changed=%bd", VCO_CURR, chged );
//	#endif
	return chged;
}
#endif

#if defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
/**
* Read HSync&VSync input polarity status register.
* And, Set LLPLL input polarity & VSYNC output polarity.
*
*	Read R1C1[6] and set R1C0[2]   hPol
*	Read R1C1[7] and set R1CC[1]   vPol
*
*	PC uses aRGB_SetPolarity(0) and Component uses aRGB_SetPolarity(1).
*
* register
*	R1C0[2]	- LLPLL input polarity. Need Negative. CA_PAS need a normal
*	R1C1[6]	- HSync input polarity
*	R1C1[7] - VSync input polarity 
*	R1CC[1] - VSYNC output polarity. Need Positive

* ==>ARGBSetPolarity
* othername PolarityAdjust
*
* @param
*	fUseCAPAS.	If "1", R1C0[2] always use 0.
*				component use fUseCAPAS=1. 
*
*/
#if 0
void aRGB_SetPolarity(BYTE fUseCAPAS)
{
	volatile BYTE r1c1;
	int i;

	WriteTW88Page(PAGE1_VADC );
	for(i=0; i < 5; i++) {
		r1c1 = ReadTW88(REG1C1);
		Printf("\n\raRGB_SetPolarity hPol:%bx vPol:%bx", r1c1 & 0x40 ? 1:0, r1c1 & 0x80 ? 1:0);
		delay1ms(10);
	}
	if(fUseCAPAS) {
		//CA_PAS need a normal
		WriteTW88(REG1C0, ReadTW88(REG1C0) & ~0x04);
		WriteTW88(REG1CC, ReadTW88(REG1CC) & ~0x02);	//if active high, no inv.
	}
	else {
		//check HS_POL.		Make LLPLL input polarity Negative.
		//LLPLL request an active low.
		//Meas prefers an active high.
		if(r1c1 & 0x40) WriteTW88(REG1C0, ReadTW88(REG1C0) | 0x04);		//if active high, ((invert)). make active low.
		else			WriteTW88(REG1C0, ReadTW88(REG1C0) & ~0x04);	//if active low, ((normal)). keep active low.
		//check VS_POL.		Make VS output polarity Positive.
		//Meas prefers an active high.
		if(r1c1 & 0x80) WriteTW88(REG1CC, ReadTW88(REG1CC) & ~0x02);	//if active high, normal.
		else			WriteTW88(REG1CC, ReadTW88(REG1CC) | 0x02);		//if active low, invert.
	}
}
#endif
#endif
#ifdef UNCALLED_SEGMENT
//-----------------------------------------------------------------------------
//desc: check input
//@param
//	type	0:YPbPr, 1:PC
//		YPbPr	Use CompositeSync(CSYNC) with clamping & Slicing
//		PC		Use a seperate HSYNC & VSYNC PIN
//
//for YPbPr
//	0x1C0[7:6]	= 0		default
//	0x1C0[3] = 0		Select Clamping output(not HSYNC)
//	0x1C0[4] = 1		Select CS_PAS
//
//for PC(aRGB)
//preprocess
//	0x1C0[3] = 1	Select HSYNC
//	0x1C0[4] = 0	Select HSYNC(or Slice, Not a CS_PAS)
//	
//detect
//	0x1C1[6]	Detected HSYNC polarity
//	0x1C1[4]	HSYNC detect status
//postprocess
//	0x1C0[2]	PLL reference input polarity	
//
//return
//	0: fail
//	else: R1C1 value		
BYTE aRGB_CheckInput(BYTE type)
{
//	BYTE value;
	volatile BYTE rvalue;
	BYTE check;
	BYTE i;

	ePrintf("\n\raRGB_CheckInput(%bx) %s",type,type ? "PC": "YPbPr" );

	WriteTW88Page(PAGE1_VADC );

//	//power up PLL, SOG,....
//	value = 0x40;										// powerup PLL
//	if(type==0)	value |= 0x80;							// powerup SOG
//	WriteTW88(REG1CB, (ReadTW88(REG1CB) & 0x1F) | value );	// keep SOG Slicer threshold & coast
		
	if(type==0) check = 0x08;	//check CompositeSynch
	else 		check = 0x30;	//check HSynch & VSynch

	//(YPbPr need more then 370ms, PC need 200ms). max 500ms wait
	for(i=0; i < 50; i++) {
		rvalue = TW8835_R1C1;
		dPrintf(" %02bx",rvalue);

		if((rvalue & check) == check) {
			ePrintf("->success:%bd",i);
			return rvalue;	
		}
		delay1ms(10);
	}
	ePrintf("->fail");

//	WriteTW88(REG1CB, ReadTW88(REG1CB) & 0x1F);	//PowerDown

	return 0;	//No detect		
}
#endif


//-----------------------------------------------------------------------------
//only for test
//-----------------------------------------------------------------------------
#ifdef UNCALLED_SEGMENT
BYTE aRGB_DoubleCheckInput(BYTE detected)
{
	BYTE i, count;
	BYTE rvalue;
	BYTE old = detected;

	ePrintf("\n\raRGB_DoubleCheckInput");
	count=0;

	WriteTW88Page(PAGE1_VADC );
	for(i=0; i < 200; i++) {
		rvalue = ReadTW88(REG1C1 );
		if(rvalue == old) count++;
		else {
			dPrintf(" %02bx@%bd", rvalue, i);
			old = rvalue;
			count=0;
		}
		if(count >= 30) {
			ePrintf("->success");
			return rvalue;
		}
		delay1ms(10);
	}
	ePrintf("->fail");
	return 0;
}
#endif


//0x1C1 - LLPLL Input Detection Register 



#ifdef UNCALLED_SEGMENT
#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
//power down SOG,PLL,Coast
//register
//	R1CB[7]	SOG power down.	1=Powerup
//	R1CB[6]	PLL power down.	1=Powerup
//	R1CB[5]	PLL coast function. 1=Enable
//-----------------------------------------------------------------------------
void aRGB_SetPowerDown(void)
{
	WriteTW88Page(PAGE1_VADC );
	WriteTW88(REG1CB, (ReadTW88(REG1CB) & 0x1F));	
}
#endif
#endif

//-----------------------------------------------------------------------------
//register
//	R1CB[4:0]  SOG Slicer Threshold Register
//-----------------------------------------------------------------------------
#ifdef UNCALLED_SEGMENT
void aRGB_SetSOGThreshold(BYTE value)
{
	WriteTW88Page(PAGE1_VADC );
	WriteTW88(REG1CB, ReadTW88(REG1CB) & ~0x1F | value);	
}
#endif

//-----------------------------------------------------------------------------
//register
//	R1D4[7:0]
//-----------------------------------------------------------------------------
#if 0
void aRGB_SetClampMode(BYTE value)
{
	WriteTW88Page(PAGE1_VADC );
	WriteTW88(REG1D4, value );
}
#endif
//-----------------------------------------------------------------------------
//register
//	R1E6[5]	PGA control	0=low speed operation. 1=high speed operation
//-----------------------------------------------------------------------------
#ifdef UNCALLED_SEGMENT
/**
* set PGA control
*/
void aRGB_SetPGAControl(BYTE fHigh)
{
	WriteTW88Page(PAGE1_VADC );
	if(fHigh)	WriteTW88(REG1E6, ReadTW88(REG1E6) | 0x20);		//HighSpeed
	else		WriteTW88(REG1E6, ReadTW88(REG1E6) & ~0x20);	//LowSpeed
}
#endif
#ifdef SUPPORT_PC
//-----------------------------------------------------------------------------
/**
* set inputcrop for PC
*
* output
*	RGB_hStart
*/
#if 0
static void PCSetInputCrop( BYTE mode )
{
	BYTE hPol, vPol;
	WORD hStart, vStart;
	BYTE bTemp;
	WORD SyncWidth;
	WORD Meas_hActive,Meas_vActive;
	WORD Meas_Start;

	bTemp = ReadTW88(REG041);
	if(bTemp & 0x08) 	vPol = 1;	//0:rising edge, 1:falling edge
	else				vPol = 0;
	if(bTemp & 0x04) 	hPol = 1;	//0:rising edge, 1:falling edge
	else				hPol = 0;

	dPrintf("\n\rPCSetInputCrop offset:%bd, HPol: %bd, VPol: %bd", hPol, vPol );

	//read sync width.
	//HSyncWidth comes from .., not real value.
	//Meas_HPulse = MeasGetHSyncRiseToFallWidth();
	//Meas_VPulse = MeasGetVSyncRiseToFallWidth();
	//dPrintf("\n\r\tHPulse: %d, VPulse: %d", Meas_HPulse, Meas_VPulse );



	//meas uses active high sync, and use a rising edge.
	//if you give a active low sync to meas, we will have a wrong result.
	//meas has an internal delay.
	//PC has 5 offset on horizontal.
	//       1 offset on vertical.
	//if you use a falling edge in inputcrop, you have to remove SyncWidth.
	Meas_hActive = MeasGetHActive( &Meas_Start );
	hStart = Meas_Start +5;
	if(hPol) {
		SyncWidth = MeasGetHSyncRiseToFallWidth();
		hStart -= SyncWidth;	
	}

	Meas_vActive = MeasGetVActive( &Meas_Start );
	vStart = Meas_Start +1;
	if(vPol) {
		SyncWidth = MeasGetVSyncRiseToFallWidth();
		vStart -= SyncWidth;
	}

	RGB_HSTART = hStart;	//save for IE.

	//adjust EEPROM. 0..100. base 50. reversed value.
	bTemp = GetHActiveEE(mode); //PcBasePosH;
	if(bTemp != 50) {
		hStart += 50;
		hStart -= bTemp;
		dPrintf("\n\r\tModified HS:%d->%d, VS:%d", RGB_HSTART, hStart, vStart );
	}
	dPrintf("\n\r\tHLen: %d, VLen: %d", Meas_hActive, Meas_vActive );

//#ifdef MODEL_TW8835
//	InputSetCrop(hstart, 1, Meas_hActive, 0x7FE);
//#else
	InputSetCrop(hStart, vStart, Meas_hActive, Meas_vActive);
//#endif
}
#endif
#endif //#ifdef SUPPORT_PC
#ifdef SUPPORT_PC
//-----------------------------------------------------------------------------
/** 
* check PC mode
*
* @return
*	0xFF:fail
*	other: pc mode value
*/
#if 0
static BYTE PCCheckMode(void)
{
	BYTE i, mode;
	WORD VTotal;	//dummy

	for(i=0; i < 10; i++) {
		if(MeasStartMeasure())
			return 0;

 		mode = FindInputModePC(&VTotal);	// find input mode from Vfreq and VPeriod
		if(mode!=0xFF) {
			dPrintf("\n\rPCCheckMode ret:%bd",mode);
			return mode;
		}
	}

	return 0xFF;
}
#endif
#endif
#if 0 //defined(SUPPORT_COMP) ...not yet working..
//BYTE CheckAndSetComponent( void )
//{
//	BYTE	i;
//	BYTE	mode, new_mode;
//	BYTE ret;
//	WORD temp16;
//	DWORD temp32;
//	volatile BYTE InputStatus;
//
//	DECLARE_LOCAL_page
//	ReadTW88Page(page);
//
//	Input_aRGBMode = 0;		//BK111012
//
//	//wait..
//	for(i=0; i < 2; i++) {
//		ret=MeasStartMeasure();
//		if(ret==ERR_SUCCESS)
//			break;
//		delay1ms(10);
//	}	
//	if(ret)	{
//#ifdef DEBUG_COMP
//		Printf("\n\rCheckAndSetCOMP fail 1");
//#endif
//		WriteTW88Page(page);
//		return ERR_FAIL;
//	}
//
//	//Init LLPLL.
//	//	if input changes from 720P to low resulution, vtotal has a small value.
//	WriteTW88(REG1CD, ReadTW88(REG1CD) | 0x01);
//
//
//	//Find input mode
//	//	use vFreq and vPeriod.
//	mode = FindInputModeCOMP();	
//	if(mode==0) {
//#ifdef DEBUG_COMP
//		Printf("\n\rCheckAndSetCOMP fail 2. No proper mode");
//#endif
//		WriteTW88Page(page );
//		return ERR_FAIL;
//	}
//
//	//
//	//set LLPLL	& wait
//	//
//	//BKFYI. aRGB_LLPLLUpdateDivider(, 1,) has a MeasStartMeasure().
//	//
//	aRGB_SetLLPLLControl(0xF2);	// POST[7:6]= 3 -> div 1, VCO: 40~216, Charge Pump: 5uA
//	ret = aRGB_LLPLLUpdateDivider(YUVDividerPLL[ mode-1 ] - 1, 1, 0 );
//	if(ret==ERR_FAIL) {
//		WriteTW88Page(page );
//		return ERR_FAIL;
//	}		
//	//LLPLL needs a time until it becomes a stable state.
//	//TW8836 needs 110ms delay to get the correct vPol on PC.	How about COMP ?
//	//wait a detection flag.
//	for(i=0; i < 50; i++) {
//		InputStatus = ReadTW88(REG1C1);
//		if(InputStatus & 0x08) {
//			if((InputStatus & 0x07) != 0x07)
//				break;
//		}
//		delay1ms(10);
//	}
//#ifdef DEBUG_COMP
//	Printf("\n\rstep %bd mode:%bx",i, InputStatus & 0x0F); //
//#endif
//
//
//	//find input mode and compare it is same or not.
//	//if mode!=new_mode, just use mode..
//	new_mode = FindInputModeCOMP();	
//	if(mode!=new_mode) {
//		Printf("\n\rWARNING mode curr:%bd new:%bd",mode, new_mode);
//	}
//
//	//now adjust mode.
//	mode--;
//	dPrintf("\n\rFind YUV mode: %bd %dx%d@", mode, YUVCropH[mode], YUVCropV[mode] );
//
//	Input_aRGBMode = mode + EE_YUVDATA_START;
//	InputSubMode = ConvertComponentMode2HW(mode);
//
//	InitComponentReg(mode);
//
//	aRGB_SetClampModeHSyncEdge(ON);					//R1D4[5]
//	aRGB_SetClampPosition(YUVClampPos[mode]);
//
//	MeasSetErrTolerance(4);							//tolerance set to 32
//	MeasEnableChangedDetection(ON);					// set EN. Changed Detection
//
//
//	//BKTODO130208: assign correct aRGB_output_polarity and remove this routine.
//	//check VPulse & adjust polarity
//	temp16 = MeasGetVSyncRiseToFallWidth();
//	if(temp16 > YUVDisplayV[mode]) {
//		dPrintf("\n\rVSyncWidth:%d", temp16);
//		WriteTW88Page(PAGE1_VADC );		
//		WriteTW88(REG1CC, ReadTW88(REG1CC) | 0x02);	
//
//		MeasStartMeasure();
//		temp16 = MeasGetVSyncRiseToFallWidth();
//		dPrintf("=>%d", temp16);		
//	}
//
//	//TW8836 needs input crop value on 480i & 576i.
//	InputSetCrop(YUVStartH[mode], MYStartV[mode], YUVDisplayH[mode], YUVCropV[mode]);
//
//	YUVSetOutput(mode);
//
//	Sspll1SetFreqReg(SSPLL_108M_REG);	//108MHz. Where is a POST value ?
//	YUV_PrepareInfoString(mode);
//
//	//BKTODO130208. It is a debug routine. Please remove..
//	//
//	//check HStart
//	MeasStartMeasure();
//	temp16 = MeasGetHActive2();
//	dPrintf("\n\r**measure:%d",temp16);
//	if(mode < 4) { //SDTV or EDTV
//	}
//	else {			//HDTV
//		temp16 = MYStartH[mode] + temp16;
//	}
//	temp16 -=16;	//HWidth
//	temp16 += 3;
//	temp16 += ((YUVCropH[mode] - YUVDisplayH[mode]) / 2);
//	dPrintf("\n\r**HStart:%d suggest:%d",YUVStartH[mode],temp16);	
//
//	//
//	//check VDE
//	if(mode < 4) { //SDTV or EDTV
//		temp16 = MeasGetVActive2();
//		dPrintf("\n\r**measure:%d",temp16);
//		//temp16 += 0.5;
//		temp16 -= MeasGetVSyncRiseToFallWidth();  //if use faling.
//		temp16 += ((YUVCropV[mode] - YUVDisplayV[mode]) / 2);
//		temp32 = temp16;
//		temp32 *= PANEL_V;
//		temp32 /= YUVDisplayV[mode];
//		dPrintf("\n\r**VDE:%bd suggest:%d",YUVOffsetV[mode],(WORD)temp32);
//			
//		temp16 = MeasGetVActive2();
//		temp16 += 1;	//NOTE
//		temp16 -= MeasGetVSyncRiseToFallWidth(); //if use faling.
//		temp16 += ((YUVCropV[mode] - YUVDisplayV[mode]) / 2);
//		temp32 = temp16;
//		temp32 *= PANEL_V;
//		temp32 /= YUVDisplayV[mode];
//		dPrintf("~%d",(WORD)temp32);
//	}
//	else {			//HDTV
//		temp16 = MYStartV[mode];
//		//temp16 += 0.5;
//		temp16 -= MeasGetVSyncRiseToFallWidth();
//		temp16 += ((YUVCropV[mode] - YUVDisplayV[mode]) / 2);
//		temp32 = temp16;
//		temp32 *= PANEL_V;
//		temp32 /= YUVDisplayV[mode];
//		dPrintf("\n\r**VDE:%bd suggest:%d",YUVOffsetV[mode],(WORD)temp32);	
//		temp16 = MYStartV[mode];
//		temp16 += 1;   	//NOTE
//		temp16 -= MeasGetVSyncRiseToFallWidth();
//		temp16 += ((YUVCropV[mode] - YUVDisplayV[mode]) / 2);
//		temp32 = temp16;
//		temp32 *= PANEL_V;
//		temp32 /= YUVDisplayV[mode];
//		dPrintf("~%d",(WORD)temp32);	
//	}
//	
//
//	//TW8836 uses "1" on 480i & 576i also.
//	InputSetFieldPolarity(1);
//
//	WriteTW88Page(page );
//	return ERR_SUCCESS;
//}
#endif
#if 0
//BYTE CheckAndSetPC_________________OLD(void)
//{
//	BYTE mode,new_mode;
//  
//#ifdef CHECK_USEDTIME
//	DWORD UsedTime;
//#endif
//	BYTE value;
//	BYTE value1;
//	WORD new_VTotal;
//	WORD wTemp;
//	BYTE ret;
//	DECLARE_LOCAL_page
//
//
//#ifdef CHECK_USEDTIME
//	UsedTime = SystemClock;
//#endif
//	Input_aRGBMode = 0;
//	InputSubMode = Input_aRGBMode;
//
//	do {
//		mode = PCCheckMode();
//		if(mode==0) {
//			WriteTW88Page(page );
//			return ERR_FAIL;
//		}
//
//		//
//		//set LLPLL	& wait
//		//
//		//BK110927
//		//BK110928 assume LoopGain:2.
//		//LLPLL divider:PCMDATA[ mode ].htotal - 1
//		//ControlValue, 0xF2.  POST[7:6]= 3 -> div 1, VCO: 40~216, Charge Pump: 5uA
//		//LLPLL init: ON
//		//Wait delay for WaitStableLLPLL: 40ms
//		//LLPLL Control.
//		aRGB_SetLLPLLControl(0xF2);	// POST[7:6]= 3 -> div 1, VCO: 40~216, Charge Pump: 5uA
//		ret = aRGB_LLPLLUpdateDivider(PCMDATA[ mode ].htotal - 1, 1, 40 );
//		if(ret==ERR_FAIL) {
//			WriteTW88Page(page );
//			return ERR_FAIL;
//		}
//		//TW8836 needs 110ms delay to get the correct vPol.
//		delay1ms(120);
//		//WaitStableLLPLL(120);
// 		aRGB_SetPolarity(0);
//
//		//check Phase EEPROM.
//		value = GetPhaseEE(mode);
//		if(value!=0xFF) {
//			dPrintf("\n\ruse Phase 0x%bx",value);
//			//we read first. because update routine can make a shaking.
//			value1=aRGB_GetPhase();
//			if(value != value1) {
//				dPrintf("  update from 0x%bx",value1);
//				aRGB_SetPhase(value, 0);	//BKTODO? Why it does not have a init ?
//				if(WaitStableLLPLL(0)) {
//					WriteTW88Page(page );
//					return ERR_FAIL;
//				}
//			}
//			else {
//				WaitStableLLPLL(0); //BK110830
//				MeasCheckVPulse();
//			}
//		}
//		else 
//		{
//			AutoTunePhase();
//			value=aRGB_GetPhase();
//			dPrintf("\n\rcalculate Phase %bx",value);
//			SavePhaseEE(mode,value);
//
//			if(WaitStableLLPLL(0)) {
//				WriteTW88Page(page );
//				return ERR_FAIL;
//			}
//		}
//		//adjust polarity again and update all measured value
//		aRGB_SetPolarity(0);
//		MeasStartMeasure();
//
//		MeasCheckVPulse(); //BK130122
//
//
//		//use measured value.  
//		MeasVLen = MeasGetVActive( &MeasVStart );				//v_active_start v_active_perios
//		MeasHLen = MeasGetHActive( &MeasHStart );				//h_active_start h_active_perios
//
//		dPrintf("\n\rMeasure Value Start %dx%d Len %dx%d", MeasHStart,MeasVStart, MeasHLen,MeasVLen);
//
//		if ( MeasVLen < PCMDATA[ mode ].van ) {			// use table
//			MeasVStart = PCMDATA[mode].vstart;
//			MeasVLen = PCMDATA[mode].van;
//			dPrintf("->VS:%d VLen:%d",MeasVStart,MeasVLen);
//		}
//		if ( MeasHLen < PCMDATA[ mode ].han ) {			// use table
//			MeasHStart = PCMDATA[mode].hstart;
//			MeasHLen = PCMDATA[mode].han;
//			dPrintf("->HS:%d HLen:%d",MeasHStart,MeasHLen);
//		}
//
//		PCSetInputCrop(mode);
//		PCSetOutput(mode);
//
//		new_mode = FindInputModePC(&new_VTotal);
//	} while(mode != new_mode);
//
//	Input_aRGBMode = mode;
//	InputSubMode = Input_aRGBMode;
//
//	//EE
//	//PCLKAdjust();
//	AdjustPixelClk(0, mode); //BK120117 need a divider value
//
//	//adjust PCPixelClock here.
//	//If R1C4[], measure block use a wrong value.
//	wTemp = PCMDATA[ mode ].htotal - 1;
//	dPrintf("\n\rPixelClk %d",wTemp);
//	wTemp += GetPixelClkEE(mode);	//0..100. default:50
//	wTemp -= 50;
//	dPrintf("->%d EEPROM:%bd",wTemp,GetPixelClkEE(mode));
//
//	aRGB_LLPLLUpdateDivider(wTemp, OFF, 0);	//without init. Do you need aRGB_SetLLPLLControl(0xF2) ?
//	
//	MeasSetErrTolerance(4);						//tolerance set to 32
//	MeasEnableChangedDetection(ON);				//set EN. Changed Detection
//	
//	PC_PrepareInfoString(mode);
//
//#ifdef CHECK_USEDTIME
//	UsedTime = SystemClock - UsedTime;
//	Printf("\n\rUsedTime:%ld.%ldsec", UsedTime/100, UsedTime%100 );
//#endif
//			
//	WriteTW88Page(page );
//  
//	return ERR_SUCCESS;
//}
#endif
//check routine
#if defined(SUPPORT_COMP) || defined(SUPPORT_PC)
#if 0
BYTE Check_aRGB(BYTE mode)
{
	struct VESA_VIDEO_TIME_TABLE_s *pTimeTable;
	pTimeTable = &TW8836_VESA_TABLE[mode];
	//check AnaliogMux

	//check Video Status

	//check aRGB input status
	InputStatus = ReadTW88(REG1C1);
	if(InputMain == INPUT_COMP) {
		if((InputStatus & 0x04) == 0) {
			Printf("CSync");
			return ERR_ERROR;
		}
		if(mode != (InputStatus & 0x07)) {
			Printf("mode");
			return ERR_ERROR;
		}

	}
	else {
		if((InputStatus & 0x30) != 0x30) {
			Printf("Sync");
			return ERR_ERROR;
		}
	}
	
	//check aRGB path

	//check aRGB polarity
	if(InputMain == INPUT_COMP) {
	}
	else {
		pol = 0;
		if(InputStatus & 0x80) pol |= 0x01;	//vPol
		if(InputStatus & 0x40) pol |= 0x10;	//hPol
		if(pTimeTable->pol != pol) {
			Puts("Input Pol");
			return ERR_ERROR;
		}
		if(hPol != LLPLL_InputPol) {
			Puts("LLPLL_ipol");
			return ERR_ERROR	
		}
		if(vPol != Output_vPol) {
			Puts("vPol");
			return ERR_ERROR	
		}
	}
	
	//check meased polarity
	
	
	
		 
	return ERR_SUCCESS;
}
#endif
#endif

//parameter InputMain value or BT565...
/**
* desc: Select BT656 MUX
* @param 
*		BT656ENC_SRC_DEC
*		BT656ENC_SRC_ARGB
*		BT656ENC_SRC_DTV
*		BT656ENC_SRC_LVDS
*		BT656ENC_SRC_PANEL
*		BT656ENC_SRC_OFF
*
*		BT656ENC_SRC_AUTO can not use.
*
* assume, TW8836 already has a correct InMux path.
*
* set BT656 mux. 
*/
// call from FOsdMenu & monitor "BT656" command.
// It will be removed.....
#if 0 //it will be removed
void SetBT656Output(BYTE bt656_src_mode)
{
	BYTE temp = bt656_src_mode;
 	BYTE HPol,VPol;
	BYTE p;
	WORD h_SAV,v_SAV,HLen,VLen;
	WORD hDelay,vDelay,hActive,vActive;

	BYTE bTemp;
	WORD wTemp;
	BYTE failed;
	//BYTE OLD_InputMain;
	//BYTE bt656_src_mode;
	BYTE mode;

	Printf("\n\rSetBT656Output(%bd)",bt656_src_mode);

	BT656_Enable(OFF);

	//recover some test or temp value.
//	PclkoSetDiv(2 /* div 3 = 108 / 3 = 36MHz */);		//panel use 27MHz. I just want to recover before I start.
	WriteTW88(REG068, ReadTW88(REG068) & ~0x80);		//clear "use Input Field"
	WriteTW88(REG303, (ReadTW88(REG303) + 1) & 0xFE);	//EVEN FOSD DE.

	if(bt656_src_mode == BT656ENC_SRC_OFF)
		return;

	//check video input. Do not change video mux.
	failed=0;
	switch(bt656_src_mode) {
	case BT656ENC_SRC_DEC:	//DEC
		WriteTW88Page(0);
		mode = ReadTW88(REG101);		//read Chip Status
		if (mode & 0x80) {
			//video loss
			failed = 1;
			break;
		}
		mode >>= 4;
		BT656_A_Output(BT656_A_OUT_DEC_I,0,0);	//BT656_A_SelectOutput(BT656_INTERLACE, 0,0, 1);	//I, ?,?, Dec
		//BT656_A_SelectCLKO(0,0);

		//BKTODO: Do not scale the video source.

		//h_SAV = DecoderGetHDelay();
		//v_SAV = DecoderGetVDelay();
		//HLen = DecoderGetHActive();	HLen = 720;
		//VLen = DecoderGetVActive();
		//BT656_D_Crop(h_SAV,v_SAV,HLen,VLen);
		//BT656_SelectSource(0x00);

		if(mode==0 || mode==3 || mode==4 || mode==6) {
			//ntsc
			BT656_InitExtEncoder(BT656_8BIT_525I_YCBCR_TO_CVBS);
		}
		else if(mode==1 || mode==2 || mode==5) {
			//pal
			BT656_InitExtEncoder(BT656_8BIT_625I_YCBCR_TO_CVBS);
		}
		else {
			//BUG...
		}

		break;
	case BT656ENC_SRC_ARGB: //aRGB
		mode = aRGB_GetInputStatus();
		if((mode & 0x38) == 0) {
			// no signal
			failed = 1;
			break;
		}
		if(mode & 0x08) { // component
			BYTE ck2s;




//			HPol = mode & 0x80 ? 1:0;
//			VPol = mode & 0x40 ? 1:0;
			switch(mode & 0x07) {
//#if 0
//			case 0:	p = 0; ck2s=0;		break;	//480i
//			case 1:	p = 0; ck2s=0;		break;	//576i
//			case 2:	p = 1; ck2s=2;		break;	//480p
//			case 3:	p = 1; ck2s=2;		break;	//576p
//#else
			//ADC, ADC,
			case 0:	p = 2; ck2s=2;		break;	//480i
			case 1:	p = 2; ck2s=2;		break;	//576i
			case 2:	p = 2; ck2s=2;		break;	//480p
			case 3:	p = 2; ck2s=2;		break;	//576p
//#endif
			case 4:	failed=1;			break; 	//1080i out of BT565
			case 5:	failed=1;			break; 	//720p  out of BT656
			case 6: failed=1;			break; 	//1080p out of BT656
			default:
					failed=1;			break;
			}
			if(failed)
				break;

			//make LLPLL double
			//wTemp = PCMDATA[ mode & 0x07 ].htotal;
			//wTemp = YUVDividerPLL[mode & 0x07];
			switch(mode & 0x07) {
			case 0:	/*BT656_InitExtEncoder(BT656_8BIT_525I_YCBCR_TO_CVBS);*/ wTemp = 858;	break;	//480i
			case 1:	/*BT656_InitExtEncoder(BT656_8BIT_625I_YCBCR_TO_CVBS);*/ wTemp = 864;	break;	//576i
			case 2:	/*BT656_InitExtEncoder(BT656_8BIT_525P_YCBCR_TO_YPBPR);*/ wTemp = 858;	break;	//480p
			case 3:	/*BT656_InitExtEncoder(BT656_8BIT_625P_YCBCR_TO_YPBPR);*/ wTemp = 864;	break;	//576p
			}
			wTemp *= 2;
			wTemp -= 1;
			Write2TW88(REG1C3,REG1C4, wTemp);
			//use div2 RGBCLK, ADCCLK
			//WriteTW88(REG1EA, ReadTW88(REG1EA) & ~0x03);
			BT656_AdcRgbNoClkDivider2(0,0);


			//BT656_A_SelectOutput(p, 0/*HPol*/, 0/*VPol*/, 0);
			//BT656_A_SelectCLKO(ck2s,0);
			BT656_A_Output(BT656_A_OUT_DEC_ADC, 0,0);

		}
		else {	//PC input
			//read LLPLL Divider & assume input video format.
			//need measure...
			//wTemp = VAdcLLPLLGetDivider();
			//if(wTemp == )

			//use Input_aRGBMode.
			//extern code struct _PCMODEDATA PCMDATA[];
			//struct _PCMODEDATA *pTbl;
			struct VIDEO_TIME_TABLE_s *pVesaTimeTable;




			if(Input_aRGBMode > 8) {
				failed=1;
				break;	
			}
			//pTbl = &PCMDATA[Input_aRGBMode];
			pVesaTimeTable = &TW8836_VESA_TABLE[Input_aRGBMode];

			HPol = pVesaTimeTable->pol & HPOL_P ? 1: 0;
			VPol = pVesaTimeTable->pol & VPOL_P ? 1: 0;

			//BT656_A_SelectOutput(1, HPol, VPol, 0);
			//BT656_A_SelectCLKO(0,0);
			BT656_A_Output(BT656_A_OUT_DEC_ADC, 0,0);

			//BT656_InitExtEncoder(BT656_8BIT_525P_YCBCR_TO_YPBPR);  //480p
		}
		//h_SAV=InputGetHStart();
		//v_SAV=InputGetVStart();
		//HLen=InputGetHLen();
		//VLen=InputGetVLen();
		//BT656_D_Crop(h_SAV,v_SAV,HLen,VLen);
		BT656_SelectSource(0x01);
		break;
	case BT656ENC_SRC_DTV: //DTV
#if 1 //BK130109
		HLen=InputGetHLen();
		VLen=InputGetVLen();
		BT656_D_Crop(32,32,HLen,VLen);
		BT656_SelectSource(0x02);
#else
		HPol = 0;
		VPol = 0;
		h_SAV=InputGetHStart();
		v_SAV=InputGetVStart();
		HLen=InputGetHLen();
		VLen=InputGetVLen();

		if(HLen > 720) {
			failed=1;
			break;				
		}
		BT656_A_SelectOutput(1, HPol, VPol, 0);
		BT656_A_SelectCLKO(0,0);
		BT656_D_Crop(h_SAV,v_SAV,HLen,VLen);


		BT656_InitExtEncoder(BT656_8BIT_525P_YCBCR_TO_YPBPR);  //480p
#endif
		break;

	case BT656ENC_SRC_LVDS: //LVDS
#if 1	//480p
		//{3,		720,480,60,		HPOL_N | VPOL_N,	858,16,62,60,		525,9,6,30},
		//==>       1440,482,         +H +V            ????,?,8,204,     ???,??,1,29       
		hDelay = 56;
		vDelay = 46;
		hActive = 720;
		vActive = 480;

		//720P
		//{4,		1280,720,60,	HPOL_P | VPOL_P,	1650,110,40,220,	750,5,5,20},

		//576P	{18,	720,576,50,		HPOL_N | VPOL_N,	864,12,64,68,		625,5,5,39	},



		BT656_D_Crop(hDelay,vDelay,hActive,vActive);
		BT656_SelectSource(0x03);
#endif

#if 0 //BK130109
		HLen=InputGetHLen();
		VLen=InputGetVLen();

		BT656_D_Crop(32,32,HLen,VLen);
		BT656_SelectSource(0x03);
#endif
#if 0
		//I don't know how to ...yet.
		//failed = 1;

		//let's try some fixed value
		HPol = 0;
		VPol = 0;
		h_SAV= 60; //InputGetHStart();
		v_SAV= 30; //InputGetVStart();
		HLen=720; //InputGetHLen();
		VLen=480; //InputGetVLen();

		BT656_A_SelectOutput(1, HPol, VPol, 0);
		BT656_A_SelectCLKO(0,0);
		BT656_D_Crop(h_SAV,v_SAV,HLen,VLen);


		BT656_InitExtEncoder(BT656_8BIT_525P_YCBCR_TO_YPBPR);  //480p
#endif
		break;

	case BT656ENC_SRC_PANEL: //Panel
#if 1 //BK130109
		//h_SAV = ScalerReadHDEReg();
		h_SAV = ScalerReadLineBufferDelay() + 26;
		bTemp = ReadTW88(REG20D) & 0x04;
		wTemp = ReadTW88(REG221) & 0x03; wTemp <<= 8;
		wTemp |= ReadTW88(REG213);
		if(h_SAV < wTemp)
			h_SAV = 0;
		else
			h_SAV -= wTemp; //HSyncPos.
		if(bTemp ==0) {
			bTemp = ReadTW88(REG214); //HSyncWidth
			if(h_SAV < bTemp)
				h_SAV = 0;
			else 
				h_SAV -= bTemp;
		}
		h_SAV &= 0xFFFE;	//prefer EVEN.

		v_SAV = ScalerReadVDEReg() + 7;
		Printf("\n\rSAV h:%d, v:%d",h_SAV,v_SAV);
		BT656_D_Crop(h_SAV,v_SAV,800,480);
		BT656_SelectSource(0x04);
		WriteTW88(REG068, ReadTW88(REG068) | 0x80);		//use Input Field
//		PclkoSetDiv(3 /* div 4 = 108 / 4 = 27MHz */);

#else
		failed = 1;
#endif
		break;
	}

	if(failed) {
		//try PANEL.
		Printf("\=======BT656 PANEL======>Force 720x480p");
		ScalerTest(2); //force 720x480p
		BT656_InitExtEncoder(BT656_8BIT_525P_YCBCR_TO_YPBPR);  //480p

		//current EVB can support only SD. not 800x480.
		//at first, call "Scaler x" for 720x480p output.


		bTemp = ReadTW88(REG20D);
		HPol = bTemp & 0x04 ? 1: 0;
		VPol = bTemp & 0x08 ? 1: 0;
		BT656_A_SelectOutput(1, HPol, VPol, 0);
		BT656_A_SelectCLKO(1,0);
				
		h_SAV=ScalerReadHDEReg();
		v_SAV=ScalerReadLineBufferDelay()+33;
		HLen=720;
		VLen=480;
		BT656_D_Crop(h_SAV,v_SAV,HLen,VLen);

	}
	BT656_Enable(ON);
}
#endif
#if 0
void ChangeBT656__MAIN(BYTE mode)
{
//	BYTE fError;
	BYTE bt656_src_mode;
	WORD wTemp;
	struct COMP_VIDEO_TIME_TABLE_s *pTimeTable;
	WORD hActive,vActive;
	WORD hCropStart,vCropStart;
	BYTE ext_ic_mode;
 
 	//struct VIDEO_TIME_TABLE_s *pTimeTable;
	//pTimeTable = &TW8836_VESA_TABLE[Input_aRGBMode];

	Printf("\n\rChangeBT656__MAIN(%bd)",mode);

	BT656_Enable(OFF);
	if(mode == BT656ENC_SRC_OFF)
		return;
	
	bt656_src_mode = BT656_CheckLoopbackCombination(mode);
	Printf("\n\rChangeBT656__MAIN bt656_src_mode:%bd",bt656_src_mode);
	if(bt656_src_mode == 0xFF) {
		return;
	}
	//now BT656ENC_SRC_OFF, BT656ENC_SRC_AUTO can not pass.

	if(bt656_src_mode == BT656ENC_SRC_OFF) {
		BT656_Enable(OFF);
		return;
	}

	//
	//check input source and adjust the input configure if need.
	//

	//if analog, adjust input crop.
	//if digital, adjust input crop.
	if(bt656_src_mode==BT656ENC_SRC_DEC) {
		//if() 
		//	DecoderSetOutputCrop(8,720+28,12,240+2);
		//else if()
		//	DecoderSetOutputCrop(8,748,28,288+2);
		//else
		//	Printf("\n\rTodo..");
			
	}
	else if(bt656_src_mode==BT656ENC_SRC_ARGB) {
		if(InputMain==INPUT_COMP) {
			//COMP
			switch(Input_aRGBMode) {
			case MEAS_COMP_MODE_480I:
				DecoderSetOutputCrop(8,720+28,12,240+2);
				break;
			case MEAS_COMP_MODE_480P:
				DecoderSetOutputCrop(8,720+28,24,480+4);
				break;
			case MEAS_COMP_MODE_576I:
				DecoderSetOutputCrop(8,720+28,28,288+2);
				break;
			case MEAS_COMP_MODE_576P:
				DecoderSetOutputCrop(8,720+28,28,576+4);
				break;
			default:
				break;
			}
			//make LLPLL double.
			pTimeTable = &TW8836_COMP_TABLE[mode];
			wTemp = pTimeTable->hTotal * 2 - 1;
			Write2TW88(REG1C3,REG1C4, wTemp);
		}
		else {
			//WORD hFPorch;
			struct VIDEO_TIME_TABLE_s *pVesaTimeTable;
			pVesaTimeTable = &TW8836_VESA_TABLE[Input_aRGBMode];
			
			DecoderSetOutputCrop(
				2,pVesaTimeTable->hActive + pVesaTimeTable->hFPorch +2, 
				2,pVesaTimeTable->vTotal-2-1);

			//PC
			//make LLPLL double.
			//pTimeTable = &TW8836_COMP_TABLE[mode];
			wTemp = pVesaTimeTable->hTotal * 2 - 1;
			Write2TW88(REG1C3,REG1C4, wTemp);
		}	
	}
	else {
		//digital.
		//set  REG068~REG06C
	}


	BT656_Enable(ON);
	BT656_SelectSource(bt656_src_mode);

	if(bt656_src_mode == BT656ENC_SRC_DEC) {
		//analog
		BT656_A_Output(BT656_A_OUT_DEC_I,0,0);
	} 
	else if(bt656_src_mode == BT656ENC_SRC_ARGB) {
		//analog
		BT656_A_Output(BT656_A_OUT_DEC_ADC, 0,0);
	}
	else if(bt656_src_mode == BT656ENC_SRC_DTV) {
		//not yet...
	}
	else if(bt656_src_mode == BT656ENC_SRC_LVDS) {
		//digital
		BT656_D_SetRGB(ON);

		MeasStartMeasure();
		hActive = MeasGetHActive( &hCropStart );
		vActive = MeasGetVActive( &vCropStart );
		Printf("\n\rPrevMeas %d hCropStart:%d+4",hActive, hCropStart);
		Printf("\n\r         %d vCropStart:%d+1",vActive, vCropStart);
		//magic ?
		if     (vActive==240) { hCropStart = 54; vCropStart += (6 -1);  ext_ic_mode = BT656_8BIT_525I_YCBCR_TO_CVBS; }
		else if(vActive==480) { hCropStart = 54; vCropStart += (12 -1); ext_ic_mode = BT656_8BIT_525P_YCBCR_TO_YPBPR; }
		else if(vActive==288) { hCropStart = 65; vCropStart += (5 - 1); ext_ic_mode = BT656_8BIT_625I_YCBCR_TO_CVBS; }
		else if(vActive==576) { hCropStart = 65; vCropStart += (8 - 1); ext_ic_mode = BT656_8BIT_625P_YCBCR_TO_YPBPR; }
		else {
			Printf("\n\rwho are you ?");
			hCropStart = 54;
			vCropStart = 24;
			ext_ic_mode = BT656_8BIT_525I_YCBCR_TO_CVBS;
		}
		BT656_D_Crop(hCropStart, vCropStart, hActive,vActive);
		BT656_InitExtEncoder(ext_ic_mode);
	}
	else if(bt656_src_mode == BT656ENC_SRC_PANEL) {
		//not yet
	} 
	else {
		//something wrong.
	}

	CheckAndSetBT656DecOnLoop(bt656_src_mode);
}
#endif
#if 0
//BYTE ChangeBT656Input(BYTE mode)
//{
//	BYTE bt656_src_mode;
//	BYTE error;
//	WORD hDelay,vDelay,hActive,vActive;
//
//	if(mode == BT656ENC_SRC_OFF) {
//		BT656_Enable(OFF);
//		return ERR_SUCCESS;
//	}
//
//	//check input combination
//	error = 0;
//	switch(InputMain) {
//	case INPUT_CVBS:
//	case INPUT_SVIDEO:
//		if(mode != BT656ENC_SRC_DEC)
//			error = 1;
//		break;
//	case INPUT_COMP:
//	case INPUT_PC:
//		if(mode != BT656ENC_SRC_ARGB)
//			error = 1;
//		break;
//	case INPUT_DVI:
//	case INPUT_HDMIPC:
//	case INPUT_HDMITV:
//		if(mode != BT656ENC_SRC_DTV)
//			error = 1;
//		break;
//	case INPUT_LVDS:
//		if(mode != BT656ENC_SRC_LVDS)
//			error = 1;
//		break;
//	case INPUT_BT656: //for Loopback
//	default:
//		error=1;
//		break;
//	}
//	if(error) {
//		Printf("\n\rInvalid InputMain:");
//		PrintfInput(InputMain, 0);
//		Printf(" and BT656ENC_SRC_%bx",mode);
//		return 1;
//	}
//	bt656_src_mode = mode;
//	BT656_Enable(ON);
//	BT656_SelectSource(bt656_src_mode);
//	if(bt656_src_mode == BT656ENC_SRC_DEC) {
//		//at first, select DEC(I).
//		//If you need DEC(DI), you have to adjust it by manually.
//		BT656_A_SelectOutput(0, 0,0, 1);
//		BT656_A_SelectCLKO(0, 1);
//		BT656_A_SetLLCLK_Pol(0);
//		if(InputMain == INPUT_COMP || InputMain == INPUT_PC) {
//			BT656_AdcRgbNoClkDivider2(0,0);	//use div2
//		}
//		else {
//			BT656_AdcRgbNoClkDivider2(1,1);	//no divider
//		}
//	}	
//	else if(bt656_src_mode == BT656ENC_SRC_ARGB) {
//		//select ADC.
//		BT656_A_SelectOutput(2, 0,0, 0);
//		BT656_A_SelectCLKO(2, 1);
//		BT656_A_SetLLCLK_Pol(0);
//		BT656_AdcRgbNoClkDivider2(1,1);	//no divider
//	}
//	else {
//		//all digital. (DTV,LVDSRx,Panel)
//		//read input & adjust delay,active value.
//		//
//		//for example: LVDS 480P
//		hDelay = 56;
//		vDelay = 46;
//		hActive = 720;
//		vActive = 480;
//		BT656_D_Crop(hDelay,vDelay,hActive,vActive);
//	}		
//	return ERR_SUCCESS;		
//}
#endif
/**
*
* Check DTV BT656Dec input and set scaler.
* If InputMain has a conflict, stop and print only a debug message.
* This function is for BT656 Loop back feature.
* 
* extern
*	InputMain
*	InputBT656
*
*	select i656(or p656) DTV input.
* 	read BT656 module.
*	read measured value.
*
* If loopback is avaiable,
*	adjust input crop, scaler.
*/
//BYTE CheckAndSetDtvBT656(BYTE mode)
#if 0
BYTE CheckAndSetBT656DecOnLoop(BYTE bt656enc_src_mode)
{
	BYTE ret = bt656enc_src_mode;
	BYTE hPol,vPol;		
	WORD /*hTotal,*/ vTotal;
	WORD hActive,vActive;
	WORD hSync,vSync;
		
	WORD hCropStart,vCropStart;
	BYTE dtv_route, dtv_format;
	WORD wTemp;
	struct VIDEO_TIME_TABLE_s *pTimeTable;
	pTimeTable = &TW8836_VESA_TABLE[Input_aRGBMode];


	Printf("\n\rCheckAndSetBT656DecOnLoop(%bd)",bt656enc_src_mode);

	//select DTV routing and format.
	if(bt656enc_src_mode == BT656ENC_SRC_DEC) {
		dtv_format = DTV_FORMAT_INTERLACED_ITU656;
	}
	else if(bt656enc_src_mode == BT656ENC_SRC_ARGB) {
		//if(InputMain == INPUT_CVBS 
		//|| InputMain == INPUT_SVIDEO) {
		//	dtv_format = DTV_FORMAT_PROGRESSIVE_ITU656;
		//}
		//else 
		if(InputMain == INPUT_COMP) {
			if(Input_aRGBMode == MEAS_COMP_MODE_480I
			|| Input_aRGBMode == MEAS_COMP_MODE_576I)
				dtv_format = DTV_FORMAT_INTERLACED_ITU656;
			else
				dtv_format = DTV_FORMAT_PROGRESSIVE_ITU656;
		}
		else {
			//INPUT_PC
			dtv_format = DTV_FORMAT_PROGRESSIVE_ITU656;
		}
	}
	else {
		if(InputMain == INPUT_LVDS)
			Bt656DecSetClkPol(ON);	//invert-clock

		dtv_format = DTV_FORMAT_PROGRESSIVE_ITU656;
	}
	dtv_route = DTV_ROUTE_PbYPr;
	DtvSetRouteFormat(dtv_route, dtv_format);
	DtvSetReverseBusOrder(ON);

//access = 0;	  //off visr
	Interrupt_enableVideoDetect(OFF);


	//select InMux
	if(bt656enc_src_mode == BT656ENC_SRC_DEC)
		InputSetSource(INPUT_PATH_DTV, INPUT_FORMAT_YCBCR);
	else if(bt656enc_src_mode == BT656ENC_SRC_ARGB) {
		if(InputMain == INPUT_COMP)
			InputSetSource(INPUT_PATH_DTV, INPUT_FORMAT_YCBCR);
		else
			InputSetSource(INPUT_PATH_DTV, INPUT_FORMAT_RGB);
	}
	else if(bt656enc_src_mode == BT656ENC_SRC_LVDS) {
		InputSetSource(INPUT_PATH_DTV, INPUT_FORMAT_YCBCR);
	}
	else
		InputSetSource(INPUT_PATH_DTV, INPUT_FORMAT_RGB);
	WriteTW88(REG040,ReadTW88(REG040) | 0x04);	//enable DTVCLK2

	//read measure
	if(MeasStartMeasure()) {
		Printf("\n\rMeasure Failed....");
		return 1;
	}
	delay1ms(500);
	MeasStartMeasure();


	hActive = MeasGetHActive( &hCropStart );
	vActive = MeasGetVActive( &vCropStart );

	if ( hSync > (hActive/2) )	hPol = 1;	//active low. something wrong.
	else						hPol = 0;	//active high
	if ( vSync > (vTotal/2) )	vPol = 1;	//active low. something wrong.
	else						vPol = 0;	//active high


//#ifdef DEBUG_DTV
	Printf("\n\rNewMeas %d hPol:%bx hCropStart:%d+4",hActive, hPol, hCropStart);
	Printf("\n\r        %d vPol:%bx vCropStart:%d+1",vActive, vPol, vCropStart);
//#endif
   	Printf("\n\rmode:%bd",bt656enc_src_mode);

	//set scaler
	//struct COMP_VIDEO_TIME_TABLE_s *pTimeTable;
	//Input_aRGBMode
	if(bt656enc_src_mode == BT656ENC_SRC_DEC) {
		//InputSubMode
		if(InputSubMode==0) {
			DecoderSetOutputCrop(8,720+28,14,240+2);
			InputSetCrop(217, 19, 1440+8, 242);
			ScalerSetHScale(720);
			ScalerSetVScale(240);
			ScalerSetLineBufferSize(720+6);
			wTemp = 19 * PANEL_V / 240 + 2;
			ScalerWriteVDEReg(wTemp);
		}
		else if(InputSubMode==1) {
			DecoderSetOutputCrop(2+3,720+28,23,288+2);
			InputSetCrop(222, 21, 1440+8, 288+2);
			ScalerSetHScale(720);
			ScalerSetVScale(288);
			ScalerSetLineBufferSize(720+6);
			wTemp = 21 * PANEL_V / 288 + 2;
			ScalerWriteVDEReg(wTemp);
		}
		else {
			Printf("\n\rBKTODO...");
		}
	}
	else if(bt656enc_src_mode == BT656ENC_SRC_ARGB) {
		//
		if(InputMain == INPUT_COMP) {
			aRGB_COMP_set_scaler(Input_aRGBMode, 1, 0); //BT656. NoOverScan
			//hLen = ;
		}
		else {

			vCropStart = pTimeTable->vSync+pTimeTable->vBPorch  + 1;

			//Input_aRGBMode 
			InputSetCrop(
				hCropStart + 4*2+10,  		//aRGB_delay(4)*2 + BT656_delay(10) 
				vCropStart - 14,
				pTimeTable->hActive*2+1, 
				pTimeTable->vActive+2);
			ScalerSetHScale(pTimeTable->hActive);
			ScalerSetVScale(pTimeTable->vActive);
			ScalerSetLineBufferSize(pTimeTable->hActive+6);
			wTemp = (vCropStart-14) * PANEL_V / pTimeTable->vActive + 2;
			ScalerWriteVDEReg(wTemp);
		}
	}
	else if(bt656enc_src_mode == BT656ENC_SRC_LVDS) {
#if 0
			InputSetCrop(
				hCropStart + 4, 
				vCropStart + 1,
				pTimeTable->hActive, 
				pTimeTable->vActive);
			ScalerSetHScale(pTimeTable->hActive);
			ScalerSetVScale(pTimeTable->vActive);
			ScalerSetLineBufferSize(pTimeTable->hActive);
			wTemp = vCropStart * PANEL_V / pTimeTable->vActive;
			ScalerWriteVDEReg(wTemp);
#endif
			InputSetCrop(
				hCropStart + 4, 
				vCropStart + 1,
				hActive, //pTimeTable->hActive, 
				vActive); //pTimeTable->vActive);
			ScalerSetHScale(hActive / 2); //pTimeTable->hActive);
			ScalerSetVScale(vActive); //pTimeTable->vActive);
			ScalerSetLineBufferSize(hActive / 2); //pTimeTable->hActive);
			wTemp = vCropStart * PANEL_V / vActive; // pTimeTable->vActive;
			ScalerWriteVDEReg(wTemp);
	}
	else {
		//digital
		//no loopback....
	}


	Interrupt_enableVideoDetect(ON);			//turnoff Video Signal Interrupt
	//Interrupt_enableSyncDetect(ON);


	return ret;
}
#endif
#if 0  //=========OLD======================*/
BYTE CheckAndSetDecoderScaler____OLD( void )
{
	BYTE	mode;
	DWORD	vPeriod, vDelay;
	BYTE vDelayAdd;
	DWORD x_ratio, y_ratio;
	DWORD dTemp;

#ifdef DEBUG_DEC
	dPrintf("\n\rCheckAndSetDecoderScaler start.");
#endif	
	if ( DecoderCheckVDLOSS(100) ) {
#ifdef DEBUG_DEC
		ePuts("\n\rCheckAndSetDecoderScaler VDLOSS");
#endif
		DecoderFreerun(DECODER_FREERUN_60HZ);
		//ScalerSetFreerunAutoManual(ON, ON);	//ScalerSetFreerunManual( ON );
		return( 1 );
	}
	//get standard
	mode = DecoderCheckSTD(100);
	if ( mode == 0x80 ) {
	    ePrintf("\n\rCheckAndSetDecoderScaler NoSTD");
		return( 2 );
	}
	mode >>= 4;
	InputSubMode = mode;

	VideoAspect = GetAspectModeEE();

	//read VSynch Time+VBackPorch value
	vDelay = DecoderGetVDelay();

	//reduce VPeriod to scale up.
	//and adjust V-DE start.

	//720x240 => 800x480
	x_ratio = PANEL_H;
	x_ratio *=100;
	x_ratio /= 720;
	y_ratio = PANEL_V;
	y_ratio *=100;
	y_ratio /= 480;
#ifdef DEBUG_DEC
	dPrintf("\n\rXYRatio X:%ld Y:%ld",x_ratio,y_ratio);
#endif

	if(VideoAspect==VIDEO_ASPECT_ZOOM) {
		if(x_ratio > y_ratio) {
			dPrintf(" use x. adjust Y");
			y_ratio = 0;
		}
		else {
			dPrintf(" use y. adjust X");	
			x_ratio = 0;
		}
	}
	else if(VideoAspect==VIDEO_ASPECT_NORMAL) {
		if(x_ratio > y_ratio) {
			dPrintf(" use y. adjust X");
			x_ratio = 0;
		}
		else {
			dPrintf(" use x. adjust Y");
			y_ratio = 0;
		}
	}
	else {
		x_ratio = 0;
		y_ratio = 0;
	}
	//720x288 => 800x480

	if ( mode == 0 ) {				// NTSC(M)
		vPeriod = 228;				// NTSC line number.(reduced from 240 to 228)
		//vDelay += 12; 			// (6 = ( 240-228 ) / 2) times 2 because. 240->480
		//vDelay += 27;				// for V Back Porch & V top border
		vDelayAdd = 39;

		if(VideoAspect==VIDEO_ASPECT_ZOOM) {
			vDelayAdd += 30;
			vDelayAdd += 5; //???
		}

		//crop value
		//	---normal---
		//	hStart:8   hActive:720
		//	vStart:21  hActive:240				ScalerOutputVDE = 27*480/228 =42
		//	---overscan---
		//	hStart:8  	  hActive:720
		//	vStart:21+6   vActive:228(240*0.95)  ScalerOutputVDE = 27*480/228 = 56.8


		//DecoderSetVActive(240);		//set:240 0x0F0
		////BK130123 When it change from PAL to NTSC, we need Vdelay 18.	REG108[]
		//DecoderSetVDelay(18);
		//DecoderSetOutputCrop(8+3,720-6,21,240);
		DecoderSetOutputCrop(3+3,720-6,21,240);
		vDelay=21;
//#if defined(PANEL_AUO_B133EW01)
//		vDelayAdd = 74;
//#else
//		vDelayAdd = 36;
//#endif
		//if PANEL_V is 800, vDelayAdd is 74.
		//if PANEL_V is 480, vDelayAdd is 36.
		dTemp = (DWORD)(vDelay+6) * PANEL_V / 228;
		dTemp += 1;
		vDelayAdd = dTemp - vDelay;


		WriteTW88(REG040, ReadTW88(REG040) & ~0x10);	//recover. pal using "1".

		//prepare info
		FOsdSetInputMainString2FOsdMsgBuff();	//GetInputMainString(FOsdMsgBuff);									 	
		TWstrcat(FOsdMsgBuff," NTSC");			//BK110811. call FOsdCopyMsgBuff2Osdram(OFF); to draw
	}
	else if ( mode == 1 ) {			 //PAL(B,D,G,H,I)
		vPeriod = 275;				// PAL line number,(Reduced from 288 to 275)
#if 0
		//vDelay += 7; 				// 6.7 = ( 288-275 ) / 2
		//vDelay += 2;				// add some more for V Back Porch & V Top border
		vDelayAdd = 25;
#else
		//vDelay += 14; 			// (6.7 = ( 288-275 ) / 2  ) * 2
		//vDelay += 25;				// add some more for V Back Porch & V Top border
		vDelayAdd = 39;
#endif
		if(VideoAspect==VIDEO_ASPECT_ZOOM)
			vDelayAdd += 33;

		//crop value
		//	---normal---
		//	hStart:6  hActive:720
		//	vStart:23 vActive:288			ScalerOutputVDE = 23*480/288=38.3
		//	---overscan---
		//	hStart:6  hActive:720	
		//	vStart:23+(13/2)  vActive:275(288*0.95)	 ScalerOutputVDE = 29.5*480/275=51.4

		//DecoderSetVActive(288);		//set:288. 0x120
		////BK1211129 Vdelay need 22.	REG108[]
		//DecoderSetVDelay(22);
//		DecoderSetOutputCrop(6+3,720-6,23,288);
		DecoderSetOutputCrop(2+3,720-6,23,288);
		vDelay = 23;	//real vDelay is a 23.9.
//#if defined(PANEL_AUO_B133EW01)
//		vDelayAdd = 53;
//#else
//		vDelayAdd = 30;
//#endif	

		dTemp = (DWORD)(20+1+6) * PANEL_V / 275;
		dTemp += 1;
		vDelayAdd = dTemp - 20;

	
		WriteTW88(REG040, ReadTW88(REG040) | 0x10);	//clk pol invert. clear some noise.
		 
		//prepare info
		FOsdSetInputMainString2FOsdMsgBuff();	//GetInputMainString(FOsdMsgBuff);									 	
		TWstrcat(FOsdMsgBuff," PAL");			//BK110811. call FOsdCopyMsgBuff2Osdram(OFF); to draw
	}
	//BKTODO: Support more mode
	//0 = NTSC(M)          
	//1 = PAL (B,D,G,H,I)          
	//2 = SECAM          
	//3 = NTSC4.43
	//4 = PAL (M)            
	//5 = PAL (CN)                     
	//6 = PAL 60  
	else if ( mode == 3 //MTSC4
		   || mode == 4 //PAL-M
	       || mode == 6 //PAL-60			 
	) {				
		vPeriod = 228;
		vDelayAdd = 39;
 		if(VideoAspect==VIDEO_ASPECT_ZOOM) {
			vDelayAdd += 30;
			vDelayAdd += 5; //???
		}
 		//DecoderSetVActive(240);		//set:240 0x0F0
		DecoderSetOutputCrop(8+3,720-6,21,240);
		vDelay=21;
#if defined(PANEL_AUO_B133EW01) || defined(PANEL_1024X600)
		vDelayAdd = 74;
#else
		vDelayAdd = 36;
#endif

		WriteTW88(REG040, ReadTW88(REG040) & ~0x10);	//recover. pal using "1".

		//prepare info
		FOsdSetInputMainString2FOsdMsgBuff();									 	
		if(mode==3) TWstrcat(FOsdMsgBuff," NTSC4");		
		if(mode==4) TWstrcat(FOsdMsgBuff," PAL-M");		
		if(mode==6) TWstrcat(FOsdMsgBuff," PAL-60");		
   }     
	else if ( mode == 2 //SECAM
		  ||  mode == 5 //PAL-CN
	) {	
		vPeriod = 275;
		vDelayAdd = 39;			
		if(VideoAspect==VIDEO_ASPECT_ZOOM)
			vDelayAdd += 33;

		//DecoderSetVActive(288);		//set:288. 0x120
		DecoderSetOutputCrop(6+3,720-6,23,288);
		vDelay = 23;
#if defined(PANEL_AUO_B133EW01) || defined(PANEL_1024X600)
		vDelayAdd = 53;
#else
		vDelayAdd = 28;
		vDelayAdd += 3;	//??
#endif		
		 
		WriteTW88(REG040, ReadTW88(REG040) & ~0x10);	//recover. pal using "1".

		//prepare info
		FOsdSetInputMainString2FOsdMsgBuff();									 	
		if(mode==2) TWstrcat(FOsdMsgBuff," SECAM");
		if(mode==4) TWstrcat(FOsdMsgBuff," PAL-CN");
	}
	else {
#ifdef DEBUG_DEC
		ePrintf( "\n\rCheckAndSetDecoderScaler Input Mode %bd does not support now", mode );
#endif
		return(3);
	}
	
	ScalerSetLineBufferSize(720-6);	//BK120116	- temp location. pls remove

	if(y_ratio) ScalerSetHScaleWithRatio(720-6, (WORD)y_ratio);
	else		ScalerSetHScale(720-6);					//PC->CVBS need it.
	if(x_ratio)	ScalerSetVScaleWithRatio(vPeriod, (WORD)x_ratio);
	else 		ScalerSetVScale(vPeriod);				//R206[7:0]R205[7:0]	= vPeriod
	
	ScalerWriteVDEReg(vDelay+vDelayAdd);			//R215[7:0]=vDelay, R217[3:0]R216[7:0]=PANEL_V

#ifdef DEBUG_DEC
 	//dPrintf( "\n\rInput_Mode:%02bx VDE_width:%ld, vBackPorch:%ld", mode, vPeriod, vDelay );
	ePrintf( "\n\rInput_Mode:%s VDE_width_for_scaler:%ld, V-DE:%ld+%bd", mode ? "PAL":"NTSC", vPeriod, vDelay,vDelayAdd );
#endif	
	return(0);
}
#endif

//===================================================
// New Scaler routines comes from TW8809.
// Under developing.
//===================================================

//-----------------------------------------------------------------------------
/**
* description
* 	update Scaler ScaleRate
*
* ex: Scaler09SetScaleRate(720,240, 720,480)
*     Scaler09SetScaleRate(1280,720, 800, 480)
*/
#if 0
void ScalerSetScaleRate(WORD hIn, WORD vIn, WORD hOut, WORD vOut)
{
	DWORD dTemp;
	WORD hdScale,xScale,yScale;

	// X scale
	if(hIn > hOut) {
		//X downscale
		xScale = 0x2000;
		dTemp = hIn;
		dTemp *= 0x400L;
		dTemp /= hOut;
		hdScale = dTemp;
	}
	else {
		//X upscale
		hdScale = 0x400;
		dTemp = hIn;
		dTemp *= 0x2000L;
		dTemp /= hOut;
		xScale = dTemp;
	}
	// Y scale
	dTemp = vIn;
	dTemp *= 0x2000L;
	dTemp /= vOut;
	yScale = dTemp;

	//update HW register
	WriteTW88Page(2);
	I2C_Buffer[0] = (BYTE)xScale;
	I2C_Buffer[1] = (BYTE)(xScale >> 8);
	I2C_Buffer[2] = (BYTE)yScale;
	I2C_Buffer[3] = (BYTE)(yScale >> 8);
	WriteBlockTW88(REG203, I2C_Buffer, 4);
	I2C_Buffer[0] = (BYTE)(hdScale);
	I2C_Buffer[1] = ReadTW88(REG20A) & 0xF0;
	I2C_Buffer[1] |= (BYTE)(hdScale>>8);
	WriteBlockTW88(REG209, I2C_Buffer, 2);
}
#endif


//-----------------------------------------------------------------------------
/**
* description
* 	update Scaler Output.
*
* hSync 		hSync Width
* hBPorch		BackPorch width
* hActive		Active Width
* hFPorch		FrontPorch width
* hPol			hsync polarity
*
*/
#define SCALER_TW8835_HDE_DELAY		32
#if 0
void ScalerSetOutputTimeRegs(BYTE index, WORD InputVDeStart,WORD InputVDEWidth) 
{
	struct s_DTV_table *pVid;


							//------------------------------
							//TW8809 BT656 Output registers
//	WORD bt656enc_hdly;		//R068{1:0]R06C[7:0] 
//	WORD bt656enc_hact;		//R069[3:0]R06D[7:0]
//	BYTE bt656enc_vdelay;	//         R06A[7:0]
//	WORD bt656enc_vact;		//R069[6:4]R06B[7:0]
							//------------------------------
							//TW8835 Scaler Output registers
	WORD hdelay2; 			//         R20B[7:0]
	WORD hactive2;			//R20E[6:4]R20C[7:0]
	WORD hpadj;				//R20E[3:0]R20F[7:0]
	WORD ha_pos; 			//R211[5:4]R210[7:0]
	WORD ha_len;			//R212[3:0]R211[7:0]
	WORD hs_pos;			//R211[1:0]R213[7:0]
	BYTE hs_len;			//         R214[7:0]
	WORD va_pos;			//R220[5:4]R215[7:0]
	WORD va_len;			//R217{3:0]R216[7:0]
	BYTE vs_len;			//         R218[7:6]
	WORD vs_pos;			//R220[3:0]R218[5:0]

	pVid = &DTV_table[index];
	Printf("\n\rScalerSetOutputTimeRegs(%bd,%d,%d)",index,InputVDeStart,InputVDEWidth);
	Printf("\n\rOutput DTV_table[%bd]	",index);
	Printf("%bd %dx%d%s@%bd POL:%02bx   %d,%d,%d,%d   %d,%bd,%bd,%bd,%bd %ld",
		pVid->vid,
		pVid->hDeWidth,pVid->vDeWidth,	pVid->fIorP == 1 ? "I" : "P", 
		pVid->vfreq == FREQ_60 ? 60 : pVid->vfreq == FREQ_50 ? 50 : 0,
		pVid->Pol,
		pVid->hTotal,	(WORD)pVid->hFPorch,(WORD)pVid->hSyncWidth,(WORD)pVid->hBPorch,
		pVid->vTotal,	pVid->vPad,pVid->vSyncStart,pVid->vSyncWidth,pVid->vBPorch,
		pVid->pixelfreq
		);

	//
	//horizontal related value.
	//
	hpadj = DTV_table[index].hFPorch;
	hs_pos=0;	
	hs_len=DTV_table[index].hSyncWidth-1;	//HW starts from 0.
	ha_pos = hs_pos + (hs_len + 1) + DTV_table[index].hBPorch;
	ha_pos -= SCALER_TW8835_HDE_DELAY; /* adjust internal delay. It is a minimum value */
	if(InputMain==INPUT_PC) {
		hs_len = ReadTW88(REG1DC) & 0x3F;
		//ha_pos = DTV_table[index].hBPorch -  ReadTW88(REG1DC) & 0x3F;
		ha_pos = 33;
	}
	ha_len = DTV_table[index].hDeWidth;
#if 0
//	hdelay2 = DTV_table[index].hBPorch;
//	hdelay2 -= 27; /*adjust internal delay */
//	if(InputMain==INPUT_CVBS)
//		hdelay2 -= 2;	
//	else if(index==VID_800X480P_IDX)
//		hdelay2 = (WORD)hdelay2 * 169 / 100;  /* 74175000L / 44000000L */
//	hdelay2 += hs_pos;
#else
	hdelay2 = ha_pos - SCALER_TW8835_HDE_DELAY;
	if(InputMain==INPUT_CVBS) {
		hdelay2 += 3; //BK121129
	}
#endif

	//if(hdelay2 & 0x01) hdelay2+=1;	only for TW8809
	hactive2 = DTV_table[index].hDeWidth;

	//new
	hpadj = pVid->hFPorch;
	hs_pos=0;	
	hs_len=pVid->hSyncWidth-1;	//HW starts from 0.
	ha_pos = hs_pos + (hs_len + 1) + pVid->hBPorch;
	ha_pos -= SCALER_TW8835_HDE_DELAY; /* adjust internal delay. It is a minimum value */
	if(InputMain==INPUT_PC || InputMain==INPUT_COMP) {
		hs_len = ReadTW88(REG1DC) & 0x3F;
		//ha_pos = DTV_table[index].hBPorch -  ReadTW88(REG1DC) & 0x3F;
		ha_pos = 33;
	}
	ha_len = pVid->hDeWidth;
	hdelay2 = ha_pos - SCALER_TW8835_HDE_DELAY;
	if(InputMain==INPUT_CVBS) {
		hdelay2 += 3; //BK121129
	}
	hactive2 = pVid->hDeWidth;



//	bt656enc_hdly = 0;
//	bt656enc_hact = hactive2;

	//
	//vertical related value
	//
	if(index==VID_800X480P_IDX) {
		vs_pos = 0; //DTV_table[index].vPad + DTV_table[index].vSyncStart;
		vs_len = 0; //DTV_table[index].vSyncWidth-1;
	}
	else {
		vs_pos = DTV_table[index].vPad + DTV_table[index].vSyncStart;
		vs_len = DTV_table[index].vSyncWidth-1;
		if(vs_len > 3) {
			vs_len = 3;
			vs_pos += (DTV_table[index].vSyncWidth-1 - 3);
		}
		if(InputMain==INPUT_CVBS) {
			vs_pos += 10;
		}
	}

	if(index==VID_800X480P_IDX) {
#if 0
		va_pos = vs_pos;
		va_pos += (vs_len+1);
		va_pos += DTV_table[index].vBPorch;
		va_pos -= 13; //11
#else		
		//use scaled value.
		//va_pos = vs_len+1;
		//va_pos += DTV_table[index].vBPorch;
		//va_pos *= 480;
		//va_pos /= DTV_table[index].vDeWidth;			
		va_pos = InputVDeStart;
		va_pos *= 480;
		va_pos /= InputVDEWidth;			
#endif
	}
	else {
		va_pos = DTV_table[index].vBPorch;
		if(InputMain==INPUT_CVBS) {
			va_pos += 8;
		}
	}
	va_len = DTV_table[index].vDeWidth;

//	bt656enc_vdelay = DTV_table[index].vPad + DTV_table[index].vSyncStart + DTV_table[index].vSyncWidth;
//	bt656enc_vact = va_len;


	//
	// update HW registers
	//
	WriteTW88Page(2);
	I2C_Buffer[0]  = hdelay2;									//REG20B
	I2C_Buffer[1]  = (BYTE)hactive2;							//REG20C
	I2C_Buffer[2]  = ReadTW88(REG20D);							//REG20D
	I2C_Buffer[3]  = ((hactive2 & 0xF00) >> 4) | (hpadj >> 8);	//REG20E
	I2C_Buffer[4]  = (BYTE)hpadj;								//REG20F
	I2C_Buffer[5]  = (BYTE)ha_pos;								//REG210
	I2C_Buffer[6]  = (BYTE)ha_len;								//REG211
	I2C_Buffer[7]  = (ReadTW88(REG212) & 0xF0) | (ha_len >> 8);	//REG212
	I2C_Buffer[8]  = (BYTE)hs_pos;								//REG213
	I2C_Buffer[9]  = hs_len;									//REG214
	I2C_Buffer[10] = (BYTE)va_pos;								//REG215
	I2C_Buffer[11] = (BYTE)va_len;								//REG216
	I2C_Buffer[12] = (ReadTW88(REG217) & 0xF0) | (va_len >> 8);	//REG217
	I2C_Buffer[13] = (vs_len << 6) | (BYTE)(vs_pos & 0x3F);		//REG218
	WriteBlockTW88(REG20B, I2C_Buffer, 14);

	I2C_Buffer[0]  = ((va_pos & 0x300)>>4) | (vs_pos>>8); 		//REG220
	I2C_Buffer[1]  = ((ha_pos & 0x300)>>4) | (hs_pos>>8); 		//REG221
	WriteBlockTW88(REG220, I2C_Buffer, 2);

//	WriteTW88Page(0);
//	I2C_Buffer[0]  = (ReadTW88(REG068) & 0xFC) | (bt656enc_hdly >> 8);		//REG068
//	I2C_Buffer[1]  = (bt656enc_vact & 0x700) >> 4 | (bt656enc_hact >>8);	//REG069
//	I2C_Buffer[2]  = bt656enc_vdelay;										//REG06A
//	I2C_Buffer[3]  = (BYTE)bt656enc_vact;									//REG06B
//	I2C_Buffer[4]  = (BYTE)bt656enc_hdly;									//REG06C
//	I2C_Buffer[5]  = (BYTE)bt656enc_hact;									//REG06D
//	WriteBlockTW88(REG068, I2C_Buffer, 6);
}
#endif

//-----------------------------------------------------------------------------
// Test routines
//-----------------------------------------------------------------------------

#define SCALER_MODE_FOSSIL		0
#define SCALER_MODE_FULL		1
#define SCALER_MODE_OVERSCAN	2
#define SCALER_MODE_720X480P	3	//for BT656 TEST
#if 0
void ScalerTest_Decoder(BYTE scaler_mode)
{
	BYTE cvbs_mode;
	cvbs_mode = DecoderCheckSTD(100);
	if(cvbs_mode & 0x80) {
	    ePrintf("\n\rScalerTest_Decoder NoSTD");
		return; //( 2 );
	}
	cvbs_mode >>= 4;
	if(cvbs_mode==0) {
		//NTSC
		if(scaler_mode==SCALER_MODE_FULL) {
			//scaler full mode
			//DecoderSetOutputCrop(720,240,9,18+2)
			//DecoderSetVDelay(18+2); //spec+delay
			//DecoderSetVActive(240);
			//DecoderSetHDelay(9); //workaround value
			//DecoderSetHActive(720);
			DecoderSetOutputCrop(9,720,18+2,240);
	
			ScalerSetScaleRate(720,240,800,480);
			ScalerSetOutputTimeRegs(VID_800X480P_IDX,/*DTV_table[VID_800X480P_IDX].vBPorch=>*/20, 240);
			//ScalerSetOutputTimeRegs(VID_720X480P_IDX);
		}
		else if(scaler_mode==SCALER_MODE_OVERSCAN) {
			//scaler overscan mode
			//DecoderSetOutputCrop(720,240,9,18+2)
			//DecoderSetVDelay(18+2); //spec+delay
			//DecoderSetVActive(240);
			//DecoderSetHDelay(9); //workaround value
			//DecoderSetHActive(720);
			DecoderSetOutputCrop(9,720,18+2,240);
	
			ScalerSetScaleRate(720,228,800,480);
			ScalerSetOutputTimeRegs(VID_800X480P_IDX,/*DTV_table[VID_800X480P_IDX].vBPorch=>*/20+6, 228);
			//ScalerSetOutputTimeRegs(VID_720X480P_IDX);
		}
		else {
			Puts("\n\rSorry! only support 0,1,2");
		}
	} 
	else {
		//assume PAL
		if(scaler_mode==1) {
			//DecoderSetOutputCrop(720,288,6,20+2+1)
			//DecoderSetVDelay(22+2);	//spec+delay
			//DecoderSetVActive(288);
			//DecoderSetHDelay(11); //workground value
			//DecoderSetHActive(720);
			DecoderSetOutputCrop(11,720,22+2,288);
	
			ScalerSetScaleRate(720,288,800,480);
			ScalerSetOutputTimeRegs(VID_800X480P_IDX,/*DTV_table[VID_800X480P_IDX].vBPorch=>*/22+2, 288);
		} 
		else if(scaler_mode==2) {
			//overscan mode. use 95%
			//DecoderSetOutputCrop(720,288,6,20+2+1)
			//DecoderSetVDelay(22+2);	//spec+delay
			//DecoderSetVActive(288);
			//DecoderSetHDelay(11); //workground value
			//DecoderSetHActive(720);
			DecoderSetOutputCrop(11,720,22+2,288);
	
			ScalerSetScaleRate(720,274,800,480);
			ScalerSetOutputTimeRegs(VID_800X480P_IDX,/*DTV_table[VID_800X480P_IDX].vBPorch=>*/22+2+7, 274); 
		}
	}
}
#endif
//typedef struct s_VIDEO_TIME {
//	WORD hDeWidth,vDeWidth;		BYTE fIorP,vfreq;
//	BYTE Pol;
//	WORD hTotal;	BYTE hFPorch, hSyncWidth, hBPorch;  
//	WORD vTotal; 	BYTE vPad, vSyncStart, vSyncWidth, vBPorch;
//} t_VIDEO_TIME;
#if 0
t_VIDEO_TIME VideoTime;
#endif

#if 0
void ScalerTest_Component(BYTE scaler_mode)
{

	WORD hActive, hStart;	//hTotal  ,hStart,hSync
	WORD vTotal,vActive, vStart; //,,vSync,vBPorch;  //

	BYTE index;
	BYTE vfreq;
	//WORD table_hTotal;
	//WORD wTemp;

//	t_VIDEO_TIME *pVidTime;
	struct s_DTV_table *pVid;

	if(MeasStartMeasure()) {
		//something wrong.
		dPrintf("==>FAIL!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		return;
	}			
	//===========================================
	// read measured value & adjust it.
	// we only have a vfreq & vtotal
	//===========================================
	vfreq  = MeasGetVFreq();
	if ( vfreq < 55 ) vfreq = 50;
	else  vfreq = 60;
	vTotal = MeasGetVPeriod();;

	//print measured value
	Printf("\n\rMeas vTotal:%d vfreq:%bd", vTotal,vfreq);
	
	//search from table
	for(index=0; index < DTV_TABLE_MAX; index++) {
		if(index==0)	continue;

		pVid = &DTV_table[index];
		if((vTotal-2) <= pVid->vTotal && (vTotal+2) >= pVid->vTotal) {
			//==check active. If ext device give an active low sync polarity, we can not use a hTotal.
			//table_hTotal = pVid->hDeWidth;
			//if((hActive-2) <=  table_hTotal && (hActive+2) >= table_hTotal) {
			//	ePuts("\n\r=3=>");
			//	break;
			//}
			if(vfreq == pVid->vfreq)
				break;
		}
	}
	if(index >= DTV_TABLE_MAX) {
		Printf("\n\r=>GiveUp");
		return;
	}

	//add "1" for overscan on vActive
	hStart = ReadTW88(REG530); hStart <<= 8; hStart |= ReadTW88(REG531);
	vStart = ReadTW88(REG538); hStart <<= 8; vStart |= ReadTW88(REG539);
	InputSetCrop(hStart-16, vStart+1+1, pVid->hDeWidth, pVid->vDeWidth+1);
	Printf("\n\rCrop H Pol:%bd Start:%d Active:%d", 0, pVid->hBPorch-16, pVid->hDeWidth);
	Printf("\n\r     V Pol:%bd Start:%d Active:%d", 0, pVid->vBPorch+1,pVid->vDeWidth+1);

				
	Printf("\n\rInput DTV_table[%bd]	",index);
	Printf("%bd %dx%d%s@%bd POL:%02bx   %d,%d,%d,%d   %d,%bd,%bd,%bd,%bd %ld",
		pVid->vid,
		pVid->hDeWidth,pVid->vDeWidth,	pVid->fIorP == 1 ? "I" : "P", 
		pVid->vfreq == FREQ_60 ? 60 : pVid->vfreq == FREQ_50 ? 50 : 0,
		pVid->Pol,
		pVid->hTotal,	(WORD)pVid->hFPorch,(WORD)pVid->hSyncWidth,(WORD)pVid->hBPorch,
		pVid->vTotal,	pVid->vPad,pVid->vSyncStart,pVid->vSyncWidth,pVid->vBPorch,
		pVid->pixelfreq
		);

	
	hActive = pVid->hDeWidth;
	vActive = pVid->vDeWidth;
	vStart =  pVid->vSyncWidth + pVid->vBPorch;


	if(scaler_mode==SCALER_MODE_720X480P) {
		ScalerSetScaleRate(hActive,vActive,720,480);
		ScalerSetOutputTimeRegs(VID_720X480P_IDX, vStart, vActive);
	}
	else {
		ScalerSetScaleRate(hActive,vActive,800,480);
		ScalerSetOutputTimeRegs(VID_800X480P_IDX, vStart, vActive);
	}	
}
#endif


#if 0
void ScalerTest_PC(BYTE scaler_mode)
{
	WORD hActive; // hTotal, ,hStart,hSync
	WORD vTotal,vActive,vStart; //,vSync,vBPorch;//

	BYTE index;
	BYTE vfreq;
//	WORD table_hTotal;
	WORD wTemp;

//	t_VIDEO_TIME *pVidTime;
	struct s_VESA_table *pVid;

	Printf("\n\rScalerTest_PC(%bd)",scaler_mode);

	if(MeasStartMeasure()) {
		//something wrong.
		dPrintf("==>FAIL!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		return;
	}			
	//===========================================
	// read measured value & adjust it.
	// vtotal, vactive, vfreq
	// hactive
	//===========================================
	
	//Horizontal
	//hTotal = MeasGetVsyncRisePos();
	hActive = MeasGetHActive(&wTemp);
	vfreq  = MeasGetVFreq();
	//hSync = MeasGetHSyncRiseToFallWidth();
	//if(hTotal < hActive)
	//	//if input is an interlaced, it can be a measured value on the odd field.
	//	//in this case, make it double.
	//	hTotal *=2;	
	//hStart = wTemp+4; //add HW offset value. hStart can be hBPorch or hSync+hBPorch.

	//Vertical
	vTotal = MeasGetVPeriod();;
	vActive = MeasGetVActive(&wTemp);
	//vSync = MeasGetVSyncRiseToFallWidth();
	//vStart = wTemp-1; //subtract HW offset value. vStart can be vBPorch or vSync+vBPorch.
	//vBPorch = vStart - vSync;

	//print measured value
	Printf("\n\rMeas hActive:%d vActive:%d vTotal:%d vfreq:%bd", hActive, vActive,vTotal, vfreq);

	//search from table
	for(index=0; index < VESA_TABLE_MAX; index++) {
		pVid = &VESA_table[index];
		if(pVid->hDeWidth == 0xFFFF && pVid->vDeWidth==0xFFFF) {
			//return 0xFF;	//give up
			index = 0xFF;
			break;
		}
		if(vTotal < (pVid->vTotal-1) || vTotal > (pVid->vTotal+1))
			continue;
		if(vfreq < (pVid->vfreq-1) || vfreq > (pVid->vfreq+1))
			continue;
		if(vActive < (pVid->vDeWidth-2) || vActive >  (pVid->vDeWidth+2))
			continue;
		//found
		break;
	}
	if(index == 0xFF) {
		Printf("\n\r=>GiveUp");
		return;
	}
	Printf("\n\rInput VESA_table[%bd]	",index);
	Printf("%bd %dx%d@%bd POL:%02bx   %d,%d,%d,%d   %d,%bd,%bd,%bd %ld",
		pVid->vid,
		pVid->hDeWidth,pVid->vDeWidth,	//pVid->fIorP == 1 ? "I" : "P", 
		pVid->vfreq == FREQ_60 ? 60 : pVid->vfreq == FREQ_50 ? 50 : 0,
		pVid->Pol,
		pVid->hTotal,	(WORD)pVid->hFPorch,(WORD)pVid->hSyncWidth,(WORD)pVid->hBPorch,
		pVid->vTotal,	pVid->vFPorch,pVid->vSyncWidth,pVid->vBPorch,
		pVid->pixelfreq
		);


	//add "1" for overscan on vActive
	InputSetCrop(pVid->hBPorch-16, pVid->vBPorch+1, pVid->hDeWidth, pVid->vDeWidth+1);
	Printf("\n\rCrop H Pol:%bd Start:%d Active:%d", 0, pVid->hBPorch-16, pVid->hDeWidth);
	Printf("\n\r     V Pol:%bd Start:%d Active:%d", 0, pVid->vBPorch+1,pVid->vDeWidth+1);


	hActive = pVid->hDeWidth;
	vActive = pVid->vDeWidth;
	vStart =  pVid->vBPorch +1;

	if(scaler_mode==SCALER_MODE_720X480P) {
		ScalerSetScaleRate(hActive,vActive,720,480);
		ScalerSetOutputTimeRegs(VID_720X480P_IDX, vStart, vActive);
	}
	else {
		ScalerSetScaleRate(hActive,vActive,800,480);
		ScalerSetOutputTimeRegs(VID_800X480P_IDX, vStart, vActive);
	}	
}
#endif

#if 0
void ScalerTest_DTV(BYTE scaler_mode)
{
	WORD hTotal,hActive,hStart,hSync;
	WORD vTotal,vActive,vStart,vSync,vBPorch;

	BYTE index;
	BYTE vfreq;
	WORD table_hTotal;
	WORD wTemp;

	t_VIDEO_TIME *pVidTime;
	struct s_DTV_table *pVid;

	if(MeasStartMeasure()) {
		//something wrong.
		dPrintf("==>FAIL!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		return;
	}			
	pVidTime = &VideoTime;
			

	//===========================================
	// read measured value & adjust it.
	//===========================================
	
	//Horizontal
	hTotal = MeasGetVsyncRisePos();
	hActive = MeasGetHActive(&wTemp);
	vfreq  = MeasGetVFreq();
	hSync = MeasGetHSyncRiseToFallWidth();
	if(hTotal < hActive)
		//if input is an interlaced, it can be a measured value on the odd field.
		//in this case, make it double.
		hTotal *=2;	
	hStart = wTemp+4; //add HW offset value. hStart can be hBPorch or hSync+hBPorch.

	//Vertical
	vTotal = MeasGetVPeriod();;
	vActive = MeasGetVActive(&wTemp);
	vSync = MeasGetVSyncRiseToFallWidth();
	vStart = wTemp-1; //subtract HW offset value. vStart can be vBPorch or vSync+vBPorch.
	vBPorch = vStart - vSync;

	//print measured value
	Printf("\n\rMeas H hActive:%d hTotal:%d hStart:%d hSync:%d ", 					hActive,hTotal,hStart,hSync);
	Printf("\n\r     V vActive:%d vTotal:%d vStart:%d vSync:%d vBPorch:%d vfreq:%bd", vActive,vTotal,vStart,vSync, vBPorch, vfreq);

	DtvSetPolarity(0,0);
	//add "1" for overscan on vActive
	InputSetCrop(hStart, vStart, hActive, vActive+1);
	Printf("\n\rCrop H Pol:%bd Start:%d Active:%d", 0, hStart,hActive);
	Printf("\n\r     V Pol:%bd Start:%d Active:%d", 0, vStart,vActive+1);

	if(scaler_mode==SCALER_MODE_720X480P) {
		ScalerSetScaleRate(hActive,vActive,720,480);
		ScalerSetOutputTimeRegs(VID_720X480P_IDX, vStart, vActive);
	}
	else {
		ScalerSetScaleRate(hActive,vActive,800,480);
		ScalerSetOutputTimeRegs(VID_800X480P_IDX, vStart, vActive);
	}
	//debug.....
	//search
	for(index=0; index < DTV_TABLE_MAX; index++) {
		pVid = &DTV_table[index];
		if((vTotal-2) <= pVid->vTotal && (vTotal+2) >= pVid->vTotal) {
			table_hTotal = pVid->hTotal;
			if((hTotal-2) <=  table_hTotal && (hTotal+2) >= table_hTotal) {
				ePuts("\n\r=1=>");
				break;
			}
			if(pVid->fIorP==1) {
				table_hTotal >>= 1;
				if((hTotal-2) <=  table_hTotal && (hTotal+2) >= table_hTotal) {
					ePuts("\n\r=2=>");
					break;
				}

			}
			//==check active. If ext device give an active low sync polarity, we can not use a hTotal.
			table_hTotal = pVid->hDeWidth;
			if((hActive-2) <=  table_hTotal && (hActive+2) >= table_hTotal) {
				ePuts("\n\r=3=>");
				break;
			}
		}
	}
	if(index >= DTV_TABLE_MAX) {
		Printf("\n\r=>GiveUp");
		return;
	}			
	//pVid = &DTV_table[index];

	Printf("\n\rInput DTV_table[%bd]	",index);
	Printf("%bd %dx%d%s@%bd POL:%02bx   %d,%d,%d,%d   %d,%bd,%bd,%bd,%bd %ld",
		pVid->vid,
		pVid->hDeWidth,pVid->vDeWidth,	pVid->fIorP == 1 ? "I" : "P", 
		pVid->vfreq == FREQ_60 ? 60 : pVid->vfreq == FREQ_50 ? 50 : 0,
		pVid->Pol,
		pVid->hTotal,	(WORD)pVid->hFPorch,(WORD)pVid->hSyncWidth,(WORD)pVid->hBPorch,
		pVid->vTotal,	pVid->vPad,pVid->vSyncStart,pVid->vSyncWidth,pVid->vBPorch,
		pVid->pixelfreq
		);

//	InputSetCrop(hStart, vStart, hActive, vActive+1);		
	Printf("\n\rCrop H Pol:%bd start:%d hActive:%d", DTV_table[index].Pol >> 4,   hStart,hActive);
	Printf("\n\r     V Pol:%bd vstart:%d vActive:%d",DTV_table[index].Pol & 0x0F, vStart,vActive);
	if(scaler_mode==SCALER_MODE_720X480P) {
		ScalerSetScaleRate(hActive,vActive,720,480);
		ScalerSetOutputTimeRegs(VID_720X480P_IDX, vStart, vActive);
	}
	else {
		ScalerSetScaleRate(hActive,vActive,800,480);
		ScalerSetOutputTimeRegs(VID_800X480P_IDX, vStart, vActive);
	}

}
#endif

#if 0 //working....121128
void ScalerTest_DTV(void)
{
	WORD hstart,vstart;
	WORD hActive,vActive;
	WORD htotal,vtotal;


//	BYTE offset, VPulse, HPulse;
//	BYTE HPol, VPol;
//	WORD Meas_HPulse,Meas_VPulse;


	BYTE index;
	BYTE vfreq;
	WORD table_hTotal;
//	BYTE bTemp;
//	BYTE speed;
	WORD wTemp;

	t_VIDEO_TIME *pVidTime;
	pVidTime = &VideoTime;

	if(MeasStartMeasure()) {
		//something wrong.
		dPrintf("==>FAIL!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		return;
	}			
			
	DtvSetPolarity(0,0);
	//we have a measured value.
	//read hTotal and vTotal then find out DTV mode.
	htotal = MeasGetVsyncRisePos();
	vtotal = MeasGetVPeriod();
	vfreq  = MeasGetVFreq();
	Printf("\n\rMeas %dx%d@%bd:", htotal,vtotal,vfreq);


	//===========================================
	// read measured value & adjust it.
	//===========================================

	pVidTime->hDeWidth = MeasGetHActive(&wTemp);
	pVidTime->hBPorch = wTemp;

	pVidTime->vDeWidth = MeasGetVActive(&wTemp);
	pVidTime->vBPorch = wTemp;

	//pVidTime->fIorP = 
	pVidTime->vfreq = vfreq;
	//pVidTime->Pol = 

	pVidTime->hTotal = htotal;
	pVidTime->hFPorch = htotal - MeasGetHSyncRiseToHActiveEnd();
	pVidTime->hSyncWidth = MeasGetHSyncRiseToFallWidth();
	
	pVidTime->vTotal = vtotal;
	pVidTime->vSyncWidth = MeasGetVSyncRiseToFallWidth();
	pVidTime->vPad = vtotal - pVidTime->vSyncWidth - pVidTime->vBPorch - pVidTime->vDeWidth;
	pVidTime->vSyncStart = 0;

	dPrintf("\n\rMeasured");
	dPrintf("\n\rH total:%4d FPorch:%d Sync:%d BPorch:%d active:%d",  
		pVidTime->hTotal,
		(WORD)pVidTime->hFPorch,	   	//temp
		(WORD)pVidTime->hSyncWidth,	//temp
		(WORD)pVidTime->hBPorch,
		pVidTime->hDeWidth);
	dPrintf("\n\rV total:%4d FPorch:%d Sync:%d BPorch:%d active:%d",  
		pVidTime->vTotal,
		(WORD)pVidTime->vPad + pVidTime->vSyncStart,	   //temp
		(WORD)pVidTime->vSyncWidth,
		(WORD)pVidTime->vBPorch,
		pVidTime->vDeWidth);		

	//
	//start adjust the measured value.
	//
	//check vfreq
	if(pVidTime->vfreq==59) 	
		pVidTime->vfreq = FREQ_60;
	//check interlaced......1080i incorrect.
	pVidTime->fIorP = 0; //'P';
	if(htotal < pVidTime->hDeWidth) {
		pVidTime->fIorP = 1; //'I';
		htotal *= 2;
		pVidTime->hFPorch = htotal - MeasGetHSyncRiseToHActiveEnd();
	}
	//check polarity.
	if ( pVidTime->hSyncWidth > (pVidTime->hTotal/2) ) {
		//pVidTime->hSyncWidth = pVidTime->hTotal - pVidTime->hSyncWidth;
		pVidTime->hBPorch = pVidTime->hDeWidth - MeasGetHSyncRiseToFallWidth();
		pVidTime->hFPorch = MeasGetHSyncRiseToFallWidth() - pVidTime->hDeWidth - pVidTime->hBPorch;
		//FAIL:::pVidTime->hSyncWidth = ??
		//FAIL:::oVidTime->htotal==??
		//You have to use invert H polarity on DTV and assign only hBPorch.
		pVidTime->Pol = 0x10;	
	}
	else  {
		pVidTime->hBPorch += 4;
		pVidTime->hBPorch -= pVidTime->hSyncWidth;

		pVidTime->Pol = 0x00;
	}
	if ( pVidTime->vSyncWidth > (pVidTime->vTotal/2) ) {
		pVidTime->vSyncWidth = pVidTime->vTotal - MeasGetVSyncRiseToFallWidth();
		MeasGetVActive(&wTemp);
		pVidTime->vBPorch = wTemp - 1;
		//pVidTime->vFPorch = pVidTime->vTotal - pVidTime->vSyncWidth - pVidTime->vBPorch - pVidTime->vDeWidth;
		pVidTime->vPad = pVidTime->vTotal - pVidTime->vSyncWidth - pVidTime->vBPorch - pVidTime->vDeWidth;
		pVidTime->Pol |= 0x01;
	}
	else  {
		pVidTime->vBPorch -= 1;
		pVidTime->vBPorch -= pVidTime->vSyncWidth;
		pVidTime->vPad = pVidTime->vTotal - pVidTime->vSyncWidth - pVidTime->vBPorch - pVidTime->vDeWidth;
	}
	//pVidTime->vPad = pVidTime->vTotal - pVidTime->vSyncWidth - pVidTime->vBPorch - pVidTime->vDeWidth;
	//
	//dPrintf("\n\rAdjusted1");
	//dPrintf("\n\rH total:%4d FPorch:%d Sync:%d BPorch:%d active:%d Pol:%bx",  
	//	pVidTime->hTotal,
	//	(WORD)pVidTime->hFPorch,	   	//temp
	//	(WORD)pVidTime->hSyncWidth,	//done
	//	(WORD)pVidTime->hBPorch,
	//	pVidTime->hDeWidth,
	//	pVidTime->Pol >> 4);
	//dPrintf("\n\rV total:%4d FPorch:%d Sync:%d BPorch:%d active:%d Pol:%bx",  
	//	pVidTime->vTotal,
	//	(WORD)pVidTime->vPad + pVidTime->vSyncStart,
	//	(WORD)pVidTime->vSyncWidth,	//done
	//	(WORD)pVidTime->vBPorch,
	//	pVidTime->vDeWidth,
	//	pVidTime->Pol & 0x0F);		

	//adjust measure module delay value.
	//pVidTime->hBPorch += 4;
	//pVidTime->hBPorch -= pVidTime->hSyncWidth;
	//pVidTime->vBPorch -= 1;
	//pVidTime->vBPorch -= pVidTime->vSyncWidth;
	//pVidTime->vPad = pVidTime->vTotal - pVidTime->vSyncWidth - pVidTime->vBPorch - pVidTime->vDeWidth;

	dPrintf("\n\rAdjusted2 ");
	if(pVidTime->Pol & 0x10) {
		dPrintf("\n\rH total:???? FPorch:%d Sync:?? BPorch:%d active:%d Pol:%bx",  
			//pVidTime->hTotal,
			(WORD)pVidTime->hFPorch,
			//(WORD)pVidTime->hSyncWidth,
			(WORD)pVidTime->hBPorch,
			pVidTime->hDeWidth,
			pVidTime->Pol >> 4);
	}
	else {
		dPrintf("\n\rH total:%4d FPorch:%d Sync:%d BPorch:%d active:%d Pol:%bx",  
			pVidTime->hTotal,
			(WORD)pVidTime->hFPorch,	//temp
			(WORD)pVidTime->hSyncWidth,	//done
			(WORD)pVidTime->hBPorch,
			pVidTime->hDeWidth,
			pVidTime->Pol >> 4);
	}
	dPrintf("\n\rV total:%4d FPorch:%d Sync:%d BPorch:%d active:%d Pol:%bx",  
		pVidTime->vTotal,
		(WORD)pVidTime->vPad + pVidTime->vSyncStart,
		(WORD)pVidTime->vSyncWidth,	//done
		(WORD)pVidTime->vBPorch,
		pVidTime->vDeWidth,
		pVidTime->Pol & 0x0F);		









	//search

	for(index=0; index < DTV_TABLE_MAX; index++) {
		if((vtotal-2) <= DTV_table[index].vTotal && (vtotal+2) >= DTV_table[index].vTotal) {
			table_hTotal = DTV_table[index].hTotal;
			if((htotal-2) <=  table_hTotal && (htotal+2) >= table_hTotal) {
				Printf("=>index[%bd] %dx%d", index, DTV_table[index].hDeWidth, DTV_table[index].vDeWidth);
				break;
			}
			if(DTV_table[index].fIorP==1) {
				table_hTotal >>= 1;
				if((htotal-2) <=  table_hTotal && (htotal+2) >= table_hTotal) {
					Printf("=>index[%bd] %dx%dI", index, DTV_table[index].hDeWidth, DTV_table[index].vDeWidth*2);
					break;
				}

			}
		}
	}
	if(index >= DTV_TABLE_MAX) {
		Printf("=>GiveUp");
		return;
	}	
		

	//===============
	// DTV09SetInputCrop
	//================
	DtvSetPolarity(DTV_table[index].Pol >> 4,DTV_table[index].Pol & 0x0F);

	hstart = DTV_table[index].hBPorch;
	if((DTV_table[index].Pol & 0x10) ==0)
		hstart += DTV_table[index].hSyncWidth;

	vstart = DTV_table[index].vBPorch;
	if((DTV_table[index].Pol & 0x01) ==0)
		vstart += DTV_table[index].vSyncWidth;

	hActive = DTV_table[index].hDeWidth;
	vActive = DTV_table[index].vDeWidth;
	//if(index == VID_720X480I_IDX || index == VID_720X576I_IDX) {
	//	hstart /= 2;
	//	hActive /= 2;
	//	vActive *= 2;
	//}
	InputSetCrop(hstart, vstart, hActive, vActive);
	Printf("\n\rCrop hstart:%d hActive:%d",hstart,hActive);
	Printf("\n\r     vstart:%d vActive:%d",vstart,vActive);

	//====================
	// DTV SetOutput
	//====================
	ScalerSetScaleRate(DTV_table[index].hDeWidth,DTV_table[index].vDeWidth,800,480);
	ScalerSetOutputTimeRegs(VID_800X480P_IDX, (WORD)vstart, DTV_table[index].vDeWidth);
}
#endif
#if 0
//--------------successed code------------------
void ScalerTest_DTV(void)
{
	WORD hstart,vstart;
	WORD hActive,vActive;
	WORD htotal,vtotal;


	BYTE offset, VPulse, HPulse;
	BYTE HPol, VPol;
	WORD Meas_HPulse,Meas_VPulse;


	BYTE index;
	BYTE vfreq;
	WORD table_hTotal;
//	BYTE bTemp;
//	BYTE speed;
	WORD wTemp;

	t_VIDEO_TIME *pVidTime;
	pVidTime = &VideoTime;

	if(MeasStartMeasure()) {
		//something wrong.
		dPrintf("==>FAIL!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		return;
	}			
			
	DtvSetPolarity(0,0);
	//we have a measured value.
	//read hTotal and vTotal then find out DTV mode.
	htotal = MeasGetVsyncRisePos();
	vtotal = MeasGetVPeriod();
	vfreq  = MeasGetVFreq();
	Printf("\n\rMeas %dx%d@%bd:", htotal,vtotal,vfreq);


	//===========================================
	// read measured value & adjust it.
	//===========================================

	pVidTime->hDeWidth = MeasGetHActive(&wTemp);
	pVidTime->hBPorch = wTemp;

	pVidTime->vDeWidth = MeasGetVActive(&wTemp);
	pVidTime->vBPorch = wTemp;

	//pVidTime->fIorP = 
	if(vfreq==59 || vfreq==60) 	pVidTime->vfreq = FREQ_60;
	else						pVidTime->vfreq = FREQ_50;	//assume
	//pVidTime->Pol = 

	pVidTime->fIorP = 0; //'P';
	if(htotal < pVidTime->hDeWidth) {
		pVidTime->fIorP = 1; //'I';
		htotal *= 2;
	}
	pVidTime->hTotal = htotal;
	pVidTime->hFPorch = htotal - MeasGetHSyncRiseToHActiveEnd();
	pVidTime->hSyncWidth = MeasGetHSyncRiseToFallWidth();
	
	pVidTime->vTotal = vtotal;
	pVidTime->vSyncWidth = MeasGetVSyncRiseToFallWidth();
	pVidTime->vPad = vtotal - pVidTime->vSyncWidth - pVidTime->vBPorch - pVidTime->vDeWidth;
	pVidTime->vSyncStart = 0;


	//search

	for(index=0; index < DTV_TABLE_MAX; index++) {
		if((vtotal-2) <= DTV_table[index].vTotal && (vtotal+2) >= DTV_table[index].vTotal) {
			table_hTotal = DTV_table[index].hTotal;
			if((htotal-2) <=  table_hTotal && (htotal+2) >= table_hTotal) {
				Printf("=>index[%bd] %dx%d", index, DTV_table[index].hDeWidth, DTV_table[index].vDeWidth);
				break;
			}
			if(DTV_table[index].fIorP==1) {
				table_hTotal >>= 1;
				if((htotal-2) <=  table_hTotal && (htotal+2) >= table_hTotal) {
					Printf("=>index[%bd] %dx%dI", index, DTV_table[index].hDeWidth, DTV_table[index].vDeWidth*2);
					break;
				}

			}
		}
	}
	if(index >= DTV_TABLE_MAX) {
		Printf("=>GiveUp");
		return;
	}	
		

	//===============
	// DTV09SetInputCrop
	//================
	DtvSetPolarity(DTV_table[index].Pol >> 4,DTV_table[index].Pol & 0x0F);

	hstart = DTV_table[index].hBPorch;
	if((DTV_table[index].Pol & 0x10) ==0)
		hstart += DTV_table[index].hSyncWidth;

	vstart = DTV_table[index].vBPorch;
	if((DTV_table[index].Pol & 0x01) ==0)
		vstart += DTV_table[index].vSyncWidth;

	hActive = DTV_table[index].hDeWidth;
	vActive = DTV_table[index].vDeWidth;
	//if(index == VID_720X480I_IDX || index == VID_720X576I_IDX) {
	//	hstart /= 2;
	//	hActive /= 2;
	//	vActive *= 2;
	//}
	InputSetCrop(hstart, vstart, hActive, vActive);
	Printf("\n\rCrop hstart:%d hActive:%d",hstart,hActive);
	Printf("\n\r     vstart:%d vActive:%d",vstart,vActive);

	//========================
	//debug area


#ifdef DEBUG_DTV
#endif

	dPrintf("\n\rMeasured");
	dPrintf("\n\rH total:%4d FPorch:%d Sync:%d BPorch:%d active:%d",  
		pVidTime->hTotal,
		(WORD)pVidTime->hFPorch,	   	//temp
		(WORD)pVidTime->hSyncWidth,	//temp
		(WORD)pVidTime->hBPorch,
		pVidTime->hDeWidth);
	dPrintf("\n\rV total:%4d FPorch:%d Sync:%d BPorch:%d active:%d",  
		pVidTime->vTotal,
		(WORD)pVidTime->vPad + pVidTime->vSyncStart,	   //temp
		(WORD)pVidTime->vSyncWidth,
		(WORD)pVidTime->vBPorch,
		pVidTime->vDeWidth);		

	if ( pVidTime->hSyncWidth > (pVidTime->hTotal/2) ) {
		pVidTime->hSyncWidth = pVidTime->hTotal - pVidTime->hSyncWidth;
		pVidTime->Pol = 0x10;	
	}
	else  {
		pVidTime->Pol = 0x00;
	}

	// v meas delay value:-1.
	if ( pVidTime->vSyncWidth > (pVidTime->vTotal/2) ) {
		pVidTime->vSyncWidth = pVidTime->vTotal - pVidTime->vSyncWidth;
		pVidTime->Pol |= 0x01;
	}
	else  {
	}
	pVidTime->vPad = pVidTime->vTotal - pVidTime->vSyncWidth - pVidTime->vBPorch - pVidTime->vDeWidth;

	dPrintf("\n\rAdjusted1");
	dPrintf("\n\rH total:%4d FPorch:%d Sync:%d BPorch:%d active:%d Pol:%bx",  
		pVidTime->hTotal,
		(WORD)pVidTime->hFPorch,	   	//temp
		(WORD)pVidTime->hSyncWidth,	//done
		(WORD)pVidTime->hBPorch,
		pVidTime->hDeWidth,
		pVidTime->Pol >> 4);
	dPrintf("\n\rV total:%4d FPorch:%d Sync:%d BPorch:%d active:%d Pol:%bx",  
		pVidTime->vTotal,
		(WORD)pVidTime->vPad + pVidTime->vSyncStart,
		(WORD)pVidTime->vSyncWidth,	//done
		(WORD)pVidTime->vBPorch,
		pVidTime->vDeWidth,
		pVidTime->Pol & 0x0F);		

	pVidTime->hBPorch += 4;
	pVidTime->hBPorch -= pVidTime->hSyncWidth;
	pVidTime->vBPorch -= 1;
	pVidTime->vBPorch -= pVidTime->vSyncWidth;
	pVidTime->vPad = pVidTime->vTotal - pVidTime->vSyncWidth - pVidTime->vBPorch - pVidTime->vDeWidth;

	dPrintf("\n\rAdjusted2");
	dPrintf("\n\rH total:%4d FPorch:%d Sync:%d BPorch:%d active:%d Pol:%bx",  
		pVidTime->hTotal,
		(WORD)pVidTime->hFPorch,	   	//temp
		(WORD)pVidTime->hSyncWidth,	//done
		(WORD)pVidTime->hBPorch,
		pVidTime->hDeWidth,
		pVidTime->Pol >> 4);
	dPrintf("\n\rV total:%4d FPorch:%d Sync:%d BPorch:%d active:%d Pol:%bx",  
		pVidTime->vTotal,
		(WORD)pVidTime->vPad + pVidTime->vSyncStart,
		(WORD)pVidTime->vSyncWidth,	//done
		(WORD)pVidTime->vBPorch,
		pVidTime->vDeWidth,
		pVidTime->Pol & 0x0F);		


	//====================
	// DTV SetOutput
	//====================
	ScalerSetScaleRate(DTV_table[index].hDeWidth,DTV_table[index].vDeWidth,800,480);
	ScalerSetOutputTimeRegs(VID_800X480P_IDX, (WORD)vstart, DTV_table[index].vDeWidth);
}
#endif


//mode
//	0:Original Scaler
//	1:New method
//	2:target 720x480p for BT656
//void ScalerTest(BYTE mode)
//{
//	//scaler test
//	if(mode) {
//		switch(InputMain) {
//		case INPUT_CVBS:
//			ScalerTest_Decoder(mode);
//			break;
//		case INPUT_SVIDEO:
//			ScalerTest_Decoder(mode);
//			break;
//		case INPUT_COMP:
//			ScalerTest_Component(mode);
//			break;
//		case INPUT_PC:
//			ScalerTest_PC(mode);
//			break;
//		case INPUT_DVI:
//			ScalerTest_DTV(mode);
//			break;
//		case INPUT_HDMIPC:
//		case INPUT_HDMITV:
//			ScalerTest_DTV(mode);
//			break;
//		case INPUT_BT656:
//			ScalerTest_DTV(mode);
//			break;
//		case INPUT_LVDS:
//			ScalerTest_DTV(mode);
//			break;
//		}
//	}
//	else {
//		//CheckAndSetInput will recover the test value.
//		CheckAndSetInput();
//	}
//}

//void BUGGY_PclkSetFunc(DWORD freq)
//{
//	BYTE freq1m;
//  
//	freq1m = freq/1000000L;		//base:1MHz
//
//	//adjust pclko divider. see SetDefaultPClock()
//#if defined(PANEL_AUO_B133EW01) || defined(PANEL_TM070DDH01)
////	PclkoSetDiv( (ppf+PANEL_PCLK_TYP -1) / PANEL_PCLK_TYP - 1);
////  BKTODO: I need 1.5 divider.
//#else
//	PclkoSetDiv( (freq1m+39) / 40 - 1); //pixel clock polarity : Invert 0:div1, 1:div2, 2:div3
//										//BKTODO:move pixel clock polarity...	
//#endif
//#if 1 //BK131028
////remove
//#else
//	PclkoSetPolarity(1);	//invert
//#endif
//
//}
