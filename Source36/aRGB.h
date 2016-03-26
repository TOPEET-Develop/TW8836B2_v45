/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/
#ifndef __VIDEO_ADC__
#define	__VIDEO_ADC__

#if !defined(SUPPORT_COMPONENT) && !defined(SUPPORT_PC)
//----------------------------
//Trick for Bank Code Segment
//----------------------------
void Dummy_ARGB_func(void);
#endif


extern XDATA	BYTE	Input_aRGBMode;


void aRGB_LLPLLSetDivider(WORD value, BYTE fInit);
WORD aRGB_LLPLLGetDivider(void);
void aRGB_SetLLPLLControl(BYTE value);

void aRGB_SetClampModeHSyncEdge(BYTE fOn);
void aRGB_SetClampPosition(BYTE value);

void aRGB_SetPhase(BYTE value, BYTE fInit); 	//WithInit
BYTE aRGB_GetPhase(void);
void aRGB_SetFilterBandwidth(BYTE value, WORD delay);

void aRGB_SetDefaultFor(void);
void aRGB_SetPolarity(BYTE fUseCAPAS);
BYTE aRGB_GetInputStatus(void);
void aRGB_AdjustPhase(BYTE mode);
void aRGB_setSignalPath(BYTE fInputPC);
//BYTE ConvertComponentMode2SW(BYTE mode);

void aRGB_COMP_set_scaler(BYTE mode, BYTE fBT656, BYTE fOverScan);
BYTE aRGB_LLPLLUpdateDivider(WORD divider, /*BYTE ctrl,*/ BYTE fInit, BYTE delay);




void AdjustPixelClk(WORD divider, BYTE mode );
void AdjustPixelClk_TEST(DWORD hPeriod);
void AutoColorAdjust(void);


BYTE CheckAndSetComponent( void );
BYTE ChangeComponent( void );


void aRGB_SetChannelGainReg(WORD GainG,WORD GainB,WORD GainR);
WORD aRGB_ReadGChannelGainReg(void);
WORD aRGB_ReadBChannelGainReg(void);
WORD aRGB_ReadRChannelGainReg(void);
BYTE CheckAndSetPC(void);
BYTE ChangePC( void );


//=============================================================================
//setup menu interface
//=============================================================================
extern void PCRestoreH(void);
extern void PCRestoreV(void);
extern void PCResetCurrEEPROMMode(void);


#endif //__VIDEO_ADC__
