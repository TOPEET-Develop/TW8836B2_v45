/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/
#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#define SSPLL_54M_REG	0x010000
#define SSPLL_72M_REG	0x015555
#define SSPLL_81M_REG	0x018000
#define SSPLL_90M_REG	0x01AAAB
#define SSPLL_105M_REG	0x01F1C7
#define SSPLL_108M_REG	0x020000
#define SSPLL_110M_REG	0x02097B	
#define SSPLL_120M_REG	0x0238E3
#define SSPLL_126M_REG	0x02557B
#define SSPLL_130M_REG	0x02684B
#define SSPLL_133M_REG	0x027685
#define SSPLL_135M_REG	0x028000
#define SSPLL_150M_REG	0x02C71C
#define SSPLL_160M_REG	0x02F684
#define SSPLL_MAX_REG	SSPLL_160M_REG
#define SSPLL_162M_REG	0x030000


extern CODE BYTE TW8835_GENERAL[];
extern CODE BYTE TW8835_SSPLL[];
extern CODE BYTE TW8835_DECODER[];
extern CODE BYTE TW8835_ADCLLPLL[];
//extern CODE BYTE TW8835_SCALER[];
extern CODE BYTE TW8835_TCON[];
extern CODE BYTE TW8835_GAMMA[];
extern CODE BYTE TW8835_EN_OUTPUT[];
//extern CODE BYTE TW8835_INIT_TCON_SCALER[];
//extern CODE BYTE TW8835_INIT_ADC_PLL[];


void SW_Reset_TW8836B(void);
void LV_Reset_TW8836B(void);


//---SSPLL------------------------------
void  Sspll1PowerUp(BYTE fOn);
void  Sspll1SetFreqReg(DWORD FPLL);
DWORD Sspll1GetFreq(void);
void  Sspll1SetFreq(DWORD freq, BYTE fControl);

void  Sspll2PowerUp(BYTE fOn);
void  Sspll2SetFreqReg(DWORD FPLL);
DWORD Sspll2GetFreq(void);
void  Sspll2SetFreq(DWORD freq, BYTE fControl);
void  BUGGY_PclkSetFunc(DWORD freq);  //please remove..

//--PCLK & PCLKO------------------------
void  PclkSetDividerReg(BYTE divider);
//void  PclkSetPolarity(BYTE pol);
DWORD PclkGetFreq(DWORD sspll);
DWORD PclkoGetFreq(DWORD pclk);
void  PclkoSetDiv(BYTE div);
BYTE  PclkoReadDiv(void);
void  PclkoSetDiv_with_pol(BYTE div);
void  PclkoPrintDiv(BYTE div);
void  PclkoSetPolarity(BYTE pol);


#define POL_PCLK 		0x80
#define POL_PCLKO		0x08

#define PCLK_DIV1		0x00			
#define PCLK_DIV15		0x10
#define PCLK_DIV2		0x20
#define PCLK_DIV25		0x30
#define PCLK_DIV3		0x40
#define PCLK_DIV35		0x50
#define PCLK_DIV4		0x60
#define PCLK_DIV5		0x70
/* note: N means PCLKO polarity */
//#define PCLK_DIV1_N		PCLK_DIV1	|	POL_PCLKO	
//#define PCLK_DIV15_N	PCLK_DIV15	|	POL_PCLKO	
//#define PCLK_DIV2_N		PCLK_DIV2	|	POL_PCLKO
//#define PCLK_DIV25_N	PCLK_DIV25	|	POL_PCLKO	
//#define PCLK_DIV3_N		PCLK_DIV3	|	POL_PCLKO	
//#define PCLK_DIV35_N	PCLK_DIV35	|	POL_PCLKO	
//#define PCLK_DIV4_N		PCLK_DIV4	|	POL_PCLKO
//#define PCLK_DIV5_N		PCLK_DIV5	|	POL_PCLKO

#define PCLKO_DIV1		0x00
#define PCLKO_DIV2		0x01
#define PCLKO_DIV3		0x02
#define PCLKO_DIV4		0x03
#define PCLKO_DIV15		0x06
#define PCLKO_DIV1_N	PCLKO_DIV1	| POL_PCLKO	
#define PCLKO_DIV2_N	PCLKO_DIV2	| POL_PCLKO	
#define PCLKO_DIV3_N	PCLKO_DIV3	| POL_PCLKO	
#define PCLKO_DIV4_N	PCLKO_DIV4	| POL_PCLKO	
#define PCLKO_DIV15_N	PCLKO_DIV15	| POL_PCLKO




//#define PCLKO_DIV1_POL	PCLKO_DIV1 | POL_PCLKO		

//void TW8836_PCLK_set(DWORD sspll_reg, BYTE pclk_div, BYTE pclko_div);
//BYTE TW8836_MCLK_set(BYTE mcuclk_sel, BYTE pllclk_sel, BYTE pllclk_div);

//---PllClk------------------------------
#define PLLCLK_SSPLL		0
#define PLLCLK_PLL108		1

#define PLLCLK_DIV_1P0		0
#define PLLCLK_DIV_1P5		1
#define PLLCLK_DIV_2P0		2
#define PLLCLK_DIV_2P5		3
#define PLLCLK_DIV_3P0		4
#define PLLCLK_DIV_3P5		5
#define PLLCLK_DIV_4P0		6
#define PLLCLK_DIV_5P0		7

void  PllClkSetSource(BYTE fPLL108);
BYTE  PllClkGetSource(void);

//void  PllClkSetSpiInputClockLatch(BYTE property);
void  PllClkSetDividerReg(BYTE divider);
void  PllClkSetSelDiv(BYTE ClkPllSel, BYTE ClkPllDiv);
DWORD PllClkGetFreq(void);


//---McuSpiClk---------------------------------

//extern BYTE shadow_r4e0;
//extern BYTE shadow_r4e1;
//BYTE McuSpiClkToPclk_Forced(BYTE divider); //testing
//void McuSpiClkRestore_Forced(void);		  //testing
//BYTE McuSpiClkReadSelectReg(void);
#define MCUSPI_CLK_27M		0
#define MCUSPI_CLK_32K		1
#define MCUSPI_CLK_PLLCLK	2

BYTE SpiClkReadSource(void);
void SpiClkSetSource(BYTE mode);
void SpiClkRecover27MSource(void);


//void McuSpiClkSet(BYTE McuSpiClkSel, BYTE ClkPllSel, BYTE ClkPllDiv); 
//void McuSpiClkSelect(BYTE McuSpiClkSel);
//DWORD McuGetClkFreq(void);

//--------------------
//
//--------------------
void SpiClk_SetSync(void);
void SpiClk_SetAsync(BYTE mcu_div, BYTE wait, BYTE fSpiOSD, BYTE fMcuFetch);
BYTE SpiClk_GetMinAsyncWaitValue(BYTE mcu_div);
BYTE SpiClk_overclocking(BYTE fOn);
DWORD SpiClkGetFreq(void);
DWORD McuClkGetFreq(DWORD spiclk);

void aRGBSetClockSource(BYTE use_27M);
//void SetDefaultPClock(void);

void DumpClock(void);
void DumpRegister(BYTE page);

//---MUX---------------------------------
void AMuxSetInput(BYTE InputMode);

//---FrontPanel---------------------------------

BYTE DCDC_On(BYTE step);
BYTE FrontPanel_StartUP(void);

void FP_BiasOnOff(BYTE fOn);
void FP_PWC_OnOff(BYTE fOn);
void InitGpioDefault(void);

void EnableExtLvdsTxChip(BYTE fOn);

//void InitCVBS(void);
//void InitWithNTSC(void);
//void EnOutput2DCDC2LEDC(BYTE on);
//void InitTconScaler(void);
//void InitWithNTSC(void);
//void Init8836AsDefault_for_watchdog(void);
void Init8836AsDefault(BYTE _InputMain, BYTE _fPowerOn);

#define BT656_8BIT_525I_YCBCR_TO_CVBS		0
#define BT656_8BIT_625I_YCBCR_TO_CVBS		1
#define BT656_8BIT_525P_YCBCR_TO_YPBPR		2
#define BT656_8BIT_625P_YCBCR_TO_YPBPR		3
#if 0
#define BT656_8BIT_720P_YCBCR_TO_YPBPR		4
#define BT656_8BIT_1080I_YCBCR_TO_YPBPR		5
#endif
#define BT656_UNKNOWN_TO_UNKNOWN			0xFF

void BT656_InitExtEncoder(BYTE mode);

void InitComponentReg(BYTE mode);

//BYTE CheckVDLossAndSetFreeRun(void);
void TEMP_init_BT656(BYTE test);


void	LedBackLight( BYTE on );
void	BlackScreen( BYTE on );

void PowerUpLedBackLight(void);
void DCDCOn(void);
void BT656EncOutputEnable(BYTE fOn, BYTE clear_port);
void OutputEnablePin(BYTE fFPDataPin, BYTE fOutputPin);

//#ifdef PANEL_HJ070NA
BYTE AdjustSSPLL_with_HTotal(void);
//#endif


BYTE CheckSpiClock(BYTE max);
BYTE CheckPclko_polarity(void);

#endif
