/**
 * @file
 * DECODER.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	Internal Decoder module 
 ******************************************************************************
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
#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"
#include "Global.h"

#include "main.h"
#include "Printf.h"
#include "Monitor.h"
#include "I2C.h"
#include "CPU.h"
#include "Scaler.h"
#include "InputCtrl.h"
#include "util.h"

#include "EEPROM.h"
#include "Decoder.h"

#include "FOsd.h"

//-----------------------------------------------------------------------------
/*
	Decoder Signal.

NTSC
ITU-R BT.470-7
			Total  	Active	Blank		ScanLine
			Samples
NTSC		858		720		138			525@60Hz
PAL			864		720		144			625@50Hz

*/
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
/**
* check Video Loss
*
* @param n: wait counter
* @return
*	0:Video detected
*	1:Video not present. Video Loss
*
* register
*	R101[0]
*/ 
BYTE DecoderCheckVDLOSS( BYTE n )
{
	volatile BYTE	mode;
	BYTE start;

#ifdef DEBUG_DEC
	dPrintf("\n\rDecoderCheckVDLOSS(%d) start",(WORD)n);
#endif
	start = n;

	while (n--) {
		mode = ReadTW88(REG101);		//read Chip Status
		if (( mode & 0x80 ) == 0 ) {
#ifdef DEBUG_DEC
			dPrintf("->end%bd",start - n);
#endif
			return ( 0 );				//check video detect flag
		}
		delay1ms(10);
	}
#ifdef DEBUG_DEC
	ePrintf("\n\rDecoderCheckVDLOSS->fail");
#endif
	return ( 1 );						//fail. We loss the Video
}


//-----------------------------------------------------------------------------
/**
* desc 
* 	set output crop for Decoder
*
*
* register
*	vDelay		R107[7:6]R108[7:0]
*	vActive		R107[5:4]R109[7:0]
*	hDelay		R107[3:2]R10A[7:0]
*	hActive		R107[1:0]R10B[7:0]
*
*		hDelay hActive	vDelay vActive
* NTSC	8		720		21		240
* PAL	6		720		23		288	
*
*/
void DecoderSetOutputCrop(WORD hDelay, WORD hActive, WORD vDelay, WORD vActive)
{
	BYTE bTemp;
	bTemp  = vDelay >> 8;	bTemp <<= 2;
	bTemp |= vActive >> 8;	bTemp <<= 2;
	bTemp |= hDelay >> 8;	bTemp <<= 2;
	bTemp |= hActive >> 8;
	WriteTW88(REG107, bTemp);
	WriteTW88(REG108, (BYTE)vDelay);
	WriteTW88(REG109, (BYTE)vActive);
	WriteTW88(REG10A, (BYTE)hDelay);
	WriteTW88(REG10B, (BYTE)hActive);
}


/* read decoder outputcrop value */
#ifdef DEBUG_SCALER_OVERWRITE_TABLE
WORD DecoderGet_vDelay(void)
{
	WORD wTemp;
	wTemp = ReadTW88(REG107) & 0xC0;
	wTemp <<= 2;
	wTemp |= ReadTW88(REG108);
	return wTemp;
}
WORD DecoderGet_vActive(void)
{
	WORD wTemp;
	wTemp = ReadTW88(REG107) & 0x30;
	wTemp <<= 4;
	wTemp |= ReadTW88(REG109);
	return wTemp;
}
WORD DecoderGet_hDelay(void)
{
	WORD wTemp;
	wTemp = ReadTW88(REG107) & 0x0C;
	wTemp <<= 6;
	wTemp |= ReadTW88(REG10A);
	return wTemp;
}
WORD DecoderGet_hActive(void)
{
	WORD wTemp;
	wTemp = ReadTW88(REG107) & 0x03;
	wTemp <<= 8;
	wTemp |= ReadTW88(REG10B);
	return wTemp;
}
#endif

//-----------------------------------------------------------------------------
/**
* read detected decoder mode
*
*	register
*	R11C[7]		0:idle, 1:detection in progress
*	R11C[6:4]	000: NTSC
*				001: PAL
*				...
*				111:N/A
*/
BYTE DecoderReadDetectedMode(void)
{
	BYTE mode;
	mode = ReadTW88(REG11C);
	mode >>= 4;
	return mode;
}

//-----------------------------------------------------------------------------
/**
* check detected decoder video input standard
*
*	To get a stable the correct REG11C[6:4] value,
*		read REG101[6] and REG130[7:5] also.
*	I saw the following values(BK110303)
* 		E7 E7 67 67 87 87 87 87 ..... 87 87 87 87 87 87 87 87 87 07 07 07 .... 
* 		B7 B7 B7 37 37 87 87 87 ..... 87 87 87 87 87 87 87 87 87 07 07 07 07 07 07 07
*
* oldname: CheckDecoderSTD
*
* register
*	R11C[6:4].
* 	R101[6].
*	R130[7:5].
* @return
*	0x80: filed.
*	other: detected standard value.
*/
BYTE DecoderCheckSTD( BYTE n )
{
	volatile BYTE	r11c,r101,r130;
	BYTE start=n;
	BYTE count;
#ifdef DEBUG_DEC
	ePrintf("\n\rDecoderCheckSTD(%d) start",(WORD)n);
#endif
	
	count=0;
	while (n--) {
		r11c = ReadTW88(REG11C);
		if (( r11c & 0x80 ) == 0 ) {
			r101 = ReadTW88(REG101);
			r130 = ReadTW88(REG130);
#ifdef DEBUG_DEC
			dPrintf("\n\r%02bx:%02bx-%02bx-%02bx ",start-n, r11c, r101,r130);
#endif
			if((r101 & 0x40) && ((r130 & 0xE0)==0)) {
#ifdef DEBUG_DEC
				ePrintf("->success:%d",(WORD)start-n);
#endif
				if(count > 4)
					return (r11c);
				count++;
			}
 		}
		delay1ms(5);
	}
#ifdef DEBUG_DEC
	ePrintf("->fail");
#endif

	//This is only for pattern generator.
	if((r101 & 0xC1) == 0x41) //PAL ? 
		return (r11c);

	return ( 0x80 );
}

//-----------------------------------------------------------------------------
/**
* set decoder freerun mode
*
* example
*   DecoderFreerun(DECODER_FREERUN_60HZ);
*
* R133[7:6]
* @param
*	mode	0:AutoMode
*			1:AutoMode
*			2:60Hz
*			3:50Hz
*/
void DecoderFreerun(BYTE mode)
{
	WriteTW88(REG133, (ReadTW88(REG133) & 0x3F) | (mode<<6));
}

static void StrCatDecoderModeName(BYTE mode)
{
	switch(mode) {
	case 0:		TWstrcat(FOsdMsgBuff," NTSC");		break;
	case 1:		TWstrcat(FOsdMsgBuff," PAL");		break;
	case 2:		TWstrcat(FOsdMsgBuff," SECAM");		break;
	case 3:		TWstrcat(FOsdMsgBuff," NTSC4");		break;
	case 4:		TWstrcat(FOsdMsgBuff," PAL-M");		break;
	case 5:		TWstrcat(FOsdMsgBuff," PAL-CN");	break;
	case 6:		TWstrcat(FOsdMsgBuff," PAL-60");	break;
	default:	TWstrcat(FOsdMsgBuff," Unknown");	break;	
	}
}


//=============================================================================
// Change to DECODER. (CVBS & SVIDEO)
//=============================================================================


//-----------------------------------------------------------------------------
/**
* check and set the decoder input
*
* @return
*	0: success
*	1: VDLOSS
*	2: No Standard
*	3: Not Support Mode
*
* extern
*	InputSubMode
*
* measure result
*
* NTSC									PAL		
*	vTotal:262 vFreq:59						vTotal:313 vFreq:49.99
*	hSync:1 vSync:0							hSync:1 vSync:0
*
*	hActive	 		vActive
*	2,73			1,1	   					2,65		1,1
*	1713,1711		261,262					1725,1086	311,312
*	---------		-------					----		----
*	1712			261						1724		311
*
*	H Rise to Act End:1						1								   	
*	VS Rise Pos. in on e H:1317				1327 	
*	FIFO Read Start :0						0
*
*   1440x480i	1716,38,124,114				1440x576i	1728,24,126,138
*				262,4,3,15								312,2,3,19
*
*	DecInputCrop
*		8,720,21,240						6,720,23,288
*/
extern code struct DEC_VIDEO_TIME_TABLE_s TW8836_DEC_TABLE[];


BYTE CheckAndSetDecoderScaler( void )
{
	BYTE mode;	//0 = NTSC(M), 1 = PAL (B,D,G,H,I), ...
	struct DEC_VIDEO_TIME_TABLE_s *pVideoTable;
	struct SCALER_TIME_TABLE_s *pScaler;
	WORD hActive,vActive;
	BYTE hStart, vStart;
	BYTE fScale;

	/*check video signal */
	if ( DecoderCheckVDLOSS(100) ) {
		ePuts("\n\rCheckAndSetDecoderScaler VDLOSS");
		DecoderFreerun(DECODER_FREERUN_60HZ);
		return( 1 );
	}
	/*get standard */
	mode = DecoderCheckSTD(100);
	if ( mode == 0x80 ) {
	    ePrintf("\n\rCheckAndSetDecoderScaler NoSTD");
		DecoderFreerun(DECODER_FREERUN_60HZ);
		return( 2 );
	}
	mode >>= 4;
	InputSubMode = mode; //save sub mode.
	if(mode >= 7) {
		DecoderFreerun(DECODER_FREERUN_60HZ);
		return 3;
	}

	/* link table */
	pVideoTable = &TW8836_DEC_TABLE[mode];
	/* read scale mode. 0:overscan(default), 1:full */
	fScale = EE_Read(EEP_INPUT_DEC);

    //dPrintf("\n\rCheckAndSetDecoderScaler mode:%bd",mode);
	//dPrintf("  %dx%d", pVideoTable->hActive,pVideoTable->vActive);
	//dPrintf(" hDelay:%bd vDelay:%bd", pVideoTable->hDelay,pVideoTable->vDelay);
	//if(fScale==0)/*ANALOG_OVERSCAN*/
	//	dPrintf(" hOverScan:%bd vOverScan:%bd", pVideoTable->hOverScan,pVideoTable->vOverScan);

	hStart  = pVideoTable->hDelay;
	hActive = pVideoTable->hActive;
	vStart  = pVideoTable->vDelay;
	vActive = pVideoTable->vActive;

	if(fScale==0) {	/*ANALOG_OVERSCAN*/
		hStart  += pVideoTable->hOverScan;
		hActive -= (pVideoTable->hOverScan*2);
		vStart  += pVideoTable->vOverScan;
		vActive -= (pVideoTable->vOverScan *2);
	}
	/* set decoder output crop */
	DecoderSetOutputCrop(hStart,hActive,vStart,vActive);

	/* set scaler */
	ScalerSetLineBufferSize(hActive);
	ScalerSetHScale(hActive);
	ScalerSetVScale(vActive);
	ScalerSet_vDE_value(vStart);

#if 0  //BK150717
	scaler_set_output(hActive,vActive,vStart,0);
#endif	

	/*scaler input clock polarity */
	InputSetClockPolarity(pVideoTable->pol);

	/*black level for NTSC */
	if(mode==0)	WriteTW88(REG10C, ReadTW88(REG10C) | 0x10);
	else		WriteTW88(REG10C, ReadTW88(REG10C) & ~0x10);

	/* search scaler table. If success, overwrite */
#ifdef SUPPORT_SCALER_OVERWRITE_TABLE
	pScaler = FindScalerTable(InputMain, pVideoTable->hActive,pVideoTable->vActive,pVideoTable->vFreq, 0,1);
	if(pScaler != NULL)
		OverWriteScalerWithTable(pScaler,1,1);
#endif

	//prepare info string
	FOsdSetInputMainString2FOsdMsgBuff();									 	
	StrCatDecoderModeName(mode);
		
	return(0);
}

#if defined(SUPPORT_FAST_INPUT_TOGGLE)
XDATA REG_IDX_DATA_t Fast_Cvbs_Buff[] = {
	{REG040, 0x00},
	{REG041, 0xC0},
	
	{REG203, 0x00},
	{REG204, 0x00},
	{REG205, 0x00},
	{REG206, 0x00},
 	{REG207, 0x00},
	{REG208, 0x00},
	{REG209, 0x00},
	{REG20A, 0x00},
	{REG20B, 0x00},
	{REG20C, 0x00},
	{REG20D, 0x00},
	{REG20E, 0x00},
	{REG20F, 0x00},
	{REG210, 0x00},
	{REG211, 0x00},
	{REG212, 0x00},
	{REG213, 0x00},
	{REG214, 0x00},
	{REG215, 0x00},

	{0x000, 0x00}
};
#endif

//-----------------------------------------------------------------------------
/**
* Change to Decoder
*
* extern
*	InputMain
*	InputSubMode
* @param
*	fSVIDEO		0:CVBS, 1:SVIDEO
* @return
*	- 0: success
*	- 1: No Update happen
*	- 2: No Signal or unknown video sidnal.
*	- 3: NO STD
* @see InitInputAsDefault
* @see CheckAndSetDecoderScaler
* @see VInput_enableOutput
* @see VInput_gotoFreerun
*/
static BYTE ChangeDecoder(BYTE fSVIDEO)
{
	BYTE ret;

#if defined(SUPPORT_FAST_INPUT_TOGGLE)
	if(g_cvbs_checked) {
		InputMain = INPUT_CVBS;

		ChangeFastInputMain();

		//check vdloss.
		if(DecoderCheckVDLOSS(100) ==0) {
			WORD hTotal, vTotal;
			ScalerCalcFreerunValue(&hTotal,&vTotal);
			ScalerWriteFreerunTotal(hTotal,vTotal);				
			return 0;
		}
		//If vdloss, we have to use the normal routines...
	}
#endif

	if(fSVIDEO) {
		if ( InputMain == INPUT_SVIDEO ) {
#ifdef DEBUG_DEC
			dPrintf("\n\rSkip ChangeSVIDEO");
#endif
			return(1);
		}
		InputMain = INPUT_SVIDEO;
	}
	else {
		if ( InputMain == INPUT_CVBS ) {
#ifdef DEBUG_DEC
			dPrintf("\n\rSkip ChangeCVBS");
#endif
			return(1);
		}
		InputMain = INPUT_CVBS;
	}
	InputSubMode = 7; //clear.

	if(GetInputMainEE() != InputMain) 	
		SaveInputMainEE( InputMain );

	//----------------
	// initialize video input
	InitInputAsDefault();


	//BKFYI: We need a delay before call DecoderCheckVDLOSS() on CheckAndSetDecoderScaler()
	//But, if fRCDMode, InputMode comes from others, not CVBS, not SVIDEO. We don't need a delay. 
	delay1ms(350);

	//
	// Check and Set 
	//
	ret = CheckAndSetDecoderScaler();
	if(ret==ERR_SUCCESS) {
		//success
		VInput_enableOutput(0);
#if defined(SUPPORT_FAST_INPUT_TOGGLE)
		ReadTW88Reg2Buff(Fast_VIDEO_Toggle_info,Fast_CVBS_Toggle_Buff);
		g_cvbs_checked = 1;
#endif
		return 0;
	}
	//------------------
	// NO SIGNAL
	//------------------
	VInput_gotoFreerun(ret-1);	//1->0:NoSignal 2->1:NO STD
#if defined(SUPPORT_FAST_INPUT_TOGGLE)
	g_cvbs_checked = 0;
#endif
	return (ret+1);	 			//2:NoSignal 3:NO STD
}

//-----------------------------------------------------------------------------
/**
* Change to CVBS
*
* @return
*	- 0: success
*	- 1: No Update happen
*	- 2: No Signal or unknown video sidnal.
*	- 3: NO STD
* @see ChangeDecoder
*/
BYTE ChangeCVBS( void )
{
	return ChangeDecoder(0);
}

//-----------------------------------------------------------------------------
/**
* Change to SVIDEO
*
* @return
*	- 0: success
*	- 1: No Update happen
*	- 2: No Signal or unknown video sidnal.
*	- 3: NO STD
* @see ChangeDecoder
*/
BYTE	ChangeSVIDEO( void )
{
	return ChangeDecoder(1);
}


#ifdef SUPPORT_FOSD_MENU
//=============================================================================
// for FontOSD MENU
//=============================================================================
//-----------------------------------------------------------------------------
/**
* Is it a video Loss State
*
* @return
*	- 1:If no Input
*	- 0:Found Input
*/
BYTE DecoderIsNoInput(void)
{
	BYTE ret;
	
	ret = TW8835_R101;	
	if(ret & 0x80)
		return 1;	//No Input
	return 0;		//found Input
}
//-----------------------------------------------------------------------------
/**
* read video input standard
*
* BKTODO120201 Pls, remove this
*/
BYTE DecoderReadVInputSTD(void)
{
	BYTE std, ret;

	if( DecoderIsNoInput() ) ret = 1; // Noinput!!	BUGBUG

	std = DecoderReadDetectedMode();
	if(std & 0x08) 
		ret = 0xff;	// Detection in progress..
	else
		ret = std + 1;

	return (ret);
}
#endif




