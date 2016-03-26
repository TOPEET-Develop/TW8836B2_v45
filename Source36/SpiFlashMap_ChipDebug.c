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
//        |                            |
//010_0000+----------------------------+    
//        |                            |
//        |     MENU IMG               |
//        |                            |
//        |                            |
//05E_0000+----------------------------+    
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
//        |  img offset 0x090D00       | 0x69-0D00
//        | LUT for Pigion&Rose        |
//        |  offset 0x217700           | 0x81_7A00
//        |                            |
//082_0000+----------------------------+
//        | Slide Image                |    
//        |                            |
//        |  size 7B0000               |
//        |                            |
//0FD_0000+----------------------------+
//        |                            |
//        |                            |
//100_0000+----------------------------+
//        |                            |
//        |                            |
//200_0000+----------------------------+
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

#include "SOsd.h"
#include "FOsd.h"

#include "SpiFlashMap.h"
#include "SOsdMenu.h"

#ifdef SPIOSD_USE_800X480_IMG
#ifdef SUPPORT_SPIOSD

#define MAP0_START	SFLASH_IMG_ADDR
//#define MAP0_START	0

//===================================
//BKANG test area
//0x300000
//===================================



//code image_info_t img_main_test1_header = {1, 0x86, 800, 480 /*,0x01803E*/};
//code image_info_t img_main_test2_header = {1, 0x86, 800, 480 /*,0x015307*/};
//code image_info_t img_main_test3_header = {1, 0x86, 800, 480 /*,0x0136E8*/};


//code image_item_info_t img_main_test1	= {0, MENU_TEST_FLASH_START+0x000000, &img_main_test1_header, 0xff};    
//code image_item_info_t img_main_test2	= {0, MENU_TEST_FLASH_START+0x01803E, &img_main_test1_header, 0xff};    
//code image_item_info_t img_main_test3	= {0, MENU_TEST_FLASH_START+0x02D345, &img_main_test1_header, 0xff};    





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





//code image_info_t img_logo_header = { };

//code image_item_info_t img_ = {+0x000000, 0x0010F0 },    // FontAll 

//=================================
// FPGA TEST IMAGE
//=================================


code image_item_info_t img_logo							= {1, MENU_B_FLASH_START+0x000000, NULL,	0xff};    // Intersil-Techwell
code image_item_info_t img_navi_return  				= {1, MENU_B_FLASH_START+0x0049C0, NULL,	0xff};    // img_navi_return 
code image_item_info_t img_navi_return1  				= {1, MENU_B_FLASH_START+0x0054D0, NULL,	0xff};    // img_navi_return1 
code image_item_info_t img_navi_close  					= {1, MENU_B_FLASH_START+0x005FE0, NULL,	0xff};    // img_navi_close 
code image_item_info_t img_navi_close1  				= {1, MENU_B_FLASH_START+0x006AF0, NULL,	0xff};    // img_navi_close1 
code image_item_info_t img_navi_setup  					= {1, MENU_B_FLASH_START+0x007600, NULL,	0xff};    // img_navi_setup 
code image_item_info_t img_navi_setup1  				= {1, MENU_B_FLASH_START+0x008110, NULL,	0xff};    // img_navi_setup1 
code image_item_info_t img_main_bg  					= {1, MENU_B_FLASH_START+0x008C20, NULL,	0xff};    // img_main_bg 
code image_item_info_t img_main_input  					= {1, MENU_B_FLASH_START+0x01E260, NULL,	0xff};    // img_main_input 
code image_item_info_t img_main_input1  				= {1, MENU_B_FLASH_START+0x021930, NULL,	0xff};    // img_main_input1 
code image_item_info_t img_main_audio  					= {1, MENU_B_FLASH_START+0x025000, NULL,	0xff};    // img_main_audio 
code image_item_info_t img_main_audio1  				= {1, MENU_B_FLASH_START+0x028350, NULL,	0xff};    // img_main_audio1 
code image_item_info_t img_main_system  				= {1, MENU_B_FLASH_START+0x02B6A0, NULL,	0xff};    // img_main_system 
code image_item_info_t img_main_system1  				= {1, MENU_B_FLASH_START+0x02F9D0, NULL,	0xff};    // img_main_system1 
code image_item_info_t img_input_bg_bottom  			= {1, MENU_B_FLASH_START+0x033D00, NULL,	0x5B};    // img_input_bg_bottom 
code image_item_info_t img_input_bg_top  				= {1, MENU_B_FLASH_START+0x0359B0, NULL,	0x5B};    // img_input_bg_top 
code image_item_info_t img_input_select  				= {1, MENU_B_FLASH_START+0x043BA0, NULL,	0x5B};    // img_input_select 
code image_item_info_t img_input_cvbs 		 			= {1, MENU_B_FLASH_START+0x044010, NULL,	0xff};    // img_input_cvbs 
code image_item_info_t img_input_cvbs1 		 			= {1, MENU_B_FLASH_START+0x0453A0, NULL,	0xff};    // img_input_cvbs1 
code image_item_info_t img_input_svideo  				= {1, MENU_B_FLASH_START+0x046770, NULL,	0xff};    // img_input_svideo 
code image_item_info_t img_input_svideo1  				= {1, MENU_B_FLASH_START+0x047EB0, NULL,	0xff};    // img_input_svideo1 
code image_item_info_t img_input_ypbpr  				= {1, MENU_B_FLASH_START+0x0495F0, NULL,	0xff};    // img_input_Ypbpr 
code image_item_info_t img_input_ypbpr1  				= {1, MENU_B_FLASH_START+0x04AC10, NULL,	0xff};    // img_input_Ypbpr1 
code image_item_info_t img_input_pc  					= {1, MENU_B_FLASH_START+0x04C1F0, NULL,	0xff};    // img_input_pc 
code image_item_info_t img_input_pc1  					= {1, MENU_B_FLASH_START+0x04D780, NULL,	0xff};    // img_input_pc1 
code image_item_info_t img_input_dvi  					= {1, MENU_B_FLASH_START+0x04ED40, NULL,	0xff};    // img_input_dvi 
code image_item_info_t img_input_dvi1  					= {1, MENU_B_FLASH_START+0x050190, NULL,	0xff};    // img_input_dvi1 
code image_item_info_t img_input_hdmi  					= {1, MENU_B_FLASH_START+0x0515E0, NULL,	0xff};    // img_input_hdmi 
code image_item_info_t img_input_hdmi1  				= {1, MENU_B_FLASH_START+0x052BB0, NULL,	0xff};    // img_input_hdmi1 
code image_item_info_t img_input_ext  					= {1, MENU_B_FLASH_START+0x054100, NULL,	0xff};    // img_input_ext 
code image_item_info_t img_input_ext1  					= {1, MENU_B_FLASH_START+0x0553A0, NULL,	0xFF};    // img_input_ext1 
code image_item_info_t img_input_return  				= {1, MENU_B_FLASH_START+0x056640, NULL,	0xff};    // img_input_return 
code image_item_info_t img_input_return1  				= {1, MENU_B_FLASH_START+0x056CE0, NULL,	0xFF};    // img_input_return1 
code image_item_info_t img_yuv_menu_bg  				= {1, MENU_B_FLASH_START+0x057380, NULL,	0x00};    // img_yuv_menu_bg 
code image_item_info_t img_yuv_bright  					= {1, MENU_B_FLASH_START+0x069C10, NULL,	0xff};    // img_yuv_bright 
code image_item_info_t img_yuv_bright1  				= {1, MENU_B_FLASH_START+0x06C800, NULL,	0xff};    // img_yuv_bright1 
code image_item_info_t img_yuv_contrast  				= {1, MENU_B_FLASH_START+0x06F3F0, NULL,	0xff};    // img_yuv_contrast 
code image_item_info_t img_yuv_contrast1  				= {1, MENU_B_FLASH_START+0x071FE0, NULL,	0xff};    // img_yuv_contrast1 
code image_item_info_t img_dialog_ok		  			= {1, MENU_B_FLASH_START+0x074BD0, NULL,	0xff};    // img_dialog_ok 
code image_item_info_t img_dialog_ok1	  				= {1, MENU_B_FLASH_START+0x0757E0, NULL,	0xff};    // img_dialog_ok1 
code image_item_info_t img_dialog_cancel  				= {1, MENU_B_FLASH_START+0x0763F0, NULL,	0xff};    // img_dialog_cancel 
code image_item_info_t img_dialog_cancel1  				= {1, MENU_B_FLASH_START+0x077640, NULL,	0xff};    // img_dialog_cancel1 
code image_item_info_t img_slide_bg  					= {1, MENU_B_FLASH_START+0x078890, NULL,	0xff};    // img_slide_bg 
code image_item_info_t img_slide_gray  					= {1, MENU_B_FLASH_START+0x07A0E0, NULL,	0xff};    // img_slide_gray 
code image_item_info_t img_slide_red  					= {1, MENU_B_FLASH_START+0x07A8F0, NULL,	0xff};    // img_slide_red 
code image_item_info_t img_slide_red1  					= {1, MENU_B_FLASH_START+0x07B100, NULL,	0xff};    // img_slide_red 
code image_item_info_t img_slide_bright  				= {1, MENU_B_FLASH_START+0x07B910, NULL,	0xff};    // img_slide_bright 
code image_item_info_t img_slide_contrast  				= {1, MENU_B_FLASH_START+0x07C0D0, NULL,	0xff};    // img_slide_contrast 
code image_item_info_t img_screen_800_4_c46				= {1, MENU_B_FLASH_START+0x07C3F0, NULL,	0xFF};
code image_item_info_t img_screen_800_4 				= {1, MENU_B_FLASH_START+0x08E390, NULL,	0xFF};
code image_item_info_t img_screen_1024_4 				= {1, MENU_B_FLASH_START+0x0BD1E0, NULL,	0xFF};
code image_item_info_t img_screen_1280_4 				= {1, MENU_B_FLASH_START+0x108230, NULL,	0xFF};
code image_item_info_t img_screen_1366_4 				= {1, MENU_B_FLASH_START+0x185280, NULL,	0xFF};
code image_item_info_t img_800          				= {1, MENU_B_FLASH_START+0x2053D0, NULL,	0xFF};
code image_item_info_t img_400          				= {1, MENU_B_FLASH_START+0x2059B0, NULL,	0xFF};
code image_item_info_t img_300          				= {1, MENU_B_FLASH_START+0x205ED0, NULL,	0xFF};
code image_item_info_t img_200          				= {1, MENU_B_FLASH_START+0x2063E0, NULL,	0xFF};
code image_item_info_t img_alphabar          			= {1, MENU_B_FLASH_START+0x2068E0, NULL,	0xFF};
code image_item_info_t img_cbar_luts       				= {1, MENU_B_FLASH_START+0x21A570, NULL,	0xFF};
code image_item_info_t img_cbar_lut       				= {1, MENU_B_FLASH_START+0x22E200, NULL,	0xFF};
code image_item_info_t img_rle80       					= {1, MENU_B_FLASH_START+0x241E90, NULL,	0xFF};
code image_item_info_t img_rle82       					= {1, MENU_B_FLASH_START+0x255B20, NULL,	0xFF};
code image_item_info_t img_rle83       					= {1, MENU_B_FLASH_START+0x264990, NULL,	0xFF};
code image_item_info_t img_rle84       					= {1, MENU_B_FLASH_START+0x26B330, NULL,	0xFF};
code image_item_info_t img_rle85       					= {1, MENU_B_FLASH_START+0x26F420, NULL,	0xFF};
code image_item_info_t img_rle86       					= {1, MENU_B_FLASH_START+0x271B10, NULL,	0xFF};
code image_item_info_t img_rle87       					= {1, MENU_B_FLASH_START+0x2732B0, NULL,	0xFF};
code image_item_info_t img_rle88       					= {1, MENU_B_FLASH_START+0x2740C0, NULL,	0xFF};
code image_item_info_t img_rle89       					= {1, MENU_B_FLASH_START+0x274B00, NULL,	0xFF};
code image_item_info_t img_rle8A       					= {1, MENU_B_FLASH_START+0x2750F0, NULL,	0xFF};
code image_item_info_t img_rle8F       					= {1, MENU_B_FLASH_START+0x275700, NULL,	0xFF};
code image_item_info_t img_rle60       					= {1, MENU_B_FLASH_START+0x275B20, NULL,	0xFF};
code image_item_info_t img_rle62       					= {1, MENU_B_FLASH_START+0x284690, NULL,	0xFF};
code image_item_info_t img_rle63       					= {1, MENU_B_FLASH_START+0x290AF0, NULL,	0xFF};
code image_item_info_t img_rle64       					= {1, MENU_B_FLASH_START+0x2961F0, NULL,	0xFF};
code image_item_info_t img_rle65       					= {1, MENU_B_FLASH_START+0x299730, NULL,	0xFF};
code image_item_info_t img_rle66       					= {1, MENU_B_FLASH_START+0x29B670, NULL,	0xFF};
code image_item_info_t img_rle67       					= {1, MENU_B_FLASH_START+0x29C8A0, NULL,	0xFF};
code image_item_info_t img_rle68       					= {1, MENU_B_FLASH_START+0x29D280, NULL,	0xFF};
code image_item_info_t img_rle69       					= {1, MENU_B_FLASH_START+0x29D910, NULL,	0xFF};
code image_item_info_t img_rle6A       					= {1, MENU_B_FLASH_START+0x29DBD0, NULL,	0xFF};
code image_item_info_t img_rle6F       					= {1, MENU_B_FLASH_START+0x29DEB0, NULL,	0xFF};
code image_item_info_t img_rle40       					= {1, MENU_B_FLASH_START+0x29DFD0, NULL,	0xFF};
code image_item_info_t img_rle42       					= {1, MENU_B_FLASH_START+0x2A7C60, NULL,	0xFF};
code image_item_info_t img_rle43       					= {1, MENU_B_FLASH_START+0x2AE500, NULL,	0xFF};
code image_item_info_t img_rle44       					= {1, MENU_B_FLASH_START+0x2B2BA0, NULL,	0xFF};
code image_item_info_t img_rle45       					= {1, MENU_B_FLASH_START+0x2B5770, NULL,	0xFF};
code image_item_info_t img_rle46       					= {1, MENU_B_FLASH_START+0x2B7140, NULL,	0xFF};
code image_item_info_t img_rle47       					= {1, MENU_B_FLASH_START+0x2B8040, NULL,	0xFF};
code image_item_info_t img_rle48       					= {1, MENU_B_FLASH_START+0x2B8830, NULL,	0xFF};
code image_item_info_t img_rle49       					= {1, MENU_B_FLASH_START+0x2B8D50, NULL,	0xFF};
code image_item_info_t img_rle4A       					= {1, MENU_B_FLASH_START+0x2B8F20, NULL,	0xFF};
code image_item_info_t img_rle4F       					= {1, MENU_B_FLASH_START+0x2B9100, NULL,	0xFF};
#if 0
#endif

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
	{ &img_screen_800_4_c46,"800_4_c46"}, //12
	{ &img_screen_800_4,    "800_4    "}, //13
	{ &img_screen_1024_4   ,"1024_4   "}, //14
	{ &img_screen_1280_4   ,"1280_4   "}, //15
	{ &img_screen_1366_4   ,"1366_4   "}, //16
	//===================================
	{ &img_alphabar, 	    "alphabar "}, //17
	//===================================
    // DEMO, search DEMO1024x600IMG_START.
	//===================================
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
//	{ &img_demo_16, 		"16_girl8 "},	 //33
	{ &img_customer, 		"customer "},	 //33

//	//===================================
//  // RLE
//	//===================================
	{ &img_rle80, 		    "rle80    "},	 //34
	{ &img_rle82, 		    "rle82    "},	 //35
	{ &img_rle83, 		    "rle83    "},	 //36
	{ &img_rle84, 		    "rle84    "},	 //37
	{ &img_rle85, 		    "rle85    "},	 //38
	{ &img_rle86, 		    "rle86    "},	 //39
	{ &img_rle87, 		    "rle87    "},	 //40
	{ &img_rle88, 		    "rle88    "},	 //41
	{ &img_rle89, 		    "rle89    "},	 //42
	{ &img_rle8A, 		    "rle8A    "},	 //43
	{ &img_rle8F, 		    "rle8F    "},	 //44

	{ &img_rle60, 		    "rle60    "},	 //45
	{ &img_rle62, 		    "rle62    "},	 //46
	{ &img_rle63, 		    "rle63    "},	 //47
	{ &img_rle64, 		    "rle64    "},	 //48
	{ &img_rle65, 		    "rle65    "},	 //49
	{ &img_rle66, 		    "rle66    "},	 //50
	{ &img_rle67, 		    "rle67    "},	 //51
	{ &img_rle68, 		    "rle68    "},	 //52
	{ &img_rle69, 		    "rle69    "},	 //53
	{ &img_rle6A, 		    "rle6A    "},	 //54
	{ &img_rle6F, 		    "rle6F    "},	 //55

	{ &img_rle40, 		    "rle40    "},	 //56
	{ &img_rle42, 		    "rle42    "},	 //57
	{ &img_rle43, 		    "rle43    "},	 //58
	{ &img_rle44, 		    "rle44    "},	 //59
	{ &img_rle45, 		    "rle45    "},	 //60
	{ &img_rle46, 		    "rle46    "},	 //61
	{ &img_rle47, 		    "rle47    "},	 //62
	{ &img_rle48, 		    "rle48    "},	 //63
	{ &img_rle49, 		    "rle49    "},	 //64
	{ &img_rle4A, 		    "rle4A    "},	 //65
	{ &img_rle4F, 		    "rle4F    "},	 //66
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
		Printf("\nFail req:%bd max:%bd", num, max);
		return NULL;
	}

	Printf("\nMonSOsd %bd %s",num,MonSOsdImgTable[num].name);
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

	for(i=0; i < (12+6+16+33); i++)
	{
		Printf("\n%02bd %s",i,MonSOsdImgTable[i].name);
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


	Printf("\nMonOsdLutLoad(%bd,%bd,%d)",img_n,lut);

	//BYTE i;
	image = MonSOsdImgTable[img_n].image;
	//prepare header
	MenuPrepareImageHeader(image);
	
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

	Printf("\nMonOsdImgLoad(%bd,%bd,%d)",img_n,sosd_win,item_lut);

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
#endif //..SPIOSD_USE_800X480_IMG