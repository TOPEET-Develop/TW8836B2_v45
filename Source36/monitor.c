/**
 * @file
 * Monitor.c 
 * @author Harry Han
 * @author YoungHwan Bae
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	Interface between TW_Terminal2 and Firmware.
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

/*  TW-Dongle2 LED.
    D2 D3   mode   D4:Command  D5-Connect
    1  0    I2C
    0  1    UART
    1  1    SPI
*/

//*****************************************************************************
//
//								Monitor.c
//
//*****************************************************************************
//																
//
//#include <intrins.h>

#include "config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "global.h"
#include "cpu.h"
#include "printf.h"	
#include "util.h"
#include "monitor.h"
#include "monitor_MCU.h"
#include "monitor_SPI.h"
#include "monitor_MENU.h"
#include "monitor_ChipTest.h"
		  
#include "i2c.h"
#include "spi.h"
#include "I2cSpi.h"

#include "main.h"
#include "SOsd.h"
#include "FOsd.h"
#include "Measure.h"
#include "Settings.h"
#include "Remo.h"
#include "scaler.h"
#ifdef SUPPORT_DELTA_RGB
#include "DeltaRGB.h"
#endif
#include "InputCtrl.h"
#include "ImageCtrl.h"
#include "TouchKey.h"
#include "measure.h"

#include "Decoder.h"
#include "aRGB.h"
#include "DTV.h"
#include "EEPROM.H"

#include "SOsdMenu.h"

#if defined(SUPPORT_HDMI_EP907M)
#include "HDMI_EP907M.h"
#include "HDMI_EP9351.h"
#include "HDMI_EP9553.h"
#endif

#include "BT656.h"
#include "Demo.h"


		BYTE 	DebugLevel;
XDATA	BYTE	MonAddress = TW88I2CAddress;	
XDATA	WORD	MonIndex;
XDATA	BYTE	MonIndexLen;
XDATA	DWORD	MonWdata;
XDATA	BYTE	MonDataLen;
XDATA	BYTE	monstr[50];				// buffer for input string
XDATA	BYTE 	*argv[12];
XDATA	BYTE	argc=0;
		bit		echo=1;
		bit		g_access=1;
XDATA	BYTE	SW_key;

#ifdef SUPPORT_UART1
XDATA	WORD	Mon1Index;
XDATA	BYTE	Mon1IndexLen;
XDATA	DWORD	Mon1Wdata;
XDATA	BYTE	Mon1DataLen;
XDATA	BYTE	mon1str[40];			// buffer for input string
XDATA	BYTE 	*argv1[10];
XDATA	BYTE	argc1=0;
#endif

// function declear
void Test_Checkclock2(void);
void Test_Checkclock(void);
void Test_McuSpeed(WORD count);

#ifdef SUPPORT_DELTA_RGB
static void monitor_auo(void)
{
	if( argc==1 )	Puts( "\r\nIncorrect commad - AUO r ii or AUO w ii ddd" );
	else {
		if( !stricmp( argv[1], "w" ) ) {
			WORD val;
			val= a2h(argv[3]);
			Printf("\r\n SPI Write: Addr:%2x  Data:%4x", (WORD)a2h(argv[2]), val);
			WriteAUO(a2h(argv[2]), val );
		}
		else if( !stricmp( argv[1], "r" ) ) {
			WORD val;
			val = ReadAUO(a2h(argv[2]));
			Printf("\r\n SPI Read: Addr:%2x  Data:%4x", (WORD)a2h(argv[2]), val);
		}
	}
}
static void monitor_auo1(void)
{
		if( argc==1 )	Puts( "\r\nIncorrect commad - AUO2 r ii or AUO2 w ii ddd" );
		else {
			
			BYTE val, addr;
			if( !stricmp( argv[1], "w" ) ) {

				addr = a2h( argv[2] );
				val= a2h( argv[3] );
				Printf("\r\n SPI Write: Addr:%02bx  Data:%02bx", addr, val);
				WriteAUO2( addr, val );
			}
			else if( !stricmp( argv[1], "r" ) ) {
				addr = a2h( argv[2] );
				val = ReadAUO2( addr );
				Printf("\r\n SPI Read: Addr:%02bx  Data:%02bx", addr, val);
			}
		}
}
#endif

//==============================================
// BT656 TEST routine
//==============================================
#if	defined(SUPPORT_BT656)
static void monitor_bt656(void)
{
	if(argc < 2) {
		Printf("\n\rusage: BT656 {off|dec|argb|dtv|lvds|panel|loop}");
		Printf("\n\r\tBT656 off		; off");
		Printf("\n\r\tBT656 dec		; decoder, svideo");
		Printf("\n\r\tBT656 argb		; component, PC");
		Printf("\n\r\tBT656 dtv		; hdmi");
		Printf("\n\r\tBT656 lvds      ; lvds");
		Printf("\n\r\tBT656 panel     ; panel");
		Printf("\n\r\tBT656 loop      ; loopback if available");
		Printf("\n\r\tBT656 ?         ; current info");
		return;
	}
	if( !stricmp( argv[1], "?" ) ) {
		Printf("\n\rDo you need BT656 Info ?..");
		BT656Enc_Info();
	}
	else if( !stricmp( argv[1], "off" ) )
		BT656Enc_Enable(OFF);
	else if(!stricmp( argv[1], "dec" ))
		BT656Enc_Setup(BT656ENC_SRC_DEC);
	else if (!stricmp( argv[1], "aRGB" ))
		BT656Enc_Setup(BT656ENC_SRC_ARGB);
	else if (!stricmp( argv[1], "dtv" )) 
		BT656Enc_Setup(BT656ENC_SRC_DTV);
	else if (!stricmp( argv[1], "lvds" ))
		BT656Enc_Setup(BT656ENC_SRC_LVDS);
	else if (!stricmp( argv[1], "panel" ))
		BT656Enc_Setup(BT656ENC_SRC_PANEL);
	else if (!stricmp( argv[1], "loop" ))
		ChangeBT656__MAIN(BT656ENC_SRC_AUTO);
	else {
		Printf("\n\r invalid...");
	}
}
#endif

static void monitor_bt656e(void)
{
	BYTE mode;
	if(argc < 2) mode = 0;
	else 		 mode = a2i( argv[1] );
	if(mode > 5)
		mode = 0;
	BT656_InitExtEncoder(mode);
}

static void monitor_checkandset(void)
{
	BYTE ret;

	switch(InputMain) {
	case INPUT_CVBS:	
#if defined(SUPPORT_FAST_INPUT_TOGGLE)
		g_cvbs_checked = 0;
#endif
	case INPUT_SVIDEO:	ret=CheckAndSetDecoderScaler();	break;
#ifdef SUPPORT_COMPONENT
	case INPUT_COMP:	ret=CheckAndSetComponent();		break;
#endif
#ifdef SUPPORT_PC
	case INPUT_PC:		ret=CheckAndSetPC();			break;
#endif
#ifdef SUPPORT_DVI
	case INPUT_DVI:		ret=CheckAndSetDVI();			break;
#endif
#if defined(SUPPORT_HDMI)
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
#if defined(SUPPORT_FAST_INPUT_TOGGLE)
						g_hdmi_checked = 0;
#endif
						ret= CheckAndSetHDMI();			break;
#endif
#ifdef SUPPORT_BT656_LOOP
	case INPUT_BT656:	ret = CheckAndSetBT656Loop();		break;
#endif
#ifdef SUPPORT_LVDSRX
	case INPUT_LVDS:	ret = CheckAndSetLVDSRx();	break;
#endif
	default:			ret = CheckAndSetUnknown();		break;
	}
	if(ret==ERR_SUCCESS) {
		//success
		VInput_enableOutput(0);
		Puts("\n\r==>SUCCESS");
	} else {
		//------------------
		// NO SIGNAL
		// Prepare NoSignal Task...
		VInput_gotoFreerun(0);
		Puts("\n\r==>FAIL");
	}
}
static void monitor_checkspeed(void)
{
		WORD count;
		if(argc < 2)	count = 100;
		else 			count = a2h( argv[1] );
		Test_McuSpeed(count);
}
static void monitor_check(void)
{
	if(argc < 2) {
		Puts("\n\r pclk sclk clock meas");
#ifdef SUPPORT_PC
		Puts("\n\r Phase|Color");
#endif
	}
	else if ( !stricmp( argv[1], "pclk" ))
		AdjustSSPLL_with_HTotal();
	else if ( !stricmp( argv[1], "sclk" )) {
		BYTE count;
		if(argc < 3) count = 32;
		else		 count = a2i(argv[2]);
		if(count < 8)
			count = 8;
		CheckSpiClock(count);
	}
#ifdef SUPPORT_PC
	else if ( !stricmp( argv[1], "PHASE" )|| !stricmp( argv[1], "PH" ))
		AutoTunePhase();
	else if ( !stricmp( argv[1], "COLOR" ))
		AutoTuneColor();
#endif	
	else if ( !stricmp( argv[1], "clock" ))
		DumpClock();
	else if ( !stricmp( argv[1], "clock1" )) 
		Test_Checkclock();
	else if ( !stricmp( argv[1], "clock2" )) 
		Test_Checkclock2();
	else if ( !stricmp( argv[1], "speed" )) 
		monitor_checkspeed();
	else if ( !stricmp( argv[1], "meas" )) 
		Measure_VideoTiming();
}



//----------------------------------------------------
//make compiler happy.
//Please, !!!DO NOT EXECUTE!!!
//----------------------------------------------------	

static void monitor_compiler(void)
{
		extern void dummy_i2c_code(void);
		extern void dummy_remo_code(void);
		extern void dummy_misc_code(void) ;
		extern void dummy_argb_code(void) ;
		extern void dummy_osdspi_code(void) ;
#ifndef SUPPORT_FOSD_MENU
		extern void Dummy_FosdMenu_func(void);
		extern void Dummy_FosdInitTable_func(void);
		extern void Dummy_FosdDispInfo_func(void);
		extern void Dummy_FosdString_func(void);
#endif
#ifdef SUPPORT_FOSD_MENU
		extern BYTE CheckAndClearFOsd(void);
#endif
#if !defined(SUPPORT_DVI) 
		extern void Dummy_DTV_func(void);
#endif
		WORD wTemp;
		BYTE bTemp;

		//add dummp code
		dummy_i2c_code();
		dummy_remo_code();
		dummy_misc_code(); 
		dummy_argb_code(); 
		dummy_osdspi_code(); 
//#ifdef USE_SFLASH_EEPROM
//		E3P_SetBanks(4);
//		E3P_SetBufferSize(SPI_BUFFER_SIZE);			//128
//		E3P_SetSize(EE_INDEX_PER_BLOCK,EE_BLOCKS);	//512 bytes 64 * 8.
//#endif
//		MonitorChipTest();
		wPuts("\n\rwPuts");
		delay1s(1, __LINE__);
		//----------------------
		// util.c
		//----------------------
		TWmemset((BYTE *)&wTemp, 0, 0);
		TWmemcmp(&bTemp,&bTemp,0);

		//----------------------
		// main.c
		//----------------------
		//----------------------
		// host.c
		//----------------------

		//----------------------
		// I2C
		//--------------------
#if defined(SUPPORT_I2C_MASTER)
		CheckI2C(0x8A);
//		WriteI2CS(0x8A, 0, &bTemp, 0);
		ReadI2CS(0x8A, 0, &bTemp, 0);
//		WriteI2CSI16(0x78, 0, &bTemp, 0);

		ReadI2CS_multi(0x78, 0x21, 0, &bTemp, 0);
		WriteI2CS_multi(0x78, 0x21, 0, &bTemp,0);
#endif
        ReadI2CS_8A(0, 1, &bTemp);

		//--------------------
		// Scaler
		//--------------------
//		ScalerSetHScaleWithRatio(0,0);	
//		ScalerSetVScaleWithRatio(0,0);
		ScalerReadOutputWidth();
#if defined(SUPPORT_PC) || defined(SUPPORT_DVI) || defined(SUPPORT_HDMI)
   		ScalerReadXDownReg();
#endif

		//--------------------
		// Setting
		//--------------------
		PclkoSetDiv(1);
		Sspll1SetFreq(108000000L, 0);
		SpiClk_overclocking(0);
		SpiClk_SetSync();
		PllClkGetSource();
		SpiClk_GetMinAsyncWaitValue(2);
		SpiClk_SetAsync(2,3, ON,ON);
		SpiClkReadSource();
		SpiClkSetSource(0);

		//--------------------
		// Measure
		//--------------------
		MeasSetErrTolerance(0);
		MeasEnableChangedDetection(0);
//		MeasEnableDeMeasure(0);
//		MeasSetThreshold(0);
		MeasGetVPeriod();
		MeasGetHTotal(1);
		MeasGetHSyncRiseToFallWidth();
		MeasGetHSyncRiseToHActiveEnd();
		MeasGetVSyncRiseToFallWidth();
		MeasGetVsyncRisePos();
		MeasGetHActive(&wTemp);
		
		MeasGetVActive(&wTemp);
#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC) || defined(SUPPORT_DVI) 
		MeasGetVPeriod27();
#endif
#if defined(SUPPORT_DVI)
		CheckMeasure();
#endif
		// 
		//--------------------
#if !defined(SUPPORT_COMPONENT) && !defined(SUPPORT_PC)
//		Dummy_ARGB_func();
#endif
#if !defined(SUPPORT_DVI) && !defined(SUPPORT_HDMI)
		Dummy_DTV_func();
#endif

#if 0	//lgnq
#ifndef SUPPORT_HDMI_EP907M
		Dummy_HDMI_EP907M_func();
#endif
#endif

		//----------------------------
		//Trick for Bank Code Segment
		//----------------------------
#ifndef SUPPORT_FOSD_MENU
		Dummy_FosdMenu_func();
		Dummy_FosdInitTable_func();
		Dummy_FosdDispInfo_func();
		Dummy_FosdString_func();
#endif
#ifdef SUPPORT_FOSD_MENU
		CheckAndClearFOsd();
#endif

    //SpiFlash_wait_cmddone(0, 0,0);
    //SpiFlash_DmaCmd(0, 0, 0, 0,0);
    SpiFlash_4B_DmaCmd(0);

    //SpiFlash_DmaCmd_ChipReg(0, 0, 0, 0, 0);
    //SpiFlash_DmaCmd_Xmem(0, 0, 0, 0, 0, 0);

    SpiFlash_SectorErase(0);
    SpiFlash_BlockErase(0);
    SpiFlash_PageProgram_XMem(0,0, 0);
    SpiFlash_PageProgram_ChipReg(0, &bTemp, 0);
    SpiFlash_FastRead_ChipReg(0,0);
    SpiFlash_FastRead_Fixed_ChipReg(0,0);
    SpiFlash_Read_XMem(0, 0, 0); 
    
    SpiFlash_Read_FOsd(0, 0, 0); 
    SpiFlash_Read_SOsd(0, 0, 0);


}


#ifdef SUPPORT_DELTA_RGB
static void monitor_delta(void)
{
	extern	BYTE	SPI_ID;
	if( argc==1 )	DeltaRGBPanelInit();
	else {
		
		if( !stricmp( argv[2], "w" ) ) {

			WORD val;
			//val= Asc2Bin(argv[4])<<8 | Asc2Bin(argv[5]);
			val= a2h(argv[4]);
			Printf("\r\n SPI Write: Addr:%2x  Data:%4x", (WORD)a2h(argv[2]), val);
			WriteSPI(a2h(argv[3]), val );
		}
		else if( !stricmp( argv[2], "r" ) ) {
			WORD val;
			val = ReadSPI(a2h(argv[3]));
			Printf("\r\n SPI Read: Addr:%2x  Data:%4x", (WORD)a2h(argv[3]), val);
		}
		else if( !stricmp( argv[2], "c" ) ) {
			if ( argc == 4 ) 
				SPI_ID = a2h(argv[3]) & 0xfc;
			Printf("\r\n SPI Device ID:%2bx", SPI_ID);
		}
	}
}
#endif

static void monitor_fosd(void)
{
	if(argc < 2) {
	}
#if 0
	else if ( !stricmp( argv[1], "LUT0" )) {
		BYTE	page, i;

		// Win4 enable and OSD RAM set
		WriteTW88(REG305, 0 );
		WriteTW88(REG30B, 0 );
		WriteTW88(REG30C, 0x3f );
		WriteTW88(REG340, 0xcb );
		WriteTW88(REG345, 0x01 );
		WriteTW88(REG346, 0x10 );


		// Enable OSD RAM
		WriteTW88(REG300, 0x11 );
		WriteTW88(REG305, 0 );
		WriteTW88(REG306, 0 );
		WriteTW88(REG304, 0 );
		WriteTW88(REG307, 0 );
		for ( i=0; i<16; i++ ) {
			FOsdRamSetAddress((WORD)i);	//WriteTW88(REG306, i );
			WriteTW88(REG307, 0 );
			WriteTW88(REG308, i );
			delay1ms(10);
		}

#if 0
		// download Font
		WriteTW88(REG300, 0x11 );
		WriteTW88(REG304, 0x0d );
		WriteTW88(REG_FOSD_CHEIGHT, 0x0d /* 26/2 */ );
		WriteTW88(REG_FOSD_MUL_CON, 0x34 );
		WriteTW88(REG309, 0x00 );

		for ( i=0; i<13; i++ ) {
			WriteTW88(REG30A, 0x00 );
			WriteTW88(REG30A, 0x00 );
			WriteTW88(REG30A, 0xff );
			WriteTW88(REG30A, 0xff );
		}
		for ( i=0; i<13; i++ ) {
			WriteTW88(REG30A, 0x00 );
			WriteTW88(REG30A, 0xff );
			WriteTW88(REG30A, 0x00 );
			WriteTW88(REG30A, 0xff );
		}
		
		color = 0;
		for ( i=0; i<64; i++ ) {
			WriteTW88(REG30c, i );
			WriteTW88(REG30D, color>>8 );
			WriteTW88(REG30E, color );

		}
#endif
	}
	else if ( !stricmp( argv[1], "LUTB" )) {	 // blue
		BYTE	page, i;
		WORD	color;

		color = 0;
		for ( i=0; i<64; i++ ) {
			WriteTW88(REG30c, i );
			WriteTW88(REG30d, color>>8 );
			WriteTW88(REG30e, color );
			if ( i%2 ) color ++;
		}
	}
	else if ( !stricmp( argv[1], "LUTC" )) {	 // clear = black
		BYTE	page, i;
		WORD	color;

		color = 0;
		for ( i=0; i<64; i++ ) {
			WriteTW88(REG30c, i );
			WriteTW88(REG30d, color>>8 );
			WriteTW88(REG30e, color );
		}
	}	
	else if ( !stricmp( argv[1], "LUTF" )) {	 // font download
		BYTE	page, i;

		// download Font
		WriteTW88(REG300, 0x11 );
		WriteTW88(REG304, 0x0d );
		WriteTW88(REG305, 0x00 );
		WriteTW88(REG309, 0x00 );
		WriteTW88(REG30B, 0x00 );
		WriteTW88(REG_FOSD_CHEIGHT, (0x1a >> 1) );	 //?? 0x1A=26 
		WriteTW88(REG_FOSD_MUL_CON, 0x34 );
		WriteTW88(REG_FOSD_MADD3, 0x00 );
		WriteTW88(REG_FOSD_MADD4, 0x00 );

		for ( i=0; i<13; i++ ) {
			WriteTW88(REG30a, 0x00 );
			WriteTW88(REG30a, 0x00 );
			WriteTW88(REG30a, 0xff );
			WriteTW88(REG30a, 0xff );
			delay1ms( 10 );
		}
		for ( i=0; i<13; i++ ) {
			WriteTW88(REG30a, 0x00 );
			WriteTW88(REG30a, 0xff );
			WriteTW88(REG30a, 0x00 );
			WriteTW88(REG30a, 0xff );
			delay1ms( 10 );
		}
		for ( i=0; i<52; i++ ) {
			WriteTW88(REG30a, 0x33 );
			delay1ms( 2 );
		}
	}
	else if ( !stricmp( argv[1], "LUTG" )) {	 // blue
		BYTE	page, i;
		WORD	color;


		color = 0;
		for ( i=0; i<64; i++ ) {
			WriteTW88(REG30c, i );
			WriteTW88(REG30d, color>>8 );
			WriteTW88(REG30e, color );
			color += 0x20;
		}
	}
	else if ( !stricmp( argv[1], "LUTR" )) {	 // red
		BYTE	page, i;
		WORD	color;

		color = 0;
		for ( i=0; i<64; i++ ) {
			WriteTW88(REG30c, i );
			WriteTW88(REG30d, color>>8 );
			WriteTW88(REG30e, color );
			if ( i%2 ) color += 0x800;
		}
	}
	else if ( !stricmp( argv[1], "LUTW" )) {	 // white
		BYTE	i;
		WORD	color;

		color = 0;
		for ( i=0; i<64; i++ ) {
			WriteTW88(REG30c, i );
			WriteTW88(REG30d, color>>8 );
			WriteTW88(REG30e, color );
			color += 0x20;
			if ( i%2 ) color += 0x801;
		}
	}
	else if ( !stricmp( argv[1], "RAMFONT" )) {
			FOsdDownloadFontCode();
	}
#endif	
	else if( !stricmp( argv[1], "testfont" ) )	 {
		extern void TestInitFontRam(WORD start);
		WORD start;
		start = a2h( argv[2] );
		TestInitFontRam(start);
	}	
}


#if 0
static void monitor_grid(void)
{
	if( !stricmp( argv[1], "ttest" ) ) {
		extern void TestDParkGridAction(void);
		TestDParkGridAction();
	}
	else if( !stricmp( argv[1], "auto" ) ) {
		extern void TestAutoDParkGridAction(BYTE positionX, BYTE positionY);
		BYTE positionX, positionY;

		if(argc==4) {
			positionX = a2h(argv[2]);
			positionY = a2h(argv[3]);
		}
		else {
			positionX = 22;
			positionY = 5;
		}
		TestAutoDParkGridAction(positionX, positionY);
	}
	else if( !stricmp( argv[1], "step" ) ) {
		extern void TestStepDParkGridAction(BYTE positionX, BYTE positionY);
		BYTE positionX, positionY;

		if(argc==4) {
			positionX = a2h(argv[2]);
			positionY = a2h(argv[3]);
		}
		else {
			positionX = 22;
			positionY = 5;
		}
		TestStepDParkGridAction(positionX, positionY);
	}

}
#endif


static void monitor_hdmi(void)
{
		//BYTE val[20];
		//BYTE cnt = 1, i;
		//if ( argc > 2 )
		//	cnt	= a2h(argv[2]);
		//if ( cnt > 20 )
		//	cnt = 20;
		//ReadI2C(I2CID_EP9351, a2h(argv[1]), val, cnt);
		//for	(i=0; i<cnt; i++) 
		//	Printf("\n\r%3bd %02bx", i, val[i]);
		if(argc < 2) {
			Printf(" init start avi timereg dnedid dnhdcp");
		}

	//init
	//info
//	else if( !stricmp( argv[0], "HDINFO" ) ) {
//		BYTE val[13];
//		WORD tmp;
//		ReadI2C(I2CID_EP9351, 0x3B, val, 13);
//		if ( val[12] & 0x80 ) 
//			Puts("\n\rInterlace Video Signal");
//		else
//			Puts("\n\rNON-Interlace Video Signal");
//		tmp = val[1]; 		tmp <<= 8;		tmp += val[0];
//		Printf("\n\rActive Pixels Per Line = %d", tmp ); 
//		tmp = val[3]; 		tmp <<= 8;		tmp += val[2];
//		Printf("\n\rHorizontal Front Porch = %d", tmp ); 
//		tmp = val[5]; 		tmp <<= 8;		tmp += val[4];
//		Printf("\n\rHorizontal Back Porch  = %d", tmp ); 
//		tmp = val[7]; 		tmp <<= 8;		tmp += val[6];
//		Printf("\n\rHorizontal Pulse Width = %d", tmp ); 
//		tmp = val[9]; 		tmp <<= 8;		tmp += val[8];
//		Printf("\n\rActive Lines Per Frame = %d", tmp ); 
//		Printf("\n\rVertical Front Porch   = %bd", val[10] ); 
//		Printf("\n\rVertical Back Porch    = %bd", val[11] ); 
//		Printf("\n\rVertical Pulse Width   = %bd", val[12]&0x7f ); 
//	}
}


#if defined(SUPPORT_I2C_MASTER)
/*
TW8836 recommends to use below step to read a byte data.
	[S] 8A <index> [S] 8B <read data> [P]
But, linux or most the other programs using below two steps.
	[S] 8A <index> [P]
	[S] 8B <read data> [P]
This routine is to validate i2c_smbus_write_byte and i2c_smbus_read_byte.

TW8836 has a Page, that can support 12bit index.
To read the Page value, 
	i2c w 8a ff
	i2c r 8a
To set the Page as 4,
	i2c w 8f ff 04

So, to read the REG000 value(Page 0, index 0x00) that has 0x36 Device ID value,
use below commands.

	i2c w 8a ff 00
	i2c w 8a 00
	i2c r 8a

If you want to read the other index value on the same page, 
you donot need to write the Page again
If you want to read index 0x01 register on the same Page,
use below commands.
	i2c w 8a 01
	i2c r 8a
*/
/* BK150602 
To validate some mixed combination, I used below steps.
I2C ID 0x40 is a GPIO Expender chip.
This routine show, it can add the other i2c command between two read steps.	
	i2c w 8a ff 00
	i2c w 8a 00
	i2c w 40 07 AB
	i2c	r 8a
*/
static void monitor_i2c(void)
{
	if(argc==1) {
		Printf("\n\rI2C {check|delay|BUS|search}");
		Printf("\n\rI2C w <i2cid> <index> [<data>...]");
		Printf("\n\rI2C r <i2cid>");
		Printf("\n\rI2C rn <i2cid> <number>");
		return;
	}
	if(!stricmp(argv[1],"check")) {
		BYTE id,temp;
		if(argc < 3) {
			Printf("\n\rI2C check devid");
			return;
		}
		id = (BYTE)a2h( argv[2] );
		temp=CheckI2C(id);
		//Printf("\n\rresult:%bx",temp);
		Puts("\n\rresult: ");
		if(temp==0) Puts("Pass");
		else        Puts("Fail");
		Printf("  I2C_delay_base:%bd", I2C_delay_base);	
	}
	else if(!stricmp(argv[1],"bus")) {
	}
	else if(!stricmp(argv[1],"delay")) {
		//BYTE temp;
        if(argc < 3) {
            Printf(" is %bd",I2C_delay_base);
            return;
        }
        I2C_delay_base = (BYTE)a2h( argv[2] );
		//if(argc < 4) {
		//	Printf("\n\ri2c delay start:%d",i2c_delay_start);
		//	Printf("\n\r          restart:%d",i2c_delay_restart);
		//	Printf("\n\r          datasetup:%d",i2c_delay_datasetup);
		//	Printf("\n\r          clockhigh:%d",i2c_delay_clockhigh);
		//	Printf("\n\r          datahold:%d",i2c_delay_datahold);
		//	return;
		//}
	//	temp = (BYTE)a2h( argv[3] );
	//	if(!stricmp(argv[2],"start"))
	//		i2c_delay_start = temp;
	//	else if(!stricmp(argv[2],"restart"))
	//		i2c_delay_restart = temp;
	//	else if(!stricmp(argv[2],"datasetup"))
	//		i2c_delay_datasetup = temp;
	//	else if(!stricmp(argv[2],"clockhigh"))
	//		i2c_delay_clockhigh = temp;
	//	else if(!stricmp(argv[2],"datahold"))
	//		i2c_delay_datahold = temp;
	//	else 
	//		Printf("\n\rubkbown");
	}
	else if(!stricmp(argv[1],"w")) { /* wirte command */
		BYTE i2cid,value;
		BYTE cnt,i;
		if(argc < 4) {
			Printf("\n\rI2C w devid index(or data)...");
			return;
		}
		i2cid = (BYTE)a2h( argv[2] );
		value = (BYTE)a2h( argv[3] );  /* index or data */
		cnt = argc - 4;
		for(i=0;i<cnt;i++)
            SPI_Buffer[i]=(BYTE)a2h( argv[i+4] );
        WriteI2CS(i2cid, value, SPI_Buffer, cnt);
	}
	else if(!stricmp(argv[1],"r")) { /* read command */
		BYTE i2cid,value;
		if(argc < 3) {
			Printf("\n\rI2C r devid");
			return;
		}
		i2cid = (BYTE)a2h( argv[2] );
        value=ReadI2C_Only(i2cid);
		Printf("=>%02bx", value);
	}
	else if(!stricmp(argv[1],"rn")) { /* sequecial read command */
		BYTE i2cid; //,value;
		WORD i,n;

		if(argc < 4) {
			Printf("\n\rI2C rn devid hex_number");
			return;
		}
		i2cid = (BYTE)a2h( argv[2] );
		n = (BYTE)a2h( argv[3] );
        /* we only have 128byte. If you use bigger, it will be crashed */
        /* I just want to show you how it works */
        ReadI2CS_Only(i2cid, SPI_Buffer, n);
        for(i=0; i < n; i++) {
			Printf(" %02bx", SPI_Buffer);
        }
	}
	else if(!stricmp(argv[1],"search")) {
		BYTE id,temp;
		for(id=2; id != 0x00; id+=2) {	//0x100 means 0x00 on BYTE.
			temp=CheckI2C(id);
			if(temp==0)
				Printf("\n\rFind device at %02bx",id);
		}
	}  
}
#endif

#if 0
void i2cspi_readstatus(WORD wait)
{
	volatile BYTE vdata;
	BYTE i,j;

	WriteI2C_8A(0xFF,0x04);
	WriteI2C_8A(I2C8REG4F3, 0x40 | 0x01);	//cmd len:1 + target:Reg4D0

	WriteI2C_8A(I2C8REG4F6, 0x04);	//Buff 0x4D0
	WriteI2C_8A(I2C8REG4F7,	0xD0); 	

	WriteI2C_8A(I2C8REG4F5, 0);		//length high
	WriteI2C_8A(I2C8REG4F8,	0); 	//length middle
	WriteI2C_8A(I2C8REG4F9,	1); 	//length low
	WriteI2C_8A(I2C8REG4FA, SPICMD_RDSR);
	for(i=0; i<100;i++) {
		WriteI2C_8A(I2C8REG4F4, 0x01 | SPI_CMD_OPT_NONE);
		for(j=0; j<100;j++) {
			vdata = ReadI2C_8A(I2C8REG4F4) & 0x07;
			if((vdata & 0x01)==0) {
				/* */
				Printf("==>Success i:%bd,j:%bd ",i,j);
				/* read 4D0 */
				vdata = ReadI2C_8A(0xD0); 

				if((vdata & 0x01)==0) {
					Printf("Done\n");
					return;
				}
				else {
					Printf("Busy\n");
					break;
				}
			}
		}
		if(wait)
			delay1ms(wait);
	}
	Puts("Fail\n");
	return;	
}
#endif 


void monitor_i2cspi_help(void)
{
	Puts("\n\ri2cspi uses EXT_MCU_SCLK(SDAT) for master, SCLK(SDAT) for slave");
	Puts("\n\rstep tw8836->rdid->mcu halt->4B Ex->clock 27 or type myenv");
	Puts("\n\r");
	Puts("\n\ri2cspi tw8836");
	Puts("\n\ri2cspi rdid");
	Puts("\n\ri2cspi 4B [En|Ex]");
	Puts("\n\ri2cspi clock [27|PLL]");
	Puts("\n\ri2cspi mcu [halt|rerun]");
	Puts("\n\ri2cspi crc [spiaddr length]");
    Puts("\n\r  example: i2c delay 0");
    Puts("\n\r         : i2cspi 0 10000");
	Puts("\n\ri2cspi se spiaddr");
	Puts("\n\ri2cspi be spiaddr");
    Puts("\n\ri2cspi swreset");
    Puts("\n\ri2cspi lvreset");
	Puts("\n\ri2cspi upload src_at_master dest_at_slave size");
	Puts("\n\r   example: i2cspi upload 60000 0 107E1");
	Puts("\n\ri2cspi xcopy addr_master_to_slave size");
	Puts("\n\r   example: i2cspi xcopy 0 30000");
	Puts("\n\r");
	Puts("\n\ri2cspi xmem [on|off]");
	Puts("\n\ri2cspi xmem d start length");
	Puts("\n\ri2cspi xmem 00 start length");
	Puts("\n\ri2cspi xmem 0F start length");
	Puts("\n\ri2cspi xmem AB start length");
	Puts("\n\ri2cspi xr spiaddr length");
	Puts("\n\ri2cspi xw spiaddr length");
}

/*
To download a image from CPU to Spiflash on TW8836 EVB, follow up below steps.
1. erase spiflash
	use sector erase or block erase
	for example: i2cspi se 60000
2. send data from CPU to XMEM(max 2048Bytes)
   CPU have to generate CRC to compare it later.
	I generate some pattern to prepare the data.
	for example: i2cspi xmem 0F 0 100
3. write data from xmem to spiflash
	for example: i2cspi xw 60000 100
4. generate CRC with a fixed read command, and then read CRC value.
	for example: i2cspi crc 60000 100
5. compare CRC of wrote data with this read back CRC.
	if these are same, the programmed data should be correct.

To validate these i2cspi command, I am using two TW8836B EVB boards.
Slave board does not have I2C Master.
Master board using EXT_MCU_SCLK and EXT_MCU_SDAT to download its firmware.
So, remove R26,R29, and install R27,R30 on TW8836EVB REV1.1.
And, connect TW-Dongle's I2C at J21 on Master board.
Connect Slave's I2C at J20 to Master's I2C at J20.
Turn on Slave first, and then turn on Master.
*/
static void monitor_i2cspi(void)
{
	BYTE bTemp;

	if(argc==1) {
        monitor_i2cspi_help();
		return;
	}
	if(!stricmp(argv[1],"myenv")) {
        I2C_delay_base=0; /* speed up*/

        WriteI2C_8A(0xFF,0x00);
        bTemp=ReadI2C_8A(0x00);
        if(bTemp != 0x36) {
           Puts("=>Fail");
           return; 
        }
        bTemp=I2CSPI_ReadId_to_chipreg();
        if(bTemp) {
            Puts("=>Fail");
            return;
        }
        bTemp=I2CSPI_mcu_halt_rerun(1);
        if(bTemp) { 
            Printf("=>Fail");
            return;
        }
        I2cSpiFlash_4B_DmaCmd(SPICMD_EX4B);
        WriteI2C_8A(0xFF,0x04);
        bTemp = ReadI2C_8A(0xE1);
        bTemp &= ~0x30;
        WriteI2C_8A(0xE1, bTemp);
    }
	else if(!stricmp(argv[1],"tw8836")) {
        WriteI2C_8A(0xFF,0x00);
        bTemp=ReadI2C_8A(0x00);
        if(bTemp != 0x36)   Puts("=>Fail");
        else                Puts("=>Found");
    }
	else if(!stricmp(argv[1],"rdid")) {
        bTemp=I2CSPI_ReadId_to_chipreg();
        if(bTemp) Puts("=>Fail");
        else {
            WriteI2C_8A(0xFF,0x04);
            Printf(" %02BX %02BX %02BX",
                ReadI2C_8A(0xD0),
                ReadI2C_8A(0xD1),
                ReadI2C_8A(0xD2));
        }
    }
	else if(!stricmp(argv[1],"mcu")) {
		if(argc <= 2) {
			WriteI2C_8A(0xFF,0x04);
			bTemp = ReadI2C_8A(0xC4);
			if((bTemp & 0x80)==0) Puts("=>Halted");
			else                  Puts("=>Runing");
		}
		else if(!stricmp(argv[2],"halt")) {
			BYTE bTemp;
            bTemp=I2CSPI_mcu_halt_rerun(1);
            if(bTemp) 
                Printf("=>Fail");
		}
		else if(!stricmp(argv[2],"rerun")) {
			volatile BYTE bTemp;
			WORD i;
            I2CSPI_mcu_halt_rerun(0);
			for(i=0; i<1000;i++) {
				bTemp = ReadI2C_8A(0xC4);
				if((bTemp & 0x80)) 
					Printf("=>Success @%d",i);
			}
            if(i==1000)
                Puts("=>Fail");
		}
		else {
			Printf("\n\runknown [%s] command",argv[2]);
		}
	}
	else if(!stricmp(argv[1],"4B")) {
		if(argc <= 2) {
			if(I2CSPI_4B_mode) Puts(" ON");
			else               Puts(" OFF");
		}
		else if(!stricmp(argv[2],"Ex"))
            I2cSpiFlash_4B_DmaCmd(SPICMD_EX4B);
		else if(!stricmp(argv[2],"En"))
            I2cSpiFlash_4B_DmaCmd(SPICMD_EN4B);
		else
			Printf("\n\runknown [%s] command",argv[2]);
    }
	else if(!stricmp(argv[1],"clock")) {
		if(argc <= 2) {                                                  
            WriteI2C_8A(0xFF,0x04);
            bTemp = ReadI2C_8A(0xE1);
			if((bTemp & 0x30)==0) Puts(" 27MHz");
			else                  Puts(" PLL");
		}
		else if(!stricmp(argv[2],"27")) {
            WriteI2C_8A(0xFF,0x04);
            bTemp = ReadI2C_8A(0xE1);
            bTemp &= ~0x30;
            WriteI2C_8A(0xE1, bTemp);
		}
        else if(!stricmp(argv[2],"PLL")) {
            WriteI2C_8A(0xFF,0x04);
            bTemp = ReadI2C_8A(0xE1);
            bTemp &= ~0x30;
            bTemp |= 0x20;
            WriteI2C_8A(0xE1, bTemp);
        }
		else
			Printf("\n\runknown [%s] command",argv[2]);
    }
	else if(!stricmp(argv[1],"crc")) {
		/*	example:
			read current CRC register:
				>i2cspi crc
			read CRC from spiaddr 0x60000 length:0x100
				>i2cspi crc 60000 100	
		*/
		DWORD start,length;
		BYTE loop;
        WORD test_crc;
        WORD result_crc;

		if(argc <= 3) {
			//read current value.
			WriteI2C_8A(0xFF,0x04);
            result_crc = ReadI2C_8A(0xEE);
            result_crc <<= 8;
            result_crc |= ReadI2C_8A(0xEF);
            Printf("\nCRC:%04X", result_crc);
			return;
		}
		start = a2h(argv[2]);
		length = a2h(argv[3]);
		Printf("\n\rcheck_crc %06lx %lx ",start,length);
        if(length >= 0x100000) {
            /* bigger than 1MByte */
            /* FW using only 64Mbyte */
            loop= 255;
        } 
        else {
            loop = (BYTE)(length>>12);
            if(loop<10)
                loop=10;
        }
        bTemp = I2CSPI_check_crc(start, length, loop , 0);
        /* read CRC */
        test_crc = ReadI2C_8A(0xEE);
        test_crc <<= 8;
        test_crc |= ReadI2C_8A(0xEF);
        Printf("\nIgnore previous error and Get CRC[%04X]", test_crc);

        Puts("..compare..");
        bTemp = I2CSPI_check_crc(start, length, loop , test_crc);
        result_crc = ReadI2C_8A(0xEE);
        result_crc <<= 8;
        result_crc |= ReadI2C_8A(0xEF);
        Printf("[%04X]", result_crc);
        if(bTemp) 
            Puts("=>Fail");
        else if(test_crc != result_crc) 
            Puts("=>Fail");
        else
            Puts("=>Success");
		return;
	}
	else if(!stricmp(argv[1],"se")) {
		DWORD spiaddr;

		if(argc <=2 ) {
			monitor_i2cspi_help();
			return;
		}
		//check xmem access
		WriteI2C_8A(0xFF,0x04);
		bTemp = ReadI2C_8A(0xC2);
		if(bTemp & 0x02) {
			Puts("\nPlease turn off Xmem access.");
			return; 
		}

		spiaddr = a2h(argv[2]);
		Printf("\nSE %06lx", spiaddr); 

		bTemp=I2CSPI_SectorErase(spiaddr);
        if(bTemp)
            Puts("=>Fail");
	}
	else if(!stricmp(argv[1],"be")) {
		DWORD spiaddr;

		if(argc <=2 ) {
			monitor_i2cspi_help();
			return;
		}
		//check xmem access
		WriteI2C_8A(0xFF,0x04);
		bTemp = ReadI2C_8A(0xC2);
		if(bTemp & 0x02) {
			Puts("\nPlease turn off Xmem access.");
			return; 
		}

		spiaddr = a2h(argv[2]);
		Printf("\nBE %06lx", spiaddr); 

		bTemp=I2CSPI_BlockErase(spiaddr);
        if(bTemp)
            Puts("=>Fail");
	}
	else if(!stricmp(argv[1],"swreset")) {
        I2CSPI_SW_reset();
    }
	else if(!stricmp(argv[1],"lvreset")) {
        I2CSPI_LV_reset();
    }
	else if(!stricmp(argv[1],"download")) {
		DWORD src_addr,dest_addr,upload_len;
		BYTE ret;

		if(argc <= 4) {
			Puts("\nUsage:i2cspi download src dest len");
			return;
		}
		src_addr = a2h(argv[2]);
		dest_addr = a2h(argv[3]);
		upload_len = a2h(argv[4]);

		ret = I2CSPI_download_main(src_addr, dest_addr, upload_len);
		if(ret) Puts("\n==>FAIL");
		else	Puts("\n==>SUCCESS");
	}
	else if(!stricmp(argv[1],"xcopy")) {
		DWORD src_addr,upload_len;
		BYTE ret;

		if(argc <= 3) {
			Puts("\nUsage:i2cspi copy master2slave_address len");
			return;
		}
		src_addr = a2h(argv[2]);
		upload_len = a2h(argv[3]);

		ret = I2CSPI_xcopy_main(src_addr, upload_len);
		if(ret) Puts("\n==>FAIL");
		else	Puts("\n==>SUCCESS");
	}
    //================================================
	else if(!stricmp(argv[1],"xmem")) {
		if(argc <= 2) {
			WriteI2C_8A(0xFF,0x04);
			bTemp = ReadI2C_8A(0xC2);
			if((bTemp & 0x02)==0) 	Puts("=>Disabled");
			else					Puts("=>Enabled");
		}
		else if(!stricmp(argv[2],"on")) {
			WriteI2C_8A(0xFF,0x04);
			bTemp = ReadI2C_8A(0xC2);
			WriteI2C_8A(0xC2, bTemp | 0x01);
		}
		else if(!stricmp(argv[2],"off")) {
			WriteI2C_8A(0xFF,0x04);
			bTemp = ReadI2C_8A(0xC2);
			WriteI2C_8A(0xC2, bTemp & ~0x01);
		}
		else if(!stricmp(argv[2],"d")) {
			WORD wTemp,i;
			if(argc <=4 ) {
				Puts("\n\ri2cspi xmem d command needs start and length");
				return;
			}
			WriteI2C_8A(0xFF,0x04);
			bTemp = ReadI2C_8A(0xC2);
			if((bTemp & 0x02)==0) {
				WriteI2C_8A(0xC2, bTemp | 0x01);
				for(i=0; i < 100; i++) {
					bTemp = ReadI2C_8A(0xC2);  //need volatile
					if(bTemp & 0x02)
						break;
					delay1ms(10);
				}
				if(i==100) {
					Puts(" !!status error!!");
					return;
				}
			}

			wTemp = a2h(argv[3]); //start 12bit
			WriteI2C_8A(0xFF,0x04);
			WriteI2C_8A(0xDB,(BYTE)(wTemp>>8));
			WriteI2C_8A(0xDC,(BYTE)wTemp);
			Printf("\n\rdump start:0x%x",wTemp);
			wTemp = a2h(argv[4]);	//length Max 2KByte
			Printf(" length:0x%x",wTemp);
			for(i=0; i<wTemp;i++) {
				if((i % 16)==0) Printf("\n\r%03X:",i);
				bTemp=ReadI2C_8A(0xDD);
				Printf("%02bx ",bTemp);
			}
			//disable XMEM access
			WriteI2C_8A(0xFF,0x04);
			bTemp = ReadI2C_8A(0xC2);
			WriteI2C_8A(0xC2, bTemp & ~0x01);
		}
		else if(!stricmp(argv[2],"00")
			 || !stricmp(argv[2],"0F")
		     || !stricmp(argv[2],"FF")) {
			WORD wTemp,i;
			BYTE pattern;
			if(argc <=4 ) {
				Puts("\n\ri2cspi xmem [00|FF] command needs start and length");
				return;
			}
			pattern = a2h(argv[2]);

			WriteI2C_8A(0xFF,0x04);
			bTemp = ReadI2C_8A(0xC2);
			if((bTemp & 0x02)==0) {
				WriteI2C_8A(0xC2, bTemp | 0x01);
				for(i=0; i < 100; i++) {
					bTemp = ReadI2C_8A(0xC2);  //need volatile
					if(bTemp & 0x02)
						break;
					delay1ms(10);
				}
				if(i==100) {
					Puts(" !!status error!!");
					return;
				}
			}

			wTemp = a2h(argv[3]); //start 12bit
			WriteI2C_8A(0xFF,0x04);
			WriteI2C_8A(0xDB,(BYTE)(wTemp>>8));
			WriteI2C_8A(0xDC,(BYTE)wTemp);
			Printf("\n\rset start:0x%x",wTemp);
			wTemp = a2h(argv[4]);	//length Max 2KByte
			Printf(" length:0x%x",wTemp);
			if(pattern==0x0F) {
				Printf(" pattern:increase");
				for(i=0; i<wTemp;i++) {
					WriteI2C_8A(0xDD,(BYTE)i);
				}
			}
			else {
				Printf(" pattern:%02bx",pattern);
				for(i=0; i<wTemp;i++) {
					WriteI2C_8A(0xDD,pattern);
				}
			}
			//disable XMEM access
			WriteI2C_8A(0xFF,0x04);
			bTemp = ReadI2C_8A(0xC2);
			WriteI2C_8A(0xC2, bTemp & ~0x01);
		}
	}
	else if(!stricmp(argv[1],"xr")) {
		DWORD spiaddr;
		WORD length;
		BYTE ret;
		//BYTE i;

		if(argc <=3 ) {
			Puts("\n\ri2cspi xr command needs start and length");
			return;
		}
		//check xmem access
		WriteI2C_8A(0xFF,0x04);
		bTemp = ReadI2C_8A(0xC2);
		if(bTemp & 0x02)
			Puts("\nPlease turn off Xmem access.");

		spiaddr = a2h(argv[2]);
		length = a2h(argv[3]);
		if(length > 2048) {
			Puts(" max length is 0x800");
			length = 2048;
		}
		//use single read
		Printf("\nREAD %06lx %x",spiaddr, length);
		WriteI2C_8A(0xFF,0x04);
		WriteI2C_8A(I2C8REG4F3, 0xC4);	//XMEM,Increase,len:1+3
		WriteI2C_8A(I2C8REG4F6, 0x00);	//XMEM start 0
		WriteI2C_8A(I2C8REG4F7, 0x00);
		WriteI2C_8A(I2C8REG4F5, (BYTE)(length>>16));
		WriteI2C_8A(I2C8REG4F8,	(BYTE)(length>>8));
		WriteI2C_8A(I2C8REG4F9,	(BYTE)length);
		WriteI2C_8A(I2C8REG4FA, SPICMD_READ_SLOW);
		WriteI2C_8A(I2C8REG4FB, (BYTE)(spiaddr>>16));
		WriteI2C_8A(I2C8REG4FC, (BYTE)(spiaddr>>8));
		WriteI2C_8A(I2C8REG4FD, (BYTE)spiaddr);
		WriteI2C_8A(I2C8REG4F4, 0x01 | SPI_CMD_OPT_BUSY);
        ret=I2CSPI_IsCmdAccepted(SPI_CMD_OPT_BUSY,100,10,ON);
        if(ret==100)    Puts("=>Fail");
		else            Puts("=>Done");

		Printf("\nTo check, use i2cspi xmem d 0 %x",length);
	}
	else if(!stricmp(argv[1],"xw")) {
		/*
			program data from xmem to spiflash
			before you execute a program, the spiflash must be erased.
			example: program xmem to spiflash 0x60000 length 0x100
				>i2cspi xw 60000 100			 				
		*/
		DWORD spiaddr;
		WORD length;
		BYTE ret;

		if(argc <=3 ) {
			Puts("\n\ri2cspi xw command needs start and length");
			return;
		}
		//check xmem access
		WriteI2C_8A(0xFF,0x04);
		bTemp = ReadI2C_8A(0xC2);
		if(bTemp & 0x02)
			Puts("\nPlease turn off Xmem access.");

		spiaddr = a2h(argv[2]);
		length = a2h(argv[3]);
		if(length > 2048) {
			Puts(" max length is 0x800");
			length = 2048;
		}
        ret = I2CSPI_WriteEnable();
		if(ret) {
			Puts("\nWREN fail");
			return;
		}

		//Write uses PP
		Printf("\nPP %06lx %x",spiaddr, length);
		WriteI2C_8A(0xFF,0x04);
		WriteI2C_8A(I2C8REG4F3, 0xC4);	//XMEM,Increase,len:1+3
		WriteI2C_8A(I2C8REG4F6, 0x00);	//XMEM start 0
		WriteI2C_8A(I2C8REG4F7, 0x00);
		WriteI2C_8A(I2C8REG4F5, (BYTE)(length>>16));
		WriteI2C_8A(I2C8REG4F8,	(BYTE)(length>>8));
		WriteI2C_8A(I2C8REG4F9,	(BYTE)length);
		WriteI2C_8A(I2C8REG4FA, SPICMD_PP);
		WriteI2C_8A(I2C8REG4FB, (BYTE)(spiaddr>>16));
		WriteI2C_8A(I2C8REG4FC, (BYTE)(spiaddr>>8));
		WriteI2C_8A(I2C8REG4FD, (BYTE)spiaddr);
		WriteI2C_8A(I2C8REG4F4, 0x01 | SPI_CMD_OPT_WRITE_BUSY);
#if 0
		ret=I2cSpiFlashDmaWait(300);
		if(ret) Puts("=>Fail");
		else    Puts("=>Done");
#else
    	ret=I2CSPI_IsCmdAccepted(SPI_CMD_OPT_WRITE_BUSY, 5,0, OFF);
    	if(ret==5) {
    		Puts("\nPP fail ");
    		//return 1;
    	}
        else {
        	ret=I2CSPI_wait_busy_with_rdsr(SPICMD_PP,10,2, OFF); /*0.5~5msec*/
        	if(ret) {
        		Puts("\nPP wait fail");
        		//return 1;
        	}
            else
                Puts("=>Done");
        }
#endif

	}
#if 0
	else if(!stricmp(argv[1],"mytest")) {
		DWORD src_addr,dest_addr,upload_len;
		BYTE ret;

        I2CSPI_test_big_dma();
        I2CSPI_LV_reset();

        src_addr=0x60000;
        dest_addr =0;
        upload_len = 0x2F800;
		ret = I2CSPI_download_main(src_addr, dest_addr, upload_len);
		if(ret) Puts("\n==>FAIL");
		else	Puts("\n==>SUCCESS");
    }
#endif
//#if 0
//	else if(!stricmp(argv[1],"bese")) {
//		/*
//	    customer complains WriteEnable can not clear REG4F4[0] between BE and SE.
//		*/
//		BYTE ret;
//		DWORD spiaddr;
//		//DWORD UsedTime;
//
//		if(argc <=2 ) {
//			Puts("\n\ri2cspi be/se command needs start_address");
//			return;
//		}
//		
//
//		//check xmem access
//		WriteI2C_8A(0xFF,0x04);
//		bTemp = ReadI2C_8A(0xC2);
//		if(bTemp & 0x02)
//			Puts("\nPlease turn off Xmem access.");
//
//		spiaddr = a2h(argv[2]);
//		Printf("\nBE:%06lx SE:%06lx", spiaddr, spiaddr+0x10000); 
//
//		//Do write_enable
//		WriteI2C_8A(0xFF,0x04);
//		WriteI2C_8A(I2C8REG4F3, (DMA_DEST_CHIPREG << 6) +1);	//len:1
//		WriteI2C_8A(I2C8REG4F5, 0);	//length high
//		WriteI2C_8A(I2C8REG4F8,	0); //length middle
//		WriteI2C_8A(I2C8REG4F9,	0); //length low
//		WriteI2C_8A(I2C8REG4FA, SPICMD_WREN);
//		WriteI2C_8A(I2C8REG4F4, 0x01 | SPI_CMD_OPT_NONE);	
//		ret=I2cSpiFlashDmaWait(2); //20ms
//		if(ret) {
//			Puts("\nWREN fail before BE");
//			return;
//		}
//		//BE
//		//Printf("\nBE %06lx %x",spiaddr);
//		WriteI2C_8A(0xFF,0x04);
//		WriteI2C_8A(I2C8REG4F3, (DMA_DEST_CHIPREG << 6) + 1+3);	//Reg,Increase,len:1+3
//		WriteI2C_8A(I2C8REG4F6, 0x04);	//Reg start 0
//		WriteI2C_8A(I2C8REG4F7, 0xD0);
//		WriteI2C_8A(I2C8REG4F5, 0);	//length high
//		WriteI2C_8A(I2C8REG4F8,	0); //length middle
//		WriteI2C_8A(I2C8REG4F9,	0); //length low
//		WriteI2C_8A(I2C8REG4FA, SPICMD_BE);
//		WriteI2C_8A(I2C8REG4FB, (BYTE)(spiaddr>>16));
//		WriteI2C_8A(I2C8REG4FC, (BYTE)(spiaddr>>8));
//		WriteI2C_8A(I2C8REG4FD, (BYTE)spiaddr);
//
////		SFRB_ET0 = 0;
////		UsedTime = SystemClock;
////		SFRB_ET0 = 1;
//
//
//		//check how long it take.
//		WriteI2C_8A(I2C8REG4F4, 0x01 | SPI_CMD_OPT_WRITE_BUSY);
//		ret = I2cSpiFlashDmaWait_loop(SPI_CMD_OPT_WRITE_BUSY, 200);
//		if(ret==200) {
//			Puts("\nBE fail");
//			return;
//		}
//		else {
//			Printf("\nBE success at %bd", ret);
//			bTemp = ReadI2C_8A(I2C8REG4F4);
//			Printf(" R4F4:%bx", bTemp);
//		}
//
////		SFRB_ET0 = 0;
////		UsedTime = SystemClock - UsedTime;
////		SFRB_ET0 = 1;
////		Printf("\n\rUsedTime:%ld.%ldsec", UsedTime/100, UsedTime%100 );
//
//
//		WriteI2C_8A(I2C8REG4F3, (DMA_DEST_CHIPREG << 6) + 1);	//Reg,Increase,len:1+3
//		WriteI2C_8A(I2C8REG4FA, SPICMD_RDSR);
//		WriteI2C_8A(I2C8REG4F9,	1); //length low
//		WriteI2C_8A(I2C8REG4F4, 0x01 | SPI_CMD_OPT_NONE);
//		ret = I2cSpiFlashDmaWait_loop(SPI_CMD_OPT_NONE, 200);
//		if(ret==200) {
//			Puts("  RDSR =>Fail");
//		}
//		else {
//			Printf("\nRDSR success at %bd", ret);
//			bTemp = ReadI2C_8A(I2C8REG4F4);
//			Printf(" R4F4:%bx", bTemp);
//		}
//		bTemp = ReadI2C_8A(0xD0);
//		Printf("\nRDSR:%bx",bTemp);
//
//
//		spiaddr += 0x10000;
//		//Printf("\nSE %06lx", spiaddr); 
//
//		//Do write_enable
//		WriteI2C_8A(0xFF,0x04);
//		WriteI2C_8A(I2C8REG4F3, (DMA_DEST_CHIPREG << 6) +1);	//len:1
//		WriteI2C_8A(I2C8REG4F5, 0);	//length high
//		WriteI2C_8A(I2C8REG4F8,	0); //length middle
//		WriteI2C_8A(I2C8REG4F9,	0); //length low
//		WriteI2C_8A(I2C8REG4FA, SPICMD_WREN);
//		WriteI2C_8A(I2C8REG4F4, 0x01 | SPI_CMD_OPT_NONE);	
//
//		//check how long it take.
//		ret = I2cSpiFlashDmaWait_loop(SPI_CMD_OPT_NONE, 200);
//		if(ret==200) {
//			Puts("\nWREN fail before SE");
//			return;
//		}
//		else {
//			Printf("\nSE WREN success at %bd", ret);
//			bTemp = ReadI2C_8A(I2C8REG4F4);
//			Printf(" R4F4:%bx", bTemp);
//		}
//
//		//SE
//		Printf("\nSE %06lx %x",spiaddr);
//		WriteI2C_8A(0xFF,0x04);
//		WriteI2C_8A(I2C8REG4F3, (DMA_DEST_CHIPREG << 6) +4);	//Reg,Increase,len:1+3
//		WriteI2C_8A(I2C8REG4F6, 0x04);	//Reg start 0
//		WriteI2C_8A(I2C8REG4F7, 0xD0);
//		WriteI2C_8A(I2C8REG4F5, 0);	//length high
//		WriteI2C_8A(I2C8REG4F8,	0); //length middle
//		WriteI2C_8A(I2C8REG4F9,	0); //length low
//		WriteI2C_8A(I2C8REG4FA, SPICMD_SE);
//		WriteI2C_8A(I2C8REG4FB, (BYTE)(spiaddr>>16));
//		WriteI2C_8A(I2C8REG4FC, (BYTE)(spiaddr>>8));
//		WriteI2C_8A(I2C8REG4FD, (BYTE)spiaddr);
//		WriteI2C_8A(I2C8REG4F4, 0x01 | SPI_CMD_OPT_WRITE_BUSY);
//		ret=I2cSpiFlashDmaWait(200);
//		if(ret) Puts("  SE =>Fail");
//		else    Puts("  BE_SE=>Done");
//	}
//	else if(!stricmp(argv[1],"cese")) {
//		/*
//		harman complains WriteEnable can not clear REG4F4[0] between BE and SE.
//		*/
//		BYTE ret;
//		DWORD spiaddr;
//		DWORD UsedTime;
//
//		if(argc <=2 ) {
//			Puts("\n\ri2cspi ce/se command needs start_address");
//			return;
//		}
//		
//
//		//check xmem access
//		WriteI2C_8A(0xFF,0x04);
//		bTemp = ReadI2C_8A(0xC2);
//		if(bTemp & 0x02)
//			Puts("\nPlease turn off Xmem access.");
//
//		spiaddr = a2h(argv[2]);
//		Printf("\nCE: SE:%06lx", spiaddr); 
//
//		//Do write_enable
//		WriteI2C_8A(0xFF,0x04);
//		WriteI2C_8A(I2C8REG4F3, (DMA_DEST_CHIPREG << 6) +1);	//len:1
//		WriteI2C_8A(I2C8REG4F5, 0);	//length high
//		WriteI2C_8A(I2C8REG4F8,	0); //length middle
//		WriteI2C_8A(I2C8REG4F9,	0); //length low
//		WriteI2C_8A(I2C8REG4FA, SPICMD_WREN);
//		WriteI2C_8A(I2C8REG4F4, 0x01 | SPI_CMD_OPT_NONE);	
//		ret=I2cSpiFlashDmaWait(2); //20ms
//		if(ret) {
//			Puts("\nWREN fail before BE");
//			return;
//		}
//		//BE
//		//Printf("\nBE %06lx %x",spiaddr);
//		WriteI2C_8A(0xFF,0x04);
//
//
//
//		WriteI2C_8A(I2C8REG4F3, (DMA_DEST_CHIPREG << 6) + 1);	//Reg,Increase,len:1+3
//		WriteI2C_8A(I2C8REG4F6, 0x04);	//Reg start 0
//		WriteI2C_8A(I2C8REG4F7, 0xD0);
//		WriteI2C_8A(I2C8REG4F5, 0);	//length high
//		WriteI2C_8A(I2C8REG4F8,	0); //length middle
//		WriteI2C_8A(I2C8REG4F9,	0); //length low
//		WriteI2C_8A(I2C8REG4FA, SPICMD_CE);
//		//WriteI2C_8A(I2C8REG4FB, (BYTE)(spiaddr>>16));
//		//WriteI2C_8A(I2C8REG4FC, (BYTE)(spiaddr>>8));
//		//WriteI2C_8A(I2C8REG4FD, (BYTE)spiaddr);
//
//		SFRB_ET0 = 0;
//		UsedTime = SystemClock;
//		SFRB_ET0 = 1;
//
//
//		//check how long it take.
//		WriteI2C_8A(I2C8REG4F4, 0x01 | SPI_CMD_OPT_WRITE_BUSY_AUTO);
//
//		ret = I2cSpiFlashDmaWait_loop(SPI_CMD_OPT_WRITE_BUSY_AUTO, 200);
//		if(ret==200) {
//			Puts("\nBE fail");
//			return;
//		}
//		else {
//			Printf("\nBE success at %bd", ret);
//			bTemp = ReadI2C_8A(I2C8REG4F4);
//			Printf(" R4F4:%bx", bTemp);
//		}
//		SFRB_ET0 = 0;
//		UsedTime = SystemClock - UsedTime;
//		SFRB_ET0 = 1;
//		Printf("\n\rUsedTime:%ld.%ldsec", UsedTime/100, UsedTime%100 );
//
//
//
//		//spiaddr += 0x10000;
//		//Printf("\nSE %06lx", spiaddr); 
//
//		//Do write_enable
//		WriteI2C_8A(0xFF,0x04);
//		WriteI2C_8A(I2C8REG4F3, (DMA_DEST_CHIPREG << 6) +1);	//len:1
//		WriteI2C_8A(I2C8REG4F5, 0);	//length high
//		WriteI2C_8A(I2C8REG4F8,	0); //length middle
//		WriteI2C_8A(I2C8REG4F9,	0); //length low
//		WriteI2C_8A(I2C8REG4FA, SPICMD_WREN);
//		WriteI2C_8A(I2C8REG4F4, 0x01 | SPI_CMD_OPT_NONE);	
//
//		//check how long it take.
//		ret = I2cSpiFlashDmaWait_loop(SPI_CMD_OPT_NONE, 200);
//		if(ret==200) {
//			Puts("\nWREN fail before SE");
//			return;
//		}
//		else {
//			Printf("\nSE WREN success at %bd", ret);
//			bTemp = ReadI2C_8A(I2C8REG4F4);
//			Printf(" R4F4:%bx", bTemp);
//		}
//
//		//SE
//		Printf("\nSE %06lx %x",spiaddr);
//		WriteI2C_8A(0xFF,0x04);
//		WriteI2C_8A(I2C8REG4F3, (DMA_DEST_CHIPREG << 6) +4);	//Reg,Increase,len:1+3
//		WriteI2C_8A(I2C8REG4F6, 0x04);	//Reg start 0
//		WriteI2C_8A(I2C8REG4F7, 0xD0);
//		WriteI2C_8A(I2C8REG4F5, 0);	//length high
//		WriteI2C_8A(I2C8REG4F8,	0); //length middle
//		WriteI2C_8A(I2C8REG4F9,	0); //length low
//		WriteI2C_8A(I2C8REG4FA, SPICMD_SE);
//		WriteI2C_8A(I2C8REG4FB, (BYTE)(spiaddr>>16));
//		WriteI2C_8A(I2C8REG4FC, (BYTE)(spiaddr>>8));
//		WriteI2C_8A(I2C8REG4FD, (BYTE)spiaddr);
//		WriteI2C_8A(I2C8REG4F4, 0x01 | SPI_CMD_OPT_WRITE_BUSY);
//		ret=I2cSpiFlashDmaWait(200);
//		if(ret) Puts("  SE =>Fail");
//		else    Puts("  BE_SE=>Done");
//	}
//#endif
	//else if(!stricmp(argv[1],"upload")) {
	//	DWORD src_addr,dest_addr,upload_len;
	//	BYTE ret;
    // 
	//	if(argc <= 4) {
	//		Puts("\nUsage:i2cspi upload src dest len");
	//		return;
	//	}
	//	src_addr = a2h(argv[2]);
	//	dest_addr = a2h(argv[3]);
	//	upload_len = a2h(argv[4]);
    //  
	//	ret = I2CSPI_upload(src_addr, dest_addr, upload_len);
	//	if(ret) Puts("==>FAIL");
	//	else	Puts("==>SUCCESS");
	//}
	//else if(!stricmp(argv[1],"upload2")) {
	//	DWORD src_addr,dest_addr,upload_len;
	//	BYTE ret;
    //  
	//	if(argc <= 4) {
	//		Puts("\nUsage:i2cspi upload src dest len");
	//		return;
	//	}
	//	src_addr = a2h(argv[2]);
	//	dest_addr = a2h(argv[3]);
	//	upload_len = a2h(argv[4]);
    //
	//	ret = I2CSPI_upload_faster(src_addr, dest_addr, upload_len);
	//	if(ret) Puts("==>FAIL");
	//	else	Puts("==>SUCCESS");
	//}
}


static void monitor_i2cspic(void)
{
	volatile BYTE cmd;
	BYTE dat0;
	BYTE i;
	BYTE cnt;
	DWORD spiaddr;
	BYTE ret;
	BYTE index;
	BYTE read_byte;

	if(argc < 2) {
		Puts("\n\rsee spic");
		return;
	}

	index = 0;
	read_byte = 0;
	if(argv[1][0]=='r') {
		read_byte = argv[1][1] - 0x30;
		if(read_byte > 8) {
			//invalid read option
			//HelpMonitorSPIC();
			Puts("\n current max is 8.  TODO increase to 256");
			return;
		}
		index = 1;
	}


	cmd = a2h( argv[index+1] );

	if(cmd == SPICMD_RDID) {	/* spic r3 9F */
		if(read_byte != 3) {
			Puts("\nuse i2cspic r3 9f");
			return;
		}
		Printf("\n\rRDID(JEDEC) ");
		SpiFlash_DmaCmd(SPICMD_RDID, DMA_TARGET_CHIP, 0x4D0, 3, DMA_OPT_NONE);
		ret=SpiFlash_wait_done(10,10);
		if(!ret) {
			WriteI2C_8A(0xFF,0x04);
			//--fast--------------------------
			//ReadI2CS_8A(0xD0, 3, SPI_CmdBuffer);
			//--------------------------------
			SPI_CmdBuffer[0] = ReadI2C_8A(0xD0);
			SPI_CmdBuffer[1] = ReadI2C_8A(0xD1);
			SPI_CmdBuffer[2] = ReadI2C_8A(0xD2);

			Printf(" %02bx %02bx %02bx ", SPI_CmdBuffer[0], SPI_CmdBuffer[1],SPI_CmdBuffer[2]);
			if     (SPI_CmdBuffer[0]==SPIFLASH_MID_MX) {
				Puts("Macronix"); 	
				//print_spiflash_status_register = &print_spiflash_status_register_macronix;
			}
			else if(SPI_CmdBuffer[0]==SPIFLASH_MID_EON) {
				Puts("EOn");
				//print_spiflash_status_register = &print_spiflash_status_register_eon_256;
			}
			else if(SPI_CmdBuffer[0]==SPIFLASH_MID_WB) {
				Puts("Winbond");
				//print_spiflash_status_register = &print_spiflash_status_register_winbond;
			}
			else if(SPI_CmdBuffer[0]==SPIFLASH_MID_MICRON) {
				Puts("Micron");
				//print_spiflash_status_register = &print_spiflash_status_register_micron;
			}
			else if(SPI_CmdBuffer[0]==SPIFLASH_MID_SPANSION)	Puts("Spansion");
			else if(SPI_CmdBuffer[0]==SPIFLASH_MID_GIGA)		Puts("Giga");
			else 												Puts("Unknown");
		}
	}
	else if(cmd == SPICMD_WREN 		/*spic 6 */
	     || cmd == SPICMD_WRDI) {	/*spic 4 */
		if(cmd == SPICMD_WRDI) Puts("\n\rWRDI ");
		else				   Puts("\n\rWREN ");
		SpiFlash_DmaCmd(cmd, DMA_TARGET_CHIP, 0, 0, DMA_OPT_NONE);
		ret=SpiFlash_wait_done(10,10);
	}
	else if(cmd == SPICMD_RDSR) {
		if(read_byte ==0) {
			Puts("\nuse spic r1 5");
			return;
		}
		ret = I2cSpiFlashChipRegCmd(cmd,0, read_byte, SPI_CMD_OPT_NONE, 200);
		WriteI2C_8A(0xFF,0x04);
		for(i=0; i < read_byte; i++)
			Printf(" %02bx", ReadI2C_8A(0xD0+i)); 
	}
	else if(cmd == SPICMD_WRSR) {
		if( argc< (index+4) ) {
			Printf("\n\rMissing Parameters !!!" );
			return;
		}
		SPI_CmdBuffer[0] = a2h(argv[index+2]);
		SPI_CmdBuffer[1] = a2h(argv[index+3]);
		ret = I2cSpiFlashChipRegCmd(cmd,2,	0, SPI_CMD_OPT_BUSY, 200);
	}
	else if(cmd == SPICMD_PP) {
		if( argc< (index+4) ) {
			Printf("\n\rMissing Parameters !!!" );
			return;
		}
		if( argc > (index+11) ) {
			Printf("\n\ronly support 8 bytes !!!" );
			argc = 11;
		}
		spiaddr = a2h( argv[index+2] );
		Printf("\n\rPP ");
		Printf(" %06lx", spiaddr);  //PrintSpiAddr(spiaddr);

		I2cSpiFlashSetAddress2CmdBuffer(spiaddr); //SPI_CmdBuffer[0]~[2] or [0]~[3]


		//BKFYI140819. PP with QuadIO has a problem on REG4D0.
		//use XMEM
		for(i=3,cnt=0; i <argc; i++,cnt++) {
			dat0 = a2h(argv[index+i]);
			SPI_Buffer[cnt]=dat0;
		}
		//if(is_micron_512()) {
		//	SpiFlashSetupBusyCheck(SPICMD_RDFREG,0x06); //pol:low,bit:7.  try bit6
		//}
		//note:I am using vblank check
		ret = I2cSpiFlashChipRegCmd(cmd, 3, cnt, /*1,*/ SPI_CMD_OPT_WRITE, 200);
		//if(is_micron_512()) {
		//	SpiFlashSetupBusyCheck(SPICMD_RDSR,0x08);
		//	SpiFlashChipRegCmd(SPICMD_RDFREG,0,1, SPI_CMD_OPT_NONE, 200);
		//}
	}
	else if(cmd == SPICMD_SE || cmd == SPICMD_BE) {
		if( argc< (index+3) ) {
			Printf("\n\rMissing Parameters !!!" );
			return;
		}
		spiaddr = a2h( argv[index+2] );
		if(cmd==cmd == SPICMD_BE) Puts("\n\rBE ");
		else					  Puts("\n\rSE ");
		Printf(" %06lx", spiaddr);  //PrintSpiAddr(spiaddr);

		I2cSpiFlashSetAddress2CmdBuffer(spiaddr); //SPI_CmdBuffer[0]~[2] or [0]~[3]

		ret = I2cSpiFlashChipRegCmd(cmd,3,0, SPI_CMD_OPT_WRITE_BUSY, 200);
		if(ret) 
			Puts("=>fail");
	}
	else if(cmd == SPICMD_READ_SLOW 
	     || cmd == SPICMD_READ_FAST
		 || cmd == SPICMD_READ_DUAL_O
		 || cmd == SPICMD_READ_QUAD_O 
		 || cmd == SPICMD_READ_DUAL_IO 
		 || cmd == SPICMD_READ_QUAD_IO ) {
		BYTE w_len;
		BYTE SPI_mode;
		BYTE SPI_mode_Reg;

		if( argc < 4 ) {
			Printf("\n\rMissing Parameters !!!" );
			return;
		}
		if(read_byte==0) {
			Printf("\n\rMissing read length !!!" ); //HelpMonitorSPIC();
			return;
		}
		//change SPI_mode
		WriteI2C_8A(0xFF,0x04);
		SPI_mode_Reg = ReadI2C_8A(I2C8REG4F0);
		if(cmd==SPICMD_READ_SLOW) {
			SPI_mode=0;
			w_len = 3;
			if((ReadI2C_8A(0xE1) & 0xC0)==0xC0) {
				Printf("\n\rEDGE CYCLE error. GiveUp!!");
				return;
			}		
		}
		else if(cmd==SPICMD_READ_FAST) {
			SPI_mode=1;
			w_len = 4;		
		}
		else if(cmd==SPICMD_READ_DUAL_O) {
			SPI_mode=2;
			w_len = 4;		
		}
		else if(cmd==SPICMD_READ_QUAD_O) {
			SPI_mode=3;
			w_len = 4;		
		}
		else if(cmd==SPICMD_READ_DUAL_IO) {
			SPI_mode=4;
			w_len = 4;		
		}
		else {
			//Printf("\n\rDualIO & QuadIO can read only 1 byte...");
			SPI_mode = 5;
			w_len = 6;		
		}

		spiaddr = a2h( argv[index+2] );
		Printf(" %06lx", spiaddr);  //PrintSpiAddr(spiaddr);

		//if(argc >= 4)
		//	cnt = a2h(argv[index+3]);
		//else cnt = 8;

		I2cSpiFlashSetAddress2CmdBuffer(spiaddr); //SPI_CmdBuffer[0]~[2] or [0]~[3]

		WriteI2C_8A(I2C8REG4F0, (SPI_mode_Reg & ~0x07) | SPI_mode);
		ret = I2cSpiFlashChipRegCmd(cmd, w_len, read_byte, SPI_CMD_OPT_BUSY, 200);
		if(!ret) {
			if(cmd == SPICMD_READ_DUAL_O
			|| cmd == SPICMD_READ_QUAD_O
			|| cmd == SPICMD_READ_DUAL_IO
			|| cmd == SPICMD_READ_QUAD_IO) {
				for(i=0; i < read_byte; i++)
					Printf(" %02bx", SPI_Buffer[i]);
			}
			else {
				for(i=0; i < read_byte; i++)
					Printf(" %02bx", SPI_CmdBuffer[i]);
			}
		}
		//restore SPI_Mode
		//WriteTW88(REG4C0, SPI_mode_Reg);
	}
	else {
		BYTE w_len;
		if(read_byte) w_len = 0; //read mode
		else {	
			w_len = argc - 2;	
		}
		for(i=0; i < w_len; i++)
			SPI_CmdBuffer[i] = a2h(argv[index+2]);
		
		ret = I2cSpiFlashChipRegCmd(cmd, w_len, read_byte, SPI_CMD_OPT_NONE, 200);
		if(!ret) {
			for(i=0; i < read_byte; i++)
				Printf(" %02bx",SPI_CmdBuffer[i]);
		}
	}
}



#if	defined(SUPPORT_BT656)
static void monitor_ibt656(void)
{
	if(argc < 2) {
		//print current value
		Printf("\n\rInputBT656:");
#if	defined(DEBUG_BT656)
		PrintfBT656Input(InputBT656,1);
		Printf("\t\tInputBT656EE:");
		PrintfBT656Input(GetInputBT656EE(),1);
#endif
		Printf("\n\rusage: iBT656 {DEC|ARGB|DTV|LVDS|PANEL|OFF|AUTO}");
		//Printf("\n\rusage: iBT656 {OFF|AUTO|PANEL|...}");
	}
	else {
		//if you directly select BT656Enc input,
		//InputMain have to be a correct setting.


		     if( !stricmp( argv[1], "OFF" ))  	InputBT656 = BT656ENC_SRC_OFF;
		else if( !stricmp( argv[1], "DEC" ))   	InputBT656 = BT656ENC_SRC_DEC;
		else if( !stricmp( argv[1], "ARGB" )) 	InputBT656 = BT656ENC_SRC_ARGB;
		else if( !stricmp( argv[1], "DTV" ))   	InputBT656 = BT656ENC_SRC_DTV;
		else if( !stricmp( argv[1], "LVDS" ))   InputBT656 = BT656ENC_SRC_LVDS;
		else if( !stricmp( argv[1], "PANEL" ))  InputBT656 = BT656ENC_SRC_PANEL;
		else if( !stricmp( argv[1], "LOOP" )) {
			//loopback mode.
			if(InputMain == INPUT_CVBS
			|| InputMain == INPUT_SVIDEO
			|| InputMain == INPUT_COMP
			|| InputMain == INPUT_PC
			|| InputMain == INPUT_LVDS) {
				switch(InputMain) {
				case INPUT_COMP:
				case INPUT_PC:	
					InputBT656 = BT656ENC_SRC_ARGB;	
					break;	
				case INPUT_LVDS:	
					InputBT656 = BT656ENC_SRC_LVDS;		
					break;
				default:		
					InputBT656 = BT656ENC_SRC_DEC;		
					break;
				}
				//passthru. or
				//ChangeBT656Module(InputBT656);
				//CheckAndSetBT656Loop();
				//ChangeBT656Input(InputBT656);
				//CheckAndSetBT656DecOnLoop(InputBT656);
				//return;
			}
			else {
				Printf("\n\riBT656 loop supports CVBS,SVideo,Component,PC,LVDS only");
				Printf("\n\ruse imain command first."); 
			}
		}
		else {
			Printf("\n\rInvalid command...");
			Prompt();
			return;                                    
		}
		if(GetInputBT656EE() != InputBT656) 	
			SaveInputBT656EE( InputBT656 );

		//-------------------------------
		//NOTE: InputMain has a high priority when InputBT656 has a conflict.
		ChangeBT656__MAIN(InputBT656);
	}
}
#endif
static void monitor_imain(void)
{
	if(argc < 2) {
		//print current value
		Printf("\n\rInputMain:");
		PrintfInput(InputMain,1);
		Printf("\t\tInputMainEE:");
		PrintfInput(GetInputMainEE(),1);

		Printf("\n\rusage: imain {CVBS|SVIDEO|COMP|PC|HDMI|BT656|LVDS}");
	}
	else {
		BYTE new_InputMain;
		if     ( !stricmp( argv[1], "CVBS" ))   InputMain = INPUT_CVBS;
		else if( !stricmp( argv[1], "SVIDEO" )) InputMain = INPUT_SVIDEO;
		else if( !stricmp( argv[1], "COMP" ))   InputMain = INPUT_COMP;
		else if( !stricmp( argv[1], "PC" ))     InputMain = INPUT_PC;
		else if( !stricmp( argv[1], "DVI" ))    InputMain = INPUT_DVI;
		else if( !stricmp( argv[1], "HDMI" )) {
			if(GetHdmiModeEE())  				InputMain = INPUT_HDMITV;
			else 				 				InputMain = INPUT_HDMIPC;
		}
		else if( !stricmp( argv[1], "BT656" ))  InputMain = INPUT_BT656;
		else if( !stricmp( argv[1], "LVDS" ))   InputMain = INPUT_LVDS;
		else                                    InputMain = INPUT_CVBS;
		InputSubMode = 7; //N/A

		LinkCheckAndSetInput();
	
		if(GetInputMainEE() != InputMain) 	
			SaveInputMainEE( InputMain );

		new_InputMain = InputMain;
		InputMain = 0xff;
		ChangeInput( new_InputMain );	
	}
}

static void HelpMonitorInit(void)
{
	Puts("\n\rInit {?|core|fpga|NTSC|panel|sspll|clock {H|L}|logo {on|off}|default}");

	Puts("\n\r\nExample:BootUp");
	Puts("\n\r init core");
	Puts("\n\r init | init NTSC");
	Puts("\n\r      | init panel");
	Puts("\n\r m");
	Puts("\n\r init logo");
	Puts("\n\r\nExample:change input");
	Puts("\n\r m [CVBS|SVIDEO|COMP|PC|DVI|HDMI|BT656]");
	Puts("\n\r or");
	Puts("\n\r input [CVBS|SVIDEO|COMP|PC|DVI|HDMI|BT656]");
	Puts("\n\r init default");
	Puts("\n\r checkandset");
}

static void monitor_init(void)
{
	if(argc < 2) {
		//InitWithNTSC();
		Init8836AsDefault(InputMain, 1);

		InitGpioDefault();
		Sspll1PowerUp(ON);
		//DCDC needs 100ms, but we have enough delay on...
		FrontPanel_StartUP();

		ScalerSetMuteManual(OFF);
		PowerUpLedBackLight();
	}
	else {
		if( !stricmp( argv[1], "?") || !stricmp( argv[1], "help") ) {
			HelpMonitorInit();
		}
		else if( !stricmp( argv[1], "fpga" ) ) {	
			InitCore(1);
			/*ee_mode = */ CheckEEPROM();
			Init8836AsDefault(InputMain, 1);
			//InitWithNTSC();
			FrontPanel_StartUP();
			ScalerSetMuteManual(OFF);
			PowerUpLedBackLight();
			PllClkSetDividerReg(PLLCLK_DIV_1P5);
			SpiOsdEnable(ON);
			SpiOsdSetDeValue();
			DebugLevel = 0;
		}	
		else if( !stricmp( argv[1], "core" ) ) {	
			InitCore(1);
		}
		else if( !stricmp( argv[1], "NTSC" ) ) {
			Init8836AsDefault(0/*InputMain*/, 1);
			InitGpioDefault();

			Puts("\nEnable OutputPin");
			OutputEnablePin(OFF,ON);		//Output enable. FP data: not yet
			Sspll1PowerUp(ON);
			SOsd_init();
			//Init8836AsDefault(0,0);
			//ChangeDecoder(0);
			InputMain = INPUT_CVBS;
			InitInputAsDefault();
			CheckAndSetDecoderScaler();
			ScalerSetFreerunAutoManual(ON,OFF);
			ScalerSetMuteAutoManual(ON,OFF);
		}
		else if( !stricmp( argv[1], "system" ) ) {
			BYTE old_access;

			old_access = g_access;
			g_access = 1;
			InitSystem(1);
			g_access = old_access;
		}
		else if( !stricmp( argv[1], "panel" ) ) {
			FrontPanel_StartUP();
			ScalerSetMuteManual(OFF);
			PowerUpLedBackLight();
		}
		else if( !stricmp( argv[1], "ee" ) ) {
			Printf("\n\rFind EEPROM variables...");
			//E3P_Init();
			E3P_Configure();
		}
		else if( !stricmp( argv[1], "sspll" ) ) {
			SpiClkRecover27MSource();  //with debug message
//BKTODO..130927
#if defined(PANEL_AUO_B133EW01)
		 	//SspllSetFreqAndPll(150000000L);
		 	Sspll2SetFreq(110000000L, ON);
			//BUGGY_PclkSetFunc(110000000L);
#elif defined(PANEL_1024X600)
		 	Sspll2SetFreq(81000000L, ON);
			//BUGGY_PclkSetFunc(81000000L);
#else
		 	Sspll2SetFreq(108000000L, ON);
			//BUGGY_PclkSetFunc(108000000L);
#endif
//			if(SpiFlashVendor==SFLASH_VENDOR_MX)
//				PllClkSetSelDiv(PLLCLK_PLL108,PLLCLK_DIV_2P0);
//			else
//				PllClkSetSelDiv(PLLCLK_PLL108,PLLCLK_DIV_1P5);
		}
//		else if( !stricmp( argv[1], "clock" ) ) {
//			if( argc>=3 ) {
//				if     ( toupper(argv[2][0])=='H' ) ClockHigh();
//				else if( toupper(argv[2][0])=='L' ) ClockLow();
//				else                                Clock27();
//			}
//			else
//				Clock27();
//		}
		else if( !stricmp( argv[1], "logo" ) ) {
			if( argc>=3 ) {
				if( !stricmp( argv[2], "on" ) )
					InitLogo1();
				else if( !stricmp( argv[2], "off" ) )
					RemoveLogoWithWait(1);
				else
					Printf("\n\rInvalid command...");
			}
			else {
				//auto
				InitLogo1();
				if(1) {
					RemoveLogoWithWait(1);
				}
			}
		}
		else if( !stricmp( argv[1], "default" ) ) {
			InitInputAsDefault();
		} 
	}
}

static void monitor_isr(void)
{
	if(argc < 2) {
		Printf("\n\rISR status:");
		if(SFRB_EX0) Printf("EX0 ");						// ext0
		if(SFRB_ET1) Printf("ET1 ");						// timer1 - touch
		if(SFRB_ET2) Printf("ET2 ");						// timer2 - Remo
		if(SFR_E2IE) Printf("E2IE:%02bx ",SFR_E2IE);	// E2IE[4] - Remo				
	}
	else if(!stricmp( argv[1], "ON" )) {
		if(SFRB_EX0) Printf("-->Skip");
		else {
			SFRB_EX0 = 1;
#ifdef SUPPORT_TOUCH
			SFRB_ET1 = 1;
#endif
			SFR_E2IE |= 0x04;
		}
	}	
	else if(!stricmp( argv[1], "OFF" ))	{
		if(SFRB_EX0 == 0) Printf("-->Skip");
		else {
			SFRB_EX0 = 0;
			SFRB_ET1 = 0;
			SFR_E2IE = 0;
		}
	}
}
//---------------- pclk -------------------------
//	pclk 1080 means 108MHz
//	pclk 27 means 27MHz
#if 0
static void monitor_pclk(void)
{
		if(argc >= 2) {
			dValue = a2i( argv[1] );
			dValue *= 100000L;
		 	Sspll1SetFreq(dValue, ON);
		}
		//print current pclk info
		Printf("\n\rsspll2:%ld",Sspll2GetFreq());
}
#endif


#if defined(REMO_RC5) && defined(DEBUG_REMO)
static void monitor_remo(void)
{
	if(!stricmp( argv[1], "ddremo")) {
		PrintRemoCapture();
	}
}
#endif

//----------------------------------------------------	
// read pixel
//----------------------------------------------------	
static void monitor_rpixel(void)
{
		WORD x0,x1, y0,y1;
		if(argc < 5) {
			Printf("\n\rusage rpixel x0 y0 x1 y1");
			Prompt();
			return;
		}
		x0 = a2i(argv[1]);
		y0 = a2i(argv[2]);
		x1 = a2i(argv[3]);
		y1 = a2i(argv[4]);
		ReadOutputPixel(x0,y0,x1,y1);
}

//----------------------------------------------------
// scaler test
//
// 0: old
// 1: new
// 2: target:720x480P
//----------------------------------------------------
static void monitor_scaler(void)
{
	//	BYTE mode;
	//	if(argc < 2) 
	//		mode = 0;
	//	else {
	//		mode = a2h(argv[1]);
	//	}
	//	ScalerTest(mode);
}


static void monitor_sosd(void)
{
	if(argc < 2) {
		Puts("lutgrid\n");
#if 0
		Puts("SLUT0\n");
		Puts("SLUT1\n");
		Puts("SLUT2\n");
#endif		
	}
	else if ( !stricmp( argv[1], "lutgrid" )) {
		DWORD addr;
		WORD offset;

		if ( argc == 3 ) 	offset = a2i(argv[2]);
		else 				offset = 0;

		addr = offset * 0x400;
		addr += 0x100000;

		//SpiOsdLoadLUT(1, 0, 0, 0x400, addr, 0xFF); 
		SOsd_SetLut(1, 0, 0, 0x400, addr, 0xFF);
		SOsd_UpdateLut(1, 0);	
	}
#if 0
	else if ( !stricmp( argv[1], "SLUT0" )) {	 // white
		WaitVBlank(1);
		//SpiOsdLoadLUT(3, 1, 0, 1024, 0xb0000, 0xFF);	 //winno ?3
		SOsd_SetLut(3, 1, 0, 1024, 0xb0000, 0xFF);
		SOsd_UpdateLut(3, 0);	
	}
	else if ( !stricmp( argv[1], "SLUT1" )) {	 // white
		WaitVBlank(1);
		//SpiOsdLoadLUT( 3, 1, 256, 1024, 0xb0400, 0xFF); //winno ?3
		SOsd_SetLut(3, 1, 256, 1024, 0xb0400, 0xFF);
		SOsd_UpdateLut(3, 0);	
	}
	else if ( !stricmp( argv[0], "SLUT2" )) {	 // white
		BYTE i;

		i = 0;
		while ( !RS_ready() ) {
			#if 0
			WaitVBlank(1);
			//SpiOsdLoadLUT( 3, 1, 128, 512, 0xb0800, 0xFF); //winno ?3
			SOsd_SetLut(3, 1, 128, 512, 0xb0800, 0xFF);
			SOsd_UpdateLut(3, 0);	
			delay1ms(100);
			
			WaitVBlank(1);
			//SpiOsdLoadLUT(3,  1, 128, 512, 0xb0a00, 0xFF); //winno ?3
			SOsd_SetLut(3,  1, 128, 512, 0xb0a00, 0xFF);
			SOsd_UpdateLut(3, 0);	
			delay1ms(100);
			#endif
			WaitVBlank(1);
			//SpiOsdLoadLUT(3,  1, 128+i++, 512, 0xb0c00, 0xFF);	//winno ? 3
			SOsd_SetLut(3,  1, 128+i++, 512, 0xb0c00, 0xFF);
			SOsd_UpdateLut(3, 0);	
			//WaitVBlank(1);
			//SpiOsdLoadLUT( 1, 128+i++, 512, 0xb0e00);
			i %= 128;
		}
	}
#endif	
		
}



static void monitor_task(void)
{
	if(argc < 2) {
		Puts("\ntask [on|off]");
		Printf(" curr:%bx",	TaskNoSignal_getCmd());
		Prompt();
		return;
	}

	if( !stricmp( argv[1], "on") ) {
		tic_task = 10*100;	// NOSIGNAL_TIME_INTERVAL
		TaskNoSignal_setCmd(TASK_CMD_RUN);
	}
	else {
		TaskSetGrid(0);

		TaskNoSignal_setCmd(TASK_CMD_DONE);
		//WriteTW88(REG003, 0xFF );	// disable all INTR
	}	
}



static void monitor_time(void)
{
	DWORD time;
	BYTE hour,min,sec,ms;

	SFRB_ET0=0;
	time = SystemClock;
	SFRB_ET0=1;
	Printf("\n\rSystem Clock %ld tic:%bd", time, tic01);
	ms = (BYTE)(time%100);
	time = time/100;
	if(time==0) {
		sec = 0;
		min = 0;
		hour = 0;
	}
	else {
		sec = (BYTE)(time % 60);
		time = time / 60;
		if(time==0) {
			min = 0;
			hour = 0;
		}
		else {
			min = (BYTE)(time%60);
			hour = time / 60;
		}
	}
	Printf("\t%bd:%02bd:%02bd,%02bd", hour,min,sec ,ms);
}

#ifdef SUPPORT_TOUCH
static void monitor_touch(void)
{
	//---------------- Touch Calibration -------------------------
	if( !stricmp( argv[1], "CALIB" ) ) {
		BYTE	n;
		if ( argc == 2 ) {
			n = a2h(argv[2]);
			if ( n > 4 ) n = 4;
			CalibTouch(n);
		}
	}
	else if(!stricmp( argv[1], "TCOUNT" ) ) {
		CpuTouchSkipCount = 100;
		if ( argc == 3 ) {
			CpuTouchSkipCount = a2i(argv[2]);
		}
	}
	else if( !stricmp( argv[1], "TDUMP" ) ) {
		Puts("\r\nTouch Dump");
		TouchDump();
	}

	else if( !stricmp( argv[1], "TOUCH" ) ) {
		if ( argc == 2 ) {
			if ( !stricmp( argv[2], "ON" ) ) {
				SFRB_ET1 = 1;	//BK110531
				SetTouchAuto(1);
				Puts("\r\nTurn ON Touch Sense");
			}
#ifdef DEBUG_TOUCH_SW
			else if ( !stricmp( argv[2], "DATA" ) ) {
				PrintCalibData();
			}
#endif
			else {
				SetTouchAuto(0);
				SFRB_ET1 = 0;	//BK110531
				Puts("\r\nTurn OFF Touch Sense");
			}
		}
		else {
			GetTouch2();
		}
	}


	else if( !stricmp( argv[1], "TRACE" ) ) {
		if ( argc == 3 ) {
			if ( !stricmp( argv[2], "ON" ) ) {
				Puts("\r\nTurn ON Touch Trace");
				SFRB_ET1 = 0;	//BK110531
				SetTraceAuto(1);
			}
			else {
				Puts("\r\nTurn OFF Touch Trace");
				SFRB_ET1 = 1;	//BK110531
				SetTraceAuto(0);
			}
		}
		else {
//			TraceTouch();
//			extern 	bit FLAG_TOUCH;
			extern	WORD	TouchX, TouchY, TouchZ1; //, AUX[4];
			//extern	BYTE	/*CpuTouchChanged,*/ AUX_Changed[4];
			BYTE	temp, atemp[4], i;
			temp = CpuTouchChanged;
			atemp[0] = CpuAUX0_Changed;
			atemp[1] = CpuAUX1_Changed;
			atemp[2] = CpuAUX2_Changed;
			atemp[3] = CpuAUX3_Changed;
			while ( !RS_ready() ) {
				delay1ms(500);
				if ( CpuTouchPressed ) {
					if (CpuTouchChanged != temp) {
						Printf("\n\rTouchX: 0x%04x, TouchY: 0x%04x, Z1: 0x%04x", TouchX, TouchY, TouchZ1);
						temp = CpuTouchChanged;
					}
				}
				if ( atemp[0] != CpuAUX0_Changed ) {
					Printf("\n\r0x%02bx - AUX[0]: 0x%04x",atemp[i], CpuAUX0);
					atemp[0] = CpuAUX0_Changed;
				}
				if ( atemp[1] != CpuAUX1_Changed ) {
					Printf("\n\r0x%02bx - AUX[1]: 0x%04x",atemp[i], CpuAUX1);
					atemp[1] = CpuAUX1_Changed;
				}
				if ( atemp[2] != CpuAUX2_Changed ) {
					Printf("\n\r0x%02bx - AUX[2]: 0x%04x",atemp[i], CpuAUX2);
					atemp[2] = CpuAUX2_Changed;
				}
				if ( atemp[3] != CpuAUX3_Changed ) {
					Printf("\n\r0x%02bx - AUX[3]: 0x%04x",atemp[i], CpuAUX3);
					atemp[3] = CpuAUX3_Changed;
				}
			}
		}
	}

	else if( !stricmp( argv[1], "TSC_DEBUG" ) ) {
		tsc_debug = 0;
		if ( argc == 2 ) {
			if ( !stricmp( argv[2], "ON" ) )
				tsc_debug = 1;
		}
		if(tsc_debug) {
			SFRB_ET1 = 0;	//disable touch timer
		}
		else {
			InitAuxADC();
		}
	}
	
}
#endif

#ifdef DEBUG_UART
//---------------- UART TEST  ------
// to check the UART FIFO.
//	"mcu uart"
//	"uartdump" and hold "a" key for 5 sec and then press "x".
//	type "mcu uart" and, check the UART0 Max value.
//----------------------------------
static void monitor_uart(void)
{
	if(!stricmp( argv[1], "DUMP")) {
		BYTE	ch;

		do {
			SFR_ES = 0;			//disable SFR_ES 
			delay1ms(100);
			delay1ms(100);
			SFR_ES = 1;			//enable SFR_ES
			delay1ms(100);

			if( !RS_ready() ) continue;
			ch = RS_rx();
			Printf("%02bx ", ch );
		} while ( ch != 'x' );
	}
}
#endif





//=============================================================================
//
//=============================================================================
/**
* print prompt
*/
void Prompt(void)
{
#ifdef BANKING
	if ( MonAddress == TW88I2CAddress )
		Printf("\n\r[B%02bx]MCU_I2C[%02bx]>", BANKREG, MonAddress);
	else
#else
	if ( MonAddress == TW88I2CAddress )
		Printf("\n\rMCU_I2C[%02bx]>", MonAddress);
	else
#endif
		Printf("\n\rI2C[%02bx]>", MonAddress);
}
#ifdef SUPPORT_UART1
void Prompt1(void)
{
	if ( MonAddress == TW88I2CAddress )
		Printf1("\n\riAP>");
	else
	Printf1("\n\rRS1_I2C[%02bx]>", MonAddress);
}
#endif

#if 0
void WaitUserInput(void)
{
	Printf("\n\rPress any key...");
	while ( !RS_ready() );
	Puts("\n\r");
}
#endif

void Mon_tx(BYTE ch)
{
	RS_tx(ch);
}
#ifdef SUPPORT_UART1
void Mon1_tx(BYTE ch)
{
	RS1_tx(ch);
}
#endif
//=============================================================================
//
//=============================================================================
/*
	format								description			function
	------								-----------			---------
	c i2cid								set I2C device ID.	SetMonAddress
	r idx								read data			MonReadI2CByte
	w idx data							write data			MonWriteI2CByte
	,									decrease 1			MonIncDecI2C
	<									decrease 10
	.									increase 1
	>									increase 10
	d [idx_from] [idx_to]				dump				MonDumpI2C

	( i2cid index						read				MonNewReadI2CByte
	) i2cid idx data					write				MonNewWriteI2CByte
	& i2cid idx_from idx_to	dump		dump				MonNewDumpI2C
	b i2cid index startbit|endbit data	bitwise wirte		MonWriteBit
	wait reg mask result max_wait		wait				MonWait

	;									comment
	/									repeat last command
	`									repeat command without CR
*/

/**
* format: c I2cId
*/
void SetMonAddress(BYTE addr)
{
	MonAddress = addr;
}


/**
* increase/decrease value
*
* @param inc 0:decrease 1:increase 10:decrease 10 value. 11:increase 10 value
*
* format
* decrease 1:  ,
* decrease 10: <
* increase 1:  .
* increase 10: >
*
* extern
*	MonIndex,MonWdata
*/
void MonIncDecI2C(BYTE inc)
{
	DWORD dTemp;

	switch(inc){
		case 0:  MonWdata--;		break;
		case 1:  MonWdata++;		break;
		case 10: MonWdata-=0x10;	break;
		case 11: MonWdata+=0x10;	break;
	}

	if ( MonAddress == TW88I2CAddress )
		WriteTW88(MonIndex, MonWdata);
#ifdef SUPPORT_I2C_MASTER
	else
		WriteI2C_multi(MonAddress,MonIndexLen<<4|MonDataLen,MonIndex,MonWdata);
#endif

	if( echo ) {
		Printf("Write ");
		if(MonIndexLen==2)		Printf("%04xh:",       MonIndex);
		else					Printf("%02bxh:",(BYTE)MonIndex);
		if(MonDataLen==4)		Printf("%08lxh",      MonWdata);
		else if(MonDataLen==2)	Printf("%04xh", (WORD)MonWdata);
		else 					Printf("%02bxh",(BYTE)MonWdata);

		if ( MonAddress == TW88I2CAddress )
			dTemp = ReadTW88(MonIndex);
#ifdef SUPPORT_I2C_MASTER
		else 
			dTemp = ReadI2C_multi(MonAddress, MonIndexLen<<4 | MonDataLen, MonIndex);
#endif
		Printf(" ==> Read ");
		if(MonIndexLen==2)		Printf("%04xh:",       MonIndex);
		else					Printf("%02bxh:",(BYTE)MonIndex);
		if(MonDataLen==4)		Printf("%08lxh",      dTemp);
		else if(MonDataLen==2)	Printf("%04xh", (WORD)dTemp);
		else 					Printf("%02bxh",(BYTE)dTemp);
	}
	Prompt();
}


//============================================================
// new commands for index and data size
//============================================================



void MonReadI2C_multi(BYTE f_id, BYTE idx_len, BYTE data_len)
{
	BYTE Slave;
#ifdef DEBUG_UART
	BYTE i;
#endif
	DWORD dTemp;


	if( argc < (2+f_id) ) {
		Printf("   --> Missing parameter !!!");
#ifdef DEBUG_UART
		for(i=0; i < 20; i++) {
			Printf(" %02bx",monstr[i]);
		}
#endif
		return;
	}

	if(f_id) {
		Slave = a2h(argv[1]);
		MonIndex = a2h( argv[2] );
	}
	else {
		Slave = MonAddress;
		MonIndex = a2h(argv[1]);
	}
	if ( Slave == TW88I2CAddress ) {
		if(MonIndexLen == 1) {
			MonIndexLen = 2;
			MonIndex |= ((WORD)ReadTW88Byte(0xff) << 8);
		}
		dTemp = ReadTW88(MonIndex);
	}	
#ifdef SUPPORT_I2C_MASTER
	else {
		dTemp = ReadI2C_multi(Slave, idx_len << 4 | data_len, MonIndex);
	}
#endif
	if( echo ) {
		Printf("\n\r<R>%02bx", Slave);
		if(idx_len==2)			Printf("[%04x]",       MonIndex);
		else					Printf("[%02bx]",(BYTE)MonIndex);

		if(data_len==4)			Printf("=%08lx",      dTemp);
		else if(data_len==2)	Printf("=%04x", (WORD)dTemp);
		else 					Printf("=%02bx",(BYTE)dTemp);
	}
	MonWdata = dTemp;	//save for increase & decrease
}

void MonWriteI2C_multi(BYTE f_id, BYTE idx_len, BYTE data_len)
{
	BYTE Slave;
	static DWORD dTemp;

	if( argc< (3+f_id) ) {
		Printf("   --> Missing parameter !!!");
#ifdef DEBUG_UART
		DEBUG_dump_uart0();
#endif
		return;
	}
	if(f_id) {
		Slave    = a2h( argv[1] );
		MonIndex = a2h( argv[2] );
		MonWdata = a2h( argv[3] );
	}
	else {
		Slave    = MonAddress;
		MonIndex = a2h( argv[1] );
		MonWdata = a2h( argv[2] );
	}

	if ( Slave == TW88I2CAddress ) {
		if(MonIndexLen == 1) {
			MonIndexLen = 2;
			MonIndex |= ((WORD)ReadTW88Byte(0xff) << 8);
		}
		WriteTW88(MonIndex, MonWdata);
	}
#ifdef SUPPORT_I2C_MASTER
	else {
		WriteI2C_multi(Slave, idx_len << 4 | data_len, MonIndex, MonWdata);
	}
#endif
	if( echo ) {
		if ( Slave == TW88I2CAddress )
			dTemp = ReadTW88(MonIndex);
#ifdef SUPPORT_I2C_MASTER
		else
			dTemp = ReadI2C_multi(Slave, idx_len<<4 |  data_len, MonIndex);
#endif
		Printf("\n\r<R>%02bx", Slave);
		if(idx_len==2)			Printf("[%04x]",       MonIndex);
		else					Printf("[%02bx]",(BYTE)MonIndex);
		if(data_len==4)			Printf("=%08lx",      dTemp);
		else if(data_len==2)	Printf("=%04x", (WORD)dTemp);
		else 					Printf("=%02bx",(BYTE)dTemp);
	}
}
void MonToggleBit_multi(BYTE f_id, BYTE idx_len, BYTE data_len)
{
	BYTE i;
	BYTE Slave;
	DWORD dTemp;
	BYTE FromBit,ToBit;
	DWORD mask, MonMask;

	Slave = f_id; //ignore f_id 

	if( argc<5 ) {
		Printf("   --> Missing parameter !!!");
#ifdef DEBUG_UART
		DEBUG_dump_uart0();
#endif
		return;
	}
	Slave = a2h(argv[1]);
	MonIndex = a2h( argv[2] );

	if(idx_len==1 && data_len==1) {
		FromBit  =(a2h( argv[3] ) >> 4) & 0x0f;
		ToBit    = a2h( argv[3] )  & 0x0f;
	}
	else { 
		FromBit = (a2h(argv[3]) >> 8) & 0xFF;
		ToBit   =  a2h(argv[3]) & 0xFF;		
	}
	MonMask  = a2h( argv[4] );
	mask = 0x04 << data_len;
	if( FromBit<ToBit || FromBit >= mask || ToBit >= mask) {
		Printf("\n\r   --> Wrong range of bit operation !!!");
		return;
	}
	
	if(data_len==0) {
		Printf("\n\r   invalid len");  
		return;
	}
	mask = 0;
	i=data_len*8-1;	//from MS bit
	while(1) {
		mask <<= 1; //make room
		if(i <= FromBit && i >= ToBit)
			mask |= 1;
		if(i==0)
			break;
		i--;
	}

	if ( Slave == TW88I2CAddress ) {
		if(MonIndexLen == 1) {
			MonIndexLen = 2;
			MonIndex |= ((WORD)ReadTW88Byte(0xff) << 8);
		}			
		dTemp = ReadTW88(MonIndex);
		MonWdata = (dTemp & (~mask)) | (MonMask & mask);					
		WriteTW88(MonIndex, MonWdata);
	}
#ifdef SUPPORT_I2C_MASTER
	else {
 		dTemp = ReadI2C_multi(Slave, idx_len<<4 |  data_len, MonIndex);
		MonWdata = (dTemp & (~mask)) | (MonMask & mask);
		WriteI2C_multi(Slave, idx_len << 4 | data_len, MonIndex, MonWdata);
	}
#endif
	if( echo ) {
		if ( Slave == TW88I2CAddress )
			dTemp = ReadTW88(MonIndex);
#ifdef SUPPORT_I2C_MASTER
		else 
			dTemp = ReadI2C_multi(Slave, idx_len<<4 |  data_len, MonIndex);
#endif
		Printf("\n\r<R>%02bx", Slave);
		if(idx_len==2)			Printf("[%04x]",        MonIndex);
		else					Printf("[%02bx]", (BYTE)MonIndex);
		if(data_len==4)			Printf("=%08lx",      dTemp);
		else if(data_len==2)	Printf("=%04x", (WORD)dTemp);
		else 					Printf("=%02bx",(BYTE)dTemp);
	}
}
					

void MonDumpI2C_multi(BYTE id, BYTE idx_len, BYTE data_len)
{
	WORD 	ToMonIndex;
	BYTE    Slave = id; //ignore id
	WORD	i;
	DWORD dTemp;
	WORD temp_MonIndex;	

	if(id) {
		Slave = a2h(argv[1]);
		if( argc>=3 ) 	MonIndex = a2h(argv[2]);
		if( argc>=4 )   ToMonIndex = a2h(argv[3]);
		else			ToMonIndex = MonIndex+7;
		//if argc < 3. use old MonIndex.	
		}
		else {
		Slave = MonAddress;
		if( argc>=2 ) MonIndex = a2h(argv[1]);
		if( argc>=3 ) ToMonIndex = a2h(argv[2]);
		else			ToMonIndex = MonIndex+7;	
		//if argc < 2. use old MonIndex.	
	}
					
	if ( Slave == TW88I2CAddress ) {
		if(MonIndexLen == 1) {
			MonIndexLen = 2;
			MonIndex |= ((WORD)ReadTW88Byte(0xff) << 8);
			ToMonIndex |= (MonIndex & 0xFF00);
		}
	}
	temp_MonIndex = MonIndex;
	for(i=MonIndex; i<=ToMonIndex; i+=data_len) {
		if ( Slave == TW88I2CAddress )
			dTemp = ReadTW88(MonIndex);
#ifdef SUPPORT_I2C_MASTER
		else 
			dTemp = ReadI2C_multi(Slave, idx_len<<4 |  data_len, MonIndex);
#endif

		Printf("\n\r<R>%02bx", Slave);
		if(idx_len==2)			Printf("[%04x]=",       MonIndex);
		else					Printf("[%02bx]=",(BYTE)MonIndex);
		if(data_len==4)			Printf("%08lx",      dTemp);
		else if(data_len==2)	Printf("%04x", (WORD)dTemp);
		else 					Printf("%02bx",(BYTE)dTemp);

		MonIndex += data_len;
	}
	//--------------------------
	//test routine
#if 0
	if(Slave != TW88I2CAddress) {
		BYTE buff[8*4];
		BYTE j;
		WORD *pTemp;
		
		MonIndex = temp_MonIndex;
		ReadI2CS_multi(Slave, idx_len<<4 |  data_len, MonIndex, buff ,8);

		Puts("\nBuff:");
		for(i=0; i < 8; i++) {
			Printf(" %02bx",buff[i]);
		}
		

		for(i=MonIndex; i<=ToMonIndex; i+=data_len) {

			Printf("\n\r<R>%02bx", Slave);
			if(idx_len==2)			Printf("[%04x]=", i);
			else					Printf("[%02bx]=", (BYTE)i);
			pTemp = (WORD *)&buff[i];

			//if(data_len==4)			Printf("%08lx",      *pTemp);
			//else if(data_len==2)	Printf("%02bx%02bx", buff[i] << 8 | buff[i+1]);
			//else 					Printf("%02bx",buff[i]);
			for(j=0;j<data_len;j++)
				Printf("%02bx",buff[i+j]);

			Printf(" %04x",*pTemp);

		}
	}
#endif
}
/**
* wait reg mask result max_wait
* Note: it is not I2C function.
*/
void MonWait(void)
{
	WORD i,max;
	BYTE reg, mask, result;
	if( argc<5 ) {
		Printf("   --> Missing parameter !!!");
#ifdef DEBUG_UART
		DEBUG_dump_uart0();
#endif
		return;
	}
	reg = a2h( argv[1] );
	mask = a2h( argv[2] );
	result = a2h( argv[3] );
	max = a2h( argv[4] );
	for(i=0; i < max; i++) {
		if((ReadTW88(reg) & mask)==result) {
			Printf("=>OK@%bd",i);
			break;
		}
		delay1ms(2);
	}
	if(i >= max)
		Printf("=>fail wait %bx %bx %bx %d->fail",reg,mask,result,max);
}

//=============================================================================
//			Help Message
//=============================================================================
void MonHelp(void)
{
	Puts("\n\r=======================================================");
	Puts("\n\r>>>     Welcome to Intersil Monitor  Rev 1.03       <<<");
	Puts("\n\r=======================================================");
	Puts("\n\r   R ii             ; Read data.(");
	Puts("\n\r   W ii dd          ; Write data.)");
	Puts("\n\r   D [ii] [cc]      ; Dump.&");
	Puts("\n\r   B AA II bb DD    ; Bit Operation. bb:StartEnd");
	Puts("\n\r    for multi size, use Rab format. a=idx size,b=data size");
	Puts("\n\r   C aa             ; Change I2C address");
	Puts("\n\r   Echo On/Off      ; Terminal Echoing On/Off");
//	Puts("\n\r   HDMI [init start dnedid dnhdcp avi timereg]");
	Puts("\n\r=======================================================");
	Puts("\n\r=== DEBUG ACCESS ECHO DELAY WAIT cache task isr time ==");
	Puts("\n\r=== MCU SPI SPIC EE I2C  key touch                   ==");
	Puts("\n\r=== menu fosd sosd init check tsc_debug trace        ==");
	Puts("\n\rimain [CVBS|SVIDEO|COMP|PC|HDMI|LVDS|BT656]  ; select Input Mode");
	Puts("\n\riBT656 [LOOP|DEC|ARGB|DTV|LVDS|OFF]          ; select BT656 Input");
	Puts("\n\rCheckAndSet                                  ; CheckAndSet input");
	Puts("\n\r=======================================================");
	Puts("\n\r");
}
void MonHelpHelp()
{
	Puts("\n\r=======================================================");
	Puts("\n\rHelp for Help");
	Puts("\n\r=======================================================");
	Puts("\n\rR idx                     ;read register");
	Puts("\n\rRab idx                   ;multi byte version of 'R' command.");
	Puts("\n\r                          ;  a:index size. 1 or 2 Bytes.");
	Puts("\n\r                          ;  b:data size. 1,2 or 4 Bytes.");
	Puts("\n\r");
	Puts("\n\r( i2cid idx               ;explicit i2cid version of R command");
	Puts("\n\r(ab i2cid idx             ;multi byte version of '(' command.");
	Puts("\n\r");
	Puts("\n\rW idx data                ;write register");
	Puts("\n\rWab idx data              ;multi byte version of 'W' command.");
	Puts("\n\r");
	Puts("\n\r) i2cid idx               ;explicit i2cid version of 'W' command");
	Puts("\n\r)ab i2cid idx             ;multi byte version of ')' command.");
	Puts("\n\r");
	Puts("\n\rD [idx] [e_idx]           ;dump registers");
	Puts("\n\rDab [idx] [e_idx]         ;multi byte version of 'D' command");
	Puts("\n\r");
	Puts("\n\r& i2cid [idx] [e_idx]     ;explicit i2cid version of 'D' command");
	Puts("\n\r&ab i2cid [idx] [e_idx]   ;multi byte version of '&' command");
	Puts("\n\r");
	Puts("\n\rB i2cid SE data           ;toggle bit");
	Puts("\n\r                          ;  S,Startbit:high nibble");
	Puts("\n\r                          ;  E,Endbit:low nibble");
	Puts("\n\rBab i2cid SE data         ;multi byte version of 'B' command");
	Puts("\n\r                          ; if b is 2, or 4, SE shell be 16bit");
	Puts("\n\r                          ; and, Startbit:MS Byte");
	Puts("\n\r                          ;      Endbit:LS Byte.");
 	Puts("\n\r");

}
#ifdef SUPPORT_UART1
void Mon1Help(void)
{
	Puts1("\n\r=======================================================");
	Puts1("\n\r>>>     Welcome to Intersil Monitor  Rev 1.03       <<<");
	Puts1("\n\r=======================================================");
	Puts1("\n\r   R ii             ; Read data.(");
	Puts1("\n\r   W ii dd          ; Write data.)");
	Puts1("\n\r   D [ii] [cc]      ; Dump.&");
	Puts1("\n\r   B AA II bb DD    ; Bit Operation. bb:StartEnd");
	Puts1("\n\r    for multi size, use Rab format. a=idx size,b=data size");
	Puts1("\n\r   C aa             ; Change I2C address");
	Puts1("\n\r   Echo On/Off      ; Terminal Echoing On/Off");
    Puts1("\n\r");
	Printf1("\n\rHELP or H or ?");
}
#endif
//=============================================================================
//
//=============================================================================
/**
* Mon GetCommand
*
* @return 0: nothing. 1: found command.
*/
BYTE MonGetCommand(void)
{
	static BYTE comment=0;
	static BYTE incnt=0, last_argc=0;
	BYTE i, ch;
	BYTE ret=0;

	if( !RS_ready() ) return 0;
	ch = RS_rx();

	//----- if comment, echo back and ignore -----
	if( comment ) {
		if( ch=='\r' || ch==0x1b ) comment = 0;
		else { 
			Mon_tx(ch);
			return 0;
		}
	}
	else if( ch==';' ) {
		comment = 1;
		Mon_tx(ch);
		return 0;
	}

	//=====================================
	switch( ch ) {
	//--- ESC
	case 0x1b:	
		argc = 0;
		incnt = 0;
		comment = 0;
		Prompt();
		return 0;

	//--- end of string
	case '\r':
		if( incnt==0 ) {
			Prompt();
			break;
		}

		monstr[incnt++] = '\0';
		argc=0;
		//ignore control char.
		for(i=0; i<incnt; i++) 
			if( monstr[i] > ' ' ) 
				break;

		if( !monstr[i] ) {
			incnt = 0;
			comment = 0;
			Prompt();
			return 0;
		}
		argv[0] = &monstr[i];
		for(; i<incnt; i++) {
			if( monstr[i] <= ' ' ) {
				monstr[i]='\0';
#ifdef DEBUG_UART
    			Printf("(%s) ",  argv[argc]);
#endif
				i++;
				while( monstr[i]==' ' || monstr[i]=='\t') i++;
				argc++;
				if( monstr[i] ){
     			 	argv[argc] = &monstr[i];
				}
			}
		}

		ret = 1;
		last_argc = argc;
		incnt = 0;
		break;

	//--- repeat command
	case '/':
		argc = last_argc;
		ret = 1;
		break;

	//--- repeat command without CR
	case '`':
	{
		BYTE i, j, ch;

		for(j=0; j<last_argc; j++) {
			for(i=0; i<100; i++) {
				ch = argv[j][i];
				if( ch==0 ) {
					if( j==last_argc-1 ) return 0;
					ch = ' ';
					RS_ungetch( ch );
					break;
				}
				else {
					RS_ungetch( ch );
				}
			}
		}
		break;
	}

	//--- back space
	case 0x08:
		if( incnt ) {
			incnt--;
			Mon_tx(ch);
			Mon_tx(' ');
			Mon_tx(ch);
		}
		break;

	//--- decreamental write
	case ',':
		if( incnt ) {
			Mon_tx(ch);
			monstr[incnt++] = ch;
		}
		else
			MonIncDecI2C(0);
		break;

	case '<':
		if( incnt ) {
			Mon_tx(ch);
			monstr[incnt++] = ch;
		}
		else
			MonIncDecI2C(10);
		break;

	//--- increamental write
	case '.':
		if( incnt ) {
			Mon_tx(ch);
			monstr[incnt++] = ch;
		}
		else
			MonIncDecI2C(1);
		break;

	case '>':
		if( incnt ) {
			Mon_tx(ch);
			monstr[incnt++] = ch;
		}
		else
			MonIncDecI2C(11);
		break;

	default:
		Mon_tx(ch);
		monstr[incnt++] = ch;
#ifdef DEBUG_UART
		if(incnt==50) {	   //BK130116
			Puts("###");  //overflow
		}
#endif
		break;
	}

	if( ret ) {
		comment = 0;
		last_argc = argc;
		return ret;
	}
	else {
		return ret;
	}
}

#ifdef SUPPORT_UART1
/**
* Mon1 GetCommand
*
*/
BYTE Mon1GetCommand(void)
{
	static BYTE comment1=0;
	static BYTE incnt1=0, last_argc1=0;
	BYTE i, ch;
	BYTE ret=0;

	if( !RS1_ready() ) return 0;
	ch = RS1_rx();

	//----- if comment, echo back and ignore -----
	if( comment1 ) {
		if( ch=='\r' || ch==0x1b ) comment1 = 0;
		else { 
			Mon1_tx(ch);
			return 0;
		}
	}
	else if( ch==';' ) {
		comment1 = 1;
		Mon1_tx(ch);
		return 0;
	}

	//=====================================
	switch( ch ) {

	case 0x1b:
		argc1 = 0;
		incnt1 = 0;
		comment1 = 0;
		Prompt1();
		return 0;

	//--- end of string
	case '\r':

		if( incnt1==0 ) {
			Prompt1();
			break;
		}

		mon1str[incnt1++] = '\0';
		argc1=0;

		for(i=0; i<incnt1; i++) if( mon1str[i] > ' ' ) break;

		if( !mon1str[i] ) {
			incnt1 = 0;
			comment1 = 0;
			Prompt1();
			return 0;
		}
		argv1[0] = &mon1str[i];
		for(; i<incnt1; i++) {
			if( mon1str[i] <= ' ' ) {
				mon1str[i]='\0';
     			 //Printf("(%s) ",  argv[argc]);
				i++;
				while( mon1str[i]==' ' ) i++;
				argc1++;
				if( mon1str[i] ){
     			 argv1[argc1] = &mon1str[i];
				}
			}
		}

		ret = 1;
		last_argc1 = argc1;
		incnt1 = 0;
		
		break;

	//--- repeat command
	case '/':
		argc1 = last_argc1;
		ret = 1;
		break;

	//--- repeat command without CR
	case '`':
	{
		BYTE i, j, ch;

		for(j=0; j<last_argc1; j++) {
			for(i=0; i<100; i++) {
				ch = argv1[j][i];
				if( ch==0 ) {
					if( j==last_argc1-1 ) return 0;
					ch = ' ';
					RS1_ungetch( ch );
					break;
				}
				else {
					RS1_ungetch( ch );
				}
			}
		}
		break;
	}

	//--- back space
	case 0x08:
		if( incnt1 ) {
			incnt1--;
			Mon1_tx(ch);
			Mon1_tx(' ');
			Mon1_tx(ch);
		}
		break;

	//--- decreamental write
	case ',':
		if( incnt1 ) {
			Mon1_tx(ch);
			mon1str[incnt1++] = ch;
		}
		//else
		//	MonIncDecI2C(0);	  //BKTODO??
		break;

	case '<':
		if( incnt1 ) {
			Mon1_tx(ch);
			mon1str[incnt1++] = ch;
		}
		//else
		//	MonIncDecI2C(10);	//??BKTODO??
		break;

	//--- increamental write
	case '.':
		if( incnt1 ) {
			Mon1_tx(ch);
			mon1str[incnt1++] = ch;
		}
		//else
		//	MonIncDecI2C(1);	//??BKTODO
		break;

	case '>':
		if( incnt1 ) {
			Mon1_tx(ch);
			mon1str[incnt1++] = ch;
		}
		//else
		//	MonIncDecI2C(11);	//BKTODO
		break;

	default:
		Mon1_tx(ch);
		mon1str[incnt1++] = ch;
		break;
	}

	if( ret ) {
		comment1 = 0;
		last_argc1 = argc1;
		return ret;
	}
	else {
		return ret;
	}
}
#endif

//*****************************************************************************
//				Monitoring Command
//*****************************************************************************

BYTE *MonString = 0;
//extern CODE BYTE DataInitADC[];
//extern CODE BYTE DataInitYUV[];
//extern CODE BYTE DataInitNTSC[];
//extern CODE BYTE DataInitDTV[];
//extern CODE BYTE DataInitTCON[];

/*
    return 1:TwCommand 0:No.
*/
BYTE MonGetTwCmdLength(void)
{
    if(argv[0][1]==0) {
        MonIndexLen=1;
        MonDataLen=1;
        return 1;
    }

    if(argv[0][1]!='1' && argv[0][1]!='2')
        return 0;
    MonIndexLen = argv[0][1] - '0';                

    if(argv[0][2] < '1' && argv[0][2] > '4')
        return 0;
    MonDataLen = argv[0][2] - '0';                
    return 1;
}

/**
* monitor.
*
* get user command and execute it.
*/
void Monitor(void)
{
	WORD wValue;

	if( MonString ) {																				  
		RS_ungetch( *MonString++ );
		if( *MonString==0 ) 
            MonString = 0;
	}

	if( !MonGetCommand() ) 
        return;

    //=====================================
    // Techwell Commands
    //=====================================

    //----------------------------------------------
    //example:
    //  r   0       ;read index:8bit  data:8bit
    //  r11 0       ;read index:8bit  data:8bit
    //  r12 0       ;read index:8bit  data:16bit
    //  r14 0       ;read index:8bit  data:24bit
    //  r21 0       ;read index:16bit data:8bit
    //  r22 0       ;read index:16bit data:16bit
    //  r24 0       ;read index:16bit data:24bit

    //----------------------------------------------
    //R	idx	   		;read register
    //Rab idx  		;multi byte version of 'R' command.
    //				;	a:index size. 1 or 2 Bytes.
    //         		;	b:data size. 1,2 or 4 Bytes.
    //( i2cid idx	;slave version of R command
    //(ab i2cid	idx	;multi byte version of '(' command.
    //----------------------------------------------
    if(argv[0][0]=='r' || argv[0][0]=='R') {
        if(MonGetTwCmdLength()) {
            MonReadI2C_multi(0, MonIndexLen,MonDataLen); 
			Prompt();
            return; 
        }
    }
    else if(argv[0][0]=='(') {
        if(MonGetTwCmdLength()) {
            MonReadI2C_multi(1, MonIndexLen,MonDataLen); 
			Prompt();
            return; 
        }
    }
    //----------------------------------------------
	//W	idx data	   		;write register
	//Wab idx data  		;multi byte version of 'W' command.
	//						;	a:index size. 1 or 2 Bytes.
	//         				;	b:data size. 1,2 or 4 Bytes.
	//) i2cid idx			;slave version of 'W' command
	//)ab i2cid	idx			;multi byte version of ')' command.
    //----------------------------------------------
	else if(argv[0][0]=='w' || argv[0][0]=='W') {
        if(MonGetTwCmdLength()) {
            MonWriteI2C_multi(0, MonIndexLen,MonDataLen); 
			Prompt();
            return; 
        }
    }
	else if(argv[0][0]==')') {
        if(MonGetTwCmdLength()) {
            MonWriteI2C_multi(1, MonIndexLen,MonDataLen); 
			Prompt();
            return; 
        }
    }
    //----------------------------------------------
	//D [idx] [end_idx]			;dump registers
	//Dab [idx] [end_idx]		;multi byte version of 'D' command
	//& i2cid [idx] [end_idx]	;slave version of 'D' command
	//&ab i2cid [idx] [end_idx]	;multi byte version of '&' command
    //----------------------------------------------
	else if(argv[0][0]=='d' || argv[0][0]=='D') {
        if(MonGetTwCmdLength()) {
            MonDumpI2C_multi(0, MonIndexLen,MonDataLen); 
			Prompt();
            return; 
        }
    }
	else if(argv[0][0]=='&') {
        if(MonGetTwCmdLength()) {
            MonDumpI2C_multi(1, MonIndexLen,MonDataLen); 
			Prompt();
            return; 
        }
    }
    //----------------------------------------------
	//B i2cid StartbitEndbit data	;toggle bit
	//								;Startbit:high nibble
	//								;Endbit:low nibble
	//Bab i2cid StartbitEndbit data	;multi byte version of 'B' command
	//								;if b is 2, or 4, StartbitEndbit shell be 16bit
	//								; and, Startbit:MSByte
	//								;      Endbit:LSByte.
    //----------------------------------------------
	else if(argv[0][0]=='b' || argv[0][0]=='B') {
        if(MonGetTwCmdLength()) {
            MonToggleBit_multi(1, MonIndexLen,MonDataLen); 
			Prompt();
            return; 
        }
    }
    //-----------------------
    //MONITOR_COMMAND_PARSE:
    //-----------------------
	//---------------- Change I2C -----------------------
	if( !stricmp( argv[0], "C" ) ) {
		MonAddress = a2h( argv[1] );
		Printf("\n\rSetMonAddress:%d",__LINE__);
	}
	//---------------- wait -----------------------
	else if( !stricmp( argv[0], "wait" ) ) {
		MonWait();
	}
	//---------------- delay -----------------------
	else if( !stricmp( argv[0], "delay" ) ) {
		wValue = a2i( argv[1] );
		delay1ms(wValue);
	}
	//---------------- Debug Level ---------------------
	else if ( !stricmp( argv[0], "DEBUG" ) ) {
		if( argc==2 ) {
			DebugLevel = a2h(argv[1]);
		}
		Printf("\n\rDebug Level = %2bx", DebugLevel);
	}
	//---------------- SW Key pad ---------------------
	else if ( !stricmp( argv[0], "KEY" ) ) {
		if( argc==2 ) {
			SW_key = a2h(argv[1]);
		}
		Printf("\n\rSW_key = %2bx", SW_key);
	}
	//---------------- Echo back on/off -----------------
	else if ( !stricmp( argv[0], "echo" ) ) {
		if( !stricmp( argv[1], "off" ) ) {
			echo = 0;
			Printf("\n\recho off");
		}
		else {
			echo = 1;
			Printf("\n\recho on");
		}
	}
	//---------------- access on/off -----------------
	else if ( !stricmp( argv[0], "ACCESS" ) ) {
		if(argc < 2) {
			Printf("\n\rAccess %s", g_access ? "on" : "off");
		}
		else if( !stricmp( argv[1], "0" ) ) {
			g_access = 0;
			Printf("\n\rAccess off");
			//disable interrupt.
			WriteTW88(REG003, 0xFE );	// enable only SW interrupt
		}
		else {
			g_access = 1;
			Printf("\n\rAccess on");
		}
	}
	//---------------- cache on/off -----------------
	else if ( !stricmp( argv[0], "cache" ) ) {
		if(argc >=2) {
			if( !stricmp( argv[1], "on" ) )
				SFR_CACHE_EN = 1;
			else if( !stricmp( argv[1], "off" ) )
				SFR_CACHE_EN = 0;
			else
				Printf("\n\rusage:cache [on|off]");
		}
		//cache status
		if(SFR_CACHE_EN)	Printf("\n\rcache on");
		else				Printf("\n\rcache off");
	}
	//---------------- Help -----------------------------
	else if( !stricmp( argv[0], "H" ) 
		||   !stricmp( argv[0], "HELP" ) 
		||   !stricmp( argv[0], "?" ) ) {
		MonHelp();
	}
	else if( !stricmp( argv[0], "??" ) ) {
		MonHelpHelp();
	}

	//------------------------------------------------------
	//---------------- OTHERS  -----------------------------
	//------------------------------------------------------
#ifdef SUPPORT_DELTA_RGB
	else if( !stricmp( argv[0], "AUO" ) ) 	monitor_auo();
	else if( !stricmp( argv[0], "AUO2" ) ) 	monitor_auo1()
#endif
#if	defined(SUPPORT_BT656)
	else if( !stricmp( argv[0], "bt656" )) 	monitor_bt656();	
#endif
	else if( !stricmp( argv[0], "bt656e" )) monitor_bt656e();
	else if( !stricmp( argv[0], "compiler_warning" )) monitor_compiler();
	else if( !stricmp( argv[0], "check" ) ) monitor_check();
	else if( !stricmp( argv[0], "CheckAndSet" ) ) monitor_checkandset();

#ifdef SUPPORT_DELTA_RGB
	else if( !stricmp( argv[0], "delta" ) ) monitor_delta();  //Delta RGB Panel Test
#endif
#ifdef USE_SFLASH_EEPROM
	else if( !stricmp( argv[0], "EE" ) ) 	MonitorEE();		//--EEPROM Debug		
#endif
	else if( !stricmp( argv[0], "fosd" )) 	MonitorFOsd();		//--Font Osd Debug
	else if( !stricmp( argv[0], "fosd" )) 	monitor_fosd();	//--FontOSD Test
#if 0
	else if( !stricmp( argv[0], "grid" ) ) 	monitor_grid();
#endif
	else if( !stricmp( argv[0], "HDMI" ) ) 	monitor_hdmi();


#if defined(SUPPORT_I2C_MASTER)	
	else if( !stricmp( argv[0], "i2c" ) ) 	monitor_i2c();	//i2c debug routine
#endif
	else if( !stricmp( argv[0], "i2cspi" ) ) 	monitor_i2cspi();
	else if( !stricmp( argv[0], "i2cspic" ) ) 	monitor_i2cspic();
	else if( !stricmp( argv[0], "ISR" ) ) 	monitor_isr();
	else if( !stricmp( argv[0], "init" ) ) 	monitor_init();
	else if( !stricmp( argv[0], "imain") ) 	monitor_imain();
#if	defined(SUPPORT_BT656)
	else if( !stricmp( argv[0], "iBT656") ) monitor_ibt656();
#endif
	else if( !stricmp( argv[0], "MCU" ) ) 	MonitorMCU();	//--MCU Debug
	else if( !stricmp( argv[0], "menu" ) ) 	MonitorMenu();  //-- MENU Debug
#if 0
	else if( !stricmp( argv[0], "pclk" ) ) 	monitor_pclk();
#endif
	else if( !stricmp( argv[0], "rpixel" )) monitor_rpixel();
#if defined(REMO_RC5) && defined(DEBUG_REMO)
	else if( !stricmp( argv[0], "remo")) 	monitor_remo();
#endif
	else if( !stricmp( argv[0], "SPI" ) ) 	MonitorSPI();   //--SPI Debug
	else if( !stricmp( argv[0], "SPIC" ) ) 	MonitorSPIC();	//--SPI Debug
	else if( !stricmp( argv[0], "sosd" ) )	MonitorSOsd();  //--SPI Osd Debug
	else if( !stricmp( argv[0], "sosd" )) 	monitor_sosd();	//--SpiOSD Test
	else if( !stricmp( argv[0], "scaler" )) monitor_scaler();
	else if( !stricmp( argv[0], "task" ) )	monitor_task();	//--task on/off
	else if( !stricmp( argv[0], "time" ) ) 	monitor_time(); //--System Clock Display
#ifdef SUPPORT_TOUCH
	else if( !stricmp( argv[0], "tsc" )) 	monitor_touch();	//--Touch Debug
#endif	
#ifdef DEBUG_UART
	else if( !stricmp( argv[0], "UART"))    monitor_uart();	//--debug UART
#endif
	//----------------------------------------------------
	else {
		Printf("\n\rInvalid command...");
	}
	Prompt();
}

#if 0
//monitor() is too long..1443 lines..
//make it simple
void Monitor()
{
	//if MonString, attach
	if( MonString ) {																				  
		RS_ungetch( *MonString++ );
		if( *MonString==0 ) 
			MonString = 0;
	}
	//If empty, exit
	if( !MonGetCommand() ) 
		return;

	//process basic commands for TW-Terminal & TW-Dongle
	//
	//c <i2c_id>
	//wait
	//delay
	//help ? ??


	//other
	//ddremo	->	PrintRemoCapture
	//UARTDUMP	->

	//i2c		-> monitor_i2c()
	//isr		->
	//init		->
	//imain		-> video input select
	//ibt656	-> bt656 input select
	//checkandset	->
	//checkclock,checkclock2
	//check ...
	//spi		-> monitor_spi
	//spic		-> monitor_spic
	//ee		-> monitor_ee
	//menu		-> monitor_menu
	//fosd		-> monitor_fontosd
	//sosd		-> monitor_spiosd
	//mcu		-> monitor_mcu
	//debug		-> 
	//key		-> sw key
	//echo		->
	//access	->
	//cache		->
	//task		->
	//time		->
	//pclk
	//testfon	->
	//ramfont
	//lutgrid	-> 
	//touch
	//tsc_debug
	//trace
	//calib
	//delta
	//auo
	//hdmi
	//tgrid autogrid  stepgrid
	//bt656		-> monitor_bt656()
	//bt656e	-> 
	//rpixel
	//compiler

	else if ( !stricmp( argv[0], "imain") ) monitor_imain();


}
#endif


//=============================================================================
//  UART1. 57600bps 8Data 1Stop NoParity NoFlowControl
//=============================================================================

#ifdef SUPPORT_UART1
BYTE *Mon1String = 0;
void ProcessFinishMessage(void)
{
	Puts1("OK");
}
/**
* monitor1
*
*/

void Monitor1(void)
{
	if( Mon1String ) {
		RS1_ungetch( *Mon1String++ );
		if( *Mon1String==0 ) Mon1String = 0;
	}

	if( !Mon1GetCommand() ) 
        return;

    //=====================================
    // Techwell Commands
    //=====================================
    //----------------------------------------------
    //R	idx	   		;read register
    //Rab idx  		;multi byte version of 'R' command.
    //				;	a:index size. 1 or 2 Bytes.
    //         		;	b:data size. 1,2 or 4 Bytes.
    //( i2cid idx	;slave version of R command
    //(ab i2cid	idx	;multi byte version of '(' command.
    //----------------------------------------------
    if(argv1[0][0]=='r' || argv1[0][0]=='R') {
        if(Mon1GetTwCmdLength()) {
            Mon1ReadI2C_multi(0, Mon1IndexLen,Mon1DataLen); 
			Prompt1();
            return; 
        }
    }
    else if(argv1[0][0]=='(') {
        if(Mon1GetTwCmdLength()) {
            Mon1ReadI2C_multi(1, Mon1IndexLen,Mon1DataLen); 
			Prompt1();
            return; 
        }
    }
    //----------------------------------------------
	//W	idx data	   		;write register
	//Wab idx data  		;multi byte version of 'W' command.
	//						;	a:index size. 1 or 2 Bytes.
	//         				;	b:data size. 1,2 or 4 Bytes.
	//) i2cid idx			;slave version of 'W' command
	//)ab i2cid	idx			;multi byte version of ')' command.
    //----------------------------------------------
	else if(argv1[0][0]=='w' || argv1[0][0]=='W') {
        if(Mon1GetTwCmdLength()) {
            Mon1WriteI2C_multi(0, Mon1IndexLen,Mon1DataLen); 
			Prompt1();
            return; 
        }
    }
	else if(argv1[0][0]==')') {
        if(Mon1GetTwCmdLength()) {
            Mon1WriteI2C_multi(1, Mon1IndexLen,Mon1DataLen); 
			Prompt1();
            return; 
        }
    }
    //----------------------------------------------
	//D [idx] [end_idx]			;dump registers
	//Dab [idx] [end_idx]		;multi byte version of 'D' command
	//& i2cid [idx] [end_idx]	;slave version of 'D' command
	//&ab i2cid [idx] [end_idx]	;multi byte version of '&' command
    //----------------------------------------------
	else if(argv1[0][0]=='d' || argv1[0][0]=='D') {
        if(Mon1GetTwCmdLength()) {
            Mon1DumpI2C_multi(0, Mon1IndexLen,Mon1DataLen); 
			Prompt1();
            return; 
        }
    }
	else if(argv1[0][0]=='&') {
        if(Mon1GetTwCmdLength()) {
            Mon1DumpI2C_multi(1, Mon1IndexLen,Mon1DataLen); 
			Prompt1();
            return; 
        }
    }
    //----------------------------------------------
	//B i2cid StartbitEndbit data	;toggle bit
	//								;Startbit:high nibble
	//								;Endbit:low nibble
	//Bab i2cid StartbitEndbit data	;multi byte version of 'B' command
	//								;if b is 2, or 4, StartbitEndbit shell be 16bit
	//								; and, Startbit:MSByte
	//								;      Endbit:LSByte.
    //----------------------------------------------
	else if(argv1[0][0]=='b' || argv1[0][0]=='B') {
        if(Mon1GetTwCmdLength()) {
            Mon1ToggleBit_multi(1, Mon1IndexLen,Mon1DataLen); 
			Prompt1();
            return; 
        }
    }
    //-----------------------
    //MONITOR_COMMAND_PARSE:
    //-----------------------
	//---------------- Change I2C -----------------------
	if( !stricmp( argv1[0], "C" ) ) {
		Mon1Address = a2h( argv1[1] );
		Printf("\n\rSetMon1Address:%d",__LINE__);
	}
	//---------------- wait -----------------------
	else if( !stricmp( argv1[0], "wait" ) ) {
		MonWait();
	}
	//---------------- delay -----------------------
	else if( !stricmp( argv1[0], "delay" ) ) {
		wValue = a2i( argv1[1] );
		delay1ms(wValue);
	}
	//---------------- Debug Level ---------------------
	else if ( !stricmp( argv1[0], "DEBUG" ) ) {
		if( argc1==2 ) {
			DebugLevel = a2h(argv1[1]);
		}
		Printf1("\n\rDebug Level = %2bx", DebugLevel);
	}
	//---------------- SW Key pad ---------------------
	else if ( !stricmp( argv1[0], "KEY" ) ) {
		if( argc1==2 ) {
			SW_key = a2h(argv1[1]);
		}
		Printf1("\n\rSW_key = %2bx", SW_key);
	}
	//---------------- Echo back on/off -----------------
	else if ( !stricmp( argv1[0], "echo" ) ) {
		if( !stricmp( argv1[1], "off" ) ) {
			echo = 0;
			Printf1("\n\recho off");
		}
		else {
			echo = 1;
			Printf1("\n\recho on");
		}
	}
	//---------------- access on/off -----------------
	else if ( !stricmp( argv1[0], "ACCESS" ) ) {
		if(argc1 < 2) {
			Printf1("\n\rAccess %s", g_access ? "on" : "off");
		}
		else if( !stricmp( argv1[1], "0" ) ) {
			g_access = 0;
			Printf1("\n\rAccess off");
			//disable interrupt.
			WriteTW88(REG003, 0xFE );	// enable only SW interrupt
		}
		else {
			g_access = 1;
			Printf1("\n\rAccess on");
		}
	}
	//---------------- cache on/off -----------------
	else if ( !stricmp( argv1[0], "cache" ) ) {
		if(argc1 >=2) {
			if( !stricmp( argv1[1], "on" ) )
				SFR_CACHE_EN = 1;
			else if( !stricmp( argv1[1], "off" ) )
				SFR_CACHE_EN = 0;
			else
				Printf1("\n\rusage:cache [on|off]");
		}
		//cache status
		if(SFR_CACHE_EN)	Printf1("\n\rcache on");
		else				Printf1("\n\rcache off");
	}
	//---------------- Help -----------------------------
	else if( !stricmp( argv1[0], "H" ) 
		||   !stricmp( argv1[0], "HELP" ) 
		||   !stricmp( argv1[0], "?" ) ) {
		Mon1Help();
	}
	else if( !stricmp( argv1[0], "??" ) ) {
		Mon1HelpHelp();
	}
	//----------------------------------------------------
	else {
		Printf1("\n\rInvalid command...");
	}
	Prompt1();
}
#endif

//---------------- check clock-------------------------
// command checkclock
//
// step.
//	access 0
//	mcu intc
//	checkclock and than type enter.
//  mcu intc again
//	test checkclock again
//..
//Note: When FW read SystemClock, use SFRB_EA.
//
void Test_Checkclock2(void)
{}
void Test_Checkclock(void)
{
	//WORD ii, loop_max;
	volatile DWORD count;
	volatile DWORD WaitTime,CapturedSystemClock;
	DWORD count_72_low=0, count_72_high;
	DWORD count_54_low=0, count_54_high;
	DWORD count_36_low=0, count_36_high;
	DWORD count_27_low=0, count_27_high;
	DWORD lloopp;
	WORD WaitInterval=50; //200*10mSec
	BYTE cache;

	//loop_max = 0x1000;
	Printf("\n\rPress any key to Quit...");

	cache = SFR_CACHE_EN;
	SFR_CACHE_EN = 0;
	WriteTW88(REG400, ReadTW88(REG400) & ~0x04);
	SFRB_EX0	= 0;
	SFRB_EINT4 = 0;
	SFRB_ET1 = 0;
	DisableRemoInt();
	lloopp = 0;

	while(!RS_ready()) {
		Printf("\r%ld",lloopp);
		lloopp++;
		//--------------------
		//Printf("\n\r27MHz..");
		WriteTW88(REG4E1, ReadTW88(REG4E1) & 0xCF);
		count = 0;
		WaitOneVBlank();
		SFRB_ET0 = 0;
		CapturedSystemClock = SystemClock;
		SFRB_ET0 = 1;
		WaitTime = CapturedSystemClock + WaitInterval;
		do {
			count++;
			SFRB_ET0 = 0;
			CapturedSystemClock = SystemClock;
			SFRB_ET0 = 1;
		} while( WaitTime > CapturedSystemClock);
		//Printf(" count:%ld",count);
		if(count_27_low==0) {
			count_27_low = count * 0.8;
			count_27_high = count * 1.2;
			Printf("\n\r27MHz...");
			Printf(" count:%ld",count);
			Printf(" %ld~%ld",count_27_low,count_27_high);
		}
		else {
			if(count < count_27_low || count > count_27_high) {
				Printf("\n\rFAIL:27MHz..");
				Printf(" count:%ld",count);
				Printf(" WaitTime:%ld,SystemClock:%ld",WaitTime,CapturedSystemClock);
				break;
			}
			Printf(".");
		}


		//Printf("\n\rPLL 72..");
		PllClkSetSource(PLLCLK_PLL108);	//WriteTW88(REG4E0, ReadTW88(REG4E0) | 0x01);
		WriteTW88(REG4E1, 0x21);
		count = 0;
		WaitOneVBlank();
		SFRB_ET0 = 0;
		CapturedSystemClock = SystemClock;
		SFRB_ET0 = 1;
		WaitTime = CapturedSystemClock + WaitInterval;
		do {
			count++;
			SFRB_ET0 = 0;
			CapturedSystemClock = SystemClock;
			SFRB_ET0 = 1;
		} while( WaitTime > CapturedSystemClock);
		//Printf(" count:%ld",count);
		if(count_72_low==0) {
			count_72_low = count * 0.8;
			count_72_high = count * 1.2;
			Printf("\n\rPLL 72..");
			Printf(" count:%ld",count);
			Printf(" %ld~%ld",count_72_low,count_72_high);
		}
		else {
			if(count < count_72_low || count > count_72_high) {
				Printf("\n\rFAIL:PLL 72..");
				Printf(" count:%ld",count);
				Printf(" WaitTime:%ld,SystemClock:%ld",WaitTime,CapturedSystemClock);
				break;
			}
			Printf(".");
		}


		//Printf("\n\rPLL 54..");
		WriteTW88(REG4E1, 0x22);
		count = 0;
		WaitOneVBlank();
		SFRB_ET0 = 0;
		CapturedSystemClock = SystemClock;
		SFRB_ET0 = 1;
		WaitTime = CapturedSystemClock + WaitInterval;
		do {
			count++;
			SFRB_ET0 = 0;
			CapturedSystemClock = SystemClock;
			SFRB_ET0 = 1;
		} while( WaitTime > CapturedSystemClock);
		//Printf(" count:%ld",count);
		if(count_54_low==0) {
			count_54_low = count * 0.8;
			count_54_high = count * 1.2;
			Printf("\n\rPLL 54..");
			Printf(" count:%ld",count);
			Printf(" %ld~%ld",count_54_low,count_54_high);
		}
		else {
			if(count < count_54_low || count > count_54_high) {
				Printf("\n\rFAIL:PLL 54..");
				Printf(" count:%ld",count);
				Printf(" WaitTime:%ld,SystemClock:%ld",WaitTime,CapturedSystemClock);
				break;
			}
			Printf(".");
		}




		//Printf("\n\rPLL 36..");
		WriteTW88(REG4E1, 0x24);
		count = 0;
		WaitOneVBlank();
		SFRB_ET0 = 0;
		CapturedSystemClock = SystemClock;
		SFRB_ET0 = 1;
		WaitTime = CapturedSystemClock + WaitInterval;
		do {
			count++;
			SFRB_ET0 = 0;
			CapturedSystemClock = SystemClock;
			SFRB_ET0 = 1;
		} while( WaitTime > CapturedSystemClock);
		//Printf(" count:%ld",count);
		if(count_36_low==0) {
			count_36_low = count * 0.8;
			count_36_high = count * 1.2;
			Printf("\n\rPLL 36..");
			Printf(" count:%ld",count);
			Printf(" %ld~%ld",count_36_low,count_36_high);

			Printf("\n\r");
		}
		else {
			if(count < count_36_low || count > count_36_high) {
				Printf("\n\rFAIL:PLL 36..");
				Printf(" count:%ld",count);
				Printf(" WaitTime:%ld,SystemClock:%ld",WaitTime,CapturedSystemClock);
				break;
			}
			Printf(".");
		}



		//Printf("\n\rPLL 27..");
		WriteTW88(REG4E1, 0x26);
		count = 0;
		WaitOneVBlank();
		SFRB_ET0 = 0;
		CapturedSystemClock = SystemClock;
		SFRB_ET0 = 1;
		WaitTime = CapturedSystemClock + WaitInterval;
		do {
			count++;
			SFRB_ET0 = 0;
			CapturedSystemClock = SystemClock;
			SFRB_ET0 = 1;
		} while( WaitTime > CapturedSystemClock);
		//Printf(" count:%ld",count);
		if(count < count_27_low || count > count_27_high) {
			Printf("\n\rFAIL:PLL 27..");
			Printf(" count:%ld",count);
			Printf(" WaitTime:%ld,SystemClock:%ld",WaitTime,CapturedSystemClock);
			break;
		}
		Printf(".");



		//--------------------
		//Printf("\n\rPCLK1 72..");
		WriteTW88(REG4E1, 0x01);	//27MHz first
		PllClkSetSource(PLLCLK_SSPLL);		//WriteTW88(REG4E0, ReadTW88(REG4E0) & ~0x01);
		WriteTW88(REG4E1, 0x21);
		count = 0;
		WaitOneVBlank();
		SFRB_ET0 = 0;
		CapturedSystemClock = SystemClock;
		SFRB_ET0 = 1;
		WaitTime = CapturedSystemClock + WaitInterval;
		do {
			count++;
			SFRB_ET0 = 0;
			CapturedSystemClock = SystemClock;
			SFRB_ET0 = 1;
		} while( WaitTime > CapturedSystemClock);
		//Printf(" count:%ld",count);
		if(count < count_72_low || count > count_72_high) {
			Printf("\n\rFAIL:PCLK 72..");
			Printf(" count:%ld",count);
			Printf(" WaitTime:%ld,SystemClock:%ld",WaitTime,CapturedSystemClock);
			break;
		}
		Printf(".");


		//Printf("\n\rPCLK1 54..");
		WriteTW88(REG4E1, 0x22);
		count = 0;
		WaitOneVBlank();
		SFRB_ET0 = 0;
		CapturedSystemClock = SystemClock;
		SFRB_ET0 = 1;
		WaitTime = CapturedSystemClock + WaitInterval;
		do {
			count++;
			SFRB_ET0 = 0;
			CapturedSystemClock = SystemClock;
			SFRB_ET0 = 1;
		} while( WaitTime > CapturedSystemClock);
		//Printf(" count:%ld",count);
		if(count < count_54_low || count > count_54_high) {
			Printf("\n\rFAIL:PCLK 54..");
			Printf(" count:%ld",count);
			Printf(" WaitTime:%ld,SystemClock:%ld",WaitTime,CapturedSystemClock);
			break;
		}
		Printf(".");


		//Printf("\n\rPCLK1 36..");
		WriteTW88(REG4E1, 0x24);
		count = 0;
		WaitOneVBlank();
		SFRB_ET0 = 0;
		CapturedSystemClock = SystemClock;
		SFRB_ET0 = 1;
		WaitTime = CapturedSystemClock + WaitInterval;
		do {
			count++;
			SFRB_ET0 = 0;
			CapturedSystemClock = SystemClock;
			SFRB_ET0 = 1;
		} while( WaitTime > CapturedSystemClock);
		//Printf(" count:%ld",count);
		if(count < count_36_low || count > count_36_high) {
			Printf("\n\rFAIL:PCLK 36..");
			Printf(" count:%ld",count);
			Printf(" WaitTime:%ld,SystemClock:%ld",WaitTime,CapturedSystemClock);
			break;
		}
		Printf(".");

		//Printf("\n\rPCLK1 27..");
		WriteTW88(REG4E1, 0x26);
		count = 0;
		WaitOneVBlank();
		SFRB_ET0 = 0;
		CapturedSystemClock = SystemClock;
		SFRB_ET0 = 1;
		WaitTime = CapturedSystemClock + WaitInterval;
		do {
			count++;
			SFRB_ET0 = 0;
			CapturedSystemClock = SystemClock;
			SFRB_ET0 = 1;
		} while( WaitTime > CapturedSystemClock);
		//Printf(" count:%ld",count);
		if(count < count_27_low || count > count_27_high) {
			Printf("\n\rFAIL:PCLK 27..");
			Printf(" count:%ld",count);
			Printf(" WaitTime:%ld,SystemClock:%ld",WaitTime,CapturedSystemClock);
			break;
		}
		Printf(".");


		//restore
		WriteTW88(REG4E1, 0x01);	//27MHz

		//Printf("+++");
		//Printf("\r");
	}
	SFR_CACHE_EN = cache;
}


void Test_McuSpeed(WORD count)
{
	DWORD StartClock, UsedClock;
	BYTE hour,min,sec,ms;
	BYTE j,k;
	WORD i;


	Printf("\n\rStart");
	SFR_CACHE_EN = 0;

	SFRB_ET0 = 0;
	StartClock = SystemClock;
	SFRB_ET0 = 1;

	for(i=0; i < count; i++) {
		for(j=0; j < 100; j++) {
			for(k=0; k < 100; k++) {
				;
			}	
		}
		Puts(".");			
	}
	SFRB_ET0 = 0;
	UsedClock = SystemClock;
	SFRB_ET0 = 1;
	UsedClock -= StartClock;



	ms = (BYTE)(UsedClock%100);
	UsedClock = UsedClock/100;
	if(UsedClock==0) {
		sec = 0;
		min = 0;
		hour = 0;
	}
	else {
		sec = (BYTE)(UsedClock % 60);
		UsedClock = UsedClock / 60;
		if(UsedClock==0) {
			min = 0;
			hour = 0;
		}
		else {
			min = (BYTE)(UsedClock%60);
			hour = UsedClock / 60;
		}
	}
	Printf("\n\rUsedClock %bd:%02bd:%02bd.%02bd", hour,min,sec ,ms);


	SFR_CACHE_EN = 1;	
}

//struct ch_t {
//	char c0;
//	char c1;
//};
//union combo_t {
//	WORD w;
//	struct ch_t ch;
//}
//
//void Test_Union(void)
//{
//	union combo_t x;
//		
//}

