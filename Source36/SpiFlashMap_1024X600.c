/**
 * @file
 * SpiFlashMap.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	SpiFlash Map for images
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
//*****************************************************************************
//
//								SPI_MAP.c
//
//*****************************************************************************
// SPI FLASH total MAP
//000_0000+----------------------------+    
//        |                            |
//        |     CODE BANK              |
//        |                            |
//004_0000+----------------------------+    
//        |     EEPROM Emulation       |
//006_0000+----------------------------+    
//        |     Blank Space            |
//008_0000+----------------------------+    
//        |                            |
//        |     Font Area              |
//        |                            |0x0100000
//010_0000+----------------------------+1MByte(8Mbit)    
//        |                            |
//        |     MENU IMG               |0x0400000
//        |                            |4MByte(32Mbit)
//        |                            |
//05E_0000+----------------------------+
//        |     Blank Space            |    
//060_0000+----------------------------+
//        | Demo Image                 |    
//        +-(000000)-------------------+    
//        | ParkGrid                   |
//        | LUT size 4*(4*265)         |
//        | img offset 0x001000        |
//        | total   size 0x06EE30      |
//        +-(700000)-------------------+    
//        | Pigeon                     |
//        |  img offset 0x070000       |
//        | Rose                       |
//        |  img offset 0x090D00       | 
//        | LUT for Pigion&Rose        | 0x0800000
//        |  offset 0x217700           | 8MByte(64Mbit)
//        |                            |
//082_0000+----------------------------+
//        | Slide Image                |    
//        |                            |
//        |  size 7B0000               |
//        |                            |
//0FD_0000+----------------------------+
//        | Blank Space                |
//        |                            |
//100_0000+----------------------------+16MByte(128Mbit)
//        | Blank Space                |
//        | (4Byte Address)            |
//200_0000+----------------------------+32MByte(256Mbit)
// 
//
// detail DEMO IMG MAP
// +----------------------------+    
// | PIGEON                     |
// +----------------------------+    
// | ROSE                       |
// +----------------------------+    
// | LUT for PIGEON&ROSE        |
// +----------------------------+    
// | GENESIS & LUT              |
// +----------------------------+    
// | GRID & LUT                 |
// +----------------------------+    
// | MESSAGE & LUT              |
// +----------------------------+    
// | COMPASS & LUT              |
// +----------------------------+    
// | Dynamic Grid & LUT         |
// +----------------------------+    
// | Dynamic Message & LUT      |
// +----------------------------+    

#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "Global.h"
#include "CPU.h"
#include "printf.h"
#include "util.h"
#include "monitor.h"

#include "I2C.h"
#include "InputCtrl.h"
#include "Spi.h"

#include "SOsd.h"
#include "FOsd.h"

#include "SpiFlashMap.h"
#include "SOsdMenu.h"

#ifdef SPIOSD_USE_1024X600_IMG


#ifdef SUPPORT_SPIOSD

#define MAP0_START	SFLASH_IMG_ADDR


//===============
// DEFAULT FONT
// 0x400000
//===============


code WORD default_LUT_bpp2[4] 		= { 0x0000,0x001F,0xF800,0xFFFF };
code WORD default_LUT_bpp3[8] 		= { 0x0000,0x001F,0x07E0,0x07FF,0xF800,0xF81F,0xFFE0,0xFFFF };
code WORD default_LUT_bpp4[16] 		= { 0x0000,0x0010,0x0400,0x0410,0x8000,0x8010,0x8400,0x8410,
										0xC618,0x001F,0x07E0,0x07FF,0xF800,0xF81F,0xFFE0,0xFFFF	};

//code WORD consolas_LUT_bpp2[4] 	= {	0x0000,0x001F,0xF800,0xFFFF };
//code WORD consolas_LUT_bpp3[8] 	= { 0x0000,0x001F,0x07E0,0x07FF,0xF800,0xF81F,0xFFE0,0xFFFF };
//code WORD consolas_LUT_bpp4[16] 	= { 0x07FF,0x20E3,0xF79E,0x62E8,0xE104,0xA944,0x39A6,0x7BAC,
// 											0x51A6,0xC617,0x9CD1,0xB5B5,0x9BC9,0xDD85,0xF643,0xAC87 };

//code WORD graynum_LUT_bpp2[4] 	= { 0xF7DE,0x0000,0x5AAB,0xC000  };
code WORD graynum_LUT_bpp3[8] 		= { 0xFFFF,0x0000,0xDEDB,0x9492,0x6B6D,0xB5B6,0x4A49,0x2124 };
//code WORD graynum_LUT_bpp4[16] 	= {	0xD6BA,0x20E3,0xF79E,0x62E8,0xE104,0xA944,0x39A6,0x7BAC,
//										0x51A6,0xC617,0x9CD1,0xB5B5,0x9BC9,0xDD85,0xF643,0xAC87};


//TW8835 max FontRam size is 0x2800. But FW use 0x3000 space. The remain 0x800 will be used for Font Information.
//                                              loc,      size    W   H   2BPP   3BPP   4BPP   MAX    palette for 2bpp,3bpp,4bpp
code FONT_SPI_INFO_t default_font 		   	= { FONT_ADDR+0x0000, 0x27F9, 12, 18, 0x100, 0x120, 0x15F, 0x17B, default_LUT_bpp2, default_LUT_bpp3, default_LUT_bpp4 };
code FONT_SPI_INFO_t consolas16x26_606C90 	= { FONT_ADDR+0x3000, 0x2080, 16, 26, 0x060, 0x06C, 0x090, 0x0A0, NULL, NULL, NULL };
code FONT_SPI_INFO_t consolas16x26_graynum 	= { FONT_ADDR+0x6000, 0x0618, 16, 26, 0x000, 0x000, 0x01E, 0x01E, NULL, graynum_LUT_bpp3, NULL };
code FONT_SPI_INFO_t kor_font		 		= { FONT_ADDR+0x9000, 0x0A20, 12, 18, 0x000, 0x000, 0x000, 0x060, NULL, NULL, NULL };
code FONT_SPI_INFO_t ram_font		 		= { FONT_ADDR+0xB000, 0x2080, 16, 18, 0x060, 0x06C, 0x090, 0x0A0, NULL, NULL, NULL };
//next 0x400000+0xB000
//next you have to move the menu images.


//===================================
// for TEST
//FAR CONST MY_SLIDEIMAGE test_IMG[] = {
//    { MAP0_START+0x0EF71A, 0x0100, 0x002B1D },    // Test_PBARPTR100_64
//};
//FAR CONST MY_RLE_INFO test_INFO[] = {
//	{ 0x60, 327,45  },		//Test_PBARPTR100_64
//};


//====================================================
// MENU IMAGE MAP
// 0x410000
//====================================================

//for fast
//code image_info_t img_navi_close0_header 	=  {1, 0x70, 0x30, 0x30 };
//code image_info_t img_navi_close1_header 	=  {1, 0x70, 0x30, 0x30 };
//code image_info_t img_navi_setup0_header 	=  {1, 0x70, 0x30, 0x30 };
//code image_info_t img_navi_setup1_header 	=  {1, 0x70, 0x30, 0x30 };



//code image_info_t img_main_input_header =  {1, 0x80, 118, 110 /*,0x01803E*/};
//code image_info_t img_main_audio_header =  {1, 0x80, 106, 114 /*,0x01803E*/};
//code image_info_t img_main_system_header = {1, 0x80, 138, 117 /*,0x01803E*/};
//code image_info_t img_wait_header 		 = {1, 0x70,  48, 50 /*,0x01803E*/};

//code image_info_t img_input_bg_bottom_header 	= {1, 0x88, 0x320, 0x042 }; //:45F13D:49 54 88 88 20 03 42 00 95 18 00 00 01 FF 60 01 
//code image_info_t img_input_nodvi_bg_top_header = {1, 0x80, 0x320, 0x046 }; //:46EFD2:49 54 88 00 20 03 46 00 C0 DA 00 00 01 FF 60 01 
//code image_info_t img_input_select_header		= {1, 0x80, 0x009, 0x009 }; //:47CEA2:49 54 88 00 09 00 09 00 51 00 00 00 01 FF 60 01 
//code image_info_t img_input_cvbs0_header 	= {1, 0x80, 0x3E, 0x40 };
//code image_info_t img_input_cvbs1_header 	= {1, 0x80, 0x3E, 0x40 };
//code image_info_t img_input_svideo0_header 	= {1, 0x80, 0x4F, 0x3E };
//code image_info_t img_input_svideo1_header 	= {1, 0x80, 0x4F, 0x3E };
//code image_info_t img_input_ypbpr0_header 	= {1, 0x80, 0x46, 0x42 };
//code image_info_t img_input_ypbpr1_header 	= {1, 0x80, 0x46, 0x42 };
//code image_info_t img_input_pc0_header 		= {1, 0x80, 0x47, 0x3F };
//code image_info_t img_input_pc1_header 		= {1, 0x80, 0x47, 0x3F };
////code image_info_t img_input_dvi0_header 	= {1, 0x80, 0x4F, 0x3E };
////code image_info_t img_input_dvi1_header 	= {1, 0x80, 0x4F, 0x3E };
//code image_info_t img_input_hdmi0_header 	= {1, 0x80, 0x48, 0x3F };
//code image_info_t img_input_hdmi1_header 	= {1, 0x80, 0x48, 0x3F };
//code image_info_t img_input_ext0_header 	= {1, 0x80, 0x3D, 0x3D };
//code image_info_t img_input_ext1_header 	= {1, 0x80, 0x3D, 0x3D };
//code image_info_t img_input_return0_header 	= {1, 0x70, 0x22, 0x22 };
//code image_info_t img_input_return1_header 	= {1, 0x70, 0x22, 0x22 };







//code image_item_info_t img_ = {+0x000000, 0x0010F0 },    // FontAll 
code image_item_info_t img_logo							= {1, MENU_B_FLASH_START+0x000000, NULL,	0xff};    // Intersil-Techwell
code image_item_info_t img_navi_menu  					= {1, MENU_B_FLASH_START+0x006660, NULL,	0x00};    // img_navi_bg 
code image_item_info_t img_navi_return  				= {1, MENU_B_FLASH_START+0x00A450, NULL,	0xff};    // img_navi_return 
code image_item_info_t img_navi_return1  				= {1, MENU_B_FLASH_START+0x00AF10, NULL,	0xff};    // img_navi_return1 
code image_item_info_t img_navi_home  					= {1, MENU_B_FLASH_START+0x00B9F0, NULL,	0xff};    // img_navi_home 
code image_item_info_t img_navi_home1  					= {1, MENU_B_FLASH_START+0x00C530, NULL,	0xff};    // img_navi_home1 
code image_item_info_t img_navi_close  					= {1, MENU_B_FLASH_START+0x00D040, NULL,	0xff};    // img_navi_close 
code image_item_info_t img_navi_close1  				= {1, MENU_B_FLASH_START+0x00DCB0, NULL,	0xff};    // img_navi_close1 
code image_item_info_t img_navi_demo  					= {1, MENU_B_FLASH_START+0x00E8C0, NULL,	0xff};    // img_navi_demo 
code image_item_info_t img_navi_demo1  					= {1, MENU_B_FLASH_START+0x00F350, NULL,	0xff};    // img_navi_demo1 
code image_item_info_t img_navi_setup  					= {1, MENU_B_FLASH_START+0x00FE10, NULL,	0xff};    // img_navi_setup 
code image_item_info_t img_navi_setup1  				= {1, MENU_B_FLASH_START+0x010B50, NULL,	0xff};    // img_navi_setup1 
code image_item_info_t img_main_bg  					= {1, MENU_B_FLASH_START+0x011930, NULL,	0xff};    // img_main_bg 
code image_item_info_t img_main_input  					= {1, MENU_B_FLASH_START+0x02AF90, NULL,	0xff};    // img_main_input 
code image_item_info_t img_main_input1  				= {1, MENU_B_FLASH_START+0x0313F0, NULL,	0xff};    // img_main_input1 
code image_item_info_t img_main_audio  					= {1, MENU_B_FLASH_START+0x0378E0, NULL,	0xff};    // img_main_audio 
code image_item_info_t img_main_audio1  				= {1, MENU_B_FLASH_START+0x03DA10, NULL,	0xff};    // img_main_audio1 
code image_item_info_t img_main_system  				= {1, MENU_B_FLASH_START+0x043850, NULL,	0xff};    // img_main_system 
code image_item_info_t img_main_system1  				= {1, MENU_B_FLASH_START+0x04B560, NULL,	0xff};    // img_main_system1 
code image_item_info_t img_main_gps  					= {1, MENU_B_FLASH_START+0x053300, NULL,	0xff};    // img_main_gps 
code image_item_info_t img_main_gps1  					= {1, MENU_B_FLASH_START+0x059AE0, NULL,	0xff};    // img_main_gps1 
code image_item_info_t img_main_phone  					= {1, MENU_B_FLASH_START+0x060300, NULL,	0xff};    // img_main_phone 
code image_item_info_t img_main_phone1  				= {1, MENU_B_FLASH_START+0x066500, NULL,	0xff};    // img_main_phone1 
code image_item_info_t img_main_carinfo  				= {1, MENU_B_FLASH_START+0x06C2C0, NULL,	0xff};    // img_main_carinfo 
code image_item_info_t img_main_carinfo1  				= {1, MENU_B_FLASH_START+0x073F40, NULL,	0xff};    // img_main_carinfo1 
code image_item_info_t img_input_bg_bottom  			= {1, MENU_B_FLASH_START+0x07B9C0, NULL,	0xD5};    // img_input_bg_bottom 
code image_item_info_t img_input_bg_top  				= {1, MENU_B_FLASH_START+0x0867B0, NULL,	0xD5};    // img_input_bg_top 
code image_item_info_t img_input_nodvi_bg_top  			= {1, MENU_B_FLASH_START+0x09CBC0, NULL,	0xD5};    // img_input_nodvi_bg_top 
code image_item_info_t img_input_select  				= {1, MENU_B_FLASH_START+0x0B2FD0, NULL,	0xD5};    // img_input_select 
code image_item_info_t img_input_cvbs 		 			= {1, MENU_B_FLASH_START+0x0B34B0, NULL,	0xff};    // img_input_cvbs 
code image_item_info_t img_input_cvbs1 		 			= {1, MENU_B_FLASH_START+0x0B5170, NULL,	0xff};    // img_input_cvbs1 
code image_item_info_t img_input_svideo  				= {1, MENU_B_FLASH_START+0x0B6DA0, NULL,	0xff};    // img_input_svideo 
code image_item_info_t img_input_svideo1  				= {1, MENU_B_FLASH_START+0x0B8FE0, NULL,	0xff};    // img_input_svideo1 
code image_item_info_t img_input_ypbpr  				= {1, MENU_B_FLASH_START+0x0BB280, NULL,	0xff};    // img_input_Ypbpr 
code image_item_info_t img_input_ypbpr1  				= {1, MENU_B_FLASH_START+0x0BD220, NULL,	0xff};    // img_input_Ypbpr1 
code image_item_info_t img_input_pc  					= {1, MENU_B_FLASH_START+0x0BF1C0, NULL,	0xff};    // img_input_pc 
code image_item_info_t img_input_pc1  					= {1, MENU_B_FLASH_START+0x0C10F0, NULL,	0xff};    // img_input_pc1 
code image_item_info_t img_input_dvi  					= {1, MENU_B_FLASH_START+0x0C3120, NULL,	0xff};    // img_input_dvi 
code image_item_info_t img_input_dvi1  					= {1, MENU_B_FLASH_START+0x0C5100, NULL,	0xff};    // img_input_dvi1 
code image_item_info_t img_input_hdmi  					= {1, MENU_B_FLASH_START+0x0C70F0, NULL,	0xff};    // img_input_hdmi 
code image_item_info_t img_input_hdmi1  				= {1, MENU_B_FLASH_START+0x0C9120, NULL,	0xff};    // img_input_hdmi1 
code image_item_info_t img_input_ext  					= {1, MENU_B_FLASH_START+0x0CB1F0, NULL,	0xff};    // img_input_ext 
code image_item_info_t img_input_ext1  					= {1, MENU_B_FLASH_START+0x0CCDD0, NULL,	0xFF};    // img_input_ext1 
code image_item_info_t img_input_return  				= {1, MENU_B_FLASH_START+0x0CE9B0, NULL,	0xff};    // img_input_return 
code image_item_info_t img_input_return1  				= {1, MENU_B_FLASH_START+0x0CF430, NULL,	0xFF};    // img_input_return1 
code image_item_info_t img_audio_bg			  			= {1, MENU_B_FLASH_START+0x0CFF10, NULL,	0xff};    // img_audio_bg 
code image_item_info_t img_system_bg_bottom  			= {1, MENU_B_FLASH_START+0x0E95A0, NULL,	0x07};    // img_system_bg_bottom 
code image_item_info_t img_system_bg_top  				= {1, MENU_B_FLASH_START+0x0F2320, NULL,	0x07};    // img_system_bg_top 
code image_item_info_t img_system_touch  				= {1, MENU_B_FLASH_START+0x108B30, NULL,	0xff};    // img_system_touch 
code image_item_info_t img_system_touch1  				= {1, MENU_B_FLASH_START+0x10A380, NULL,	0xff};    // img_system_touch1 
code image_item_info_t img_system_display 				= {1, MENU_B_FLASH_START+0x10BBD0, NULL,	0xff};    // img_system_display 
code image_item_info_t img_system_display1 				= {1, MENU_B_FLASH_START+0x10D6C0, NULL,	0xff};    // img_system_display1 
code image_item_info_t img_system_btooth  				= {1, MENU_B_FLASH_START+0x10F160, NULL,	0xff};    // img_system_btooth 
code image_item_info_t img_system_btooth1  				= {1, MENU_B_FLASH_START+0x110AE0, NULL,	0xff};    // img_system_btooth1 
code image_item_info_t img_system_restore  				= {1, MENU_B_FLASH_START+0x1124A0, NULL,	0xff};    // img_system_restore 
code image_item_info_t img_system_restore1  			= {1, MENU_B_FLASH_START+0x113CB0, NULL,	0xff};    // img_system_restore1 
code image_item_info_t img_system_sys_info  			= {1, MENU_B_FLASH_START+0x1154C0, NULL,	0xff};    // img_system_sys_info 
code image_item_info_t img_system_sys_info1  			= {1, MENU_B_FLASH_START+0x116E40, NULL,	0xff};    // img_system_sys_info1 
code image_item_info_t img_gps_bg			  			= {1, MENU_B_FLASH_START+0x1187C0, NULL,	0xff};    // img-gps-bg 
code image_item_info_t img_phone_bg			  			= {1, MENU_B_FLASH_START+0x13E9A0, NULL,	0xff};    // img-phone-bg
code image_item_info_t img_phone_00 					= {1, MENU_B_FLASH_START+0x14FD00, NULL,	0xff};    // img_phone_00 	 
code image_item_info_t img_phone_01 					= {1, MENU_B_FLASH_START+0x151300, NULL,	0xff};    // img_phone_01 	 
code image_item_info_t img_phone_10 					= {1, MENU_B_FLASH_START+0x1528F0, NULL,	0xff};    // img_phone_10 	 
code image_item_info_t img_phone_11 					= {1, MENU_B_FLASH_START+0x153E40, NULL,	0xff};    // img_phone_11 	 
code image_item_info_t img_phone_20 					= {1, MENU_B_FLASH_START+0x155480, NULL,	0xff};    // img_phone_20 	 
code image_item_info_t img_phone_21 					= {1, MENU_B_FLASH_START+0x156A80, NULL,	0xff};    // img_phone_21 	 
code image_item_info_t img_phone_30 					= {1, MENU_B_FLASH_START+0x158070, NULL,	0xff};    // img_phone_30 	 
code image_item_info_t img_phone_31 					= {1, MENU_B_FLASH_START+0x1596B0, NULL,	0xff};    // img_phone_31 	 
code image_item_info_t img_phone_40 					= {1, MENU_B_FLASH_START+0x15ACD0, NULL,	0xff};    // img_phone_40 	 
code image_item_info_t img_phone_41 					= {1, MENU_B_FLASH_START+0x15C260, NULL,	0xff};    // img_phone_41 	 
code image_item_info_t img_phone_50 					= {1, MENU_B_FLASH_START+0x15D850, NULL,	0xff};    // img_phone_50 	 
code image_item_info_t img_phone_51 					= {1, MENU_B_FLASH_START+0x15EE00, NULL,	0xff};    // img_phone_51 	 
code image_item_info_t img_phone_60 					= {1, MENU_B_FLASH_START+0x1603F0, NULL,	0xff};    // img_phone_60 	 
code image_item_info_t img_phone_61 					= {1, MENU_B_FLASH_START+0x1619E0, NULL,	0xff};    // img_phone_61 	 
code image_item_info_t img_phone_70 					= {1, MENU_B_FLASH_START+0x162FD0, NULL,	0xff};    // img_phone_70 	 
code image_item_info_t img_phone_71 					= {1, MENU_B_FLASH_START+0x164520, NULL,	0xff};    // img_phone_71 	 
code image_item_info_t img_phone_80 					= {1, MENU_B_FLASH_START+0x165B60, NULL,	0xff};    // img_phone_80 	 
code image_item_info_t img_phone_81 					= {1, MENU_B_FLASH_START+0x1670B0, NULL,	0xff};    // img_phone_81 	 
code image_item_info_t img_phone_90 					= {1, MENU_B_FLASH_START+0x1686A0, NULL,	0xff};    // img_phone_90 	 
code image_item_info_t img_phone_91 					= {1, MENU_B_FLASH_START+0x169BF0, NULL,	0xff};    // img_phone_91 	 
code image_item_info_t img_phone_star0 					= {1, MENU_B_FLASH_START+0x16B210, NULL,	0xff};    // img_phone_star0 	 
code image_item_info_t img_phone_star1 					= {1, MENU_B_FLASH_START+0x16C800, NULL,	0xff};    // img_phone_star1 	 
code image_item_info_t img_phone_sharp0 				= {1, MENU_B_FLASH_START+0x16DDF0, NULL,	0xff};    // img_phone_sharp0  
code image_item_info_t img_phone_sharp1 				= {1, MENU_B_FLASH_START+0x16F380, NULL,	0xff};    // img_phone_sharp1  
code image_item_info_t img_phone_dial0 					= {1, MENU_B_FLASH_START+0x170970, NULL,	0xff};    // img_phone_dial0 	 
code image_item_info_t img_phone_dial1 					= {1, MENU_B_FLASH_START+0x174CC0, NULL,	0xff};    // img_phone_dial1 	 
code image_item_info_t img_phone_up0 					= {1, MENU_B_FLASH_START+0x179010, NULL,	0xff};    // img_phone_up0 	 
code image_item_info_t img_phone_up1 					= {1, MENU_B_FLASH_START+0x17A560, NULL,	0xff};    // img_phone_up1 	 
code image_item_info_t img_phone_down0 					= {1, MENU_B_FLASH_START+0x17BB50, NULL,	0xff};    // img_phone_down0 	 
code image_item_info_t img_phone_down1 					= {1, MENU_B_FLASH_START+0x17D100, NULL,	0xff};    // img_phone_down1 	 
code image_item_info_t img_phone_left0 					= {1, MENU_B_FLASH_START+0x17E740, NULL,	0xff};    // img_phone_left0 	 
code image_item_info_t img_phone_left1 					= {1, MENU_B_FLASH_START+0x17FC90, NULL,	0xff};    // img_phone_left1 	 
code image_item_info_t img_phone_right0 				= {1, MENU_B_FLASH_START+0x1812B0, NULL,	0xff};    // img_phone_right0  
code image_item_info_t img_phone_right1 				= {1, MENU_B_FLASH_START+0x182800, NULL,	0xff};    // img_phone_right1  
code image_item_info_t img_phone_check0 				= {1, MENU_B_FLASH_START+0x183DF0, NULL,	0xff};    // img_phone_check0  
code image_item_info_t img_phone_check1 				= {1, MENU_B_FLASH_START+0x185310, NULL,	0xff};    // img_phone_check1  
code image_item_info_t img_phone_help0 					= {1, MENU_B_FLASH_START+0x186900, NULL,	0xff};    // img_phone_help0 	 
code image_item_info_t img_phone_help1 					= {1, MENU_B_FLASH_START+0x187F80, NULL,	0xff};    // img_phone_help1 	 
code image_item_info_t img_phone_dir0 					= {1, MENU_B_FLASH_START+0x189600, NULL,	0xff};    // img_phone_dir0 	 
code image_item_info_t img_phone_dir1 					= {1, MENU_B_FLASH_START+0x18BF10, NULL,	0xff};    // img_phone_dir1 	 
code image_item_info_t img_phone_set0 					= {1, MENU_B_FLASH_START+0x18E820, NULL,	0xff};    // img_phone_set0 	 
code image_item_info_t img_phone_set1 					= {1, MENU_B_FLASH_START+0x191130, NULL,	0xff};    // img_phone_set1 	 
code image_item_info_t img_phone_msg0 					= {1, MENU_B_FLASH_START+0x193A40, NULL,	0xff};    // img_phone_msg0 	 
code image_item_info_t img_phone_msg1 					= {1, MENU_B_FLASH_START+0x196390, NULL,	0xff};    // img_phone_msg1 	 
code image_item_info_t img_phone_menu0 					= {1, MENU_B_FLASH_START+0x198CE0, NULL,	0xff};    // img_phone_menu0 	 
code image_item_info_t img_phone_menu1 					= {1, MENU_B_FLASH_START+0x19B5F0, NULL,	0xff};    // img_phone_menu1 	 
code image_item_info_t img_carinfo_bg			  		= {1, MENU_B_FLASH_START+0x19DF00, NULL,	0xff};    // img_carinfo_bg 
code image_item_info_t img_demo_bg  					= {1, MENU_B_FLASH_START+0x1B2CF0, NULL,	0xff};    // img_demo_bg 
code image_item_info_t img_demo_grid  					= {1, MENU_B_FLASH_START+0x1C9980, NULL,	0xff};    // img_demo_grid  	
code image_item_info_t img_demo_grid1  					= {1, MENU_B_FLASH_START+0x1CE5B0, NULL,	0xff};    // img_demo_grid1  	
code image_item_info_t img_demo_rose  					= {1, MENU_B_FLASH_START+0x1D3040, NULL,	0xff};    // img_demo_rose  	
code image_item_info_t img_demo_rose1  					= {1, MENU_B_FLASH_START+0x1D8330, NULL,	0xff};    // img_demo_rose1  	
code image_item_info_t img_demo_ani		  				= {1, MENU_B_FLASH_START+0x1DD590, NULL,	0xff};    // img_demo_ani		
code image_item_info_t img_demo_ani1	  				= {1, MENU_B_FLASH_START+0x1E1D40, NULL,	0xff};    // img_demo_ani1	
code image_item_info_t img_demo_palette	  				= {1, MENU_B_FLASH_START+0x1E6600, NULL,	0xff};    // img_demo_palette	
code image_item_info_t img_demo_palette1  				= {1, MENU_B_FLASH_START+0x1EB300, NULL,	0xff};    // img_demo_palette1
code image_item_info_t img_demo_demoA	  				= {1, MENU_B_FLASH_START+0x1F0000, NULL,	0xff};    // img_demo_demoA	
code image_item_info_t img_demo_demoA1	  				= {1, MENU_B_FLASH_START+0x1F47B0, NULL,	0xff};    // img_demo_demoA1	
code image_item_info_t img_demo_demoB	  				= {1, MENU_B_FLASH_START+0x1F8F60, NULL,	0xff};    // img_demo_demoB	
code image_item_info_t img_demo_demoB1	  				= {1, MENU_B_FLASH_START+0x1FD690, NULL,	0xff};    // img_demo_demoB1	
code image_item_info_t img_touch_bg  					= {1, MENU_B_FLASH_START+0x201DC0, NULL,	0xff};    // img_touch_bg 
code image_item_info_t img_touch_bg_end					= {1, MENU_B_FLASH_START+0x205BE0, NULL,	0xff};    // img_touch_bg 
code image_item_info_t img_touch_button  				= {1, MENU_B_FLASH_START+0x20B630, NULL,	0xff};    // img_touch_button 
code image_item_info_t img_touch_button1  				= {1, MENU_B_FLASH_START+0x20B850, NULL,	0xff};    // img_touch_button1 
code image_item_info_t img_btooth_bg  					= {1, MENU_B_FLASH_START+0x20BA70, NULL,	0xff};    // img_btooth_bg 
code image_item_info_t img_yuv_menu_bg  				= {1, MENU_B_FLASH_START+0x24DCD0, NULL,	0x01};    // img_yuv_menu_bg 
code image_item_info_t img_yuv_bright  					= {1, MENU_B_FLASH_START+0x26B350, NULL,	0xff};    // img_yuv_bright 
code image_item_info_t img_yuv_bright1  				= {1, MENU_B_FLASH_START+0x26F7B0, NULL,	0xff};    // img_yuv_bright1 
code image_item_info_t img_yuv_contrast  				= {1, MENU_B_FLASH_START+0x273C10, NULL,	0xff};    // img_yuv_contrast 
code image_item_info_t img_yuv_contrast1  				= {1, MENU_B_FLASH_START+0x278070, NULL,	0xff};    // img_yuv_contrast1 
code image_item_info_t img_yuv_hue  					= {1, MENU_B_FLASH_START+0x27C4D0, NULL,	0xff};    // img_yuv_hue 
code image_item_info_t img_yuv_hue1  					= {1, MENU_B_FLASH_START+0x280930, NULL,	0xff};    // img_yuv_hue1 
code image_item_info_t img_yuv_saturate  				= {1, MENU_B_FLASH_START+0x284D90, NULL,	0xff};    // img_yuv_saturate 
code image_item_info_t img_yuv_saturate1  				= {1, MENU_B_FLASH_START+0x2891F0, NULL,	0xff};    // img_yuv_saturate1 
code image_item_info_t img_yuv_sharp  					= {1, MENU_B_FLASH_START+0x28D650, NULL,	0xff};    // img_yuv_sharp 
code image_item_info_t img_yuv_sharp1  					= {1, MENU_B_FLASH_START+0x291AB0, NULL,	0xff};    // img_yuv_sharp1 
code image_item_info_t img_rgb_menu_bg  				= {1, MENU_B_FLASH_START+0x295F10, NULL,	0x01};    // img_rgb_menu_bg 
code image_item_info_t img_rgb_bright  					= {1, MENU_B_FLASH_START+0x2B3590, NULL,	0xff};    // img_rgb_bright 
code image_item_info_t img_rgb_bright1 					= {1, MENU_B_FLASH_START+0x2BA7A0, NULL,	0xff};    // img_rgb_bright1 
code image_item_info_t img_rgb_contrast  				= {1, MENU_B_FLASH_START+0x2C19B0, NULL,	0xff};    // img_rgb_contrast 
code image_item_info_t img_rgb_contrast1  				= {1, MENU_B_FLASH_START+0x2C8BC0, NULL,	0xff};    // img_rgb_contrast1 
code image_item_info_t img_rgb_color 					= {1, MENU_B_FLASH_START+0x2CFDD0, NULL,	0xff};    // img_rgb_color 
code image_item_info_t img_rgb_color1 					= {1, MENU_B_FLASH_START+0x2D6FE0, NULL,	0xff};    // img_rgb_color1 
code image_item_info_t img_apc_menu_bg  				= {1, MENU_B_FLASH_START+0x2DE1F0, NULL,	0x01};    // img_apc_menu_bg 
code image_item_info_t img_apc_bright  					= {1, MENU_B_FLASH_START+0x2FB870, NULL,	0xff};    // img_apc_bright 
code image_item_info_t img_apc_bright1 					= {1, MENU_B_FLASH_START+0x2FE2E0, NULL,	0xff};    // img_apc_bright1 
code image_item_info_t img_apc_contrast  				= {1, MENU_B_FLASH_START+0x300D50, NULL,	0xff};    // img_apc_contrast 
code image_item_info_t img_apc_contrast1  				= {1, MENU_B_FLASH_START+0x3037C0, NULL,	0xff};    // img_apc_contrast1 
code image_item_info_t img_apc_color	  				= {1, MENU_B_FLASH_START+0x306230, NULL,	0xff};    // img_apc_color 
code image_item_info_t img_apc_color1	  				= {1, MENU_B_FLASH_START+0x308CA0, NULL,	0xff};    // img_apc_color1
code image_item_info_t img_apc_position  				= {1, MENU_B_FLASH_START+0x30B710, NULL,	0xff};    // img_apc_position 
code image_item_info_t img_apc_position1  				= {1, MENU_B_FLASH_START+0x30E180, NULL,	0xff};    // img_apc_position1 
code image_item_info_t img_apc_phase  					= {1, MENU_B_FLASH_START+0x310BF0, NULL,	0xff};    // img_apc_phase 
code image_item_info_t img_apc_phase1  					= {1, MENU_B_FLASH_START+0x313660, NULL,	0xff};    // img_apc_phase1 
code image_item_info_t img_apc_pclock  					= {1, MENU_B_FLASH_START+0x3160D0, NULL,	0xff};    // img_apc_pclock 
code image_item_info_t img_apc_pclock1 					= {1, MENU_B_FLASH_START+0x318B40, NULL,	0xff};    // img_apc_pclock1 
code image_item_info_t img_apc_autoadj  				= {1, MENU_B_FLASH_START+0x31B5B0, NULL,	0xff};    // img_apc_autoadj 
code image_item_info_t img_apc_autoadj1  				= {1, MENU_B_FLASH_START+0x31E020, NULL,	0xff};    // img_apc_autoadj1 
code image_item_info_t img_apc_autocolor  				= {1, MENU_B_FLASH_START+0x320A90, NULL,	0xff};    // img_apc_autocolor
code image_item_info_t img_apc_autocolor1  				= {1, MENU_B_FLASH_START+0x323500, NULL,	0xff};    // img_apc_autocolor1
code image_item_info_t img_hdmi_menu_bg  				= {1, MENU_B_FLASH_START+0x325F70, NULL,	0x01};    // img_hdmi_menu_bg 
code image_item_info_t img_hdmi_mode  					= {1, MENU_B_FLASH_START+0x3435F0, NULL,	0xff};    // img_hdmi_mode 
code image_item_info_t img_hdmi_mode1  					= {1, MENU_B_FLASH_START+0x34E120, NULL,	0xff};    // img_hdmi_mode1 
code image_item_info_t img_hdmi_setting  				= {1, MENU_B_FLASH_START+0x358B70, NULL,	0xff};    // img_hdmi_setting 
code image_item_info_t img_hdmi_setting1  				= {1, MENU_B_FLASH_START+0x3636A0, NULL,	0xff};    // img_hdmi_setting1 
code image_item_info_t img_display_bg  					= {1, MENU_B_FLASH_START+0x36E1D0, NULL,	0x01};    // img_display_bg 
code image_item_info_t img_display_aspect  				= {1, MENU_B_FLASH_START+0x38B850, NULL,	0xff};    // img_display_aspect 
code image_item_info_t img_display_aspect1 				= {1, MENU_B_FLASH_START+0x38FCB0, NULL,	0xff};    // img_display_aspect1 
code image_item_info_t img_display_osd  				= {1, MENU_B_FLASH_START+0x394110, NULL,	0xff};    // img_display_osd 
code image_item_info_t img_display_osd1  				= {1, MENU_B_FLASH_START+0x398570, NULL,	0xff};    // img_display_osd1 
code image_item_info_t img_display_flip  				= {1, MENU_B_FLASH_START+0x39C9D0, NULL,	0xff};    // img_display_flip 
code image_item_info_t img_display_flip1  				= {1, MENU_B_FLASH_START+0x3A0E30, NULL,	0xff};    // img_display_flip1 
code image_item_info_t img_display_backlight  			= {1, MENU_B_FLASH_START+0x3A5290, NULL,	0xff};    // img_display_backlight 
code image_item_info_t img_display_backlight1  			= {1, MENU_B_FLASH_START+0x3A96F0, NULL,	0xff};    // img_display_backlight1 
code image_item_info_t img_display_resolution  			= {1, MENU_B_FLASH_START+0x3ADB50, NULL,	0xff};    // img_display_resolution 
code image_item_info_t img_display_resolution1  		= {1, MENU_B_FLASH_START+0x3B1FB0, NULL,	0xff};    // img_display_resolution1 
code image_item_info_t img_osd_bg  						= {1, MENU_B_FLASH_START+0x3B6410, NULL,	0x01};    // img_osd_bg 
code image_item_info_t img_osd_timer  					= {1, MENU_B_FLASH_START+0x3D3A90, NULL,	0xff};    // img_osd_timer 
code image_item_info_t img_osd_timer1  					= {1, MENU_B_FLASH_START+0x3DE5C0, NULL,	0xff};    // img_osd_timer1 
code image_item_info_t img_osd_trans  					= {1, MENU_B_FLASH_START+0x3E90F0, NULL,	0xff};    // img_osd_trans 
code image_item_info_t img_osd_trans1  					= {1, MENU_B_FLASH_START+0x3F3C20, NULL,	0xff};    // img_osd_trans1 
code image_item_info_t img_dialog_ok		  			= {1, MENU_B_FLASH_START+0x3FE750, NULL,	0xff};    // img_dialog_ok 
code image_item_info_t img_dialog_ok1	  				= {1, MENU_B_FLASH_START+0x3FF940, NULL,	0xff};    // img_dialog_ok1 
code image_item_info_t img_dialog_cancel  				= {1, MENU_B_FLASH_START+0x400B30, NULL,	0xff};    // img_dialog_cancel 
code image_item_info_t img_dialog_cancel1  				= {1, MENU_B_FLASH_START+0x402740, NULL,	0xff};    // img_dialog_cancel1 
code image_item_info_t img_autoadj_bg  					= {1, MENU_B_FLASH_START+0x404350, NULL,	0xff};    // img_autoadj_bg 
code image_item_info_t img_autocolor_bg  				= {1, MENU_B_FLASH_START+0x40D380, NULL,	0xff};    // img_autocolor_bg 
code image_item_info_t img_flip_bg  					= {1, MENU_B_FLASH_START+0x415EE0, NULL,	0xff};    // img_flip_bg 
code image_item_info_t img_sysrestore_bg  				= {1, MENU_B_FLASH_START+0x41CCE0, NULL,	0xff};    // img_sysrestore_bg 
code image_item_info_t img_sysinfo_bg  					= {1, MENU_B_FLASH_START+0x424BC0, NULL,	0xff};    // img_sysinfo_bg 
code image_item_info_t img_resolution_bg  				= {1, MENU_B_FLASH_START+0x435B40, NULL,	0xff};    // img_resolution_bg 
code image_item_info_t img_slide_bg  					= {1, MENU_B_FLASH_START+0x446390, NULL,	0xff};    // img_slide_bg 
code image_item_info_t img_slide3_bg  					= {1, MENU_B_FLASH_START+0x44ACA0, NULL,	0xff};    // img_slide3_bg 
code image_item_info_t img_slide_gray  					= {1, MENU_B_FLASH_START+0x4502A0, NULL,	0xff};    // img_slide_gray 
code image_item_info_t img_slide_red  					= {1, MENU_B_FLASH_START+0x450DE0, NULL,	0xff};    // img_slide_red 
code image_item_info_t img_slide_left  					= {1, MENU_B_FLASH_START+0x451920, NULL,	0xff};    // img_slide_left 
code image_item_info_t img_slide_left1  				= {1, MENU_B_FLASH_START+0x4528E0, NULL,	0xff};    // img_slide_lef1t 
code image_item_info_t img_slide_right  				= {1, MENU_B_FLASH_START+0x4538A0, NULL,	0xff};    // img_slide_right 
code image_item_info_t img_slide_right1  				= {1, MENU_B_FLASH_START+0x454860, NULL,	0xff};    // img_slide_right1 
code image_item_info_t img_slide_backlight  			= {1, MENU_B_FLASH_START+0x455820, NULL,	0xff};    // img_slide_backlight 
code image_item_info_t img_slide_bright  				= {1, MENU_B_FLASH_START+0x456260, NULL,	0xff};    // img_slide_bright 
code image_item_info_t img_slide_clock  				= {1, MENU_B_FLASH_START+0x456DD0, NULL,	0xff};    // img_slide_clock 
code image_item_info_t img_slide_contrast  				= {1, MENU_B_FLASH_START+0x4577A0, NULL,	0xff};    // img_slide_contrast 
code image_item_info_t img_slide_rgb	  				= {1, MENU_B_FLASH_START+0x457F50, NULL,	0xff};    // img_slide_rgb 
code image_item_info_t img_slide_hue  					= {1, MENU_B_FLASH_START+0x458390, NULL,	0xff};    // img_slide_hue 
code image_item_info_t img_slide_phase  				= {1, MENU_B_FLASH_START+0x4587B0, NULL,	0xff};    // img_slide_phase 
code image_item_info_t img_slide_saturate  				= {1, MENU_B_FLASH_START+0x458D70, NULL,	0xff};    // img_slide_saturate 
code image_item_info_t img_slide_sharp  				= {1, MENU_B_FLASH_START+0x459690, NULL,	0xff};    // img_slide_sharp 
code image_item_info_t img_slide_timer  				= {1, MENU_B_FLASH_START+0x45A1C0, NULL,	0xff};    // img_slide_timer 
code image_item_info_t img_slide_trans  				= {1, MENU_B_FLASH_START+0x45A760, NULL,	0xff};    // img_slide_trasn 
code image_item_info_t img_position_bg  				= {1, MENU_B_FLASH_START+0x45B5C0, NULL,	0xff};    // img_position_bg 
code image_item_info_t img_position_box_gray  			= {1, MENU_B_FLASH_START+0x464BD0, NULL,	0xff};    // img_position_box_gray 
code image_item_info_t img_position_box_red  			= {1, MENU_B_FLASH_START+0x46B3F0, NULL,	0xff};    // img_position_box_red 
code image_item_info_t img_position_up  				= {1, MENU_B_FLASH_START+0x471A70, NULL,	0xff};    // img_position_up 
code image_item_info_t img_position_down  				= {1, MENU_B_FLASH_START+0x472410, NULL,	0xff};    // img_position_down 
code image_item_info_t img_position_left  				= {1, MENU_B_FLASH_START+0x472DB0, NULL,	0xff};    // img_position_left 
code image_item_info_t img_position_right  				= {1, MENU_B_FLASH_START+0x473750, NULL,	0xff};    // img_position_right 
code image_item_info_t img_popup_aspect_bg  			= {1, MENU_B_FLASH_START+0x4740F0, NULL,	0xff};    // img_aspect_bg 
code image_item_info_t img_popup_aspect_normal 			= {1, MENU_B_FLASH_START+0x485720, NULL,	0xff};    // img_aspect_normal 
code image_item_info_t img_popup_aspect_normal1 		= {1, MENU_B_FLASH_START+0x486540, NULL,	0xff};    // img_aspect_normal1 
code image_item_info_t img_popup_aspect_normal_select	= {1, MENU_B_FLASH_START+0x487320, NULL,	0xff};    // img_aspect_normal_select 
code image_item_info_t img_popup_aspect_zoom  			= {1, MENU_B_FLASH_START+0x487F70, NULL,	0xff};    // img_aspect_zoom 
code image_item_info_t img_popup_aspect_zoom1  			= {1, MENU_B_FLASH_START+0x488AD0, NULL,	0xff};    // img_aspect_zoom1 
code image_item_info_t img_popup_aspect_zoom_select		= {1, MENU_B_FLASH_START+0x4896E0, NULL,	0xff};    // img_aspect_zoom_select 
code image_item_info_t img_popup_aspect_full  			= {1, MENU_B_FLASH_START+0x48A300, NULL,	0xff};    // img_aspect_full
code image_item_info_t img_popup_aspect_full1  			= {1, MENU_B_FLASH_START+0x48AC80, NULL,	0xff};    // img_aspect_full1
code image_item_info_t img_popup_aspect_full_select		= {1, MENU_B_FLASH_START+0x48B630, NULL,	0xff};    // img_aspect_full_select
code image_item_info_t img_popup_aspect_pano  			= {1, MENU_B_FLASH_START+0x48C220, NULL,	0xff};    // img_aspect_pano
code image_item_info_t img_popup_aspect_pano1  			= {1, MENU_B_FLASH_START+0x48D330, NULL,	0xff};    // img_aspect_pano1
code image_item_info_t img_popup_aspect_pano_sel		= {1, MENU_B_FLASH_START+0x48E460, NULL,	0xff};    // img_aspect_pano_sel
code image_item_info_t img_dvi_mode_bg  				= {1, MENU_B_FLASH_START+0x48F050, NULL,	0xff};    // img_dvi_mode_bg 
code image_item_info_t img_dvi_mode_24bit  				= {1, MENU_B_FLASH_START+0x4AD3A0, NULL,	0xff};    // img_dvi_mode_24bit 
code image_item_info_t img_dvi_mode_24bit1  			= {1, MENU_B_FLASH_START+0x4AEC30, NULL,	0xff};    // img_dvi_mode_24bit1 
code image_item_info_t img_dvi_mode_16bit  				= {1, MENU_B_FLASH_START+0x4B05C0, NULL,	0xff};    // img_dvi_mode_16bit 
code image_item_info_t img_dvi_mode_16bit1  			= {1, MENU_B_FLASH_START+0x4B1F20, NULL,	0xff};    // img_dvi_mode_16bit1 
code image_item_info_t img_dvi_mode_select24  			= {1, MENU_B_FLASH_START+0x4B38D0, NULL,	0xff};    // img_dvi_mode_select24 
code image_item_info_t img_dvi_mode_select16  			= {1, MENU_B_FLASH_START+0x4B4B20, NULL,	0xff};    // img_dvi_mode_select16 
code image_item_info_t img_hdmi_mode_bg  				= {1, MENU_B_FLASH_START+0x4B5DB0, NULL,	0xff};    // img_hdmi_mode_bg 
code image_item_info_t img_hdmi_mode_pc  				= {1, MENU_B_FLASH_START+0x4D4100, NULL,	0xff};    // img_hdmi_mode_pc 
code image_item_info_t img_hdmi_mode_pc1  				= {1, MENU_B_FLASH_START+0x4D56C0, NULL,	0xff};    // img_hdmi_mode_pc1 
code image_item_info_t img_hdmi_mode_tv  				= {1, MENU_B_FLASH_START+0x4D6C10, NULL,	0xff};    // img_hdmi_mode_tv 
code image_item_info_t img_hdmi_mode_tv1  				= {1, MENU_B_FLASH_START+0x4D81D0, NULL,	0xff};    // img_hdmi_mode_tv1 
code image_item_info_t img_hdmi_mode_selectPC  			= {1, MENU_B_FLASH_START+0x4D9770, NULL,	0xff};    // img_hdmi_mode_selectPC 
code image_item_info_t img_hdmi_mode_selectTV  			= {1, MENU_B_FLASH_START+0x4DB0E0, NULL,	0xff};    // img_hdmi_mode_selectTV 
code image_item_info_t img_wait  						= {1, MENU_B_FLASH_START+0x4DCA50, NULL,	0x00};    // img_wait. 48x50x10 


//=================================
// FPGA TEST IMAGE
//=================================
//code image_item_info_t img_fpga_200			= {1, FPGA_TEST_IMG+0x000000, NULL, 0xff};
//code image_item_info_t img_fpga_300			= {1, FPGA_TEST_IMG+0x0005D7, NULL, 0xff};
//code image_item_info_t img_fpga_400			= {1, FPGA_TEST_IMG+0x000AE9, NULL, 0xff};
//code image_item_info_t img_fpga_800			= {1, FPGA_TEST_IMG+0x000FF3, NULL, 0xff};

//#define FPGA_TEST_IMG2	0x330000
//code image_item_info_t img_bgr_bar			= {1, FPGA_TEST_IMG2+0x000000, NULL, 0xff};
//code image_item_info_t img_alpha_bar		= {1, FPGA_TEST_IMG2+0x013C90, NULL, 0xff};
//code image_item_info_t img_1366_768			= {1, 0x800000, NULL, 0xff};  /* 0x800000~0xA00C20 */
//code image_item_info_t img_1366_ggg			= {1, 0x900610, NULL, 0xff};  /* 0x800000~0xA00C20 */
//#define FPGA_TEST_BAR864	0xA10000		/* +0x9F4A */
//code image_item_info_t img_bar864_8			= {1, FPGA_TEST_BAR864+0x000000, NULL, 0xff}; // Bar327x45_8bpp   
//code image_item_info_t img_bar864_8C		= {1, FPGA_TEST_BAR864+0x003D8B, NULL, 0xff}; // Bar327x45_8bpp_Rle  
//code image_item_info_t img_bar864_6			= {1, FPGA_TEST_BAR864+0x004904, NULL, 0xff}; // Bar327x45_6bpp  
//code image_item_info_t img_bar864_6C		= {1, FPGA_TEST_BAR864+0x007531, NULL, 0xff}; // Bar327x45_6bpp_Rle  
//code image_item_info_t img_bar864_4			= {1, FPGA_TEST_BAR864+0x007C9D, NULL, 0xff}; // Bar327x45_4bpp  
//code image_item_info_t img_bar864_4C		= {1, FPGA_TEST_BAR864+0x0099AB, NULL, 0xff}; // Bar327x45_4bpp_Rle  


//==============================
// demo image 1024x600
//===============================
code image_item_info_t img_demo_01			= {1, SLIDESHOW_START+0x000000, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img_demo_02			= {1, SLIDESHOW_START+0x096410, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img_demo_03			= {1, SLIDESHOW_START+0x0990E0, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img_demo_04			= {1, SLIDESHOW_START+0x09B670, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img_demo_05			= {1, SLIDESHOW_START+0x09D710, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img_demo_06			= {1, SLIDESHOW_START+0x133B20, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img_demo_07			= {1, SLIDESHOW_START+0x1C9F30, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img_demo_08			= {1, SLIDESHOW_START+0x260340, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img_demo_09			= {1, SLIDESHOW_START+0x2F6750, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img_demo_10			= {1, SLIDESHOW_START+0x38CB60, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img_demo_11			= {1, SLIDESHOW_START+0x422F70, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img_demo_12			= {1, SLIDESHOW_START+0x4B9380, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img_demo_13			= {1, SLIDESHOW_START+0x54F790, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img_demo_14			= {1, SLIDESHOW_START+0x5E5BA0, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img_demo_15			= {1, SLIDESHOW_START+0x67BFB0, NULL, 0xff}; // Bar327x45_8bpp   
//code image_item_info_t img_demo_16			= {1, SLIDESHOW_START+0x7123C0, NULL, 0xff}; // Bar327x45_8bpp   
//I need a custom logo...I will remove img_demo_16 and use igm_custom
code image_item_info_t img_customer			= {1, 0xF40000,                 NULL, 0xff}; // Bar327x45_8bpp   


//==============================
// demo image 1280x800
//===============================
#define DEMO_1280x800	0x1000000
code image_item_info_t img1_demo_01			= {1, DEMO_1280x800+0x000000, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img1_demo_02			= {1, DEMO_1280x800+0x0FA410, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img1_demo_03			= {1, DEMO_1280x800+0x1F4820, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img1_demo_04			= {1, DEMO_1280x800+0x2EEC30, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img1_demo_05			= {1, DEMO_1280x800+0x3E9040, NULL, 0xff}; // Bar327x45_8bpp   








code image_item_info_t img_screen_1280_4 				= {1, MENU_B_FLASH_START+0x108230, NULL,	0xFF};


//typedef struct MonOsdData_s {
//	struct image_item_info_s *image;
//	BYTE name[10];	
//} MonOsdData_t;

code MonOsdData_t MonSOsdImgTable[] = {
	{ &img_logo, 			"logo     "}, //0
	{ &img_main_bg, 		"main_bg  "}, //1
	{ &img_main_input,		"input0   "}, //2
	{ &img_main_input1,		"input1   "}, //3
	{ &img_main_system,		"system0  "}, //4
	{ &img_main_system1,	"system1  "}, //5
	{ &img_input_bg_bottom,	"input_bg0"}, //6
	{ &img_input_bg_top,	"input_bg1"}, //7
	{ &img_input_cvbs,		"cvbs0    "}, //8
	{ &img_input_cvbs1,		"cvbs1    "}, //9
	{ &img_navi_return,		"return0  "}, //10
	{ &img_navi_return1,	"return1  "}, //11
	//===================================
	{ &img_slide_bg, 		"slide_bg "}, //12
	{ &img_slide3_bg, 		"slide3_bg"}, //13
	{ &img_input_hdmi, 		"hdmi     "}, //14
//	{ &img_dvi_mode_24bit, 	"dvi24    "}, //15
	{ &img_screen_1280_4   ,"1280_4   "}, //15
	//===================================
	{ &img_dvi_mode_16bit, 	"dvi16    "},
	{ &img_dvi_mode_16bit1, "dvi16-1  "},	//17
//	{ &img_1366_768, 		"1366_768"},	 //18
//	{ &img_1366_ggg, 		"1366_ggg"},	 //19
//	//===================================
//	{ &img_bar864_8, 		"Bar8bpp"},
//	{ &img_bar864_8C, 		"Bar8bppC"},
//	{ &img_bar864_6, 		"Bar6bpp"},
//	{ &img_bar864_6C, 		"Bar6bppC"},
//	{ &img_bar864_4, 		"Bar4bpp"},
//	{ &img_bar864_4C, 		"Bar4bppC"},
//	//===================================

	{ &img_demo_01, 		"1_NoRLE  "},	 //18
	{ &img_demo_02, 		"2_RLE8   "},	 //19
	{ &img_demo_03, 		"3_RLE6   "},	 //20
	{ &img_demo_04, 		"4_RLE4   "},	 //21
	{ &img_demo_05, 		"5_Simpson"},	 //22
	{ &img_demo_06, 		"6_Win7   "},	 //23
	{ &img_demo_07, 		"7_Speed  "},	 //24
	{ &img_demo_08, 		"8_tangled"},	 //25
	{ &img_demo_09, 		"9_girl1  "},	 //26
	{ &img_demo_10, 		"10_girl2 "},	 //27
	{ &img_demo_11, 		"11_girl3 "},	 //28
	{ &img_demo_12, 		"12_girl4 "},	 //29
	{ &img_demo_13, 		"13_girl5 "},	 //30
	{ &img_demo_14, 		"14_girl6 "},	 //31
	{ &img_demo_15, 		"15_girl7 "},	 //32
	//{ &img_demo_16, 		"16_girl8 "},	 //33
	{ &img_customer, 		"customer "},	 //33

	{ &img1_demo_01, 		"FFantasy2"},	 //34 <--
	{ &img1_demo_02, 		"TombRaide"},	 //35
	{ &img1_demo_03, 		"ice_age  "},	 //36
	{ &img1_demo_04, 		"Luisana1 "},	 //37
	{ &img1_demo_05, 		"Luisana2 "},	 //38
	{ NULL,                 NULL       }
};
/**
* description
*	get max number of MonSOsdImgTable[]
*/
BYTE MonSOsdImg_GetMax(void)
{
	BYTE max;
	
	max=0;
	while(1) {
		if(MonSOsdImgTable[max].image == NULL)
			break;
		max++;
	}
	return max;
}
/**
* description
*	get item from MonSOsdImgTable[]
* @return
*	fail: null
*	success: image_item_info
*/
struct image_item_info_s *MonSOsdImg_Get_ImgeItem(BYTE num)
{
	BYTE max;

	max = MonSOsdImg_GetMax();
	if(num >= max) {
		Printf("\n\rFail req:%bd max:%bd", num, max);
		return NULL;
	}

	Printf("\n\rMonSOsd %bd %s",num,MonSOsdImgTable[num].name);
	return MonSOsdImgTable[num].image;
}


extern menu_image_header_t header_table;
extern BYTE MenuPrepareImageHeader(struct image_item_info_s *image);
/**
* description
*
*	Read MonSOsdImgTable[] and print the name and the header info.
*/
void MonSOsdImgInfo(void)
{
	struct image_item_info_s *image;
	//image_info_t *info;
	menu_image_header_t *header = &header_table;	//link header buffer.
	BYTE i;

	for(i=0; i < (12+4+2+1+6); i++)
	{
		Printf("\n\r%02bd %s",i,MonSOsdImgTable[i].name);
		image = MonSOsdImgTable[i].image;
		Printf(" loc:%lx alpha:%bx",image->loc,image->alpha);

		//prepare header
		MenuPrepareImageHeader(image);

		//header info
		Printf(" bpp%bd", header->bpp);
		Printf(" rle%bd", header->rle);
		Printf(" %dx%d", header->dx, header->dy);
		Printf(" alpha:%2bx",image->alpha);
		Printf(" lut%s size:%d*4",header->lut_type? "s": " ", header->lut_size >> 2);		 
	}
}
/**
* description
*	download palette of the select item
*/
BYTE MonOsdLutLoad(BYTE img_n, BYTE sosd_win, WORD lut)
{
	struct image_item_info_s *image;
	BYTE ret;
	menu_image_header_t *header = &header_table;	//link header buffer.


	ePrintf("\n\rMonOsdLutLoad(%bd,%bd,%d)",img_n,lut);

	//BYTE i;
	image = MonSOsdImgTable[img_n].image;
	//prepare header
	ret=MenuPrepareImageHeader(image);
	if(ret) {
		ePrintf("  Header Fail!!");
		return ret;
	}
	//Load Palette
	//SpiOsdLoadLUT(sosd_win, header->lut_type, lut, header->lut_size, header->lut_loc,0xFF);
	SOsd_SetLut(sosd_win, header->lut_type, lut, header->lut_size, header->lut_loc,0xFF);
	SOsd_UpdateLut(sosd_win, 0);	

	ret = SpiOsdCheckLut(sosd_win, header->lut_type, lut, header->lut_size, header->lut_loc);	
	return ret;
}

extern BYTE UseSOsdHwBuff;
//extern WORD SOsdHwBuff_alpha_A;
//extern WORD SOsdHwBuff_alpha_B;
/**
* description
*	draw image
*/
void MonOsdImgLoad(BYTE img_n, BYTE sosd_win, WORD item_lut)
{
	struct image_item_info_s *image;
	menu_image_header_t *header = &header_table;	//link header buffer.
//	BYTE i;
	WORD sx,sy;

	Printf("\n\rMonOsdImgLoad(%bd,%bd,%d)",img_n,sosd_win,item_lut);

#if 0
	UseSOsdHwBuff=1;
#endif
	sx=sy=0;
//	SOsd_CleanReg();

	image = MonSOsdImgTable[img_n].image;

	//prepare header
	MenuPrepareImageHeader(image);


	//see MenuDrawCurrImage
	//fill out sosd_buff
	SOsd_SetSpiStartOffset( sosd_win, header->image_loc); 
	SOsd_SetImageWidthHeight( sosd_win, header->dx, header->dy );
	SOsd_SetScreen( sosd_win, sx, sy, header->dx, header->dy );
	if(sosd_win==0) {
		SOsd_SetWin0ImageOffsetXY( 0, 0 );
		SOsd_SetWin0Animation( 1, 0, 0, 0);
	}
	if(image->alpha != 0xFF)
		SOsd_SetPixelAlpha( sosd_win, ON );
	else {
		SOsd_SetGlobalAlpha( sosd_win, 0 /*EE_Read(EEP_OSD_TRANSPARENCY)*/);
	}
	SOsd_SetPixelWidth(sosd_win, header->bpp);
	SOsd_SetLutOffset(sosd_win, item_lut);

	SOsd_Enable( sosd_win, ON );
	//
	//write to HW
	//
	if(UseSOsdHwBuff) 
	{
		if(header->rle)
			SOsd_SetRlc(sosd_win,header->bpp,header->rle);
		SOsd_SetLut(sosd_win, header->lut_type, item_lut, header->lut_size, header->lut_loc, image->alpha);
	
		//pixel alpha blending. after load Palette
		//if(image->alpha != 0xFF)
		//	//SOsdHwBuffSetAlpha(sosd_win, item_lut+image->alpha);
		//	SOsd_SetPixelAlphaIndex(sosd_win,image->alpha);

		SOsd_UpdateReg(sosd_win, sosd_win); //SOsd_show();
		UseSOsdHwBuff = 0;


#if 1
//void SOsd_UpdateLut(BYTE win, BYTE fAlpha);
//void SOsd_UpdatePixelAlpha(BYTE win);

		SOsd_UpdatePixelAlpha(sosd_win);
#else
		//update ALPHA
//		if(SOsdHwBuff_alpha_A != 0xFFFF) {
//			WriteTW88(REG410, 0xc3 );    		// LUT Write Mode, En & byte ptr inc.
//  
//			if(SOsdHwBuff_alpha_A >> 8)	WriteTW88(REG410, ReadTW88(REG410) | 0x08);	//support 512 palette
//			else            			WriteTW88(REG410, ReadTW88(REG410) & 0xF7);
//			WriteTW88(REG411, (BYTE)SOsdHwBuff_alpha_A ); 	// alpha index
//			WriteTW88(REG412, 0x7F/*value*/ ); 			// alpha value
//  
//			SOsdHwBuff_alpha_A = 0xFFFF;
//		}
//		if(SOsdHwBuff_alpha_B != 0xFFFF) {
//			WriteTW88(REG410, 0xc3 | 0x04);    		// LUT Write Mode, En & byte ptr inc.
//
//			WriteTW88(REG410, ReadTW88(REG410) & 0xF7);
//			WriteTW88(REG411, (BYTE)SOsdHwBuff_alpha_B ); 	// alpha index
//			WriteTW88(REG412, 0x7F/*value*/ ); 			// alpha value
//
//			SOsdHwBuff_alpha_B = 0xFFFF;
//		}
#endif
	}
	//else 
	{
		//WaitVBlank(1);
		if(header->rle) {	//need RLE ?
			SOsd_SetRlc( sosd_win, header->bpp,header->rle);
			SOsd_UpdateRlc();
		}	
		else {
			//We using RLE only on the background.
			//if(item == 0) {
			//	SpiOsdDisableRlcReg(??winno)
			//	SpiOsdRlcReg( 0,0,0); //BK110217
			//}
		}
		WaitVBlank(1);
	
		//Load Palette
		//SpiOsdLoadLUT(header->lut_type, menu_item->lut, header->lut_size, header->lut_loc, image->alpha);
		//SpiOsdLoadLUT(sosd_win, header->lut_type, item_lut, header->lut_size, header->lut_loc, image->alpha);
		SOsd_SetLut(sosd_win, header->lut_type, item_lut, header->lut_size, header->lut_loc, image->alpha);
		SOsd_UpdateLut(sosd_win, 1);	

		//WaitVBlank(1);
		//update HW
		SOsd_UpdateReg(sosd_win, sosd_win);
	
	}
}


#endif //..SUPPORT_SPIOSD
#endif //..SPIOSD_USE_1024X600_IMG