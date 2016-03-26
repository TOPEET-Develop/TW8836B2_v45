/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/
#ifndef _INPUTCTRL_H_
#define _INPUTCTRL_H_

#define	INPUT_CVBS		0   //composite
#define	INPUT_SVIDEO	1	//Y & C
#define INPUT_COMP		2	//INPUT_YUV. analog. SOG
#define INPUT_PC		3	//INPUT_RGB	 RGB with HSYNC & VSYHC
#define	INPUT_DVI		4	
#define INPUT_HDMIPC	5
#define INPUT_HDMITV	6
#define	INPUT_BT656		7	//BT656Loopback. 8bit
#define INPUT_LVDS		8
#define INPUT_TOTAL		9

#define	INPUT_CHANGE_DELAY	300
#define	INPUT_CHANGE_DELAY_BASE10MS	30

extern XDATA	BYTE	InputMain;
extern XDATA	BYTE	InputBT656;
extern XDATA	BYTE	InputSubMode;
extern XDATA	BYTE	OldInputMain;


//--------------------------------
// input module
//--------------------------------
BYTE GetInputMain(void);			//friend
void SetInputMain(BYTE input);		//friend..only update InputMain global variable
void InitInputAsDefault(void);


//scaler input
#define INPUT_PATH_DECODER	0x00
#define INPUT_PATH_VADC		0x01
#define INPUT_PATH_DTV		0x02
#define INPUT_PATH_LVDS		0x03	//LVDS_RX
#define INPUT_PATH_BT656	0x06	//DTV+2ndDTV_CLK

#define INPUT_FORMAT_YCBCR	0
#define INPUT_FORMAT_RGB	1
void InputSetSource(BYTE path, BYTE format);
void InputSetClockPolarity(BYTE fInv);
void InputSetFieldPolarity(BYTE fInv);
void InputSetProgressiveField(BYTE fOn);
void InputSetCrop( WORD x, WORD y, WORD w, WORD h );
void InputSetHStart( WORD x);
void InputSetVStart( WORD y);
WORD InputGetHStart(void);
WORD InputGetVStart(void);
WORD InputGetHLen(void);
WORD InputGetVLen(void);
void InputSetPolarity(BYTE V,BYTE H, BYTE F);

void BT656DecSetFreerunClk(BYTE fFreerun, BYTE fInvClk);
void Bt656DecSetClkPol(BYTE fInvClk);
void BT656DecSetFreerun(BYTE fOn);

void LvdsRxEnable(BYTE fOn);
void LvdsRxPowerDown(BYTE fOn);
void InitLvdsRx(void);


void PrintfInput(BYTE Input, BYTE debug);
void PrintfBT656Input(BYTE Input, BYTE debug);


void ChangeInput( BYTE mode );
void InputModeNext( void );

BYTE CheckInput( void );

//void SetDefault_Decoder(void);
void VInput_enableOutput(BYTE fRecheck);
void VInput_gotoFreerun(BYTE reason);


struct REG_BUFF_INFO_s {
	WORD reg;	//TW8836 register index
	BYTE idx;	//Buffer index
};

#if defined(SUPPORT_FAST_INPUT_TOGGLE)
extern BYTE Fast_CVBS_Toggle_Buff[11];
extern BYTE Fast_DTV_Toggle_Buff[11];
extern CODE struct REG_BUFF_INFO_s Fast_VIDEO_Toggle_info[];
void ChangeFastInputMain(void);
#endif


#endif
