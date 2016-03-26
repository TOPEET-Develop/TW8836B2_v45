/**
 * @file
 * scaler.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	video scaler module 
 *
 * input => (scale down) => line buff => (scale up) =>  output	panel
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

#include "Printf.h"
#include "Monitor.h"
#include "I2C.h"
#include "CPU.h"
#include "SOsd.h"
#include "FOsd.h"

#include "InputCtrl.h"
#include "aRGB.h"
#include "PC_modes.h"
#include "measure.h"
#include "settings.h"
#define INCLUDE_DTV_TABLE
#include "scaler.h"

#include "eeprom.h"
#include "util.h"

#include "decoder.h"
#include "dtv.h"
#include "main.h"
#include "BT656.h"

//=============================================================================
// Video Tables
//=============================================================================
#include "scaler_time.c"
#include "scaler_800x480.c"
#include "scaler_1024x600.c"


//---------------------
//AT080 TCON 800x480
//---------------------

/*
sspll2 & pclko divider
the panel needs a 60Hz.
This 60Hz frequency is composite with pclko,hTotal,vTotal.(pclko is an output pixel clock)
	(1/60 = hTotal * vTotal / pclko).
In TW88xx, Scaler uses an input vTotal to generate the output vTotal.
	(output_vTotal = input_vTotal * 8192 / vScaleRate).

output hTotal and pclko have a max and min value that depend on the panel vendor.
(Panel vendor calls our pclko as DCLK).

in tw88xx team, we call the system_clk as PCLK. 
It is very confuse name. So, I will call it as system_clk.

the system_clk has to be big enough to handle the scaler image
that is cropped by input_crop and can be a scaled downowned image if it needs H scaled down.

pclk = system_clk = sspll2 / pclk_div. (pclk_div has alaways 1 in my solution)
pclko = system_clk / pclko_div.

this pclko has to be bigger then PANEL_PCLK_MIN and smaller then PANEL_PCLK_MAX.

the output vTotal is a fixed value that depend on the input vTotal.
and we know, the panel has a typical hTotal value, PANEL_H_TYP.
..
..
..
*/

/**
* Find ScalerTable
* @param InputMain
* @param hActive
* @param vActive
* @param vFreq
* @param vStart
*	if vStart is 0, ignore vPol
* @param vPol
*	if vStart is 0, ignore vPol
* @return 
*	fail:NULL:fail
*	success: table location
*/	 
#ifdef SUPPORT_SCALER_OVERWRITE_TABLE
struct SCALER_TIME_TABLE_s *FindScalerTable(BYTE _InputMain,WORD hActive,WORD vActive,BYTE vFreq, BYTE vStart, BYTE vPol)
{
	struct SCALER_TIME_TABLE_s *p;	//result
	BYTE fScale;
	BYTE vDE;

	switch(_InputMain) {
	case INPUT_CVBS:
	case INPUT_SVIDEO:
		fScale = EE_Read(EEP_INPUT_DEC);
		if(fScale) 	p = SCALER_CVBS_NORMAL_time;
		else		p = SCALER_CVBS_OVERSCAN_time; 
		break;	
	case INPUT_COMP:
		fScale = EE_Read(EEP_INPUT_DEC);
		if(fScale) 	p = SCALER_COMP_NORMAL_time;
		else		p = SCALER_COMP_OVERSCAN_time; 
		break;	
	case INPUT_PC:
		p = SCALER_PC_time; 
		break;	
	default:
		p = SCALER_DTV_time; 
		break;	
	}

	Printf("\nFindScalerTable(%bd,%dx%d@%bd vStart:%bd,vPol:%bd)", _InputMain, hActive,vActive,vFreq, vStart,vPol);

	//horizontal values are not accurate.
	// FW will use +-2 value.
	//vertical values are accurate.
	// But, sometimes, external device send an incorrect video source.
	// FW will check +1 value on vFreq search.
	while(p->hActive) {
		if(p->vActive == vActive) {
			if(p->hActive  >= (hActive-2)  
			&& p->hActive <= (hActive+2)) {	
				if((p->vFreq == vFreq)
				|| ((p->vFreq+p->vClone) == vFreq)) {
					if(vStart==0) //if vStart is 0, ignore
						return p;

					vDE = p->vBPorch;
					if(vPol) vDE += p->vSyncWidth;
					if(vDE == vStart)
						return p;
				}
			}
		}		  
		p++;
	}
	Puts(" Fail");
	return NULL;	
}
#endif

/**
* Overwrite the scaler related registers by a table value.
*
* On TW8836B, FW uses this function to verify and update the incorrect value.
* If the calculated value is different with the table value, this function 
* makes a debug message and then update the register values. 
* If scaler table has 0 value, FW will not update this item.
*
* The SCALER_TIME_TABLE is on the scaler_800x480.c, or scaler_1024x600.c 
* depend on the panel resolution. and 1280x800 is not ready yet.
* The SCALER_TIME_TABLE has 4 different tables depend on the input type.
*
*	SCALER_CVBS_time[] for CVBS and SVideo input
*	SCALER_COMP_time[] for Component input
*	SCALER_PC_time[] for PC input
*	SCALER_DTV_time[] for DTV input that includes HDMI and LVDS.
*
* for BT656Loopback, FW uses BT656ENC_TIME_TABLE_s.
*
*
* @param pScaler
* 	this function does not check the pScaler.
* 	You have to assign the correct table pointer.
* @param hPol 
*   1: Negative, ActiveLow on Sync (High on Active Video Area)
*      If hPol has a negative FW has to remove SyncWidth value from the table
* @param vPol 
*   1: Negative, ActiveLow on Sync (High on Active Video Area)
*      If vPol has a negative FW has to remove SyncWidth value from the table
*/	 
#ifdef SUPPORT_SCALER_OVERWRITE_TABLE
void OverWriteScalerWithTable(struct SCALER_TIME_TABLE_s *pScaler, BYTE hPol, BYTE vPol)
{
	BYTE  bTemp,bTemp2;
	WORD  wTemp;
	WORD  hStart;
	BYTE  vStart;
	
	Printf("\nOverWriteScalerWithTable %dx%d@%bd",
		pScaler->hActive, pScaler->vActive, pScaler->vFreq);
#ifdef PANEL_AUO_B133EW01
	Puts("-->skip");	
	return;
#endif
		 
	if(pScaler->vScale) {
		wTemp = ScalerReadVScaleReg();
		if(wTemp != pScaler->vScale) {
			//Printf("\n  vScale:%x->%x", wTemp,pScaler->vScale);
			ScalerWriteVScaleReg(pScaler->vScale);
		} 
	}
	if(pScaler->hBuff) {
		wTemp = ScalerGetLineBufferSize();
		if(wTemp != pScaler->hBuff) {
	 		//Printf("\n  hBuff:%d->%d", wTemp,pScaler->hBuff); 
			ScalerSetLineBufferSize(pScaler->hBuff);
		}
	}
	if(pScaler->hScale) {
		if(pScaler->hActive > PANEL_H) {
#ifdef DEBUG_SCALER_OVERWRITE_TABLE
			wTemp = ScalerReadXDownReg();
			if(wTemp != pScaler->hScale)
				Printf("\n  hScaleDn:%x->%x",wTemp,pScaler->hScale);
#endif
			ScalerWriteXDownReg(pScaler->hScale);
			ScalerWriteXUpReg(0x2000);
		} 
		else {
#ifdef DEBUG_SCALER_OVERWRITE_TABLE
			wTemp = ScalerReadXUpReg();
			if(wTemp != pScaler->hScale)
				Printf("\n  hScaleUp:%x->%x",wTemp,pScaler->hScale);
#endif
			ScalerWriteXDownReg(0x0400);
			ScalerWriteXUpReg(pScaler->hScale);
		}
	}

	// input crop
	if(InputMain==INPUT_CVBS || InputMain==INPUT_SVIDEO) {
#ifdef DEBUG_SCALER_OVERWRITE_TABLE
		wTemp = DecoderGet_vDelay();
		if(wTemp != pScaler->vStart)
			Printf("\n  vStart:%d->%bd", wTemp,pScaler->vStart);
		wTemp = DecoderGet_vActive();
		if(wTemp != pScaler->vLen)
			Printf("\n  vLen:%d->%d", wTemp,pScaler->vLen); 

		wTemp = DecoderGet_hDelay();
		if(wTemp != pScaler->hStart)
			Printf("\n  hStart:%d->%d", wTemp,pScaler->hStart); 
		wTemp = DecoderGet_hActive();
		if(wTemp != pScaler->hLen)
			Printf("\n  hLen:%d->%d", wTemp,pScaler->hLen); 
#endif
		DecoderSetOutputCrop(pScaler->hStart,pScaler->hLen,pScaler->vStart,pScaler->vLen);
	}
	else {
		hStart = pScaler->hStart;
		if(hPol==0) hStart -= pScaler->hSync;
		vStart = pScaler->vStart;
		if(vPol==0) vStart -= pScaler->vSync;

#ifdef DEBUG_SCALER_OVERWRITE_TABLE
		bTemp = InputGetVStart();
		if(bTemp != vStart)
			Printf("\n  vStart:%bd->%bd", bTemp,vStart); 
		wTemp = InputGetVLen();
		if(wTemp != pScaler->vLen)
			Printf("\n  vLen:%d->%d", wTemp,pScaler->vLen); 
		wTemp = InputGetHStart();
		if(wTemp != hStart)
			Printf("\n  hStart:%d->%d", wTemp,hStart); 
		wTemp = InputGetHLen();
		if(wTemp != pScaler->hLen)
			Printf("\n  hLen:%d->%d", wTemp,pScaler->hLen); 
#endif
		InputSetCrop(hStart,vStart,pScaler->hLen,pScaler->vLen);
	}
	if( pScaler->vDE) {
		wTemp = ScalerReadVDEReg();
		if(wTemp != pScaler->vDE) {
			Printf("\n  vDE:%d->%d", wTemp,pScaler->vDE); 
			ScalerWriteVDEReg(pScaler->vDE);
		}
	}

	//V,H,F,C,P
	InputSetPolarity(pScaler->pol &POL_S_V, pScaler->pol&POL_S_H, pScaler->pol&POL_S_F);	//V,H,F
	InputSetClockPolarity(pScaler->pol&POL_S_C);									//C
#if 1 //Errata150407
	if(InputMain==INPUT_LVDS)
		InputSetClockPolarity(1);
#endif

	if(pScaler->sspll2) {
		Sspll2SetFreq(pScaler->sspll2*10000L, ON);
		PclkSetDividerReg(pScaler->div >> 4);	//pclk divider  [7:6] =>[4:4][2:0]
		PclkoSetDiv_with_pol(pScaler->div & 0x07);		//pclko divider [2:0] =>[5:5][1:0]			
	}
#if 1 //only for debugging
	bTemp2=CheckPclko_polarity();
	bTemp = ReadTW88(REG20D) & 0x10;
	bTemp >>=4;
	if(bTemp != bTemp2) { 
		Printf("\npclko %bd->%bd",bTemp, bTemp2);
		PclkoSetPolarity(bTemp2);
	}
#endif
}
#endif

//====================
// BT656Loopback
//====================

struct BT656ENC_TIME_TABLE_s *G_pBt656Scaler;


/**
* Overwrite the BT656 Encoder and scaler related registers by a table value.
*
* @param step
*	0 - called on BT656Enc setup procedure.
*		It updates the BT656Enc related registers
*	1 - called between BT656Enc and BT656Dec loopback procedure.
*		It updates DTV related registers.
*	2 - called on BT656Dec procedure.
*		It will updates the scaler related registers
*
* @see OverWriteScalerWithTable
*/
#if defined(SUPPORT_BT656)
void OverWriteBt656ScalerTable(struct BT656ENC_TIME_TABLE_s *pBt656Scaler, BYTE step)
{
	BYTE  bTemp;
	WORD  wTemp;
	
	Printf("\nOverWriteBt656ScalerTable(%bd)",step);
#ifdef PANEL_AUO_B133EW01
	//i don't have table yet.
	Puts("-->skip");	
	return;
#endif		 

	if(step==0) {
		BT656Enc_Crop(pBt656Scaler->i_hStart, pBt656Scaler->i_vStart, pBt656Scaler->i_hActive,pBt656Scaler->i_vActive);
		BT656Enc_SetVSyncDelay(pBt656Scaler->i_vSyncDelay);
		BT656Enc_SetScaler(pBt656Scaler->i_hScale,pBt656Scaler->i_vScale);
		BT656Enc_SetOutputActiveLen(pBt656Scaler->o_hStart,pBt656Scaler->o_hActive);

		BT656Enc_GenLock(pBt656Scaler->i_genlock & 0x0F, pBt656Scaler->i_genlock >> 4); //PreDiv:4,x4
		BT656Enc_SetOutputClkPol(pBt656Scaler->pol & POL_VDCLK);

		if(InputMain==INPUT_CVBS || InputMain==INPUT_SVIDEO) {
			//pBt656Scaler does not have the decoder output crop value.
			//FW updates it depend on NTSC or PAL.
			bTemp = MeasGetVFreq();
#if	(PANEL_H==800 && PANEL_V==480)
			if(bTemp==50) { 
				Printf("\n720x288@50");
				DecoderSetOutputCrop(7,720,24,288);
			}
			else {
				Printf("\n720x240@%bd",bTemp);
				DecoderSetOutputCrop(10,720,21,240);
			}
#else
			if(bTemp==50) { 
				Printf("\n720x288@50");
				DecoderSetOutputCrop(10,720,24,288);
			}
			else {
				Printf("\n720x240@%bd",bTemp);
				DecoderSetOutputCrop(10,720,21,240);
			}
#endif
		}

	}
	else if(step==1) {
		//dtv clock pol.
		Bt656DecSetClkPol(pBt656Scaler->pol & POL_DTVCLK);
	}
	else {	//(step==2)

		if(pBt656Scaler->s_vScale) {
			wTemp = ScalerReadVScaleReg();
			if(wTemp != pBt656Scaler->s_vScale) {
				Printf("\n  vScale:%x->%x", wTemp,pBt656Scaler->s_vScale);
				ScalerWriteVScaleReg(pBt656Scaler->s_vScale);
			}
		}
		wTemp = ScalerGetLineBufferSize();
		if(wTemp != pBt656Scaler->s_hBuff) {
	 		Printf("\n  hBuff:%d->%d", wTemp,pBt656Scaler->s_hBuff); 
			ScalerSetLineBufferSize(pBt656Scaler->s_hBuff);
		}

		if(pBt656Scaler->s_hScale) {	   
			wTemp = pBt656Scaler->s_hLen / 2;
			wTemp -= 3;
			if(wTemp > PANEL_H) {
				wTemp = ScalerReadXDownReg();
				if(wTemp != pBt656Scaler->s_hScale) {
					Printf("\n  hScaleDn:%x->%x",wTemp,pBt656Scaler->s_hScale);
					ScalerSetHScaleReg(pBt656Scaler->s_hScale,0x2000);
				}
			} 
			else {
				wTemp = ScalerReadXUpReg();
				if(wTemp != pBt656Scaler->s_hScale) {
					Printf("\n  hScaleUp:%x->%x",wTemp,pBt656Scaler->s_hScale);
					ScalerSetHScaleReg(0x0400, pBt656Scaler->s_hScale);
				}
			}
		}
//		bTemp = InputGetVStart();
//		if(bTemp != pBt656Scaler->s_vStart)
//			Printf("\n  vStart:%bd->%bd", bTemp,pBt656Scaler->s_vStart); 
//		wTemp = InputGetVLen();
//		if(wTemp != pBt656Scaler->s_vLen)
//			Printf("\n  vLen:%d->%d", wTemp,pBt656Scaler->s_vLen);			 		
//		wTemp = InputGetHStart();
//		if(wTemp != pBt656Scaler->s_hStart)
//			Printf("\n  hStart:%d->%d", wTemp,pBt656Scaler->s_hStart); 
//		wTemp = InputGetHLen();
//		if(wTemp != pBt656Scaler->s_hLen)
//			Printf("\n  hLen:%d->%d", wTemp,pBt656Scaler->s_hLen); 
		//wTemp = ScalerGetLineBufferSize();
		//if(wTemp != pBt656Scaler->s_hBuff)
	 	//	Printf("\n  hBuff:%d->%d", wTemp,pBt656Scaler->s_hBuff); 
		//wTemp = ScalerReadVDEReg();
		//if(wTemp != pBt656Scaler->s_vDE)
		//	Printf("\n  vDE:%d->%d", wTemp,pBt656Scaler->s_vDE); 
		InputSetCrop(pBt656Scaler->s_hStart,pBt656Scaler->s_vStart,pBt656Scaler->s_hLen,pBt656Scaler->s_vLen);

		wTemp = ScalerReadVDEReg();
		if(wTemp != pBt656Scaler->s_vDE) {
			Printf("\n  vDE:%d->%d", wTemp,pBt656Scaler->s_vDE); 
			ScalerWriteVDEReg(pBt656Scaler->s_vDE);
		}

		//V,H,F,C,P
		InputSetPolarity(pBt656Scaler->pol & POL_S_V, pBt656Scaler->pol&POL_S_H, pBt656Scaler->pol&POL_S_F);	//V,H,F
		InputSetClockPolarity(pBt656Scaler->pol&POL_S_C);													//C
	
		if(pBt656Scaler->sspll2) {
			Sspll2SetFreq(pBt656Scaler->sspll2*10000L, ON);
			//BUGGY_PclkSetFunc(pBt656Scaler->sspll2);
			PclkSetDividerReg(pBt656Scaler->div >> 4);	//pclk divider  [7:6] =>[4:4][2:0]
			//PclkoSetDiv(pBt656Scaler->div & 0x07);		//pclko divider [2:0] => [5:5][1:0]			
			//PclkoSetPolarity(pBt656Scaler->div & POL_PCLKO);															//P
			PclkoSetDiv_with_pol(pBt656Scaler->div & 0x07);
		}
	}
}
#endif

BYTE VideoAspect;

#define XSCALE_DN_BASE	0x0400L
#define XSCALE_UP_BASE	0x2000L	
#define YSCALE_BASE		0x2000L
#define PXSCALE_BASE	0x0800L

#define SCALER_HDE_BASE				0x30
#define SCALER_HDELAY2_BASE			0x10
#define SCALER_MAX_LINEBUFF_SIZE	1366

/**
* print routines for TIME_TABLE
*/
void PrintCbvsVideoTimeTable(struct DEC_VIDEO_TIME_TABLE_s *p)
{
	Printf("\nCVBS %dx%d@%bd pol:%02bx  h:%bd  v:%bd",
		p->hActive,p->vActive,
		p->vFreq,
		p->pol,
		p->hDelay,
		p->vDelay
	);
}
#if defined(SUPPORT_ARGB)
void PrintCompVideoTimeTable(struct COMP_VIDEO_TIME_TABLE_s *p)
{
	Printf("\nCOMP %dx%d@%bd pol:%02bx  %d:%d,%d  %d:%bd,%bd",
		p->hActive,p->vActive,p->vFreq,p->pol,
		p->hTotal,p->hSync,p->hBPorch,
		p->vTotal,p->vSync,p->vBPorch
	);
}

void PrintVesaVideoTimeTable(struct DIGIT_VIDEO_TIME_TABLE_s *p)
{
	Printf("\nVESA %dx%d@%bd pol:%02bx  %d:%d,%d,%d  %d:%bd,%bd,%bd",
		p->hActive,p->vActive,p->vFreq,p->pol,
		p->hTotal,p->hFPorch,p->hSync, p->hBPorch,
		p->vTotal,p->vFPorch,p->vSync,p->vBPorch
	);
}
#endif

void PrintCEAVideoTimeTable(struct DIGIT_VIDEO_TIME_TABLE_s *p)
{
	if(p==NULL) {
		Puts("\nInvalid table");
		return;
	}
	Printf("\nCEA  %dx%d@%bd pol:%02bx",
		p->hActive,p->vActive,p->vFreq,p->pol);

	Printf(" %s", p->pol & INTERLACED ? "I" : "P");
	Printf("_%s", p->pol & HPOL_P ? "HP" : "HN");
	Printf("_%s", p->pol & VPOL_P ? "VP" : "VN");
	Printf("  %d:%d,%d,%d  %d:%bd,%bd,%bd",
		p->hTotal,p->hFPorch,p->hSync, p->hBPorch,
		p->vTotal,p->vFPorch,p->vSync, p->vBPorch
	);
}

/**
* print current measured result
*/
void PrintMeasValue(char *who)
{
	WORD hActive,hSync,hStart;
	WORD vStart,vSync,vActive,vTotal;
	BYTE vFreq;

	vActive = MeasGetVActive( &vStart );
	hActive = MeasGetHActive( &hStart );
	vFreq = MeasGetVFreq();
	hSync=MeasGetHSyncRiseToFallWidth();
	vTotal = MeasGetVPeriod();
	vSync=MeasGetVSyncRiseToFallWidth();

	Printf("\n%s Meas %dx%d@%bd hSync:%d hStart:%d vTotal:%d vSync:%d vStart:%d",
		who,
		hActive,vActive,vFreq,
		hSync,hStart,
		vTotal,vSync,vStart);
}
void PrintMeasAdjValue(char *who, WORD _hStart,WORD _vStart)
{
	WORD hActive,hStart;
	WORD vStart,vActive;

	vActive = MeasGetVActive( &vStart );
	hActive = MeasGetHActive( &hStart );
	Printf("\n%s Adj  hStart:%d->%d vStart:%d->%d",
		who,
		hStart,_hStart,
		vStart,_vStart);
}


/**
*
* vStart is a 8bit, but I am using U16.
*/
#ifdef DEBUG_BT656
void PrintScalerInputCrop(char *who,WORD hStart,WORD vStart,WORD hActive,WORD vActive)
{
	Printf("\n%s ICrop ",who);
	Printf("hStart:%d hActive:%d  vStart:%d vActive:%d",
		hStart,hActive,
		vStart,vActive);
}
#endif
#ifdef DEBUG_BT656
void PrintDecoderOutputCrop(char *who, WORD hDelay, WORD hActive, WORD vDelay, WORD vActive)
{
	Printf("\n%s DecCrop ",who);
	Printf("hDelay:%d hActive:%d  vDelay:%d vActive:%d",
		hDelay,hActive,
		vDelay,vActive);
}
#endif

/**
* find CEA861 Video time table
*/
struct DIGIT_VIDEO_TIME_TABLE_s *Find_CEA861_VideoTable(WORD hActive,WORD vActive,BYTE vFreq)
{
	struct DIGIT_VIDEO_TIME_TABLE_s *p;

	p = CEA861_table;
	while(p->hActive != 0xFFFF) {
		if(p->hActive==hActive || p->hActive==(hActive+1)) {
			if(p->vActive==vActive) {
				if(p->vFreq == vFreq || p->vFreq == (vFreq+1)) {
#ifdef DEBUG_BT656
					Printf("\nFind CEA861 video %dx%d%s@%02bd",p->hActive,p->vActive, p->pol & INTERLACED ? "I":"P",p->vFreq);
					Printf("  %d:%d,%d,%d  %d:%bd,%bd,%bd",
						p->hTotal,p->hFPorch, p->hSync, (WORD)p->hBPorch,  
						p->vTotal,p->vFPorch, p->vSync, p->vBPorch
						);
#endif
					return p; 
				} 
			}
		}
		p++;
	}
	//fail.., try VESA
	p = TW8836_VESA_TABLE;
	while(p->hActive != 0xFFFF) {
		if(p->hActive==hActive || p->hActive==(hActive+1)) {
			if(p->vActive==vActive) {
				if(p->vFreq == vFreq || p->vFreq == (vFreq+1)) {
#ifdef DEBUG_BT656
					Printf("\nFind VESA video %dx%d%s@%02bd",p->hActive,p->vActive, p->pol & INTERLACED ? "I":"P",p->vFreq);
					Printf("  %d:%d,%d,%d  %d:%bd,%bd,%bd",
						p->hTotal,p->hFPorch, p->hSync, (WORD)p->hBPorch,  
						p->vTotal,p->vFPorch, p->vSync, p->vBPorch
						);
#endif
					return p; 
				} 
			}
		}
		p++;
	}
	//fail
	Printf("\nFind_CEA861_VideoTable fail %dx%d@%bd", hActive,vActive,vFreq);

	return NULL;
}

//=============================================================================
// Register Functions
//=============================================================================

//static function prototypes


#ifdef UNCALLED_SEGMENT
//-----------------------------------------------------------------------------
/**
* LNFIX		R201[2]		on/off
*			1 = Fix the scaler output line number defined by register LNTT.
*			0 = Output line number determined by scaling factor.
* LNTT		R20D[7:6]R219[7:0]	lines
*			It controls the scaler total output lines when LNFIX=1. 
*			It is used in special case. A 10-bit register.
*other name: Limit V Total
*/
//void ScalerSetOutputFixedVline(BYTE onoff)
//{
//	if(onoff)	WriteTW88(REG201, ReadTW88(REG201) | 0x04);
//	else		WriteTW88(REG201, ReadTW88(REG201) & 0xFB);
//}
#endif

#ifdef UNCALLED_SEGMENT
//-----------------------------------------------------------------------------
/**
* register
*	R202[5:0]
*/
//void ScalerSetFieldOffset(BYTE fieldOffset)
//{
//	WriteTW88(REG202, (ReadTW88(REG202) & 0xC0) | filedOffset);
//}
#endif

//-------------------------------------
//Scaler scale ratio
//-------------------------------------

//-----------------------------------------------------------------------------
/**
* description
* write Horizontal Up Scale register
*
* Up scaling ratio control in X-direction.\n 
* A 16-bit register.\n 
* The scaling ratio is defined as 2000h / XSCALE\n
*
*	register
*	R204[7:0]R203[7:0]	XSCALE UP
*/
void ScalerWriteXUpReg(WORD value)
{
	WriteTW88(REG204, (BYTE)(value>>8));		
	WriteTW88(REG203, (BYTE)value);
}
//-----------------------------------------------------------------------------
/**
* description
* write Horizontal Down Scale register
*
* Down scaling ratio control in X-direction. 
* A 12-bit register. 
* The down scaling ratio is defined as 400h / HDSCALE
*	register
*	R204[7:0]R203[7:0]	XSCALE UP
*	R20A[3:0]R209[7:0]	X-Down
*/
void ScalerWriteXDownReg(WORD value)
{
	WriteTW88(REG20A, (ReadTW88(REG20A) & 0xF0) | (value >> 8));
	WriteTW88(REG209, (BYTE)value);
}

//-----------------------------------------------------------------------------
/**
* description
* Horizontal Down/Up Scale register.
* Scaler do downscale first, and then do upscale.
*
* The down scaling ratio is defined as 400h / DOWN_SCALE.
*	register
*	R20A[3:0]R209[7:0]	X-Down. 12bit
*
* The up scaling ratio is defined as 2000h / UP_SCALE.
*	register
*	R204[7:0]R203[7:0]	XSCALE UP. 16bit
*/
void ScalerSetHScaleReg(WORD down, WORD up)
{
	WriteTW88(REG20A, (ReadTW88(REG20A) & 0xF0) | (down >> 8));
	WriteTW88(REG209, (BYTE)down);

	WriteTW88(REG204, (BYTE)(up>>8));		
	WriteTW88(REG203, (BYTE)up);
}

//-----------------------------------------------------------------------------
/**
* description
* read Horizontal Up Scale register
*
*/
WORD ScalerReadXUpReg(void)
{
	WORD wValue;
	Read2TW88(REG204,REG203,wValue);
	return wValue;
}


#if defined(SUPPORT_PC) || defined(SUPPORT_DVI) || defined(SUPPORT_HDMI)
//-----------------------------------------------------------------------------
/**
* description
* read Horizontal Down Scale register
*
* @see ScalerWriteXDownReg
*/
WORD ScalerReadXDownReg(void)
{
	WORD HDown;

	HDown = ReadTW88(REG20A ) & 0x0F;
	HDown <<= 8;
	HDown += ReadTW88(REG209 );
	return HDown;
}
#endif



//-----------------------------------------------------------------------------
//internal
/**
* set Horizontal Scaler value on FULL mode
*
* @see ScalerSetHScale
*/
void ScalerSetHScale_FULL(WORD Length)	
{
	DWORD	dTemp;

	//BK130131. Now HDE need some min value, because FOSD DE.
	//LineBuffDelay also related with HDE.
	//I temporary increase it.

	ScalerWriteLineBufferDelay(SCALER_HDELAY2_BASE);
	ScalerPanoramaOnOff(OFF);

	//if SCALER_MAX_LINEBUFF_SIZE < PANEL_H, we have to use SCALER_MAX_LINEBUFF_SIZE.
	if(PANEL_H >= Length) { 
		//-----------------------								
		//UP SCALE
		//I don't want to add a garbage on the right side.
		//use a round down value, so image will be scaled up and cut on the right side.
		//for example
		//	input:720	output:800
		//
		//	720*8192/hScale=800
		//	hScale = 720*8192/800
		//	hScale = 7372.8
		//	So, we will use 7373 for hScale.
		//
		//	720*8192/7372=800.08681
		//--------------------
		dTemp = Length;
		dTemp <<= 13;	/* *0x2000L */
		dTemp /= PANEL_H;
		ScalerSetHScaleReg(0x0400, dTemp);
		ScalerSetLineBufferSize(Length);
	}
	else {
		//-----------------------								
		//DOWN SCALE
		//for example
		//	input:1920, output:800
		//
		//	1920*1024/800 = 2457.6
		//	So, I will use 2457 for hScale.
		//
		//	1920*1024/2457=800.19536
		//--------------------------
		dTemp = Length;
		dTemp <<= 10;	/* *0x400L*/
		dTemp /= PANEL_H;

		ScalerSetHScaleReg((WORD)dTemp, 0x2000);
		ScalerSetLineBufferSize(PANEL_H);
	}
}

//-----------------------------------------------------------------------------
//internal
/**
* set Horizontal Scaler value on Panorama mode
*
* @see ScalerSetHScale
*/
#if 0
//void ScalerSetHScale_Panorama(WORD Length)	
//{
//	DWORD	temp;
//	WORD	X1;
//	WORD 	linebuff;
//
//	X1 = Length;
//	X1 += 32;
//
//	if(PANEL_H >= X1) {
//		//
//		//UP SCALE
//		//
//		X1 = Length;
//		X1 += 34;				//32+2
//		linebuff = Length+1;
//
//		//dPrintf("\n\rScalerSetHScale(%d->%d) ",Length,X1); 
//
//		temp = X1 * 0x2000L;					//8192
//		temp /= PANEL_H;						//800
//		ScalerSetHScaleReg(0x0400, temp);
//		//dPrintf("DN:0x0400 UP:0x%04lx lbuff:%d", temp,linebuff);
//		ScalerSetLineBufferSize(linebuff);
//	}
//	else {
//		//
//		// DOWN SCALE
//		//
//		linebuff = PANEL_H - 34*2;				//(32+2)*2
//		temp = Length * 0x0400L;				//1024						
//		temp /= (linebuff - 1);					//target 800->731
//		ScalerSetHScaleReg(temp, 0x2000);
//		//dPrintf("DN:0x%04lx UP:0x2000 lbuff:%d", temp,linebuff);
//		ScalerSetLineBufferSize(linebuff+1);
//	}
//	ScalerSetPanorama(0x400,0x20);
//	ScalerPanoramaOnOff(ON);
//}
#endif

//-----------------------------------------------------------------------------
/**
* set Horizontal Scaler value with ratio
*
*/
//#if 0
//void ScalerSetHScaleWithRatio(WORD Length, WORD ratio)	
//{
//	DWORD temp;
//	WORD new_Length;
//	//dPrintf("\n\rScalerSetHScaleWithRatio(%d,%d)",Length,ratio);
//	ScalerWriteLineBufferDelay(SCALER_HDELAY2_BASE);
//	ScalerPanoramaOnOff(OFF);
//
//  
//	temp = Length;
//	temp *= ratio;
//	temp /= 100;	//new length
//	new_Length = temp;
//
//	//dPrintf("\n\rHLength %d->%d", Length, new_Length);
//	if(ratio < 100) {
//		//down scale
//		temp = 0x0400L;
//		temp *= ratio;
//		temp /= 100;
//		ScalerSetHScaleReg(temp, 0x2000);
//	}
//	else {
//		//upscale
//		temp = 0x2000L;
//		temp *= ratio;
//		temp /= 100;
//		ScalerSetHScaleReg(0x400, temp);
//	}
//	if(new_Length < PANEL_H) {
//		//adjust buffer output delay
//		ScalerWriteLineBufferDelay((PANEL_H - new_Length) / 2 +SCALER_HDELAY2_BASE);
//	}
//}
//#endif

//-----------------------------------------------------------------------------
/**
* set Horizontal Scaler value
*
* @see ScalerSetHScale_FULL
* @see ScalerSetHScale_Panorama
*/
void ScalerSetHScale(WORD Length)	
{
//	VideoAspect = GetAspectModeEE();

	//dPrintf("\n\rScalerSetHScale(%d)",Length);
	//dPrintf("\n\rVideoAspect:%bd",VideoAspect);

//#if 0
//	if((InputMain==INPUT_CVBS || InputMain==INPUT_SVIDEO)
//	&& VideoAspect == VIDEO_ASPECT_NORMAL) {
//
//		ScalerWriteLineBufferDelay(SCALER_HDELAY2_BASE);	//BK110916 test. Normal needs it
//		ScalerPanoramaOnOff(OFF);
//
//
//		//only at CVBS
//		ScalerSetHScaleReg(0x400, 0x2000);	//clear down,up scale
//		//adjust buffer output delay
//		ScalerWriteLineBufferDelay((PANEL_H - Length) / 2 +SCALER_HDELAY2_BASE);
//
//		ScalerSetLineBufferSize(Length); //BK120111
//	}
//	else if(VideoAspect == VIDEO_ASPECT_PANO)
//		ScalerSetHScale_Panorama(Length);
//	else 
//		//	 VideoAspect == VIDEO_ASPECT_FULL
//		//of VideoAspect == VIDEO_ASPECT_ZOOM
//#endif
		ScalerSetHScale_FULL(Length);
}


//YSCALE
//-----------------------------------------------------------------------------
/**
* Up / down scaling ratio control in Y-direction. 
*
* The scaling ratio is defined as 2000h / YSCALE.
* A 16-bit register. 
*
*	register
*	R206[7:0]R205[7:0]	YSCALE	
*/
void ScalerWriteVScaleReg(WORD value)
{
	WriteTW88(REG206, (BYTE)(value>>8));
	WriteTW88(REG205, (BYTE)value);
}

//-----------------------------------------------------------------------------
/**
* read Vertical Scale register value
*
* @see ScalerWriteVScaleReg
*/
WORD ScalerReadVScaleReg(void)
{
	WORD VScale;
	Read2TW88(REG206,REG205, VScale);
	return VScale;
}

//-----------------------------------------------------------------------------
/**
* set Vertical Scale with Ratio
*
*/
//#if 0
//void ScalerSetVScaleWithRatio(WORD Length, WORD ratio)
//{
//	DWORD temp;
//	WORD new_Length;
//
//	temp = Length;
//	temp *= ratio;
//	temp /= 100;	//new length
//	new_Length = temp - Length;			//offset.
//	new_Length = Length - new_Length;	//final 
//	//dPrintf("\n\rVLength %d->%d", Length, new_Length);
//
//	temp = new_Length * 0x2000L;
//	temp /= PANEL_V;
//	ScalerWriteVScaleReg(temp);
//}
//#endif

//-----------------------------------------------------------------------------
/**
* set Vertical Scale
*
* output_length = input_length * 0x2000 / YSCALE_REG
* YSCALE_REG = input_length * 0x2000 / output_length
* YSCALE_REG = input_length * 0x2000 / PANEL_V
* YSCALE_REG = (input_length * 0x2000 + r) / PANEL_V. r = PANEL_V/2
*/
void ScalerSetVScale(WORD Length)
{
	//----------------
	//output = input * 8192 / PANEL_V
	//----------------
	DWORD dTemp;
#if (PANEL_V==480)
	WORD wDiv;
#endif

#if (PANEL_V==480)
	//if we need scale down, I will use 479.50 not a 480.
	if(Length > PANEL_V) {
		dTemp = Length;
		dTemp *= 8192L;
		dTemp <<=1;
		wDiv = PANEL_V;
		wDiv <<=1;
		wDiv -= 1;
		dTemp /= wDiv;
	}
	else {
		dTemp = Length;
		dTemp *= 8192L;
		dTemp /= PANEL_V;
	}
#else
	dTemp = Length;
	dTemp <<= 13; //*= 8192L;
	dTemp /= PANEL_V;

#endif
	ScalerWriteVScaleReg((WORD)dTemp);
}

//-------------------------------------
//Scaler Panorama
//-------------------------------------

//-----------------------------------------------------------------------------
/**
* set Panorama mode
*
*	register
*	R201[6]	Enable Panorama/waterglass display
*/
void ScalerPanoramaOnOff(BYTE fOn)
{
	if(fOn)	WriteTW88(REG201, ReadTW88(REG201) | 0x40);
	else    WriteTW88(REG201, ReadTW88(REG201) & ~0x40);
}

//-----------------------------------------------------------------------------
/**
* set the panorama parameters
*
* @param px_scale
* @px_inc 
*
*	register
*	R207[7:0]R212[7:4]	PXSCALE.
*	R217[7:4]R208[7:0]	PXINC.	2's complement.
*/
#if 0
void ScalerSetPanorama(WORD px_scale, short px_inc)
{
	BYTE temp;
	WriteTW88(REG207, px_scale >> 4);
	WriteTW88(REG212, (ReadTW88(REG212) & 0x0F) | (BYTE)(px_scale & 0x0F));
	temp = (BYTE)(px_inc >> 8);
	temp <<= 4;
	WriteTW88(REG217, (ReadTW88(REG217) & 0x0F) | temp);
	WriteTW88(REG208, (BYTE)px_inc);
}
#endif

//-------------------------------------
//	Scaler LineBuffer
//-------------------------------------
//Output Buffer Delay R20B[7;0]
//Output Buffer Length R20E[6:4]R20C[7:0]
//See the Horizontal Timing on "AN-TW8832,33 Scaler & TCON".
//set the "Output Delay" and "Output Length" of the Line Buffer Output on Horizontal Timming Flow.

//-----------------------------------------------------------------------------
/**
* Write scaler LineBuffer output delay
*
* HDE is related with this delay value.
*
*	register
*	R20B
*	R20B[7:0]			HDELAY2	
*		Scaler buffer data output delay in number of pixels in relation to the H sync.
*/
void ScalerWriteLineBufferDelay(BYTE delay)
{
	WriteTW88(REG20B, delay);
}
//-----------------------------------------------------------------------------
/**
* Read scaler LineBuffer output delay
*
* @see ScalerWriteLineBufferDelay
*/
BYTE ScalerReadLineBufferDelay(void)
{
	return ReadTW88(REG20B);
}

//-----------------------------------------------------------------------------
/**
* set Scaler OutputLength that is related with the line buffer size. 
*
* max 1024.
* 
*	register
*	R20E[6:4]:R20C[7:0]		HACTIVE
*		Scaler data output length in number of pixels. 
*		A 10-bit register.==>11
*/
void ScalerSetLineBufferSize(WORD len)
{
	//TW8836B uses +1 or +2 values.
	if(len> (PANEL_H +2))
		len=PANEL_H;

	WriteTW88(REG20E, (ReadTW88(REG20E) & 0x8F) | ((len & 0x700) >> 4));
	WriteTW88(REG20C, (BYTE)len);
}
WORD ScalerGetLineBufferSize(void)
{
	WORD wTemp;
	wTemp = ReadTW88(REG20E) & 0x70;
	wTemp <<=4;
	wTemp |= ReadTW88(REG20C);
	return wTemp;
}

//#ifdef UNCALLED_SEGMENT
////-----------------------------------------------------------------------------
//void ScalerSetLineBuffer(BYTE delay, WORD len)
//{
//	ScalerWriteLineBufferDelay(delay);
//	ScalerSetLineBufferSize(len);
//}
//#endif

//only for LCOS
//#if 0
////-----------------------------------------------------------------------------
//void ScalerSetFPHSOutputPolarity(BYTE fInvert)
//{
//	if(fInvert) WriteTW88(REG20D, ReadTW88(REG20D) |  0x04);
//	else		WriteTW88(REG20D, ReadTW88(REG20D) & ~0x04);
//}
//#endif

//HPADJ		R20E[3:0]R20F[7:0]
//			Blanking H period adjustment. A 12-bit 2's complement register
//#ifdef UNCALLED_SEGMENT
////-----------------------------------------------------------------------------
//void ScalerWriteOutputHBlank(WORD length)
//{
//	WriteTW88(REG20E, (ReadTW88(REG20E) & 0xF0) | (length >> 8) );
//	WriteTW88(REG20F, (BYTE)length);
//}
//#endif


//-----------------------------------------------------------------------------
/**
* set Horizontal DE position(DEstart) & length(active, DEwidth).
*
*	REG221[5:4]R210[7:0]	HA_POS
*		Output DE position control relative to the internal reference in number of output clock
* set the "DEstart" and "DEwidth" on Horizontal Timming Flow.
*
* This register is related with HDELAY2.
* We have to control it together.
*/
void ScalerWriteHDEReg(WORD pos)
{
	//FYI: DO NOT add debugmsg, it will makes a blink.
	//dPrintf("\n\rScalerWriteHDEReg pos:%bd",pos);
	WriteTW88(REG221, (ReadTW88(REG221) & 0xCF) | ((pos >> 4) & 0x30));
	WriteTW88(REG210, (BYTE)pos);
}
//-----------------------------------------------------------------------------
/**
* read HDE register
*
* @see ScalerWriteHDEReg
*/
WORD ScalerReadHDEReg(void)
{
	WORD wTemp;
	wTemp = ReadTW88(REG221) & 0x30; wTemp <<= 4;
	wTemp |= ReadTW88(REG210);
	return wTemp;
}

//-----------------------------------------------------------------------------
/**
* Calculate HDE value.
*
*	method
*	Buffer_Delay = REG(0x20b[7:0])
*	result = Buffer_Delay + 32
*/
WORD ScalerCalcHDE(void)
{
	WORD wTemp;
	wTemp = ScalerReadLineBufferDelay();
	return wTemp+32;
}


//-----------------------------------------------------------------------------
/**
* Read output witdh
*
*register
*	R212[3:0]R211[7:0]	HA_LEN
*						Output DE length control in number of the output clocks. 
*						A 12-bit register
*						output height. normally PANEL_V
*/

WORD ScalerReadOutputWidth(void)
{
	WORD HActive;
	HActive = ReadTW88(REG212) & 0x0F;
	HActive <<= 8;
	HActive |= ReadTW88(REG211);
	return HActive;
}

//set Scaler.Output.Width
//#ifdef UNCALLED_SEGMENT
////-----------------------------------------------------------------------------
//void ScalerWriteOutputWidth(WORD width)
//{
//	WriteTW88(REG212, (ReadTW88(REG212) & 0xF0) | (BYTE)(width>>8));
//	WriteTW88(REG211, (BYTE)width);
//}
//#endif

//HS_POS	R221[1:0]R213[7:0]	HSynch pos
//			Output H sync position relative to internal reference in number of output clocks.
//HS_LEN	R214[3:0]	HSynch width
//			Output H sync length in number of output clocks
//HSstart 
//HSwidth
//set the "HSstart" and "HSwidth" on Horizontal Timming Flow.
// Scaler set output HSynch
#ifdef UNCALLED_SEGMENT
//-----------------------------------------------------------------------------
void ScalerSetHSyncPosLen(WORD pos, BYTE len)
{
	WriteTW88(REG213, (BYTE)pos);
	WriteTW88(REG221, (ReadTW88(REG221) & 0xFC) | (pos>>8));
	WriteTW88(REG214, (ReadTW88(REG214) & 0xF0) | len);

}
#endif


//-----------------------------------------------------------------------------
//VA_POS	REG220[5:4]R215[7:0]		VDE pos 
//			Output DE position control relative to the internal reference in number of output lines
//VA_LEN	R217[3:0]R216[7:0]	  width
//			Output DE control in number of the output lines. A 12-bit register
/**
* Read Vertical DE register
*/
WORD ScalerReadVDEReg(void)
{
	WORD wTemp;
	wTemp = ReadTW88(REG220) & 0x30;	wTemp <<= 4;
	wTemp |= ReadTW88(REG215);
	return wTemp;
}
//-----------------------------------------------------------------------------
/**
* Write Vertical DE register
*/
void ScalerWriteVDEReg(WORD pos)
{
	//dPrintf("\n\rScalerSetVDEAndWidth pos:%bd len:%d",pos,len);
	WriteTW88(REG220, (ReadTW88(REG220) & 0xCF) | ((pos >> 4) & 0x30));
	WriteTW88(REG215, (BYTE)pos);
}


//-----------------------------------------------------------------------------
/**
* calculate & write vDEReg value.
*
* simple Formula is 	vDE = (DWORD)vStart * PANEL_V / vActive;
* but, it is unaccurate and people does not like it.
* read real vscaler value and calculate.
*/
void ScalerSet_vDE_value(BYTE vStart)
{
	WORD wTemp;
	DWORD dTemp;

	wTemp = ScalerReadVScaleReg();
	dTemp = vStart;
	dTemp <<= 13;  //*= 8192;
	dTemp /= wTemp;
	ScalerWriteVDEReg((WORD)dTemp);
	Printf("\nvStart:%bd *8192/%d => vDE:%d",vStart,wTemp,(WORD)dTemp);
}


//-----------------------------------------------------------------------------
//register
//	R217[3:0]R216[7:0]	output width. normally PANLE_H+1
#ifdef UNCALLED_SEGMENT
void ScalerWriteOutputHeight(WORD height)
{
	WriteTW88(REG217, (ReadTW88(REG217) & 0xF0) | (BYTE)(height>>8));
	WriteTW88(REG216, (BYTE)height);
}
#endif
//-----------------------------------------------------------------------------
/**
* Read Outout Height register
*/
#if 0
WORD ScalerReadOutputHeight(void)
{
 	WORD height;
	height = ReadTW88(REG217) & 0x0F;
	height <<= 8;
	height |= ReadTW88(REG216);	//V Width
	return height;
}
#endif

#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC) || defined(SUPPORT_DVI)
//-----------------------------------------------------------------------------
/**
* Calculate VDE value.
*
*	method
*	VStart = REG(0x536[7:0],0x537[7:0])
*	VPulse = REG(0x52a[7:0],0x52b[7:0])
*	VPol = REG(0x041[3:3])
*	VScale = REG(0x206[7:0],0x205[7:0])
*	result = ((VStart - (VPulse * VPol)) * 8192 / VScale) + 1
*
* This mehtod working only when the input has a active..
*/
#if 0
WORD ScalerCalcVDE(void)
{
	BYTE VPol;
	WORD VStart,VPulse,VScale;
	DWORD dTemp;;

	Read2TW88(REG536,REG537, VStart);	//incorrect....
	Read2TW88(REG52A,REG52B, VPulse);

#ifdef SUPPORT_DVI
//BKTODO150417....Something wrong...Do not use...and remove it.
	if(InputMain==INPUT_DVI) {
		VPol = ReadTW88(REG050) & 0x02 ? 1: 0;
	}
	else 
#endif
	{
		VPol = ReadTW88(REG041) & 0x08 ? 1: 0;
	}
	VScale = ScalerReadVScaleReg();

	dTemp = VStart;
	if(VPol) {
		if(dTemp < VPulse) {
			ePrintf("\n\rBugBug: dTemp:%ld < VPulse:%d",dTemp,VPulse);
			Read2TW88(REG538,REG539, VStart);
			dTemp = VStart;
		}

		dTemp -= VPulse;
	}
	dTemp = (dTemp * 8192L) / VScale;
	dTemp++;
	
	return (WORD)dTemp;
}
#endif
WORD ScalerCalcVDE2(WORD vStart, char vde_offset)
{
	BYTE VPol;
	WORD VPulse,VScale;	 //VStart
	DWORD dTemp;;

	Printf("\n\rvBPorch:%d, offset:%bd",vStart,vde_offset);

	//Read2TW88(REG536,REG537, VStart);	//incorrect....
	Read2TW88(REG52A,REG52B, VPulse);	 //?vSync


	VPol = ReadTW88(REG041) & 0x08 ? 1: 0;

	VScale = ScalerReadVScaleReg();

	dTemp = vStart;
	dTemp <<= 4;
	dTemp += vde_offset;
	dTemp = (dTemp * 8192L) / VScale;
	dTemp >>=4;	
	Printf(" vDe:%d",(WORD)dTemp);

	return (WORD)dTemp;
}
#endif

//-----------------------------------------------------------------------------
//set the "VDEstart" and "VDEwidth" on Vertical Timming Flow.
#if 0
void ScalerSetVDEPosHeight(BYTE pos, WORD len)
{
	//dPrintf("\n\rScalerSetVDEPosHeight pos:%bd len:%d",pos,len);
	ScalerWriteVDEReg(pos);
	ScalerWriteOutputHeight(len);
}
#endif


//-----------------------------------------------------------------------------
//register
//	R212[3:0]R211[7:0]	HA_LEN
//						Output DE length control in number of the output clocks. A 12-bit register
//						output height. normally PANEL_V
//	R217[3:0]R216[7:0]	output width. normally PANLE_H+1
#ifdef UNCALLED_SEGMENT
void ScalerSetOutputWidthAndHeight(WORD width, WORD height)
{
	ScalerWriteOutputWidth(width);
	ScalerWriteOutputHeight(height);
}
#endif


//-----------------------------------------------------------------------------
//VS_LEN		R218[7:6]	VSyhch width
//VS_POS		R220[3:0]R218[5:0]	VSyhch pos
//set the "VSstart" and "VSwidth" on Vertical Timming Flow.
//Scaler set output VSynch
#ifdef UNCALLED_SEGMENT
void ScalerSetVSyncPosLen(WORD pos, BYTE len)
{
	WriteTW88(REG218, len << 6 | (pos & 0x3F));
	WriteTW88(REG220, ReadTW88(REG220) & 0xF0 | (pos >> 6));
}
#endif


#if 0
//-----------------------------------------------------------------------------
/**
* Scaler Freerun VTotal value.
* If manual is offed, it will read the internal value.
*
*	R20D[7:6]:R219[7:0]
*/
void ScalerWriteFreerunVtotal(WORD value)
{
	WriteTW88(REG20D, (ReadTW88(REG20D)&0x3F)|(value>>2)&0xC0);
	WriteTW88(REG219, (BYTE)value );
}
#endif

WORD ScalerReadFreerunVtotal(void)
{
	WORD value;

	value = ReadTW88(REG20D) & 0xC0;
	value <<= 2;
	value |= ReadTW88(REG219);
	return value;
}
//-----------------------------------------------------------------------------
/**
* calcualte Freerun VTotal value
*
*	method
*	VPN    = REG(0x522[7:0],0x523[7:0])
*	Vscale = REG(0x206[7:0],0x205[7:0])
*	result = VPN / (Vscale / 8192)
*
* NOTE: It needs a MeasStart.
*/
#if 0
WORD ScalerCalcFreerunVtotal(void)
{
	WORD VScale;
	DWORD temp32;
	
	VScale = ScalerReadVScaleReg();
	
	temp32 = MeasGetVPeriod();
	temp32 *= 8192L;
	temp32 /= VScale;
	
	return (WORD)temp32;
}
#endif

//-----------------------------------------------------------------------------
//DM_TOP	R21A[7:0]	top 	 
//DM_BOT	R21B[7:0]	bottom 
//set number of data masked lines from the top of DE and the end of DE.
#ifdef UNCALLED_SEGMENT
void ScalerSetVDEMask(BYTE top, BYTE bottom)
{
	WriteTW88(REG21A, top);
	WriteTW88(REG21B, bottom);
}
#endif

//-----------------------------------------------------------------------------
/**
* description
* 	Scaler Freerun Htotal value. 
* 	If manual is offed, it will read the internal value.
* register
*	R21C[7:4]:R21D[7:0]
*/
#if 0
void ScalerWriteFreerunHtotal(WORD value)
{
	WriteTW88(REG21C, (ReadTW88(REG21C)&0x0F)|(value>>4)&0xF0);
	WriteTW88(REG21D, (BYTE)value );
}
#endif

WORD ScalerReadFreerunHtotal(void)
{
	WORD value;

	value = ReadTW88(REG21C) & 0xF0;
	value <<= 4;
	value |= ReadTW88(REG21D);
	return value;
}

//-----------------------------------------------------------------------------
/*!
* \brief calculate the Freerun HTotal value
* \return freerun hTotal
*
*	method
*	HPN    = REG(0x524[7:0],0x525[7:0])
*	Vscale = REG(0x206[7:0],0x205[7:0])
*	PCLKO_div  = REG(0x20d[1:0]) + 1
*	PCLKO_mul  = REG(0x20d[5]) + 1
*	result = (HPN * Vscale / 8192) / PCLKO_div * PCLK_mul
*/
#if 0
WORD ScalerCalcFreerunHtotal(void)
{
	WORD VScale;
	DWORD temp32;
	BYTE bTemp;
	BYTE div,mul;

	VScale = ScalerReadVScaleReg();
	bTemp = ReadTW88(REG20D);

	div = (bTemp & 0x03) + 1;
	if(bTemp & 0x20) 	mul = 2;
	else				mul = 1;

	temp32 = MeasGetHPeriodReg();
	temp32 *= VScale;
	temp32 /= 8192L;

	temp32 /= div;
	temp32 *= mul;


//#if defined(DEBUG_SCALER)
//	dPrintf("\n\rScalerCalcFreerunHtotal temp:%ld",temp32);
//#endif	
	return (WORD)temp32;
}
#endif

/*!
* \fn void ScalerCalcFreerunValue(WORD *hTotal, WORD *vTotal)
* \brief calc freerun value
* \param hTotal horizontal freerun value
* \param vTotal vertical freerun value
* \return success or fail
*
* need vScaleRate, measured value.
*
*	VPN    = REG(0x522[7:0],0x523[7:0])
*	Vscale = REG(0x206[7:0],0x205[7:0])
*	vTotal = VPN / (Vscale / 8192)
*
*	HPN    = REG(0x524[7:0],0x525[7:0])
*	Vscale = REG(0x206[7:0],0x205[7:0])
*	PCLKO_div  = REG(0x20d[1:0]) + 1
*	PCLKO_mul  = REG(0x20d[5]) + 1
*	hTotal = (HPN * Vscale / 8192) / PCLKO_div * PCLK_mul
*
*/
void ScalerCalcFreerunValue(WORD *Freerun_hTotal, WORD *Freerun_vTotal)
{
	DWORD dTemp;
	WORD vScaleRate;
	BYTE bTemp;
	BYTE div,mul;

	vScaleRate = ScalerReadVScaleReg();
	bTemp = ReadTW88(REG20D);	//PCLKO
	div = (bTemp & 0x03) + 1;
	if(bTemp & 0x20) 	mul = 2;
	else				mul = 1;

	//calc freerun vTotal
	dTemp = MeasGetVPeriod();
	dTemp *= 8192L;
	dTemp /= vScaleRate;
	*Freerun_vTotal = dTemp;

	//calc freerun hTotal
	dTemp = MeasGetHPeriodReg();
	dTemp *= vScaleRate;
	dTemp /= 8192L;
	dTemp *= mul;
	dTemp /= div;
	*Freerun_hTotal = dTemp;

	//dPrintf("\n\rScalerCalcFreerunValue hTotal:%d vTotal:%d",*Freerun_hTotal,*Freerun_vTotal);
}


//RRUN		R21C[2]
//			Panel free run control. 1 = free run with HTOTAL and LNTT.
//-----------------------------------------------------------------------------
/**
* description
*
* comes fromTW8823.
* free run control R21C[2] -Panel free run control. 
*	1 = free run with HTOTAL(R21C[7:4]R21D[7:0] and LNTT(R20D[7:6]R219[7:0]).
* free run on the condition of input loss R21C[1].
*/
//#if 0
//void ScalerSetFreerunManual( BYTE on )
//{
//	Printf("\n\rScalerSetFreerunManual(%bd)",on);
//  
//	if(on)	WriteTW88(REG21C, (ReadTW88(REG21C) | 0x04) );		//on manual freerun
//	else 	WriteTW88(REG21C, (ReadTW88(REG21C) & ~0x04) );		//off manual freerun
//}
//#endif
//#ifdef UNCALLED_CODE
//BYTE ScalerIsFreerunManual( void )
//{
//	BYTE value;
//
//	value = ReadTW88(REG21C);
//	if(value & 0x04) return 1;
//	return 0;
//}
//#endif
//-----------------------------------------------------------------------------
/**
* set FreerunAuto and FreerunManual
*/
void ScalerSetFreerunAutoManual(BYTE fAuto, BYTE fManual)
{
	BYTE value;

	//dPrintf("\n\rScalerSetFreerunAutoManual(%bd,%bd)",fAuto,fManual);
	value = ReadTW88(REG21C);
	if(fAuto != 0x02) {
		if(fAuto)	value |= 0x02;		//on auto freerun
		else 		value &= ~0x02;		//off auto freerun		
	}
	if(fManual != 0x02) {
		if(fManual)	value |= 0x04;		//on manual freerun
		else 		value &= ~0x04;		//off manual freerun		
	}
	WriteTW88(REG21C, value);

	//BK130516
	Puts("-END");
}
//-----------------------------------------------------------------------------
/**
* description
* 	Force scaler data output to all 0’s.
* 	set MuteAuto & MuteManual
*/
void ScalerSetMuteAutoManual(BYTE fAuto, BYTE fManual)
{
	BYTE value;
	
	value = ReadTW88(REG21E);
	if(fAuto != 0x02) {
		if(fAuto)	value |= 0x02;		//on auto mute
		else 		value &= ~0x02;		//off auto mute		
	}
	if(fManual != 0x02) {
		if(fManual)	value |= 0x01;		//on manual mute
		else 		value &= ~0x01;		//off manual mute		
	}
	WriteTW88(REG21E, value);
}

//-----------------------------------------------------------------------------
/**
* set MuteManual
*/
void ScalerSetMuteManual( BYTE on )
{
	if(on) {
		//on manual mute.
		//do not wait vblank. 
		//if Freerun_Manual was offed on VDLoss, you can not use WaitVBlank().
		WriteTW88(REG21E, (ReadTW88(REG21E) | 0x01) );	
	}
	else {
		//off manual mute
		WaitOneVBlank();
		WriteTW88(REG21E, (ReadTW88(REG21E) & ~0x01) );	
	}
}

/**
* description
*	read and print the scaler freerun values.
*/
#ifdef DEBUG_SCALER
void ScalerPrintFreerunInfo(void)
{
	BYTE bTemp;
	WORD wTemp;

	Printf("\n\rFreerun ");
	bTemp = ReadTW88(REG21C);
	Printf(" fixed:%s", bTemp & 0x01 ? "On" : "Off");
	Printf(" auto:%s", bTemp & 0x02 ? "On" : "Off");
	Printf(" manual:%s", bTemp & 0x04 ? "On" : "Off");

	wTemp = ReadTW88(REG21C) & 0xF0;
	wTemp <<= 4;
	wTemp |= ReadTW88(REG21D);
	Printf(" hTotal:%d",wTemp);

	wTemp = ReadTW88(REG20D) & 0xC0;
	wTemp <<= 2;
	wTemp |= ReadTW88(REG219);
	Printf(" vTotal:%d",wTemp);

	if((bTemp & 0x04)==0) {
		WriteTW88(REG21C, bTemp | 0x04);
		wTemp = ReadTW88(REG21C) & 0xF0;
		wTemp <<= 4;
		wTemp |= ReadTW88(REG21D);
		Printf(" hTotal_on_manual:%d",wTemp);
		WriteTW88(REG21C, bTemp & ~0x04);
	}
}
#endif

//PanelFreerun value
//
//
//      component					CVBS
//mode	Htotal	Vtotal XYScale		Htotal	Vtotal	XYScale
//----	------	------ -------		------	------	-------
//480i	1069	551	   1B5C	0F33	1069	551(553)1C00 0F33
//576i	1294	548	   1B5C	1244	1299	544		1C00 1255
//480p	1069	552
//576p	1292	548
//1080i	1122	525
//720p	1122	526
//1080p	????	???
//
//BKFYI
//	I saw the measure block can detect the Period value on CVBS & DVI.
//	And, CVBS(SVideo) use 27MKz fixed clock, so I don't need to consider InputMain.
//

//BKFYI:HW default HTOTAL:1024,VTOTAL:512. I don't need a force mode.
//-----------------------------------------------------------------------------
/**
* set Scaler Freerun value
*/
void ScalerSetFreerunValue(void)
{
	WORD HTotal;
	WORD VTotal;
	BYTE ret;


	//Before measure, disable an En.Change Detection. and then start a measure.
	//MeasStartMeasure will capture a reference value for "En.Change detection".
	MeasEnableChangedDetection(OFF);

	//call measure once to update the value or use a table value
	Meas_StartMeasure();
	ret=Meas_IsMeasureDone(50);
	if(ret) {
		Printf("\n\rBUG FreerunValue failed!!");
		//assign the emergency default value.
		HTotal = FREERUN_DEFAULT_HTOTAL;
		VTotal = FREERUN_DEFAULT_VTOTAL;
	}
	else {
		ScalerCalcFreerunValue(&HTotal,&VTotal);
#if defined(DEBUG_SCALER)
		ScalerPrintFreerunInfo();
#endif
	}
	//turn on the En.Changed Detection.
	MeasEnableChangedDetection(ON);

	ScalerWriteFreerunTotal(HTotal,VTotal);			
}

//-----------------------------------------------------------------------------
/**
* check the freerun value before we go into the freerun mode.
*	
* Hmin = HDE+HWIDTH
*
* Vmin = VDE+VWIdth
*/
#if 0 //removed
//void ScalerCheckPanelFreerunValue(void)
//{
//	WORD Total, Min;
//	BYTE changed;
//
//	changed = 0;
//
//	// Horizontal
//	Total = ScalerReadFreerunHtotal();
//	Min = ScalerReadOutputWidth();	//H Width
//	Min += ScalerReadHDEReg();		//H-DE
//	Min += 2;
//dPrintf("\n\rScaler Freerun HTotal %d->%d",Total, Min);
//	if(Total < Min) {
////		dPrintf("\n\rScaler Freerun HTotal %d->%d",Total, Min);
//		Total = Min;
////		ScalerWriteFreerunHtotal(Total);
//		changed++;
//	}
//
//	//Vertical
//	Total = ScalerReadFreerunVtotal();
//	Min = ScalerReadOutputHeight();	//V Width
//	Min += ScalerReadVDEReg();		//V-DE
//	Min += 2;
//dPrintf("\n\rScaler Freerun VTotal %d->%d",Total, Min);
//	if(Total < Min) {
////		dPrintf("\n\rScaler Freerun VTotal %d->%d",Total, Min);
//		Total = Min;
////		ScalerWriteFreerunVtotal(Total);
//		changed++;
//	}
//
//
//	ScalerWriteFreerunHtotal(global_Freerun_hTotal);
//	ScalerWriteFreerunVtotal(global_Freerun_vTotal);
//
//	if(changed) {
//		SpiOsdSetDeValue();	//BK111013
//		FOsdSetDeValue();
//	}
//}
#endif

//BKFYI.
//if ManualFlag, REG21C[2], was 0, we can not read back.
//	But, the writen value is working.
//if AutoFlag, REG21C[1], was 1 with InputSignal, 
//	the writen value will be overwrite with internal value. 
/**
* description
*	write freerun total values.
* 	it only can write on Freerun Manual.
*/
void ScalerWriteFreerunTotal(WORD h, WORD v)
{
	BYTE bTemp;

	//dPrintf("\n\rScalerFreerunValue HTotal:%d VTotal:%d",h,v);

	bTemp = ReadTW88(REG21C) & 0x0F;
	//bTemp |= 0x04;	//on Manual
	WriteTW88(REG21C, bTemp |((h>>4)&0xF0));
	WriteTW88(REG21D, (BYTE)h );
	bTemp = ReadTW88(REG20D)&0x3F;
	WriteTW88(REG20D, bTemp | ((v>>2)&0xC0));
	WriteTW88(REG219, (BYTE)v );

	//save to global. 
	global_Freerun_hTotal = h;
	global_Freerun_vTotal = v;
}


//-----------------------------------------------------------------------------
/**
* description
*
* step
*	set freerun manual.
*	read freerun value
*	if not correct, overwrite with the previous saved value
*/
void ScalerCheckAndSetFreerunManual(void)
{
	BYTE r21C,r20D;
	BYTE bTemp;
#ifdef DEBUG_SCALER
	WORD wTemp;
	WORD hTotal, vTotal;
	WORD minimum, maximum;
#endif

#ifdef DEBUG_SCALER
	ScalerPrintFreerunInfo();
#endif

	//On Manual.
	r21C = ReadTW88(REG21C);
	r21C |= 0x04;
	WriteTW88(REG21C, r21C);

#ifdef DEBUG_SCALER
	//----------------
	// hTotal
	//----------------
	//read hTotal
	wTemp = r21C & 0xF0;
	wTemp <<=4;
	wTemp |= ReadTW88(REG20D);
	hTotal = wTemp;

	//check minimum value
	wTemp = PANEL_H + 1;
	wTemp += ScalerReadHDEReg();
	wTemp += 2;
	minimum = wTemp;
	maximum = minimum * 3 / 2; 
	Printf("\n\rFreerun hTotal cur:%d min:%d max:%d saved:%d", hTotal,minimum,maximum,global_Freerun_hTotal);
#endif
	bTemp = global_Freerun_hTotal >> 4;
	bTemp &= 0xF0;
	WriteTW88(REG21C, (r21C & 0x0F) | bTemp);
	WriteTW88(REG21D, (BYTE)global_Freerun_hTotal );

	//----------------
	// vTotal
	//----------------
	r20D = ReadTW88(REG20D);
#ifdef DEBUG_SCALER
	wTemp = r20D & 0xC0;
	wTemp <<= 2;
	wTemp |= ReadTW88(REG219);
	vTotal = wTemp;

	//check minimum value
	wTemp = PANEL_V;
	wTemp += ScalerReadVDEReg();
	wTemp += 2;
	minimum = wTemp;
	maximum = minimum * 3 / 2; 
	Printf("\n\rFreerun vTotal cur:%d min:%d max:%d saved:%d", vTotal,minimum,maximum,global_Freerun_vTotal);
#endif
	bTemp = global_Freerun_vTotal >> 2;
	bTemp &= 0xC0;
	WriteTW88(REG20D, (r20D & 0x3F)| bTemp);
	WriteTW88(REG219, (BYTE)global_Freerun_vTotal );

#if defined(DEBUG_SCALER)
	ScalerPrintFreerunInfo();
#endif
}


