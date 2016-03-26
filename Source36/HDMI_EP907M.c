/**
 * @file
 * HDMI_EP907M.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	HDMI EP9553E device driver


 [KL_166]

HDCP Key download from MCU
HDMI Controller Version 0.26
---------------------------
HDMI Auto, 480P,576P,720P@50, 720P@60,1080i@50,1080i@60,1080P@60

BB_EP9553E-130424-TW8836.hex

 [KL_166 130424]

HDCP Key download from MCU
HDMI(EP9553E) Controller Version 0.26

HDMI Auto, NTSC,PAL, 480P,576P,720P@50, 720P@60,1080i@50,1080i@60,1080P@60


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
#include "typedefs.h"
#include "TW8836.h"

#include "Global.h"
#include "cpu.h"
#include "printf.h"
#include "util.h"

#include "i2c.h"

#include "main.h"
#include "dtv.h"

#include "EP907M_RegDef.h"
//#include "HDMI_EP9553E.h"
#include "EP9553_RegDef.h"

#include "HDMI_EP907M.h"


#ifndef SUPPORT_HDMI_EP907M
//==========================================
//----------------------------
/**
* Trick for Bank Code Segment
*/
//----------------------------
CODE BYTE DUMMY_HDMI_EP907M_CODE;
void Dummy_HDMI_EP907M_func(void)
{
	BYTE temp;
	temp = DUMMY_HDMI_EP907M_CODE;
}
#else //..SUPPORT_HDMI_EP907M
//==========================================

/**
* EDID data
*/


/*	HDCP Program

	HDCP key Area	$00~$27 : encrypted 40 56-bit HDCP keys
	BKSV key Area   $28     : 40-bit BKSV

	$48,Byte0[2] = 1; //EE_DIS: disable HDCP key downloading from external EE. HDCP keys are written by MCU.
	download HDCP
*/

/**
* HDCP key data
*/

													   		



#ifdef UNCALLED_SEGMENT
#endif //..UNCALLED_SEGMENT





/**
* download EDID data from FW to EP9351 RAM
*
* On the develop
* @param addr. if 0, use HDMI_EDID_DATA[] on the code segment.
*/


/**
* download HDCP key from FW to EP9351 RAM
*
* On the develop
*/


/**
* init EP9351 HW.
*
* call from InitSystem.
* follow up the power up sequency. (curr, ignore)
* download EDID and HDCP.
* result:
*	PowerDown state & Mute state
*
* BugFix120802. Increase TempByte[6] to TempByte[20]
*/
//#if 0
//void Hdmi_SystemInit_EP907M(void)
//{}
//#endif
/**
* init EP9351 CHIP
* Power up, unmute and assign some default values.
*/
void HdmiInitEp907MChip(void)
{
	BYTE i;
	BYTE bTemp;

	//select PORT1.(HDMI port)
	//Printf("\n\rselect PORT1.(HDMI port)");
	Printf("\n\rUse KL_166 130123 or higher.");


	//check EP907M before we request something.
	for(i=0; i < 100; i++) {
		WriteI2C_multi(I2CID_EP907M,0x21, EP907M_System_Control, EP907M_System_Control__PORT_SEL__P2);

		//bTemp = ReadI2C_multi(I2CID_EP907M,0x21, 0x2100);
		//bTemp = ReadI2C_multi(I2CID_EP907M,0x21, 0x2100);
		delay1ms(10);
		bTemp = ReadI2C_multi(I2CID_EP907M,0x21, EP907M_System_Control);
		if((bTemp & 0x30) == 0x10)
			break;
		delay1ms(10);
	}
	Printf(" wait:%d", (WORD)i);
	//WriteI2C_multi(I2CID_EP907M,0x21, EP907M_System_Control, EP907M_System_Control__PORT_SEL__P2);

	//Audio. MCLK=128.
	//WriteI2C_multi(I2CID_EP907M,0x21, EP907M_Audio_Output_Control, EP907M_Audio_Output_Control__AAM_EN | 0x01);

	WriteI2C_multi(I2CID_EP907M,0x21, EP907M_Audio_Output_Control, EP907M_Audio_Output_Control__AAM_EN | 0x01);
#if defined(EVB_10)
	//this board prefers the falling dege DCLK.
	WriteI2C_multi(I2CID_EP907M,0x21, EP907M_Video_Output_Control, 0x00);	 
#endif
}

#if 1 //see BKFYI130224
#ifdef SUPPORT_HDMI_EP907M
/**
* check AVI InfoFrame
* 
* call when $3D[7:6] == 11b
*           $3C[4] == 1
* check $29[0] first.
*/
BYTE CheckAviInfoFrame(void)
{
	BYTE TempByte[15];
	BYTE bTemp;
	BYTE result;
	//------------------------------------
	//check AVI InfoFrame

	bTemp = ReadI2C_multi(I2CID_EP907M,0x21,EP907M_Interrupt_Flags);			//$29

#ifdef DEBUG_DTV
	Printf("\n\rCheckAviInfoFrame $0100:%02bx", bTemp);
#endif
	if((bTemp & 0x01) == 0) {
#ifdef DEBUG_DTV
		Puts(" FAIL");
#endif
		return ERR_FAIL;
	}

	//---------------------
	// found AVI InfoFrame.
	//---------------------
	
	//read AVI InfoFrame from $2A
	ReadI2CS_multi(I2CID_EP907M,0x21, EP907M_AVI_Information, TempByte, 6);
	//DBG_PrintAviInfoFrame();

	//---------------------
	//color convert to RGB
	//---------------------
	//	 Y [2][6:5]	   Input HDMI format
	//			0:RGB
	//			1:YUV(422)
	//			2:YUV(444)
	//			3:Unused
	bTemp = (TempByte[1] & 0x60) >> 5;
//#ifdef DEBUG_DTV
//	Puts("\n\rInput HDMI format ");
//	if(bTemp == 0) 		Puts("RGB");
//	else if (bTemp==1)  Puts("YUV(422)");
//	else if (bTemp==2)  Puts("YUV(444)");
//	else				Puts("unknown");
//#endif
	result = 0;
	if (bTemp==1)  		result = 0x50;
	else if (bTemp==2)  result = 0x10;

	// TempByte3[7:6] Colorimetry
	bTemp = (TempByte[2] & 0xc0)>>6;
	if(bTemp==2) result |= 0x04;	//BT.709
	//else if(bTemp==3) {
	//	//Extended Colorimetry Info
	//	i = TempByte[4]&0x70)>>4;
	//	...
	//}

	//	TempByte6[3:0] Pixel Repetition Factor
	//bTemp = TempByte[6] & 0x0F;
	//if(bTemp > 3) i = 0x03;	//EP9351 supports only 2 bits.
	//result |= bTemp;

//BK120731 test
//		TempBit03 =	(!TempBit04 && !(pEP9351C_Registers->Video_Status[0] & EP907M_Video_Status_0__VIN_FMT_Full_Range) || // Input RGB LR
//			 		 !TempBit05 && !(pEP9351C_Registers->Output_Format_Control & EP907M_Output_Format_Control__VOUT_FMT_Full_Range) );	// or Output RGB LR
//	result |= 0x08; //full range

	WriteI2C_multi(I2CID_EP907M,0x21, EP907M_Video_Format_Control, result);

	return ERR_SUCCESS;
}
#endif
#endif

BYTE HdmiCheckConnection(void)
{
//	BYTE ret;
//	BYTE i;

#ifdef DEBUG_DTV__TEST
	BYTE TempByte[8];
	WORD hFPorch,vFPorch;
	WORD hBPorch,vBPorch;
#endif
//	WORD hActive,vActive;
#if defined(SUPPORT_HDMI_EP907M)
#else
	WORD Old_hActive,Old_vActive;
#endif
//	BYTE vFreq;
		
//	WORD hCropStart,vCropStart;
	BYTE bTemp;
//	struct DIGIT_VIDEO_TIME_TABLE_s *pCEA861;
//	struct SCALER_TIME_TABLE_s *pScaler;

//	WORD hSync,vSync;
//	BYTE hPol,vPol;



	//read System Status register $201.
	//If it is too fast, VISR will take care.
	bTemp = ReadI2C_multi(I2CID_EP907M,0x21, EP907M_System_Status_1 );
	if((bTemp & 0xC0) != 0xC0) {
		Printf(" => NoSignal");
#ifdef DEBUG_DTV
		dPrintf(" $201:%bx",bTemp);
#endif
		return ERR_FAIL;
	}
	return 0;
}
/**
* @return 
*	1:hdmi
*	0:dvi
*/
BYTE HdmiCheckMode(void)
{
	BYTE i;
	BYTE bTemp;

	//read System Status register $200.
	//if $200[4]==1, HDMI mode.
	//actually, ...
	for(i=0; i < 10; i++) {
		bTemp = ReadI2C_multi(I2CID_EP907M,0x21, EP907M_System_Status_0 );
		if(bTemp & EP907M_System_Status_0__HDMI) {
			Puts(" HDMI mode");
			return 1; //it is a hdmi mode
		}
		delay1ms(10);
	}
	Puts(" DVI or VESA mode");
	return 0;
	//dPrintf(" $200:%bx @%bx",bTemp, i);
}
BYTE HdmiSetColorSpace(BYTE ColorSpace)
{
	WriteI2C_multi(I2CID_EP907M,0x21, EP907M_Video_Format_Control, ColorSpace ); 	//clear	
	return 0;
}

BYTE HdmiDebugTimingValue(void)
{
	BYTE TempByte[8];
	WORD hFPorch,vFPorch;
	WORD hSync,vSync;
	WORD hBPorch,vBPorch;
	WORD hActive,vActive;
		
	WORD hCropStart,vCropStart;
	BYTE bTemp;

	BYTE hPol,vPol;
	WORD vTotal,hTotal;

	//-----------------------------
	// read HSync timing register value, $0600~$0607.
	//-----------------------------
	ReadI2CS_multi(I2CID_EP907M,0x21, EP907M_Hsync_Timing, TempByte, 8);

	hActive = TempByte[0]; 	hActive <<= 8;		hActive += TempByte[1];
	hFPorch = TempByte[2]; 	hFPorch <<= 8;		hFPorch += TempByte[3];
	hBPorch = TempByte[4]; 	hBPorch <<= 8;		hBPorch += TempByte[5];
	hSync   = TempByte[6]; 	hSync <<= 8;		hSync   += TempByte[7];
	hTotal = hSync + hBPorch + hActive + hFPorch;

	//-----------------------------
	// read VSync timing register value, $0700~0704.
	//-----------------------------
	ReadI2CS_multi(I2CID_EP907M,0x21, EP907M_Vsync_Timing, TempByte, 5);
	vActive = TempByte[0]; 	vActive <<= 8;		vActive += TempByte[1];
	vFPorch =  TempByte[2];
	vBPorch =  TempByte[3];
	vSync =  TempByte[4]&0x7F;
	vTotal = vSync + vBPorch + vActive + vFPorch;

	//-----------------------------
	// read Polarity Control register. $2001
	// $2001[0] HS_POL
	//		1 = HSYNC pin is negative polarity (high during video active period).
	// $2001[1] VS_POL
	//		1 = VSYNC pin is negative polarity (high during video active period).
	//
	// I do not change $2001[1:0], so I assume, it is a 00b.
	//-----------------------------
	bTemp = ReadI2C_multi(I2CID_EP907M,0x21,EP907M_Polarity_Control);  /* NOTE: $41 */
	hPol = bTemp & EP907M_Polarity_Control__HS_POL_Low ? 0:1;		//ActiveHigh
	vPol = bTemp & EP907M_Polarity_Control__VS_POL_Low ? 0:1;		//ActiveHigh

	//select hStart, vStart for InputCrop.
	hCropStart = hBPorch;
	if(hPol)
		hCropStart += hSync;	
	vCropStart = vBPorch;
	if(vPol)
		vCropStart += vSync;


	//Printf("\n\rTimeReg %d hPol:%bx hSync:%d hBPorch:%d hCropStart:%d",hActive, hPol, hSync, hBPorch, hCropStart);
	//Printf("\n\r        %d vPol:%bx vSync:%d vBPorch:%d vCropStart:%d",vActive, vPol, vSync, vBPorch, vCropStart);	
	//

	Printf("\n\rTimeReg %dx%d Pol:%02bx ",hActive,vActive, hPol << 4 | vPol);
	Printf(" %d:%d,%d,%d",hTotal, hFPorch, hSync, hBPorch);
	Printf(" %d:%d,%d,%d",vTotal, vFPorch, vSync, vBPorch);
	Printf(" hStart:%d vStart:%d",hCropStart,vCropStart);

	//if source is 720x240, EP907M reports it as 1440x240.
	//I assume, 720x288, also same.
	if(hActive==1440) {
		if(vActive==240 || vActive==288) {
			hActive = 720;
			hCropStart >>=1;	 //div2
			Printf(" hActive:%d hStart:%d", hActive,hCropStart); 
		}
	}


	return 0;
}


#endif //..ifdef SUPPORT_HDMI_EP907M

