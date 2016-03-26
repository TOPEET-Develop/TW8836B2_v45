/**
 * @file
 * InputCtrl.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	see video input control 
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
* video input control 
*
*	+-----+ LoSpeed  +-----+  +---------+   +-+           +------+
*	|     | Decoder  |     |=>| decoder |==>| |==========>|      |
*	|     | =======> |     |  +---------+   |I|           |      |
*	|     |          |     |                |n|           |      |
*	|     | HiSpeed  |AMux |                | |           |      |
*	|     | ARGB     |     |  +---------+   |M|           |      |
*	|INPUT| =======> |     |=>|  ARGB   |==>|U|==========>|Scaler|
*	|     |          +-----+  +---------+   |X|           |      |
*	|     | Digital                         | |           |      |
*	|     | DTV                             | |  +-----+  |      |
*	|     | ===============================>| |=>| DTV |=>|      |
*	+-----+                                 +-+  +-----+  +------+
*	                                         |    +--------+
*	                                         +==> |Measure |
*	                                              +--------+
*/

#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "Global.h"
#include "CPU.h"
#include "Printf.h"
#include "util.h"
#include "Monitor.h"

#include "I2C.h"
#include "spi.h"

#include "main.h"
#include "SOsd.h"
#include "FOsd.h"
#include "decoder.h"
#include "Scaler.h"
#include "InputCtrl.h"
#include "EEPROM.h"
#include "ImageCtrl.h"
#include "Settings.h"
#include "measure.h"
#include "aRGB.h"
#include "dtv.h"
#include "InputCtrl.h"
//#include "OutputCtrl.h"
#include "SOsdMenu.h"

#ifdef SUPPORT_HDMI_TW8837
#include "hdmi_TW8837.H"
#endif
#ifdef SUPPORT_HDMI_EP907M
#include "hdmi_EP907M.H"
#endif

#include "BT656.h"
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
// INPUT CONTROL
//=============================================================================
// Input Module
// start from 0x040
//0x040~0x049
//R040[1:0]	Input Select		0:InternalDecoder,1:ARGB/YUV(YPbPr),2:DTV(BT656)
//R041[0]	Input data format	0:YCbCr 1:RGB
//=============================================================================

XDATA	BYTE	InputMain;
XDATA	BYTE	InputBT656;
XDATA	BYTE	InputSubMode;

//-----------------------------------------------------------------------------
/**
* Get InputMain value
*
* friend function.
* Other bank, specially Menu Bank(Bank2) needs this InputMain global variable.
*/
BYTE GetInputMain(void)
{
	return InputMain;
}
//-----------------------------------------------------------------------------
/**
* Set InputMain value
*
* @see GetInputMain
*/
void SetInputMain(BYTE input)
{
	InputMain = input;
	//update EE
}

#define VBLANK_WAIT_VALUE	0xFFFE 

//-----------------------------------------------------------------------------
/**
* wait Vertical Blank
*
* You can use this function after you turn off the PD_SSPLL(REG0FC[7]).
* 0xFFFE value uses max 40ms on Cache + 72MHz.
*/
void WaitVBlank(BYTE cnt)
{
	XDATA	BYTE i;
	WORD loop;

	for ( i=0; i<cnt; i++ ) {
		WriteTW88(REG002, 0xff );
		loop = 0;
		while (!( ReadTW88(REG002 ) & 0x40 ) ) {
			// wait VBlank
			loop++;
			if(loop > VBLANK_WAIT_VALUE  ) {
				wPrintf("\n\rERR:WaitVBlank");
				break;
			}
		}		
	}
}

//-----------------------------------------------------------------------------
/**
* wait Vertical Blank
*
* @see WaitVBlank
*/
void WaitOneVBlank(void)
{
	WORD loop;
	//volatile BYTE vdata; if you want to chech REG002, use volatile.

	WriteTW88(REG002, 0xff );
	loop = 0;
	while (!( ReadTW88(REG002 ) & 0x40 ) ) {
		// wait VBlank
		loop++;
		if(loop > VBLANK_WAIT_VALUE  ) {
			wPrintf("\n\rERR:WaitVBlank");
			break;
		}
	}
}

//-----------------------------------------------------------------------------
//class:Input
/**
* Set input path & color domain
*
*	register
#	REG040[7:6]
*	REG040[5]
*	REG040[4]	input clock polarity.	1:inversion
*	REG040[3]	Enable DTVDE on PIN63 for DTV
*	REG040[2]	Enable 2nd DTVCLK on PIN62 (for BT656Decoder)
*	REG040[1:0]	Input Selection
*		00:Decoder, 01:aRGB  10:DTV 11:LVDSRX
*	REG041[0]
*		0:YCbCr  1:RGB
*
*	REG070[5] Process as RGB
*
* @param path: input mode
*		- 0:InternalDecoder
*		- 1:AnalogRGB/Component. PC or Component
*		- 2:DTV
*		- 3:LVDS-RX 
*		- 0x06: BT656. DTV+2nd DTVCLK. (SW)                                 
* @param format: data format.
*		- 0:YUV 1:RGB
*
*/
//scaler_set_input_source
void InputSetSource(BYTE path, BYTE format)
{
	BYTE r040, r041;

	r040 = ReadTW88(REG040_INPUT_CTRL_I) & ~0x17;	//clear [2] also.
	r041 = ReadTW88(REG041_INPUT_CTRL_II) & ~0x3F;
	r040 |= path;
	r041 |= format;

	if(path==INPUT_PATH_DECODER) {		//InternalDecoder
										//It is an ImplicitDE, but donot need a IMPDE flag.
		r041 |= 0x0C;					//input sync detion edge control. falling edge
	}
	else if(path==INPUT_PATH_VADC) {	//ARGB(PC or Component)
										//It is an ImplicitDE, but donot need a IMPDE flag.
		r040 |= 0x10;					//invert clock
		if(InputMain==INPUT_COMP) {
			r041 |= 0x20;				//progressive
			r041 |= 0x10;				//implicit DE mode.(Component, don't care)
			r041 |= 0x0C;				//input sync detion edge control. falling edge
			r041 |= 0x02;				//input field inversion
		}
		else {
			//r041 |= 0x20;				//progressive
			r041 |= 0x10;				//implicit DE mode.(Component, don't care)
			//r041 |= 0x0C;				//input sync detion edge control. falling edge
		}
	}
	else if(path==INPUT_PATH_DTV) {		//DTV
										//clock normal
		r040 |= 0x08;					//INT_4 pin is turn into dtvde pin
		//r041 |= 0x20;					// progressive
		r041 |= 0x10;					//implicit DE mode
		//r041 |= 0x0C;					//input sync detion edge control. falling edge
	}
	else if(path==INPUT_PATH_LVDS) {	//LVDS_RX
		r040 |= 0x08;					//INT_4 pin is turn into dtvde pin
#if 1 //Errara150407
		r040 |= 0x10;					//use Invert Clock for LVDS-RX
#endif
		r041 |= 0x10;					//implicit DE mode
	}
	else if(path==INPUT_PATH_BT656) {
		//target r040:0x06 r041:0x00
	}
	//dPrintf("\n\rInputSetSource r040:%bx r041:%bx",r040,r041);
	WriteTW88(REG040_INPUT_CTRL_I,r040);
	WriteTW88(REG041_INPUT_CTRL_II,r041);
}

void InputSetClockPolarity(BYTE fInv)
{
	BYTE bTemp;
	bTemp = ReadTW88(REG040);
	if(fInv) WriteTW88(REG040, bTemp |  0x10);
	else	 WriteTW88(REG040, bTemp & ~0x10);
}


//-----------------------------------------------------------------------------
//class:Input
#ifdef SUPPORT_ARGB
void InputSetProgressiveField(BYTE fOn)
{
	if(fOn)	WriteTW88(REG041_INPUT_CTRL_II, ReadTW88(REG041_INPUT_CTRL_II) | 0x20);	    //On Field for Prog
	else	WriteTW88(REG041_INPUT_CTRL_II, ReadTW88(REG041_INPUT_CTRL_II) & ~0x20);	//Off Field for Prog
}
#endif

//-----------------------------------------------------------------------------
//class:Input
//scaler_set_input_polarity
void InputSetPolarity(BYTE V,BYTE H, BYTE F)
{
	BYTE r041;

	r041 = ReadTW88(REG041_INPUT_CTRL_II ) & ~0x0E;
	if(V)	r041 |= 0x08;
	if(H)	r041 |= 0x04;
	if(F)	r041 |= 0x02;
	WriteTW88(REG041_INPUT_CTRL_II, r041);
}
#ifdef UNCALLED_SEGMENT
//-----------------------------------------------------------------------------
//class:Input
//scaler_get_input_polarity_verti
//BYTE InputGetVPolarity(void)
//{
//	BYTE r041;
//
//	r041 = ReadTW88(REG041_INPUT_CTRL_II );
//	if(r041 & 0x08)	return ON;		//detect falling edge
//	else			return OFF;		//detect rising edge
//}
//-----------------------------------------------------------------------------
//class:Input
//scaler_get_input_polarity_horiz
//BYTE InputGetHPolarity(void)
//{
//	BYTE r041;
//
//	r041 = ReadTW88(REG041_INPUT_CTRL_II );
//	if(r041 & 0x04)	return ON;		//detect falling edge
//	else			return OFF;		//detect rising edge
//}
//-----------------------------------------------------------------------------
//class:Input
//scaler_get_input_polarity_field
//BYTE InputGetFieldPolarity(void)
//{
//	BYTE r041;
//
//	r041 = ReadTW88(REG041_INPUT_CTRL_II );
//	if(r041 & 0x02)	return ON;		//input field inversion
//	else			return OFF;		//
//}
#endif

#if defined(SUPPORT_COMPONENT)
//-----------------------------------------------------------------------------
//class:Input
/**
* set Field Polarity
*
* R041[1] input field control. 1:inversion
*
* 480i & 576i need a "1" on TW8836
*/
//scaler_set_input_field_pol
void InputSetFieldPolarity(BYTE fInv)
{
	BYTE r041;

	r041 = ReadTW88(REG041_INPUT_CTRL_II );
	if(fInv)	WriteTW88(REG041_INPUT_CTRL_II, r041 | 0x02);
	else 		WriteTW88(REG041_INPUT_CTRL_II, r041 & ~0x02);
}
#endif

#ifdef UNCALLED_SEGMENT
//-----------------------------------------------------------------------------
//class:Input
/*
* R041[0] Input data format selection 1:RGB
*/
//scaler_get_color_domain
BYTE InputGetColorDomain(void)
{
	BYTE r041;

	r041 = ReadTW88(REG041_INPUT_CTRL_II );
	if(r041 & 0x01)	return ON;		//RGB color
	else			return OFF;		//YUV color
}
#endif

//-----------------------------------------------------------------------------
//class:Input
/**
* set input crop
*
* input cropping for implicit DE.
* NOTE:InternalDecoder is not an implicit DE.
*
*	register
*	REG040[7:6]REG045[7:0]	HCropStart
*			   REG043[7:0]	VCropStart
*	REG042[6:4]REG044[7:0]	VCropLength
*	REG042[3:0]REG046[7:0]	HCropLength
*/
//scaler_set_inputcrop
void InputSetCrop( WORD x, WORD y, WORD w, WORD h )
{
	WriteTW88(REG040_INPUT_CTRL_I, (ReadTW88(REG040_INPUT_CTRL_I) & 0x3F) | ((x & 0x300)>>2) );
	WriteTW88(REG045, (BYTE)x);
	WriteTW88(REG043, (BYTE)y);

	WriteTW88(REG042, ((h&0xF00) >> 4)|(w >>8) );
	WriteTW88(REG044, (BYTE)h);
	WriteTW88(REG046, (BYTE)w);
	//dPrintf("\n\rInput Crop Window: x = %d, y = %d, w = %d, h = %d", x, y, w, h );
}


#ifdef SUPPORT_PC
//-----------------------------------------------------------------------------
//class:Input
/**
* set Horizontal Start at InputCrop
*/
//scaler_set_inputcrop_hstart
void InputSetHStart( WORD x)
{
	WriteTW88(REG040, (ReadTW88(REG040) & 0x3F) | ((x & 0xF00)>>2) );
	WriteTW88(REG045, (BYTE)x);
	//dPrintf("\n\rInput Crop Window: x = %d", x);
}
#endif
//-----------------------------------------------------------------------------
//class:Input
/**
* get Horizontal Start at InputCrop
*/
//scaler_get_inputcrop_hstart
WORD InputGetHStart(void)
{
	WORD wValue;

	wValue = ReadTW88(REG040) & 0xC0;
	wValue <<= 2;
	wValue |=  ReadTW88(REG045);
	return wValue;
}

#if 0
//-----------------------------------------------------------------------------
//class:Input
//scaler_set_inputcrop_vstart
void InputSetVStart( WORD y)
{
	WriteTW88(REG043, (BYTE)y);
	//dPrintf("\n\rInput Crop Window: y = %d", y);
}
#endif
//-----------------------------------------------------------------------------
//class:Input
#ifdef DEBUG_SCALER_OVERWRITE_TABLE
//scaler_get_inputcrop_vstart
WORD InputGetVStart(void)
{
	WORD wValue;

	wValue = ReadTW88(REG043 );
	return wValue;
}
#endif

#if 1
//-----------------------------------------------------------------------------
//class:Input
//scaler_get_inputcrop_hactive()
WORD InputGetHLen(void)
{
	WORD len;
	len =ReadTW88(REG042) & 0x0F;
	len <<=8;
	len |= ReadTW88(REG046);
	return len;
}
//-----------------------------------------------------------------------------
//class:Input
//scaler_get_inputcrop_vactive()
WORD InputGetVLen(void)
{
	WORD len;
	len =ReadTW88(REG042) & 0x70;
	len <<=4;
	len |= ReadTW88(REG044);
	return len;
}
#endif


//-----------------------------------------------------------------------------
//class:BT656Input
//register
//	R047[7]	BT656 input control	0:External input, 1:Internal pattern generator
void BT656DecSetFreerun(BYTE fOn)
{
	if(fOn)	WriteTW88(REG047,ReadTW88(REG047) | 0x80);
	else	WriteTW88(REG047,ReadTW88(REG047) & ~0x80);
}
//-----------------------------------------------------------------------------
//class:BT656Input
/**
* set Freerun and invert clock flag on BT656
*
*	R047[7]
*	R047[5]
*/
#if 0
void BT656DecSetFreerunClk(BYTE fFreerun, BYTE fInvClk)
{
	BYTE value;
	value = ReadTW88(REG047);
	if(fFreerun)	value |= 0x80;
	else			value &= ~0x80;
	
	if(fInvClk)		value |= 0x20;
	else			value &= ~0x20;
	WriteTW88(REG047, value);
}
#endif
void Bt656DecSetClkPol(BYTE fInvClk)
{
	BYTE value;
	value = ReadTW88(REG047);
	if(fInvClk)		value |= 0x20;
	else			value &= ~0x20;
	WriteTW88(REG047, value);
}


//=====================================================
// LVDS Rx
//=====================================================
void LvdsRxEnable(BYTE fOn)
{
	BYTE bTemp;
	bTemp = ReadTW88(REG648);
	if(fOn) bTemp |=  0x02;
	else    bTemp &= ~0x02;
	WriteTW88(REG648, bTemp);
}
void LvdsRxPowerDown(BYTE fOn)
{
	BYTE bTemp;
	bTemp = ReadTW88(REG64C);
	if(fOn) bTemp &= ~0x40; //power down
	else    bTemp |=  0x40; //normal
	WriteTW88(REG64C, bTemp);
}
void InitLvdsRx(void)
{
	WriteTW88(REG648, 0x07);
	WriteTW88(REG649, 0x01);
	WriteTW88(REG64A, 0x00);
	WriteTW88(REG64B, 0x34);
	WriteTW88(REG64C, 0x40);
	WriteTW88(REG64D, 0x17);
	WriteTW88(REG64E, 0x00);
}



//-----------------------------------------------------------------------------
/**
* Change Video Input.
*
* @param mode
*	- INPUT_CVBS : ChangeCVBS
*	- INPUT_SVIDEO: ChangeCVBS
*	- INPUT_COMP : ChangeCOMPONENT
*	- INPUT_PC :  ChangePC
*	- INPUT_DVI : ChangeDVI
* 	- INPUT_HDMIPC:
*	- INPUT_HDMITV:	ChangeHDMI
*	- INPUT_BT656: ChangeBT656
* @see ChangeCVBS
*
* NOTE: DO NOT TURN ON SPIOSD when you change input.
*
*/
void ChangeInput( BYTE mode )
{
	Printf("\n\rChangeInput:");
	PrintfInput(mode,0);

	if(getNoSignalLogoStatus())
		RemoveLogo();


	switch ( mode ) {
#ifdef SUPPORT_CVBS
		case INPUT_CVBS:	ChangeCVBS();		break;
#endif
#ifdef SUPPORT_SVIDEO
		case INPUT_SVIDEO:	ChangeSVIDEO();		break;
#endif
#ifdef SUPPORT_COMPONENT				   
		case INPUT_COMP:	ChangeComponent();	break;
#endif
#ifdef SUPPORT_PC
		case INPUT_PC:		ChangePC();			break;
#endif
#ifdef SUPPORT_DVI
		case INPUT_DVI:		ChangeDVI();		break;
#endif
#if defined(SUPPORT_HDMI)
		case INPUT_HDMIPC:
		case INPUT_HDMITV: 	ChangeHDMI(); 		break;
#endif
#ifdef SUPPORT_BT656_LOOP
		case INPUT_BT656:	ChangeBT656Loop();	break;
#endif
#ifdef SUPPORT_LVDSRX
		case INPUT_LVDS: 	ChangeLVDSRx();	 	break;
#endif
		default:
			ChangeCVBS();
			break;
	}
}
//-----------------------------------------------------------------------------
/**
* move to next video input
*/
void	InputModeNext( void )
{
	BYTE next_input;

#if defined(SUPPORT_HDMI)
	if(InputMain==INPUT_HDMIPC)
		next_input = InputMain + 2;
	else
#endif
	next_input = InputMain + 1;

	do {
		if(next_input == INPUT_TOTAL)
			next_input = INPUT_CVBS;	
#ifndef SUPPORT_CVBS
		if(next_input==INPUT_CVBS)		next_input++;	
#endif
#ifndef SUPPORT_SVIDEO
		if(next_input==INPUT_SVIDEO)	next_input++;	
#endif
#ifndef SUPPORT_COMPONENT
		if(next_input==INPUT_COMP)		next_input++;	
#endif
#ifndef SUPPORT_PC
		if(next_input==INPUT_PC)		next_input++;	
#endif
#ifndef SUPPORT_DVI
		if(next_input==INPUT_DVI)		next_input++;	
#endif
#if defined(SUPPORT_HDMI)
		if(next_input==INPUT_HDMIPC)
			next_input+=2;	
		else if(next_input==INPUT_HDMITV)
			next_input++;
#endif
#ifndef SUPPORT_BT656_LOOP
		if(next_input==INPUT_COMP)		next_input++;	
#endif
#if defined(SUPPORT_HDMI)
		if(next_input==INPUT_HDMIPC) {
			if(GetHdmiModeEE())  next_input = INPUT_HDMITV;
		}
#endif
#ifndef SUPPORT_LVDSRX
		if(next_input==INPUT_LVDS)
			next_input++;	
#endif
	} while(next_input==INPUT_TOTAL);

	ChangeInput(next_input);
}


//=============================================================================
// Input Control routine
//=============================================================================

//extern CODE BYTE DataInitNTSC[];


//-----------------------------------------------------------------------------
/**
* prepare video input register after FW download the default init values.
*
*	select input path
*	turnoff freerun manual & turnon freerun auto.
*	assign default freerun Htotal&Vtotal
*
* @see I2CDeviceInitialize
*/
//extern void TEMP_init_BT656(void);
		
void InitInputAsDefault(void)
{
	//---------------------------------
	//step1:
	//Before FW starts the ChangeInput, 
	//		link ISR & turnoff signal interrupt & NoSignal task,
	//		turn off LCD.
	FOsdIndexMsgPrint(FOSD_STR5_INPUTMAIN);		//prepare InputMain string

	LinkCheckAndSetInput();						//link CheckAndSetInput
	Interrupt_enableVideoDetect(OFF);			//turnoff Video Signal Interrupt
	TaskNoSignal_setCmd(TASK_CMD_DONE);			//turnoff NoSignal Task
	LedBackLight(OFF);							//turnoff LedBackLight

	//---------------------------------
	//step2:
	//recover default value
	//	Download the recover register values.
	//	set sspll
	//	select MCU/SPI Clock
	Init8836AsDefault(InputMain,0);

	//---------------------------------
	//step3:
	//	InputSource=>InMux=>Decoder=>aRGB=>BT656=>DTV=>Scaler=>Measure
	//-------------------

	//InputSource  (InMux)
	switch(InputMain) {
	case INPUT_CVBS:
	case INPUT_SVIDEO:
		InputSetSource(INPUT_PATH_DECODER,INPUT_FORMAT_YCBCR);
		break;
	case INPUT_COMP:
		InputSetSource(INPUT_PATH_VADC,INPUT_FORMAT_YCBCR);		
		break;
	case INPUT_PC:
		InputSetSource(INPUT_PATH_VADC,INPUT_FORMAT_RGB);		
		break;
	case INPUT_DVI:
		InputSetSource(INPUT_PATH_DTV,INPUT_FORMAT_RGB);		
		break;
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
		InputSetSource(INPUT_PATH_DTV,INPUT_FORMAT_RGB);		
		break;
	case INPUT_BT656:
		InputSetSource(INPUT_PATH_BT656,INPUT_FORMAT_YCBCR);	 
		break;
	case INPUT_LVDS:
		InputSetSource(INPUT_PATH_LVDS,INPUT_FORMAT_RGB);	 
		break;
	}

	//Analog Mux
	AMuxSetInput(InputMain);

	//Decoder freerun	
	DecoderFreerun(DECODER_FREERUN_AUTO);

	//aRGB(VAdc)
	aRGB_SetDefaultFor();

	//BT656 Output
#ifdef SUPPORT_UART1
	BT656EncOutputEnable(OFF,0);
#elif defined(SUPPORT_RCD)
	//enable only when you selectINPUT_BT656.
	//you have to select other input to activate RCD.
	if(InputMain==INPUT_BT656)
		BT656EncOutputEnable(ON,0);
	else
		BT656EncOutputEnable(OFF,0);
	BT656_A_Output(0 /*BT656_A_OUT_DEC_I*/,0,0); //BK150417
#else
	BT656EncOutputEnable(ON,0);
	BT656_A_Output(0 /*BT656_A_OUT_DEC_I*/,0,0); //BK141216
#endif

	//DTV BT656Dec Freerun & clock
	BT656DecSetFreerun(OFF);
	switch(InputMain) {
	case INPUT_CVBS:
	case INPUT_SVIDEO:
	case INPUT_COMP:
	case INPUT_PC:
		break;
	case INPUT_DVI:
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
	case INPUT_LVDS:
		Bt656DecSetClkPol(OFF); //normal_clk
		break;
	case INPUT_BT656:
		Bt656DecSetClkPol(ON); //invert_clk
		break;
	}

	//DTV
	switch(InputMain) {
	case INPUT_CVBS:
	case INPUT_SVIDEO:
	case INPUT_COMP:
	case INPUT_PC:
		break;
#ifdef SUPPORT_DVI
	case INPUT_DVI:
		DtvSetDelay(1/*clock*/,4/*vSync*/);

		DtvSetFieldDetectionRegion(ON,0x11);	// set Det field by WIN
		DtvSetSyncPolarity(0,0);
		DtvSetRouteFormat(DTV_ROUTE_YPbPr,DTV_FORMAT_RGB565);
		break;
#endif
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
#if defined(SUPPORT_HDMI)
#ifdef SUPPORT_HDMI_24BIT
		DtvSetRouteFormat(DTV_ROUTE_BGR,DTV_FORMAT_RGB); //RGB24.
#else
		DtvSetRouteFormat(DTV_ROUTE_565_MSB_B_LSB_R_REVERSED,DTV_FORMAT_RGB565);
#endif
		//BK121213. TW8836 EVB10 has reversed ORDER..
		DtvSetReverseBusOrder(1);

		DtvSetDelay(1/*clock*/,4/*vSync*/);	 /* clock 0:better, 1:worse on 480i,576i*/

		DtvSetFieldDetectionRegion(ON,0x11);	// set Det field by WIN
#endif	//..SUPPORT_HDMI
		break;

	case INPUT_LVDS:
#if defined(SUPPORT_LVDSRX)
		/* LVDS-Rx chip (SN65LVDS93A) uses 24 input pins */
		DtvSetRouteFormat(DTV_ROUTE_BGR,DTV_FORMAT_RGB); //RGB24.

		//BK121213. TW8836 EVB10 has reversed ORDER..
		DtvSetReverseBusOrder(1);

		DtvSetDelay(1/*clock*/,4/*vSync*/);		 

		DtvSetFieldDetectionRegion(ON,0x11);	// set Det field by WIN
#endif	//..SUPPORT_LVDSRX
		break;

	case INPUT_BT656:
#ifdef SUPPORT_BT656_LOOP
		DtvSetRouteFormat(DTV_ROUTE_PbYPr,DTV_FORMAT_INTERLACED_ITU656);
		DtvSetReverseBusOrder(1);
#endif
		break;

	}

	//LVDSRx
	switch(InputMain) {
	case INPUT_CVBS:
	case INPUT_SVIDEO:
	case INPUT_COMP:
	case INPUT_PC:
	case INPUT_DVI:
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
		EnableExtLvdsTxChip(OFF);	//GPIO EXPANDER IO[4]
		LvdsRxEnable(OFF);
		LvdsRxPowerDown(ON);
		break;
	case INPUT_LVDS:
		EnableExtLvdsTxChip(ON);	//GPIO EXPANDER IO[4]
		InitLvdsRx();
		break;
	}

	//scaler
	ScalerSetFreerunAutoManual(ON,ON);
	ScalerWriteFreerunTotal(FREERUN_DEFAULT_HTOTAL, FREERUN_DEFAULT_VTOTAL);


	//measure
	MeasSetWindow( 0, 0, 0xfff, 0xfff );//set dummy window. 1600x600
	WriteTW88(REG508, 0x08 );			// field:Both. Note:DO not turn on the start
	WriteTW88(REG50B, 0x40 );			// Threshold active detection 

	switch(InputMain) {
	case INPUT_CVBS:
	case INPUT_SVIDEO:
	case INPUT_COMP:
	case INPUT_PC:
		MeasEnableDeMeasure(OFF);		// Disable DE Measure
		break;
	case INPUT_DVI:
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
	case INPUT_LVDS:
	case INPUT_BT656:
		MeasEnableDeMeasure(ON);		//Enable DE Measure
		MeasSetErrTolerance(4);			//tolerance set to 32
		MeasEnableChangedDetection(ON);	//set EN. Changed Detection
		break;
	}

	//image effect
	SetImage(InputMain);	//set saved image effect(contrast,....)

	//---------------------------
	//BT656 Output
	//---------------------------

}


//-----------------------------------------------------------------------------
/**
* enable video Output 
*
* call when CheckAndSet[Input] is successed.
*/
extern BYTE SW_Video_Status;
void VInput_enableOutput(BYTE fRecheck)
{
	Printf("\n\rVInput_enableOutput(%bd)",fRecheck);

	if(fRecheck) {
		//dPrintf("====Found Recheck:%d",VH_Loss_Changed);
		// do not turn on here. We need a retry.
	}
	else {
		ScalerSetFreerunAutoManual(ON,OFF);
		ScalerSetMuteAutoManual(ON,OFF);
		ScalerSetFreerunValue();		//calculate freerun value

		SpiOsdSetDeValue();
		FOsdSetDeValue();
		LedBackLight(ON);				//TurnOn Display
	}
	TaskNoSignal_setCmd(TASK_CMD_DONE);
	
	Interrupt_enableVideoDetect(ON);

	//BK130204. enable sync on HDMI. if it is a PC mode, we need a sync.
	if(InputMain == INPUT_DVI
	//|| InputMain == INPUT_HDMIPC
	//|| InputMain == INPUT_HDMITV
	|| InputMain == INPUT_BT656
	//|| InputMain == INPUT_LVDS 
	) {	
		//digital input.
		; //SKIP
	}
	else
		Interrupt_enableSyncDetect(ON);

	//BK130102
	if(InputMain == INPUT_LVDS )	
		SW_Video_Status = 1;

#ifdef PANEL_AUO_B133EW01
	//FW does not have overwrite, so I need it.
	AdjustSSPLL_with_HTotal();
#endif

}


//-----------------------------------------------------------------------------
/**
* goto Freerun move
*
* call when CheckAndSet[Input] is failed.
* oldname: VInputGotoFreerun
* input
*	reason
*		0: No Signal
*		1: No STD
*		2: Out of range
*/
void VInput_gotoFreerun(BYTE reason)
{
	//dPrintf("\n\rVInput_gotoFreerun(%bd)",reason);
	//Freerun
	if(InputMain == INPUT_BT656) {
	}
	else {
		DecoderFreerun(DECODER_FREERUN_60HZ);
	}

	ScalerCheckAndSetFreerunManual();

	if(InputMain == INPUT_HDMIPC 
	|| InputMain == INPUT_HDMITV
	|| InputMain == INPUT_BT656
	|| InputMain == INPUT_LVDS) {
	}
	else {
		ScalerSetMuteManual( ON );
	}
	// Prepare NoSignal Task...
	if(reason==0 && MenuGetLevel()==0) { //0:NoSignal 1:NO STD,...
		if(g_access) {
			FOsdSetDeValue();
			FOsdIndexMsgPrint(FOSD_STR2_NOSIGNAL);
			tic_task = 0;

			TaskNoSignal_setCmd(TASK_CMD_WAIT_VIDEO);
		}
	}

	if(InputMain == INPUT_PC) {
		//BK111019. I need a default RGB_START,RGB_vDE value for position menu.
		RGB_hStart = InputGetHStart();
		RGB_vDE = ScalerReadVDEReg();
	}

	LedBackLight(ON);

	Interrupt_enableVideoDetect(ON);
}

#if defined(SUPPORT_FAST_INPUT_TOGGLE)

//TW8836 has a limited XDATA memory, 2048Bytes.
//to reduce XDATA memory, Firwmware uses a REG_BUFF_INFO structure and a real XDATA buffer.
BYTE Fast_CVBS_Toggle_Buff[11];
BYTE Fast_DTV_Toggle_Buff[11];
CODE struct REG_BUFF_INFO_s Fast_VIDEO_Toggle_info[] = {
	{REG040,   	0},
	{REG041,	1},
	{REG205,	2},	//Y-Scale
	{REG206,	3},	
	{REG215,	4}, //OutputVDE
	{REG20C,	5}, //BuffLen
	{REG20E,	6},
	{REG203,	7},	//X-Scaleup
	{REG204,	8},
	{REG209,	9},	//X-ScaleDown
	{REG20A,	10},
	{0x000,     0}	   //EndOfArray..
};


/*
example
	to HDMI
	WriteTW88Buff2Reg(Fast_VIDEO_Toggle_info,Fast_DTV_Toggle_Buff);
	to CVBS
	WriteTW88Buff2Reg(Fast_VIDEO_Toggle_info,Fast_CVBS_Toggle_Buff);

NOTE::    DO NOT USE SPIOSD.
    If you need SPIOSD, turn off SPIOSD, execute it, wait, and then turn on SPIOSD
    for example
        turn off SPIOSD
        ChangeFastInputMain();
        WaitVBlank(3)
        turn on SPIOSD.
*/
void ChangeFastInputMain(void)
{
	BYTE r201,r21C,r21E;
	BYTE *buff;

	r201 = ReadTW88(REG201);
	r21C = ReadTW88(REG21C);
	r21E = ReadTW88(REG21E);

	if(InputMain==INPUT_CVBS)
		buff=Fast_CVBS_Toggle_Buff;
	else
		buff=Fast_DTV_Toggle_Buff;

	WriteTW88(REG21E, r21E | 0x01);	//OnMUTE
	WaitVBlank(1);
	WriteTW88(REG201, r201 | 0x04);	//FixedHTotal
	WriteTW88(REG21C, r21C | 0x01);	//FixedVTotal

	WriteTW88Buff2Reg(Fast_VIDEO_Toggle_info,buff);

	WriteTW88(REG201, r201);		//Remove FixedVTotal
    WriteTW88(REG21C, r21C);		//Remove FixedHTotal
	WriteTW88(REG21E, r21E);		//OffMute
 }
 #endif
