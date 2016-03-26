/**
 * @file
 *   aRGB.c
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	Video HighSpeed ADC module for aRGB 
 *
 *  VADC means Video ADC. we also call it as aRGB.
 *  VADC consist of "SYNC Processor" + "LLPLL" + "ADC".
 *  Component & PC inputs use VADC module. 	
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
//-------------------------------------------------------------------
// global function
//	CheckAndSetComponent
//	CheckAndSetPC
//	aRGB_SetDefault
//-------------------------------------------------------------------
#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "Printf.h"
#include "Monitor.h"
#include "I2C.h"
#include "CPU.h"
#include "global.h"
#include "Scaler.h"

#include "InputCtrl.h"

#include "measure.h"
#include "PC_modes.h"
	
#include "aRGB.h"
#include "eeprom.h"
#include "settings.h"

#include "util.h"
#include "FOsd.h"

void dummy_argb_code(void)
{
	Puts("dummy_argb_code");
}

XDATA	BYTE	Input_aRGBMode;


#if defined(DEBUG_COMP) || defined(DEBUG_PC)
	#define argb_Printf		Printf
	#define argb_Puts		Puts	
	#define argb_ePrintf	ePrintf
	#define argb_ePuts		ePuts	
	#define argb_wPrintf	wPrintf
	#define argb_wPuts		wPuts	
	#define argb_dPrintf	dPrintf
	#define argb_dPuts		dPuts	
#else
	#define argb_Printf		nullFn
	#define argb_Puts		nullFn	
	#define argb_ePrintf	nullFn
	#define argb_ePuts		nullFn	
	#define argb_wPrintf	nullFn
	#define argb_wPuts		nullFn	
	#define argb_dPrintf	nullFn
	#define argb_dPuts		nullFn	
#endif


/*		

Component PATH
==============
									   +=>vSync=> VSYNCO => 		   Scaler
									   |
 SOG  => SOG Slicer  =>	SyncSeperator =+   
									   |
									   +=>hSync=> LLPLL  => HSO =>	   Scaler


PC PATH
=======

 HSYNC => LLPLL =>	HSO	  =>	 Scaler
								 Scaler
 VSYNC          =>


Module Polarity
===============					                                  
								Fixed
                module			output
				======		   polarity
			+----------+         +-+
			|	SOG	   |   ==>   | |
			|	Slicer |        -+ +-
			+----------+


   Request						Fixed
   INPUT        module          output
   polarity						polarity
   =====		=======			======
    +-+ 	  +---------+		-+ +-
    | |	  =>  |  Sync   |  =>    | |
   -+ +-      |Seperator|		 +-+
  active      +---------+      active 
    High					    Low
                             V: VSYNCO
							 H: CS_PAS


   Request						Fixed
   INPUT        module          output
   polarity						polarity
   =====		=======			======
   -+ +-	  +---------+		 +-+	 
    | |	  =>  |  LLPLL  |  =>    | |
	+-+		  +---------+		-+ +-
  active					   active 
    low						    high


   Prefer
   INPUT        module    
   =====	   =======	
    +-+   	  +---------+
    | |   =>  | measure | 
   -+ +-	  +---------+
   active  				
    high   				

*/


static BYTE WaitStableLLPLL(void);



/**
* Description
*	Set aRGB signal path.
* @param input
*	0:component, 1:PC.
*
* registers
*	REG1C0[3]
*	REG1C0[4]
*	REG1CC[4]
*	REG1CC[3:2] - always HSO.
*	REG1CC[0] - if HSO, always 0.(ByPass)
*/
#ifdef SUPPORT_ARGB
void aRGB_setSignalPath(BYTE fInputPC)
{
	BYTE bTemp;

	bTemp = ReadTW88(REG1C0);
	bTemp &= ~0x18;
	if(fInputPC)  bTemp |=  0x08;
	else		  bTemp |=  0x10;
	WriteTW88(REG1C0, bTemp);

	bTemp = ReadTW88(REG1CC);
	bTemp &= ~0x1D;
	if(fInputPC) bTemp |= 0x10;
	WriteTW88(REG1CC,bTemp);
}
#endif


/**
* Description
*	Select LLPLL input polarity.
*   This function needs a correct Path.
*	Execute aRGB_setSignalPath first.
*
* @param fInputPC
*	0:component
*	1:PC
* @param fActiveHighPol.Positive
*	if input is component, select value will be bypass,REG1C0[2]=1.
*		because, video path will be SOG_Slicer=>SyncSeperator. 
*		and hPol is active low that LLPLL requests. 
*	if input is PC, use fPol value that comes from REG1C1[6].
*		If hSync is active high (REG1C1[6] is 1), invert it. 
*		LLPLL requests an Active Low.
*
*	register REG1C0[2]
*/
#ifdef SUPPORT_ARGB
void aRGB_SetLLPLL_InputPolarity(BYTE fInputPC, BYTE fActiveHighPol)
{
	BYTE bTemp;

	bTemp = ReadTW88(REG1C0);
	if(fInputPC) {
		if(fActiveHighPol)	bTemp &= ~0x04;	 //invert.
		else				bTemp |=  0x04;  //bypass.			
	}
	else {
		bTemp |=  0x04; //bypass
	}
	WriteTW88(REG1C0, bTemp);
}
#endif

/**
* Description
* 	select vSync output polarity.
*
*	scaler & measure like a positive polarity.
*	if input is component, SyncSeperator output is active low.
*		scaler need a inverted sync input.
*	if input is PC, make the scaler input as an active high.
*/
#ifdef SUPPORT_ARGB
void aRGB_Set_vSyncOutPolarity(BYTE fInputPC, BYTE fActiveHighPol)
{
	BYTE bTemp;

	bTemp = ReadTW88(REG1CC);
	if(fInputPC) {
		if(fActiveHighPol) bTemp &= ~0x02;	//bypass
		else               bTemp |=  0x02;	//invert
	}
	else
		bTemp |= 0x02;		//invert
	WriteTW88(REG1CC,bTemp);
}
#endif

//-----------------------------------------------------------------------------
// component video table.
// Overscanned value.
//-----------------------------------------------------------------------------


extern code struct COMP_VIDEO_TIME_TABLE_s TW8836_COMP_TABLE[];


#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC) || defined(SUPPORT_BT656_LOOP)
//-----------------------------------------------------------------------------
/**
* Read aRGB(VAdc) InputStauts
*
*	register
*	R1C1	LLPLL Input Detection Register
*	R1C1[7] - VSync input polarity. 1:Active High. 
*	R1C1[6]	- HSync input polarity. 1:Active High.
*	R1C1[5]	- VSYNC pulse detection status. 1=detected
*	R1C1[4]	- HSYNC pulse detection status. 1=detected
*	R1C1[3]	- Composite Sync detection status	
*	R1C1[2:0] Input source format detection in case of composite sync.
*				0:480i	1:576i	3:480p	3:576p
*				4:1080i	5:720p	6:1080p	7:fail
*/
#ifdef SUPPORT_ARGB
BYTE aRGB_GetInputStatus(void)
{
	BYTE value;
	value = ReadTW88(REG1C1);
	return value;
}
#endif
#endif


#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC)
#define LLPLL_POST_8		0x00
#define LLPLL_POST_4		0x40
#define LLPLL_POST_2		0x80
#define LLPLL_POST_1		0xC0 //*
#define LLPLL_VCO_40TO216	0x30 //*
#define LLPLL_PUMP_5		0x02 //*
//-----------------------------------------------------------------------------
/**
* Set LLPLL Control
*
*	register
*	R1C2[7:6]	PLL post divider
*	R1C2[5:4]	VCO range select
*	R1C2[2:0]	Charge pump current
*/
void aRGB_SetLLPLLControl(BYTE value)
{
	WriteTW88(REG1C2, value);
}
#endif


//-----------------------------------------------------------------------------
// LLPLL Divider
//-----------------------------------------------------------------------------
#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
/**
* Write LLPLL divider
*
* @param	value: PLL value. Use (Htotal-1)
* @param	fInit:	init flag
*
*	register
*		R1C3[3:0]R1C4[7:0] - LLPLL Divider. PLL feedback divider. A 12-bit register 
*/
void aRGB_LLPLLSetDivider(WORD value, BYTE fInit)
{
	volatile BYTE mode;

	Write2TW88(REG1C3,REG1C4, value);
	if(fInit) {	
		WriteTW88(REG1CD, ReadTW88(REG1CD) | 0x01);		// PLL init
		//wait
		do {
			mode = TW8835_R1CD;
		} while(mode & 0x01);
	}
}
#endif

#if defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
/**
* Read LLPLL divider value
*
* other name: GetCoarse(void)
*/
WORD aRGB_LLPLLGetDivider(void)
{
	WORD value;

	Read2TW88(REG1C3,REG1C4,value);
	return value & 0x0FFF;
}
#endif

//-----------------------------------------------------------------------------
// LLPLL Clock PHASE
//-----------------------------------------------------------------------------
#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
/**
* set Phase value
*
*	register
*	R1C5[4:0]
* @param value: Phase value
* @param fInit:	init flag
*/
void aRGB_SetPhase(BYTE value, BYTE fInit)
{
	volatile BYTE mode;

	WriteTW88(REG1C5, value&0x1f);
	if(fInit) {
		WriteTW88(REG1CD, ReadTW88(REG1CD) | 0x01);	// PLL init
		//wait
		do {
			mode = TW8835_R1CD;
		} while(mode & 0x01);
	}
}
#endif

#if defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
/**
* get Phase value
*/
//-----------------------------------------------------------------------------
BYTE aRGB_GetPhase(void)
{
	return ReadTW88(REG1C5) & 0x1f;		//VADC_PHASE
}
#endif



//-----------------------------------------------------------------------------
// LLPLL Filter BandWidth
//---------------------------
//-----------------------------------------------------------------------------
/**
* set filter bandwidth
*
*	register
*	R1C6[2:0]	R1C6 default: 0x20.
*/
void aRGB_SetFilterBandwidth(BYTE value, WORD delay)
{						 
	if(delay)
		delay1ms(delay);

	WriteTW88(REG1C6, (ReadTW88(REG1C6) & 0xF8) | value);
}




//-----------------------------------------------------------------------------
#ifdef SUPPORT_COMPONENT
/**
* set clamp mode and HSync Edge
*
*	register
*	R1D4[5]
*/
void aRGB_SetClampModeHSyncEdge(BYTE fOn)
{
	BYTE bTemp;
	bTemp =  ReadTW88(REG1D4);

	if(fOn)	bTemp |=  0x20;
	else	bTemp &= ~0x20;

	WriteTW88(REG1D4, bTemp);
}
#endif

#ifdef SUPPORT_COMPONENT
//-----------------------------------------------------------------------------
/**
* set clamp position
*
*	register
*	R1D7[7:0]
*/
void aRGB_SetClampPosition(BYTE value)
{
	WriteTW88(REG1D7, value );	// ADC clamp position from HSync edge by TABLE ClampPos[]
}
#endif


//===================================================================
//
//===================================================================
//-----------------------------------------------------------------------------
/**
* set default VAdc for PC & Component.
*
* If input is not PC or Component, powerdown VAdc.
*	R1C0[]	10
*	R1C2[]	d2
*	* R1C6	20
*	R1CB[]
*	R1CC[]
*	R1D4[]	00	20
*	R1D6[]	10	10
*	R1D7[]		00
*	R1DA[]	80	01
*	R1DB[]	80	01
*	R1E6[]	00  20		PGA high
* external
*  InputMain
* @todo pls, remove or refind. it is too big.
*/

void aRGB_SetDefaultFor(void)
{
#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC)
	BYTE rvalue;
#endif
	//dPrintf("\n\raRGB_SetDefaultFor()");

	if ( InputMain == INPUT_COMP ) {
#ifdef SUPPORT_COMPONENT
		WriteTW88(REG1C0,0x10);	// mode for SOG slicer
		WriteTW88(REG1C2,0xD2);	// ==> VCO Charge pump		POST:1. VCO:10~54MHz Pump:5uA
		WriteTW88(REG1C6,0x20);	// PLL loop control
		WriteTW88(REG1C9,0x00);	// Pre-coast = 0
		WriteTW88(REG1CA,0x00);	// Post-coast = 0
		WriteTW88(REG1CB,0xD6);	// Power up PLL, SOG
		WriteTW88(REG1CC,0x00);	// ==> Sync selection

		WriteTW88(REG1D0,0x00);	// ADC gain
		WriteTW88(REG1D1,0xF0);	// 
		WriteTW88(REG1D2,0xF0);	// 
		WriteTW88(REG1D3,0xF0);	// 

		WriteTW88(REG1D4,0x20);	// clamp mode
		WriteTW88(REG1D5,0x00);	// clamp start
		WriteTW88(REG1D6,0x10);	// clamp stop
		WriteTW88(REG1D7,0x00);	// clamp pos.
		WriteTW88(REG1D9,0x02);	// clamp Y level
		WriteTW88(REG1DA,0x80);	// clamp U level
		WriteTW88(REG1DB,0x80);	// clamp V level
		WriteTW88(REG1DC,0x10);	// HS width

		WriteTW88(REG1E2,0x59);	//***	0x59
		WriteTW88(REG1E3,0x17);	//***	0x37
		WriteTW88(REG1E4,0x34);	//***	0x55
		WriteTW88(REG1E5,0x33);	//***	0x55

		WriteTW88(REG1E6,0x20);	// PGA high speed

		//set default divider(856-1. for 480i or 480p) & phase. 
		aRGB_LLPLLSetDivider(0x035A, 1);
		//rvalue=GetPhaseEE(EE_YUVDATA_START+0);
		//if(rvalue==0xff)
			rvalue=0;
		aRGB_SetPhase(rvalue, 0);

		aRGB_setSignalPath(0); //add 130227

#endif
	}
	else if ( InputMain == INPUT_PC ) {
#ifdef SUPPORT_PC
		WriteTW88(REG1C0,0x08);	// mode for HV sync
		WriteTW88(REG1C2,0xD2);	// ==> VCO Charge pump		POST:1. VCO:10~54MHz Pump:5uA
		WriteTW88(REG1C6,0x20);	// PLL loop control
		WriteTW88(REG1C9,0x00);	// Pre-coast = 0
		WriteTW88(REG1CA,0x00);	// Post-coast = 0
		WriteTW88(REG1CB,0x56);	// Power up PLL
		WriteTW88(REG1CC,0x12);	// ==> Sync selection

		WriteTW88(REG1D0,0x00);	// ADC gain
		WriteTW88(REG1D1,0xF0);	// 
		WriteTW88(REG1D2,0xF0);	// 
		WriteTW88(REG1D3,0xF0);	// 

		WriteTW88(REG1D4,0x20);	// clamp mode
		WriteTW88(REG1D5,0x00);	// clamp start
		WriteTW88(REG1D6,0x10);	// clamp stop
		WriteTW88(REG1D7,0x00);	// clamp pos.
		WriteTW88(REG1D9,0x02);	// clamp G/Y level
		WriteTW88(REG1DA,0x01);	// clamp B/U level
		WriteTW88(REG1DB,0x01);	// clamp R/V level
		WriteTW88(REG1DC,0x10);	// HS width

		WriteTW88(REG1E2,0x59);	//***  0x59
		WriteTW88(REG1E3,0x17);	//***  0x37
		WriteTW88(REG1E4,0x34);	//***  0x55
		WriteTW88(REG1E5,0x33);	//***  0x55

		WriteTW88(REG1E6,0x20);	// PGA high speed

		//set default divider(1056, for SVGA) & phase. 
		aRGB_LLPLLSetDivider(0x0420, 1);	
		rvalue=GetPhaseEE(5);	//SVGA.
		if(rvalue==0xff)
			rvalue=0;
		aRGB_SetPhase(rvalue, 0); //VGA

		aRGB_setSignalPath(1); //add 130207
#endif
	}
	else {
		//power down SOG,PLL,Coast
		//same as aRGB_SetPowerDown();	
		WriteTW88(REG1CB, (ReadTW88(REG1CB) & 0x1F));
		aRGBSetClockSource(1);			//select 27MHz. R1C0[0]
	}	
}




//-----------------------------------------------------------------------------
//R1D0
//R1D1 Y channel gain
//R1D2 C channel gain
//R1D3 V channel gain
//read RGB max value from meas and adjust color gain value on VAdc.
//-----------------------------------------------------------------------------

#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
//extern BYTE WaitStableLLPLL(WORD delay);
//BYTE aRGB_SetupLLPLL(WORD divider, /*BYTE ctrl,*/ BYTE fInit, BYTE delay)
//-----------------------------------------------------------------------------
/**
* update LLPLL divider
*
*/
BYTE aRGB_LLPLLUpdateDivider(WORD divider, BYTE fInit, BYTE delay)
{
	BYTE ret;
	
	ret = ERR_SUCCESS;
	
	aRGB_SetFilterBandwidth(0, 0);	//clear	filter bandwidth

	aRGB_LLPLLSetDivider(divider, fInit);
	if(fInit) {
		delay1ms(delay);
		if(WaitStableLLPLL())
			ret = ERR_FAIL;
	}
	aRGB_SetFilterBandwidth(7, 0);	//restore
	
	return ret;
}
#endif


#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
/**
* wait stable LLPLL input.
*
* @return
*	0:success. ERR_SUCCESS.
*	1:fail. ERR_FAIL
*/
static BYTE WaitStableLLPLL(void)
{
	BYTE	i;
	WORD	HActive, HActiveOld, HStart;
	volatile BYTE	status;
	BYTE StatusOld;

	//if(delay)
	//	delay1ms(delay);

	argb_dPrintf("WaitStableLLPLL: ");
	for(i=0; i < 128; i++) {	//max loop
		Meas_StartMeasure();
		if(Meas_IsMeasureDone(50)) {
			argb_dPrintf("fail measure\n");
			return ERR_FAIL;
		}
		HActive = MeasGetHActive( &HStart );
		status = aRGB_GetInputStatus();
		if(i==0) {
			//skip. 
			//I will update StatusOld and HActiveOld.
		}
		else if((HActive==HActiveOld) && (status == StatusOld)) {
			argb_dPrintf("%bd times, HStart:%d, HActive:%d InputStatus:0x%bx\n", i, HStart, HActive, status);
			return ERR_SUCCESS;
		}
		HActiveOld = HActive;
		StatusOld = status;
	}
	argb_dPrintf("fail max loop\n");
	return ERR_FAIL;
}
#endif


#if defined(SUPPORT_PC) || defined(SUPPORT_DVI) || defined(SUPPORT_HDMI)
//-----------------------------------------------------------------------------
/** 
* adjust the pixel clock
*
* oldname: void	PCLKAdjust( BYTE mode )
*
* INPUT_PC
*	use mode.
*
* INPUT_DVI
*	skip divider & mode.
*
* INPUT_HDMI
*	use divider.
*/
#ifdef SUPPORT_ARGB
void AdjustPixelClk(WORD digital_divider, BYTE mode )
{
#if defined(PANEL_AUO_B133EW01) || defined(PANEL_1024X600)
	//do nothing
	WORD wTemp;
	wTemp = digital_divider;
	wTemp = mode;
#else
	DWORD	PCLK, PCLK1, PCLK2;	
	BYTE	i, PCLKO;
	WORD	HDown, /*HPeriod,*/  Divider, VPN, VScale, HActive, H_DE;
	DWORD	/*VPeriod,*/ VFreq;


	PCLK = Sspll2GetFreq();
	//	FPCLK1 calculation
	//	FREQ = REG(0x0f8[3:0],0x0f9[7:0],0x0fa[7:0])															   	
	//	POST = REG(0x0fd[7:6])
	//	Hperiod = REG(0x524[7:0],0x525[7:0])
	//	Divider = REG(0x1c3[3:0],0x1c4[7:0]) ;;InFreq = (Divider+1) * (27000000 * FREQ / ((2^15)*(2^POST))) / Hperiod
	//	Hdown = REG(0x20a[3:0],0x209[7:0])
	//	PCLKO = REG(0x20d[1:0]) {1,1,2,3}
	//	PCLKx = REG(0x20d[1:0]) {1,2,3,4}
	//	result = ((Divider+1) * (27000000 * FREQ / ((2^15)*(2^POST))) / Hperiod) * (1024 / Hdown) * (PCLKx / PCLKO)
	//	result = ((Divider+1) * FPCLK / Hperiod) * (1024 / Hdown) * (PCLKx / PCLKO)

	HDown=ScalerReadXDownReg();
	//HPeriod = MeasGetHPeriodReg();
	//VPeriod = MeasGetVPeriod27();
	//VFreq = 27000000L / VPeriod;
	VFreq = MeasGetVFreq();
#if defined(SUPPORT_PC)
	if(InputMain==INPUT_PC /*|| InputMain==INPUT_COMP*/) {
		Divider = aRGB_LLPLLGetDivider() + 1;
		//Divider = PCMDATA[ mode ].htotal - 1 +1;
	}
	else 
#endif
	{
		//DTV input(DVI,HDMI)
#ifdef SUPPORT_DVI
		//if DVI ??. No Component
		if(InputMain==INPUT_DVI) {
			Divider = MeasGetDviDivider();
		}
		else 
#endif
		{
			//HDMI
			Divider = digital_divider; //DVI_Divider;
		}
	}

	VPN = MeasGetVPeriod();
	VScale = ScalerReadVScaleReg();

	H_DE = ScalerReadHDEReg();
	HActive = ScalerReadOutputWidth();
	//	FPCLK2 calculation
	//	PCLKx = REG(0x20d[1:0]) {1,2,3,4}
	//	VPN    = REG(0x522[7:0],0x523[7:0])
	//	Vscale = REG(0x206[7:0],0x205[7:0]) ;;Vtotal = VPN / (Vscale / 8192)
	//	H_DE   = REG(0x210[7:0])
	//	Hactive= REG(0x212[3:0],0x211[7:0]) ;;Htotal = H_DE + Hactive + 10
	//	Vperiod = REG(0x543[7:0],0x544[7:0],0x545[7:0]) ;;Vfreq = 27000000 / Vperiod
	//	result = (H_DE + Hactive + 1) * (VPN / (Vscale / 8192)) * (27000000 / Vperiod) * PCLKx

	//dPrintf("\n\rPCLK:%ld, Divider: %d, HPeriod: %d, HDown: %d", PCLK, Divider, HPeriod, HDown);
	dPrintf("PCLK:%ld, Divider: %d,  HDown: %d\n", PCLK, Divider, HDown);
	if(InputMain==INPUT_PC) {
		for ( i=2; i<=4; i++ ) {
			//PCLK1 = (DWORD)(((Divider+1) * PCLK / HPeriod) * (1024 / HDown) * i ) / (i-1);
			/*
			PCLK1 = PCLK / HPeriod;
			dPrintf("\n\r PCLK1 = PCLK / HPeriod :: %ld", PCLK1 );
			PCLK1 *= (Divider+1);
			dPrintf("\n\r PCLK1 *= (Divider+1) :: %ld", PCLK1 );
			PCLK1 /= HDown;
			dPrintf("\n\r PCLK1 /= HDown :: %ld", PCLK1 );
			PCLK1 *= 1024;
			dPrintf("\n\r PCLK1 *= 1024 :: %ld", PCLK1 );
			PCLK1 = (PCLK1 * i) / (i-1);
			*/
			PCLK1 = ((((Divider+1) * VFreq * VPN ) / HDown) * 1024 * i ) / (i-1);
			PCLK2 = (DWORD)( H_DE + HActive + 1 ) * ( VPN * 8192L* VFreq * i / VScale ) ;
			/*
			pclk1 = hPeriod * vPeriod * vfreq * (1024 / h_down) * (i / (i-1))
			      = input_pclk * (1024/h_down) * (i/(i-1))
			pclk2 = (hDE+PANEL_H + 1) * vPeriod * (8192/vScale) * vFreq * i
			      = min_output_h_total * output_vTotal * vFreq * i
				  = min_output_pclk *i
			*/
			dPrintf("[%bd] - PCLK1: %ld, PCLK2: %ld\n", i, PCLK1, PCLK2);
			if ( i == 2 ) {
				PCLKO = 2;
				if ( PCLK1 > PCLK2 ) {
					PCLK = PCLK1;
				}
				else {
					PCLK = PCLK2;
				}
			}
			else {
				if ( PCLK1 > PCLK2 ) {
					if ( PCLK > PCLK1 )	{
						PCLK = PCLK1;
						PCLKO = i;
					}
				}
				else {
					if ( PCLK > PCLK2 )	{
						PCLK = PCLK2;
						PCLKO = i;
					}
				}
			}
		}
		PclkoSetDiv(PCLKO-1);
		if(mode>=5 && mode <= 8)	//640x480@60 
			PclkoSetPolarity(0);	//normal
		else
			PclkoSetPolarity(1);	//invert


		dPrintf("Minimum PCLK is %ld at PCLKO: %bd", PCLK, PCLKO );
		PCLK = PCLK + 4000000L;
		dPrintf("  Add 2MHz to PCLK is %ld\n", PCLK );
	
		Sspll2SetFreq(PCLK, 0); 	
	}
	else {
		//DVI & HDMI
		i = 3;
		{
			PCLK1 = ((((Divider+1) * VFreq * VPN ) / HDown) * 1024 * i ) / (i-1);
			PCLK2 = (DWORD)( H_DE + HActive + 1 ) * ( VPN * 8192L* VFreq * i / VScale ) ;
			dPrintf("[%bd] - PCLK1: %ld, PCLK2: %ld\n", i, PCLK1, PCLK2);
			if ( PCLK1 > PCLK2 ) {
				PCLK = PCLK1;
			}
			else {
				PCLK = PCLK2;
			}
			PCLK += 5000000L;
			if ( PCLK < 108000000L )	
				PCLK = 108000000L;
			else if ( PCLK > 120000000L )
				PCLK = 120000000L;
		}
		dPrintf("  Found PCLK is %ld\n", PCLK, PCLKO );
		Sspll2SetFreq(PCLK, 0);	
	}
#endif //..PANEL_AUO_B133EW01
}
#endif
#endif


#if 0 //only for test
code BYTE div2_table[5] = { 2,3,4,6,8 };
//BKFYI: PANEL_AUO_B133EW01 needs high speed sspll for SPI OSD.
//       we can use this method.
//       But, the other will be working.
void AdjustPixelClk_TEST(DWORD hPeriod)
{
	DWORD sspll;
	WORD hDownScale;
	BYTE vfreq;
	WORD vPeriod, vScale;
	BYTE i;
	DWORD output_pclk;
	DWORD target_hPeriod;
	DWORD output_vPeriod;
	BYTE start,suggest,stop;


	sspll = Sspll1GetFreq();
	hDownScale=ScalerReadXDownReg();
	vfreq = MeasGetVFreq();
	//hPeriod = 
	vPeriod = MeasGetVPeriod();
	vScale = ScalerReadVScaleReg();

	//Output_hDE = ScalerReadHDEReg();
	//Output_hActive = ScalerReadOutputWidth();
//	output_min_hPeriod = ScalerReadHDEReg() +  PANEL_H + 1;
	output_vPeriod = (DWORD)vPeriod * 8192 / vScale; 		//it is a fixed value.

//	input_pclk = hPeriod * vPeriod * vfreq * 1024 / hDownScale;
//	output_pclk	= min_output_hPeriod * output_vPeriod * vfreq;

	//output_hPeriod

	start = 0;
	suggest = 0;
	stop = 0;
	//We have a panel spec that indicate min, max value for panel
	for(i= PANEL_PCLK_MIN; i <= PANEL_PCLK_MAX; i++) {
		output_pclk = (DWORD)i * 1000000; //make MHz. *0xF4240
		target_hPeriod = output_pclk / output_vPeriod / vfreq;
		Printf("\n\ri:%bd hPeriod:%ld",i,target_hPeriod);
		if(PANEL_H_MIN && start==0) {
			if(target_hPeriod > PANEL_H_MIN) {
				Printf(" --> start");
				start=i;
			}
		}
		if(suggest==0) {
			if(target_hPeriod > PANEL_H_TYP) {
				Printf(" --> suggest. I want to use big#");
				suggest = i;
			}
		}
		if(PANEL_H_MAX && stop == 0) {
			if(target_hPeriod > PANEL_H_MAX) {
				Printf(" --> stop. do not use it");
				stop=i;
				break;
			}
		}
	}
	if(start==0) {
		Printf("\n\rNo start. Use SSPLL:%bd PCLKO:%bd", PANEL_SSPLL, PANEL_PCLKO_DIV);
		return; //fail
	}
	if(suggest==0) {
		suggest = PANEL_PCLK_MAX;		
	}
	//find divider & sspll. 
	i = 5;
	do {
		i--;
		sspll = suggest * div2_table[i];
		sspll >>= 1;
		Printf("\n\ri:%bd sspll:%ld",i,sspll);

		if(sspll < 130) {		
			if(sspll > 80) {
				Printf(" --> use it. div%bd.%bd", div2_table[i] >> 1, div2_table[i] & 1 ? 5: 0);				
			}
		}
	} while(i);
}
#endif

//=============================================================================
//setup menu interface
//=============================================================================
#ifdef SUPPORT_PC
//-----------------------------------------------------------------------------
/**
*
* extern
*	RGB_hStart
*/
void PCRestoreH(void)
{
	WORD hstart;
	hstart = RGB_hStart;

	if(Input_aRGBMode==0) {
		//?Freerun mode
		return;
	}
	//adjust EEPROM. 0..100. base 50. reversed value.
	hstart += 50;
	hstart -= GetHActiveEE(Input_aRGBMode); //PcBasePosH;
	InputSetHStart(hstart);
}
//-----------------------------------------------------------------------------
/**
*
* extern
*	RGB_vDE
*/
void PCRestoreV(void)
{
	WORD temp16;
	temp16 = RGB_vDE;
	dPrintf("\n\r\tV-DE start = %d", temp16);

	if(Input_aRGBMode==0) {
		//?Freerun mode
		return;
	}

	temp16 += GetVBackPorchEE(Input_aRGBMode);
	temp16 -= 50;
	dPrintf("=> %d", temp16);
	ScalerWriteVDEReg((BYTE)temp16);
//BKTODO131011: use void ScalerSet_vDE_value(BYTE vStart)
}
//-----------------------------------------------------------------------------
/**
*
* extern
*	Input_aRGBMode
*/
void PCResetCurrEEPROMMode(void)
{
	BYTE temp;
	temp = GetPixelClkEE(Input_aRGBMode);
	if(temp!=50)
		SavePixelClkEE(Input_aRGBMode,50);

	temp = GetPhaseEE(Input_aRGBMode);
	if(temp != 0xFF)
		SavePhaseEE(Input_aRGBMode,0xFF);	

	temp = GetHActiveEE(Input_aRGBMode);
	if(temp!=50)
		SaveHActiveEE(Input_aRGBMode,50);

	temp = GetVBackPorchEE(Input_aRGBMode);
	if(temp!=50)
		SaveVBackPorchEE(Input_aRGBMode,50);	

	//BK131025 ?? Where is VActive? No one uses it.
	//char GetVActiveEE(BYTE mode)
	//void SaveVActiveEE(BYTE mode, char value)
}
#endif



//=============================================================================
// Change to COMPONENT (YPBPR)
//=============================================================================


#ifdef SUPPORT_COMPONENT


//-----------------------------------------------------------------------------
/**
* find component input mode
*
* @return
*	0xFF: fail.
*	other:success. component mode value.
*
*	0:480i
*	1:576i@50
*	2:480p
*	3:576p@50
*	4:1080i@50
*	5:1080i
*	6:720p@50
*	7:720p
*	8:1080p@50
*   9:1080p
*
*/
static BYTE FindInputModeCOMP( void )
{
	WORD	vtotal;
	BYTE	vfreq, i;
	WORD wTemp;

	//
	// get a vertical frequency and  a vertical total scan lines.
	//
	//BKFYI. We donot have a PLL value yet that depend on the mode.
	//so, we better use 27MHz register.

	vtotal = MeasGetVPeriod();	//Vertical Period Registers

	//if video signal starts, HW needs a time.
	//normally 30mS.
	for(i=0; i < 10; i++) {
		Meas_StartMeasure();
		Meas_IsMeasureDone(50);
		wTemp = MeasGetVPeriod();	//Vertical Period Registers	
		if(wTemp == vtotal)
			break;
		vtotal = wTemp;
		delay1ms(5);
	}
	vfreq = MeasGetVFreq();
	if ( vfreq < 55 ) vfreq = 50;
	else  vfreq = 60;

	if ( vfreq == 50 ) {
		if ( vtotal < 200)			i = 0xFF;
		else if ( vtotal < 320 )	i = MEAS_COMP_MODE_576I;	// 576i	 = 625 for 2, 312.5
		else if ( vtotal < 590 )	i = MEAS_COMP_MODE_1080I25;	// 1080i50A
		else if ( vtotal < 630 )	i = MEAS_COMP_MODE_576P;	// 576P=625	or 1080i50B = sync=5	  
																// vblank length different 576P=45, 1080i=21
																// can check with even/odd measure
		else if ( vtotal < 800 )	i = MEAS_COMP_MODE_720P50;	// 720P = 750
		else if ( vtotal < 1300 )	i = MEAS_COMP_MODE_1080P50;	// 1080P = 1250 total from set-top box
		else 						i = 0xFF;	
	}
	else {
		if ( vtotal < 200)			i = 0xFF;
		else if ( vtotal < 300 )	i = MEAS_COMP_MODE_480I;	// 480i = 525 for 2, 262.5
		else if ( vtotal < 540 )	i = MEAS_COMP_MODE_480P;	// 480P	= 525
		else if ( vtotal < 600 )	i = MEAS_COMP_MODE_1080I30;	// 1080i
		else if ( vtotal < 800 )	i = MEAS_COMP_MODE_720P60;	// 720P = 750
		else if ( vtotal < 1300 )	i = MEAS_COMP_MODE_1080P60;	// 1080P
		else 						i = 0xFF;	
	}

	dPrintf( "\n\rCOMP %bd: vFreq:%bdHz, vTotal:%d", i, vfreq, vtotal );
	return (i);							// if 0xff, not support
}

//-----------------------------------------------------------------------------
/**
* convert the component mode to HW mode.
*
* SW and HW use a different mode value.
* ISR will check the HW mode value to check the SYNC change.
*
* Or, just read REG1C1[2:0].
*/
static BYTE ConvertComponentMode2HW(BYTE mode)
{
	BYTE new_mode;
	switch(mode) {
	case 0: new_mode = mode;	break;	//480i
	case 1:	new_mode = mode;	break;	//576i
	case 2:	new_mode = mode;	break;	//480p
	case 3:	new_mode = mode;	break;	//576p
	case 4:	new_mode = 4;		break;	//1080i25->1080i
	case 5:	new_mode = 4;		break;	//1080i30->1080i
	case 6:	new_mode = 5;		break;	//720p50->720p
	case 7:	new_mode = 5;		break;	//720p60->720p
	case 8:	new_mode = 6;  		break;	//1080p50->1080p
	case 9:	new_mode = 6;		break;	//1080p60->1080p
	default: new_mode = 7;		break;	//UNKNOWN->non of above
	}
	return new_mode;
}
//BYTE ConvertComponentMode2SW(BYTE mode)
//{
//	BYTE new_mode;
//	switch(mode) {
//	case 0: new_mode = mode;	break;	//480i
//	case 1:	new_mode = mode;	break;	//576i
//	case 2:	new_mode = mode;	break;	//480p
//	case 3:	new_mode = mode;	break;	//576p
//	case 4: new_mode = 5;       break;	//1080i->1080i30
//	case 5: new_mode = 7;		break;	//720p->720p60
//	case 6: new_mode = 9;		break;	//1080p->1080p60
//	default: new_mode = 2;		break;	//unknown->480p
//	}
//	return new_mode;
//}


/**
 *
 * param fOverScan : turn on overscan feature.
 *
 */
//#if 0
//void aRGB_COMP_set_scaler(BYTE mode, BYTE fBT656, BYTE fOverScan)
//{
//	//BYTE	i,j;
//	//BYTE	mode, modeNew;
//	//BYTE ret;
//
//	BYTE bTemp;
//	WORD wTemp;
//	WORD hStart,vStart;
//	WORD hActive,vActive;
//	struct COMP_VIDEO_TIME_TABLE_s *pTimeTable;
//
//	pTimeTable = &TW8836_COMP_TABLE[mode];
//
//	Printf("\n\raRGB_COMP_set_scaler(%bd,%bd)",mode,fOverScan);
//	Printf(" %dx%d%s@%bd", pTimeTable->hActive, pTimeTable->vActive, pTimeTable->pol & INTERLACED ? "I":"P", pTimeTable->vFreq);
//
//	hStart  = pTimeTable->hStart;
//	hActive = pTimeTable->hActive;
//	vStart  = pTimeTable->vBPorch + pTimeTable->vSync;
//	vActive = pTimeTable->vActive;
//	if(fOverScan) {
//		hStart  += pTimeTable->hOverScan;
//		hActive -= pTimeTable->hOverScan*2;
//		vStart  += pTimeTable->vOverScan;
//		vActive -= pTimeTable->vOverScan*2;
//	}
//	if(fBT656) {
//		hStart += 57;
//
//		hActive += 8;
//		hActive *= 2;
//	}			
//  
//	InputSetCrop(hStart, vStart, hActive, vActive);	
//	if(mode==MEAS_COMP_MODE_480I 
//	|| mode==MEAS_COMP_MODE_576I 
//	|| mode==MEAS_COMP_MODE_720P60)
//		InputSetFieldPolarity(0);
//	else
//		InputSetFieldPolarity(1);
//
//	//if(pTimeTable->pol & INTERLACED)
//	//	WriteTW88(REG041, ReadTW88(REG041) & ~0x20);
//	InputSetProgressiveField(pTimeTable->pol & INTERLACED ? 0 : 1)
//	if(fBT656)
//		hActive /= 2;
//
//	ScalerSetHScale(hActive);
//	ScalerSetVScale(vActive);
//
//	//hDE
//	bTemp = ScalerCalcHDE();	//ScalerReadLineBufferDelay() + 32;
//	ScalerWriteHDEReg(bTemp);
//	//vDE
//	wTemp = (DWORD)vStart * PANEL_V / vActive;
//	ScalerWriteVDEReg(wTemp);
////BKTODO131011: use void ScalerSet_vDE_value(BYTE vStart)
//
//	//temp for BT656....
//	ScalerSetLineBufferSize(hActive);
//}
//#endif


/**
* prepare Info String for Component.
* ex: "Component 1080p 60Hz".
*/
static void YUV_PrepareInfoString(BYTE mode)
{
	FOsdSetInputMainString2FOsdMsgBuff();										 	
	TWstrcat(FOsdMsgBuff," ");
	switch(mode) {
	case 0:	TWstrcat(FOsdMsgBuff,"480i");	break;
	case 1:	TWstrcat(FOsdMsgBuff,"576i");	break;
	case 2:	TWstrcat(FOsdMsgBuff,"480p");	break;
	case 3:	TWstrcat(FOsdMsgBuff,"576p");	break;
	case 4:	TWstrcat(FOsdMsgBuff,"1080i 50Hz");	break;
	case 5:	TWstrcat(FOsdMsgBuff,"1080i 60Hz");	break;
	case 6:	TWstrcat(FOsdMsgBuff,"720p 50Hz");	break;
	case 7:	TWstrcat(FOsdMsgBuff,"720p 60Hz");	break;
	case 8:	TWstrcat(FOsdMsgBuff,"1080p 50Hz");	break;
	case 9:	TWstrcat(FOsdMsgBuff,"1080p 60Hz");	break;
	default:TWstrcat(FOsdMsgBuff,"Unknown");	break;
	}
}

//-----------------------------------------------------------------------------
/**
* check and set the componnent
*
* oldname: BYTE CheckAndSetYPBPR( void )
* @return
*	success	:ERR_SUCCESS
*	fail	:ERR_FAIL
*/
BYTE CheckAndSetComponent( void )
{
	BYTE	i,j;
	BYTE	mode, modeNew;
	BYTE ret;

	BYTE bTemp;
	WORD hStart,vStart;
	WORD hActive,vActive;
	BYTE fScale;

	struct COMP_VIDEO_TIME_TABLE_s *pTimeTable;
	struct SCALER_TIME_TABLE_s *pScaler;

	/*search component video mode by vFreq and vTotal.*/
	Input_aRGBMode = 0;
	for(i=0; i < 10; i++) {
		for(j=0; j < 10; j++) {
			Meas_StartMeasure();
			if(Meas_IsMeasureDone(50)) {
				return ERR_FAIL;
			}
			/* find input mode from Vfreq and VPeriod */
			mode = FindInputModeCOMP();	
			if(mode != 0xFF)
				break;
		}	
		if(mode==0xFF) {
			return ERR_FAIL;
		}

		pTimeTable = &TW8836_COMP_TABLE[mode];

		aRGB_SetLLPLLControl(0xF2);	// POST[7:6]= 3 -> div 1, VCO: 40~216, Charge Pump: 5uA
		ret = aRGB_LLPLLUpdateDivider(pTimeTable->hTotal - 1, 1, 0 );
		if(ret==ERR_FAIL) {
			return ERR_FAIL;
		}		

		/* find input mode again and compare it is same or not */
		modeNew = FindInputModeCOMP();	
		if(mode==modeNew)
			break;
		//retry..
	}
	PrintCompVideoTimeTable(pTimeTable);

	Input_aRGBMode = mode;							//SW value
	InputSubMode = ConvertComponentMode2HW(mode);	//HW value

	InitComponentReg(mode);


	aRGB_SetClampModeHSyncEdge(ON);
	aRGB_SetClampPosition(pTimeTable->ClampPos);

	MeasSetErrTolerance(4);							//tolerance set to 32
	MeasEnableChangedDetection(ON);					// set EN. Changed Detection

	hStart  = pTimeTable->hBPorch;
	hActive = pTimeTable->hActive;
	vStart  = pTimeTable->vBPorch + pTimeTable->vSync;
	vActive = pTimeTable->vActive;

	/* read scale mode. 0:overscan(default), 1:full */
	fScale = EE_Read(EEP_INPUT_COMP);
	if(fScale==0) { /*ANALOG_OVERSCAN*/
		hStart  += pTimeTable->hOverScan;
		hActive -= pTimeTable->hOverScan*2;
		vStart  += pTimeTable->vOverScan;
		vActive -= pTimeTable->vOverScan;
	}
	/* set input crop */
	InputSetCrop(hStart, vStart, hActive+1, vActive+1);	

	//BKTODO140908..Field & Pclko polarity need more investigation.
	//              I am using table...
	if(mode==MEAS_COMP_MODE_480I 
	|| mode==MEAS_COMP_MODE_576I 
	|| mode==MEAS_COMP_MODE_720P60)
		InputSetFieldPolarity(0);
	else
		InputSetFieldPolarity(1);
	InputSetProgressiveField(pTimeTable->pol & INTERLACED ? 0 : 1);
 
	if(fScale==0) /*ANALOG_OVERSCAN*/
		vActive -= pTimeTable->vOverScan;

	/* set scaler */
	ScalerSetHScale(hActive);
	ScalerSetVScale(vActive);
	ScalerSet_vDE_value(vStart);

	//hDE
	bTemp = ScalerCalcHDE(); //ScalerReadLineBufferDelay() + 32;
	ScalerWriteHDEReg(bTemp);

#if 0  //BK150717
	scaler_set_output(hActive,vActive,vStart,0/*pTimeTable->vOffset*/);
#endif	


 	PclkoSetPolarity(pTimeTable->pclko_pol);	

#if defined(PANEL_AUO_B133EW01) || defined(PANEL_1024X600)
	//use a table or 
	AdjustSSPLL_with_HTotal();
#else
	Sspll2SetFreqReg(SSPLL_72M_REG);	//72MHz/1/2=36MHz 
	PclkSetDividerReg(PCLK_DIV1);
	PclkoSetDiv_with_pol(PCLKO_DIV2);
#endif

	/* search scaler table. If success, overwrite */
#ifdef SUPPORT_SCALER_OVERWRITE_TABLE
	pScaler = FindScalerTable(InputMain, pTimeTable->hActive,pTimeTable->vActive,pTimeTable->vFreq, 0,0);
	if(pScaler != NULL)
		OverWriteScalerWithTable(pScaler,1,1);
#endif

	YUV_PrepareInfoString(mode);

	return ERR_SUCCESS;
}


//-----------------------------------------------------------------------------
/**
* Change to Component
*
* @return
*	- 0: success
*	- 1: No Update happen
*	- 2: No Signal or unknown video sidnal.
*/
BYTE ChangeComponent( void )
{
	BYTE ret;

	if ( InputMain == INPUT_COMP ) {
		dPrintf("\n\rSkip ChangeComponent");
		return(1);
	}
		
	InputMain = INPUT_COMP;
	InputSubMode = 7; //N/A. Note:7 is a correct value.

 	if(GetInputMainEE() != InputMain)
		SaveInputMainEE( InputMain );

	//----------------
	// initialize video input
	InitInputAsDefault();

	//
	// Check and Set aRGB,mesaure,Scaler for Component input
	//
	ret = CheckAndSetComponent();
	if(ret==ERR_SUCCESS) {
		//success
		VInput_enableOutput(0);
		return 0;
	}
	//------------------
	// NO SIGNAL

	//start recover & force some test image.
	VInput_gotoFreerun(0);

	return(2);  //fail
}
#endif
//=============================================================================
// Change to PC
//=============================================================================


#ifdef SUPPORT_PC

//-----------------------------------------------------------------------------
/**
* gain control
*
*	register
*	R1D0[2]R1D1[7:0]	Y/G channel gain
*	R1D0[1]R1D2[7:0]	C/B channel gain
*	R1D0[0]R1D3[7:0]	V/R channel gain
*/
void aRGB_SetChannelGainReg(WORD GainG,WORD GainB,WORD GainR)
{
	WriteTW88(REG1D1, GainG );
	WriteTW88(REG1D2, GainB );
	WriteTW88(REG1D3, GainR );
	WriteTW88(REG1D0, (GainR >> 8)+ ((GainB >> 7) & 2) + ((GainG >> 6) & 4 ));
}
WORD aRGB_ReadGChannelGainReg(void)
{
	WORD wTemp;
	wTemp = ReadTW88(REG1D0) & 0x04;
	wTemp <<= 6;
	wTemp |= ReadTW88(REG1D1);
	return wTemp;
}
WORD aRGB_ReadBChannelGainReg(void)
{
	WORD wTemp;
	wTemp = ReadTW88(REG1D0) & 0x02;
	wTemp <<= 7;
	wTemp |= ReadTW88(REG1D2);
	return wTemp;
}
WORD aRGB_ReadRChannelGainReg(void)
{
	WORD wTemp;
	wTemp = ReadTW88(REG1D0) & 0x01;
	wTemp <<= 8;
	wTemp |= ReadTW88(REG1D3);
	return wTemp;
}

/**
* get a vertical frequency and  a vertical total scan lines.
*
*/
static void get_vTotal_vFreq(WORD *p_vTotal, BYTE *p_vFreq)
{
	WORD vTotal, wTemp;
	BYTE vFreq;
	BYTE i;

	vTotal = MeasGetVPeriod();
	for(i=0; i < 10; i++) {
		Meas_StartMeasure();
		Meas_IsMeasureDone(50);
		wTemp = MeasGetVPeriod();	//Vertical Period Registers	
		if(wTemp == vTotal)
			break;
		vTotal = wTemp;
	}
	*p_vTotal = vTotal;
	vFreq = MeasGetVFreq();
	*p_vFreq = MeasRoundDownVFreqValue(vFreq);
}


//-----------------------------------------------------------------------------
/**
* find PC input mode
*
* @return
*	0xFF: fail
*	else: success.
*		  index number of PC Mode Data Table.
*/
static BYTE FindInputModePC(BYTE start, WORD *vt)
{
	WORD	vtotal;
	//WORD 	wTemp;
	BYTE	vFreq, i;
	struct DIGIT_VIDEO_TIME_TABLE_s *pTimeTable;
	start++;	//start from next mode.

	//
	// get a vertical frequency and  a vertical total scan lines.
	//
	//We donot have a PLL value yet that depend on the mode.
	//so, we are using 27MHz register.
#if 0
	vtotal = MeasGetVPeriod();	//Vertical Period Registers
	//if video signal starts, HW needs a time.
	//normally ?? mS.  I saw, Compment needs 30mS.
	for(i=0; i < 10; i++) {
		Meas_StartMeasure();
		Meas_IsMeasureDone(50);
		wTemp = MeasGetVPeriod();	//Vertical Period Registers	
		if(wTemp == vtotal)
			break;
		vtotal = wTemp;
		delay1ms(5);
	}
	*vt = vtotal;

	vFreq = MeasGetVFreq();
	vFreq = MeasRoundDownVFreqValue(vFreq);

#else
	get_vTotal_vFreq(&vtotal, &vFreq);
	*vt = vtotal;
#endif
	Printf("\nFindInputModePC search vFreq:%bd vTotal:%d",vFreq,vtotal);
	//
	//Search PC mode.
	//0 is unknown.
	for (i=start;  ; i++) {
		pTimeTable = &TW8836_VESA_TABLE[i];
		if(	pTimeTable->hActive ==0 && pTimeTable->vActive ==0)
			break;
		if ( pTimeTable->vid == 0 ) //?support
			continue;

		if ( pTimeTable->vFreq == vFreq ) {			//check vfreq
			if(( pTimeTable->vTotal == vtotal )		//check vtotal 
			|| ( pTimeTable->vTotal == (vtotal+1) ) 
			|| ( pTimeTable->vTotal == (vtotal-1) )
			){
				//dPrintf("\n\rFindInputModePC mode:%bd %dx%d@%bd vTotal:%d",
				//	i,pTimeTable->hActive, pTimeTable->vActive, pTimeTable->vFreq, vtotal);
				PrintVesaVideoTimeTable(pTimeTable);
				return (i);
			}
		}
	}

	ePuts( "\n\rCurrent Input resolution IS Not Supported." );
	ePrintf(" V total: %d, V freq: %bd", vtotal, vFreq );
	return (0xFF);		// not support
}


static void PC_SetScaler(BYTE mode)
{
	WORD hStart, vStart;
	WORD hActive, vActive;
	WORD Meas_hActive,Meas_vActive;
	WORD Meas_hStart,Meas_vStart;
	BYTE bTemp;
	WORD wTemp;
	struct DIGIT_VIDEO_TIME_TABLE_s *pTimeTable;

	pTimeTable = &TW8836_VESA_TABLE[mode];

#if 1 //DEBUG_PC
	//read measured value. 
	Meas_vActive = MeasGetVActive( &Meas_vStart );
	Meas_hActive = MeasGetHActive( &Meas_hStart );
	PrintMeasValue("PC");
	PrintVesaVideoTimeTable(pTimeTable);

	//measure adj
	//	hStart+4  vStart-1.
	//	on DTV, it was hStart+4,vStart-1	
	bTemp = ReadTW88(REG1CC);
	if((bTemp & 0x0C) == 0x00) {
		//if use HSO, add +4.
		//if use HS Pin, it is a correct value
		Meas_hStart += 4;
	}
	Meas_vStart -= 1;   //minus meas delay.
	PrintMeasAdjValue("PC", Meas_hStart,Meas_vStart);
#endif

	//h is unstable....I will use a table
	Meas_hStart = pTimeTable->hSync+pTimeTable->hBPorch;
	Meas_hActive = pTimeTable->hActive;
	Meas_vStart = pTimeTable->vSync + pTimeTable->vBPorch;
	Meas_vActive = pTimeTable->vActive;
	PrintMeasAdjValue("PC table", Meas_hStart,Meas_vStart);

	//read sync width.
	//If we use a HSO, HSyncWidth is not real value, it is comes from aRGB.
	//
	//meas uses active high sync, and use a rising edge.
	//if you give a active low sync to meas, we will have a wrong result.
	//I means, below two function will have a wrong value.
	//	Meas_HPulse = MeasGetHSyncRiseToFallWidth();
	//	Meas_VPulse = MeasGetVSyncRiseToFallWidth();
	//You have to adjust REG1CC[0] and REG1CC[1] to make a active high signal.
	//If you use a falling edge in inputcrop, you have to remove SyncWidth.
	//

	//read Scaler input polarity.
	//0:rising edge, 1:falling edge.
#if 1 //DEBUG_PC
	bTemp = ReadTW88(REG041);
	if(bTemp & 0xC0) 
		Printf("\nBUG..Use rising edge with ActiveHigh..");
#endif

	//-------------------------------
	//scaler adjust
	//	hStart-2, vStart+1.
	//	on DTV, it was hStart-1,vStart-1.

	hStart = Meas_hStart;
	vStart = Meas_vStart;
	hActive = Meas_hActive;
	vActive= Meas_vActive;

	RGB_hStart = hStart-2;	//save for IE.

	//adjust EEPROM value. [0..100] base 50. reversed value.
	bTemp = GetHActiveEE(mode);
	if(bTemp != 0 && bTemp != 50) {
		hStart += 50;
		hStart -= bTemp;
		dPrintf("\n\r\tModified HS:%d->%d, VS:%d", RGB_hStart, hStart, vStart );
	}
	Printf("\n\rPC Crop %d+2 hCropStart:%d-2",hActive, hStart);
	Printf("\n\r        %d+2 vCropStart:%d+1",vActive, vStart);

	InputSetCrop(hStart-2, vStart+1, hActive+2, vActive+2);
	//PrintScalerInputCrop("PC",

	ScalerSetVScale(vActive);
	ScalerSetHScale(hActive);

	//=============VDE=====================
	wTemp = ScalerCalcVDE2(vStart, pTimeTable->vOffset);	 
	dPrintf("\n\r\tV-DE start = %d", wTemp);

	RGB_vDE = wTemp;	//save for IE.	
	//adjust EEPROM value. [0..100] base 50.
	bTemp = GetVBackPorchEE(mode);
	if(bTemp != 0 && bTemp != 50) {
		wTemp += bTemp;
		wTemp -= 50;
		dPrintf("=> %d", wTemp);
	}
	ScalerWriteVDEReg(wTemp);

	//=============HDE=====================
	wTemp = ScalerCalcHDE();
	dPrintf("\n\r\tH-DE start = %d", wTemp);
	ScalerWriteHDEReg(wTemp);

#if 0  //BK150717
	scaler_set_output(hActive,vActive,vStart,pTimeTable->vOffset);
#endif	


	//FYI.
	//ScalerSetFreerun will be updated at VInput_enableOutput().

	PclkoSetPolarity(0 /*pTimeTable->pclko_pol*/); //let's do it on scaler table
}

/**
* prepare Info String for PC.
* ex: "PC 1024x768 60Hz"
*/
static void PC_PrepareInfoString(BYTE mode)
{
	struct DIGIT_VIDEO_TIME_TABLE_s *pTimeTable;
	BYTE itoa_buff[5];					

	pTimeTable = &TW8836_VESA_TABLE[mode];
	//prepare info. 
	FOsdSetInputMainString2FOsdMsgBuff();										 	
	TWstrcat(FOsdMsgBuff," ");
	TWitoa(pTimeTable->hActive, itoa_buff);
	TWstrcat(FOsdMsgBuff,itoa_buff);
	TWstrcat(FOsdMsgBuff,"x");
	TWitoa(pTimeTable->vActive, itoa_buff);
	TWstrcat(FOsdMsgBuff,itoa_buff);
	TWstrcat(FOsdMsgBuff," ");
	TWitoa(pTimeTable->vFreq, itoa_buff);
	TWstrcat(FOsdMsgBuff,itoa_buff);
	TWstrcat(FOsdMsgBuff,"Hz");
}


#undef CHECK_USEDTIME
//-----------------------------------------------------------------------------
/**
* check and set the PC
*
* calls from ChangePC and Interrupt Handler
* @return
*	0:ERR_SUCCESS
*	1:ERR_FAIL
* @see ChangePC
* @see CheckAndSetInput
* @see NoSignalTask
*/
#if 0
BYTE CheckAndSetPC___OLD(void)
{
	BYTE mode,old_mode;
	BYTE i;
	BYTE bTemp;

#ifdef CHECK_USEDTIME
	DWORD UsedTime;
#endif
	BYTE value;
	BYTE value1;
	WORD wTemp;
	BYTE ret;
	volatile BYTE InputStatus;
	struct DIGIT_VIDEO_TIME_TABLE_s *pTimeTable;
	struct SCALER_TIME_TABLE_s *pScaler;

	WORD hActive,vActive;
	WORD hCropStart,vCropStart;
	BYTE vFreq;

#ifdef CHECK_USEDTIME
    SFRB_ET0=0;
	UsedTime = SystemClock;
    SFRB_ET0=1;
#endif
	Input_aRGBMode = 0;
	InputSubMode = Input_aRGBMode;

	//check signal. if fail, give up.
	for(i=0; i < 2; i++) {
		Meas_StartMeasure();
		ret=Meas_IsMeasureDone(50);
		if(ret==ERR_SUCCESS)
			break;
		delay1ms(10);
	}	
	if(ret)	{
		argb_Printf("\n\rCheckAndSetPC fail 1");
		return 1;
	}

	/*find PC mode.*/
	old_mode = 0;
	while(1) {
		mode = FindInputModePC(old_mode, &wTemp/*&vTotal*/);
		if(mode==0xFF) {
			argb_Printf("\n\rCheckAndSetPC fail 2. No proper mode");
			return 2;
		}
//		if(old_mode == mode)
//			break;
		old_mode = mode;

		pTimeTable = &TW8836_VESA_TABLE[mode];

		//
		//set LLPLL	& wait
		//
		aRGB_SetLLPLLControl(0xF2);	// POST[7:6]= 3 -> div 1, VCO: 40~216, Charge Pump: 5uA
		ret = aRGB_LLPLLUpdateDivider(pTimeTable->hTotal - 1, 1, 40 );
		if(ret==ERR_FAIL) {
			argb_Printf("\n\rCheckAndSetPC fail 3. No stable LLPLL");
			return 3;
		}
		//LLPLL needs a time until it becomes a stable state.
		//TW8836 needs 110ms delay to get the correct vPol.
		delay1ms(120);

		//wait a detection flag.
		for(i=0; i < 50; i++) {
			InputStatus = ReadTW88(REG1C1);
			if((InputStatus & 0x30) == 0x30)
				break;
			delay1ms(10);
		}
		//note. The detected polarity can be incorrect.
		aRGB_Set_vSyncOutPolarity(1 /*1 means PC */, InputStatus & 0x80);	//VSync Polarity
		aRGB_SetLLPLL_InputPolarity(1 /*1 means PC */, InputStatus & 0x40); //HSync Polarity

		//update Phase.
		value = GetPhaseEE(mode);
		if(value == 0xFF) {
			//No previous data. We need a AutoTunePhase.
			AutoTunePhase();
			value=aRGB_GetPhase();
			argb_dPrintf("\n\rAutoTune Phase 0x%bx",value);
			SavePhaseEE(mode,value);
		}
		else {
			argb_dPrintf("\n\ruse EE Phase 0x%bx",value);
			//we read first, because update routine can make a shaking.
			value1=aRGB_GetPhase();
			if(value != value1) {
				argb_dPrintf("  update from 0x%bx",value1);
				aRGB_SetPhase(value, 0);	//BKTODO? Why it does not have a init ?
			}
		}
		ret=WaitStableLLPLL();
		if(ret) {
			ePrintf("\n\rWARNING WaitStableLLPLL faile at %d",__LINE__);
		}

		//adjust polarity again
		for(i=0; i < 50; i++) {
			InputStatus = ReadTW88(REG1C1);
			if((InputStatus & 0x30) == 0x30)
				break;
			delay1ms(10);
		}
		aRGB_Set_vSyncOutPolarity(1 /*1 means PC */, InputStatus & 0x80);   //VSync Polarity
		aRGB_SetLLPLL_InputPolarity(1 /*1 means PC */, InputStatus & 0x40); //HSync Polarity


		//reflash measure value
		Meas_StartMeasure();
		Meas_IsMeasureDone(50);
	}

	//check polarity.
	//REG1C1[7:6] and syncpol[7:6] use a same value. 
	InputStatus = ReadTW88(REG1C1);
	if((pTimeTable->pol & 0xC0) != (InputStatus & 0xC0)) {
		//incorrect polarity.
		Printf("\n\rmode:%bd hPol %s->%s vPol %s->%s",mode,
			pTimeTable->pol & HPOL_P ? "P" : "N",
			InputStatus & HPOL_P 	 ? "P" : "N",	 
			pTimeTable->pol & VPOL_P ? "P" : "N",
			InputStatus & VPOL_P 	 ? "P" : "N");	 

	}

	//final check.
//#if 0
//	old_mode = mode;
//	mode = FindInputModePC(&new_VTotal);
//	if(old_mode != mode) {
//		Printf("\n\rWARNING mode curr:%bd new:%bd",old_mode, mode);
//		mode = old_mode;
//	}
//#endif
//wTemp = aRGB_LLPLLGetDivider();
//Printf("\n PC%d LLPLL %d-1 read:%d",__LINE__, pTimeTable->hTotal, wTemp);

	PC_SetScaler(mode);

// wTemp = aRGB_LLPLLGetDivider();
//Printf("\n PC%d LLPLL %d-1 read:%d",__LINE__, pTimeTable->hTotal, wTemp);

	Input_aRGBMode = mode;
	InputSubMode = Input_aRGBMode;


	//AdjustPixelClk_TEST(pTimeTable->hTotal);
	AdjustPixelClk(0, mode); //BK120117 need a divider value

//wTemp = aRGB_LLPLLGetDivider();
//Printf("\n PC%d LLPLL %d-1 read:%d",__LINE__, pTimeTable->hTotal, wTemp);

	//update EEPROM pixel clock.
	//Note: It is not a pixel clock. It is a LLPLL divider.
	//      
	bTemp = GetPixelClkEE(mode);  //value 0..100
#if 1
	//if E3PROM does not have value, it will return 0.
	//if we receive 0, FW will subtract 50.
	//If you see the below message, you need to execute "EE default"
	if(bTemp == 0) {
		Printf("\nMaybe, something wrong!!...Please update your eeprom.");
		bTemp = 50;
	}
#endif

	if(bTemp != 50) {
		wTemp = pTimeTable->hTotal - 1;
		//Printf("\n\r!!!EEPROM has a PixelClock value %bd",bTemp);
		//Printf("\tchange LLPLL %d",wTemp);
		wTemp += bTemp;
		wTemp -= 50;
		//Printf("->%d",wTemp);
		aRGB_LLPLLUpdateDivider(wTemp, OFF, 0);	//without init.
	}

//wTemp = aRGB_LLPLLGetDivider();
//Printf("\n PC%d LLPLL %d-1 read:%d",__LINE__, pTimeTable->hTotal, wTemp);
	MeasSetErrTolerance(4);						//tolerance set to 32
	MeasEnableChangedDetection(ON);				//set EN. Changed Detection
	
	Printf("\nVESA %dx%d@%bd", pTimeTable->hActive,pTimeTable->vActive,pTimeTable->vFreq);

//wTemp = aRGB_LLPLLGetDivider();
//Printf("\n PC%d LLPLL %d-1 read:%d",__LINE__, pTimeTable->hTotal, wTemp);

 	hActive = MeasGetHActive( &hCropStart );
	vActive = MeasGetVActive( &vCropStart );
	vFreq = MeasGetVFreq();
	PrintMeasValue("PC");

	/* search scaler table. If success, overwrite */
#ifdef SUPPORT_SCALER_OVERWRITE_TABLE
	pScaler = FindScalerTable(InputMain, hActive,vActive,vFreq, vCropStart,1);
	if(pScaler != NULL)
		OverWriteScalerWithTable(pScaler,1,1);
#endif
	/* FYI:
	if 1280x1024@75, increase YScale +10.
	if 1600x1200@60, increase YScale +16.
	but, it is moved to the table.
	*/


	PC_PrepareInfoString(mode);

#ifdef CHECK_USEDTIME
	UsedTime = SystemClock - UsedTime;
	Printf("\n\rUsedTime:%ld.%ldsec", UsedTime/100, UsedTime%100 );
#endif
			
	return ERR_SUCCESS;
}
#endif

#if 1
BYTE CheckAndSetPC(void)
{
	BYTE mode,old_mode;
	BYTE i;
	BYTE bTemp;

#ifdef CHECK_USEDTIME
	DWORD UsedTime;
#endif
	BYTE value;
	BYTE value1;
	WORD wTemp;
	BYTE ret;
	volatile BYTE InputStatus;
	struct DIGIT_VIDEO_TIME_TABLE_s *pTimeTable;
	struct SCALER_TIME_TABLE_s *pScaler;

	WORD hActive,vActive;
	WORD hCropStart,vCropStart;
	BYTE vFreq;


#ifdef CHECK_USEDTIME
	SFRB_ET0 = 0;
	UsedTime = SystemClock;
	SFRB_ET0 = 1;
#endif

	/*STEP1: check signal. if fail, give up. */
	for(i=0; i < 2; i++) {
		Meas_StartMeasure();
		ret=Meas_IsMeasureDone(50);
		if(ret==ERR_SUCCESS)
			break;
		delay1ms(10);
	}	
	if(ret)	{
		argb_Printf("\n\rCheckAndSetPC fail 1");
		return 1;
	}
	/*
		FindInputModePC
	*/
	old_mode = 0xFF;
	while(1) {
		/* STEP2: find PC mode.*/
		mode = FindInputModePC(old_mode, &wTemp/*&vTotal*/);
		if(mode==0xFF) {
			argb_Printf("\n\rCheckAndSetPC fail 2. No proper mode");
			return 2;
		}
		if(old_mode == mode)
			break;
		old_mode = mode;

		pTimeTable = &TW8836_VESA_TABLE[mode];
		
		//
		//set LLPLL	& wait
		//
		aRGB_SetLLPLLControl(0xF2);	// POST[7:6]= 3 -> div 1, VCO: 40~216, Charge Pump: 5uA
		ret = aRGB_LLPLLUpdateDivider(pTimeTable->hTotal - 1, 1, 40 );
		if(ret==ERR_FAIL) {
			argb_Printf("\n\rCheckAndSetPC fail 3. No stable LLPLL");
			return 3;
		}
		//wait a detection flag.
		for(i=0; i < 50; i++) {
			InputStatus = ReadTW88(REG1C1);
			if((InputStatus & 0x30) == 0x30)
				break;
			delay1ms(10);
		}
		if(i==50) 
			Printf("\nERR Sync Fail");
		//Printf("\nH_%c V_%c ",InputStatus & 0x40 ? 'N' : 'P', InputStatus & 0x80 ? 'N' : 'P');		
		if((pTimeTable->pol & 0xC0) != (InputStatus & 0xC0)) {
			//incorrect polarity.
			Printf("\n\rmode:%bd invalid hPol %s->%s vPol %s->%s",mode,
				pTimeTable->pol & HPOL_P ? "P" : "N",
				InputStatus & HPOL_P 	 ? "P" : "N",	 
				pTimeTable->pol & VPOL_P ? "P" : "N",
				InputStatus & VPOL_P 	 ? "P" : "N");	 
	
		}
		aRGB_Set_vSyncOutPolarity(1 /*1 means PC */, InputStatus & 0x80);	//VSync Polarity
		aRGB_SetLLPLL_InputPolarity(1 /*1 means PC */, InputStatus & 0x40); //HSync Polarity

		Meas_StartMeasure();
		Meas_IsMeasureDone(50);
		hActive = MeasGetHActive( &wTemp );
		if(pTimeTable->hActive >= hActive-2
		&& pTimeTable->hActive <= hActive+2)
			break;
	}
	//print selected video
	Printf("\nUse mode:%bd", mode);	
	//PrintVesaVideoTimeTable(pTimeTable);

	//adjust Phase..
	value = GetPhaseEE(mode);
	if(value == 0xFF) {
		//No previous data. We need a AutoTunePhase.
		AutoTunePhase();
		value=aRGB_GetPhase();
		argb_dPrintf("\n\rAutoTune Phase 0x%bx",value);
		SavePhaseEE(mode,value);
	}
	else {
		argb_dPrintf("\n\ruse EE Phase 0x%bx",value);
		//we read first, because update routine can make a shaking.
		value1=aRGB_GetPhase();
		if(value != value1) {
			argb_dPrintf("  update from 0x%bx",value1);
			aRGB_SetPhase(value, 0);	//BKTODO? Why it does not have a init ?
		}
	}

	PC_SetScaler(mode);
	Input_aRGBMode = mode;
	InputSubMode = Input_aRGBMode;

	AdjustPixelClk(0, mode); //BK120117 need a divider value
	bTemp = GetPixelClkEE(mode);  //value 0..100
#if 1
	//if E3PROM does not have value, it will return 0.
	//if we receive 0, FW will subtract 50.
	//If you see the below message, you need to execute "EE default"
	if(bTemp == 0) {
		Printf("\nMaybe, something wrong!!...Please update your eeprom.");
		bTemp = 50;
	}
#endif
	if(bTemp != 50) {
		wTemp = pTimeTable->hTotal - 1;
		//Printf("\n\r!!!EEPROM has a PixelClock value %bd",bTemp);
		//Printf("\tchange LLPLL %d",wTemp);
		wTemp += bTemp;
		wTemp -= 50;
		//Printf("->%d",wTemp);
		aRGB_LLPLLUpdateDivider(wTemp, OFF, 0);	//without init.
	}

	MeasSetErrTolerance(4);						//tolerance set to 32
	MeasEnableChangedDetection(ON);				//set EN. Changed Detection
	
	//Printf("\nVESA %dx%d@%bd", pTimeTable->hActive,pTimeTable->vActive,pTimeTable->vFreq);

 	hActive = MeasGetHActive( &hCropStart );
	vActive = MeasGetVActive( &vCropStart );
	vFreq = MeasGetVFreq();
	PrintMeasValue("PC");

	/* search scaler table. If success, overwrite */
#ifdef SUPPORT_SCALER_OVERWRITE_TABLE
	pScaler = FindScalerTable(InputMain, hActive,vActive,vFreq, vCropStart,1);
	if(pScaler != NULL)
		OverWriteScalerWithTable(pScaler,1,1);
#endif
	PC_PrepareInfoString(mode);


#ifdef CHECK_USEDTIME
	SFRB_ET0 = 0;
	UsedTime = SystemClock - UsedTime;
	SFRB_ET0 = 1;
	Printf("\n\rUsedTime:%ld.%ldsec", UsedTime/100, UsedTime%100 );
#endif
			
	return ERR_SUCCESS;
	
}
#endif

#undef CHECK_USEDTIME

//-----------------------------------------------------------------------------
/**
* Change to PC
*
* @return
*	- 0: success
*	- 1: No Update happen
*	- 2: No Signal or unknown video sidnal.
*/
BYTE ChangePC( void )
{
	BYTE ret;

	if ( InputMain == INPUT_PC ) {
		dPrintf("\n\rSkip ChangePC");
		return(1);
	}

	InputMain = INPUT_PC;
	InputSubMode = 0;

	if(GetInputMainEE() != InputMain)
		SaveInputMainEE( InputMain );

	//----------------
	// initialize video input
	InitInputAsDefault();

	//
	// Check and Set aRGB,mesaure,Scaler for Analog PC input
	//
	ret = CheckAndSetPC();
	if(ret==ERR_SUCCESS) {
		//success
		VInput_enableOutput(0);
		return 0;
	}

	//------------------
	// NO SIGNAL
	// Prepare NoSignal Task...

	//free run		
	//start recover & force some test image.
	VInput_gotoFreerun(0);

	return 2;	//fail..
}
#endif


