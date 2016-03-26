/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/
#ifndef __EEP_H__
#define __EEP_H__

//========== EEPROM Emulation ==========
//
// size = SPI_SECTOR_SIZE * EE_SPI_BANKS * EE_BLOCKS
//	    = 4K              * 4             * 8
//		= 128K = 0x20000.
#ifdef USE_SFLASH_EEPROM
	#include "e3prom.h"
	
	#define E3P_SPI_SECTOR0		0x040000L	// EE Start Address
	#define E3P_SPI_BANKS		4
	#define E3P_INDEX_PER_BLOCK	64			// 32 or 64
	#define E3P_BLOCKS			8
	#define E3P_MAX_INDEX		(E3P_INDEX_PER_BLOCK*E3P_BLOCKS)
	
	
	#define EE_Read(index)			E3P_Read(index)
	#define EE_Write(index, value) 	E3P_Write(index,value)

	extern DWORD e3p_spi_start_addr;	  //BK150126
	void E3P_Configure(void);
#elif defined(NO_EEPROM)  //..USE_SFLASH_EEPROM
	BYTE EE_Read                (WORD index);
	void EE_Write               (WORD index, BYTE dat);
#else
	#define EE_Read(index)		ReadI2CByte(ADDRESS_EEPROM + ((index >> 8) << 1),(BYTE)index)
	#define EE_Write(index,dat)	WriteI2CByte(ADDRESS_EEPROM + ((index >> 8) << 1),(BYTE)index,dat)
#endif //..USE_SFLASH_EEPROM


////eeprom.h

//=========================================================================
//	EEPROM structure
//=========================================================================
//  TW8836 TAG                  0x00	// 'T'
//                              0x01	// '8'
//                              0x02	// '3'
//                              0x03	// '6'
#define	EEP_FWREV_MAJOR			0x04	//F/W Rev.-major(Hex)	
#define	EEP_FWREV_MINOR			0x05	//F/W Rev.-minor(Hex)
#define	EEP_DEBUGLEVEL			0x06	//DebugLevel (0~3)
//			---------------------------------------------------------------


#define EEP_AUTODETECT			0x07	//reserved	Flag for Input Auto Detect	-0:Auto, 1:NTSC,....
#define EEP_AUTODETECTTYTE		0x08	//reserved	Type of Auto-detect(will be value of register 0x1d)
//			---------------------------------------------------------------
#define EEP_WIDEMODE			0x09	//reserved	Wide Mode

#define EEP_0A_RESERVED			0x0A	//blank....

#define EEP_VIDEOMODE			0x0b	//reserved Video Mode
#define EEP_OSDLANG				0x0c	//reserved OSDLang		

#define EEP_OSDPOSITIONMODE 	0x0d	//reserved OSD Position Mode
#define EEP_CCD					0x0e	//reserved Closed Caption-	0: off, 1:on

#define EEP_BOOTMODE			0x0F	//0:Normal 1,RCD, 2:...

//----------------------------------
// Input Option
// 0x10~0x1F
//----------------------------------
#define EEP_INPUT_MAIN			0x10	//BYTE	1	InputSelection
#define EEP_INPUT_DEC			0x11	//BYTE  1	0:Overscan,1:Normal
#define EEP_INPUT_COMP			0x12	//BYTE  1	0:Overscan,1:Normal

//----------------------------------
// BT656Encoder
// 0x20~0x2F
//----------------------------------
//CEA861
//	[7:6]	1080p scale option
//	[5:4]	1080i scale option
//	[3:2]	720p  scale option
//	[1]		unused
//	[0]		2DDI option
//VESA
//	[]
#define EEP_INPUT_BT656			0x20	//BT656Enc Input Source (0~8)	
#define EEP_BT656ENC_DEC		0x21	//0:Normal,1:2DDI
#define EEP_BT656ENC_ARGB_COMP	0x22	//0:Normal 1:option1, 2:option2
#define EEP_BT656ENC_ARGB_PC	0x23	//0:Normal 1:option1, 2:option2
#define EEP_BT656ENC_DTV_TV		0x24	//0:Normal 1:option1, 2:option2
#define EEP_BT656ENC_DTV_PC		0x25	//0:Normal 1:option1, 2:option2
#define EEP_BT656ENC_LVDS_TV	0x26	//0:Normal 1:option1, 2:option2
#define EEP_BT656ENC_LVDS_PC	0x27	//0:Normal 1:option1, 2:option2
#define EEP_BT656ENC_PANEL		0x28	


//----------------------------------
// DEBUG Area
// 0x30~0x3F
//----------------------------------
#define EEP_DEBUG_BOOT			0x30	//Boot Count
#define EEP_DEBUG_WATCHDOG		0x32	//Watchdog Boot Count


//----------------------------------
// Video ImageEnhancement
// 0x40~0x7F
//----------------------------------
/*
	--CVBS(YUV)				--SVIDEO(YUV)				--COMPONENT(YUV)	
0x40	CONTRAST		0x45	CONTRAST			0x4A	CONTRAST
0x41	BRIGHT			0x46	BRIGHT				0x4B	BRIGHT
0x42	SATURATE		0x47	SATURATE			0x4C	SATURATE
0x43	HUE				0x48	HUE					0x4D	HUE
0x44	SHARPNESS		0x49	SHARPNESS			0x4E	SHARPNESS

	--PC(RGB)				--DVI(RGB)					--HDMI PC (RGB)
0x4F	contrastY		0x54	contrastY			0x59	contrastY
0x50	brightY			0x55	brightY				0x5A	brightY
0x51	contrast RGB	0x56	contrast RGB		0x5B	contrast RGB
0x52					0x57						0x5C
0x53					0x58						0x5D

	--HDMI TV (YUV)			--BT656Loop (YUV)			--LVDS Rx
0x5E	CONTRAST		0x63	CONTRAST			0x68	CONTRAST
0x5F	BRIGHT			0x64	BRIGHT				0x69	BRIGHT
0x60	SATURATE		0x65	SATURATE			0x6A	SATURATE
0x61	HUE				0x66	HUE					0x6B	HUE
0x62	SHARPNESS		0x67	SHARPNESS			0x6C	SHARPNESS

*/
//----------------------------------
#define EEP_IA_START			0x40


//backend image adjustment
#define IA_TOT_VIDEO			5
//--for analog (YUV)
#define EEP_IA_CONTRASE_Y		0
#define EEP_IA_BRIGHTNESS_Y		1
#define EEP_IA_SATURATION		2	//for contrast_Cb,contrast_Cr
#define EEP_IA_HUE				3
#define EEP_IA_SHARPNESS		4
//--for digital (RGB)
#define EEP_IA_CONTRAST_R		2
#define EEP_IA_CONTRAST_G		3
#define EEP_IA_CONTRAST_B		4

#define EEP_CVBS				EEP_IA_START					//0x40~0x44						
#define EEP_SVIDEO				(EEP_IA_START+IA_TOT_VIDEO)		//0x45~0x49
#define EEP_YPBPR				(EEP_IA_START+IA_TOT_VIDEO*2)	//0x4A~0x4E
#define EEP_PC					(EEP_IA_START+IA_TOT_VIDEO*3)	//0x4F~0x53
#define EEP_DVI					(EEP_IA_START+IA_TOT_VIDEO*4)	//0x54~0x58
#define EEP_HDMI_PC				(EEP_IA_START+IA_TOT_VIDEO*5)	//0x59~0x5D
#define EEP_HDMI_TV				(EEP_IA_START+IA_TOT_VIDEO*6)	//0x5E~0x62
#define EEP_BT656				(EEP_IA_START+IA_TOT_VIDEO*7)	//0x63~0x58
#define EEP_LVDS				(EEP_IA_START+IA_TOT_VIDEO*8)	//0x68~0x6C
#define EEP_INPUT_IMAGE_END		(EEP_IA_START+IA_TOT_VIDEO*8)	//0x6D 


//----------------------------------
// Touch
// 0x80~0x93
//----------------------------------
#define EEP_TOUCH_CALIB_X		0x80
#define EEP_TOUCH_CALIB_Y		0x80+10
#define EEP_TOUCH_CALIB_END		0x80+20	  //..0x94

//----------------------------------
// Video Effect
// 0xA0~0xAF
//----------------------------------

#define EEP_VE_START			0xA0	//Video Effect
#define EEP_ASPECT_MODE			EEP_VE_START+0x00	//0:Normal,1:Zoom,2:full,3:Panorama
#define EEP_OSD_TRANSPARENCY	EEP_VE_START+0x01
#define EEP_OSD_TIMEOUT			EEP_VE_START+0x02
#define EEP_FLIP				EEP_VE_START+0x03	//0:default,1:flip
#define EEP_BACKLIGHT			EEP_VE_START+0x04
#define EEP_HDMI_MODE			EEP_VE_START+0x05
#define EEP_DVI_MODE			EEP_VE_START+0x06

//----------------------------------
// Old Items
// 0xB0~0xBF
//----------------------------------
/*
--PC
	position(X,Y)
--DVI-24bit
	position (X,Y)
	pclk
	phase
--DVI-12bit
	position (X,Y)
	pclk
	phase
--HDMI PC
	position (X,Y)
	pclk
	phase
*/



#define EEP_OLD_ITEMS_START		0xB0
//	    	---------------------------------------------------------------
#define	EEP_PIPMODE				EEP_OLD_ITEMS_START+0		//0xB0
#define	EEP_PIP1SELECTION		EEP_OLD_ITEMS_START+1		//0xB1
#define	EEP_PIP2SELECTION		EEP_OLD_ITEMS_START+2		//0xB2
//	    	---------------------------------------------------------------
#define EEP_AUDIOVOL			EEP_OLD_ITEMS_START+3		//0xB3	//BYTE	1   AudioVol
#define EEP_AUDIOBALANCE		EEP_OLD_ITEMS_START+4		//0xB4	//BYTE	1   AudioBalance
#define EEP_AUDIOBASS			EEP_OLD_ITEMS_START+5		//0xB5    //BYTE  1   AudioBass
#define EEP_AUDIOTREBLE			EEP_OLD_ITEMS_START+6		//0xB6	//BYTE  1   AudioTreble
#define EEP_AUDIOEFFECT			EEP_OLD_ITEMS_START+7		//0xB7	//BYTE  1
//
//	    	---------------------------------------------------------------

#define EEP_BLOCKMOVIE 			EEP_OLD_ITEMS_START+8		//0xB8	BYTE	1	BlockedMovie:Blocked rating for Movie	
#define EEP_BLOCKTV				EEP_OLD_ITEMS_START+9		//0xB9 	BYTE	1	BlockedTV:Blocked rating for TV			
#define EEP_FVSLD				EEP_OLD_ITEMS_START+10		//0xBA	BYTE  	6
//						                7    6       4    3    2    1    0
//  FVSLD Level                        ALL   FV(V)   S    L    D    
//  0xBA    BYTE    1   TV-Y            X  
//  0xBB	BYTE    1   TV-Y7           X    X 
//  0xBC	BYTE    1   TV-G            X 
//  0xBD	BYTE    1   TV-PG           X       X    X    X    X
//  0xBE	BYTE    1   TV-14           X       X    X    X    X 
//  0xBF	BYTE    1   TV-MA           X       X    X    X
//
#define	EEP_VCHIPPASSWORD		EEP_OLD_ITEMS_START+0x10 	//0xC0	 BYTE	4   OSDPassword		//Defualt:3366
//			---------------------------------------------------------------
//
//
//	0xD0	WORD	2   PanelXRes
//	0xD2	WORD	2	PanelYRes
//	0xD4	BYTE	1	PanelHsyncMinPulseWidth
//	0xD5	BYTE	1	PanelVsyncMinPulseWidth
//	0xD6	WORD	2	PanelHminBackPorch
//	0xD8	BYTE	1	PanelHsyncPolarity
//	0xD9	BYTE	1	PanelVsyncPolarity
//	0xDA	WORD	2	PanelDotClock
//	0xDC	BYTE	1	PanelPixsPerClock
//	0xDD	BYTE	1	PanelDEonly
//			---------------------------------------------------------------
//


//----------------------------------
// PC MODE
// 0xD0~0x1FF
//----------------------------------

#define EEP_PC_MODE_START		0x0D0
#define EEP_PC_MODE_MAX			64



WORD GetFWRevEE(void);
void SaveFWRevEE(WORD);

BYTE GetDebugLevelEE(void);
void SaveDebugLevelEE(BYTE);

//BYTE GetWideModeEE(void);
//void SaveWideModeEE(BYTE dl);

//BYTE GetPossibleAutoDetectStdEE(void);

BYTE GetOSDLangEE(void);
void SaveOSDLangEE(BYTE val);

BYTE GetOSDPositionModeEE(void);
void SaveOSDPositionModeEE(BYTE ndata);

BYTE GetInputMainEE( void );
void SaveInputMainEE( BYTE mode );
BYTE GetInputBT656EE( void );
void SaveInputBT656EE( BYTE mode );



BYTE GetVideoDatafromEE(BYTE offset);
void SaveVideoDatatoEE(BYTE offset, BYTE ndata);
void ResetVideoValue(void);

BYTE GetAspectModeEE(void);
void SaveAspectModeEE(BYTE mode);

BYTE GetHdmiModeEE(void);
void SaveHdmiModeEE(BYTE mode);
BYTE GetDviModeEE(void);
void SaveDviModeEE(BYTE mode);

void ResetAudioValue(void);

//----------------------------
//
//----------------------------
void CheckSystemVersion(void);
BYTE CheckEEPROM(void);

void InitializeEE( void );
void ClearBasicEE(void);

//void SavePIP1EE( BYTE mode );
//void SavePIP2EE( BYTE mode );


//=============================
// PC EEPROM
//=============================
#define EE_ADC_GO		EEP_ADC_GAIN_START		// ADC Gain Offset for PC
#define EE_ADC_GO_DTV	EEP_ADC_GAIN_START+6	// ADC Gain Offset for DTV

#define	EE_PCDATA		EEP_PC_MODE_START	// StartAddress of EEPROM for PCDATA


#if defined( SUPPORT_PC ) || defined (SUPPORT_DVI)
#define EE_YUVDATA_START	50
#else
#define EE_YUVDATA_START	0
#endif
#define EE_EOF_PCDATA		EE_YUVDATA_START+7		

#define EE_PCDATA_CLOCK			0
#define EE_PCDATA_PHASE			1
#define EE_PCDATA_VACTIVE		2
#define EE_PCDATA_VBACKPORCH	3
#define EE_PCDATA_HACTIVE		4
#define	LEN_PCDATA				5					// Length of PCDATA

//void SavePixelClkEE(BYTE mode);
//void SavePhaseEE(BYTE mode);
//void SaveVBackPorchEE(BYTE mode);

//WORD GetVActiveStartEE(BYTE mode);
//WORD GetHActiveStartEE(BYTE mode);
char GetHActiveEE(BYTE mode);
char GetVActiveEE(BYTE mode);
void SaveHActiveEE(BYTE mode, char value);
void SaveVActiveEE(BYTE mode, char value);
char GetVBackPorchEE(BYTE mode);
void SaveVBackPorchEE(BYTE mode, char value);


//BYTE GetPixelClkEE(BYTE fRGB);
//void MY_SavePixelClkEE(BYTE fRGB, BYTE value);
//BYTE GetPhaseEE(BYTE fRGB);
//void MY_SavePhaseEE(BYTE fRGB, BYTE value);
char GetPixelClkEE(BYTE mode);
void SavePixelClkEE(BYTE mode, char val);
BYTE GetPhaseEE(BYTE mode);
void SavePhaseEE(BYTE mode, BYTE val);

void InitPCDataEE(void);


void SaveADCGainOffsetEE(BYTE mod);
void GetADCGainOffsetEE(void);

void EE_Increase_WORD(WORD index);
void EE_Increase_Counter_Boot(void);
void EE_Increase_Counter_Watchdog(void);




#endif	// __ETC_EEP__
