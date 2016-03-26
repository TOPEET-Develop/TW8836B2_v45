/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/
#ifndef __BT656_H__
#define	__BT656_H__

#include "scaler.h"

#define BT656_INPUT_CVBS		0
#define BT656_INPUT_SVIDEO		1
#define BT656_INPUT_COMP		2
#define BT656_INPUT_PC			3
#define BT656_INPUT_DVI			4
#define BT656_INPUT_HDMIPC		5	//RGB
#define BT656_INPUT_HDMITV		6	//YUV
#define BT656_INPUT_LVDS		7
#define BT656_INPUT_PANEL		8

#define BT656ENC_SRC_DEC			0
#define BT656ENC_SRC_ARGB			1
#define BT656ENC_SRC_DTV			2
#define BT656ENC_SRC_LVDS			3
#define BT656ENC_SRC_PANEL			4
#define BT656ENC_SRC_OFF			5
#define BT656ENC_SRC_AUTO			6


#define BT656_A_OUPPUT_DATA_DEC_I	0
#define BT656_A_OUPPUT_DATA_DEC_P	1
#define BT656_A_OUPPUT_DATA_ADC		2


BYTE BT656Enc_Enable(BYTE fOn);
void BT656Enc_SetOutputClkPol(BYTE fActiveHigh);
void BT656Enc_SelectSource(BYTE fMode);
void BT656Enc_Crop(WORD h_SAV, WORD v_SAV, WORD HLen, WORD VLen);
void BT656Enc_SetScaler(WORD h, BYTE v);
void BT656Enc_SetOutputActiveLen(WORD hStart,WORD hLen);
void BT656Enc_SetVSyncDelay(BYTE delay);
void BT656Enc_GenLock(BYTE PreDiv, BYTE x8);


//void BT656_A_DeInterlace_Set(BYTE hdelay, BYTE hstart);
void BT656_A_SelectOutput(BYTE mode,BYTE Hpol,BYTE Vpol, BYTE hv_sel);
void BT656_A_SetLLCLK_Pol(BYTE pol);
void BT656_A_SelectCLKO(BYTE mode, BYTE YOut);
void BT656Enc_En2DDI(BYTE fOn);
void BT656Enc_D_SetRGB(BYTE fOn);
void BT656_A_Output(BYTE mode, BYTE hPol, BYTE vPol);

//void SetBT656Output(BYTE mode);

BYTE BT656Enc_Info(void);
struct DIGIT_VIDEO_TIME_TABLE_s * BT656Enc_Setup(BYTE BT656EncSource);
BYTE BT656Dec_LoopBack(BYTE BT656EncSource, struct DIGIT_VIDEO_TIME_TABLE_s *_p);
//BYTE BT656Dec_LoopBack(BYTE BT656EncSource);


void ChangeBT656__MAIN(BYTE mode);


#endif //..__BT656_H__
