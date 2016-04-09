/**
 * @file
 * BT656.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2012-2013 Intersil Corporation
 * @section DESCRIPTION
 *	BT656
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
#include "decoder.h"
#include "aRGB.h"
#include "dtv.h"
#include "measure.h"
#include "PC_modes.h"

#include "BT656.h"
#include "Settings.h"


struct BT656SCALE_OPTION_s {
	WORD hActive,vActive;
	BYTE vFreq;
	BYTE option0, option1, option2;
};
BYTE FindBT656ScalerTable(struct BT656SCALE_OPTION_s *table,WORD hActive, WORD vActive, BYTE vFreq, BYTE fScale)
{
	BYTE idx;

	struct BT656SCALE_OPTION_s * p = table;

	if(vFreq==59)
		vFreq=60;

	while(p->hActive) {
		if(hActive == p->hActive
		&& vActive == p->vActive
		&& vFreq   == p->vFreq) {
			switch(fScale) {
			case 2:  idx = p->option2; break;
			case 1:  idx = p->option1; break;
			default: idx = p->option0; break;
			}
			if(idx == 0xFF)
				idx = p->option0;
			return idx;
		}
		p++;
	}
#ifdef DEBUG_BT656
	Printf("\nFindBT656ScalerTable %dx%d%bd fScale:%bx Fail",hActive,vActive,vFreq,fScale); 
#endif
	return 0xFF; //fail
}

WORD BT656Enc_Calc_hActive(WORD hActive, WORD hScale)
{
	DWORD dTemp;

	dTemp = hActive;
	dTemp *= 0x400;
	dTemp /= hScale;
	return (WORD)dTemp;
}
WORD BT656Enc_Calc_vActive(WORD vActive, BYTE vScale)
{
	DWORD dTemp;

	dTemp = vActive;
	switch(vScale) {
	case 2: dTemp <<= 1; break;
	case 6: dTemp <<= 1; dTemp /= 3; break;
	case 8: dTemp >>= 1; break;
	case 9: dTemp <<= 2; dTemp /= 9; break;
	default: break;
	}
	return (WORD)dTemp;
}
 

/**
* Enable BT656 Encoder Module
*
* REG007[3]
* @param fOn 1:On 0:Off.
* @return : 1:changed
*/
BYTE BT656Enc_Enable(BYTE fOn)
{
	BYTE temp;
	BYTE ret=1;

	temp = ReadTW88(REG007);
	if(fOn) {
		if(temp & 0x08) ret = 0;
		temp |= 0x08;
	}
	else {
		if((temp & 0x08) == 0) ret = 0;
	   temp &= ~0x08;
	}
	WriteTW88(REG007,temp);
	return ret;
}

/**
* Control BT656(BT601) Clock Output polarity.
*
* @param fActiveHigh 
* 	0:Low Active
*	1:High Active 
*
* REG040[5] VDCLK_POL 0:Low Active
*/
void BT656Enc_SetOutputClkPol(BYTE fActiveHigh)
{
	BYTE temp;
	temp = ReadTW88(REG040) & ~0x20;
	if(fActiveHigh) temp |= 0x20;
	WriteTW88(REG040, temp);
}


#if 0
/**
* Control BT656(BT601) Synch output.
*
* fOn : enable HSync & VSync pin Output
* h	 Horizontal Polarity 0:Normal, 1:Inverse.
* v	 Vertical   Polarity 0:Normal, 1:Inverse.
*
* REG048[6] Enable BT656 H/VSYNC output through aRGB H/V.
* REG066[1] Output VSYNC polarity
* REG066[0] Putput HSYNC polarity
* REG06E[7:0]			vSync Width
* REG06F[7:0]			hSync Width
*/
//void BT656_SyncPolarity(BYTE fOn, BYTE hPol, BYTE hSync, BYTE vPol, BYTE vSync)
//{
//	BYTE bTemp;
//	if(fOn) {
//		WriteTW88(REG048, ReadTW88(REG048) | 0x40);
//		bTemp = ReadTW88(REG066) & 0xFC;
//		bTemp |= (vPol << 1);
//		bTemp |= hPol;
//		WriteTW88(REG066, bTemp);
//		WriteTW88(REG06E,vSync);
//		WriteTW88(REG06F,hSync);
//	}
//	else {
//		WriteTW88(REG048, ReadTW88(REG048) & ~0x40);
//	}
//}
#endif

/**
* Set 2D-DI Odd/Even Offset
* REG066[7:6] : Odd Offset
* REG066[5:4] : Even Offset
*/
void BT656Enc_2DDI_FieldOffset(BYTE Odd, BYTE Even)
{
	BYTE temp;
	temp = ReadTW88(REG066) & 0x0F;
	temp |= (Odd << 6);
	temp |= (Even << 4);
	WriteTW88(REG066, temp);
}


/**
* Select RGB or YUV.
* only for DTV,LVDSRX, Panel.
* REG067[6]
*/
void BT656Enc_D_SetRGB(BYTE fOn)
{
	BYTE bTemp;
	bTemp = ReadTW88(REG067) & ~0x40;
	if(fOn)	bTemp |= 0x40;
	WriteTW88(REG067,bTemp);
}

//REG067[5]
//REG067[4]
//REG067[3]

/**
* Select BT656Enc source
* If it is dec or aRGB, it uses an analog block.
* If it is dtv, lvds, panel, it uses a digital block.
*
* @param fMode 0:dec,1:aRGB,2:DTV,3:LVDS,4:Panel
* REG067[2:0]
*/
void BT656Enc_SelectSource(BYTE fMode)
{
	BYTE bTemp;

	bTemp = ReadTW88(REG067) & 0xF8;
	bTemp |= fMode;
	WriteTW88(REG067,bTemp);
}

//REG068

/**
* set input crop for BT656Enc digital block.
* FYI. the analog block uses a decoder input crop registers.
*
* Horizontal Start Active Video
* Vertical Start Active Video
* REG068[]
* REG069[]
* REG06A[]
* REG06B[]
* REG06C[]
* REG06D[]
*/
void BT656Enc_Crop(WORD h_SAV, WORD v_SAV, WORD HLen, WORD VLen)
{
	BYTE bTemp;
	
#ifdef DEBUG_BT656
	//Printf("\n\rBT656Enc Crop %d hCropStart:%d",HLen, h_SAV);
	//Printf("\n\r              %d vCropStart:%d",VLen, v_SAV);
	Printf("\n\rBT656Enc BTCrop hStart:%d hLen:%d  vStart:%d vLen:%d", h_SAV,HLen, v_SAV,VLen);
#endif

	bTemp = ReadTW88(REG068) & 0xFC;
	bTemp |= (h_SAV >> 8);
	WriteTW88(REG068, bTemp);
	bTemp = ReadTW88(REG069) & 0x80;
	bTemp |= ((BYTE)(VLen >> 4) & 0x70);
	bTemp |= (BYTE)(HLen >> 8);
	WriteTW88(REG069, bTemp);
	WriteTW88(REG06A, (BYTE)v_SAV);
	WriteTW88(REG06B, (BYTE)VLen);
	WriteTW88(REG06C, (BYTE)h_SAV);
	WriteTW88(REG06D, (BYTE)HLen);
}

#if 0
/**
* ??for BT601 ?. passthru aRGB H & V.
* REG06E
* REG06F
*/
void BT656_SyncWidth(BYTE h, BYTE v)
{
	WriteTW88(REG06F,h);
	WriteTW88(REG06E,v);
}
#endif

/**
*
* REG077[4] Enable double data rate output mode for VD656 bus.   
*			(Not valid on interlaced decoder BT656 output)
*/
#if 0
void BT656Enc_EnableDDR(BYTE fOn)
{
	BYTE temp;
	temp = ReadTW88(REG077);
	if(fOn) temp |=  0x10;
	else    temp &= ~0x10;
	WriteTW88(REG077,temp);
}
#endif
/**
*
* REG077[5]	Enable double code width when double data rate output mode is enabled.
*			(Not valid on interlaced decoder BT656 output)
*			1: Code word is repeated FF-FF-00-00-00-00-XX-XX
*			0: Normal code word FF-00-00-XX
*
* REG061[4] Enable VD656 bus output clock phase delay 90 degree 
*			when the output is configured as double data rate. 
*			(Not valid on interlaced decoder BT656 output)
*/
#if 0
void BT656Enc_SetDDR_Options(BYTE fRepeat, BYTE fClock90)
{
	BYTE temp;
	temp = ReadTW88(REG077);
	if(fRepeat) temp |=  0x20;
	else    	temp &= ~0x20;
	WriteTW88(REG077,temp);

	temp = ReadTW88(REG061);
	if(fClock90) temp |=  0x10;
	else    	 temp &= ~0x10;
	WriteTW88(REG061,temp);
}
#endif


//
//BT656I BT656 interlaced
//BT656P BT656 DeInterlaced. Progressive
//ADC
//

/**
* description
*	select analog BT656 output.
* @param mode video data outputmode
*		0: interlaced BT656
*		1: Progressive BT656
*		2: BT656 ADC
* @param hPol
*		1: invert hSync ouput pin
* @param vPol
*		1: invert vSync output pin
* @param hv_sel hSync Output, vSync Output control
*		00b : use aRGB module hSync(HSY_SEL), vSync(VSY_SEL). See REG1CC[4] and REG1CC[3:2].
*		01b : use decoder generated hSync and vSync
*		10b : use de-interlaced hSync and vSync.
* REG105
*/
void BT656_A_SelectOutput(BYTE mode,BYTE hPol,BYTE vPol, BYTE hv_sel)
{
	BYTE bTemp;

#ifdef DEBUG_BT656
	Printf("\n\rBT656_A_Output(%bd,,)",mode);
#endif
  
	bTemp = mode << 4;
	if(hPol==0) bTemp |= 0x08;
	if(vPol==0) bTemp |= 0x04;
	bTemp |= hv_sel;
	WriteTW88(REG105,bTemp);
}

#define BT656_A_OUT_DEC_I		0
#define BT656_A_OUT_DEC_DI		1
#define BT656_A_OUT_DEC_ADC		2
/**
* set BT656Enc Analog output control
*
* REG105[3] 0:HSO pin output inversion
*			1:HSO pin output no inversion
* REG105[2] 0:VSO pin output inversion
*			1:VSO pin output no inversion
*/
void BT656_A_Output(BYTE mode, BYTE hPol, BYTE vPol)
{
	BYTE bTemp;

#ifdef DEBUG_BT656
	Printf("\n\rBT656_A_Output(%bd,%bd,%bd)",mode,hPol,vPol);
#endif

	bTemp = 0;
	switch(mode) {
	case BT656_A_OUT_DEC_I:		bTemp |= 0x01;	break;
	case BT656_A_OUT_DEC_DI:	bTemp |= 0x12;	break;
	case BT656_A_OUT_DEC_ADC:	bTemp |= 0x20;	break;
	default:					bTemp |= 0x10;	break;
	}
	if(hPol)					bTemp |= 0x08;
	if(vPol)					bTemp |= 0x04;
	WriteTW88(REG105 , bTemp);	

	bTemp = ReadTW88(REG1E9) & 0x3F;
	switch(mode) {
	case BT656_A_OUT_DEC_I:		bTemp |= 0x00;	break;
	case BT656_A_OUT_DEC_DI:	bTemp |= 0x40;	break;
	case BT656_A_OUT_DEC_ADC:	bTemp |= 0x80;	break; //??
	default:					bTemp |= 0x00;	break;
	}
	WriteTW88(REG1E9 , bTemp);	
}


#if 0
/**
* moved to REG066
* REG136[3:2]
* REG136[1:0]
*/
//void BT656_A_SetDeInterlaceFieldOffset(BYTE Odd, BYTE Even)
//{
//	BYTE bTemp;
//	bTemp = ReadTW88(REG136) & 0xF0;
//	bTemp |= (Odd < 2);
//	bTemp |= (Even);
//	WriteTW88(REG136,bTemp);
//}
#endif

/**
removed 
* REG137[7:0]
* REG138[7:0]
*/
#if 0
//void BT656_A_DeInterlace_Set(BYTE hdelay, BYTE hstart)
//{
//	WriteTW88(REG137,hdelay);
//	WriteTW88(REG138,hstart);
//}
#endif

//REG1E8[]. move from TW8835 REG105.


//----------------------------
// BT656 Analog Control.
// for ADC and aRGB only
//----------------------------
void BT656_A_SetLLCLK_Pol(BYTE pol)
{
	BYTE bTemp;
	bTemp = ReadTW88(REG1E9) & 0xFD; //~0x02
	bTemp |= pol << 1;
	WriteTW88(REG1E9,bTemp);
}
void BT656_A_SelectCLKO(BYTE mode, BYTE PolClko)
{
	BYTE bTemp;
	bTemp = ReadTW88(REG1E9) & 0x1F;
	bTemp = mode << 6;
	bTemp |= PolClko << 5;
	WriteTW88(REG1E9,bTemp);
}
void BT656Enc_En2DDI(BYTE fOn)
{
	BYTE bTemp;
	bTemp = ReadTW88(REG1E9) & 0xFE;
	if(fOn) bTemp |= 0x01;
	WriteTW88(REG1E9,bTemp);
}


#if 0
void BT656Enc_GenLockPowerDown(BYTE fOn)
{
	BYTE bTemp;
	bTemp = ReadTW88(REG1E1) & ~0x20;
	if(fOn) bTemp |= 0x20; //powerdown
	WriteTW88(REG1E9,bTemp);
}
#endif

/**
*
* param
*	PreDiv: REG077[3:0]
*	x8:     REG1E1[6]
*/
#define GENLOCK_X4	0
#define GENLOCK_X8	1
void BT656Enc_GenLock(BYTE PreDiv, BYTE x8)
{
	BYTE temp;

	WriteTW88(REG077, (ReadTW88(REG077) & 0xF0) | PreDiv);

	temp = ReadTW88(REG1E1);
	if(x8) temp |=  0x40;
	else   temp &= ~0x40;
	WriteTW88(REG1E1,temp);	
}


/**
* REG1EA[1] 0: RGBCLK div2, 1:RGBCLK
* REG1EA[0] 0: ADCCLK div2, 1:ADCCLK
*/
#if 0
void BT656_AdcRgbNoClkDivider2(BYTE fAdcOn, BYTE fRgbOn)
{
	BYTE bTemp;
	bTemp = ReadTW88(REG1EA) & ~0x03;
	if(fRgbOn) bTemp |= 0x02;
	if(fAdcOn) bTemp |= 0x01;
	WriteTW88(REG1EA, bTemp);
}
#endif

//=============================================================================
//
//=============================================================================

/**
*
*/
BYTE BT656_CheckLoopbackCombination(BYTE mode)
{
	BYTE fError;
	BYTE bt656_src_mode;

	if(mode==BT656ENC_SRC_DEC 
	|| mode==BT656ENC_SRC_ARGB 
	|| mode==BT656ENC_SRC_DTV 
	|| mode==BT656ENC_SRC_LVDS) {
		//check input combination
		fError = 0;
		switch(InputMain) {
		case INPUT_CVBS:
		case INPUT_SVIDEO:
			if(mode != BT656ENC_SRC_DEC) 
				fError = 1;
			break;
		case INPUT_COMP:
		case INPUT_PC:
			if(mode != BT656ENC_SRC_ARGB)
				fError = 1;
			break;
		case INPUT_DVI:
		case INPUT_HDMIPC:
		case INPUT_HDMITV:
			if(mode != BT656ENC_SRC_DTV)
				fError = 1;
			break;
		case INPUT_LVDS:
			if(mode != BT656ENC_SRC_LVDS)
				fError = 1;
			break;
		case INPUT_BT656:
			//it is a loopback. input and output have to different.
			if(mode == BT656ENC_SRC_DTV)
				fError = 1;			
			break;
		}
		if(fError) {
			Puts("\n\r FAIL");	
			PrintfInput(InputMain,0);
			Puts(" ");	
#if	defined(DEBUG_BT656)
			PrintfBT656Input(mode, 0);
#endif
			return 0xFF;
		}
		bt656_src_mode = mode;
	}
	else if(mode==BT656ENC_SRC_AUTO) {
		//select BT656ENC_SRC_MODE
		switch(InputMain) {
		case INPUT_CVBS:
		case INPUT_SVIDEO:
			bt656_src_mode = BT656ENC_SRC_DEC;
			break;
		case INPUT_COMP:
		case INPUT_PC:
			bt656_src_mode = BT656ENC_SRC_ARGB;
			break;
		case INPUT_DVI:
		case INPUT_HDMIPC:
		case INPUT_HDMITV:
			bt656_src_mode = BT656ENC_SRC_DTV;
			break;
		case INPUT_LVDS:
			bt656_src_mode = BT656ENC_SRC_LVDS;
			break;
		case INPUT_BT656:	//loopback
			bt656_src_mode = BT656ENC_SRC_DEC;
			break;
		default:
			//bt656_src_mode = mode;
			Printf("\n\rUnknown InputMain:%bd",InputMain);
			return 0xFF;	
		}
	}
	else {
		//panel
		bt656_src_mode = mode;
	}

	return bt656_src_mode;
}


/**
* Change BT656 input.
* Do not change InputMain.
* If InputMain has a conflict, stop and print only a debug message.	
*
* extern
*	InputMain
*	InputBT656
*
* @param mode
*	BT656ENC_SRC_DEC	
*	BT656ENC_SRC_ARGB	
*	BT656ENC_SRC_DTV	
*	BT656ENC_SRC_LVDS	
*	BT656ENC_SRC_PANEL	
*	BT656ENC_SRC_OFF
* @return
*	0: success
*	1:invalid combination
*	2:out of range. only for 480i,576i,480p and 576p.	
*/



//==================================================================================
// NEW
//==================================================================================


/**
* Set BT656 Encoder Scaler
*
* h:12bit. b:4bit
* @param h
*	horizontal scale value. base 0x400. 12bit.
*	1920 to 720 = 1920 * 1024 / 720 = 2730 = 0xAAA.
*	1280 to 720 = 1280 * 1024 / 720 = 1820 = 0x71C.
* @param v
*	vertical scale options
*	0:No Scale
*	2:DI(2x up)
*	4:Cropping only
*	6:DownScale 1.5
*	8:DownScale 2.0
*	9:DownScale 2.25
*/
void BT656Enc_SetScaler(WORD h, BYTE v)
{
	WriteTW88(REG064, (v << 4) | (BYTE)(h >> 8));
	WriteTW88(REG065, (BYTE)h);
		
}
//void BT656Enc_SetPreDivider(BYTE div)
//{
//	WriteTW88(REG077, (ReadTW88(REG077) & 0xF0) | div);
//}
/**
*
* @see BT656Enc_Calc2_hStart_hActive()
*/
void BT656Enc_SetOutputActiveLen(WORD hStart,WORD hLen)
{
	WriteTW88(REG07A, (ReadTW88(REG07A) & 0xF0) | (BYTE)(hStart >> 8));
	WriteTW88(REG07B,(BYTE)hStart);
	WriteTW88(REG07C, (ReadTW88(REG07C) & 0xF0) | (BYTE)(hLen >> 8));
	WriteTW88(REG07D,(BYTE)hLen);
}
/**
* If input vSync is too small, BT656Enc scaler need to add some delay.
* REG07A[7:4]
*/
void BT656Enc_SetVSyncDelay(BYTE delay)
{
	WriteTW88(REG07A, (ReadTW88(REG07A) & 0x0F) | (delay <<4));
}



void BT656Enc_Setup_Init(BYTE source)
{
	WriteTW88(REG062,0x00);
	WriteTW88(REG063,0x00);
	WriteTW88(REG064,0x04);
	WriteTW88(REG065,0x00);
	WriteTW88(REG066,0x30);
	WriteTW88(REG067,0x00);
	WriteTW88(REG068,0x00);
	WriteTW88(REG069,0x02);
	WriteTW88(REG06A,0x20);
	WriteTW88(REG06B,0xF0);
	WriteTW88(REG06C,0x20);
	WriteTW88(REG06D,0xD0);
	WriteTW88(REG06E,0x10);
	WriteTW88(REG06F,0x10);
	WriteTW88(REG077,0x04);
	WriteTW88(REG078,0x00);
	WriteTW88(REG079,0x04);
	WriteTW88(REG07A,0x40);
	WriteTW88(REG07B,0x28);
	WriteTW88(REG07C,0x03);
	WriteTW88(REG07D,0x00);
	WriteTW88(REG07E,0x00);
	WriteTW88(REG07F,0x80);

	BT656Enc_Enable(ON);
	BT656Enc_SelectSource(source);
}

void BT656Enc_Update_BT656ExternalEncoder(WORD hActive, WORD vActive)
{
	BYTE ext_ic_mode;

	ext_ic_mode = 0xFF;
	if(hActive==720) {
		if(vActive==240)
			ext_ic_mode = BT656_8BIT_525I_YCBCR_TO_CVBS;  
		if(vActive==288)
			ext_ic_mode = BT656_8BIT_625I_YCBCR_TO_CVBS; 
	}
	if(ext_ic_mode != 0xFF)
		BT656_InitExtEncoder(ext_ic_mode);	
}



/**
 * Decoder option
 * BYPASS or 2DDI
 * Use EEPROM[0x21]
 */
struct BT656SCALE_OPTION_s BT656SCALE_DEC_OPTION_table[] = {

	{720,240,60,	BT656SCALER_IDX_CVBS_NTSC,			BT656SCALER_IDX_CVBS_NTSC_2D, 0xFF},
	{720,288,50,	BT656SCALER_IDX_CVBS_PAL,			BT656SCALER_IDX_CVBS_PAL_2D, 0xFF},
	{0,}
};
/**
 * Setup Decoder
 *
 * Input
 *	VideoInputSub
 * Output
 *	G_pBt656Scaler
 */
#if defined(SUPPORT_BT656)
struct DIGIT_VIDEO_TIME_TABLE_s * BT656Enc_Setup_Dec(void)
{
	BYTE fScale;
	struct DEC_VIDEO_TIME_TABLE_s *pVideoTable;
	WORD hDelay,hActive, vDelay, vActive;
	BYTE VideoInputMain= InputMain;
	BYTE VideoInputSub = InputSubMode;
	BYTE vFreq;
	struct DIGIT_VIDEO_TIME_TABLE_s *p = NULL;
	struct BT656ENC_TIME_TABLE_s *pBt656Scaler;
	BYTE idx_dec;

	Printf("\nBT656Enc_Setup_Dec VideoInputMain:%bd VideoInputSub:%bd",VideoInputMain,VideoInputSub);

	//BT656Enc Input Crop for Decoder Input.
	//Decoder input uses an overscan value, but, BT656Enc needs a normal crop value.
	pVideoTable = &TW8836_DEC_TABLE[VideoInputSub];
	PrintCbvsVideoTimeTable(pVideoTable);

	hDelay =  pVideoTable->hDelay + pVideoTable->hOverScan;
	hActive = pVideoTable->hActive;
	vDelay  = pVideoTable->vDelay; 
	vActive = pVideoTable->vActive;
	vFreq   = pVideoTable->vFreq;

	DecoderSetOutputCrop(hDelay, hActive, vDelay, vActive);
#ifdef DEBUG_BT656
	PrintDecoderOutputCrop("BT656Enc", hDelay, hActive, vDelay, vActive);
#endif
	p = Find_CEA861_VideoTable(hActive*2,vActive,vFreq);
	if(p == NULL) {
		Printf("\nFail Find_CEA861_VideoTable %dx%d@%bd",hActive*2,vActive,vFreq);
		return NULL;
	}
	//Read Scale option. Bit0 is for 2DDI option.
	fScale = EE_Read(EEP_BT656ENC_DEC);
	idx_dec=FindBT656ScalerTable(BT656SCALE_DEC_OPTION_table,hActive,vActive,vFreq,fScale);
	if(idx_dec==0xFF) {
		Puts("\nFail FindBT656ScalerTable");
		return NULL;
	}	
	pBt656Scaler = &BT656SCALER_time[idx_dec];
	G_pBt656Scaler = pBt656Scaler;
	if(fScale & 0x01) {
		//2D-DI
		BT656Enc_En2DDI(ON);	//en 2d-di
		BT656Enc_2DDI_FieldOffset(0,1); //even offset needs "1".
		BT656_A_Output(BT656_A_OUT_DEC_DI,0,0);
	}
	else {
		BT656_A_Output(BT656_A_OUT_DEC_I,0,0);
	}
	OverWriteBt656ScalerTable(pBt656Scaler, 0); //step0.

	//
	//Your output can be scaled down.
	//Find VideoTable again
	//
	vActive = BT656Enc_Calc_vActive(vActive, pBt656Scaler->i_vScale);
	p = Find_CEA861_VideoTable(hActive*2,vActive,vFreq);
	if(p)
		PrintCEAVideoTimeTable(p);

	//update BT656 External Encoder
	BT656Enc_Update_BT656ExternalEncoder(hActive,vActive);

	return p;	
}

#endif

/**
 * Component option
 * Use EEPROM[0x22]
 */

#define BT656SCALER_IDX_COMP_1080I50 0xFF
#define BT656SCALER_IDX_COMP_1080I60 0xFF
#define BT656SCALER_IDX_COMP_720P50_480P 0xFF
#define BT656SCALER_IDX_COMP_720P60_480P 0xFF

#if defined(SUPPORT_ARGB)
struct BT656SCALE_OPTION_s BT656SCALE_COMP_OPTION_table[] = {
	{720,240,60,	BT656SCALER_IDX_COMP_480I,			BT656SCALER_IDX_COMP_480P, 0xFF},
	{720,288,50,	BT656SCALER_IDX_COMP_576I,			BT656SCALER_IDX_COMP_576P, 0xFF},
	{720,480,60,	BT656SCALER_IDX_COMP_480P,			0xFF,0xFF},
	{720,576,50,	BT656SCALER_IDX_COMP_576P,			0xFF,0xFF},
	{1080,540,50,	BT656SCALER_IDX_COMP_1080I50_QHD,	BT656SCALER_IDX_COMP_1080I50, 0xFF},
	{1080,540,60,	BT656SCALER_IDX_COMP_1080I60_QHD,	BT656SCALER_IDX_COMP_1080I60, 0xFF},
	{1280,720,50,	BT656SCALER_IDX_COMP_720P50,		BT656SCALER_IDX_COMP_720P50_480P, 0xFF},
	{1280,720,60,	BT656SCALER_IDX_COMP_720P60,		BT656SCALER_IDX_COMP_720P60_480P, 0xFF},
	{1920,1080,50,	BT656SCALER_IDX_COMP_1080P50_QHD,	0xFF, 0xFF},
	{1920,1080,60,	BT656SCALER_IDX_COMP_1080P60_QHD,	0xFF, 0xFF},
	{0,}
};
#endif
/**
 * Setup Component Input
 *  
 * Input
 *	VideoInputSub
 * Output
 *	G_pBt656Scaler
 */

#if defined(SUPPORT_BT656)
#if defined(SUPPORT_ARGB)
struct DIGIT_VIDEO_TIME_TABLE_s * BT656Enc_Setup_aRGB_COMP(void)
{
	BYTE fScale;
	BYTE VideoInputSub = InputSubMode;
	struct DIGIT_VIDEO_TIME_TABLE_s *p = NULL;
	struct COMP_VIDEO_TIME_TABLE_s *pVideoTable;
	struct BT656ENC_TIME_TABLE_s *pBt656Scaler;
	WORD hDelay,hActive, vDelay, vActive;
	BYTE vFreq;
	BYTE idx_comp;

	fScale = EE_Read(EEP_BT656ENC_ARGB_COMP);

	//BUG. InputSubMode is a HW value. I need a SW value; nput_aRGBMode	
	VideoInputSub = Input_aRGBMode;

	//adjust
	pVideoTable = &TW8836_COMP_TABLE[VideoInputSub];
	PrintCompVideoTimeTable(pVideoTable);
	hActive = pVideoTable->hActive;
	vActive = pVideoTable->vActive;
	vFreq   = pVideoTable->vFreq;

	hDelay  = pVideoTable->hBPorch + pVideoTable->hSync;
	vDelay  = pVideoTable->vBPorch-1;  // + pVideoTable->vSync;
	PrintMeasAdjValue("BT656Enc", hDelay,vDelay);

	p = Find_CEA861_VideoTable(hActive,vActive,vFreq);
	if(p==NULL) {
		Printf("\nFail Find_CEA861_VideoTable %dx%d@%bd",hActive,vActive,vFreq);
		return NULL;
	}
	idx_comp = FindBT656ScalerTable(BT656SCALE_COMP_OPTION_table,hActive,vActive,vFreq,fScale);
	if(idx_comp==0xFF) {
		Puts("\nFail FindBT656ScalerTable");
		return NULL;
	}
	pBt656Scaler = &BT656SCALER_time[idx_comp];
	G_pBt656Scaler = pBt656Scaler;
	BT656_A_Output(BT656_A_OUT_DEC_ADC, 0,0);
	OverWriteBt656ScalerTable(pBt656Scaler, 0); //step0.

	//
	//Your output can be scaled down.
	//Find VideoTable again
	//
	hActive = BT656Enc_Calc_hActive(hActive, pBt656Scaler->i_hScale);
	vActive = BT656Enc_Calc_vActive(vActive, pBt656Scaler->i_vScale);
	 
	p = Find_CEA861_VideoTable(hActive,vActive,vFreq);
	if(p)
		PrintCEAVideoTimeTable(p);

 	//update BT656 External Encoder
	BT656Enc_Update_BT656ExternalEncoder(hActive,vActive);

	return p;
}
#endif
#endif

#if defined(SUPPORT_ARGB)
struct BT656SCALE_OPTION_s BT656SCALE_PC_OPTION_table[] = {

	{640,480,60,	BT656SCALER_IDX_PC_0660,			0xFF, 0xFF},
	{800,600,60,	BT656SCALER_IDX_PC_0860,			0xFF, 0xFF},
	{1024,768,60,	BT656SCALER_IDX_PC_1060,			0xFF,0xFF},
	{1280,960,50,	BT656SCALER_IDX_PC_126A_1024X640,	0xFF,0xFF},
	{0,}
};
#endif

#if defined(SUPPORT_BT656)
#if defined(SUPPORT_ARGB)
struct DIGIT_VIDEO_TIME_TABLE_s *  BT656Enc_Setup_aRGB_PC(void)
{
	WORD hDelay,hActive, vDelay, vActive;
	BYTE VideoInputSub = InputSubMode;
	BYTE vFreq;
	struct DIGIT_VIDEO_TIME_TABLE_s *p = NULL;
	BYTE fScale;
	BYTE idx_pc;

	struct DIGIT_VIDEO_TIME_TABLE_s *pVideoTable;
	struct BT656ENC_TIME_TABLE_s *pBt656Scaler;


	fScale = EE_Read(EEP_BT656ENC_ARGB_PC);

	pVideoTable = &TW8836_VESA_TABLE[VideoInputSub];
	PrintVesaVideoTimeTable(pVideoTable);

	hActive = pVideoTable->hActive;
	vActive = pVideoTable->vActive;
	vFreq   = pVideoTable->vFreq;

	hDelay  = pVideoTable->hBPorch + pVideoTable->hSync;
	vDelay  = pVideoTable->vBPorch+1+ pVideoTable->vSync;
	PrintMeasAdjValue("BT656Enc", hDelay,vDelay);

	p = Find_CEA861_VideoTable(hActive,vActive,vFreq);
	if(p==NULL) {
		Printf("\nFail Find_CEA861_VideoTable %dx%d@%bd",hActive,vActive,vFreq);
		return NULL;
	}
	idx_pc = FindBT656ScalerTable(BT656SCALE_PC_OPTION_table,hActive,vActive,vFreq,fScale);
	if(idx_pc==0xFF) {
		Puts("\nFail FindBT656ScalerTable");
		return NULL;
	}
	pBt656Scaler = &BT656SCALER_time[idx_pc];
	G_pBt656Scaler = pBt656Scaler;
	BT656Enc_D_SetRGB(ON);
	BT656_A_Output(BT656_A_OUT_DEC_ADC, 0,0);
	OverWriteBt656ScalerTable(pBt656Scaler,0); //step0.

	return p;
}
#endif
#endif


#if defined(SUPPORT_BT656)

#define BT656SCALER_IDX_DTV_1080I50 0xFF
#define BT656SCALER_IDX_DTV_1080I60 0xFF
#define BT656SCALER_IDX_DTV_720P50_480P 0xFF

struct BT656SCALE_OPTION_s BT656SCALE_DTV_OPTION_table[] = {

	{720,240,60,	BT656SCALER_IDX_DTV_480I,			BT656SCALER_IDX_DTV_480P, 0xFF},
	{720,288,50,	BT656SCALER_IDX_DTV_576I,			BT656SCALER_IDX_DTV_576P, 0xFF},
	{640,480,60,	BT656SCALER_IDX_DTV_0660,			0xFF,0xFF},
	{720,480,60,	BT656SCALER_IDX_DTV_480P,			BT656SCALER_IDX_DTV_480P,0xFF},
	{720,576,50,	BT656SCALER_IDX_DTV_576P,			BT656SCALER_IDX_DTV_576P,0xFF},
	{1920,540,50,	BT656SCALER_IDX_DTV_1080I50_QHD,	BT656SCALER_IDX_DTV_1080I50, 0xFF},
	{1920,540,60,	BT656SCALER_IDX_DTV_1080I60_QHD,	BT656SCALER_IDX_DTV_1080I60, 0xFF},
	{800,600,60,	BT656SCALER_IDX_DTV_0860,			0xFF, 0xFF},
	{1280,720,50,	BT656SCALER_IDX_DTV_720P50,			BT656SCALER_IDX_DTV_720P50_480P, 0xFF},
	{1280,720,60,	BT656SCALER_IDX_DTV_720P60,			BT656SCALER_IDX_DTV_720P60_480P, 0xFF},
	{1024,768,60,	BT656SCALER_IDX_DTV_1060,			0xFF, 0xFF},
	{1280,960,60,	BT656SCALER_IDX_DTV_126A_1024X640,	0xFF, 0xFF},
	{1920,1080,50,	BT656SCALER_IDX_DTV_1080P50_QHD,	0xFF, 0xFF},
	{1920,1080,60,	BT656SCALER_IDX_DTV_1080P60_QHD,	0xFF, 0xFF},
	{0,}
};
#endif


#if defined(SUPPORT_BT656)
struct DIGIT_VIDEO_TIME_TABLE_s *  BT656Enc_Setup_DTV(void)
{
	WORD hDelay,hActive, vDelay, vActive;
	BYTE vFreq;
	struct DIGIT_VIDEO_TIME_TABLE_s *p = NULL;
	BYTE fScale;
	BYTE idx_dtv;
	struct BT656ENC_TIME_TABLE_s *pBt656Scaler;

	fScale = EE_Read(EEP_BT656ENC_DTV_TV);

	//digital

	//
	//Measure input signal
	//
	Meas_StartMeasure();
	Meas_IsMeasureDone(50);
	hActive = MeasGetHActive( &hDelay );
	vActive = MeasGetVActive( &vDelay );
	vFreq = MeasGetVFreq();
	Printf("\n\rBT656Enc Meas %d hCropStart:%d+4",hActive, hDelay);
	Printf("\n\r              %d vCropStart:%d-1",vActive, vDelay);

	//
	//Find video timing table
	//& use the table value. 
	p = Find_CEA861_VideoTable(hActive,vActive,vFreq);
	if(p==NULL) {
		Printf("\nFail Find_CEA861_VideoTable %dx%d@%bd",hActive,vActive,vFreq);
		//..
		//..please giveup....
		return NULL;
	}
	PrintCEAVideoTimeTable(p);
	//
	//update value
	hActive = p->hActive;
	vActive = p->vActive;
	hDelay = p->hSync + p->hBPorch;
	vDelay = p->vSync + p->vBPorch;
	idx_dtv=FindBT656ScalerTable(BT656SCALE_DTV_OPTION_table,hActive,vActive,vFreq,fScale);
	if(idx_dtv==0xFF) {
		Puts("\nFail FindBT656ScalerTable");
		//now, we are failed,  just setup some default...
		//..
		//..please giveup....
		return NULL;
	}	
	pBt656Scaler = &BT656SCALER_time[idx_dtv];
	G_pBt656Scaler = pBt656Scaler;
	BT656Enc_D_SetRGB(ON);
	BT656_A_Output(BT656_A_OUT_DEC_ADC, 0,0);
	OverWriteBt656ScalerTable(pBt656Scaler, 0); //step0.

	//
	//Your output can be scaled down.
	//Find VideoTable again
	//
	hActive = BT656Enc_Calc_hActive(hActive, pBt656Scaler->i_hScale);
	vActive = BT656Enc_Calc_vActive(vActive, pBt656Scaler->i_vScale);

	p = Find_CEA861_VideoTable(hActive,vActive,vFreq);
	if(p)
		PrintCEAVideoTimeTable(p);

	//update BT656 External Encoder
	BT656Enc_Update_BT656ExternalEncoder(hActive,vActive);

	return p;
}
#endif

struct DIGIT_VIDEO_TIME_TABLE_s * BT656Enc_Setup_PANEL(void)
{
	struct DIGIT_VIDEO_TIME_TABLE_s *p = NULL;

	BT656Enc_D_SetRGB(ON);
	BT656Enc_Crop(54, 24, PANEL_H, PANEL_V);

	if(PANEL_H==1280 && PANEL_V==800) {
		BT656Enc_SetScaler(0x0AAA,2);
		BT656Enc_SetOutputActiveLen(40,1678); //??
		BT656Enc_GenLock(8, GENLOCK_X4); //PreDiv:8,x4

		p = Find_CEA861_VideoTable(800,400,60);
	}
	else if(PANEL_H==1024 && PANEL_V==600) {
		BT656Enc_SetScaler(0x0AAA,9);
		BT656Enc_SetOutputActiveLen(40,1678); //??
		BT656Enc_GenLock(8, GENLOCK_X4); //PreDiv:8,x4
		p = Find_CEA861_VideoTable(800,400,60);
	}
	else { /* assume 800x480 */
		BT656Enc_SetScaler(0x0AAA,9);
		BT656Enc_SetOutputActiveLen(40,1678); //??
		BT656Enc_GenLock(8, GENLOCK_X4); //PreDiv:8,x4
		p = Find_CEA861_VideoTable(800,480,60);
	}

	return p;
}



/**
* set BT656Encoder
* extern InputMain, InputSubMode
*/
#if defined(SUPPORT_BT656)

struct DIGIT_VIDEO_TIME_TABLE_s *BT656Enc_Setup(BYTE BT656EncSource)
{
	BYTE VideoInputMain= InputMain;
	BYTE VideoInputSub = InputSubMode;
	struct DIGIT_VIDEO_TIME_TABLE_s *p = NULL;

	if(BT656EncSource > BT656ENC_SRC_PANEL) {
		BT656Enc_Enable(OFF);
		return NULL;
	}
	G_pBt656Scaler = NULL;

	//set default....temp
	BT656Enc_Setup_Init(BT656EncSource);

	p = NULL;
	if(BT656EncSource==BT656ENC_SRC_DEC)  
		p = BT656Enc_Setup_Dec();
#if defined(SUPPORT_ARGB)
	else if(BT656EncSource==BT656ENC_SRC_ARGB) {
		if(VideoInputMain==INPUT_COMP)	
			p = BT656Enc_Setup_aRGB_COMP();
		else							
			p = BT656Enc_Setup_aRGB_PC();
	}
#endif
	else if(BT656EncSource==BT656ENC_SRC_DTV)
		BT656Enc_Setup_DTV(); //Note:Do not update p.
	else if(BT656EncSource==BT656ENC_SRC_LVDS)
		p = BT656Enc_Setup_DTV();
	else
		BT656Enc_Setup_PANEL(); //Note:Do not update p.


	//P is for loopback...
	return p;
}
#endif


/**
* set BT656Decoder for loopback
*/

#if 0
	//now valid inputs are BT656ENC_SRC_DEC,BT656ENC_SRC_ARGB,BT656ENC_SRC_LVDS
void MeasureAndSetBt656Dec(BYTE BT656EncSource)
{
//	BYTE dtv_route, dtv_format;
	WORD hCropStart,vCropStart;
	WORD hActive,vActive;
//	WORD hDelay,vDelay;
	BYTE vFreq;


	WORD wTemp;
//	BYTE input_format;
	BYTE VideoInputMain= InputMain;
	BYTE VideoInputSub = InputSubMode;
	struct DIGIT_VIDEO_TIME_TABLE_s *p;
//	BYTE fScale;
	//struct BT656ENC_TIME_TABLE_s *pBT;
	struct BT656ENC_TIME_TABLE_s *pBt656Scaler=G_pBt656Scaler;
	

	//read measure with DE.
	MeasEnableDeMeasure(ON);
	Meas_StartMeasure();
	if(Meas_IsMeasureDone(50)) {
		Printf("\n\rMeasure Failed....");
		return;
	}
	delay1ms(500);
	Meas_StartMeasure();
	Meas_IsMeasureDone(50);

	hActive = MeasGetHActive( &hCropStart );
	vActive = MeasGetVActive( &vCropStart );
	vFreq = MeasGetVFreq();
	Printf("\n\rBT656Dec Meas %d hCropStart:%d+4",hActive, hCropStart);
	Printf("\n\r              %d vCropStart:%d-1",vActive, vCropStart);
	p = Find_CEA861_VideoTable(hActive,vActive,vFreq);
	if(p==NULL) {
		Printf("\nNo CEA861 Video Table");
	}
	//now, i assume we have a valid p.

	//setup scaler
	if(BT656EncSource==BT656ENC_SRC_DEC) {
		//----------------------------
		//decoder uses measured value.
		//----------------------------
		if(vActive==288) {
			//unstable ???. Read assgined value.
			vCropStart = ReadTW88(REG108);
			Printf("=>%d+1",vCropStart);	
		}
		//Printf("\n\rBT656Dec Crop %d hCropStart:%d",hActive, hCropStart+4);
		//Printf("\n\r              %d vCropStart:%d",vActive+1, vCropStart-1+2);

		InputSetCrop(hCropStart+4, vCropStart-1+2, hActive, vActive+1);
#ifdef DEBUG_BT656
		PrintScalerInputCrop("BT656Dec",hCropStart+4, vCropStart-1+2, hActive, vActive+1);
#endif
		ScalerSetHScale(hActive / 2);
		ScalerSetVScale(vActive);
		ScalerSetLineBufferSize(hActive / 2);
		wTemp = ScalerCalcVDE2(vCropStart-1+2,-1);
		ScalerWriteVDEReg((BYTE)wTemp);
		//BK131011
		//or use 
		//ScalerSet_vDE_value(vCropStart-1+2);
	}
	else
	if(BT656EncSource==BT656ENC_SRC_ARGB && VideoInputMain==INPUT_COMP) {
		//----------------------------
		//component uses measured value.
		//----------------------------

		//hCropStart += 4;
		//vCropStart += 1; //(-1+2)

		//Printf("\n\rBT656Dec ICrop %d*2+1 hCropStart:%d+4",p->hActive, hCropStart);
		//Printf("\n\r               %d+1 vCropStart:%d+1",p->vActive, vCropStart);

		InputSetCrop(
			hCropStart + 4, //hSync+hBPorch
			vCropStart + 1,	//vSync+vBPorch +2
			p->hActive *2 +1,
			p->vActive +1);
#ifdef DEBUG_BT656
		PrintScalerInputCrop("BT656Dec",hCropStart + 4, vCropStart + 1,	p->hActive *2 +1,p->vActive +1); 
#endif
		ScalerSetHScale(p->hActive); 
		ScalerSetVScale(p->vActive); 
		ScalerSetLineBufferSize(p->hActive); 
		ScalerSet_vDE_value(vCropStart);
	}
	else
	if(BT656EncSource==BT656ENC_SRC_ARGB && VideoInputMain==INPUT_PC) {
		//----------------------------
		//PC uses measured value.
		//----------------------------
		struct DIGIT_VIDEO_TIME_TABLE_s *pVideoTable;
		pVideoTable = &TW8836_VESA_TABLE[VideoInputSub];
		vCropStart = pVideoTable->vSync+pVideoTable->vBPorch  + 1;

		//Printf("\n\rBT656Dec Crop %d hCropStart:%d",hActive, hCropStart+4);
		//Printf("\n\r              %d vCropStart:%d",vActive+1, vCropStart-1+2);

		//Input_aRGBMode 
		InputSetCrop(
			hCropStart + 4*2+10,  		//aRGB_delay(4)*2 + BT656_delay(10) 
			vCropStart - 14,
			pVideoTable->hActive*2+1, 
			pVideoTable->vActive+2);
#ifdef DEBUG_BT656
		PrintScalerInputCrop("BT656Dec",hCropStart + 4*2+10,vCropStart - 14,pVideoTable->hActive*2+1,pVideoTable->vActive+2);
#endif

		ScalerSetHScale(pVideoTable->hActive);
		ScalerSetVScale(pVideoTable->vActive);
		ScalerSetLineBufferSize(pVideoTable->hActive+6);
#if 0 //BK131011
		wTemp = (vCropStart-14) * PANEL_V / pVideoTable->vActive + 2;
		ScalerWriteVDEReg(wTemp);
#else
		ScalerSet_vDE_value(vCropStart -14);
#endif
	}
	else //assume (BT656EncSource==BT656ENC_SRC_LVDS) 
	{		
		//Printf("\n\rBT656Dec Crop %d hCropStart:%d",p->hActive * 2, hCropStart+4);
		//Printf("\n\r              %d vCropStart:%d",p->vActive, vCropStart-1+2);
		//----------------------------
		//LVDS(digital) can use a video table.
		//----------------------------
		InputSetCrop(
			hCropStart + 4, 
			vCropStart + 1,
			p->hActive * 2,
			p->vActive);
#ifdef DEBUG_BT656
		PrintScalerInputCrop("BT656Dec", hCropStart + 4,vCropStart + 1,	p->hActive * 2,	p->vActive);
#endif
		ScalerSetHScale(p->hActive); 
		ScalerSetVScale(p->vActive); 
		ScalerSetLineBufferSize(p->hActive); 
		ScalerSet_vDE_value(vCropStart);
	}
}
#endif

#if defined(SUPPORT_BT656)
BYTE BT656Dec_LoopBack(BYTE BT656EncSource, struct DIGIT_VIDEO_TIME_TABLE_s *_p)
{
	BYTE dtv_route, dtv_format;
	BYTE input_format;
	BYTE VideoInputMain= InputMain;
	BYTE VideoInputSub = InputSubMode;
	struct DIGIT_VIDEO_TIME_TABLE_s *p=_p;
	struct BT656ENC_TIME_TABLE_s *pBt656Scaler=G_pBt656Scaler;

//-------------
// add
//			bTemp = DtvFindFieldWindow(pCEA861->hTotal / 2);
//			DtvSetFieldDetectionRegion(ON, bTemp);
//-------------

	//if PANEL or DTV, we can not make a loop.
	if(BT656EncSource >= BT656ENC_SRC_PANEL)
		return 1;
	if(BT656EncSource == BT656ENC_SRC_DTV)
		return 1;

	if(pBt656Scaler->pol & POL_INTERLACED)
		dtv_format = DTV_FORMAT_INTERLACED_ITU656;
	else		 
		dtv_format = DTV_FORMAT_PROGRESSIVE_ITU656;
	input_format = INPUT_FORMAT_YCBCR;
	dtv_route = DTV_ROUTE_PbYPr;
	DtvSetRouteFormat(dtv_route, dtv_format);
	DtvSetReverseBusOrder(ON);

	Bt656DecSetClkPol(pBt656Scaler->pol & POL_DTVCLK);


	//turn off Video Signal Interrupt.
	//I will trun on later.
	Interrupt_enableVideoDetect(OFF);

	//change input to DTV for BT656Dec.
	//and use DTVCLK2.
	InputSetSource(INPUT_PATH_DTV,input_format);
	WriteTW88(REG040,ReadTW88(REG040) | 0x04);

	OverWriteBt656ScalerTable(pBt656Scaler, 2); //step2.
#if 0	
	//measure & setup scaler
	MeasureAndSetBt656Dec(BT656EncSource);
#endif
	//turn on Video Signal Interrupt
	Interrupt_enableVideoDetect(ON);			
	//skip Interrupt_enableSyncDetect(ON);

	return 0;
}
#endif

/*
* Change InputMain & BT656 Mux.
* usage: iBT656 {LOOP|PANEL|DEC|ARGB|DTV|LVDS|OFF}
* It will call SetBT656Output() at the end of this procedure.
*
* @param mode
*	BT656ENC_SRC_PANEL	
*	BT656ENC_SRC_DEC	
*	BT656ENC_SRC_ARGB	
*	BT656ENC_SRC_DTV	
*	BT656ENC_SRC_LVDS	
*	BT656ENC_SRC_OFF	
*	BT656ENC_SRC_AUTO	
*/
void ChangeBT656__MAIN(BYTE mode)
{
	BYTE bt656_src_mode;
	BYTE ret;
	struct DIGIT_VIDEO_TIME_TABLE_s *p;

	Printf("\n\rChangeBT656__MAIN(%bd)",mode);
	bt656_src_mode = BT656_CheckLoopbackCombination(mode);
	if(bt656_src_mode == 0xFF) {
		//maybe OFF, or something wrong.
		return;
	}
	//setup BT656Enc
#if defined(SUPPORT_BT656)
	p = BT656Enc_Setup(bt656_src_mode);
	if(p == NULL)
		return;
#endif
	//setup BT656Dec to support LOOPBACK mode.
#if defined(SUPPORT_BT656)
	ret = BT656Dec_LoopBack(bt656_src_mode,p);
#endif
}

BYTE BT656Enc_Info(void)
{
	BYTE bTemp;

	bTemp = ReadTW88(REG007);
	Printf("\n\rBT656 %s", bTemp & 0x08 ? "On" : "Off");
	if(bTemp & 0x08 ==0)
		return 1;

	bTemp = ReadTW88(REG067) & 0x07;
	switch(bTemp) {
	case 0:
		Printf("\n\rDecoder");
		break;
	case 1:
		Printf("\n\raRGB");
		break;
	case 2:
		Printf("\n\rDTV");
		break;
	case 3:
		Printf("\n\rLVDS-Rx");
		break;
	case 4:
		Printf("\n\rPanel");
		break;
	default:
		Printf("\n\runknown:%bx",bTemp);
		//add some dummy code.
		ScalerReadHDEReg();
		ScalerReadLineBufferDelay();
		BT656_A_SelectCLKO(1,OFF);
		BT656_A_SetLLCLK_Pol(OFF);
		BT656Enc_En2DDI(ON);
//		BT656_A_DeInterlace_Set(29, 97);
		BT656_A_SelectOutput(1, 0/*HPol*/, 0/*VPol*/, 0);
		InputGetHLen();
		InputGetVLen();

		return 2;
	}
	return 0;		
}

