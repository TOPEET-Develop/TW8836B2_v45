/**
 * @file
 * DTV.c 
 * @author Brian Kang
 * @version 1.2
 * @section LICENSE
 *	Copyright (C) 2011~2014 Intersil Corporation
 * @section DESCRIPTION
 *	DTV for DVI,HDMI,BT656, and LVDS.
 *
 * history
 *  120803	add MeasStartMeasure before FW use a measured value.
 *			update Freerun Htotal,VTotal at VBlank.
 *			add checkroutine for HDMI detect flag.
 *	141010	Support ActiveLow Horizontal/Vertical Sync.
 *			Minimize HDMI external chip routines. 
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

#include "CPU.h"
#include "Printf.h"
#include "Monitor.h"

#include "I2C.h"

#include "main.h"
#include "Scaler.h"
#include "InputCtrl.h"
#include "eeprom.h"
#include "Decoder.h"
#include "aRGB.h"
#include "dtv.h"
#include "measure.h"
#include "PC_modes.h"


#ifdef SUPPORT_HDMI_TW8837
#include "HDMI_TW8837.h"
#endif
#ifdef SUPPORT_HDMI_EP907M
#include "HDMI_EP907M.h"
#include "EP907M_RegDef.h"
#endif

#include "Settings.h"

#include "util.h"
#include "FOsd.h"
#include "SOsdMenu.h"


#ifdef DEBUG_DTV
	#define dtv_dPrintf		dPrintf
	#define dtv_dPuts		dPuts	
#else
	#define argb_dPrintf	nullFn
	#define argb_dPuts		nullFn	
#endif


#if !defined(SUPPORT_DVI) && !defined(SUPPORT_HDMI)
//----------------------------
/**
* Trick for Bank Code Segment
*/
//----------------------------
CODE BYTE DUMMY_DTV_CODE;
void Dummy_DTV_func(void)
{
	BYTE temp;
	temp = DUMMY_DTV_CODE;
}
#endif


/**
*
* REG040[2] Enable 2nd DTVCLK
*/

						

#if defined(SUPPORT_DVI) || defined(SUPPORT_HDMI)
/**
* set DTV Sync Polarity
* @param	hPol	1:ActiveLow
* @param	vPol	1:ActiveLow
*
*	register
*	R050[2]	hPol	1:AvtiveLow(Invert)
*	R050[1]	vPol	1:ActiveLow(Invert)
*
*	This register does not work on Measure. 
*   Only works with scaler.
*
*/
void DtvSetSyncPolarity(BYTE hPol, BYTE vPol)
{
	BYTE value;

	value = ReadTW88(REG050) & ~0x06;
	if(hPol)	value |= 0x04;	//H Active Low		
	if(vPol)	value |= 0x02;	//V Active Low		
	WriteTW88(REG050, value);	
}
#endif



/**
* set DTV Color bus order
*
*	register
*	R051[3]	 1: Reverse data line order on each 8bit bus.
*/
void DtvSetReverseBusOrder(BYTE fReverse)
{
	BYTE value;

	value = ReadTW88(REG051) & 0xF7;
	if(fReverse)	value |=  0x08;		

	WriteTW88(REG051, value);		
}

#if defined(SUPPORT_DVI) || defined(SUPPORT_BT656_LOOP) || defined(SUPPORT_HDMI)
/**
* set DTV DataRouting and InputFormat
*
*	register
*	R052[2:0]	DataRouting:100 Y(R):Pb(G):Pr(B)
*	R053[3:0]	InputFormat:1000=RGB565
*
* @param 	route:	Data bus routing selection for DTV.
* @param	format
*	0: Interlaced ITU656		i656  	
*	1: Progressive ITU656		p656
*	2: 8bit 601					422 8bit
*	3: 16bit 601				422 16bit
*	4: 24bit 601				444 24bit
*	5: 16/18/24 bit RGB			RGB 24bit
*	6: ITU1120					BT1120					
*	7: SMP 720P					SMPTE296M
*	8: RGB565					RGB565
*
*
* Data bus routing
* ================
* 
* For 24 bit YPbPr					 For 24bit RGB
* ----------------					 ----------------
* 	DTVD	DTVD	DTVD				DTVD	DTVD	DTVD
*     [23:16]	[15:8]	[7:0]		    [23:16]	[15:8]	[7:0]	
* 0:	Pr		Y		Pb			 0:	R		G		B
* 1:	Pr		Pb		Y			 1:	R		B		G
* 2:	Pb		Y		Pr			 2:	B		G		R
* 3:	Pb		Pr		Y			 3:	B		R		G
* 4:	Y		Pb		Pr			 4:	G		B		R
* 5:	Y		Pr		Pb			 5:	G		R		B
*									 
*
* For 16bit RGB565					 For 16 bit YPbPr
* ----------------					 ----------------
* 	DTVD	DTVD	DTVD			 	DTVD	DTVD	DTVD
*     [23:16]	[15:8]	[7:0]		     [23:16]	[15:8]	[7:0]	
* 0:	LSB				MSB			 0:			MSB		LSB
* 1:	LSB		MSB					 1:			LSB		MSB
* 2:	MSB				LSB			 2:	LSB		MSB		
* 3:	MSB		LSB					 3:	LSB				MSB
* 4:			MSB		LSB			 4:	MSB		LSB		
* 5:			LSB		MSB			 5:	MSB				PbLSB
* 
* 
* For 8 bit YPbPr
* ----------------
* 	DTVD	DTVD	DTVD
*     [23:16]	[15:8]	[7:0]	
* 0:	data	
* 1:	data
* 2:					data
* 3:			data
* 4:					data
* 5:			data
* 
* 8 bit data order
* ----------------
* DTV_DODR is set, DE_POL is ignored.	
* (??HW captures signal at DE edge.)
* Pb or Pr always comes frist depend on DTV_CR601.
* 
* DTV_DODR	DTV_CR601	Data Order
* R051[5]		R052[3]
*    1		  	0		Pb-Y-Pr-Y			
*    1          1       Pr-Y-Pb-Y
* 
* DTV_DODR is reset,  DE_POL makes affect on the Data Order.
* (??HW captures signal at HSync)
* If DE_POL is set, Y comes first.
* 
* DTV_DODR	DE_POL		DTV_CR601	Data Order
* R051[5]	R050[3]		R052[3]
*    0		 0			0			Pb-Y-Pr-Y			
*    0       0			1         	Pr-Y-Pb-Y
*    0		 1			0			Y-Pb-Y-Pr			
*    0       1			1         	Y-Pr-Y-Pb
* 
* The BT656 uses Cb-Y-Cr-Y order.	 
* 
*        +-+-+-+-+----------
*  Y	 |0|1|2|3|
*        +-+-+-+-+----------
*  Cr	 | 0 | 1 |
*        +---+---+-----------
*  Cb    | 0 | 1 |	
*        +---+---+-----------
* 
* data order  Cb0 Y0 Cr0 Y1, Cb1 Y2
* So, choose R051[5]=1, R052[3]=0.
* 
* 
* Data Bus Bit Order
* ===================
* REG051[3] 1:Reverse 0:Normal.
* 

=======<<Below Example is incorrect....Need to redefine....>>===========
* example. For 16bit RGB565
* ----------------
* 	DTVD	DTVD	DTVD
*    [23:16]	[15:8]	[7:0]	
* 1:	LSB		MSB		
* 
* 
* If input source order is reversed, select Route Order "1" and set REG051[3].
* 
* INPUT	   			Connect		On Reverse.
* ===========		======		========
* Dinput03(B3)		DTVD23		08	[LSB]
* Dinput04(B4)		DTVD22		09
* Dinput05(B5)		DTVD21		10
* Dinput06(B6)		DTVD20		11
* Dinput07(B7)		DTVD19		12
* .
* Dinput10(G2)		DTVD18		13
* Dinput11(G3)		DTVD17		14
* Dinput12(G4)		DTVD16		15

* Dinput13(G5)		DTVD15		16
* Dinput14(G6)		DTVD14		17
* Dinput15(G7)		DTVD13		18
* .
* Dinput19(R3)		DTVD12		19
* Dinput20(R4)		DTVD11		20
* Dinput21(R5)		DTVD10		21
* Dinput22(R6)		DTVD09		22
* Dinput23(R7)		DTVD08		23	[MSB]
* 
* 
*
* If input order is normal, select Route Order "3".
* For 16bit RGB565
* ----------------
* 	DTVD	DTVD	DTVD
*     [23:16]	[15:8]	[7:0]	
* 3:	MSB		LSB		
* 
* INPUT	TW8836		
* 		======
* R7		DTV23  [MSB]		
* R6		DTV22		
* R5		DTV21		
* R4		DTV20		
* R3		DTV19		
* 		
* G7		DTV18		
* G6		DTV17		
* G5		DTV16		
* G4		DTV15		
* G3		DTV14		
* G2		DTV13		
* 		
* B7		DTV12		
* B6		DTV11		
* B5		DTV10		
* B4		DTV09		
* B3		DTV08  [LSB]		
*
*/
void DtvSetRouteFormat(BYTE route, BYTE format)
{
	WriteTW88(REG052, (ReadTW88(REG052) & 0xF8) | route);
	WriteTW88(REG053, (ReadTW88(REG053) & 0xF0) | format);
}
#endif

#if defined(SUPPORT_DVI) || defined(SUPPORT_HDMI)
/**
* set DTV Field Detection Register.
* looking for ODD field.
* param
*	register
*	R054[7:4]	End Location
*	R054[3:0]	Start Location
* example:
*	DtvSetFieldDetectionRegion(ON, (end<<4) | start)
*
* 	480i 		384 < 492 < 512		vs_lead
*	1080i@50 	960 < 1320 < 1408 	vs_lead
*	1080i@60	960 < 1100 < 1408 	vs_lead
*
* To verify, 
*	select ODD; REG508[3:2].
*	repeat below two steps
*		start measure.
*		check vPeriod(REG522:REG523) and VS_Position(REG52C:REG52D);
*			vPeriod will have big value, and VS_Position will have (vTotal/2). 	
*/
void DtvSetFieldDetectionRegion(BYTE fOn, BYTE r054)
{
	if(fOn) {
		WriteTW88(REG050, ReadTW88(REG050) | 0xA0 );	// set Det field by WIN. use vs_lead.
		WriteTW88(REG054, r054 );						// set window
	}
	else {
		WriteTW88(REG050, ReadTW88(REG050) & ~0x80 );	// use VSync/HSync Pulse
	}
}
struct DTV_FIELD_WIN_s {
	WORD start;
	WORD end;
};
code struct DTV_FIELD_WIN_s DtvFieldWin[] = {
/*0*/ {32,64},		/*1*/ {64,128},		/*2*/ {128,256},	/*3*/ {192,384},
/*4*/ {256,512},	/*5*/ {320,640},	/*6*/ {384,768},	/*7*/ {448,896},
/*8*/ {512,1024},	/*9*/ {576,1152},	/*A*/ {640,1280},	/*B*/ {704,1408},
/*C*/ {768,1536},	/*D*/ {832,1664},	/*E*/ {896,1792},	/*F*/ {960,1920}
};
BYTE DtvFindFieldWindow(WORD base)
{
	BYTE i;
	BYTE start,end;
	struct DTV_FIELD_WIN_s *p;

	//search Start
	p = &DtvFieldWin[15];
	i=15;
	while(i) {
		if(p->start < base)
			break;
		p--;
		i--;
	}
	start = i;
	//search End
	p = DtvFieldWin;
	for(i=0; i<16; i++) {
		if(p->end > base)
			break;
		p++;
	}
	end = i;

	Printf("\nDtvFindFieldWindow(%d) %d~%d", base, DtvFieldWin[start].start, DtvFieldWin[end].end); 
	return ((end << 4) | start);
}

#endif



#if defined(SUPPORT_DVI)  || defined(SUPPORT_HDMI)

/**
* set DTV delay
* register
*	R051[2:0]	Input clock DTVCLK delay time selection.
*	R056[7:0]	Input Vsync delay,  applicable to DTV only (one input hsync per increment)
*/
void DtvSetDelay(BYTE clock, BYTE vSync)
{
	WriteTW88(REG051, (ReadTW88(REG051) & 0xF8) | clock);
	WriteTW88(REG056, vSync);
}

#endif


//=============================================================================
// DVI
//=============================================================================

#if defined(SUPPORT_DVI)
//-----------------------------------------------------------------------------
//		void 	DVISetInputCrop( void )
//-----------------------------------------------------------------------------
/**
* set InputCrop for DVI
*
* extern
*	MeasHPulse ->Removed
*	MeasVPulse ->Removed
*	MeasVStart
*/
static void DVISetInputCrop( void )
{
	BYTE	offset, VPulse, HPulse;
	WORD	hstart, vstart, vtotal, hActive;
	BYTE HPol, VPol;
	WORD Meas_HPulse,Meas_VPulse;

	Meas_HPulse = MeasGetHSyncRiseToFallWidth();
	Meas_VPulse = MeasGetVSyncRiseToFallWidth();
	vtotal = MeasGetVPeriod();
	hActive = MeasGetHActive( &hstart );

#ifdef DEBUG_DTV
	dPuts("\n\rMeas");
	dPrintf("\n\r\tH           Pulse:%4d BPorch:%3d Active:%4d hAvtive:%d",Meas_HPulse,hstart,MeasHLen,hActive);
	dPrintf("\n\r\tV Total:%4d Pulse:%4d BPorch:%3d Active:%4d",vtotal,Meas_VPulse,MeasVStart,MeasVLen);
#endif

	offset = 5;  //meas delay value:4
	//hstart = MeasHStart + offset;
	hstart += offset;
	if ( Meas_HPulse > (hActive/2) ) {
		if(hActive > Meas_HPulse)
			HPulse = hActive - Meas_HPulse;
		else
			HPulse = Meas_HPulse - hActive;
		HPol = 0;	
	}
	else  {
		HPulse = Meas_HPulse;
		HPol = 1;
		hstart -= HPulse;	// correct position
	}

	if ( Meas_VPulse > (vtotal/2) ) {
		VPulse = vtotal - Meas_VPulse;
		VPol = 0;
	}
	else  {
		VPulse = Meas_VPulse;
		VPol = 1;
	}
	vstart = MeasVStart + VPulse;

	DtvSetSyncPolarity(HPol,VPol);

#ifdef DEBUG_DTV
	dPuts("\n\rmodified");
	dPrintf("\n\r\tH           Pulse:%2bd BPorch:%3d Active:%4d Pol:%bd hActive:%4d ",HPulse,hstart,MeasHLen,HPol, hActive);
	dPrintf("\n\r\tV Total:%4d Pulse:%2bd BPorch:%3d Active:%4d Pol:%bd",vtotal,VPulse,vstart,MeasVLen,VPol);
#endif
	//BKFYI. The calculated method have to use "InputSetCrop(hstart, vstart, MeasHLen, MeasVLen);"
	//		 But, we using a big VLen value to extend the vblank area.
	InputSetCrop(hstart, 1, MeasHLen, 0x7fe);
}

/**
* set Output for DVI
*/
static void DVISetOutput( void )
{
	BYTE	HDE;
	WORD temp16;

	ScalerSetHScale(MeasHLen);
	ScalerSetVScale(MeasVLen);

	//=============HDE=====================
	HDE = ScalerCalcHDE();
#ifdef DEBUG_DTV
	dPrintf("\n\r\tH-DE start = %bd", HDE);
#endif
	ScalerWriteHDEReg(HDE);


	//=============VDE=====================
	// 	MeasVStart ??R536:R537
	//	MeasVPulse ??R52A,R52B
	temp16 = ScalerCalcVDE();
#ifdef DEBUG_DTV
	dPrintf("\n\r\tV-DE start = %d", temp16);
#endif
	ScalerWriteVDEReg((BYTE)temp16);
		   //BKTODO131011: use void ScalerSet_vDE_value(BYTE vStart)

	//FYI.
	//ScalerSetFreerun will be updated at VInput_enableOutput().
}

/**
* Check and Set DVI
*
* extern
*	MeasVStart,MeasVLen,MeasHLen	
* @return 
*	-0:ERR_SUCCESS
*	-1:ERR_FAIL
*/
BYTE CheckAndSetDVI( void )
{
	WORD	Meas_HStart;
	WORD	MeasVLenDebug, MeasHLenDebug;
	WORD    MeasVStartDebug, MeasHStartDebug;
//	struct DIGIT_VIDEO_TIME_TABLE_s *pTimeTable;
//	struct SCALER_TIME_TABLE_s *pScaler;

	//DtvSetSyncPolarity(0,0);

	do {														
		Meas_StartMeasure();
		if(Meas_IsMeasureDone(50)) {
			return ERR_FAIL;
		}
		MeasVLen = MeasGetVActive( &MeasVStart );				//v_active_start v_active_perios
		MeasHLen = MeasGetHActive( &Meas_HStart );				//h_active_start h_active_perios
#ifdef DEBUG_DTV
		dPrintf("\n\rDVI Measure Value: %dx%d HS:%d VS:%d",MeasHLen,MeasVLen, Meas_HStart, MeasVStart);
		dPrintf("==>Htotal:%d",  MeasGetVsyncRisePos());
#endif

		DVISetInputCrop();
		DVISetOutput();

		MeasVLenDebug = MeasGetVActive( &MeasVStartDebug );		//v_active_start v_active_perios
		MeasHLenDebug = MeasGetHActive( &MeasHStartDebug );		//h_active_start h_active_perios

	} while (( MeasVLenDebug != MeasVLen ) || ( MeasHLenDebug != MeasHLen )) ;

	//AdjustPixelClk_TEST(need htotal);
	AdjustPixelClk(0, 0);	//NOTE:it uses DVI_Divider.


#ifdef SUPPORT_SCALER_OVERWRITE_TABLE
//	pScaler = FindScalerTable(InputMain, pTimeTable->hActive,pTimeTable->vActive,pTimeTable->vFreq,1);
//	if(pScaler != NULL)
//		OverWriteScalerWithTable(pScaler,1,1);
#endif

	//prepare info
	DVI_PrepareInfoString(MeasHLen,MeasVLen,MeasGetVFreq() /*0*/ /*freq*/);

	//for debug. check the measure value again
	CheckMeasure();

	return ERR_SUCCESS;
}

//=============================================================================
// Change to DVI
//=============================================================================

//-----------------------------------------------------------------------------
/**
* Change to DVI
*
* linked with SIL151
* @return
*	- 0: success
*	- 1: No Update happen
*	- 2: No Signal or unknown video sidnal.
*/
BYTE	ChangeDVI( void )
{
	BYTE ret;

	if ( InputMain == INPUT_DVI ) {
		//dPrintf("\n\rSkip ChangeDVI");
		return(1);
	}

	InputMain = INPUT_DVI;

	if(GetInputMainEE() != InputMain)
		SaveInputMainEE( InputMain );

	//----------------
	// initialize video input
	InitInputAsDefault();

	//
	// Check and Set VADC,mesaure,Scaler for Analog PC input
	//
	ret = CheckAndSetDVI();		//same as CheckAndSetInput()
	if(ret==0) {
		//success
		VInput_enableOutput(0);
		return 0;
	}

	//------------------
	// NO SIGNAL
	// Prepare NoSignal Task...
	VInput_gotoFreerun(0);

	//dPrintf("\n\rChangeDVI--END");
	return(2);
}
#endif

//=============================================================================
// HDMI
//=============================================================================

#if defined(SUPPORT_HDMI)
/**
* Set Scaler Output for HDMI
*/
static void	HDMISetOutput(WORD HActive, WORD VActive, BYTE	vDE )
{
	BYTE	hDE;
	WORD VScale;
	DWORD dTemp;

	ScalerSetVScale(VActive);
	ScalerSetHScale(HActive);

	//=============HDE=====================
	hDE = ScalerCalcHDE();
#ifdef DEBUG_DTV
	dPrintf("\n\r\tH-DE start = %bd", hDE);
#endif


	//=============VDE=====================

	VScale = ScalerReadVScaleReg();

//#ifdef DEBUG_DTV
//	dPrintf("\n\r\tV-DE start = %bd", vDE);
//#endif
	//pal need to add 0.5. The real value is 23.5.
	dTemp = vDE;
	dTemp <<= 1;
	if(VActive==288) //is pal
		dTemp+=1;
	dTemp *= 8192L;
	dTemp /= VScale;
	dTemp >>= 1;
	vDE = dTemp;

//#ifdef DEBUG_DTV
//	dPrintf("=> %bd", vDE);
//#endif

	//---------------------UPDATE-----------------------------

	ScalerWriteHDEReg(hDE);
	ScalerWriteVDEReg(vDE);

	//BKTODO: aspect will overwrite it. let me solve later.
	if(HActive <= PANEL_H)
		ScalerSetLineBufferSize(HActive+1);
	else
		ScalerSetLineBufferSize(PANEL_H+1);
}

//CheckAndSetHDMI and CheckAndSetLVDS use a same routines to detect the HDMI chip output.
//let's merge it.
/*
	DEBUG_DTV__TEST.

	EP907M time register and TW8836 meas value have a same result.
	So, if some assumeption is correct, we can reduce the code size.

				pol		Sync	BPorch		ScalerStart
		1080p	h:0 	44(44)	148(189)	192+1(189+4)
				v:0		5(5)	36(42)		41+2(42+1)
		1080i	h:0		44(44)	148(188)	192+1(188+4+1)
				v:0		5(5)	15(21)		20+2(21+1)
		 720p	h:0		40(40)	220(256)	260+1(256+4+1)
		 		v:0		5(5)	20(26)		25+2(26+1)
		480p	h:0		62(62)	60(118)		122+1(118+4)
				v:0		6(6)	30(37)		36+2(37+1)
		480i	h:0		124(62)	114(115)	(124+114+1)/2(115+4)
				v:0		3(3)	15(19)		18+2(19+1)		
		note:() is for meas value.	

		Timing Register value and the measued value have a same result.
		We can ignore one of them.
*/
BYTE CheckHdmiChipRegister(void)
{
	BYTE ret;

#ifdef DEBUG_DTV__TEST
	BYTE TempByte[8];
	WORD hFPorch,vFPorch;
	WORD hBPorch,vBPorch;
#endif

	WORD hActive,vActive;

#if defined(SUPPORT_HDMI_EP907M)
#else
	WORD Old_hActive, Old_vActive;
	BYTE i;
#endif

	BYTE vFreq;
		
	WORD hCropStart, vCropStart;
	BYTE bTemp;
	struct DIGIT_VIDEO_TIME_TABLE_s *pCEA861;
	struct SCALER_TIME_TABLE_s *pScaler;

	WORD hSync, vSync;
	BYTE hPol, vPol;

#if defined(SUPPORT_HDMI_EP907M)
	/*
	If you can control the external device, 
	*/
	ret = HdmiCheckConnection();
	if (ret)
		return ret;

	ret = HdmiCheckMode();	//HDMI or DVI

	//If we using AUTO_VFMTb==0, we don't need it.
	//
	//set color space.
	if (ret)
	{
		ret = CheckAviInfoFrame();
	}
	else
	{
		//BKTODO:We need default color space value. RGB,YUV422,YUV420
		//BK130204. We need a EEPROM value.
		HdmiSetColorSpace(0x00); //clear
	}

	HdmiDebugTimingValue();	

	Meas_StartMeasure();
	ret = Meas_IsMeasureDone(50);
	if (ret)
	{
		/*if measure fail, it measn no signal.*/

		Printf(" meas=> NoSignal");
		return ERR_FAIL;
	}
#else //..defined(SUPPORT_HDMI_EP907M)
	//If you cannot control the external device, 
	//use the measure method.

	//If we don't know HDMI RX chip, we have to wait until DTV has a stable image.
	//I will wait total 3 sec.
	//MeasStartMeasure() use 500ms.
	for (i = 0; i < 6; i++)
	{
		Meas_StartMeasure();
		ret = Meas_IsMeasureDone(50);
		if (ret == 0)
			break;
	}

	if (ret)
	{
		/*if measure fail, it measn no signal...*/
		Printf(" meas=> NoSignal");
		return ERR_FAIL;
	}
	
	/*wait until it has a stable value.*/
	Old_hActive = MeasGetHActive( &hCropStart );
	Old_vActive = MeasGetVActive( &vCropStart );
	for (i = 0; i < 10; i++)
	{
		delay1ms(10);
		Meas_StartMeasure();
		ret=Meas_IsMeasureDone(50);
		hActive = MeasGetHActive( &hCropStart );
		vActive = MeasGetVActive( &vCropStart );
		if (Old_hActive==hActive && Old_vActive==vActive)
			break;
		Old_hActive = hActive;
		Old_vActive = vActive;
	}
#endif
	//=================================
	// Read Measured data
	//=================================
	hSync = MeasGetHSyncRiseToFallWidth();
	vSync = MeasGetVSyncRiseToFallWidth();

//	hTotal = MeasGetVsyncRisePos();
//	vTotal = MeasGetVPeriod();  //BK130204, vtotal is better. We can use a meas vCropStart. 
	hActive = MeasGetHActive( &hCropStart );
	vActive = MeasGetVActive( &vCropStart );
	vFreq = MeasGetVFreq();
	PrintMeasValue("DTV");
	if (vFreq == 59)
		vFreq = 60;

	/*check sync polarity */
	if ( hSync > (hActive/2) )	hPol = 0;	//active low. something wrong.
	else						hPol = 1;	//active high.(Low signal on the Active Video Area)
	if ( vSync > (vActive/2) )	vPol = 0;	//active low. something wrong.
	else						vPol = 1;	//active high.(Low signal on the Active Video Area)
	//if(hPol==0 || vPol==0)
	//	Printf("\n\rWarning:: hPol:%bx vPol:%bx",hPol,vPol);
#if 0


#endif



	/* search video table */
	pCEA861 = Find_CEA861_VideoTable(hActive,vActive,vFreq);
#ifdef SUPPORT_DEONLY_DTV
	if(ReadTW88(REG050) & 0x10) {
		Puts("\n!!!DE only!!");
		if(pCEA861)
			PrintCEAVideoTimeTable(pCEA861);
		pCEA861 = NULL;
	}
#endif
	if(pCEA861) {
		PrintCEAVideoTimeTable(pCEA861);
		//use table	value
		hActive =    pCEA861->hActive;
		hCropStart = pCEA861->hBPorch;
		if(hPol) 
			hCropStart += pCEA861->hSync;
		vActive = 	 pCEA861->vActive;
		vCropStart = pCEA861->vBPorch;
		if(vPol) 
			vCropStart += pCEA861->vSync;
		PrintMeasAdjValue("DTV_TBL",hCropStart,vCropStart); 
	}
	else {
		//---------------------
		//DTV measure adjust. hStart+4, vStart-1.
		//---------------------
		hCropStart += 4;
		vCropStart -= 1;
		PrintMeasAdjValue("DTV",hCropStart,vCropStart); 
	}
	//if source is 720x240, EP907M reports it as 1440x240 on power up.
	//I assume, 720x288, also same.
	if(hActive==1440) {
		if(vActive==240 || vActive==288) {
			hActive >>= 1;
			hCropStart >>=1;
			Printf("\n=>hActive:%d =>hCropStart:%d",hActive, hCropStart);
		}
	}

	//
	//set DTV hPol and vPol polarity.
	//FW uses the measured hActiveStart+4 & vActiveStart+1.
	//so FW donot use Dtv polarity.
	DtvSetSyncPolarity(0 ,0);	//base ActiveHigh		

	// Interlaced Field detection.
	if(pCEA861) {
		if(pCEA861->pol & INTERLACED) {
			bTemp = DtvFindFieldWindow(pCEA861->hTotal / 2);
			DtvSetFieldDetectionRegion(ON, bTemp);
			Meas_StartMeasure();
			Meas_IsMeasureDone(50);
			PrintMeasValue("DTV");
		}
	}

	/* set input crop */ 
	InputSetCrop(hCropStart-1, vCropStart-1, hActive+2, vActive+2);
	HDMISetOutput( hActive, vActive,  vCropStart );
	//scaler_set_output(hActive, vActive, vCropStart, 0);

	/* search scaler table. If success, overwrite */
#ifdef SUPPORT_SCALER_OVERWRITE_TABLE
	pScaler = FindScalerTable(InputMain, hActive,vActive,vFreq, vCropStart,vPol);
#ifdef SUPPORT_DEONLY_DTV
	if(ReadTW88(REG050) & 0x10) {
		pScaler = NULL;
	}
#endif
	if(pScaler != NULL)
		OverWriteScalerWithTable(pScaler,hPol,vPol);
#endif
	

#if 0 //def SUPPORT_DEONLY_DTV
	if(hActive==720) {
		if(vActive==240 || vActive==288) {
			if(ReadTW88(REG004) & 0x01) {
				WriteTW88(REG004, ReadTW88(REG004) | 0x01);
			}
		}
	}
#endif

	return ERR_SUCCESS;
}

/**
* Check and Set HDMI
*
* Hot Boot: Reset only TW8835.
*		Hot boot needs a EP9351 Software Reset, but FW does not support it anymore.
*		Please, use a Reset button or Power Switch.
*/
BYTE CheckAndSetHDMI(void)
{
	dPuts("\n\rCheckAndSetHDMI START");

	//return CheckAndSetDtvSignal();
	return CheckHdmiChipRegister();
}
#endif 

//=============================================================================
// Change to HDMI
//=============================================================================

//-----------------------------------------------------------------------------
/**
* Change to HDMI
*
* linked with EP9553E.
* @return
*	- 0: success
*	- 1: No Update happen
*	- 2: No Signal or unknown video sidnal.
*/
extern void Interrupt_enableVideoDetect(BYTE fOn);

#if defined(SUPPORT_FAST_INPUT_TOGGLE)
XDATA REG_IDX_DATA_t Fast_Hdmi_Buff[] = {
	{REG040, 0x12},
	{REG041, 0x51},
	
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

BYTE ChangeHDMI(void)
{
	BYTE ret;

#if defined(SUPPORT_FAST_INPUT_TOGGLE)
	if(g_hdmi_checked) {
		InputMain = INPUT_HDMIPC;

		LinkCheckAndSetInput();						//link CheckAndSetInput
		Interrupt_enableVideoDetect(OFF);			//turnoff Video Signal Interrupt
		TaskNoSignal_setCmd(TASK_CMD_DONE);			//turnoff NoSignal Task

		ChangeFastInputMain();

		//
		//check vdloss.
		//
		Meas_StartMeasure();
		ret=Meas_IsMeasureDone(50);
		if(ret==0) {
			WORD hTotal, vTotal;
			ScalerCalcFreerunValue(&hTotal,&vTotal);
			ScalerWriteFreerunTotal(hTotal,vTotal);				

			//success
			TaskNoSignal_setCmd(TASK_CMD_DONE);	
			Interrupt_enableVideoDetect(ON);
			return 0;

		}
		//If measure was fail, we have to use the normal routines...
	}
#endif

	if ( InputMain == INPUT_HDMIPC || InputMain == INPUT_HDMITV ) {
		//dPrintf("\n\rSkip ChangeHDMI");
		return(1);
	}

	if(GetHdmiModeEE())  InputMain = INPUT_HDMITV;
	else 				 InputMain = INPUT_HDMIPC;

	if(GetInputMainEE() != InputMain)
		SaveInputMainEE( InputMain );

	//dPrintf("\n\rChangeHDMI InputMain:%02bx",InputMain);

	//----------------
	// initialize video input
	InitInputAsDefault();


#ifdef SUPPORT_HDMI_EP907M
	HdmiInitEp907MChip();
#endif

#ifdef SUPPORT_DEONLY_DTV
	WriteTW88(REG050, ReadTW88(REG050) | 0x10);
#endif
#if 0 //def SUPPORT_DEONLY_DTV
	WriteTW88(REG050, ReadTW88(REG050) &  ~0x80);
#endif

	//
	// Check and Set 
	//
#if defined(SUPPORT_HDMI)
	ret = CheckAndSetHDMI();
#else
	ret=ERR_FAIL;
#endif
	if(ret==ERR_SUCCESS) {
		//success
		VInput_enableOutput(0);
#if defined(SUPPORT_FAST_INPUT_TOGGLE)
		ReadTW88Reg2Buff(Fast_VIDEO_Toggle_info,Fast_DTV_Toggle_Buff);
		g_hdmi_checked = 1;
#endif
		return 0;
	}

	//------------------
	// NO SIGNAL
	// Prepare NoSignal Task...
	VInput_gotoFreerun(0);
	//dPrintf("\n\rChangeHDMI--END");
#if defined(SUPPORT_FAST_INPUT_TOGGLE)
	g_hdmi_checked = 0;
#endif
	return(2);
}


//=============================================================================
// Change to LVDSRx
//=============================================================================
#ifdef SUPPORT_LVDSRX

/**
* TW8836 EVB1.0 uses a HDMI output for LVDS Tx chip input.
*
* video path
* HDMI=>LVDS Tx chip =>TW8836 LVDSRx => Scaler => Panel
*/
BYTE CheckAndSetLVDSRx(void)
{
	dPuts("\n\rCheckAndSetLVDS START");
	return CheckAndSetDtvSignal();
}

BYTE ChangeLVDSRx(void)
{
	BYTE ret;

	if ( InputMain == INPUT_LVDS ) {
		//dPrintf("\n\rSkip ChangeLVDSRx");
		return(1);
	}
	InputMain = INPUT_LVDS;

	if(GetInputMainEE() != InputMain)
		SaveInputMainEE( InputMain );

	//----------------
	// initialize video input
	InitInputAsDefault();

	//
	// Check and Set Input crop, scale rate, scaler output time.
	//
#ifdef SUPPORT_LVDSRX
	ret = CheckAndSetLVDSRx();		
 	if(ret==0) {
		//success
		VInput_enableOutput(0);
		return 0;
	}
#endif
	//------------------
	// NO SIGNAL
	// Prepare NoSignal Task...

	VInput_gotoFreerun(0);
	return(2);
}
#endif





//=============================================================================
// Change to BT656
//=============================================================================
#ifdef SUPPORT_BT656_LOOP


//-----------------------------------------------------------------------------
/**
* Check and Set BT656
*
* @return
*	- 0: success
*	- other: error
* @todo REG04A[0] does not work.
*
* video path
* CVBS=>BT656Out=>DTV i656=>Scaler=>Panel.
*/
BYTE CheckAndSetBT656Loop(void)
{
	BYTE	mode;
	BYTE hDelay,vDelay;
	DWORD dTemp;
	BYTE bTemp;
	WORD MeasVStart, MeasHStart;
	WORD MeasHLen, MeasVLen;
	struct DEC_VIDEO_TIME_TABLE_s *pVideoTable;

	//dPrintf("\n\rCheckAndSetBT656Loop start.");

	//
	//check decoder
	//
	if ( DecoderCheckVDLOSS(100) ) {
		return 1; //no decoder signal
	}
	mode = DecoderCheckSTD(100);
	if ( mode == 0x80 ) {
	    ePrintf("\n\r NoSTD");
		return( 2 );
	}
	mode >>= 4;
	InputSubMode = mode;
	if(mode >= 7)
		return 3;

	pVideoTable = &TW8836_DEC_TABLE[mode];

	//
	//init decoder input crop.
	//
	if(mode==0 || mode==3 || mode==4 || mode ==6) {
		//NTSC.
		//Decoder & BT656Loopback using different value.
		//Decoder using	 8,720,21,240.
		//BT656 will use 13,720,21,240.
		hDelay = 13;   vDelay = 21;

	}
	else if(mode==1 || mode==2 || mode==5) {
		// PAL.
		//Decoder & BT656Loopback using different value.
		//Decoder using	 6,720,23,288.
		//BT656 will use 11,720,22,288
		hDelay = 11;   vDelay = 22;

	}
	else {
		Printf("\n\rUnknown mode;%bx",mode);
		return 3;
	}


	DecoderSetOutputCrop(hDelay,pVideoTable->hActive,vDelay,pVideoTable->vActive);
	//PrintDecoderOutputCrop("BT656Loop",);
	//
	//set External BT656 Encoder
	//
	if(mode==0 || mode==3 || mode==4 || mode ==6)
		BT656_InitExtEncoder(BT656_8BIT_525I_YCBCR_TO_CVBS);
	else
		BT656_InitExtEncoder(BT656_8BIT_625I_YCBCR_TO_CVBS);
	
	//
	//init scaler inputcrop,scale rate, scale output time.	
	//
	Meas_StartMeasure();
	if(Meas_IsMeasureDone(50)) {
		//it is comes from decoder. let's use a measure.
		//if meas success, please remove this routine...
		ScalerSetLineBufferSize(pVideoTable->hActive);	//BK120116	- temp location. pls remove
	
		ScalerSetHScale(pVideoTable->hActive);
		ScalerSetVScale(pVideoTable->vActive);
		dTemp = vDelay;
		dTemp *= PANEL_V;
		dTemp /= pVideoTable->vActive;

		//BK130129. If NTSC, use 38(it was 42). so reduce 4;
		if(mode==0)
			dTemp -=4;

		ScalerWriteVDEReg((WORD)dTemp);
//BKTODO131011: use void ScalerSet_vDE_value(BYTE vStart)
		return 0;
	}

	MeasVLen = MeasGetVActive( &MeasVStart );				//v_active_start v_active_perios
	MeasHLen = MeasGetHActive( &MeasHStart );				//h_active_start h_active_perios
	//dPrintf("\n\rBT656 Measure Value: %dx%d hStart:%d+4 vStart:%d-1",MeasHLen,MeasVLen, MeasHStart, MeasVStart);
	//dPrintf("==>Htotal:%d",  MeasGetVsyncRisePos());
	InputSetCrop(MeasHStart+4,MeasVStart+1, MeasHLen,MeasVLen);	 //add 2 more for scaler on H value.
#ifdef DEBUG_BT656
	PrintScalerInputCrop("BT656Loop",MeasHStart+4,MeasVStart+1, MeasHLen,MeasVLen); 
#endif

	//implicit DE.
	bTemp = ReadTW88(REG041);
	bTemp &= ~0x01;		//YUV;
	bTemp |= 0x10;		//Implicit DE
	WriteTW88(REG041,bTemp);

	ScalerSetLineBufferSize(MeasHLen / 2);
	ScalerSetHScale(MeasHLen / 2);
	ScalerSetVScale(MeasVLen);
	ScalerSetVScale(pVideoTable->vActive);

	dTemp = MeasVStart + 1;
	dTemp *= PANEL_V;
	dTemp /= MeasVLen;
	if(mode==1)	// is pal
		dTemp += 6;
	if(PANEL_V==600)
		dTemp -= 1;	 
	ScalerWriteVDEReg((WORD)dTemp);

//BKTODO131011: use void ScalerSet_vDE_value(BYTE vStart)

	return 0;
}

//-----------------------------------------------------------------------------
/**
* Change to BT656Loop
*
* VideoPath
*	CVBS=>BT656 Out=>VD[] bus=>DTV(i656)=>Scaler=>Panel.
*
* @return
*	- 0: success
*	- 1: No Update happen
*	- 2: No Signal or unknown video sidnal.
*/
BYTE ChangeBT656Loop(void)
{
	BYTE ret;

	if ( InputMain == INPUT_BT656 ) {
		//dPrintf("\n\rSkip ChangeBT656");
		return(1);
	}
	InputMain = INPUT_BT656;

	if(GetInputMainEE() != InputMain)
		SaveInputMainEE( InputMain );

	//----------------
	// initialize video input
	InitInputAsDefault();

	delay1ms(350);	//decoder need a delay.

	//
	// Check and Set DEC,mesaure,Scaler for BT656 Loopback input
	//
	ret = CheckAndSetBT656Loop();
 	if(ret==0) {
		//success
		VInput_enableOutput(0);
		return 0;
	}

	//------------------
	// NO SIGNAL
	// Prepare NoSignal Task...

	VInput_gotoFreerun(0);

	return(2);
}
#endif
/*
 *
 */
#if defined(SUPPORT_LVDSRX)
BYTE CheckAndSetDtvSignal(void)
{
	BYTE ret;
	BYTE i;
	BYTE vFreq;
	WORD hActive,vActive;
	WORD hStart,vStart;

	WORD hPeriod,vPeriod;
	WORD hFPorch,vFPorch;
	WORD hSync,vSync;
	WORD hBPorch,vBPorch;
		
	struct DIGIT_VIDEO_TIME_TABLE_s *pCEA861;
	struct SCALER_TIME_TABLE_s *pScaler;

	BYTE hPol,vPol;
	WORD wTemp0,wTemp1;
	DWORD dTemp,pclk;

	/*If someone changed meas, recover it*/
	MeasSetWindow( 0, 0, 0xfff, 0xfff );//set dummy window. 1600x600
	WriteTW88(REG50A, ReadTW88(REG50A) | 0x01);	//Enable DE measure
	WriteTW88(REG508, ReadTW88(REG508) & ~0x10);

	/* check video signal */
	for(i=0; i < 5; i++) {
		Meas_StartMeasure();
		ret=Meas_IsMeasureDone(50);
		if(ret==0)
			break;
		delay1ms(10);
		Puts("."); //I am alive...
	}
	if(ret) {
		/*if measure fail, it measn no signal...*/
		Printf(" meas=> NoSignal");
		return ERR_FAIL;
	}
	/* wait until DTV has a stable image. */
	wTemp0 = MeasGetHActive( &hStart );
	wTemp1 = MeasGetVActive( &vStart );
	for(i=0; i < 20; i++) {
		delay1ms(10);
		Meas_StartMeasure();
		ret=Meas_IsMeasureDone(50);
		hActive = MeasGetHActive( &hStart );
		vActive = MeasGetVActive( &vStart );
		if(wTemp0==hActive && wTemp1==vActive)
			break;
		wTemp0 = hActive;
		wTemp1 = vActive;
		Puts("*"); //I am alive...
	}
	/* read measured registers */
	hSync = MeasGetHSyncRiseToFallWidth();
	vSync = MeasGetVSyncRiseToFallWidth();
	hPeriod = MeasGetVsyncRisePos();
	vPeriod = MeasGetVPeriod();  
	vFreq = MeasGetVFreq();
	PrintMeasValue("DTV");
	//---------------------
	//DTV measure adjust. hStart+4, vStart-1.
	//If pol is active high, hStart is Sync+BPorch
	//If pol ia active low,  hStart is BPorch
	//---------------------
	hStart += 4;
	vStart -= 1;
	PrintMeasAdjValue("DTV",hStart,vStart); 

	/*Find CEA861 or VESA table*/
	pCEA861 = Find_CEA861_VideoTable(hActive,vActive,vFreq);
	if(pCEA861) {
		PrintCEAVideoTimeTable(pCEA861);
		/* If meas has a differnet value, use CEA861 value.
	     for example
			if(hActive==1151)	hActive = 1152;
			if(hActive==1279)	hActive = 1280;
			if(hActive==1439)	hActive = 1440;
			if(hActive==1599)	hActive = 1600;
			if(hActive==1919)	hActive = 1920;
		*/
		hActive = pCEA861->hActive;
	}

	/*check Sync polarity.*/
	/*Firmware likes the active high polarity.*/
	if ( hSync > hActive )	hPol = 0;	//active low.  Negative
	else					hPol = 1;	//active high. Positive
	if ( vSync > vActive )	vPol = 0;	//active low.  Negative
	else					vPol = 1;	//active high. Positive

	/*set DTV hPol and vPol polarity.*/
	//These start values are BPorch or Sync+BPorch that depend on the polarity.
	//so FW do not use Dtv polarity.
	DtvSetSyncPolarity(0,0);

	/*set field window for Interlaced*/
	if(pCEA861) {
		if(pCEA861->pol & INTERLACED) {
			i = DtvFindFieldWindow(pCEA861->hTotal / 2);
			DtvSetFieldDetectionRegion(ON, i);
			Meas_StartMeasure();
			Meas_IsMeasureDone(50);
			PrintMeasValue("DTV");
		}
	}

	/*Find vFPorch,vSync,vBPorch.*/
	if(vPol==0) {
		vSync = vPeriod - MeasGetVSyncRiseToFallWidth();
		vBPorch = vStart;
		vFPorch = vPeriod - vSync - vBPorch - vActive;
	}
	else {
		vSync = MeasGetVSyncRiseToFallWidth();
		vBPorch = vStart - vSync;
		vFPorch = vPeriod - vSync - vBPorch - vActive;
	}
	/*Find hFPorch,hSync,hBPorch and hPeriod.*/
	if(hPol==0) {
		hBPorch = hStart;
		hFPorch = MeasGetHSyncRiseToFallWidth() - MeasGetHSyncRiseToHActiveEnd();
		WriteTW88(REG508, ReadTW88(REG508) | 0x10);
		Meas_StartMeasure();
		Meas_IsMeasureDone(50);
		hPeriod = MeasGetHSyncRiseToFallWidth();
		hSync = hPeriod - hFPorch - hBPorch - hActive;
		
		//BKTODO141009 remove it
		WriteTW88(REG508, ReadTW88(REG508) & ~0x10);
		Meas_StartMeasure();
		Meas_IsMeasureDone(50);
	}
	else {
		hSync = MeasGetHSyncRiseToFallWidth();
		hBPorch = hStart - hSync;
		hPeriod = MeasGetVsyncRisePos();    
		hFPorch = hPeriod - hBPorch - hSync - hActive;
	}
	/*Find an input pixel clock.*/
	//	InputPixelClock = hPeriod * pclk / MeasGetHPeriodReg();
	wTemp0 = MeasGetHPeriodReg();
	dTemp = Sspll2GetFreq();
	pclk   = PclkGetFreq(dTemp);
	pclk /= 1000;
	dTemp = hPeriod;
	dTemp *= pclk;
	dTemp /= wTemp0;

	//print out the result
	Printf("\n%dx%d@%bd", hActive,vActive,vFreq);
	Printf(" hStart:%d vStart:%d", hStart, vStart);

	Printf(" %s", hPol ? "HP" : "HN");
	Printf("_%s", vPol ? "VP" : "VN");

	Printf(" %d,%d,%d,%d",
		hPeriod, hFPorch, hSync, hBPorch);
	Printf(" %d,%d,%d,%d",
		vPeriod, vFPorch, vSync, vBPorch);
	Printf("\n pclk:%ld SSPLL:%ld", dTemp,pclk);

	//if (( pclk / InputPixelClock) > 2), use 2 for hStart crop value. 
	wTemp1= wTemp0 / hPeriod;
	if(wTemp1 > 2) 	i = 2;
	else 			i = 1;
	Printf(" hOffset:%bd",i);	

	//If input has a pixel repeat, some HDMI Rx chip send a double horizontal values.
	if(hActive==1440) {
		if(vActive==240 || vActive==288) {
			hActive >>= 1;
			hStart >>=1;
			Printf("\n=>hActive:%d =>hCropStart:%d",hActive, hStart);
		}
	}

	//-------------------------------------------
	// input crop
	InputSetCrop(hStart-i, vStart-vPol, hActive+2, vActive+2);
	HDMISetOutput( hActive, vActive,  vStart );

	//Find the scaler table and then overwrite.
	//because,...
#ifdef SUPPORT_SCALER_OVERWRITE_TABLE
	pScaler = FindScalerTable(InputMain, hActive,vActive,vFreq, vStart,vPol);
	if(pScaler != NULL)
		OverWriteScalerWithTable(pScaler, hPol, vPol);
#endif

	return ERR_SUCCESS;
}
#endif


//=====================================NEW CODE==============================
#if 0
/**
* It will read HDMI chip to get AVI.
*/
BYTE ChangeHDMI(void)
{
}
BYTE CheckAndSetHdmi(void)
{
#if defined(SUPPORT_HDMI_EP907M)
	/*
	If you can control the external device, 
	*/
	ret = HdmiCheckConnection();
	if(ret)
		return ret;
	ret = HdmiCheckMode();	//HDMI or DVI

	//If we using AUTO_VFMTb==0, we don't need it.
	//
	//set color space.
	if(ret) {
		ret = CheckAviInfoFrame();
	}
	else {
		//BKTODO:We need default color space value. RGB,YUV422,YUV420
		//BK130204. We need a EEPROM value.
		HdmiSetColorSpace(0x00); //clear
	}

	HdmiDebugTimingValue();	

	Meas_StartMeasure();
	ret=Meas_IsMeasureDone(50);
	if(ret) {
		/*if measure fail, it measn no signal.*/

		Printf(" meas=> NoSignal");
		return ERR_FAIL;
	}	
#endif
}

/**
* On this mode, I am using only measure block.
* I want to add DE only option...
*/
BYTE ChangeDTV(void)
{
}
BYTE CheckAndSetDTV(void)
{
	/* If you cannot control the external device, use the measure method.*/

	/* wait until you get a video signal */
	/* max wait is 3 sec. */
	/* If video comes later, ISR will take care */
	for(i=0; i < 6; i++) {
		Meas_StartMeasure();
		ret=Meas_IsMeasureDone(50);	/*500ms */
		if(ret==0)
			break;
	}
	if(ret) {
		/*if measure fail, it measn no signal...*/
		Printf(" meas=> NoSignal");
		return ERR_FAIL;
	}
	/*wait until it has a stable value.*/
	Old_hActive = MeasGetHActive( &hCropStart );
	Old_vActive = MeasGetVActive( &vCropStart );
	for(i=0; i < 10; i++) {
		delay1ms(10);
		Meas_StartMeasure();
		ret=Meas_IsMeasureDone(50);
		hActive = MeasGetHActive( &hCropStart );
		vActive = MeasGetVActive( &vCropStart );
		if(Old_hActive==hActive && Old_vActive==vActive)
			break;
		Old_hActive = hActive;
		Old_vActive = vActive;
	}

	/*=================================
	 Read Measured data
	=================================*/
	hSync = MeasGetHSyncRiseToFallWidth();
	vSync = MeasGetVSyncRiseToFallWidth();

//	hTotal = MeasGetVsyncRisePos();
//	vTotal = MeasGetVPeriod();  //BK130204, vtotal is better. We can use a meas vCropStart. 
	hActive = MeasGetHActive(&hCropStart);
	vActive = MeasGetVActive(&vCropStart);
	vFreq = MeasGetVFreq();
	PrintMeasValue("DTV");
	if(vFreq==59)
		vFreq=60;

	/* check sync polarity */
	if ( hSync > (hActive/2) )	hPol = 0;	//active low. something wrong.
	else						hPol = 1;	//active high.(Low signal on the Active Video Area)
	if ( vSync > (vActive/2) )	vPol = 0;	//active low. something wrong.
	else						vPol = 1;	//active high.(Low signal on the Active Video Area)
	//if(hPol==0 || vPol==0)
	//	Printf("\n\rWarning:: hPol:%bx vPol:%bx",hPol,vPol);

	/* search video table */
	pCEA861 = Find_CEA861_VideoTable(hActive,vActive,vFreq);
	if(pCEA861 == NULL) {
		/* No standard */
		/* adjust measured data. hStart+4, vStart-1 */
		hCropStart += 4;
		vCropStart -= 1;
	}
	else {
		/* use table value */
		PrintCEAVideoTimeTable(pCEA861);
		hActive =    pCEA861->hActive;
		hCropStart = pCEA861->hBPorch;
		if(hPol) 
			hCropStart += pCEA861->hSync;
		vActive = 	 pCEA861->vActive;
		vCropStart = pCEA861->vBPorch;
		if(vPol) 
			vCropStart += pCEA861->vSync;
		PrintMeasAdjValue("DTV_TBL",hCropStart,vCropStart); 
	}
	/* adjust double pixeled video */
	/* If source is 720x480i or 720x288i, some video chip makes a double pixels */
	if(hActive==1440) {
		if(vActive==240 || vActive==288) {
			hActive >>= 1;
			hCropStart >>=1;
			Printf("\n=>hActive:%d =>hCropStart:%d",hActive, hCropStart);
		}
	}

	/* FW not using DTV polarity registers */	
	DtvSetSyncPolarity(0 ,0);	//base ActiveHigh		
	
	/* Field detection for interlaced video */
	if(pCEA861 && (pCEA861->pol & INTERLACED)) {
		bTemp = DtvFindFieldWindow(pCEA861->hTotal / 2);
		DtvSetFieldDetectionRegion(ON, bTemp);
		Meas_StartMeasure();
		Meas_IsMeasureDone(50);
		PrintMeasValue("DTV");
	}

	/* set scaler input crop */ 
	InputSetCrop(hCropStart-1, vCropStart-1, hActive+2, vActive+2);

	/* set scaler output */
	DtvSetScalerOutput( hActive, vActive,  vCropStart );

	/* adjust pclk */
	xdown = ReadTW88(REG20A) & 0x0F;
	xdown <<=4;
	xdown |= ReadTW88(REG209);
	if(xdown > 400) {
		/* SSPLL2 = PCLK = PCLKO *2 */
	}
	else {
		/* SSPLL2 = PCLK = PCLKO *1 */
	}


	/* search scaler table. 
	   If success, overwrite.
	   If failed, just use a calculated values
	 */
#ifdef SUPPORT_SCALER_OVERWRITE_TABLE
	pScaler = FindScalerTable(InputMain, hActive,vActive,vFreq, vCropStart,vPol);
	if(pScaler != NULL)
		OverWriteScalerWithTable(pScaler,hPol,vPol);
#endif
	
	return ERR_SUCCESS;
}

static void	DtvSetScalerOutput(WORD hActive, WORD vActive, BYTE	vDE )
{
	BYTE	hDE;
	WORD vScale;
	DWORD dTemp;

	ScalerSetVScale(vActive);
	ScalerSetHScale(hActive);

	hDE = ScalerCalcHDE();
	ScalerWriteHDEReg(hDE);

	vScale = ScalerReadVScaleReg();
	dTemp = vDE;
	dTemp <<= 1;
	if(vActive==288) /*if pal, add 0.5 */
		dTemp+=1;
	dTemp <<= 13; /* *8192L */
	dTemp /= vScale;
	dTemp >>= 1;
	vDE = dTemp;
	ScalerWriteVDEReg(vDE);

	if(hActive <= PANEL_H)
		ScalerSetLineBufferSize(hActive+1);
	else
		ScalerSetLineBufferSize(PANEL_H+1);
}

#endif



