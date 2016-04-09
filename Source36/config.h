/*
 *  config.h - System Configurations 
 *
 *  Copyright (C) 2011~2014 Intersil Corporation
 *
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
/*****************************************************************************/
/*																			 */
/*                           System Configurations                           */
/*																			 */
/*****************************************************************************/		   
#ifndef __CONFIG_H__
#define __CONFIG_H__

//-----------------------------------------------------------------------------
// MCU
//-----------------------------------------------------------------------------
// DP80390XP - TW8836A, TW8836B.

//-----------------------------------------------------------------------------
// MODEL
//-----------------------------------------------------------------------------
#define MODEL_TW8836B2
//#define MODEL_TW8836DEMO
/*
	DEMO set use
	#define PANEL_1024X600
	#define SUPPORT_HDMI_24BIT
	#undef SUPPORT_BT656_LOOP		

*/

//-----------------------------------------------------------------------------
// Firmware Version
//-----------------------------------------------------------------------------
#define	FWVER			0x045		//REV 0.45 160218 I2CSPI

//-----------------------------------------------------------------------------
// PANEL
// select only one
//-----------------------------------------------------------------------------
#define PANEL_800X480
//#define PANEL_1024X600
//#define PANEL_1280X800

#if defined(MODEL_TW8836DEMO)
	#define PANEL_1024X600
#endif



//-----------------------------------------------------------------------------
// Hardware
//-----------------------------------------------------------------------------
#define SUPPORT_I2C_MASTER		//some customer ask to disable.
#define USE_SFLASH_EEPROM		//E3PROM(FlashEepromEmulator)
#undef  NO_EEPROM				
#define SUPPORT_SPIOSD

#define  SUPPORT_UDFONT			//RAM Font. Only for TEST
#undef  SUPPORT_UART1			//cannot share with BT656 output(ExtCVBS)

#define SUPPORT_TOUCH
#undef  SUPPORT_DIPSW
//#define SUPPORT_KEYPAD

//#define SUPPORT_WATCHDOG
#define SUPPORT_SPIFLASH_4BYTES_ADDRESS


//-----------------------------------------------------------------------------
// Software	MODELs
//
//-----------------------------------------------------------------------------
#define SUPPORT_I2CCMD_SERVER
//#define SUPPORT_RCD					//RearCameraDisplay. use EE[0x0F]
#undef SUPPORT_FAST_INPUT_TOGGLE	//only for CVBS & DTV
#undef SUPPORT_FOSD_MENU			//FontOSD MENU


//-----------------------------------------------------------------------------
//		IR Remote Controller Type
//-----------------------------------------------------------------------------
#define REMO_RC5					// RC5 style
#define TECHWELL_REMOCON
//#define REMO_NEC					// NEC style
//#define PHILIPS_REMOCON 			// New remocon 


//-----------------------------------------------------------------------------
//		Options for Possible Inputs
//-----------------------------------------------------------------------------

#define SUPPORT_DEC
#ifdef SUPPORT_DEC
	#define SUPPORT_CVBS
	#define SUPPORT_SVIDEO
	#undef SUPPORT_DCVBS
#endif

#define SUPPORT_ARGB
#ifdef SUPPORT_ARGB
	#define SUPPORT_COMPONENT			// support component analog
	#define SUPPORT_PC  				// support PC
#endif
	#define ANALOG_OVERSCAN

#define SUPPORT_DTV
#ifdef SUPPORT_DTV
	#define SUPPORT_HDMI
	#ifdef SUPPORT_I2C_MASTER
		#undef SUPPORT_HDMI_EP907M
	#endif
	#undef SUPPORT_HDMI_24BIT		/* conflict with BT656_LOOP */
	#undef SUPPORT_DEONLY_DTV

	#define SUPPORT_DTV656		//BT656 Decoder at DTV i656 input. Not related with BT656 module.
#endif
#if defined(MODEL_TW8836DEMO)
	#define SUPPORT_HDMI_24BIT		/* conflict with BT656_LOOP */
#endif


#define SUPPORT_LVDSRX
#define SUPPORT_BT656		//BT656 Encoder

#if defined(SUPPORT_CVBS) && defined(SUPPORT_BT656) && defined(SUPPORT_DTV656)
	//CVBS=>BT656Enc=>BT656Dec=>Scaler=>Panel.
    //aRGB(low resolution)=>BT656Enc=>BT656Dec=>Scaler=>Panel.
	//LVDS-RX(low resolution)=>BT656Enc=>BT656Dec=>Scaler=>Panel.
	#define SUPPORT_BT656_LOOP		
#endif

#if defined(MODEL_TW8836DEMO)
	#undef SUPPORT_BT656_LOOP		
#endif



#define SUPPORT_SCALER_OVERWRITE_TABLE


//-----------------------------------------------------------------------------
//		Options for Possible Standards
//		Default:NTSC
//-----------------------------------------------------------------------------
#ifdef SUPPORT_FOSD_MENU
	#define SUPPORT_PAL			
	#define SUPPORT_SECAM
	#define SUPPORT_NTSC4		
	#define SUPPORT_PALM	
	#define SUPPORT_PALN		
	#define SUPPORT_PAL60		
#endif

//-----------------------------------------------------------------------------
//		Options for Debugging/Release
//-----------------------------------------------------------------------------
#define DEBUG		// include debug information
#ifdef DEBUG
	#undef DEBUG_MAIN
	#undef DEBUG_TIME
	#undef DEBUG_ISR
	#undef DEBUG_TW88
	#undef DEBUG_UART
	#undef DEBUG_I2C
	#undef DEBUG_SPI
	#undef DEBUG_EEP
	#undef DEBUG_SFLASH_EEPROM
	#define DEBUG_SPIFLASH_TEST
	#undef DEBUG_OSD
		#undef DEBUG_FOSD
		#undef DEBUG_SOSD
	#undef DEBUG_AUDIO
	#undef DEBUG_SETPANEL
	#undef DEBUG_DECODER
	#define DEBUG_PC
	#undef DEBUG_DTV
	#undef DEBUG_BT656
	#undef DEBUG_BANK
	#undef DEBUG_PAUSE
	#undef DEBUG_MENU
	#undef DEBUG_KEYREMO
	#undef DEBUG_TOUCH_HW
	#undef DEBUG_TOUCH_SW
	#undef DEBUG_REMO
	#undef DEBUG_REMO_NEC
	#ifdef SUPPORT_WATCHDOG
		#undef DEBUG_WATCHDOG
	#endif
	#undef DEBUG_SCALER
	#undef DEBUG_SCALER_OVERWRITE_TABLE
#endif


//-----------------------------------------------------------------------------
//		Panel Vendor Specific
// select only ONE !!
//-----------------------------------------------------------------------------
#ifdef PANEL_800X480
	//default. for PANEL_INNOLUX_AT080TN03 & PANEL_INNILUX_AT070TN84
	#define PANEL_TCON
	#undef  PANEL_SRGB
	#undef	PANEL_FP_LSB
	#define PANEL_FORMAT_666

	#define PANEL_H			800
	#define PANEL_V			480
	#define PANEL_H_MIN		848		//unknown
	#define PANEL_H_TYP		1056	//1088
	#define PANEL_H_MAX		2000	//unknown
	#define PANEL_PCLK_MIN	27		//unknown
	#define PANEL_PCLK_TYP	40
	#define PANEL_PCLK_MAX	45

	#define PANEL_SSPLL		108
	#define PANEL_PCLK_DIV	1
	#define PANEL_PCLKO_DIV	3		
#endif

/*
current TW8836 SpiOSD menu needs 94MHz for 1024x600 panel. 131115			
*/
#ifdef PANEL_1024X600
	//#define PANEL_TM070DDH01
	//#define PANEL_HJ070NA
	#define PANEL_LVDS
	#define PANEL_FORMAT_888
	#define PANEL_H			1024
	#define PANEL_V			600
	#define PANEL_H_MIN		1114
	#define PANEL_H_TYP		1344
	#define PANEL_H_MAX		1400
	#define PANEL_PCLK_MIN	41		//spec. 40.8 @60Hz	
	#define PANEL_PCLK_TYP	51   	//      51.2 
	#define PANEL_PCLK_MAX	67		//      67.2

	#define PANEL_SSPLL		90
	#define PANEL_PCLK_DIV	1
	#define PANEL_PCLKO_DIV	2		
#endif

#ifdef PANEL_1280X800
	//only for test
	#define PANEL_AUO_B133EW01
	#define PANEL_LVDS
	#define PANEL_FORMAT_666
	#define PANEL_H			1280
	#define PANEL_V			800
	#define PANEL_H_MIN		1302
	#define PANEL_H_TYP		1408
	#define PANEL_H_MAX		1700
	#define PANEL_PCLK_MIN	50
	#define PANEL_PCLK_TYP	75		//68.9   
	#define PANEL_PCLK_MAX	80

	#define PANEL_SSPLL		130
	#define PANEL_PCLK_DIV	1
	#define PANEL_PCLKO_DIV	2	//?? 1.5	
#endif

//-----------------------------------------------------------------------------
//	SPIOSD_IMAGE SIZE
//-----------------------------------------------------------------------------
#if (PANEL_H==1280 && PANEL_V==800)
	#define SPIOSD_USE_1024X600_IMG
#elif (PANEL_H==1024 && PANEL_V==600)
	#define SPIOSD_USE_1024X600_IMG
#else
	#define SPIOSD_USE_800X480_IMG
#endif


//-----------------------------------------------------------------------------
// default FREERUN value.
// It is depend on PANEL size & PCLK.
//-----------------------------------------------------------------------------
#if (PANEL_H==1280 && PANEL_V==800)
//	#define FREERUN_DEFAULT_HTOTAL	1400
//	#define FREERUN_DEFAULT_VTOTAL	875
	//SSPLL:120MHz, PCLKO:80MHz
	#define FREERUN_DEFAULT_HTOTAL	1768
	#define FREERUN_DEFAULT_VTOTAL	875
#elif (PANEL_H==1024 && PANEL_V==600)
//@90MHz div2
//	#define FREERUN_DEFAULT_HTOTAL	1144
//	#define FREERUN_DEFAULT_VTOTAL	656
//@90MHz div1.5
//	#define FREERUN_DEFAULT_HTOTAL	1450
//	#define FREERUN_DEFAULT_VTOTAL	692
//108M div2
//	#define FREERUN_DEFAULT_HTOTAL	1304
//	#define FREERUN_DEFAULT_VTOTAL	689
	#define FREERUN_DEFAULT_HTOTAL	1376
	#define FREERUN_DEFAULT_VTOTAL	656
#else
	#define FREERUN_DEFAULT_HTOTAL	1085
	#define FREERUN_DEFAULT_VTOTAL	553
#endif



//-----------------------------------------------------------------------------
//		Special Features
//-----------------------------------------------------------------------------
//#define SUPPORT_GAMMA
//#define SUPPORT_DELTA_RGB


//-----------------------------------------------------------------------------
//removed definitions
//-----------------------------------------------------------------------------
// CTEST_TIMER2_UART
// TW8836_CACHE_WATCHDOG
// MODEL_TW8836FPGA
// TW8836_CHIPDEBUG
// SUPPORT_LANAI_1				//MIPI_CSI daughter board.(need HDMI_24BIT)
// MODEL_TW8835_EXTI2C
// SUPPORT_I2CCMD_TEST
// SUPPORT_SELECTKEY
// NO_INITIALIZE
// CHIP_MANUAL_TEST
// MODEL_TW8835_MASTER
// NOSIGNAL_LOGO
// USE_FRONT_IMAGECTRL
// ON_CHIP_EDID_ENABLE
// ON_CHIP_HDCP_ENABLE

#endif // __CONFIG_H__

