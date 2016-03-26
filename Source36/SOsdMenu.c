/**
 * @file
 * SOsdMenu.c 
 * @author Brian Kang
 * @version 1.1
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	SpiOsd Menu routines
 * TW8835 have 2KB XDATA, we have to move the static table datas to CODE segment.
 *	code segment size: 0x10000 64KB.
 *	xdata segment size: 0x0800 2KB.
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
/*
1.1 160210 Add curr_menu_watchdog_focus & curr_menu_watchdog_select.

*/

//*****************************************************************************
//
//									TW8835 MENU-E.c
//
//*****************************************************************************
//	   
//

#include "config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "Global.h"
#include "CPU.h"
#include "printf.h"
#include "util.h"

#include "I2C.h"
#include "spi.h"

#include "main.h"
#include "SOsd.h"
#include "FOsd.h"
#include "InputCtrl.h"

#include "SpiFlashMap.h"
#include "SOsdMenu.h"
#include "eeprom.h"
#include "ImageCtrl.h"
#include "Settings.h"
#include "Demo.h"

#include "Remo.h"
#include "TouchKey.h"

#include "measure.h"
#include "decoder.h"
#include "aRGB.h"
#include "dtv.h"
#include "PC_modes.h"
#include "scaler.h"
#include "debug.h"




#ifdef SUPPORT_SPIOSD
//=============================
// define
//=============================

#ifdef DEBUG_MENU
	#define MenuPrintf	Printf
	#define MenuPuts	Puts	
	#define eMenuPrintf	ePrintf
	#define eMenuPuts	ePuts	
	#define wMenuPrintf	wPrintf
	#define wMenuPuts	wPuts	
	#define dMenuPrintf	dPrintf
	#define dMenuPuts	dPuts	
#else
	#define MenuPrintf	nullFn
	#define MenuPuts	nullFn	
	#define eMenuPrintf	nullFn
	#define eMenuPuts	nullFn	
	#define wMenuPrintf	nullFn
	#define wMenuPuts	nullFn	
	#define dMenuPrintf	nullFn
	#define dMenuPuts	nullFn	
#endif


//if we are using MREL, The first 0x10 bytes are a image info.
//we need to skip 0x10 size to access the LUT & image.
#define MRLE_INFO_SIZE		0x10

//=============================
// external
//=============================

//=============================
//global vaiable
//=============================
//------------------------------------------
// 
//------------------------------------------
BYTE menu_on;
BYTE UseSOsdHwBuff;
BYTE osd_lang;

//------------------------------------------
// 
//------------------------------------------
menu_page_info_t *curr_menu;
BYTE curr_menu_watchdog_focus;
BYTE curr_menu_watchdog_select;
BYTE menu_level;
menu_page_info_t *menu_level_stack[4]; 		//max menu depth
BYTE menu_lock_scroll = 0;
BYTE touch_curr_menu_focus;
BYTE cancel_recover_value;

//------------------------------------------
// string buffer
//------------------------------------------
BYTE menu_str_buff[5];

//------------------------------------------
// touch
//------------------------------------------
int last_touch_x;	//for slide bar & position pox dialog.
int last_touch_y;

//------------------------------------------
// slider
//------------------------------------------
image_item_info_t *slide_title_img;
BYTE curr_slide_value;
BYTE last_slide3_value[3];
BYTE curr_slide3_value[3];
void (*SaveSlideValue)(BYTE val);
WORD (*GetSlideValue)(void);
WORD (*VarySlideValue)(BYTE val);
WORD (*SetSlideValue)(BYTE val);

void (*SaveSlide3Value)(BYTE index,BYTE val);
WORD (*GetSlide3Value)(BYTE index);
WORD (*VarySlide3Value)(BYTE index, BYTE val);
WORD (*SetSlide3Value)(BYTE index, BYTE val);

//------------------------------------------
//image header
//
// menu uses header_table.
// FW reads MREL header area to rle_header and convert it to header_table.
// or, FW converts image->info to header_table.
//------------------------------------------
menu_image_header_t header_table;
struct RLE2_HEADER rle_header;

//------------------------------------------
// For BANK
//------------------------------------------
BYTE MenuInputMain; 
BYTE MenuNoSignal;

//--------------------------
//local prototypes
//--------------------------
void MenuDrawSlideNeedleImageXY(BYTE index, BYTE value,	BYTE item_red);
void MenuDrawSlideCurrValue(BYTE index, short value);
void MenuUpdateSlideNeedleByPosition(int x /*, int y*/);

void proc_slider_left(void);
void proc_slider_right(void);
void proc_slider3_left(void);
void proc_slider3_right(void);
void proc_slider_digit(void);


//-------------------------------------
// MENU Draw Images
//-------------------------------------
#ifdef DEBUG_MENU
void PrintCurrMenu(void)
{
	dMenuPrintf("\n\rMENU ");
	if     (curr_menu == &menu_main_page) 			{ dMenuPrintf("main_page"); 				}
	else if(curr_menu == &menu_demo_page) 			{ dMenuPrintf("demo_page");					}
	else if(curr_menu == &menu_input_page)			{ dMenuPrintf("input_page");				}
	else if(curr_menu == &menu_system_page)			{ dMenuPrintf("system_page");				}
	else if(curr_menu == &menu_touch_page)			{ dMenuPrintf("touch_page");				}
	else if(curr_menu == &menu_yuv_list_page)		{ dMenuPrintf("yuv_list_page");				}
	else if(curr_menu == &menu_rgb_list_page)		{ dMenuPrintf("rgb_list_page");				}
	else if(curr_menu == &menu_apc_list_page)		{ dMenuPrintf("apc_list_page");				}
	else if(curr_menu == &menu_hdmi_list_page)		{ dMenuPrintf("hdmi_list_page");			}
	else if(curr_menu == &menu_display_page)		{ dMenuPrintf("display_page");				}
	else if(curr_menu == &menu_osd_page)			{ dMenuPrintf("osd_page");					}
//	else if(curr_menu == &menu_dvi_mode_page)		{ dMenuPrintf("dvi_mode_page");				}
	else if(curr_menu == &menu_hdmi_mode_page)		{ dMenuPrintf("hdmi_mode_page");			}
	else if(curr_menu == &menu_autoadj_page)		{ dMenuPrintf("autoadj_page");				}
	else if(curr_menu == &menu_autocolor_page)		{ dMenuPrintf("autocolor_page");			}
	else if(curr_menu == &menu_slider_page)			{ dMenuPrintf("slider_page");				}
	else if(curr_menu == &menu_slider3_page)		{ dMenuPrintf("slider3_page");				}
	else if(curr_menu == &menu_aspect_page)			{ dMenuPrintf("aspect_page");				}
	else if(curr_menu == &menu_position_page)		{ dMenuPrintf("position_page");				}
	else if(curr_menu == &menu_mirror_page)			{ dMenuPrintf("mirror_page");				}
	else if(curr_menu == &menu_restore_page)		{ dMenuPrintf("restore_page");				}
	else if(curr_menu == &menu_sys_info_page) 		{ dMenuPrintf("sys_info_page");				}
	else											{ dMenuPrintf("unknown page");				}
}
#endif


/**
* desc:convert info to header. 
*		header_table is a global.
*/
void info_to_header(image_info_t *info)
{
	menu_image_header_t *header;
	header = &header_table;

	//data from spiflash 
	header->lut_type = info->lut_type;
	header->bpp = info->rle >> 4;
	header->rle = info->rle & 0x0F;
	header->dx = info->dx;
	header->dy = info->dy;
	header->lut_size = 0x004 << header->bpp;
}
/**
* desc: convert rle2 to header
*		header_table is a global	
*/
void rle2_to_header(struct RLE2_HEADER *rle_header)
{
	menu_image_header_t *header;
	WORD colors;
	BYTE bpp;

	header = &header_table;

	header->lut_type = rle_header->LUT_format & 0x01;
	colors = (WORD)rle_header->LUT_colors+1; //word
	for(bpp=0; bpp < 9 ;) {
		if(colors & 0x01)
			break;
		colors >>= 1;
		bpp++;
	}
	header->bpp = bpp;
	header->rle = rle_header->rledata_cnt & 0x0F;
	header->dx = rle_header->w;
	header->dy = rle_header->h;
	header->lut_size = 0x004 << bpp;
}

#ifdef DEBUG_MENU
/**
* desc: print current item infomation
*/
void PrintCurrItem(BYTE item,menu_image_header_t *header)
{
	menu_sitem_info_t *menu_item;
	struct image_item_info_s *image;

	menu_item = &curr_menu->items[item];
	image = menu_item->image;

	dMenuPrintf("\n\r");
	dMenuPrintf(" item:%bd focus:%bd ",item, curr_menu->focus);
	dMenuPrintf(" %dx%d",menu_item->x, menu_item->y);
	dMenuPrintf(" align:%02bx",menu_item->align);
	dMenuPrintf(" win:%02bx",menu_item->win);
	dMenuPrintf(" lut:%x",menu_item->lut);
	
	dMenuPrintf("\n\r");
	dMenuPrintf(" image");
	dMenuPrintf(" type:%bx",image->type);
	dMenuPrintf(" loc:%06lx",image->loc);

	dMenuPrintf(" bpp%bd", header->bpp);
	dMenuPrintf(" rle%bd", header->rle);
	dMenuPrintf(" %dx%d", header->dx, header->dy);
	dMenuPrintf(" alpha:%2bx",image->alpha);
	dMenuPrintf(" lut%s size:%d*4",header->lut_type? "s": " ", header->lut_size >> 2);
}
#endif

//=========================
// stack
//=========================
//-------------------------------------
// MENU	LEVEL (Parent & Child)
//-------------------------------------
//----------------------------
/**
*desc: push/pop menu. 
*	use menu_level_stack[] to move the parent and child menu.
*/
void push_menu_stack_level(void)
{
	menu_level_stack[menu_level] = curr_menu; 
	menu_level++;
	menu_lock_scroll = 0;	//clear. If menu was a scroll, I want to scroll the menu.
}
void pop_menu_stack_level(void)
{
	menu_level--;
    curr_menu = menu_level_stack[menu_level];
	menu_lock_scroll = 0;	//clear
}
/**
* desc: get menu level.
*	To solve the Bank issue.
*/
BYTE MenuGetLevel(void)
{
	return menu_level+menu_on;
}

//=========================
// Header
//=========================

//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
BYTE MenuReadRleHeader(DWORD spi_loc,struct RLE2_HEADER *header)
{
	WORD wTemp;
	DWORD dTemp;
	BYTE i, *ptr;	

	dMenuPrintf("\n\rMenuReadRleHeader:%06lx:",spi_loc);
	WaitVBlank(1);

	SpiFlash_Read_XMem(spi_loc, (WORD)header, sizeof(struct RLE2_HEADER));
	//SpiFlashDmaRead2XMem((BYTE *)header, spi_loc, sizeof(struct RLE2_HEADER), SPIFLASH_WAIT_READ);
	
	ptr = (BYTE *)header;
	for(i=0; i < sizeof(struct RLE2_HEADER); i++) {
		dMenuPrintf("%02bx ",*ptr++);
	}
	if(header->id[0]!='I' || header->id[1]!='T') {
		//wMenuPuts(" MenuReadRleHeader fail");
		Printf("\n\rMenuReadRleHeader fail @%lx", spi_loc);
		return 1;	//fail
	}
	//swap
	wTemp = header->w;
	header->w = (wTemp >> 8 | wTemp << 8);
	wTemp = header->h;
	header->h = (wTemp >> 8 | wTemp << 8);

	dTemp=header->size;
	header->size = dTemp & 0x000000FF;
	header->size <<= 8; dTemp >>= 8;
	header->size |= dTemp & 0x000000FF;
	header->size <<= 8; dTemp >>= 8;
	header->size |= dTemp & 0x000000FF;
	header->size <<= 8; dTemp >>= 8;
	header->size |= dTemp & 0x000000FF;

//	dMenuPrintf("\n\rloc:%06lx", spi_loc);
//	dMenuPrintf(" op:%02bx",header->op_bmpbits); 
//	dMenuPrintf(" rle:%02bx",header->rledata_cnt);
//	dMenuPrintf(" wh:%dx%d",header->w, header->h);
//	dMenuPrintf(" size:%lx", header->size);
//	dMenuPrintf(" LUT format:%bx color:%bx", header->LUT_format,header->LUT_colors);
//	dMenuPrintf(" dummy %bx %bx",header->dummy0, header->dummy1);

	return 0;
}


//-----------------------------------------------------------------------------
/**
* desc: update header_table
* @param
* @return
*/
BYTE MenuPrepareImageHeader(struct image_item_info_s *image)
{
	BYTE ret;
	menu_image_header_t *header = &header_table;

	ret = 0;
	if(image->type==IMAGE_ITEM_TYPE_1) {
		//if we are using MREL, we need to skip 0x10 size.  
		ret=MenuReadRleHeader(image->loc,&rle_header);
		rle2_to_header(&rle_header);
		header->lut_loc = image->loc + MRLE_INFO_SIZE;
		header->image_loc = image->loc + header->lut_size + MRLE_INFO_SIZE;
	}
	else if(image->type==IMAGE_ITEM_TYPE_2){
		//if we are using MREL, we need to skip 0x10 size.  
		info_to_header(image->info);
		header->lut_loc = image->loc;
		header->image_loc = image->loc + header->lut_size;
		header->lut_loc += MRLE_INFO_SIZE;
		header->image_loc += MRLE_INFO_SIZE;
	}
	else {
		//old stype
		info_to_header(image->info);
		header->lut_loc = image->loc;
		header->image_loc = image->loc + header->lut_size;
	}
	return ret;
}

//=========================
// Init & TEST
//=========================
#define WIN_LOGO 1
BYTE NoSignalLogoShow;
//-----------------------------------------------------------------------------
/**
* desc: draw logo image
* @param
* @return
*/
void InitLogo1(void)
{
	struct image_item_info_s *image;
	menu_image_header_t *header = &header_table;
	WORD lut_loc;
	BYTE customer_logo;

	WORD sx = SOSD_X_START;
	WORD sy = SOSD_Y_START;

	eMenuPrintf("\n\rInitLog1");

	customer_logo = 1;
	WaitVBlank(1);
	SpiFlash_Read_XMem(img_customer.loc, (WORD)&rle_header, sizeof(struct RLE2_HEADER));
	//SpiFlashDmaRead2XMem((BYTE *)&rle_header, img_customer.loc, sizeof(struct RLE2_HEADER), SPIFLASH_WAIT_READ);
	if (rle_header.id[0]!='I' || rle_header.id[1]!='T')
	{
		Printf("\n\rCustomer Logo failed...Use IntersilLogo");
		customer_logo = 0;
	}

	if (customer_logo == 0)
	{ 
		//BK130417. Add check routine.
		//If fail, it can hangup system.
		WaitVBlank(1);
		SpiFlash_Read_XMem(img_logo.loc, (WORD)&rle_header, sizeof(struct RLE2_HEADER));
		//SpiFlashDmaRead2XMem((BYTE *)&rle_header, img_logo.loc, sizeof(struct RLE2_HEADER), SPIFLASH_WAIT_READ);
		if (rle_header.id[0]!='I' || rle_header.id[1]!='T')
		{
			Printf("\n\rInitLogo failed...Do not use SpiOSD!!!!");
			Printf(" %bx %bx",rle_header.id[0],rle_header.id[1]);
			return;
		}
	}
	
	NoSignalLogoShow = 1;

	//init DE
	SpiOsdSetDeValue();

	SOsd_CleanReg();
	SOsd_CleanLut();
	SOsd_CleanRlc();
	SOsd_UpdateReg(0, 8);
	SOsd_UpdateRlc();

	SpiOsdEnable(ON);

	if (customer_logo) 
		image = &img_customer;
	else
		image = &img_logo;
	
	lut_loc = 0x000;
	//lut_size = 0x004 << (image->rle >> 4);

	//prepare header. update header_table
	MenuPrepareImageHeader(image);

	SOsd_SetSpiStartOffset(WIN_LOGO, header->image_loc); 
	SOsd_SetImageWidthHeight( WIN_LOGO, header->dx, header->dy );
	SOsd_SetScreen( WIN_LOGO, sx, sy, header->dx, header->dy );
	if (WIN_LOGO == 0)
	{
		SOsd_SetWin0ImageOffsetXY( 0, 0 );
		SOsd_SetWin0Animation( 1, 0, 0, 0);
	}
	SOsd_SetPixelAlpha(WIN_LOGO, ON );
	SOsd_SetGlobalAlpha(WIN_LOGO, 0);
	SOsd_SetPixelWidth(WIN_LOGO, header->bpp);
	SOsd_SetLutOffset(WIN_LOGO, 0 /* menu_item->osd_s.lut */);

	SOsd_Enable(WIN_LOGO, ON);

	//write to HW
	if (header->rle)
		SOsd_SetRlc(WIN_LOGO, header->bpp, header->rle);

	//update HW ---same as SOsd_show()
	WaitVBlank(1);
	SOsd_UpdateReg(WIN_LOGO, WIN_LOGO);
	SOsd_UpdateRlc();

	//Load Palette
#if 0
	//SpiOsdLoadLUT(WIN_LOGO, header->lut_type, lut_loc, header->lut_size, header->lut_loc, image->alpha);
	SOsd_SetLut(WIN_LOGO, header->lut_type, lut_loc, header->lut_size, header->lut_loc, image->alpha);
	SOsd_UpdateLut(WIN_LOGO, 1);	
#else
	SOsd_SetLut(WIN_LOGO, header->lut_type, lut_loc, header->lut_size, header->lut_loc, image->alpha);
	SOsd_UpdateLut(WIN_LOGO, 0);
#endif
	// finish draw
}

//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void RemoveLogoWithWait(BYTE fPowerUpBoot)
{
	BYTE i;
	volatile BYTE human_input;

	eMenuPrintf("\n\rRemoveLogo2(%bx)",fPowerUpBoot);
	NoSignalLogoShow = 0;

	//wait & dimmer
	if(fPowerUpBoot) {
		for ( i=0; i<128; i++ ) {
			human_input = CheckHumanInputs(1);
			if (human_input && human_input!= HINPUT_TSC  ) break;
			delay1ms(10);
		}
		if ( i != 128 ) {
			wMenuPuts("\n\rLogo Stop by interrupt");
			goto END_END;
		}
	
		for ( i=0; i < 128; i++ ) {
			SOsd_SetGlobalAlpha(WIN_LOGO, i);
			WaitVBlank(1);	//it needs
			SOsd_UpdateReg(WIN_LOGO, WIN_LOGO);
	  		delay1ms(10);
			human_input = CheckHumanInputs(1);
			if (human_input && human_input!= HINPUT_TSC  ) 
				break;
		}
		if ( i != 128 ) {
			wMenuPuts("\n\rLogo Stop by interrupt");
		}
	}
END_END:	
	WaitVBlank(1);
	SOsd_Enable( WIN_LOGO, OFF ); //disable window.
	SpiOsdEnable(OFF);
}
void RemoveLogo(void)
{
	eMenuPrintf("\n\rRemoveLogo()");
	NoSignalLogoShow = 0;

	WaitVBlank(1);
	SOsd_Enable( WIN_LOGO, OFF ); //disable window.
	SpiOsdEnable(OFF);
}

//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
BYTE getNoSignalLogoStatus(void)
{
	return NoSignalLogoShow;
}


//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
/**
* description
*	init FontOSD Window.
*	I will use 3BPP for digit.
*/
void MenuFontOsdItemInit(BYTE item, BYTE f1BPP_ALPHA)
{
	BYTE winno;
	BYTE palette;
	menu_sitem_info_t *menu_item;

	menu_fitem_info_t *text_info;

	menu_item = &curr_menu->items[0];
	text_info = curr_menu->texts;

	text_info += item;

	winno = text_info->winno;
	palette = text_info->BPP3_palette;


	dMenuPrintf("\n\r");
	dMenuPrintf(" winno:%02bx",winno);
	dMenuPrintf(" palette:%02bx",palette);
	dMenuPrintf(" osdram:%d",text_info->osdram);

	//FOsdWinInit(winno);

	FOsdWinEnable(winno,OFF);	// WINn disable

#if 1
	//assuem page3
	//pre fix
	WriteTW88(REG304, ReadTW88(REG304) & 0xDF);		//default: lower 256 char.		
	WriteTW88(REG304, ReadTW88(REG304) & 0xFE);		//OsdRam access mode
	
	FOsdWinMulticolor(winno,1);
#endif

	
	FOsdWinScreenXY(winno,menu_item->x + text_info->x, menu_item->y + text_info->y);		
	FOsdWinScreenWH(winno, text_info->w, text_info->h);
 	FOsdWinZoom(winno, text_info->zoom_h, text_info->zoom_v);
	//3BPP
	FOsdWinSetBpp3Alpha(winno,palette);	//use BPP3_alpha_lut_offset[].
	FOsdSetPaletteColor(palette, text_info->BPP3_color, 8, 0);
	FOsdWinSetOsdRamStartAddr(winno, text_info->osdram);

#if 0
	//assume page3
	//post fix
	WriteTW88(REG304, ReadTW88(REG304) & 0xDF);		//default: lower 256 char.		
	WriteTW88(REG304, ReadTW88(REG304) & 0xFE);		//OsdRam access mode
	
	FOsdWinMulticolor(winno,1);
#endif

	if(f1BPP_ALPHA) {
		FOsdWinAlphaPixel(winno, text_info->BPP1Color >> 4, 15);
	}

	FOsdWinEnable(winno,ON);	//WINn enable

	//WriteTW88(REG310, ReadTW88(REG310) | 0x40 ); // temp - enable MultiBPP on Win1
}

#if 1
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void MenuFontOsdItemInitNew(BYTE item_start, BYTE item_end)
{
	BYTE winno;
	BYTE palette;
	menu_sitem_info_t *menu_item;
	menu_fitem_info_t *text_info;
	BYTE item;
	BYTE value;
	//BYTE high_byte,low_byte;

	menu_item = &curr_menu->items[0];

	for(item=item_start; item <= item_end; item++) {
		text_info = curr_menu->texts;
		text_info += item;	   //??working on Keil->Good

		winno = text_info->winno;
		palette = text_info->BPP3_palette;

	
		dMenuPrintf("\n\r");
		dMenuPrintf(" winno:%02bx",winno);
		dMenuPrintf(" palette:%02bx",palette);
		dMenuPrintf(" osdram:%d",text_info->osdram);

		FOsdWinEnable(winno,OFF);	// WINn disable

		value = ReadTW88(REG304);
		WriteTW88(REG304, value & 0xDE);	//lower 256 char, OsdRam access mode.

		FOsdWinMulticolor(winno,1);

		FOsdWinScreenXY(winno, menu_item->x + text_info->x,	menu_item->y + text_info->y);
		FOsdWinScreenWH(winno,	text_info->w, text_info->h);
	 	FOsdWinZoom(winno, text_info->zoom_h, text_info->zoom_v);

		//3BPP
		FOsdWinSetBpp3Alpha(winno,palette);	//use BPP3_alpha_lut_offset[].

		//high_byte = (BYTE)(text_info->BPP3_color >> 8);
		//low_byte = (BYTE)text_info->BPP3_color;
		//for(i=0; i < 8; i++) {
		//	//FOsdSetPaletteColor(palette+i, text_info->BPP3_color, 0);	//test color. Lime 		
		//	WriteTW88(REG30C, (ReadTW88(REG30C) & 0xC0) | (palette+i)); 
		//	WriteTW88(REG30D, high_byte);
		//	WriteTW88(REG30E, low_byte);
		//}
		FOsdSetPaletteColor(palette, text_info->BPP3_color, 8, 0);
		FOsdWinSetOsdRamStartAddr(winno, text_info->osdram);

		FOsdWinEnable(winno,ON);	//WINn enable
	}
}
#endif


//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
//please move to test_xxx.c
//-------------------------
// test routines
#if 0
void TestMainMenuImage(BYTE type)
{
	//see MenuDrawCurrImage
	WORD sx,sy;
	//menu_sitem_info_t *menu_item;
	struct image_item_info_s *image;
	menu_image_header_t *header = &header_table;
	BYTE sosd_win;
	//WORD lut_size;

	switch(type) {
	case 2:		image = &img_main_test2;	break;
	case 3:		image = &img_main_test3;	break;
	default:	image = &img_main_test1; 	break;
	}

	sx = 0;
	sy = 0;
	sosd_win = 1;
//	lut_size = 0x004 << (image->rle >> 4);
	//prepare header
	MenuPrepareImageHeader(image);

	SOsd_SetSpiStartOffset( sosd_win, header->image_loc);
	SOsd_SetImageWidthHeight( sosd_win, header->dx, header->dy );
	if(sosd_win==0)
		SOsd_SetWin0ImageOffsetXY( 0, 0 );
	SOsd_SetScreen( sosd_win, sx, sy, header->dx, header->dy );
	if(sosd_win==0)
		SOsd_SetWin0Animation( 1, 0, 0, 0);
	SOsd_SetPixelAlpha( sosd_win, ON );
	SOsd_SetPixelWidth(sosd_win, header->bpp);
	SOsd_SetLutOffset(sosd_win, 0 /*menu_item->osd_s.lut*/);


	SOsd_Enable( sosd_win, ON );

	//write to HW
	WaitVBlank(1);
	if(header->rle) {	//need RLE ?
		SOsd_SetRlc( sosd_win, header->bpp, header->rle);
		SOsd_UpdateRlc();
	}
	SOsd_UpdateReg(1,1 /*menu_item->osd_s.win, menu_item->osd_s.win*/);
	//SpiOsdLoadLUT(sosd_win, header->lut_type, 0 /*menu_item->osd_s.lut*/, header->lut_size, header->lut_loc,image->alpha);
	SOsd_SetLut(sosd_win, header->lut_type, 0 /*menu_item->osd_s.lut*/, header->lut_size, header->lut_loc,image->alpha);
	SOsd_UpdateLut(sosd_win, 1);	
}
#endif


//=========================
// DRAW
//=========================
#ifdef UNCALLED_SEGMENT_CODE
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void MenuDrawCurrValue(BYTE itemno, WORD value)
{
	BYTE temp_byte = itemno;
	BYTE temp_word = value;
}
void MenuDrawCurrString(BYTE itemno, BYTE *str)
{
	BYTE temp_byte = itemno;
	BYTE *temp_pbyte = str;
}
#endif

/**
* desc: draw the selected item. It can be background image, focused image,...
* use curr_menu.menu_info[item]
* tip: we need a osdlang.
* @param use1
*		0:Normal 1:focused Image.
* @param item
*		item number.
*
*/
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param use1
*	1:select second image, normally to indicate the pressed icon.
* @param item
*	item number on current menu page.
* @return
*/
void MenuDrawCurrImage(BYTE use1, BYTE item)
{
	WORD sx,sy;
	menu_sitem_info_t *menu_item;
	struct image_item_info_s *image;
	BYTE sosd_win;
	menu_image_header_t *header = &header_table;

	menu_item = &curr_menu->items[item];
	if(use1)	image = menu_item->image1;
	else		image = menu_item->image;
	if(image==NULL) {
		eMenuPrintf("\n\rimage%bd NULL @ item:%bd",use1,item); 
		return;
	}

	if(curr_menu == &menu_slider_page || curr_menu == &menu_slider3_page) {
		if(item == 1) {
			//slide title
			image = slide_title_img;
		}
	}
	//prepare header
	MenuPrepareImageHeader(image);

	//debug info
#ifdef DEBUG_MENU
	PrintCurrMenu();
	PrintCurrItem(item, header);
#endif

	sx = menu_item->x;
	sy = menu_item->y;
	sosd_win = menu_item->win;   

	//adjust sx,sy
	if(menu_item->align != ALIGN_TOPLEFT) {
		//please adjust new sx,sy
	}

	//fill out sosd_buff
	SOsd_SetSpiStartOffset( sosd_win, header->image_loc); 
	SOsd_SetImageWidthHeight( sosd_win, header->dx, header->dy );
	SOsd_SetScreen( sosd_win, sx, sy, header->dx, header->dy );
	if(sosd_win==0) {
		SOsd_SetWin0ImageOffsetXY( 0, 0 );
		SOsd_SetWin0Animation( 1, 0, 0, 0);
	}
	if(image->alpha != 0xFF) {
		SOsd_SetPixelAlpha( sosd_win, ON );
		SOsd_SetPixelAlphaIndex(sosd_win,image->alpha); //BK130325
	}
	else {
		SOsd_SetGlobalAlpha( sosd_win, 0 /*EE_Read(EEP_OSD_TRANSPARENCY)*/);
	}
	SOsd_SetPixelWidth(sosd_win, header->bpp);
	SOsd_SetLutOffset(sosd_win,menu_item->lut);

	SOsd_Enable( sosd_win, ON );

	if(UseSOsdHwBuff) {
		//
		//write to buffer
		//
		if(header->rle)
			SOsd_SetRlc(sosd_win,header->bpp,header->rle);
		SOsd_SetLut(sosd_win, header->lut_type, menu_item->lut, header->lut_size, header->lut_loc, image->alpha);
	
		//pixel alpha blending. after load Palette
		//if(image->alpha != 0xFF)
		//	//SOsdHwBuffSetAlpha(sosd_win, menu_item->lut+image->alpha);
		//	SOsd_SetPixelAlphaIndex(sosd_win,image->alpha);
	}
	else {
		//
		//write to HW
		//

		//WaitVBlank(1);
		if(header->rle) {	//need RLE ?
			SOsd_SetRlc( sosd_win, header->bpp,header->rle);
			SOsd_UpdateRlc();
		}	
		else {
			//We using RLE only on the background.
			if(item == 0) {
				SOsd_CleanRlc();
				SOsd_UpdateRlc();
			}
		}
		WaitVBlank(1);
	
		//Load Palette.
		//SpiOsdLoadLUT(sosd_win, header->lut_type, menu_item->lut, header->lut_size, header->lut_loc,image->alpha);
		SOsd_SetLut(sosd_win, header->lut_type, menu_item->lut, header->lut_size, header->lut_loc,image->alpha);
		SOsd_UpdateLut(sosd_win, 1);	
	
		//WaitVBlank(1);
		//update HW
		SOsd_UpdateReg(menu_item->win, menu_item->win);
	}
}

//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
#define SCROLL_SPEED_Y		2
//BK130103 Found BUG......
//#define SCROLL_SPEED_X		4
#define SCROLL_SPEED_X		15  //(16-1)
//FYI:I will call MenuDrawCurrImage(0,0) first. So We don't need to worry about RLE window.
void MenuDrawScrollImage(BYTE item, BYTE direction)
{
	WORD 	i;
	DWORD 	start_byte;
	BYTE	start_bit;
	menu_sitem_info_t *menu_item;
	struct image_item_info_s *image;
	BYTE sosd_win;

	WORD sx = SOSD_X_START;
	WORD sy = SOSD_Y_START;

	menu_image_header_t *header = &header_table;


	menu_item = &curr_menu->items[item];
	image = menu_item->image;

	//prepare header
	MenuPrepareImageHeader(image);

	//if it is a RLE, maybe this will not working....
	//I will draw it with Fixed image.
	if(header->rle)	{ 
		wMenuPuts("\n\rWarning:Scroll with RLE.\n"); 
		MenuDrawCurrImage(0,item);
		return;	
	}

	sosd_win = menu_item->win;

	//load LUT first
	WaitVBlank(1);
	//SpiOsdLoadLUT(sosd_win, header->lut_type, menu_item->lut, header->lut_size, header->lut_loc,image->alpha);	
	SOsd_SetLut(sosd_win, header->lut_type, menu_item->lut, header->lut_size, header->lut_loc,image->alpha);
	SOsd_UpdateLut(sosd_win, 1);	

	SOsd_Enable( sosd_win, ON );
	SOsd_SetPixelAlpha( sosd_win, ON );
	SOsd_SetPixelWidth(sosd_win, header->bpp);
	SOsd_SetLutOffset(sosd_win,menu_item->lut);
	if(sosd_win==0) {
		SOsd_SetWin0ImageOffsetXY( 0, 0 );
		SOsd_SetWin0Animation( 1, 0, 0, 0);
	}

	if(direction==NAVI_KEY_DOWN) {		
		for(i=1; i <= header->dy; i++) {
			start_byte = (header->dy - i) * header->dx;
			start_bit = 0;
			if(start_byte) {
				if(header->bpp == 6) {
					start_byte = start_byte * 6;
					start_bit = start_byte % 8;
					start_bit >>= 1;
					start_byte = start_byte / 8;	
				}
				else if(header->bpp == 4) {
					start_byte = start_byte * 4;
					start_bit = start_byte % 8;
					start_bit >>= 1;
					start_byte = start_byte / 8;	
				}
			}

			SOsd_SetSpiStartOffset(sosd_win, header->image_loc + start_byte);
			SOsd_SetSpiStartBit(sosd_win, start_bit);
			SOsd_SetScreen(sosd_win, sx,sy, header->dx, i);
			SOsd_SetImageWidthHeight(sosd_win, header->dx, i);	   //if not SOSD_WIN0, you can move up.
			//?delay
			WaitVBlank(1);
			SOsd_UpdateReg(menu_item->win, menu_item->win);
			//speed up
			if( (i+SCROLL_SPEED_Y) >= header->dy) {
				//stop it
				if(i !=  header->dy ) {
					i = header->dy - 1;
				}
				// next will be increased with 1.
			}
			else {
				i += SCROLL_SPEED_Y;	
				// next will be increased with 1.
			}
		}
	}
	else if(direction==NAVI_KEY_RIGHT)
	{
		//BK130114. clean RLC. onlu for test
		//SpiOsdRlcReg(0,0,0);
		//WriteTW88(REG420,0);	//disable 0
		//WriteTW88(REG450,0);	//disable 2
		//WriteTW88(REG460,0);	//disable 3
		//WriteTW88(REG470,0);	//disable 4
		//WriteTW88(REG480,0);	//disable 5
		//WriteTW88(REG490,0);	//disable 6
		//WriteTW88(REG4A0,0);	//disable 7
		//WriteTW88(REG4B0,0);	//disable 8


		SOsd_SetImageWidthHeight(sosd_win, header->dx, header->dy);

		for(i=0; i <= header->dx; i++) {
			start_byte = header->dx - i;
			start_bit = 0;
			if(start_byte) {
				if(header->bpp==6) {
					start_byte = start_byte * 6;
					start_bit = start_byte % 8;
					start_bit >>= 1;
					start_byte = start_byte / 8;
				}
				else if(header->bpp==4) {
					start_byte = start_byte * 4;
					start_bit = start_byte % 8;
					start_bit >>= 1;
					start_byte = start_byte / 8;	
				}
			}
			SOsd_SetSpiStartOffset(sosd_win, header->image_loc + start_byte);
			SOsd_SetSpiStartBit(sosd_win, start_bit);
			SOsd_SetScreen(sosd_win, sx,sy, i, header->dy);
			//delay
			WaitVBlank(1);
			SOsd_UpdateReg(menu_item->win, menu_item->win);

			//speed up
			if( (i+SCROLL_SPEED_X) >= header->dx) {
				//stop it
				if(i !=  header->dx ) {
					i = header->dx - 1;
				}
				// next will be increased with 1.
			}
			else {
				i += SCROLL_SPEED_X;	
				// next will be increased with 1.
			}
		}
	}
	else {
		//right to left and bottom to top is not implemented.
		//It is more easy then left2right, top2bottom.
		MenuDrawCurrImage(0,item);
	}
}
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
//desc: turn of curr_menu->focus
void MenuTurnOffCurrImage(BYTE item)
{
	menu_sitem_info_t *menu_item;
	BYTE sosd_win;

	menu_item = &curr_menu->items[item];
	sosd_win = menu_item->win;
	WaitVBlank(1);
	SOsd_Enable( sosd_win, OFF );
	SOsd_UpdateReg(sosd_win, sosd_win);
}

//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void MenuDrawDeactivatedItemImage(BYTE sosd_win, BYTE item)
{
BYTE temp =  sosd_win;
BYTE test = item;
	//fill out with half transparent window or use deactivated image.
}

#ifdef UNCALLED_SEGMENT_CODE
void FontOsdWinPuts2(struct menu_sitem_info_s *item_info, BYTE *str)
{...}
#endif



//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void MenuFontOsdItemString(BYTE index, BYTE *str)
{
	BYTE n_char;
	BYTE palette;

	menu_fitem_info_t *text_info = curr_menu->texts;
	text_info += index;

	//string use BPP1
	palette = text_info->BPP1Color;
	FOsdRamSetAddrAttr(text_info->osdram,text_info->BPP1Color);
	//assume page3
	n_char=0;
	while(*str) {
		WriteTW88(REG307, *str++);
		n_char++;
	}

	//adjust FontOSD width.
	FOsdWinScreenWH(text_info->winno, n_char,1);
}


//merged background
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void proc_menu_bg(void)
{
	BYTE curr_menu_type;
	curr_menu_type = curr_menu->type & 0x0F;

	CpuTouchSkipCount = 1*500*2;	//0.5 sec

	UseSOsdHwBuff = 1;
	SOsd_CleanLut();	
	SOsd_CleanRlc();
	SOsd_CleanReg();
//	SOsd_CleanLutOffset();

	MenuDrawCurrImage(0,0);									//draw background

	if(curr_menu_type==MENU_TYPE_LIST 
	|| curr_menu_type==MENU_TYPE_SCRLDN) {
		//update HW and clear UseSOsdHwBuff mode.
		SOsd_show(1);
		UseSOsdHwBuff = 0;

		if(menu_lock_scroll==0) {
			if(curr_menu_type == MENU_TYPE_LIST)
				MenuDrawScrollImage(1, NAVI_KEY_RIGHT);		//left side
			else if(curr_menu_type == MENU_TYPE_SCRLDN) {
				MenuDrawScrollImage(1, NAVI_KEY_DOWN);		//top topdown moving
			}
			else {
				dMenuPrintf("\n\rinvalid curr_menu->type:%bx",curr_menu->type);
				MenuDrawCurrImage(0,1);
			}
			menu_lock_scroll=1;
		}
		else
			MenuDrawCurrImage(0,1);
	}

	MenuDrawCurrImage(0,curr_menu->focus);			//draw focused item
	if(curr_menu->type & MENU_TYPE_NOTE ) 			//draw selected note symbol(bar or point)
		MenuDrawCurrImage(0,curr_menu->select+curr_menu->item_total);	//select

	//if(UseSOsdHwBuff) 
	{
		SOsd_show(UseSOsdHwBuff);
		UseSOsdHwBuff = 0;
	}
}


//=========================
// KEY & TOUCH
//=========================
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
BYTE MenuKeyInput_slider_page(BYTE key)
{
	switch(key) {
	case NAVI_KEY_ENTER:
		break;
	case NAVI_KEY_UP:
		curr_menu->focus = curr_menu->items[curr_menu->focus].up;
        curr_menu_watchdog_focus = curr_menu->focus;
		curr_menu->items[0].p(); //redraw all proc_slider_bg()
		key = NAVI_KEY_NONE; //eat key
		break;
	case NAVI_KEY_DOWN:
		curr_menu->focus = curr_menu->items[curr_menu->focus].down;
        curr_menu_watchdog_focus = curr_menu->focus;
		curr_menu->items[0].p();  //redraw all	proc_slider_bg()
		key = NAVI_KEY_NONE; //eat key
		break;
	case NAVI_KEY_LEFT:
		if(curr_menu->focus == MENU_SLIDER_BAR_INPUT_RED) {
			proc_slider_left();
			key = NAVI_KEY_NONE; //eat key
		}
		break;
	case NAVI_KEY_RIGHT:
		if(curr_menu->focus == MENU_SLIDER_BAR_INPUT_RED) {
			proc_slider_right();
			key = NAVI_KEY_NONE; //eat key
		}
		break;
	default:
		// do nothing.
		break;
	}
	return key;
}

//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
BYTE MenuKeyInput_slider3_page(BYTE key)
{
	switch(key) {
	case NAVI_KEY_ENTER:
		break;
	case NAVI_KEY_UP:
		curr_menu->focus = curr_menu->items[curr_menu->focus].up;
        curr_menu_watchdog_focus = curr_menu->focus;
		if(curr_menu->focus >= curr_menu->item_start) {
			curr_menu->select = curr_menu->focus;
            curr_menu_watchdog_select = curr_menu->select;
        }
		curr_menu->items[0].p(); //redraw all proc_slider3_bg()
		key = NAVI_KEY_NONE; //eat key
		break;
	case NAVI_KEY_DOWN:
		curr_menu->focus = curr_menu->items[curr_menu->focus].down;
        curr_menu_watchdog_focus = curr_menu->focus;
		if(curr_menu->focus >= curr_menu->item_start) {
			curr_menu->select = curr_menu->focus;
            curr_menu_watchdog_select = curr_menu->select;
        }
		curr_menu->items[0].p();  //redraw all	proc_slider3_bg()
		key = NAVI_KEY_NONE; //eat key
		break;
	case NAVI_KEY_LEFT:
	 	if(curr_menu->focus == SLIDE3_1 || curr_menu->focus == SLIDE3_2 || curr_menu->focus == SLIDE3_3) {
			proc_slider3_left();
			key = NAVI_KEY_NONE; //eat key
		}
		break;
	case NAVI_KEY_RIGHT:
	 	if(curr_menu->focus == SLIDE3_1 || curr_menu->focus == SLIDE3_2 || curr_menu->focus == SLIDE3_3) {
			proc_slider3_right();
			key = NAVI_KEY_NONE; //eat key
		}
		break;
	default:
		// do nothing.
		break;
	}
	return key;
}

//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
BYTE MenuKeyInput_main_page(BYTE key)
{
	BYTE new;

	switch(key) {
	case NAVI_KEY_ENTER:
		break;
	case NAVI_KEY_UP:
		if(curr_menu->focus < curr_menu->item_start) {
			//NAVI to item
			MenuTurnOffCurrImage(curr_menu->focus);
			curr_menu->focus = curr_menu->select;
            curr_menu_watchdog_focus = curr_menu->focus;
			MenuDrawCurrImage(0,curr_menu->focus);
			key = NAVI_KEY_NONE; //eat key			
		}
		else {
			//item to NAVI or item to item
			new = curr_menu->items[curr_menu->focus].up;
			if(new >=  curr_menu->item_start) {
				curr_menu->select = new;	
                curr_menu_watchdog_select = curr_menu->select;
            }
		}
		break;

	case NAVI_KEY_DOWN:
		if(curr_menu->focus < curr_menu->item_start) {
			//NAVI to item
			MenuTurnOffCurrImage(curr_menu->focus);
			curr_menu->focus = curr_menu->select;
            curr_menu_watchdog_focus = curr_menu->focus;
			MenuDrawCurrImage(0,curr_menu->focus);
			key = NAVI_KEY_NONE; //eat key			
		}
		else {
			//item to NAVI
			new = curr_menu->items[curr_menu->focus].down;
			if(new >=  curr_menu->item_start) {
				curr_menu->select = new;
                curr_menu_watchdog_select = curr_menu->select;
            }	
		}
		break;

	case NAVI_KEY_LEFT:
		if(curr_menu->focus >= curr_menu->item_start) {
			curr_menu->select = curr_menu->items[curr_menu->focus].left;
            curr_menu_watchdog_select = curr_menu->select;
		}
		break;
	case NAVI_KEY_RIGHT:
		if(curr_menu->focus >= curr_menu->item_start) {
			curr_menu->select = curr_menu->items[curr_menu->focus].right;
            curr_menu_watchdog_select = curr_menu->select;
		}
		break;
	default:
		// do nothing.
		break;
	}
	return key;
}


//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
BYTE MenuKeyInput_input_page(BYTE key)
{
	switch(key) {
	case NAVI_KEY_ENTER:
		break;
	case NAVI_KEY_UP:
		MenuTurnOffCurrImage(curr_menu->focus);
		if(curr_menu->focus < curr_menu->item_start) {
			//NAVI to item
			curr_menu->focus = curr_menu->select;
            curr_menu_watchdog_focus = curr_menu->focus;			
		}
		else {
			//item to NAVI
			curr_menu->focus = curr_menu->items[curr_menu->focus].up;
            curr_menu_watchdog_focus = curr_menu->focus;
			if(curr_menu->type & MENU_TYPE_NOTE )
				MenuDrawCurrImage(0,curr_menu->select+curr_menu->item_total);
		}
		MenuDrawCurrImage(0,curr_menu->focus);
		key = NAVI_KEY_NONE; //eat key
		break;

	case NAVI_KEY_DOWN:
		MenuTurnOffCurrImage(curr_menu->focus);
		if(curr_menu->focus < curr_menu->item_start) {
			//NAVI to item
			curr_menu->focus = curr_menu->select;
            curr_menu_watchdog_focus = curr_menu->focus;			
		}
		else {
			//item to NAVI
			curr_menu->focus = curr_menu->items[curr_menu->focus].down;
            curr_menu_watchdog_focus = curr_menu->focus;
			if(curr_menu->type & MENU_TYPE_NOTE )
				MenuDrawCurrImage(0,curr_menu->select+curr_menu->item_total);	//select
		}
		MenuDrawCurrImage(0,curr_menu->focus);
		key = NAVI_KEY_NONE; //eat key
		break;

	case NAVI_KEY_LEFT:
		break;
	case NAVI_KEY_RIGHT:
		break;
	default:
		// do nothing.
		break;
	}
	return key;
}

//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
BYTE MenuKeyInput_position_page(BYTE key)
{
#ifdef SUPPORT_PC
	BYTE digit;
#endif

	switch(key) {
	case NAVI_KEY_ENTER:
		break;
	case NAVI_KEY_UP:
#ifdef SUPPORT_PC
		if(curr_menu->focus == MENU_POSITION_INPUT_RED) {
			//adjust active area box
			digit = GetRGBPosV();
			if(digit<100)
				SetRGBPosV(++digit);
			proc_position_V_digit();
			key = NAVI_KEY_NONE; //eat key
		}
#endif
		break;
	case NAVI_KEY_DOWN:
#ifdef SUPPORT_PC
		if(curr_menu->focus == MENU_POSITION_INPUT_RED) {
			//adjust active area box
			digit = GetRGBPosV();
			if(digit)
				SetRGBPosV( --digit);
			proc_position_V_digit();
			key = NAVI_KEY_NONE; //eat key
		}
#endif
		break;
	case NAVI_KEY_LEFT:
#ifdef SUPPORT_PC
		if(curr_menu->focus == MENU_POSITION_INPUT_RED) {
			//adjust active area box
			digit = GetRGBPosH();
			if(digit)
				SetRGBPosH(--digit);
			proc_position_H_digit();
			key = NAVI_KEY_NONE; //eat key
		}
#endif
		break;
	case NAVI_KEY_RIGHT:
#ifdef SUPPORT_PC
		if(curr_menu->focus == MENU_POSITION_INPUT_RED) {
			//adjust active area box
			digit = GetRGBPosH();
			if(digit<100)
				SetRGBPosH(++digit);
			proc_position_H_digit();
			key = NAVI_KEY_NONE; //eat key
		}
#endif
		break;
	default:
		// do nothing.
		break;
	}
	return key;
}


//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void MenuKeyInput(BYTE key)
{
#ifdef DEBUG_MENU
	switch(key) {
	case NAVI_KEY_ENTER: 	dMenuPrintf("\n\rNAVI_KEY_ENTER");	break;
	case NAVI_KEY_UP: 		dMenuPrintf("\n\rNAVI_KEY_UP");		break;
	case NAVI_KEY_DOWN: 	dMenuPrintf("\n\rNAVI_KEY_DOWN");		break;
	case NAVI_KEY_LEFT: 	dMenuPrintf("\n\rNAVI_KEY_LEFT");		break;
	case NAVI_KEY_RIGHT: 	dMenuPrintf("\n\rNAVI_KEY_RIGHT");	break;
	default: 				dMenuPrintf("\n\rNAVI_KEY_knonwn:%02bx",key);	break;
	}
    dPrintCurrMenu();
    dPrintf(" curr_menu type:%bd focus:%bd select:%bd item_start:%bd item_total:%bd", 
        curr_menu->type,
        curr_menu->focus, 
        curr_menu->select,
        curr_menu->item_start,
        curr_menu->item_total);
#endif

	//
	//special
	// eat the key or return the same key.
	if(curr_menu==&menu_slider_page)
		key = MenuKeyInput_slider_page(key);
	else if(curr_menu==&menu_slider3_page)
		key =MenuKeyInput_slider3_page(key);
	else if(curr_menu==&menu_position_page)
		key = MenuKeyInput_position_page(key);
	else if(curr_menu==&menu_input_page || curr_menu==&menu_system_page)
		key = MenuKeyInput_input_page(key);
	else if(curr_menu==&menu_main_page) {
		key = MenuKeyInput_main_page(key);
	}


	switch(key) {
	case NAVI_KEY_ENTER:
		curr_menu->items[curr_menu->focus].p();
		//NOTE:DO NOT USE curr_menu, it will be updated.
		break;
	case NAVI_KEY_UP:
		MenuTurnOffCurrImage(curr_menu->focus);
		curr_menu->focus = curr_menu->items[curr_menu->focus].up;
        curr_menu_watchdog_focus = curr_menu->focus;
		MenuDrawCurrImage(0,curr_menu->focus);
		break;
	case NAVI_KEY_DOWN:
		MenuTurnOffCurrImage(curr_menu->focus);
		curr_menu->focus = curr_menu->items[curr_menu->focus].down;
        curr_menu_watchdog_focus = curr_menu->focus;
		MenuDrawCurrImage(0,curr_menu->focus);
		break;
	case NAVI_KEY_LEFT:
		MenuTurnOffCurrImage(curr_menu->focus);
		curr_menu->focus = curr_menu->items[curr_menu->focus].left;
        curr_menu_watchdog_focus = curr_menu->focus;
		MenuDrawCurrImage(0,curr_menu->focus);
		break;
	case NAVI_KEY_RIGHT:
		//dMenuPrintf("\n\rfocus:%d->",(WORD)curr_menu->focus);
		MenuTurnOffCurrImage(curr_menu->focus);
		curr_menu->focus = curr_menu->items[curr_menu->focus].right;
        curr_menu_watchdog_focus = curr_menu->focus;
		//dMenuPrintf("%d",(WORD)curr_menu->focus);
		MenuDrawCurrImage(0,curr_menu->focus);
		break;
	default:
		// do nothing.
		break;
	}

	//if deactivated item, passthru.
	//BKTODO: Do you have Only one deactivated item ?
	//NOTE: curr_menu.item can not be 0 on key input mode.
	//if(redraw) {
	//	//curr_menu->items[0].p();
	//	MenuDrawCurrImage(0,curr_menu->focus);
	//}
}
/**
* description
*	check input and return the selected item number
* return:
*	0: failed
*	other: selected item number
*/
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
#ifdef SUPPORT_TOUCH
BYTE MenuCheckItemPosition(int x, int y)
{
	struct image_item_info_s *image;
	menu_image_header_t *header = &header_table;
	BYTE i;
	BYTE max;

	dMenuPrintf("\n\rMenuCheckItemPosition(%d,%d)",x,y);

	max = curr_menu->item_start + curr_menu->item_total;
	if(curr_menu->type & MENU_TYPE_NOTE)
		max += curr_menu->item_total;

	//dMenuPrintf("\n\r??item_start:%bd item_total:%bd",curr_menu->item_start,curr_menu->item_total  );
	for(i=0; i < max; i++) {
		if(curr_menu->items[i].osd_type == TYPE_FOSD) continue;		//skip for FOSD
		if(curr_menu->items[i].osd_type == TYPE_BG) continue;		//skip for Background
		if(curr_menu->items[i].osd_type == TYPE_MBG) continue;		//skip for Moving SOSD Background
		if(curr_menu->items[i].osd_type == TYPE_SLIDE) {
			if( x >= (curr_menu->items[0].x + curr_menu->items[i].x) 
			&&  x <= (curr_menu->items[0].x + curr_menu->items[i+1].x)
			&&  y >= (curr_menu->items[0].y + curr_menu->items[i].y) 
			&&  y <= (curr_menu->items[0].y + curr_menu->items[i+1].y)) {
				return i+1;
			}
			i++;
		 	continue;		//need special
		}
		//
		//normal item
		//
		//dMenuPrintf("\n\r**%bx %dx%d", i,curr_menu->items[i].x,curr_menu->items[i].y );
		if(curr_menu->items[i].x <= x && curr_menu->items[i].y <= y) {
			image = curr_menu->items[i].image;
			if(image==NULL)
				image = curr_menu->items[i].image1;
			if(image==NULL) {
				eMenuPrintf("\n\rNULL image @%bd",i);
				continue;
			}

			//MenuPrintf("\n\r%bd:",i);
			MenuPrepareImageHeader(image);

			//dMenuPrintf(" %dx%d", header->dx, header->dy );
			if( (curr_menu->items[i].x+header->dx) > x && (curr_menu->items[i].y + header->dy) > y) {
				//found selected item.
				//dMenuPrintf("\n\r%bd ",i,curr_menu->items[i].x,curr_menu->items[i].y, 
				return i;
			}
		}
	}	
	return 0xFF;	//no. failed.
}
#endif


#ifdef SUPPORT_TOUCH
/**
* description
*	call MenuCheckItemPosition and executed the selected item.
*/
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void MenuCheckTouchInput(BYTE TscStatus, int x,int y)
{
	BYTE ret;
	BYTE index;

	ret =  MenuCheckItemPosition(x,y);

	if(ret == 0xFF) {
		//if we lost a focus.....do something.
		//TscStatus == TOUCHCLICK || TscStatus == TOUCHDOUBLECLICK

		//curr_menu->focus = touch_curr_menu_focus;
        //curr_menu_watchdog_focus = curr_menu->focus;
		if(MenuIsSlideMode()) {
			if(curr_menu==&menu_slider_page) {
				if(curr_menu->focus==MENU_SLIDER_LEFT 
				|| curr_menu->focus==MENU_SLIDER_RIGHT
				|| curr_menu->focus==MENU_SLIDER_BAR_OK
				|| curr_menu->focus==MENU_SLIDER_BAR_CANCEL
				) {
					MenuTurnOffCurrImage(curr_menu->focus);
					curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
                    curr_menu_watchdog_focus = curr_menu->focus;
					MenuDrawSlideNeedleImageXY(0, curr_slide_value, 1/*MENU_SLIDER_BAR_INPUT_RED*/);
				}
				return;
			}
			else if(curr_menu==&menu_slider3_page) {
				if(curr_menu->focus==SLIDE3_LEFT 
				|| curr_menu->focus==SLIDE3_RIGHT
				|| curr_menu->focus==SLIDE3_OK
				|| curr_menu->focus==SLIDE3_CANCEL
				) {
					MenuTurnOffCurrImage(curr_menu->focus);
					curr_menu->focus = curr_menu->select;
                    curr_menu_watchdog_focus = curr_menu->focus;

					if(curr_menu->focus == SLIDE3_1)		index = 0;
					else if(curr_menu->focus == SLIDE3_2)	index = 1;
					else if(curr_menu->focus == SLIDE3_3)	index = 2;
					else									index = 0;

					MenuDrawSlideNeedleImageXY(index, curr_slide3_value[index], 1/*MENU_SLIDER_BAR_INPUT_RED*/);
				}
				return;
			}
		}
		MenuDrawCurrImage(0, curr_menu->focus);

		return;
	}
	//dMenuPrintf("\n\r****MenuCheckItemPosition ret %bx", ret);

	if(TscStatus==TOUCHPRESS || TscStatus >= TOUCHMOVE) {
		//just draw the focused item, do not make an action.
		//if it is a left/right on slide, I need a long press
		touch_curr_menu_focus = curr_menu->focus;

		curr_menu->focus = ret;	//only focus, not select
        curr_menu_watchdog_focus = curr_menu->focus;

		if(MenuIsSlideMode()) {
			if(curr_menu==&menu_slider_page) {
				//only for slide bar
				if(ret == MENU_SLIDER_BAR_INPUT_GRAY || ret == MENU_SLIDER_BAR_INPUT_RED) {
					curr_menu->select = curr_menu->focus = ret;
                    curr_menu_watchdog_focus = curr_menu->focus;
                    curr_menu_watchdog_select = curr_menu->select;	 
					MenuUpdateSlideNeedleByPosition(x/*,y*/);
					MenuDrawSlideNeedleImageXY(0,curr_slide_value, 1/*MENU_SLIDER_BAR_INPUT_RED*/);
					MenuDrawSlideCurrValue(0, (short)curr_slide_value);
					//dMenuPrintf("\n\r MenuCheckTouchInput end4");
					return;
				}
				else {
					MenuDrawCurrImage(1, curr_menu->focus);
					//dMenuPrintf("\n\r MenuCheckTouchInput end5");
					return; 
				}
			}
			else if(curr_menu==&menu_slider3_page) {
				if(ret >= curr_menu->item_start && ret < (curr_menu->item_start+3*2)) {
					curr_menu->select = curr_menu->focus = ret;
                    curr_menu_watchdog_focus = curr_menu->focus;
                    curr_menu_watchdog_select = curr_menu->select;	 
					MenuUpdateSlideNeedleByPosition(x/*,y*/);
					curr_menu->items[0].p();
					//dMenuPrintf("\n\r MenuCheckTouchInput end4");
					return;
				}
				else {
					MenuDrawCurrImage(1, curr_menu->focus);
				}
			}
			return;
		}

		MenuDrawCurrImage(1, curr_menu->focus);
		//dMenuPrintf("\n\r MenuCheckTouchInput end1");
		return;
	}
	if(TscStatus==TOUCHCLICK || TscStatus==TOUCHDOUBLECLICK || TscStatus==TOUCHLONGCLICK) {
		//make an action.

		if(MenuIsSlideMode()) {
			if(curr_menu==&menu_slider_page) {
				if(ret == MENU_SLIDER_BAR_INPUT_GRAY || ret == MENU_SLIDER_BAR_INPUT_RED) {
					curr_menu->select = curr_menu->focus = ret;
                    curr_menu_watchdog_focus = curr_menu->focus;
                    curr_menu_watchdog_select = curr_menu->select;	 
					MenuUpdateSlideNeedleByPosition(x/*,y*/);
					MenuDrawSlideNeedleImageXY(0,curr_slide_value, 1/*MENU_SLIDER_BAR_INPUT_RED*/);
					MenuDrawSlideCurrValue(0, (short)curr_slide_value);
					//dMenuPrintf("\n\r MenuCheckTouchInput end3");
					return;
				}
			}
			else if(curr_menu==&menu_slider3_page) {
				if(ret >= curr_menu->item_start && ret < (curr_menu->item_start+3*2)) {
					curr_menu->select = curr_menu->focus = ret;	
                    curr_menu_watchdog_focus = curr_menu->focus;
                    curr_menu_watchdog_select = curr_menu->select; 
					MenuUpdateSlideNeedleByPosition(x/*,y*/);
					curr_menu->items[0].p();
					//dMenuPrintf("\n\r MenuCheckTouchInput end3");
					return;
				}
			}
		}

		//touch_curr_menu_focus = curr_menu->focus;
		curr_menu->focus = ret;	 //only focus, not select. action will decide select.
		curr_menu_watchdog_focus = curr_menu->focus;
        //MenuDrawCurrImage(1, curr_menu->focus);
		curr_menu->items[ret].p();
		//SetTouchStatus(0);  //BK110601
		dMenuPrintf("\n\r MenuCheckTouchInput end2");

		return;
	}
	eMenuPrintf("\n\rMenuCheckTouchInput TscStatus:%bx fail",TscStatus);
	//if(ret) 
	{
		last_touch_x = x;	//save last position for ProgressBar & Radio Tuner
		last_touch_y = y;
		//execute
		touch_curr_menu_focus = curr_menu->focus;
		curr_menu->focus = ret;
        curr_menu_watchdog_focus = curr_menu->focus;
		curr_menu->items[ret].p();

		//SetTouchStatus(0);  //BK110601
		//dMenuPrintf("\n\r MenuCheckTouchInput end3");
	}
}
#endif


//=========================
// NAVI ICON
//=========================
//------------------------
// common icons


//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
//move main_page->demo_page
void proc_demo_icon(void)
{
	push_menu_stack_level();
	curr_menu = &menu_demo_page;
	curr_menu->focus = curr_menu->item_start;
    curr_menu_watchdog_focus = curr_menu->focus;
	curr_menu->items[0].p();
}
void proc_exit_icon(void)
{
	//disable all enabled windows
	//call MenuEnd routine
	MenuEnd();
}
void proc_return_icon(void)
{

	FOsdWinOffAll();

	pop_menu_stack_level();
	curr_menu->items[0].p();  //redraw parent
}

void proc_home_icon(void)
{
	//call main menu
	menu_level=1;
	proc_return_icon();
}

//=========================
// SLIDE DIALOG
//=========================
//-------------------------------------
// slide popup MENU
//-------------------------------------

//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void InitSlideCtrl(image_item_info_t *title_img, void *pGet, void *pVary, void *pSet, void *pSave)
{
	UseSOsdHwBuff = 0;
	SOsd_CleanReg();
	//SOsdHw_Clean();
	SOsd_UpdateReg(0,8);
	SOsd_UpdateRlc();  //??BK130319 Do you need SOsd_CleanRlc()??

	slide_title_img = title_img;
	GetSlideValue = pGet;
	VarySlideValue = pVary;
	SetSlideValue = pSet;
	SaveSlideValue = pSave;

	last_slide3_value[0] = curr_slide_value = GetSlideValue();
}
void InitSlide3Ctrl(image_item_info_t *title_img, void *pGet, void *pVary, void *pSet, void *pSave)
{
	UseSOsdHwBuff = 0;
	SOsd_CleanReg();
	//SOsdHw_Clean();
	SOsd_UpdateReg(0,8);
	SOsd_UpdateRlc();  //??BK130319 Do you need SOsd_CleanRlc()??


	slide_title_img = title_img;
	GetSlide3Value = pGet;
	VarySlide3Value = pVary;
	SetSlide3Value = pSet;
	SaveSlide3Value = pSave;

	last_slide3_value[0] = curr_slide3_value[0] = GetSlide3Value(0);
	last_slide3_value[1] = curr_slide3_value[1] = GetSlide3Value(1);
	last_slide3_value[2] = curr_slide3_value[2] = GetSlide3Value(2);
}

//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
//I need other menu info. I will just update X,Y
//@param:
//	value: 0 ~ 100
//	item:  0:gray 1:focused		//2:gray, 3:red	
//item 0: background image
//item 1: title
//item 2: gray
//item 3: red(focused)
//..
void MenuDrawSlideNeedleImageXY(BYTE index, BYTE value,	BYTE item)	
{
	WORD sx,sy;
	menu_sitem_info_t *menu_item;
	struct image_item_info_s *image;
	menu_image_header_t *header = &header_table;
	BYTE sosd_win;
//	WORD lut_size;
	BYTE rate = 100;
	WORD position;
	WORD barsize;

	dMenuPrintf("\n\rMenuDrawSlideNeedleImageXY(index:%d value:%d, %s)",(WORD)index, (WORD)value, item ? "RED":"GRAY");

	//get window start
	menu_item = &curr_menu->items[0];
	//image = menu_item->osd_s.image;
	sx = menu_item->x;
	sy = menu_item->y;

	//get bar end position
	menu_item = &curr_menu->items[curr_menu->item_start+1+index*2];
	//image = menu_item->osd_s.image;
	barsize = menu_item->x;
	
	//get niddle start
	menu_item = &curr_menu->items[curr_menu->item_start+index*2];
	//image = menu_item->osd_s.image;
	sx += menu_item->x;
	sy += menu_item->y;

	//calculate barsize
	barsize -= menu_item->x;

	//get bar position
	position = (WORD)(((DWORD)value * 100 / rate)*  barsize / 100);
	//update sx
	sx += position;
	
	//which one, gray of red(focused)
	menu_item = &curr_menu->items[curr_menu->item_start+index*2+item];
	image = menu_item->image;


	sosd_win = menu_item->win;
	//4bpp:0x040, 6bpp:0x100, 8bpp:0x400
//	lut_size = 0x004 << (image->rle >> 4);

	//adjust sx,sy
	if(menu_item->align == ALIGN_TOPMIDDLE) {
		sx -= SLIDER_NIDDLE_HALF_WIDTH;
	}

	//prepare header
	MenuPrepareImageHeader(image);

	//fill out sosd_buff
	//if we are using MREL, we need to skip 0x10 size.
	SOsd_SetSpiStartOffset( sosd_win, header->image_loc); 
	SOsd_SetImageWidthHeight( sosd_win, header->dx, header->dy );
	SOsd_SetScreen( sosd_win, sx, sy, header->dx, header->dy );
	if(sosd_win==0) {
		SOsd_SetWin0ImageOffsetXY( 0, 0 );
		SOsd_SetWin0Animation( 1, 0, 0, 0);
	}
//	SOsd_SetPixelAlpha( sosd_win, ON );
	if(image->alpha != 0xFF)
		SOsd_SetPixelAlpha( sosd_win, ON );
	else {
		if(slide_title_img == &img_slide_trans) {
			SOsd_SetGlobalAlpha( sosd_win, OsdGetTrans());
			//only for test....
			SOsd_SetGlobalAlpha( 1, OsdGetTrans());	//bg
			SOsd_SetGlobalAlpha( 2, OsdGetTrans());	//title
			WaitVBlank(1);	//BK130104 add
			SOsd_UpdateReg(1,2);
		}
		else
			SOsd_SetGlobalAlpha( sosd_win, 0 /*EE_Read(EEP_OSD_TRANSPARENCY)*/);
	}
	SOsd_SetPixelWidth(sosd_win, header->bpp);
	SOsd_SetLutOffset(sosd_win,menu_item->lut);

	SOsd_Enable( sosd_win, ON );

	//write to HW
	WaitVBlank(1);

	if(header->rle) {	//need RLE ?
		SOsd_SetRlc( sosd_win, header->bpp, header->rle);
		SOsd_UpdateRlc();
	}	
	else {
		//BK110203
		//We using RLE only on the background.
//remove 130227
//		if(item == 0)
//			SpiOsdDisableRlcReg(0);
	}
	//update HW
	SOsd_UpdateReg(menu_item->win, menu_item->win);
	//Load Palette
	//SpiOsdLoadLUT(sosd_win, header->lut_type, menu_item->lut, header->lut_size, header->lut_loc,image->alpha);
	SOsd_SetLut(sosd_win, header->lut_type, menu_item->lut, header->lut_size, header->lut_loc,image->alpha);
	SOsd_UpdateLut(sosd_win, 1);	
}

#ifdef SUPPORT_TOUCH
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void MenuUpdateSlideNeedleByPosition(int x /*, int y*/)
{
	WORD sx,sy;
	menu_sitem_info_t *menu_item;
	struct image_item_info_s *image;
	menu_image_header_t *header = &header_table;
	//BYTE sosd_win;

	WORD position;
	WORD barsize;
	BYTE value;
	BYTE index;
	BYTE rate = 100;

	//index
	if(curr_menu==&menu_slider_page)
		index = 0;
	else /*if(curr_menu==&menu_slider3_page) */{
		if(curr_menu->focus == SLIDE3_1)		index = 0;
		else if(curr_menu->focus == SLIDE3_2)	index = 1;
		else if(curr_menu->focus == SLIDE3_3)	index = 2;
		else									index = 0; //BUG
	}

	//get window start
	menu_item = &curr_menu->items[0];
	//image = menu_item->osd_s.image;
	sx = menu_item->x;
	sy = menu_item->y;
	position = x - sx;

	//get bar end position
	menu_item = &curr_menu->items[curr_menu->item_start+1+index*2];
	//image = menu_item->osd_s.image;
	barsize = menu_item->x;
	
	//get niddle start
	menu_item = &curr_menu->items[curr_menu->item_start+index*2];
	image = menu_item->image;
	sx += menu_item->x;
	sy += menu_item->y;
	position -= menu_item->x;

	//calculate barsize
	barsize -= menu_item->x;

	//position = x - curr_menu->items[0].x - curr_menu->items[curr_menu->focus].x;		

	dMenuPrintf("\n\rx:%d->position:%d barsize:%d", x, position, barsize);


	value = ((DWORD)position * 100 / barsize) /* *(rate / 100) */;
	dMenuPrintf("\n\rMenuUpdateSlideNeedleByPosition value:%bd",value);

	if(curr_menu==&menu_slider_page) {
		curr_slide_value = value;
		curr_slide_value=SetSlideValue(curr_slide_value);


//img_slide_sharp
//	InitSlideCtrl(&img_slide_sharp, &ImgAdjGetSharpness,&ImgAdjSetSharpness,&ImgAdjChangeSharpness);


		dMenuPrintf("->%bd", curr_slide_value);
	}
	else /*if(curr_menu==&menu_slider3_page) */{
		curr_slide3_value[index] = value;
		curr_slide3_value[index]=SetSlide3Value(index, curr_slide3_value[index]);
		dMenuPrintf("->%bd", curr_slide3_value[index]);
	}
}
#endif

//code struct menu_fitem_info_s menu_slider_text_items[] = {
//	{ 0,0,	270,62,	3,1, 0,0		},
//};
//	{ 0,0,		265,52,		3,1, 0,0,	0xF0,	 0x10,	0xF800	},	//R, or F81F:Magenta
//	{ 1,3*3,	265,102,	3,1, 0,0,	0xF0,	 0x18,	0x07E0	},	//G
//	{ 2,6*3,	265,152,	3,1, 0,0,	0xF0,	 0x20,	0x001F	},	//B, or 07FF:Cyan 

//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void MenuDrawSlideCurrValue(BYTE index, short value)
{
	//struct menu_fitem_info_s *text_info; // =  &menu_slider_text_items[0];
	BYTE n_char;
	BYTE palette;
	BYTE *str;
	BYTE font;
	BYTE start=0;

	//code struct menu_fitem_info_s

	menu_fitem_info_t *text_info = curr_menu->texts;
	text_info += index;

	//we need a window & start of OSDRAM

	n_char = TWitoa(value, menu_str_buff);
	str = menu_str_buff;
							
	//FontOsdWinPuts2();
	if(IsDigit(*str)==0) {
		//first one can be minus
		palette = text_info->BPP1Color;
		FOsdRamSetAddrAttr(text_info->osdram,text_info->BPP1Color);
		//assume page3
		WriteTW88(REG307, *str);
		str++;
		start++;
	}

	palette = text_info->BPP3_palette;
	dMenuPrintf("\n\rwin:%bx osdram:%d,palette:x%02bx n_char:%02bx",text_info->winno, text_info->osdram, palette, n_char);
	dMenuPrintf("  str:%s",str);
	FOsdRamSetAddrAttr(text_info->osdram+start,palette >> 2);
	while(*str) {
		font = *str++;
		font -= 0x30;
		WriteTW88(REG307, font * 3 + BPP3DIGIT_START);
	}
	//adjust FontOSD width.
	FOsdWinScreenWH(text_info->winno, n_char,1);
}


//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/

void proc_slider_bg(void)
{
	menu_sitem_info_t *menu_item;
	BYTE sosd_win;

//	curr_menu->focus = MENU_SLIDER_BAR_INPUT;	//start from slide input
//	curr_menu->focus = MENU_SLIDER_BAR_OK; //test
//  curr_menu_watchdog_focus = curr_menu->focus;
	//draw current value
	//draw background image.
	MenuDrawCurrImage(0,0);	//background
	MenuDrawCurrImage(0,1);   //slide title. Note:MenuDrawCurrImage will takecare the exception case.

	if(curr_menu->focus == MENU_SLIDER_BAR_INPUT_RED) {
		MenuDrawSlideNeedleImageXY(0,curr_slide_value, 1/*MENU_SLIDER_BAR_INPUT_RED*/);
		//MenuTurnOffCurrImage(curr_menu->focus-1);	//disable GRAY window. BK110516
		menu_item = &curr_menu->items[curr_menu->focus-1];  //gray niddle
		sosd_win = menu_item->win;
		SOsd_Enable( sosd_win, OFF );
		SOsd_UpdateReg(sosd_win, sosd_win);
	}
	else {
		MenuDrawSlideNeedleImageXY(0,curr_slide_value, 0/*MENU_SLIDER_BAR_INPUT_GRAY*/);
		MenuDrawCurrImage(0,curr_menu->focus);
	}

	//MenuDrawCurrValue(0, GetSlideValue()); //item#, value

	//read current value & update bar position.

	MenuFontOsdItemInit(0, OFF);
	MenuDrawSlideCurrValue(0, (short)curr_slide_value);
}

void proc_slider3_bg(void)
{
	BYTE index;
	BYTE i;
	menu_sitem_info_t *menu_item;
	BYTE sosd_win;

	if(curr_menu->focus == SLIDE3_1)		index = 0;
	else if(curr_menu->focus == SLIDE3_2)	index = 1;
	else if(curr_menu->focus == SLIDE3_3)	index = 2;
	else									index = 3; //NOTE:No Focused. It can be OK or Cancel

	UseSOsdHwBuff = 1;
	//SOsdHwBuffClean();
	SOsd_CleanLut();	
	SOsd_CleanRlc();
	SOsd_CleanReg();

	MenuDrawCurrImage(0,0);		//background
	MenuDrawCurrImage(0,1);   	//slide title. Note:MenuDrawCurrImage will takecare the exception case.

	for(i=0; i < 3; i++) {
		if(	index == i) {
			MenuDrawSlideNeedleImageXY(i,curr_slide3_value[i], 1/*MENU_SLIDER_BAR_INPUT_RED*/);

			//if gray niddle is active, deactivate it.
			menu_item = &curr_menu->items[curr_menu->item_start+index*2];  //gray niddle
			sosd_win = menu_item->win;
			SOsd_Enable( sosd_win, OFF );
			SOsd_UpdateReg(sosd_win, sosd_win);
		}
		else {
			MenuDrawSlideNeedleImageXY(i,curr_slide3_value[i], 0);
		}

//		MenuFontOsdItemInit(i, OFF);
//		MenuDrawSlideCurrValue(i, (short)curr_slide3_value[i]);
	}
	if(index==3) {
		MenuDrawCurrImage(0,curr_menu->focus);
	}

	if(UseSOsdHwBuff) {
		SOsd_show(1);
		UseSOsdHwBuff = 0;
	}
	//Update FOsd
	MenuFontOsdItemInitNew(0,2);
	for(i=0; i < 3; i++) {
		MenuDrawSlideCurrValue(i, (short)curr_slide3_value[i]);
		//MenuFontOsdItemInit(i, OFF);
	}
#if 0
	//debug
	FOsdWinEnable(3 /*winno*/,ON);	//WINn enable
	FOsdRamSetAddrAttr(0x10,text_info->BPP1Color);
	for(i=0; i < 64; i++) {
		WriteTW88(REG307, *str);	
	}
#endif
}

//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void proc_slider_ctrl(void)
{
	BYTE index;
	if(curr_menu==&menu_slider_page) {
		if(curr_menu->focus == MENU_SLIDER_BAR_INPUT_RED)
			MenuDrawSlideNeedleImageXY(0,curr_slide_value, 0/*MENU_SLIDER_BAR_INPUT_GRAY*/);
	}
	else {
		if(curr_menu->focus == SLIDE3_1)		index = 0;
		else if(curr_menu->focus == SLIDE3_2)	index = 1;
		else if(curr_menu->focus == SLIDE3_3)	index = 2;
		else index = 3;
		
		if(index != 3)
			MenuDrawSlideNeedleImageXY(index,curr_slide3_value[index], 0);
	}

	//move to OK
	if(curr_menu==&menu_slider3_page) 	curr_menu->focus = SLIDE3_OK;
	else								curr_menu->focus = MENU_SLIDER_BAR_OK;
    curr_menu_watchdog_focus = curr_menu->focus;
	MenuDrawCurrImage(0,curr_menu->focus);
}
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void proc_slider_ok(void)
{
	WaitVBlank(1); 			//??
	SpiOsdEnable(OFF);		//??

	if(slide_title_img==&img_slide_clock) {
		//aRGB_LLPLLSetDivider(,1);
		WriteTW88(REG003, 0xF8);	//enable ISR.
		aRGB_SetFilterBandwidth(7, 0);
	}
	else if(slide_title_img==&img_slide_phase) {
		aRGB_SetFilterBandwidth(7, 0);
	}


	//update current value to EE
	if(curr_menu==&menu_slider3_page) {
		SaveSlide3Value(0,curr_slide3_value[0]);
		SaveSlide3Value(1,curr_slide3_value[1]);
		SaveSlide3Value(2,curr_slide3_value[2]);   //BK110222. Bugfix
	}
	else
		SaveSlideValue(curr_slide_value);
	
	SpiOsdEnable(ON);		//??BY110818 Why it need ?

	//turn off FontOSD
	FOsdWinOffAll();
	if(curr_menu==&menu_slider3_page)
		//recover 8~15 palette color
		FOsdSetDefPaletteColor(0);

	pop_menu_stack_level();

	//redraw parent bg 
	curr_menu->items[0].p();		
}
void proc_slider_undo(void)
{
	//recover last saved value
	if(curr_menu==&menu_slider3_page) {
		SetSlide3Value(0,last_slide3_value[0]);
		SetSlide3Value(1,last_slide3_value[1]);
		SetSlide3Value(2,last_slide3_value[2]);
	}
	else {
		dMenuPrintf("\n\rproc_slider_undo restore to %bd",last_slide3_value[0]);
		SetSlideValue(last_slide3_value[0]);
	}

	if(slide_title_img==&img_slide_clock) {
		WriteTW88(REG003, 0xF8);	//enable ISR.
		aRGB_SetFilterBandwidth(7, 0);
		//aRGB_LLPLLSetDivider(,1);
	}
	else if(slide_title_img==&img_slide_phase) {
		aRGB_SetFilterBandwidth(7, 0);
	}

	//turn off FontOSD
	FOsdWinOffAll();
	if(curr_menu==&menu_slider3_page)
		//recover 8~15 palette color
		FOsdSetDefPaletteColor(0);

	pop_menu_stack_level();	
	//redraw parent bg 
	curr_menu->items[0].p();		
}
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void proc_slider_left(void)
{
	dMenuPrintf("\n\rproc_slider_left curr_slide_value:%bd", curr_slide_value);

	//
	//special
	//
#if 0
	if(	GetSlideValue == GetaRGB_Clock) {
		//decrease one
		//use real value 0..0xFFF(0..4095)

		//curr_slide_value--;
		//curr_slide_value=SetSlideValue(curr_slide_value);
		//dMenuPrintf("->%d", curr_slide_value);
		//redraw
		//MenuDrawCurrImage(0,0);	//curr_menu->items[0].p();	//draw current value
		//MenuDrawCurrValue(0, GetSlideValue()); //item#, value
		//do not change needle...
		//WORD size===>MenuDrawSlideCurrValue(0, curr_slide_value);
		return;
	}
#endif
	if(curr_slide_value){
		curr_slide_value--;
		curr_slide_value=VarySlideValue(curr_slide_value);
		dMenuPrintf("->%bd", curr_slide_value);
		//redraw
		//MenuDrawCurrImage(0,0);	//curr_menu->items[0].p();	//draw current value
		//MenuDrawCurrValue(0, GetSlideValue()); //item#, value
		MenuDrawSlideNeedleImageXY(0, curr_slide_value, 1/*MENU_SLIDER_BAR_INPUT_RED*/);
		MenuDrawSlideCurrValue(0, (short)curr_slide_value);
	}
}

void proc_slider3_left(void)
{
	BYTE index;

	if(curr_menu->focus == SLIDE3_1)		index = 0;
	else if(curr_menu->focus == SLIDE3_2)	index = 1;
	else if(curr_menu->focus == SLIDE3_3)	index = 2;
	else									index = 0;

	dMenuPrintf("\n\rproc_slider_left curr_slide_value:%bd", curr_slide3_value[index]);
	if(curr_slide3_value[index]){
		curr_slide3_value[index]-=1;
		curr_slide3_value[index]=VarySlide3Value(index, curr_slide3_value[index]);
		dMenuPrintf("->%bd", curr_slide3_value[index]);
		//redraw
		MenuDrawSlideNeedleImageXY(index, curr_slide3_value[index], 1); //index,value,red(or focused)
		MenuDrawSlideCurrValue(index, (short)curr_slide3_value[index]); 		//index,value
	}
}
void proc_slider_left_tsc(void)
{
	if(curr_menu==&menu_slider3_page) {
		//remove pressed icon
		MenuTurnOffCurrImage(curr_menu->focus);

		curr_menu->focus = curr_menu->select;
        curr_menu_watchdog_focus = curr_menu->focus;

		proc_slider3_left();
	}
	else {
		//remove pressed icon
		MenuTurnOffCurrImage(curr_menu->focus);

		curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
        curr_menu_watchdog_focus = curr_menu->focus;

		proc_slider_left();
	}
}	


//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void proc_slider_right(void)
{
	dMenuPrintf("\n\rproc_slider_right curr_slide_value:%bd", curr_slide_value);

	//
	//special
	//
#if 0
	if(	GetSlideValue == GetaRGB_Clock) {
		//increase one
		//use real value 0..0xFFF(0..4095)
	 	return;
	}
#endif

	if(curr_slide_value < 100){
		curr_slide_value++;
		curr_slide_value=VarySlideValue(curr_slide_value);
		dMenuPrintf("->%bd", curr_slide_value);
		//redraw
		MenuDrawSlideNeedleImageXY(0, curr_slide_value, 1/*MENU_SLIDER_BAR_INPUT_RED*/);
		MenuDrawSlideCurrValue(0, (short)curr_slide_value);
	}
}
void proc_slider3_right(void)
{
	BYTE index;

	if(curr_menu->focus == SLIDE3_1)		index = 0;
	else if(curr_menu->focus == SLIDE3_2)	index = 1;
	else if(curr_menu->focus == SLIDE3_3)	index = 2;
	else									return;	//index = 0;

	if(curr_slide3_value[index] < 100){
		curr_slide3_value[index]+=1;
		curr_slide3_value[index]=VarySlide3Value(index, curr_slide3_value[index]);
		dMenuPrintf("->%bd", curr_slide3_value[index]);
		//redraw
		MenuDrawSlideNeedleImageXY(index, curr_slide3_value[index], 1/*MENU_SLIDER_BAR_INPUT_RED*/);
		MenuDrawSlideCurrValue(index, (short)curr_slide3_value[index]);
	}
}
void proc_slider_right_tsc(void)
{
	if(curr_menu==&menu_slider3_page) {
		//remove pressed icon
		MenuTurnOffCurrImage(curr_menu->focus);

		curr_menu->focus = curr_menu->select;
        curr_menu_watchdog_focus = curr_menu->focus;

		proc_slider3_right();
	}
	else {
		//remove pressed icon
		MenuTurnOffCurrImage(curr_menu->focus);

		curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
        curr_menu_watchdog_focus = curr_menu->focus;

		proc_slider_right();
	}
}

#ifdef UNCALLED_SEGMENT_CODE
void proc_slider_digit(void)
{
//	proc_draw_digit(curr_slide_value);
	struct menu_sitem_info_s *item_info;
	BYTE n_char;

	n_char = TWitoa((BYTE)curr_slide_value, menu_str_buff);

	item_info = &curr_menu->items[MENU_SLIDER_BAR_DIGIT];

	FontOsdWinPuts2(item_info, menu_str_buff);	
}
#endif

//need a continus touch input
//return
//	0:No
//	1:Slide
//	3: Slider3
#ifdef SUPPORT_TOUCH
BYTE MenuIsSlideMode(void)
{
	if(curr_menu==&menu_slider3_page) {
		return 3;
	}
	if(curr_menu==&menu_slider_page) {
		return 1;
	}

	return 0;
}
#endif

//=========================
// POSITION DIALOG
//=========================
//----------------------
//manu->input->setting with RGB/HDMI->display->position
//----------------------
//-------------------------------------
// position popup MENU
//-------------------------------------

//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void proc_position_bg(void)
{
#if	1 //BK110512
	proc_menu_bg();
#else
	//draw current value
	//draw background image.
	MenuDrawCurrImage(0,0);					//draw background image.
	if(curr_menu->focus != MENU_POSITION_INPUT_RED)		//2
		MenuDrawCurrImage(0,MENU_POSITION_INPUT_GRAY);	 //1
	MenuDrawCurrImage(0,curr_menu->focus);	//draw focused item
#endif
	//read current value & update bar position.
	MenuFontOsdItemInit(0, OFF);
	MenuFontOsdItemInit(1, OFF);
	proc_position_H_digit();
	proc_position_V_digit();
}
void proc_position_box(void)
{
	//move focus to OK
	curr_menu->select = curr_menu->focus = MENU_POSITION_OK;
    curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;
	//redraw OK with focused.	
	MenuDrawCurrImage(0,0);
	MenuDrawCurrImage(0,MENU_POSITION_INPUT_GRAY);	 //1
	MenuDrawCurrImage(0,curr_menu->focus);	
}
void proc_position_ok(void) 
{
	//read base & update eeprom to 50.
	//save to EEPROM
#if defined(SUPPORT_PC)
	ChangeRGBPosH();
	ChangeRGBPosV();
#endif
	//GetRGBPosBase();
	//PcEepromUseHwValue(OFF,OFF,ON);

	//turn off FontOSD
	FOsdWinOffAll();

	pop_menu_stack_level();
	//redraw parent bg 
	curr_menu->items[0].p();		
}
void proc_position_undo(void) 
{
//	BYTE value;
	//restore previous value
	//readback value freom EEPROM and do not update base.
//	PcEepromSetEffect(OFF,OFF,ON);
#ifdef SUPPORT_PC
	PCRestoreH();
	PCRestoreV();
#endif
	//turn off FontOSD
	FOsdWinOffAll();

	pop_menu_stack_level();
	//redraw parent bg 
	curr_menu->items[0].p();		
}

//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
//BKFYI: If user type ENTER(or select BOX), curr_menu->select will be MENU_POSITION_OK.
// If user press the left/right on the OK/CANCE, 
// still curr_menu->select is not a MENU_POSITION_INPUT_RED. So it should be work.
// Currently, we do not support to move back from OK/CANCEL to the position box.
void proc_position_tsc(BYTE key)
{
	MenuTurnOffCurrImage(curr_menu->focus);
	curr_menu->focus = MENU_POSITION_INPUT_RED;
	curr_menu_watchdog_focus = curr_menu->focus;
    if(curr_menu->select != MENU_POSITION_INPUT_RED) {
		MenuTurnOffCurrImage(MENU_POSITION_INPUT_GRAY);
		curr_menu->select = MENU_POSITION_INPUT_RED;
        curr_menu_watchdog_select = curr_menu->select;			
		MenuDrawCurrImage(0,curr_menu->focus);
	}
	MenuKeyInput_position_page(key);
}

void proc_position_left_tsc(void)
{
	proc_position_tsc(NAVI_KEY_LEFT);
}
void proc_position_right_tsc(void)
{
	proc_position_tsc(NAVI_KEY_RIGHT);
}
void proc_position_up_tsc(void)
{
	proc_position_tsc(NAVI_KEY_UP);
}
void proc_position_down_tsc(void)
{
	proc_position_tsc(NAVI_KEY_DOWN);
}

void proc_position_H_digit(void) 
{
	BYTE value;
#ifdef SUPPORT_PC
	value = GetRGBPosH();
#endif
	dMenuPrintf("\n\rproc_position_H_digit %bd",value);
	MenuDrawSlideCurrValue(0, (short)value);	
//test
//	short value;
//	value=-99;
//	dMenuPrintf("\n\rproc_position_H_digit %d",value);
//	MenuDrawSlideCurrValue(0, value);	
}
void proc_position_V_digit(void) 
{
#ifdef SUPPORT_PC
	BYTE value;
	value = GetRGBPosV();
	dMenuPrintf("\n\rproc_position_V_digit %bd",value);
	MenuDrawSlideCurrValue(1, (short)value);
#endif
//	short value;
//	value = -2;
//	dMenuPrintf("\n\rproc_position_V_digit %d",value);
//	MenuDrawSlideCurrValue(1, value);
}


//=========================
// DIALOG 
//=========================

//----------------------
// main->input->adc->display->autoadj MENU
//----------------------
//-------------------------------------
// autoadj popup MENU
//-------------------------------------
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void proc_autoadj_bg(void) 
{
#if	1 //BK110512
	proc_menu_bg();
#else
	MenuDrawCurrImage(0,0);					//draw background image.
	MenuDrawCurrImage(0,curr_menu->focus);	//draw focused item
#endif
}
void proc_autoadj_ok(void) 
{
#ifdef SUPPORT_PC
	//reset eeprom for curr mode
	PCResetCurrEEPROMMode();  //I need a mode
#endif
	//draw wait icon
	curr_menu->focus = MENU_AUTOADJ_WAIT;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->items[MENU_AUTOADJ_WAIT].p();

#ifdef SUPPORT_PC
	//autoadjust
	CheckAndSetPC();
#endif

	//remove wait icon
	WaitVBlank(1);
	SOsd_Enable(SOSD_WIN0,OFF);
	SOsd_UpdateReg(SOSD_WIN0, SOSD_WIN0);

	//return to parent
	pop_menu_stack_level();
	//redraw parent bg 
	curr_menu->items[0].p();		
}
void proc_autoadj_cancel(void) 
{
	pop_menu_stack_level();
	//redraw parent bg 
	curr_menu->items[0].p();		
}

void proc_autocolor_bg(void) 
{
	proc_menu_bg();
}
void proc_autocolor_ok(void) 
{
	//draw wait icon
	curr_menu->focus = MENU_AUTOADJ_WAIT;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->items[MENU_AUTOADJ_WAIT].p();

#if defined(SUPPORT_PC)
	//aoutocolor
	AutoTuneColor();
#endif

	//remove wait icon
	WaitVBlank(1);
	SOsd_Enable(SOSD_WIN0,OFF);
	SOsd_UpdateReg(SOSD_WIN0, SOSD_WIN0);


	//return to parent
	pop_menu_stack_level();
	//redraw parent bg 
	curr_menu->items[0].p();		
}
void proc_autocolor_cancel(void) 
{
	pop_menu_stack_level();
	//redraw parent bg 
	curr_menu->items[0].p();		
}

//----------------------
// main->system->restore MENU
//----------------------
//-------------------------------------
// restore popup MENU
//-------------------------------------
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void proc_restore_bg(void) 
{
	proc_menu_bg();
}
//restore all default value
void proc_restore_ok(void) 
{
	BYTE value;

	//read InputMain
	MenuInputMain = GetInputMain();

	//MenuPrintf("\n\rEE initialize........");
	ClearBasicEE();
	//SaveDebugLevelEE(0);
	//SaveFWRevEE( FWVER );
	//EE_PrintCurrInfo();

	//keep video source.
	SaveInputMainEE( MenuInputMain );

	//change HW.
	//------------------------
	// setup eeprom effect
	//------------------------
	SetImage(MenuInputMain);

	SetAspectHW(GetAspectModeEE());
	value = EE_Read(EEP_FLIP);
	if(value) {
	    WriteTW88(REG201, ReadTW88(REG201) | 0x80);
	}
	OsdSetTime(EE_Read(EEP_OSD_TIMEOUT));
	OsdSetTransparent(EE_Read(EEP_OSD_TRANSPARENCY));
	BackLightSetRate(EE_Read(EEP_BACKLIGHT));
#ifdef SUPPORT_TOUCH
	ReadCalibDataFromEE();
#endif

	pop_menu_stack_level();
	//redraw parent bg 
	curr_menu->items[0].p();		
}
void proc_restore_undo(void) 
{
	pop_menu_stack_level();
	//redraw parent bg 
	curr_menu->items[0].p();		
}

//----------------------
// main->system->sys_info MENU
//----------------------
//-------------------------------------
// info popup MENU
//-------------------------------------
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/

void proc_info_bg(void) 
{
#if	1 //BK110512
	proc_menu_bg();
#else
	MenuDrawCurrImage(0,0);					//draw background image.
	MenuDrawCurrImage(0,curr_menu->focus);	//draw focused item
	//display current info
#endif

	//GetDviModeEE();

}
void proc_info_ok(void) 
{
	//remove FOsd
	FOsdWinOffAll();

	pop_menu_stack_level();
	//redraw parent bg with focus 
	curr_menu->items[0].p();		
}

//----------------------
// main->system->more->flip MENU
//----------------------
//-------------------------------------
// flip popup MENU
//-------------------------------------
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/

void proc_flip_bg(void) 
{
	proc_menu_bg();
}
void proc_flip_ok(void) 
{
	BYTE value;
	//flip
	value=ReadTW88(REG201);
	if(value & 0x80) {
		WriteTW88(REG201, value & 0x7F);
		EE_Write(EEP_FLIP, 0);
	}
	else {
	    WriteTW88(REG201, value | 0x80);
		EE_Write(EEP_FLIP, 1);
	}
	pop_menu_stack_level();
	//redraw parent bg 
	curr_menu->items[0].p();		
}
void proc_flip_undo(void) 
{
	pop_menu_stack_level();
	//redraw parent bg 
	curr_menu->items[0].p();		
}


void proc_disp_resolution_bg(void)
{
	proc_menu_bg();
}
void proc_disp_resolution_ok(void)
{
	//remove FOsd
	FOsdWinOffAll();

	pop_menu_stack_level();
	//redraw parent bg with focus 
	curr_menu->items[0].p();		
}


//=========================
// NOTE DIALOG
//=========================
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/

void proc_dvi_mode_bg(void)	 
{
	proc_menu_bg();
}
void proc_dvi_mode_24bit(void)	 
{
	curr_menu->select = MENU_DVI_MODE_24BIT;
    curr_menu_watchdog_select = curr_menu->select;
	curr_menu->focus = MENU_DVI_MODE_OK;	
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->items[0].p();
}
void proc_dvi_mode_16bit(void)	 
{
	curr_menu->select = MENU_DVI_MODE_16BIT;
    curr_menu_watchdog_select = curr_menu->select;
	curr_menu->focus = MENU_DVI_MODE_OK;	
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->items[0].p();
}
void proc_dvi_mode_ok(void)	 
{
	SaveDviModeEE(curr_menu->select==MENU_DVI_MODE_24BIT ? 0 : 1);
	proc_return_icon();
}
void proc_dvi_mode_cancel(void)	 
{
	proc_return_icon();
}

void proc_hdmi_mode_bg(void)	 
{
	proc_menu_bg();
}
void proc_hdmi_mode_pc(void)	 
{	
	curr_menu->select = MENU_HDMI_MODE_PC;
    curr_menu_watchdog_select = curr_menu->select;
	curr_menu->focus = MENU_DVI_MODE_OK;	
	curr_menu_watchdog_focus = curr_menu->focus;
    SetImage(INPUT_HDMIPC);
	curr_menu->items[0].p();
}
void proc_hdmi_mode_tv(void)	 
{
	curr_menu->select = MENU_HDMI_MODE_TV;
    curr_menu_watchdog_select = curr_menu->select;
	curr_menu->focus = MENU_DVI_MODE_OK;	
	curr_menu_watchdog_focus = curr_menu->focus;
    SetImage(INPUT_HDMITV);
	curr_menu->items[0].p();
}
void proc_hdmi_mode_ok(void)	 
{
	BYTE hdmi_mode;
	
	hdmi_mode = curr_menu->select==MENU_HDMI_MODE_PC ? 0 : 1;
	if(GetHdmiModeEE() != hdmi_mode)
		SaveHdmiModeEE(hdmi_mode);
	if(cancel_recover_value != curr_menu->select) {
		if(curr_menu->select == MENU_HDMI_MODE_PC) 
			MenuInputMain = INPUT_HDMIPC;
		else
			MenuInputMain = INPUT_HDMITV;
		SetInputMain(MenuInputMain);
		SaveInputMainEE(MenuInputMain);
	}
	proc_return_icon();
}
void proc_hdmi_mode_cancel(void)	 
{
	//recover
	curr_menu->items[cancel_recover_value].p();

	proc_return_icon();
}

//----------------------
// main->system->more->aspect MENU
//----------------------
//-------------------------------------
// aspect popup MENU
//-------------------------------------
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/

void proc_aspect_bg(void) 
{
	proc_menu_bg();
}
void proc_aspect_normal(void) 
{
	curr_menu->focus = MENU_ASPECT_OK;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->select = MENU_ASPECT_NORMAL;
    curr_menu_watchdog_select = curr_menu->select;
	//MenuDrawCurrImage(0,0);
	//proc_menu_bg();
	MenuDrawCurrImage(0,curr_menu->focus);
	MenuDrawCurrImage(0,curr_menu->select+curr_menu->item_total);	//select

	SetAspectHW(curr_menu->select - curr_menu->item_start);
}
void proc_aspect_zoom(void) 
{
	curr_menu->focus = MENU_ASPECT_OK;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->select = MENU_ASPECT_ZOOM;
    curr_menu_watchdog_select = curr_menu->select;
	//MenuDrawCurrImage(0,0);
	//proc_menu_bg();
	MenuDrawCurrImage(0,curr_menu->focus);
	MenuDrawCurrImage(0,curr_menu->select+curr_menu->item_total);	//select

	SetAspectHW(curr_menu->select - curr_menu->item_start);
}
void proc_aspect_full(void) 
{
	curr_menu->focus = MENU_ASPECT_OK;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->select = MENU_ASPECT_FULL;
    curr_menu_watchdog_select = curr_menu->select;
	//MenuDrawCurrImage(0,0);
	//proc_menu_bg();
	MenuDrawCurrImage(0,curr_menu->focus);
	MenuDrawCurrImage(0,curr_menu->select+curr_menu->item_total);	//select
	//BKTODO110516. Change HW
	SetAspectHW(curr_menu->select - curr_menu->item_start);
}
void proc_aspect_pano(void) 
{
	curr_menu->focus = MENU_ASPECT_OK;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->select = MENU_ASPECT_PANO;
    curr_menu_watchdog_select = curr_menu->select;
	//MenuDrawCurrImage(0,0);
	//proc_menu_bg();
	MenuDrawCurrImage(0,curr_menu->focus);
	MenuDrawCurrImage(0,curr_menu->select+curr_menu->item_total);	//select
	//BKTODO110516. Change HW
	SetAspectHW(curr_menu->select - curr_menu->item_start);
}
void proc_aspect_ok(void) 
{
	if(cancel_recover_value != curr_menu->select) {
		//save to EE.
		SaveAspectModeEE(curr_menu->select-curr_menu->item_start);

		MenuInputMain = GetInputMain();

		//change input
		SetInputMain(0xff);	//InputMain = 0xff;			// start with saved input
		SpiOsdEnable(OFF);

		switch(MenuInputMain) {
		case INPUT_CVBS:	ChangeCVBS();		break;
		case INPUT_SVIDEO:	ChangeSVIDEO();		break;
#ifdef SUPPORT_COMPONENT
		case INPUT_COMP:	ChangeComponent();	break;
#endif
#ifdef SUPPORT_PC
		case INPUT_PC:		ChangePC();			break;
#endif
#ifdef SUPPORT_LVDSRX
		case INPUT_LVDS:	ChangeLVDSRx();		break;
#endif
		case INPUT_HDMIPC:	
		case INPUT_HDMITV:	ChangeHDMI();		break;
#ifdef SUPPORT_BT656_LOOP
		case INPUT_BT656:	ChangeBT656Loop();		break;
#endif
		}		
		//??SpiOsdSetDeValue();
		//??FOsdSetDeValue();
	
		SpiOsdEnable(ON);
	}
	pop_menu_stack_level();
	//redraw parent bg 
	curr_menu->items[0].p();		
}
void proc_aspect_cancel(void) 
{
	//recover
	curr_menu->items[cancel_recover_value].p();

	pop_menu_stack_level();
	//redraw parent bg 
	curr_menu->items[0].p();		
}

//=========================
// LIST MENU
//=========================
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void proc_menu_list_return(void)
{
	if(curr_menu==&menu_slider_page || curr_menu==&menu_slider3_page) {
		proc_slider_undo();
	}
	else if(curr_menu==&menu_position_page) {
		proc_position_undo();
	}
	else if(curr_menu==&menu_autoadj_page) {
		proc_autoadj_cancel();
	}
	else if(curr_menu==&menu_autocolor_page) {
		proc_autocolor_cancel();
	}
	else if(curr_menu==&menu_sys_info_page) {
		proc_info_ok();
	}
	else if(curr_menu==&menu_disp_resolution_page) {
		proc_disp_resolution_ok();
	}
	else {
		proc_return_icon();
	}
}


//=====================================
// YUV List PAGE
//
// path:main->input->set
//=====================================

//----------------------
// internal decoder image Select MENU
//----------------------
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/

void proc_yuv_list_bg(void)
{	
	proc_menu_bg();
}

void proc_yuv_list_bright(void)
{
	push_menu_stack_level();
	
	curr_menu = &menu_slider_page;
	curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->select = MENU_SLIDER_BAR_INPUT_RED;
    curr_menu_watchdog_select = curr_menu->select;
	InitSlideCtrl(&img_slide_bright, 
		&ImgAdjGetBrightnessY,
		&ImgAdjSetBrightnessY,
		&ImgAdjSetBrightnessY,
		&ImgAdjChangeBrightnessY);

	curr_menu->items[0].p();
}
void proc_yuv_list_contrast(void)
{
	push_menu_stack_level();
		
	curr_menu = &menu_slider_page;
	curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->select = MENU_SLIDER_BAR_INPUT_RED;
    curr_menu_watchdog_select = curr_menu->select;
	InitSlideCtrl(&img_slide_contrast, 
		&ImgAdjGetContrastY,
		&ImgAdjSetContrastY,
		&ImgAdjSetContrastY,
		&ImgAdjChangeContrastY);

	curr_menu->items[0].p();
}	
void proc_yuv_list_hue(void)		
{
	push_menu_stack_level();
	
	curr_menu = &menu_slider_page;
	curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
	curr_menu_watchdog_focus = curr_menu->focus;
    InitSlideCtrl(&img_slide_hue, 
		&ImgAdjGetHUE,
		&ImgAdjVaryHUE,
		&ImgAdjSetHUE,
		&ImgAdjChangeHUE);
	curr_menu->items[0].p();
}

void proc_yuv_list_saturate(void)	
{
	push_menu_stack_level();
	
	curr_menu = &menu_slider_page;
	curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
	curr_menu_watchdog_focus = curr_menu->focus;
    InitSlideCtrl(&img_slide_saturate, 
		&ImgAdjGetSaturation,
		&ImgAdjSetSaturation,
		&ImgAdjSetSaturation,
		&ImgAdjChangeSaturation);

	curr_menu->items[0].p();
}
void proc_yuv_list_sharp(void)
{
	push_menu_stack_level();
	
	curr_menu = &menu_slider_page;
	curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
	curr_menu_watchdog_focus = curr_menu->focus;
    InitSlideCtrl(&img_slide_sharp, 
		&ImgAdjGetSharpness,
		&ImgAdjSetSharpness,
		&ImgAdjSetSharpness,
		&ImgAdjChangeSharpness);

	curr_menu->items[0].p();
}

//=====================================
// RGB List PAGE
//
// path:main->input->set
//=====================================
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/

void proc_rgb_list_bg(void) 
{
	proc_menu_bg();
}

void proc_rgb_list_bright(void) 
{
	push_menu_stack_level();

	curr_menu = &menu_slider_page;
	curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->select = MENU_SLIDER_BAR_INPUT_RED;
    curr_menu_watchdog_select = curr_menu->select;
	InitSlideCtrl(&img_slide_bright,
		&ImgAdjGetBrightnessY,
		&ImgAdjSetBrightnessY,
		&ImgAdjSetBrightnessY,
		&ImgAdjChangeBrightnessY);
	curr_menu->items[0].p();
}
void proc_rgb_list_contrast(void) 
{
	push_menu_stack_level();
	
	curr_menu = &menu_slider_page;
	curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->select = MENU_SLIDER_BAR_INPUT_RED;
    curr_menu_watchdog_select = curr_menu->select;
	InitSlideCtrl(&img_slide_contrast, 
		&ImgAdjGetContrastY,
		&ImgAdjSetContrastY,
		&ImgAdjSetContrastY,
		&ImgAdjChangeContrastY);
	curr_menu->items[0].p();
}
void proc_rgb_list_color(void)
{
	push_menu_stack_level();

	curr_menu = &menu_slider3_page;
	curr_menu->select = curr_menu->focus = SLIDE3_1;
    curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;
	InitSlide3Ctrl(&img_slide_rgb, 
		&ImgAdjGetContrastRGB,
		&ImgAdjSetContrastRGB,
		&ImgAdjSetContrastRGB,
		&ImgAdjChangeContrastRGB);
	curr_menu->items[0].p();
}

//=====================================
// AnalogPC List PAGE
//
// path:main->input->set
//=====================================
//----------------------
// main->input->image(ADC) MENU
//----------------------

//----------------------
// ADC image Select MENU
//----------------------
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/

void proc_apc_list_bg(void) 
{
	proc_menu_bg();
}

void proc_apc_list_bright(void) 
{
	push_menu_stack_level();
		
	curr_menu = &menu_slider_page;
	curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
	curr_menu_watchdog_focus = curr_menu->focus;
    InitSlideCtrl(&img_slide_bright, 
		&ImgAdjGetBrightnessY,
		&ImgAdjSetBrightnessY,
		&ImgAdjSetBrightnessY,
		&ImgAdjChangeBrightnessY);

	curr_menu->items[0].p();
}
void proc_apc_list_contrast(void) 
{
	push_menu_stack_level();
	
	curr_menu = &menu_slider_page;
	curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
	curr_menu_watchdog_focus = curr_menu->focus;
    InitSlideCtrl(&img_slide_contrast, 
		&ImgAdjGetContrastY,
		&ImgAdjSetContrastY,
		&ImgAdjSetContrastY,
		&ImgAdjChangeContrastY);

	curr_menu->items[0].p();
}

void proc_apc_list_color(void) 
{
	push_menu_stack_level();

	curr_menu = &menu_slider3_page;
	curr_menu->select = curr_menu->focus = SLIDE3_1;
    curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;
	InitSlide3Ctrl(&img_slide_rgb, 
		&ImgAdjGetContrastRGB,
		&ImgAdjSetContrastRGB,
		&ImgAdjSetContrastRGB,
		&ImgAdjChangeContrastRGB);

	curr_menu->items[0].p();
}

void proc_apc_list_position(void) 
{
	push_menu_stack_level();
	
	curr_menu = &menu_position_page;
	curr_menu->select = curr_menu->focus = MENU_POSITION_INPUT_RED;
    curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;

	curr_menu->items[0].p();
}		
void proc_apc_list_phase(void) 
{
	push_menu_stack_level();
	aRGB_SetFilterBandwidth(0, 0);
	
	curr_menu = &menu_slider_page;
	curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
    curr_menu_watchdog_focus = curr_menu->focus;
#if defined(SUPPORT_PC)
	InitSlideCtrl(&img_slide_phase,
		&GetaRGB_Phase,
		&VaryaRGB_Phase,
		&SetaRGB_Phase,
		&ChangeaRGB_Phase);
#endif
	curr_menu->items[0].p();
}
		
void proc_apc_list_clock(void) 
{
	push_menu_stack_level();
	
	WriteTW88(REG003, 0xFE);	//disable ISR.
	aRGB_SetFilterBandwidth(0, 0);


	curr_menu = &menu_slider_page;
	curr_menu->select = curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
    curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;
#if defined(SUPPORT_PC) 
	InitSlideCtrl(&img_slide_clock, 
		&GetaRGB_Clock,
		&SetaRGB_Clock,
		&SetaRGB_Clock,
		&ChangeaRGB_Clock);
#endif
	curr_menu->items[0].p();
}		

//update PixelClock,Phase,Position
void proc_apc_list_autoadj(void) 
{
	push_menu_stack_level();

	curr_menu = &menu_autoadj_page;
	curr_menu->focus = MENU_AUTOADJ_OK;			//move to OK
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->items[0].p();
}
			
void proc_apc_list_autocolor(void) 
{
	push_menu_stack_level();

	curr_menu = &menu_autocolor_page;
	curr_menu->focus = MENU_AUTOCOLOR_OK;		//move to OK
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->items[0].p();
}			


//=====================================
// HDMI List PAGE
//
// path:main->input->set
//=====================================
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void proc_hdmi_list_bg(void) 
{
	proc_menu_bg();
}
void proc_hdmi_list_mode(void)
{
	push_menu_stack_level();

	curr_menu = &menu_hdmi_mode_page; //BK160209
	curr_menu->focus = curr_menu->item_start+(MenuInputMain-INPUT_HDMIPC); //GetHdmiModeEE();
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->select = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;
	cancel_recover_value = curr_menu->select;
	curr_menu->items[0].p();
}
void proc_hdmi_list_setting(void)
{
	push_menu_stack_level();

	//if(GetHdmiModeEE()==0)	curr_menu = &menu_rgb_list_page;
	//else 					curr_menu = &menu_yuv_list_page;
	if(MenuInputMain==INPUT_HDMIPC)	curr_menu = &menu_rgb_list_page;  //BK160209
	else 							curr_menu = &menu_yuv_list_page;  //BK160209

	curr_menu->focus = curr_menu->item_start;					//start from ...
	curr_menu_watchdog_focus = curr_menu->focus;

	curr_menu->items[0].p();				// call new background draw routine
}


#if 0
void proc_hdmipc_list_bg(void) 
{
	proc_menu_bg();
}
void proc_hdmipc_list_bright(void)
{
	push_menu_stack_level();

	curr_menu = &menu_slider3_page;  //BK160209
	curr_menu->select = curr_menu->focus = SLIDE3_1;
    curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;
	InitSlide3Ctrl(&img_slide_bright, 
		&ImgAdjGetBrightnessRGB,
		&ImgAdjSetBrightnessRGB,
		&ImgAdjChangeBrightnessRGB);
	curr_menu->items[0].p();
}
void proc_hdmipc_list_contrast(void)
{
	push_menu_stack_level();

	curr_menu = &menu_slider3_page;  //BK160209
	curr_menu->select = curr_menu->focus = SLIDE3_1;
    curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;
	InitSlide3Ctrl(&img_slide_contrast, 
		&ImgAdjGetContrastRGB,
		&ImgAdjSetContrastRGB,
		&ImgAdjChangeContrastRGB);
	curr_menu->items[0].p();
}

void proc_hdmipc_list_phase(void)
{
	push_menu_stack_level();
	
	curr_menu = &menu_slider_page;  //BK160209
	curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->select = MENU_SLIDER_BAR_INPUT_RED;
    curr_menu_watchdog_select = curr_menu->select;

	InitSlideCtrl(&img_slide_phase,
		&GetHdmiPhase,
		&SetHdmiPhase,
		&ChangeHdmiPhase);

	curr_menu->items[0].p();
}
#endif

//----------------------
// main->system->more->OSD MENU
//----------------------
//-------------------------------------
// osd MENU
//-------------------------------------
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/

void proc_osd_bg(void) 
{
	proc_menu_bg();
}
//void proc_osd_lang(void) {}

void proc_osd_time(void) 
{
	push_menu_stack_level();

	curr_menu = &menu_slider_page;  //BK160209
	curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->select = MENU_SLIDER_BAR_INPUT_RED;
    curr_menu_watchdog_select = curr_menu->select;
	//ref:menu_value_timeout = EE_Read(EEP_OSD_TIMEOUT);
	//BKTODO: Need callback functions
	InitSlideCtrl(&img_slide_timer, &OsdGetTime,&OsdVaryTime,&OsdSetTime,&OsdChangeTime); //BKTODO

	curr_menu->items[0].p();		
}
void proc_osd_trans(void) 
{
	push_menu_stack_level();

	curr_menu = &menu_slider_page;  //BK160209
	curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->select = MENU_SLIDER_BAR_INPUT_RED;
    curr_menu_watchdog_select = curr_menu->select;

	//ref:menu_value_transparency = EE_Read(EEP_OSD_TRANSPARENCY);
	//BKTODO: Need callback functions


	OsdSetTransparent(EE_Read(EEP_OSD_TRANSPARENCY));
	InitSlideCtrl(&img_slide_trans, &OsdGetTransparent,&OsdSetTransparent,&OsdSetTransparent,&OsdChangeTransparent);	  //BKTODO: init slide control

	curr_menu->items[0].p();		
}




 
//=========================
// PAGE MENU
//=========================
//=====================================
// MAIN PAGE
//=====================================
//-------------------------------------
// main MENU
//-------------------------------------
//desc: called from MenuStart()
//	draw background image, focus 1 intem if not use USE_TOUCH
//  use curr_menu.item
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void proc_main_bg(void)
{	
	proc_menu_bg();
}

//move main_page->input page
void proc_main_input(void)
{
	push_menu_stack_level();		//save current menu page
	curr_menu = &menu_input_page;	//move to main->input

	MenuInputMain = GetInputMain();
	switch(MenuInputMain) {
	case INPUT_CVBS:		curr_menu->focus = MENU_SRC_CVBS;		break;
	case INPUT_SVIDEO:		curr_menu->focus = MENU_SRC_SVIDEO;		break;
	case INPUT_COMP:		curr_menu->focus = MENU_SRC_YPBPR;		break;
	case INPUT_PC:			curr_menu->focus = MENU_SRC_PC;			break;
	case INPUT_LVDS:		curr_menu->focus = MENU_SRC_LVDS;		break;
	case INPUT_HDMIPC:		
	case INPUT_HDMITV:		curr_menu->focus = MENU_SRC_HDMI;		break;
	case INPUT_BT656:		curr_menu->focus = MENU_SRC_BT656;		break;
	default:				curr_menu->focus = MENU_SRC_CVBS;		break;
	}
    curr_menu_watchdog_focus = curr_menu->focus;
	curr_menu->select = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;

	curr_menu->items[0].p();		//draw background, select focus and draw the focused item.
}
//move main_page->audio_page
void proc_main_audio(void)
{
	push_menu_stack_level();
	curr_menu = &menu_audio_page; //BK160209
	curr_menu->focus = MENU_AUDIO_RETURN;
    curr_menu_watchdog_focus = curr_menu->focus;
	curr_menu->items[0].p();
}
//move main_page->system_page
void proc_main_system(void)
{
	push_menu_stack_level();		//save current menu page
	curr_menu = &menu_system_page;	//move to main->system   //BK160209

	curr_menu->focus = MENU_SYSTEM_DISPLAY; //start at display first
    curr_menu_watchdog_focus = curr_menu->focus;
	curr_menu->select = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;
	curr_menu->items[0].p();		//draw background and focused item
}
//move main_page->gps_page
void proc_main_gps(void)
{
	push_menu_stack_level();
	curr_menu = &menu_gps_page;  //BK160209
	curr_menu->focus = MENU_GPS_RETURN;
    curr_menu_watchdog_focus = curr_menu->focus;
	curr_menu->items[0].p();
}
//move main_page->phone_page
void proc_main_phone(void)
{
	push_menu_stack_level();
	curr_menu = &menu_phone_page;  //BK160209
	curr_menu->focus = MENU_PHONE_RETURN;
    curr_menu_watchdog_focus = curr_menu->focus;
	curr_menu->items[0].p();
}
//move main_page->carinfo_page
void proc_main_info(void)
{
	push_menu_stack_level();
	curr_menu = &menu_carinfo_page;    //BK160209
	curr_menu->focus = MENU_CARINFO_RETURN;
    curr_menu_watchdog_focus = curr_menu->focus;
	curr_menu->items[0].p();
}


//=====================================
// Video INPUT PAGE
//
// path:main->input
//=====================================

//----------------------
// video input background 
//----------------------
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void proc_input_bg(void)
{
#if 0
	BYTE hdmi_not_detected;	//BKTODO
	hdmi_not_detected = 1;
//	curr_menu->deactivated = 0;
#endif

	proc_menu_bg();

#if 0
	//if I have a deactived item
	if(hdmi_not_detected) {
		//let curr_menu know it.
		//BKTODO: You have to clear when you update curr_menu.
		curr_menu->deactivated = MENU_SRC_HDMI;
		//draw deactivated screen
		MenuDrawDeactivatedItemImage(SOSD_WIN7, MENU_SRC_HDMI);
	}
#endif
}
//select cvbs
void proc_input_cvbs(void)
{
	MenuInputMain = GetInputMain();
	if(MenuInputMain != INPUT_CVBS) {

		//change input
		SpiOsdEnable(OFF);	//-->27M
	
		//change input & update EE & Set Image.
		MenuNoSignal=ChangeCVBS();	//-->27M
	
		//??SpiOsdSetDeValue();
		//??FOsdSetDeValue();
	
		SpiOsdEnable(ON);	//-->108M
	}
	//draw the select Note.
	curr_menu->select = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;
	MenuDrawCurrImage(0,curr_menu->select+curr_menu->item_total);

	//move to return.
	//curr_menu->focus = MENU_SRC_RETURN;
    //curr_menu_watchdog_focus = curr_menu->focus;
	MenuDrawCurrImage(0,curr_menu->focus);
}
//select svideo
void proc_input_svideo(void)
{
	MenuInputMain = GetInputMain();
	if(MenuInputMain != INPUT_SVIDEO) {

		//change input
		SpiOsdEnable(OFF);
	
		//change input & update EE & Set Image.
		MenuNoSignal=ChangeSVIDEO();
	
		//??SpiOsdSetDeValue();
		//??FOsdSetDeValue();
	
		SpiOsdEnable(ON);
	}
	//draw the select Note.
	curr_menu->select = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;
	MenuDrawCurrImage(0,curr_menu->select+curr_menu->item_total);

	//curr_menu->focus = MENU_SRC_RETURN;
    //curr_menu_watchdog_focus = curr_menu->focus;
	MenuDrawCurrImage(0,curr_menu->focus);
}
//select component
void proc_input_ypbpr(void)
{
	MenuInputMain = GetInputMain();
	if(MenuInputMain != INPUT_COMP) {

		//change input
		SpiOsdEnable(OFF);
	
#ifdef SUPPORT_COMPONENT
		//change input & update EE & Set Image.
		MenuNoSignal=ChangeComponent();
#else
		MenuNoSignal=2;
#endif
	
		//??SpiOsdSetDeValue();
		//??FOsdSetDeValue();
	
		SpiOsdEnable(ON);
	}
	//draw the select Note
	curr_menu->select = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;
	MenuDrawCurrImage(0,curr_menu->select+curr_menu->item_total);

	//curr_menu->focus = MENU_SRC_RETURN;
    //curr_menu_watchdog_focus = curr_menu->focus;
	MenuDrawCurrImage(0,curr_menu->focus);
}
//select pc
void proc_input_pc(void)
{
	MenuInputMain = GetInputMain();
	if(MenuInputMain != INPUT_PC) {

		//change input
		SpiOsdEnable(OFF);
	
#ifdef SUPPORT_PC
		//change input & update EE & Set Image.
		MenuNoSignal=ChangePC();
#else
		MenuNoSignal= 2;		
#endif	
		//??SpiOsdSetDeValue();
		//??FOsdSetDeValue();
	
		SpiOsdEnable(ON);
	}
	//draw the select Note
	curr_menu->select = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;
	MenuDrawCurrImage(0,curr_menu->select+curr_menu->item_total);

	//curr_menu->focus = MENU_SRC_RETURN;
    //curr_menu_watchdog_focus = curr_menu->focus;
	MenuDrawCurrImage(0,curr_menu->focus);
}
//select dvi
//#ifdef SUPPORT_DVI
//void proc_input_dvi(void)
//{
//	MenuInputMain = GetInputMain();
//	if(MenuInputMain != INPUT_DVI) {
//
//		//change input
//		SpiOsdEnable(OFF);
//	
//		//change input & update EE & Set Image.
//		MenuNoSignal=ChangeDVI();
//	
//		//??SpiOsdSetDeValue();
//		//??FOsdSetDeValue();
//	
//		SpiOsdEnable(ON);
//	}
//	//draw the select Note
//	curr_menu->select = curr_menu->focus;
//  curr_menu_watchdog_select = curr_menu->select;
//	MenuDrawCurrImage(0,curr_menu->select+curr_menu->item_total);
//
//	//curr_menu->focus = MENU_SRC_RETURN;
//  //curr_menu_watchdog_focus = curr_menu->focus;
//	MenuDrawCurrImage(0,curr_menu->focus);
//}
//#endif

void proc_input_lvds(void)
{
	MenuInputMain = GetInputMain();
	if(MenuInputMain != INPUT_LVDS) {

		//change input
		SpiOsdEnable(OFF);
	
#ifdef SUPPORT_LVDSRX
		//change input & update EE & Set Image.
		MenuNoSignal=ChangeLVDSRx();
#endif	
		//??SpiOsdSetDeValue();
		//??FOsdSetDeValue();
	
		SpiOsdEnable(ON);
	}
	//draw the select Note
	curr_menu->select = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;
	MenuDrawCurrImage(0,curr_menu->select+curr_menu->item_total);

	//curr_menu->focus = MENU_SRC_RETURN;
	//curr_menu_watchdog_focus = curr_menu->focus;
    MenuDrawCurrImage(0,curr_menu->focus);
}


//select hdmi
void proc_input_hdmi(void)
{
	MenuInputMain = GetInputMain();
	if(MenuInputMain != INPUT_HDMIPC && MenuInputMain != INPUT_HDMITV) {

		//change input

		SpiOsdEnable(OFF);
	
		//change input & update EE & Set Image.
		MenuNoSignal=ChangeHDMI();
	
		//??SpiOsdSetDeValue();
		//??FOsdSetDeValue();
	
		SpiOsdEnable(ON);
	}
	//draw the select Note
	curr_menu->select = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;
	MenuDrawCurrImage(0,curr_menu->select+curr_menu->item_total);

	//curr_menu->focus = MENU_SRC_RETURN;
    //curr_menu_watchdog_focus = curr_menu->focus;
	MenuDrawCurrImage(0,curr_menu->focus);
}
//select BT656(ExternalCVBS)
void proc_input_bt656(void)
{
#ifdef SUPPORT_BT656_LOOP
	MenuInputMain = GetInputMain();
	if(MenuInputMain != INPUT_BT656) {
		//change input
		SpiOsdEnable(OFF);
	
		//BKTODO110511
		//DVI 8BIT mode
	 	MenuNoSignal=ChangeBT656Loop();
	
		//??SpiOsdSetDeValue();
		//??FOsdSetDeValue();
	
		SpiOsdEnable(ON);
	}
	//draw the select Note
	curr_menu->select = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;
	MenuDrawCurrImage(0,curr_menu->select+curr_menu->item_total);

	//curr_menu->focus = MENU_SRC_RETURN;
    //curr_menu_watchdog_focus = curr_menu->focus;
	MenuDrawCurrImage(0,curr_menu->focus);
#endif
}

//move to YUV(decoder)/aPC(pc)/RGB(dvi) image setting
void proc_input_setting(void)
{
	push_menu_stack_level();

	MenuInputMain = GetInputMain();
	if(MenuInputMain==INPUT_PC)
		curr_menu = &menu_apc_list_page;	//PC ( Analog RGB )  //BK160209
	else if(MenuInputMain==INPUT_DVI)
		curr_menu = &menu_rgb_list_page; 	//DVI  //BK160209
	else if(MenuInputMain==INPUT_HDMIPC || MenuInputMain==INPUT_HDMITV) 
		curr_menu = &menu_hdmi_list_page; 	//HDMI   //BK160209
	else 
		curr_menu = &menu_yuv_list_page;	//CVBS & SVideo	& YPBPR	& BT656     //BK160209

	curr_menu->focus = curr_menu->item_start;					//start from ...
	curr_menu_watchdog_focus = curr_menu->focus;

	curr_menu->items[0].p();				// call new background draw routine
}

//=====================================
// AUDIO PAGE
//
// path:main->audio
//=====================================
//----------------------
// audio page background
//----------------------
//-----------------------------------------------------------------------------
/**
* Function
*
* Description
* @param
* @return
*/
void proc_audio_bg(void) 
{
	proc_menu_bg();
}

//=====================================
// SYSTEM PAGE
//
// path:main->system
//=====================================
//----------------------
// system page background
//----------------------
void proc_system_bg(void)
{
	BYTE touch_not_detected;  //BKTODO
	touch_not_detected = 1;

	proc_menu_bg();

//	curr_menu->deactivated = 0;
	if(touch_not_detected) {
		//let curr_menu know it.
		//BKTODO: You have to clear when you update curr_menu.
//		curr_menu->deactivated = MENU_SETTING_TOUCH;
		//draw deactivated screen
		MenuDrawDeactivatedItemImage(SOSD_WIN7, MENU_SYSTEM_TOUCH);
	}
}
	
void proc_system_touch(void)
{
//ET1 = 0;
	curr_menu->select = MENU_SYSTEM_TOUCH;
    curr_menu_watchdog_select = curr_menu->select;
	push_menu_stack_level();

	//curr_menu.info = menu_touch_calibrate_page;
	curr_menu = &menu_touch_page;  //BK160209
	curr_menu->focus = curr_menu->item_start;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->items[0].p();
//ET1 = 1;
}

void proc_system_display(void)
{
	curr_menu->select = MENU_SYSTEM_DISPLAY;
    curr_menu_watchdog_select = curr_menu->select;
	push_menu_stack_level();

	curr_menu = &menu_display_page; //BK160209
	curr_menu->focus = curr_menu->item_start;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->items[0].p();
}	
void proc_system_btooth(void)
{
	curr_menu->select = MENU_SYSTEM_BTOOTH;
    curr_menu_watchdog_select = curr_menu->select;
	push_menu_stack_level();

	//we don't have btooth page, let's use phone page
	curr_menu = &menu_phone_page;   //BK160209
	curr_menu->focus = MENU_PHONE_RETURN;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->items[0].p();
}	
void proc_system_restore(void)
{
	curr_menu->select = MENU_SYSTEM_RESTORE;
    curr_menu_watchdog_select = curr_menu->select;
	push_menu_stack_level();

	curr_menu = &menu_restore_page;   //BK160209
	curr_menu->focus = MENU_RESTORE_OK;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->items[0].p();
}
void proc_system_info(void)
{
	BYTE str_buff[10];

	curr_menu->select = MENU_SYSTEM_SYSINFO;
    curr_menu_watchdog_select = curr_menu->select;
	push_menu_stack_level();

	curr_menu = &menu_sys_info_page;   //BK160209
	curr_menu->focus = MENU_SYS_INFO_OK;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->items[0].p();

	MenuFontOsdItemInit(0, ON);		//1BPP ALPHA ON
	MenuFontOsdItemInit(1, ON);		//1BPP ALPHA ON
#ifdef EVB_10
	MenuFontOsdItemString(0,"1.0");
#else
	MenuFontOsdItemString(0,"0.0");
#endif
	TWhtos((BYTE)(FWVER>>8),str_buff);
	TWstrcat(str_buff,".");
	TWhtos((BYTE)FWVER,menu_str_buff);
	TWstrcat(str_buff,menu_str_buff);
	MenuFontOsdItemString(1,str_buff);
}

#ifdef SUPPORT_TOUCH
BYTE MenuIsSystemPage(void)
{
	if(curr_menu == &menu_system_page)	return 1;  //BK160209
	return 0;
}
#endif

//=====================================
// GPS PAGE
//
// path:main->gps
//=====================================
//----------------------
// GPS page background
//----------------------
void proc_gps_bg(void) 
{
	proc_menu_bg();
}

//=====================================
// PHONE PAGE
//
// path:main->PHONE
//=====================================
//----------------------
// PHONE page background
//----------------------
BYTE phone_start;
void proc_phone_digit(BYTE ch, BYTE fCursor);
void proc_phone_cursor(BYTE start);
void proc_phone_string(BYTE *str);
void proc_phone_bg(void) 
{
	BYTE i,j;
	menu_fitem_info_t *text_info = curr_menu->texts;

	proc_menu_bg();
	MenuFontOsdItemInit(0, OFF);	//for FOSD WIN0
	MenuFontOsdItemInit(1, OFF);	//for FOSD WIN1

	proc_phone_string(" ");
	phone_start = 0;
	for(i=0; i < text_info->h; i++)
		for(j=0; j < text_info->w; j++)
			proc_phone_digit(0x20,0);
	phone_start = 0;
	proc_phone_cursor(phone_start);
}
void proc_phone_return(void)
{
	//turn off FontOSD
	FOsdWinOffAll();
	proc_return_icon();
}

void proc_phone_cursor(BYTE start)
{
	BYTE ch;
	menu_fitem_info_t *text_info = curr_menu->texts;

	ch = 0x5F;	//"_"
	FOsdBlinkOnOff(ON);
	FOsdRamSetAddrAttr(text_info->osdram+start,text_info->BPP1Color);
	WriteTW88(REG307, ch);
	FOsdBlinkOnOff(OFF);
}

void proc_phone_digit(BYTE ch, BYTE fCursor)
{
	BYTE palette;
	menu_fitem_info_t *text_info = curr_menu->texts;
	//text_info += index;


	if(IsDigit(ch)) {
		palette = text_info->BPP3_palette;
		FOsdRamSetAddrAttr(text_info->osdram+phone_start,palette >> 2);
		WriteTW88(REG307, (ch-0x30) * 3 + BPP3DIGIT_START);
		phone_start++;	
	}
	else {
		palette = text_info->BPP1Color;
		if(ch==0x0B) {	//backspace
			ch = 0x20;	//space
			FOsdRamSetAddrAttr(text_info->osdram+phone_start,text_info->BPP1Color);	//remove cursor
			WriteTW88(REG307, ch);
			if(phone_start)
				phone_start--;
			FOsdRamSetAddrAttr(text_info->osdram+phone_start,text_info->BPP1Color);	//remove char
			WriteTW88(REG307, ch);
		}
		else {
			FOsdRamSetAddrAttr(text_info->osdram+phone_start,text_info->BPP1Color);
			WriteTW88(REG307, ch);
			phone_start++;	
		}
	}
	if(fCursor)
		proc_phone_cursor(phone_start);


	if(phone_start >= 30)
		phone_start = 30;

}
void proc_phone_string(BYTE *str)
{
	BYTE palette;
	BYTE i,index;

	menu_fitem_info_t *text_info = curr_menu->texts;
	text_info++;	//FOSD WIN1
	
	palette = text_info->BPP1Color;
	FOsdRamSetAddrAttr(text_info->osdram,text_info->BPP1Color);
	index=0;
	while(*str) {
		WriteTW88(REG307, *str);
		str++;
		index++;
	}
	for(i=index; i < text_info->w; i++)
		WriteTW88(REG307, 0);	//space
}


void proc_phone_1(void) 
{
	proc_phone_digit('1',1);
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_2(void) 
{
	proc_phone_digit('2',1);
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_3(void) 
{
	proc_phone_digit('3',1);
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_4(void) 
{
	proc_phone_digit('4',1);
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_5(void) 
{
	proc_phone_digit('5',1);
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_6(void) 
{
	proc_phone_digit('6',1);
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_7(void) 
{
	proc_phone_digit('7',1);
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_8(void) 
{
	proc_phone_digit('8',1);
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_9(void) 
{
	proc_phone_digit('9',1);
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_star(void) 
{
	proc_phone_digit('*',1);
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_0(void) 
{
	proc_phone_digit('0',1);
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_sharp(void) 
{
	proc_phone_digit('#',1);
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_dial(void) 
{
	proc_phone_string("Dialing");
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_up(void) 
{
	proc_phone_string("up");
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_down(void) 
{
	proc_phone_string("down");
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_check(void) 
{
	proc_phone_digit(0x0B,1);	//backspace
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_left(void) 
{
	proc_phone_string("left");
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_right(void) 
{
	proc_phone_string("right");
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_help(void) 
{
	proc_phone_string("help");
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_dir(void) 
{
	proc_phone_string("Directory");
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_set(void) 
{
	proc_phone_string("Settings");
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_msg(void) 
{
	proc_phone_string("Messages");
	MenuDrawCurrImage(0, curr_menu->focus);	
}
void proc_phone_menu(void) 
{
	proc_phone_string("Menu");
	MenuDrawCurrImage(0, curr_menu->focus);	
}


//=====================================
// CARINFO PAGE
//
// path:main->carinfo
//=====================================
//----------------------
// carinfo page background
//----------------------
void proc_carinfo_bg(void) 
{
	proc_menu_bg();
}

//=====================================
// DEMO PAGE
//
// path:main->demo
//=====================================

//DEMO page background
void proc_demo_bg(void) 
{
	proc_menu_bg();
}

void proc_grid_demo_start(void) 
{
	WaitVBlank(1);
	SOsd_Enable(SOSD_WIN_ICON,OFF);
	SOsd_UpdateReg(SOSD_WIN_ICON, SOSD_WIN_ICON);

	//redraw setting again. I don't have a Animation menu.
	MenuEnd();	

	MovingGridInit();

	MovingGridTask_init();

	//block touch 
	CpuTouchSkipCount = 1*500*2;	//0.5 sec
}

#if 0
void proc_pigeon_demo_start(void)
{
	WaitVBlank(1);
	MenuEnd();
	PigeonDemo();		
}
#endif

void proc_rose_demo_start(void)
{
	WaitVBlank(1);
	MenuEnd();	

	SpiOsdWinHWOffAll(1);

	RoseDemo();

	//block touch 
	CpuTouchSkipCount = 1*500*2;	//0.5 sec
}
extern void DumpFontOsdPalette(void);
void proc_FPalette_demo_start(void)
{
	WaitVBlank(1);
	MenuEnd();	

	DumpFontOsdPalette();
}
extern void SOsdDemo1(void);
void proc_SOsd_demo1_start(void)
{
	WaitVBlank(1);
	MenuEnd();	

	SOsdDemo1();
}
extern void SOsdDemo2(void);
void proc_SOsd_demo2_start(void)
{
	WaitVBlank(1);
	MenuEnd();	

	SOsdDemo2();
}



//=========================
// WAIT
//=========================

//WaitIcon(ON/OFF)
void proc_wait_menu_start(void)
{
	menu_sitem_info_t *menu_item;
	struct image_item_info_s *image;
	menu_image_header_t *header = &header_table;
	BYTE item;
	WORD dy;  //we are using 10 animations.

	item = curr_menu->focus;
	menu_item = &curr_menu->items[item];
	image = &img_wait;	//menu_item->image;

	SpiOsdEnable(ON);

	SOsd_CleanReg();		//ClearSpiOsdWinBuffer();

	//prepare header
	MenuPrepareImageHeader(image);
	dy = header->dy / 10;  //we are using 10 animations.
	SOsd_SetSpiStartOffset( SOSD_WIN0, header->image_loc );
	SOsd_SetImageWidthHeight( SOSD_WIN0, header->dx, dy );
	//SOsd_SetScreen( SOSD_WIN0, 400 - (header->dx / 2), 240 - (dy /2), header->dx, dy );
	SOsd_SetScreen( SOSD_WIN0, (PANEL_H  - header->dx) / 2,  (PANEL_V - dy) /2, header->dx, dy );



	SOsd_SetWin0ImageOffsetXY( 0,0 );
	SOsd_SetWin0Animation( 2/*3*/, 0, 9, 3);
//	SOsd_SetWin0Priority(ON);

	//SOsd_SetGlobalAlpha( 0, 0 );
	SOsd_SetPixelAlpha( SOSD_WIN0, ON );
	SOsd_SetPixelWidth(SOSD_WIN0, header->bpp);
	SOsd_SetLutOffset(SOSD_WIN0,menu_item->lut);

	SOsd_Enable( SOSD_WIN0, ON );
	dMenuPuts("\n\rFinished wait icon window init");

	WaitVBlank(1);
	//SpiOsdLoadLUT(SOSD_WIN0, header->lut_type, menu_item->lut, header->lut_size, header->lut_loc,image->alpha);
	SOsd_SetLut(SOSD_WIN0, header->lut_type, menu_item->lut, header->lut_size, header->lut_loc,image->alpha);
	SOsd_UpdateLut(SOSD_WIN0, 1);	

	SOsd_UpdateReg(SOSD_WIN0, SOSD_WIN0);
	dMenuPuts("\n\rFinished Wait");
}

void proc_wait_demo_start(void)
{
	WaitVBlank(1); //BUG110725
	MenuEnd();
	proc_wait_menu_start();
}

//=====================================
// Touch PAGE
//
// path:main->system->touch
//=====================================

//----------------------
// main->system->touch MENU
//----------------------
//void UpdateTouchCalibXY(BYTE index,WORD x, WORD y);

void proc_touch_bg(void)
{
	proc_menu_bg();
}
void proc_touch_bg_end(void)
{
	pop_menu_stack_level();	
	curr_menu->items[0].p();  //redraw parent
}
//void proc_touch_bg_end(void)
//{
//	proc_menu_bg();
//}
void proc_touch_button1(void) 
{
//	ET1 = 0;
//	UpdateTouchCalibXY(0,x,y);

	curr_menu->focus++;
	curr_menu_watchdog_focus = curr_menu->focus;
    proc_menu_bg();
//	ET1 = 1;
}
void proc_touch_button2(void) 
{
//	ET1 = 0;
	curr_menu->focus++;
	curr_menu_watchdog_focus = curr_menu->focus;
    proc_menu_bg();
//	ET1 = 1;
}
void proc_touch_button3(void)
{
//	ET1 = 0;
	curr_menu->focus++;
	curr_menu_watchdog_focus = curr_menu->focus;
    proc_menu_bg();
//	ET1 = 1;
}

void proc_touch_button4(void)
{
//	ET1 = 0;
	curr_menu->focus++;
	curr_menu_watchdog_focus = curr_menu->focus;
    proc_menu_bg();
//	ET1 = 1;
}
void proc_touch_button5(void) 
{
	UseSOsdHwBuff = 1;
	//SOsdHwBuffClean();
	SOsd_CleanLut();	
	SOsd_CleanRlc();
	SOsd_CleanReg();
//	ET1 = 0;
//	pop_menu_stack_level();	
//	curr_menu->items[0].p();  //redraw parent
//	ET1 = 1;
	//draw item1
	MenuDrawCurrImage(0,1);	//normal, background_end
	//focus to OK
	curr_menu->focus = MENU_TOUCH_OK;
	curr_menu_watchdog_focus = curr_menu->focus;
    MenuDrawCurrImage(0,curr_menu->focus);			//draw focused item

	if(UseSOsdHwBuff) {
		SOsd_show(1);
		UseSOsdHwBuff = 0;
	}

}
#ifdef SUPPORT_TOUCH
void proc_touch_calib_end(void)
{
	//if(menu_level==0) {
	//	MenuEnd();
	//	return;
	//}
	
	pop_menu_stack_level();	
	curr_menu->items[0].p();  //redraw parent
}
#endif

void proc_touch_ok(void)
{
#ifdef SUPPORT_TOUCH
//	ET1 = 0;

	//save value to EEPROM
	//for(i=0; i < 5; i++) {
	//	w = TouchGetCalibedX(i);
	//	WriteEEPROM();
	//	w = TouchGetCalibedY(i);
	//	WriteEEPROM();
	//
	//}
	SaveCalibDataToEE(1);

//	UpdateTouchCalibXY(0,x,y);
//	UpdateTouchCalibXY(1,x,y);
//	UpdateTouchCalibXY(2,1,1 /*x,y*/);
//	UpdateTouchCalibXY(3,x,y);
//	UpdateTouchCalibXY(4,x,y);
	proc_touch_calib_end();
//	ET1 = 1;
#endif
}
void proc_touch_cancel(void)
{
#ifdef SUPPORT_TOUCH
//	BYTE i;
//	ET1 = 0;
	//readback calib value from EEPROM
	//for(i=0; i < 5; i++) {
	//	x = ReadEEPROM();
	//	y = ReadEEPROM();
	//	TouchSetCalibedXY(i, x,y);
	//}
	//ReadCalibDataFromEE();

	proc_touch_calib_end();
//	ET1 = 1;
#endif
}

#ifdef SUPPORT_TOUCH
BYTE MenuIsTouchCalibMode(void)
{
	if(curr_menu == &menu_touch_page) {  //BK160209
		if(curr_menu->focus >= curr_menu->item_start && curr_menu->focus < (curr_menu->item_start+5))
			return (curr_menu->focus - curr_menu->item_start +1);

	}
	return 0;
}
#endif

#ifdef SUPPORT_TOUCH
void MenuTouchCalibStart(void)
{
	//MenuStart();
	
	SaveCalibDataToEE(0);	//use default


	push_menu_stack_level();
	curr_menu = &menu_touch_page; //BK160209
	curr_menu->focus = curr_menu->item_start;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->select = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;

	//call bg proc
	curr_menu->items[0].p();
}
#endif

//=====================================
// Display PAGE
//
// path:main->system->display
//=====================================
//----------------------
// main->system->more MENU
//----------------------

void proc_display_bg(void) 
{
	proc_menu_bg();
}
void proc_display_aspect(void) 
{
	push_menu_stack_level();

	curr_menu = &menu_aspect_page;  //BK160209
	curr_menu->focus = MENU_ASPECT_NORMAL+GetAspectModeEE();
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->select = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;
	cancel_recover_value = curr_menu->select;
	curr_menu->items[0].p();
}

void proc_display_osd(void) 
{
	push_menu_stack_level();

	curr_menu = &menu_osd_page; //BK160209
	curr_menu->focus = curr_menu->item_start;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->items[0].p();
}
void proc_display_flip(void) 
{
	push_menu_stack_level();

	curr_menu = &menu_mirror_page; //BK160209
	curr_menu->focus = MENU_FLIP_OK;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->items[0].p();
}
void proc_display_backlight(void) 
{
	push_menu_stack_level();

	curr_menu = &menu_slider_page; //BK160209
	curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->select = MENU_SLIDER_BAR_INPUT_RED;
    curr_menu_watchdog_select = curr_menu->select;
	InitSlideCtrl(&img_slide_backlight, 
		&BackLightGetRate,
		&BackLightSetRate,
		&BackLightSetRate,
		&BackLightChangeRate);

	curr_menu->items[0].p();		
}

void proc_display_resolution(void) 
{
//	WORD wValue;
	BYTE str_buff[10];

	push_menu_stack_level();

	//BK110811
	//+---------------------------------------+
	//|               Resolution			  |
	//|										  |
	//|                INPUT     OUTPUT		  |
	//| Horizontal :   720       800          |
	//|   Vertical :   480		 480		  |
	//|  Frequency :   60					  |
	//|										  |
	//|                OK					  |
	//+---------------------------------------+
	//or
	//+---------------------------+
	//|         Resolution		  |
	//|							  |
	//| input :  720x480		  |
	//| panel :  800x480          |
	//|							  |
	//|         OK				  |
	//+---------------------------+

#if 0 //BK110811
	curr_menu = &menu_slider_page;  //BK160209
	curr_menu->focus = MENU_SLIDER_BAR_INPUT_RED;
	curr_menu_watchdog_focus = curr_menu->focus;
    curr_menu->select = MENU_SLIDER_BAR_INPUT_RED;
    curr_menu_watchdog_select = curr_menu->select;
	InitSlideCtrl(&img_slide_backlight, 
		&BackLightGetRate,
		&BackLightSetRate,
		&BackLightSetRate,
		&BackLightChangeRate);
#endif
	curr_menu = &menu_disp_resolution_page;   //BK160209
	curr_menu->focus = MENU_DISP_RESOLUTION_OK;
    curr_menu_watchdog_focus = curr_menu->focus;

	curr_menu->items[0].p();
	
	MenuFontOsdItemInit(0, ON);	//1BPP ALPHA ON
	MenuFontOsdItemInit(1, ON);	//1BPP ALPHA ON

	//add alpha here
	//FOsdWinAlphaPixel(0,1,15);
	//FOsdWinAlphaPixel(1,1,15);

	if(ReadTW88(REG21C) & 0x04) {
		//freerun manual, means no signal
		MenuFontOsdItemString(0,"N/A");
	}
	else {
#if 0
		wValue=InputGetHLen();
		TWitoa(wValue,str_buff);
		TWstrcat(str_buff,"x");
		wValue=InputGetVLen();
		TWitoa(wValue,menu_str_buff);
		TWstrcat(str_buff,menu_str_buff);
		MenuFontOsdItemString(0,str_buff);
#else
		MenuFontOsdItemString(0,FOsdMsgBuff);
		//FOsdCopyMsgBuff2Osdram(ON);
#endif
	}
	TWitoa(PANEL_H,str_buff);
	TWstrcat(str_buff,"x");
	TWitoa(PANEL_V,menu_str_buff);
	TWstrcat(str_buff,menu_str_buff);
	MenuFontOsdItemString(1,str_buff);
		
}



//----------------------
// main->system->more->OSD->language MENU
//----------------------
//-------------------------------------
// language popup MENU
//-------------------------------------

//void proc_lang_bg(void) {}
//void proc_lang_eng(void) {}
//void proc_lang_kor(void) {}
//void proc_lang_jpn(void) {}
//void proc_lang_chn(void) {}
//void proc_lang_ok(void) {}




//audio--------------
/*
one popup ok   - win 8
one popup bg   - win 7
one focus	   - win 6
one source	   - win 5
one tunner	   - win 4
one progress   - win 3
one volume	   - win 2
one bg		   - win 1
one animation  - win 0

fosd0		- title,artist...
fosd1		- progress
fosd2		- tunner

//gps-------------------
one bg
one map with full transparent
one campus
one focus


//phone-------------------
one bg
one focus
one volume
*/



//=============================================================================
// START / END
//=============================================================================

//-----------------------------------------------------------------------------
/**
* Function
*
* Description
*	Start Setup Menu
* @param 	void
* @return	void
*/
void MenuStart(void)
{
	dMenuPrintf("\n\rMenuStart");

	menu_on = 1;
	menu_lock_scroll = 0;
	UseSOsdHwBuff = 0;

	//turn off NoSignal Task if it was ON
	if(TaskNoSignal_getCmd() != TASK_CMD_DONE) {
		TaskNoSignal_setCmd(TASK_CMD_DONE); 
	}

	//turn off Logo if it was ON
	if(NoSignalLogoShow)
		RemoveLogo(); 

	//Init SPOOSD
	SOsd_init();

	//clean SOsdWinBuff[].
	SOsd_CleanReg();
	SOsd_CleanRlc();

	//init SpiOSD/FontOSD DE.
	SpiOsdSetDeValue();
	FOsdSetDeValue();

	//disable all FontOsd/SpiOsd HW Windows
	SpiOsdWinHWOffAll(0); 	//without wait

	//download font set & init Multi-BPP location
	FontOsdInit();

	//init Global variables
	menu_level = 0;

	//init SOSD_WIN0 here

	//Read EE values
	//example:
	//	MenuInputMain = GetInputMainEE();	//do not use, InputMain = GetInputEE(); in menu.c
	//	menu_value_osd_language = EE_Read(EEP_OSDLANG);
	//	menu_value_timeout = EE_Read(EEP_OSD_TIMEOUT);
 	//	menu_value_transparency = EE_Read(EEP_OSD_TRANSPARENCY);

	osd_lang = 0;	//read OSDLANG.

	//init SOSD
	SpiOsdEnable(ON);
	SOsd_UpdateRlc();
	SOsd_SetSpiStartBit(1,0);
	SOsd_SetLutOffset( 1, 0 /*SOSD_WIN_BG,  WINBG_LUTLOC*/ );  //old: SpiOsdLoadLUT_ptr
//	SpiOsdWinFillColor( 1, 0 );


	//link menu_main_info.
	curr_menu = &menu_main_page;  //BK160209
	curr_menu->focus = MENU_MAIN_VIDEO;
    curr_menu_watchdog_focus = curr_menu->focus;
	curr_menu->select = curr_menu->focus;
    curr_menu_watchdog_select = curr_menu->select;

	//call bg proc.
	curr_menu->items[0].p();
}

//-----------------------------------------------------------------------------
/**
* Description
*	End Setup Menu.
* @param 	void
* @return	void
*/
void MenuEnd(void)
{
	BYTE value;
	//
	//byebye
	//
	MenuPrintf("\n\rMenuEnd()");

	SpiOsdWinHWOffAll(1);	//with VB	
	SpiOsdEnable(OFF);
	FOsdWinOffAll();	

#ifdef USE_SFLASH_EEPROM
	//update all changed EEPROM value
	//clean EEPROM.
	E3P_Clean();
#endif

	menu_on = 0;
	menu_level = 0;

	MenuInputMain = GetInputMain();


	FontOsdInit(); //need it for Task message.
	FOsdSetDeValue();

	//prepare video info


	//turn On NoSignal Task if it is NoSignal
	//Pls, link with Task_Enable
	value = ReadTW88(REG004);
	if(value & 0x01) {
		FOsdIndexMsgPrint(FOSD_STR2_NOSIGNAL);
		TaskNoSignal_setCmd(TASK_CMD_WAIT_VIDEO);
	}
	else {
		if(MenuInputMain==INPUT_PC) {
			//if()	it is a out of range
			//	FOsdIndexMsgPrint(FOSD_STR3_OUTRANGE);
		}
	}

	//recover output active position.
	WriteTW88(REG201, ReadTW88(REG201) & 0xFD);

#if 1 //BK110707
	//do not change it here.
#else
	//turn ON ISR.
	switch(MenuInputMain) {
	case INPUT_CVBS:
	case INPUT_SVIDEO:
	case INPUT_COMP:
		WriteTW88(REG003, 0xFC );	// enable VDLOSS, SW
		break;
	case INPUT_PC:
	case INPUT_DVI:
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
	case INPUT_BT656:
		WriteTW88(REG003, 0xF8 );	// enable HSyncVSync, VDLOSS, SW
		break;
	default:
		WriteTW88(REG003, 0xFC );	// enable VDLOSS, SW
		break;
	}	
#endif
}


//reset menu_level & menu_on.
//caller has to clear SOSD
#ifdef SUPPORT_RCD
void MenuQuitMenu(void)
{
	menu_level=0;
	menu_on=0;
}
#endif

//====================================
// Slider Show Demo									   
//====================================

#define DEMO1024x600IMG_START	18
//#define DEMO1024x600IMG_NUM		16
#define DEMO1024x600IMG_NUM		15

menu_image_header_t back_header_table;
menu_image_header_t front_header_table;

/**
* description
*	set SpiOSD info for SliderShow
*/

void SliderShow_Set(BYTE win, struct image_item_info_s *image, WORD lut_loc)
{
	menu_image_header_t *header = &header_table;
	menu_image_header_t *SliderHeader;
	WORD sx,sy;

	sx = 0;
	sy = 0;
	lut_loc = 0;  //menu_item->lut. 0 or 256

	if(win==1)	SliderHeader = &back_header_table;
	else		SliderHeader = &front_header_table;

	//prepare header
	MenuPrepareImageHeader(image);
	SliderHeader->image_loc = header->image_loc;

	SOsd_SetSpiStartOffset( win, header->image_loc); 
	SOsd_SetImageWidthHeight( win, header->dx, header->dy );
	SOsd_SetScreen( win, sx, sy, header->dx, header->dy );
	if(win==0) {
		SOsd_SetWin0ImageOffsetXY( 0, 0 );
		SOsd_SetWin0Animation( 1, 0, 0, 0);
	}
	if(image->alpha != 0xFF)
		SOsd_SetPixelAlpha( win, ON );
	else
		SOsd_SetGlobalAlpha( win, 0);

	SOsd_SetPixelWidth(win, header->bpp);
	SOsd_SetLutOffset(win, lut_loc);

	SOsd_Enable( win, ON );

	//always use HwBuff.
	if(header->rle)
		SOsd_SetRlc(win,header->bpp,header->rle);
	SOsd_SetLut(win, header->lut_type, lut_loc,  header->lut_size, header->lut_loc, image->alpha);

	//pixel alpha blending. after load Palette
	//if(image->alpha != 0xFF)
	//	//SOsdHwBuffSetAlpha(win, lut_loc+image->alpha);
	//	SOsd_SetPixelAlphaIndex(win,image->alpha);
}
/**
* description
* 	show the slider show
*/
void SliderShow_Show(BYTE effect, BYTE direction)
{
	menu_image_header_t *header_back = &back_header_table;
	menu_image_header_t *header_front = &front_header_table;

	WORD i;
	BYTE win_back = 1;
	BYTE win_front = 3;
	WORD sx,sy;
	WORD dx,dy;
	DWORD dTemp;
	BYTE bTemp;

	bTemp =  effect;

	sx=sy=0;
	dx = 1024;
	dy = 600;

	if(effect==1) {
		//-----------------
		//overwrite
		if(direction == NAVI_KEY_RIGHT) {
			for(i=0; i <= 1024; i+=16) {
		
				//dTemp = back_image_loc;
				SOsd_SetSpiStartOffset(win_back, back_header_table.image_loc + i);
				SOsd_SetScreen(win_back, sx+i,sy, dx-i, dy);
			
				SOsd_SetSpiStartOffset(win_front, front_header_table.image_loc);
				SOsd_SetScreen(win_front, sx,sy, i, dy);
		
				WaitVBlank(1);
				SOsd_UpdateReg(win_back, win_front);
				
				delay1ms(10);
			}
		}
		else if(direction == NAVI_KEY_LEFT) {
			for(i=0; i <= 1024; i+=16) {
				
				SOsd_SetSpiStartOffset(win_back, back_header_table.image_loc);
				SOsd_SetScreen(win_back, sx,sy, dx-i, dy);
			
				SOsd_SetSpiStartOffset(win_front, front_header_table.image_loc + (1024-i));
				SOsd_SetScreen(win_front, sx + (1024 - i),sy, i, dy);
		
				WaitVBlank(1);
				SOsd_UpdateReg(win_back, win_front);
		
		
				delay1ms(10);
			}
		}
		else if(direction == NAVI_KEY_DOWN) {
			for(i=0; i <= 600; i+=10) {	
				dTemp = 1024;
				dTemp *= i;	
				SOsd_SetSpiStartOffset(win_back, back_header_table.image_loc + dTemp);
				SOsd_SetScreen(win_back, sx, sy+i, dx, dy-i);
			
				dTemp = 1024;
				dTemp *= (600-i);
				SOsd_SetSpiStartOffset(win_front, front_header_table.image_loc);
				SOsd_SetScreen(win_front, sx, sy, dx, i);
		
				WaitVBlank(1);
				SOsd_UpdateReg(win_back, win_front);
		
		
				delay1ms(10);
			}
		}
		else if(direction == NAVI_KEY_UP) {
			for(i=0; i <= 600; i+=10) {
		
				//dTemp = back_image_loc;
		
				SOsd_SetSpiStartOffset(win_back, back_header_table.image_loc);
				SOsd_SetScreen(win_back, sx, sy, dx, dy-i);
				dTemp = 1024;
				dTemp *= (600-i);
				SOsd_SetSpiStartOffset(win_front, front_header_table.image_loc + dTemp);
				SOsd_SetScreen(win_front, sx, 600 - i , dx, i);
		
				WaitVBlank(1);
				SOsd_UpdateReg(win_back, win_front);
		
		
				delay1ms(10);
			}
		}
	}
	else {
		//-----------------
		//shift
		if(direction == NAVI_KEY_RIGHT) {
			for(i=0; i <= 1024; i+=16) {
				SOsd_SetScreen(win_back, sx+i,sy, dx-i, dy);
			
				SOsd_SetSpiStartOffset(win_front, front_header_table.image_loc + (1024-i));
				SOsd_SetScreen(win_front, sx,sy, i, dy);
		
				WaitVBlank(1);
				SOsd_UpdateReg(win_back, win_front);
		
		
				delay1ms(10);
			}
		}
		else if(direction == NAVI_KEY_LEFT) {
			for(i=0; i <= 1024; i+=16) {		
				SOsd_SetSpiStartOffset(win_back, back_header_table.image_loc + i);
				SOsd_SetScreen(win_back, sx,sy, dx-i, dy);
			
				SOsd_SetSpiStartOffset(win_front, front_header_table.image_loc);
				SOsd_SetScreen(win_front, sx + (1024 - i),sy, i, dy);
		
				WaitVBlank(1);
				SOsd_UpdateReg(win_back, win_front);
		
		
				delay1ms(10);
			}
		}
		else if(direction == NAVI_KEY_DOWN) {
			for(i=0; i <= 600; i+=10) {		
				SOsd_SetSpiStartOffset(win_back, back_header_table.image_loc);
				SOsd_SetScreen(win_back, sx, sy+i, dx, dy-i);

				dTemp = 1024;
				dTemp *= (600-i);
				SOsd_SetSpiStartOffset(win_front, front_header_table.image_loc + dTemp);
				SOsd_SetScreen(win_front, sx, sy, dx, i);

				WaitVBlank(1);
				SOsd_UpdateReg(win_back, win_front);


				delay1ms(10);
			}
		}
		else if(direction == NAVI_KEY_UP) {
			for(i=0; i <= 600; i+=10) {
				dTemp = 1024;
				dTemp *= i;
				SOsd_SetSpiStartOffset(win_back, back_header_table.image_loc + dTemp);
				SOsd_SetScreen(win_back, sx, sy, dx, dy-i);

				SOsd_SetSpiStartOffset(win_front, front_header_table.image_loc);
				SOsd_SetScreen(win_front, sx, 600 - i , dx, i);
		
				WaitVBlank(1);
				SOsd_UpdateReg(win_back, win_front);
		
		
				delay1ms(10);
			}
		}
	}
}

/**
* description
* 	SpiOSD 1024x600 slider show demo 1.
*	draw the selected items
*/
void SOsdDemo1(void)
{
	BYTE winno = 1;
	BYTE lut = 0;
	BYTE i;
	BYTE win_back=1;
	struct image_item_info_s *image0;

//flush input
CpuTouchPressed = 0;
CpuTouchChanged = 0;


	SOsd_CleanReg();
	SpiOsdSetDeValue();
	FOsdSetDeValue();
	SpiOsdWinHWOffAll(0);

	//init SOSD
	SpiOsdEnable(ON);

	SOsd_CleanRlc();
	SOsd_UpdateRlc();
	SOsd_SetSpiStartBit(1,0);
	SOsd_SetLutOffset( 1, 0 );
	SOsd_CleanReg();
	SOsd_UpdateReg(0,8);

	for(i=0; ; i++) {
		i %= DEMO1024x600IMG_NUM;
		UseSOsdHwBuff = 1;

		SOsd_CleanLut();
		SOsd_CleanRlc();
		SOsd_CleanReg();
		//if(i==0) {
		//	SOsd_UpdateReg(0,8);
		//	SOsd_UpdateRlc();
		//}
		image0 = MonSOsdImg_Get_ImgeItem(DEMO1024x600IMG_START+i);
		SliderShow_Set(win_back, image0, 0);

		SOsd_show(1);
		UseSOsdHwBuff = 0;

		if(CheckHumanInputs(0))	//if (RS_ready())
			break;
		delay1ms(500);
		delay1ms(500);
		delay1ms(500);
		delay1ms(500);
		if(CheckHumanInputs(0))	//if (RS_ready())
			break;
	}
}

struct SOSD_DEMO2_TABLE_s {
	BYTE back_img;
	BYTE front_img;
	BYTE effect;		//0:move, 1:overwrite
	BYTE direction;
};
code struct SOSD_DEMO2_TABLE_s sosd_demo2_table[] = {
	{ 32,31, 0, NAVI_KEY_RIGHT},
	{ 31,32, 0, NAVI_KEY_LEFT},
	{ 32,31, 0, NAVI_KEY_DOWN},
	{ 31,32, 0, NAVI_KEY_UP},
	{ 32,31, 1, NAVI_KEY_RIGHT},
	{ 31,32, 1, NAVI_KEY_LEFT},
	{ 32,31, 1, NAVI_KEY_DOWN},
	{ 31,32, 1, NAVI_KEY_UP}
};


/**
* description
* SpiOSD 1024x600 slider show demo 2
*/
void SOsdDemo2(void)
{
	BYTE lut = 0;

	struct image_item_info_s *image0;
	struct image_item_info_s *image1;
	BYTE win_back=1;
	BYTE win_front=3;
	struct SOSD_DEMO2_TABLE_s *pDemoTable;
	BYTE i;

//flush input

	SOsd_CleanReg();
	SpiOsdSetDeValue();
	FOsdSetDeValue();
	SpiOsdWinHWOffAll(0);

	//init SOSD
	SpiOsdEnable(ON);
	SOsd_CleanRlc();
	SOsd_UpdateRlc();
	SOsd_SetSpiStartBit(1,0);
	SOsd_SetLutOffset( 1, 0 );
	SOsd_CleanReg();
	SOsd_UpdateReg(0,8);

	for(i=0; ; i++) {
		i &= 0x07;	//I have 8 methods
		pDemoTable = &sosd_demo2_table[i];	

		UseSOsdHwBuff = 1;

		SOsd_CleanLut();
		SOsd_CleanRlc();
		SOsd_CleanReg();
		//if(i==0) {
		//	SOsd_UpdateReg(0,8);
		//	SOsd_UpdateRlc();
		//}


		image0 = MonSOsdImg_Get_ImgeItem(pDemoTable->back_img);
		image1 = MonSOsdImg_Get_ImgeItem(pDemoTable->front_img);

		SliderShow_Set(win_back, image0, 0);
		SliderShow_Set(win_front,image1, 0);

		SOsd_SetScreen( win_front, 0, 0, 0, 600); //clear width.

		//draw first stage.
		SOsd_show(1);
		UseSOsdHwBuff = 0;

		//scroll
		SliderShow_Show(pDemoTable->effect, pDemoTable->direction);

		if(CheckHumanInputs(0))	//if (RS_ready())
			break;
		delay1ms(500);
		delay1ms(500);
		delay1ms(500);
		delay1ms(500);
		if(CheckHumanInputs(0))	//if (RS_ready())
			break;
	}
}


/**
* description
* 	SpiOSD 1280x800 from 0x100_0000 slider show demo 3.
*	draw the selected items
*/
#define DEMO1280x800IMG_NUM	5
#define DEMO1280x800IMG_START 34
void SOsdDemo3(void)
{
	BYTE winno = 1;
	BYTE lut = 0;
	BYTE i;
	BYTE win_back=1;
	struct image_item_info_s *image0;
	BYTE bTemp;

	SOsd_CleanReg();
	SpiOsdSetDeValue();
	FOsdSetDeValue();
	SpiOsdWinHWOffAll(0);

	//init SOSD
	SpiOsdEnable(ON);

	SOsd_CleanRlc();
	SOsd_UpdateRlc();
	SOsd_SetSpiStartBit(1,0);
	SOsd_SetLutOffset( 1, 0 );

	SOsd_CleanReg();
	SOsd_UpdateReg(0,8);

	//cheating...
	bTemp=ReadTW88(REG21C);
	WriteTW88(REG21C, bTemp | 0x07);
	ScalerWriteFreerunTotal(1758, 858);

	for(i=0; ; i++) {
		i %= DEMO1280x800IMG_NUM;
		UseSOsdHwBuff = 1;

		SOsd_CleanLut();
		SOsd_CleanRlc();
		SOsd_CleanReg();
		//if(i==0) {
		//	SOsd_UpdateReg(0,8);
		//	SOsd_UpdateRlc();
		//}
		image0 = MonSOsdImg_Get_ImgeItem(DEMO1280x800IMG_START+i);
		SliderShow_Set(win_back, image0, 0);

		SOsd_show(1);
		UseSOsdHwBuff = 0;

		if(CheckHumanInputs(0))	//if (RS_ready())
			break;
		delay1ms(500);
		delay1ms(500);
		delay1ms(500);
		delay1ms(500);
		if(CheckHumanInputs(0))	//if (RS_ready())
			break;
	}
	SOsd_Enable(win_back,0);
	SOsd_UpdateReg(win_back,win_back);
	WriteTW88(REG21C, bTemp);
}


#endif //..SUPPORT_SPIOSD


