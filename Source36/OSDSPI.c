/**
 * @file
 * OSDSPI.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2013 Intersil Corporation
 * @section DESCRIPTION
 *	low level SpiOSD layer
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
//								OSD.c
//
//*****************************************************************************
//
/*
	BandWidth = ((SPICLK / PCLKO) * Htotal - alpha ) / 2.
	alpha = (3+8+8+7+2)*3 / 2.
	      = 42

*/


//
#include "config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"
#include "Global.h"

#include "CPU.h"
#include "printf.h"
#include "Util.h"

#include "I2C.h"
#include "spi.h"

#include "SOsd.h"
#include "FOsd.h"
#include "SpiFlashMap.h"
#include "Settings.h"

void dummy_osdspi_code(void)
{
	Puts("dummy_osdspi_code");
}


//==========================================
//TW8836 SpiOSD Registers
//
//win#  addr    Group	description
//----  ----    -----	-----------
//win0: 0x420	A		animation
//win1: 0x440	B		background. low priority.
//win2: 0x430	B		background.	
//win3: 0x440   A
//...
//win8: 0x4B0	A		focused.    high priority.
//----------------------------------------
XDATA BYTE SpiWinBuff[10*0x10];


#ifdef SUPPORT_SPIOSD

struct SOsdLut_s {
	BYTE type;		//use flag (0x80) + LUTTYPE (BYTE:1, ADDR 0)	
	WORD offset;	//lut offset. if LUTTYPE_BYTE, MSB:(LutOffset >> 6),LSB:(LutOffset << 2)
	WORD size;		//lut size
	DWORD addr;		//address on SpiFlash.
	BYTE alpha;		//alpha_index. 0~0xFE. 0xFF measn the no-alpha.
}; /* 10 */
struct SOsdRle_s {
	BYTE win;
	BYTE bpp;
	BYTE count;
};

//=====================================================
// SpiOSD control buffer
//======================================================
struct SOSD_CTRL_s {
	BYTE XDATA *reg[9];
	struct SOsdLut_s lut[9]; /* 10*9 */
	struct SOsdRle_s rlc[2]; /* 3*2 */
} SOsdCtrl;


//TW8836 has 9 windows
code WORD	SpiOsdWinBase[9] = { SPI_WIN0_ST, 
		SPI_WIN1_ST, SPI_WIN2_ST, SPI_WIN3_ST, SPI_WIN4_ST,
		SPI_WIN5_ST, SPI_WIN6_ST, SPI_WIN7_ST, SPI_WIN8_ST
		};

//=============================================================================
//		OSD Window Functions
//=============================================================================
void SOsd_init(void)
{
	BYTE win;
	BYTE offset;

	for(win=0; win <= 8; win++) {
		if(win) offset = win+1;
		else	offset = 0;
		SOsdCtrl.reg[win] = &SpiWinBuff[offset << 4];
	}
}

#ifdef DEBUG_OSD
void SOsd_dump(void)
{
	DATA BYTE XDATA *data_p;
	struct SOsdLut_s *pLut;
	BYTE win;
	BYTE i;

	Printf("\n\rreg:");
	for(win=0; win <= 8; win++) {
		data_p = SOsdCtrl.reg[win];
		Printf("\n\r\t%bd:",win);
		for(i=0; i < 0x10; i++) {
			Printf("%02bx ",*data_p++);
		}
		if(win==0) {
			Printf("\n\r\t :");
			for(i=0; i < 0x10; i++) {
				Printf("%02bx ",*data_p++);
			}
		}
	}
	Printf("\n\rLut:");
	for(win=0; win <= 8; win++) {
		pLut = &SOsdCtrl.lut[win];
		Printf("\n\r\t%bd:",win);
		Printf("%02bx ",pLut->type);
		Printf("%03x ",pLut->offset);
		Printf("%04x ",pLut->size);
		Printf("%08lx ",pLut->addr);
	}	
	Printf("\n\rrlc:");
	Printf("\n\r\t%bd %bd %bd", SOsdCtrl.rlc[0].win,SOsdCtrl.rlc[0].bpp,SOsdCtrl.rlc[0].count);
	Printf("\n\r\t%bd %bd %bd", SOsdCtrl.rlc[1].win,SOsdCtrl.rlc[1].bpp,SOsdCtrl.rlc[1].count);
}
#endif


//=============================================================================
//		OSD Window Setup
//=============================================================================


//-----------------------------------------------------------------------------
// R40E[7:4]	OSD Linebuffer MSB



//-----------------------------------------------------------------------------
/**
* Description
*	Set SpiOsd DE value.
*
*	How to calculate DE value.
*	HDE = REG(0x210[7:0])
*	PCLKO = REG(0x20d[1:0]) {0,1,2,2}
*	PCLKO = REG(0x20d[1:0]) {1,1,1,1}  new
*	result = HDE + PCLKO - 17
* @param
* @return
*/
void SpiOsdSetDeValue(void)
{
	XDATA	WORD wTemp;
	BYTE hDE,pclko;

	hDE = ReadTW88(REG210);
	pclko = ReadTW88(REG20D) & 0x03;
	//if(pclko == 3)
	//	pclko = 2;
	//pclko = 1;
	pclko = 0;

	wTemp = (WORD)hDE + pclko - 18;

#if defined(PANEL_AUO_B133EW01) || defined(PANEL_1024X600)
	wTemp--;
#endif

	WriteTW88(REG40E, (BYTE)(wTemp>>8) );	// write SPI OSD DE value(high nibble)
	WriteTW88(REG40F, (BYTE)wTemp );   		// write SPI OSD DE value(low byte)
#ifdef DEBUG_OSD
	dPrintf("\n\rSpiOsdDe:%04x",wTemp);		
#endif
}


//-----------------------------------------------------------------------------
/**
* Description
*	Enable SpiOsd.
*	It is a HW function.
*
* 	If enable SpiOSD, HW uses CLKPLL.(normally use divider 1.5).
* 	If disable SpiOSD, HW uses 27MHz.
*
*	set I2C_delay_base depend on the MCU clock speed.
*
* On MODEL_TW8835_EXTI2C.
*	Do not toggle MCUSPI clock. Internal MCU can not make a synch.
*
* @param 	en
*	1: Enable SpiOSD, 0:Disable SpiOSD.
* @return	void
* 
* extern
*	I2C_delay_base	
*/
void SpiOsdEnable(BYTE en)
{
	BYTE dat;
	dat = ReadTW88(REG400);
	if( en ) {
		WriteTW88(REG400, dat | 0x04);						//enable SpiOSD
	}
	else {
		WriteTW88(REG400, dat & ~0x04);						//disable SpiOSD
	}
}

//==============================
// Windows Finctions
//==============================

//-----------------------------------------------------------------------------
/**
* Description
*	Enable SpiOsd Window. HW function.
* @param	winno
*	0 to 8.
* @param	en
*	1:Enable 0:disable
* @return	void
*/
/**
* 
*/
#if 0
void SpiOsdWinHWEnable(BYTE winno, BYTE en)
{
#if 0
	XDATA WORD index;
	XDATA	BYTE dat;

	index = SpiOsdWinBase[winno] + SPI_OSDWIN_ENABLE;

	dat = ReadTW88(index);
	if( en ) WriteTW88(index, dat | 0x01);
	else     WriteTW88(index, dat & 0xfe);
#endif
	//update buffer
	SOsd_Enable(winno,en);
	SOsd_UpdateReg(winno, winno);
}
#endif




//-----------------------------------------------------------------------------
/**
* Description
*
* @param
* @return
*/
//BYTE SpiOsdWinIsOn(BYTE winno){}


//-----------------------------------------------------------------------------
/**
* Description
*	enable SpiOSD window on WinBuff.
* @param
* @return
*/
//void OLD___SpiOsdWinBuffEnable(BYTE winno, BYTE en) {	}
void SOsd_Enable(BYTE win, BYTE fOn)
{
	XDATA BYTE  *p;
	p = SOsdCtrl.reg[win];
	if(fOn) *p |= 0x01;
	else    *p &= 0xfe;
}


//-----------------------------------------------------------------------------
/**
* Description
*	clear all SpiWinBuff[]
* @param	hw
*	1:update HW. 0:skip HW update.
* @return	void
*/
//void OLD___SOsdWinBuffClean(BYTE hw){}
void SOsd_CleanReg(void)
{
	BYTE i;
	for(i=0; i < 0xA0; i++)
		SpiWinBuff[i] =0;	
}
//void SOsd_CleanLutOffset(void)
//{
//	BYTE i;
//	for(i=0; i <= 8; i++)
//		SOsdCtrl.LutOffset[i] =0;	
//}
#if 0
void SOsdHw_Clean(void)
{
	SpiOsdWinHWOffAll(1);
 	SOsd_SetRlc(0,0,0);	//disable RLC
	SOsd_UpdateRlc();
}
//use 
//	SOsd_CleanReg()
//	SOsd_CleanRlc()
//
//	SOsd_UpdateReg()
//	SOsd_UpdateRlc()
//
#endif
//-----------------------------------------------------------------------------
/**
* Description
* 	write SpiWinBuff to HW registers
*
*	start address for ecah window
*
*	WIN			0	1	2	3	4	5	6	7	8
*   ----        --- --- --- --- --- --- --- --- ---
*	register	420 440 450 460 470 480 490 4A0 4B0
*
* @param  start: start window. between 0 to 8
* @param  end:   end window. between 0 to 8
*
* Note: this function needs a WaitVBlank(1).
*       Execute WaitVBlank(1) before you call this function.
*/
#pragma SAVE
#pragma OPTIMIZE(8,SPEED)
//void OLD___SOsdWinBuffWrite2Hw(BYTE start, BYTE end){}
void SOsd_UpdateReg(BYTE s_win, BYTE e_win)
{
	BYTE win;
	XDATA BYTE *data_p;
	DATA WORD reg_i;

#ifdef DEBUG_OSD
	dPrintf("\n\rSOsd_UpdateReg(%bd,%bd)",s_win,e_win);
	//NOTE: If you print the debug message, we have to check the vblank again.
	WaitVBlank(1);
#endif

	for(win=s_win; win <= e_win; win++) {
		data_p = SOsdCtrl.reg[win];	
		reg_i = SpiOsdWinBase[win];

		WriteTW88(reg_i++, *data_p++);	//0
		WriteTW88(reg_i++, *data_p++);	//1
		WriteTW88(reg_i++, *data_p++);	//2
		WriteTW88(reg_i++, *data_p++);	//3
		WriteTW88(reg_i++, *data_p++);	//4
		WriteTW88(reg_i++, *data_p++);	//5
		WriteTW88(reg_i++, *data_p++);	//6
		WriteTW88(reg_i++, *data_p++);	//7
		WriteTW88(reg_i++, *data_p++);	//8
		WriteTW88(reg_i++, *data_p++);	//9
		WriteTW88(reg_i++, *data_p++);	//A
		WriteTW88(reg_i++, *data_p++);	//B
		WriteTW88(reg_i++, *data_p++);	//C
		WriteTW88(reg_i++, *data_p++);	//D
		WriteTW88(reg_i++, *data_p++);	//E
		WriteTW88(reg_i++, *data_p++);	//F
		if(win==0) {
			WriteTW88(reg_i++, *data_p++);	//10  REG430
			WriteTW88(reg_i++, *data_p++);	//11
			WriteTW88(reg_i++, *data_p++);	//12
			WriteTW88(reg_i++, *data_p++);	//13
			WriteTW88(reg_i++, *data_p++);	//14
			WriteTW88(reg_i++, *data_p++);	//15
			WriteTW88(reg_i++, *data_p++);	//16 REG436
			WriteTW88(reg_i++, *data_p++);	//17 REG437
		}
	}
}
#pragma RESTORE


#if 0
//-----------------------------------------------------------------------------
/**
* Description
*	check win buff, if HW is enabled and buff is not, disable HW
* @param
* @return
*/
//void SpiOsdWinBuffSynchEnable(void)
//{
//}
#endif

//-----------------------------------------------------------------------------
/**
* Description
*	turn off all SpiOsd Window.
* @param
* @return
* @see SpiOsdWinHWEnable
*/
void SpiOsdWinHWOffAll(BYTE wait)
{
	BYTE win;

	if(wait)
		WaitVBlank(wait);

	SOsd_CleanRlc();
	for(win=0; win <= 8; win++)
		SOsd_Enable(win,OFF);

	SOsd_UpdateRlc();
	SOsd_UpdateReg(0, 8);
}
//void SOsdHw_OffAll(BYTE wait)
//{
//	if(wait)
//		WaitVBlank(wait);
//}

//-----------------------------------------------------------------------------
/**
* Description
* 	set image location
*
* @param	winno
*	SpiOsd Window number. 0..8
* @param	start:
* @return	void
*
* reg
*	WINx Image Location on SpiFlash	 
*/
//void OLD___SpiOsdWinImageLoc(BYTE winno, DWORD start)	{}
//TW8836 uses 4bit align registers.
void SOsd_SetSpiStartOffset(BYTE win, DWORD offset)
{
	DATA BYTE XDATA *data_p;

	data_p = SOsdCtrl.reg[win];	
	data_p += SPI_OSDWIN_BUFFERSTART;
	*data_p++ =  (BYTE)(offset>>20);	//+0x07
	*data_p++ =  (BYTE)(offset>>12);	//+0x08
	*data_p   =  (BYTE)(offset>>4);		//+0x09

	if(win==0) 	data_p += 0x0E;			//+0x17	 
	else		data_p += 6;			//+0x0F
	*data_p = (BYTE)offset & 0x0F;	 
}

//-----------------------------------------------------------------------------
/**
* Description
* 	set image bit location.
*
* @param	winno
*	from 1 to 8. win0 does not have a bit operation.
* @param	start
* @return
*
* reg
*	win0 win1       win2 
*	N/A  0x44A[7:6] 0x45A[7:6].,,,
*/
//void OLD___SpiOsdWinImageLocBit(BYTE winno, BYTE start){}
void SOsd_SetSpiStartBit(BYTE win, BYTE start)
{
	DATA BYTE XDATA *data_p;
	BYTE bTemp;

	if(win==0)
		//wrong. win0 do not have a bit operation.
		return;

	data_p = SOsdCtrl.reg[win];	
	data_p += SPI_OSDWIN_BUFFERSTART_BIT;
	bTemp = *data_p;
	bTemp &= 0x3F;
	bTemp |= (start << 6);
	*data_p = bTemp;
}


//-----------------------------------------------------------------------------
/**
* Description
*	set image buffer width
* @param	winno
* @param	w
*	width
* @return	void
*/
//static void OLD___SpiOsdWinImageSizeW(BYTE winno, WORD w) {}
#if 0
static void SOsd_SetImgWidth(BYTE win, WORD width)
{
	DATA BYTE XDATA *data_p;
	BYTE bTemp;

	data_p = SOsdCtrl.reg[win] + SPI_OSDWIN_DISPSIZE;	

 	bTemp = *data_p & 0xC0;
	*data_p++ = (BYTE)(width >> 8 | bTemp);		//+0x0A
	*data_p++ = (BYTE)width;					//+0x0B
}
#endif

//-----------------------------------------------------------------------------
/**
* Description
*	set image width and height
* @param	winno
* @param	w
*	width
* @param	h
*	height
* @return	void
*/
//void OLD___SpiOsdWinImageSizeWH (BYTE winno, WORD w, WORD h) {}
void SOsd_SetImageWidthHeight(BYTE win, WORD w, WORD h)
{
	DATA BYTE XDATA *data_p;
	BYTE bTemp;

	data_p = SOsdCtrl.reg[win] + SPI_OSDWIN_DISPSIZE;	
	if(win) {
		//WIN1to8 need only Width.
	 	bTemp = *data_p & 0xC0;
		*data_p++ = (BYTE)(w >> 8 | bTemp);		//+0x0A
		*data_p++ = (BYTE)w;					//+0x0B
	}
	else {
		bTemp = (BYTE)(h >> 8);
		bTemp <<= 4;
		bTemp |= (BYTE)(w >> 8);
		*data_p++ = bTemp; 		//42A
		*data_p++ = (BYTE)w;	//42B
		*data_p++ = (BYTE)h;	//42C
	}
}


//-----------------------------------------------------------------------------
/**
* Description
*	set window position and size
* @param
* @return
*/
//WINx Screen(win) Pos & Size
//void SpiOsdWinScreen(BYTE winno, WORD x, WORD y, WORD w, WORD h)
void SOsd_SetScreen(BYTE winno, WORD x, WORD y, WORD w, WORD h)
{
	DATA BYTE XDATA *data_p;
	BYTE value;

	data_p = SOsdCtrl.reg[winno];

	data_p += SPI_OSDWIN_SCREEN;
	value = (y >> 8);
	value <<= 4;
	value |= (x >> 8);
	*data_p++ = value;		//421	441...
	*data_p++ = (BYTE)x;	//422	442... 	
	*data_p++ = (BYTE)y;	//423	443...
	
	value = (h >> 8);
	value <<= 4;
	value |= (w >> 8);
	*data_p++ = value;		//424	444...
	*data_p++ = (BYTE)w;	//425	445...	 	
	*data_p++ = (BYTE)h;	//426	446...	 
}


//=============================================================================
//		Load LUT
//=============================================================================
//
//-----------------------------------------------------------------------------
/**
* Description
*	set Lut Offset
*	LUT offset use 5bit & 16 unit
* @param
* @return
*/
void SOsd_SetLutOffset( BYTE winno, WORD table_offset )
{
	DATA BYTE XDATA *data_p;

	data_p = SOsdCtrl.reg[winno];

	data_p += SPI_OSDWIN_LUT_PTR;
	if(!winno) data_p += 4;
	
	//LUT offset use 5bit & 16 unit
	*data_p = table_offset >> 4;
}

		


//=============================================================================
//		Pixel Width
//=============================================================================
//-----------------------------------------------------------------------------
/**
* Description
* 	set pixel width
*
* @param	winno
* @param	bpp
*	0:4bit, 1:6bit others:8bit.
*	7bpp uses 8bit.
* @return	void
*/
void SOsd_SetPixelWidth(BYTE winno, BYTE bpp)
{
	DATA BYTE XDATA *data_p;
	BYTE mode;

	if(bpp==4)		mode=0;
	else if(bpp==6) mode=1;
	else 			mode=2;

	data_p = SOsdCtrl.reg[winno];

	*data_p &= 0x3f;
	*data_p |= (mode <<6);
}
//-----------------------------------------------------------------------------
/**
* Description
* 	fill color
*	color will be an offset from the LUT location that Window have.
*	If window starts LUT from 80, the color value means color+80 indexed color.
* @param	winno
* @param	color
* @return
*/
/**
*/
#if 0
void SpiOsdWinFillColor( BYTE winno, BYTE color )
{
	WORD index;

	index = SpiOsdWinBase[winno];

	if ( color ) {
		WriteTW88(index, (ReadTW88(index ) | 0x04));				// en Alpha & Global
	}
	else {
		WriteTW88(index, (ReadTW88(index ) & 0xFB ) );				// dis Alpha & Global
	}
	index = SpiOsdWinBase[winno] + SPI_OSDWIN_FILLCOLOR;
	if(!winno)	index += 8;
	WriteTW88(index, color );
}
#endif

//-----------------------------------------------------------------------------
/**
* Description
* 	Enable Global Alpha and set the alpha value
* @param winno
* @param alpha
*	0 to 7F. 0x7F is a higest transparent value.
* @return
*/
/**
* set global alpha
*/
void SOsd_SetGlobalAlpha( BYTE winno, BYTE alpha )
{
	DATA BYTE XDATA *data_p;

	data_p = SOsdCtrl.reg[winno];

	*data_p &= 0xCF;
	if(alpha) *data_p |= 0x10;

	data_p += SPI_OSDWIN_ALPHA;
	if(!winno)	data_p += 4;
	*data_p = alpha;
}

//-----------------------------------------------------------------------------
/**
* Description
*	set Pixel alpha
* @param winno
* @param alpha
*	0 to 7F. 0x7F is a higest transparent value.
* @return
*/
void SOsd_SetPixelAlpha( BYTE winno, BYTE alpha )
{
	DATA BYTE XDATA *data_p;

	data_p = SOsdCtrl.reg[winno];

	if(alpha)	*data_p |= 0x30;
	else		*data_p &= 0xCF;

	data_p += SPI_OSDWIN_ALPHA;
	if(!winno)	data_p += 4;
	*data_p = alpha;
}



//-----------------------------------------------------------------------------
/**
* Description
*	adjust Win0 priority
* 	NOTE:Only fow Win0.
* @param
* @return
*/
#if 0
//TW8836. REG420[1] is for Zoom, not a priority
void SOsd_SetWin0Priority(BYTE high)
{
	DATA BYTE XDATA *data_p;

	data_p = SOsdCtrl.reg[0];

	if(high) *data_p |= 0x02;
	else   	 *data_p &= 0xfd;
}
#endif

//=============================================================================
//		Animation
//=============================================================================
//-----------------------------------------------------------------------------
/**
* Description
* 	set Animation.
*	Only for WIN0
* @param mode	
*	-0:display one time of the loop and then disappear
*	-1:display one time of the loop and then stay at the last frame
*	-2:Enable looping 
*	-3:static. Show the frame pointed by (0x431 and 0x432)
* @param Duration duration time of each frame (in unit of VSync)
*	- 0: infinite
*	- 1: One VSync period
*	- max 0xFF: 255 VSync period		
* @return
*/
void SOsd_SetWin0Animation(BYTE mode, BYTE FrameH, BYTE FrameV, BYTE Duration)
{
	DATA BYTE XDATA *data_p;

	data_p = SOsdCtrl.reg[0];
	data_p += SPI_OSDWIN_ANIMATION;

	*data_p++ = FrameH;
	*data_p++ = FrameV;
	*data_p++ = Duration;

	*data_p &= 0x3f;
	*data_p |= (mode << 6);
}

//-----------------------------------------------------------------------------
/**
* Description
*	set Win0 X,Y
* @param
* @return
*/
void SOsd_SetWin0ImageOffsetXY (WORD x, WORD y)
{
	BYTE value;
	DATA BYTE XDATA *data_p;

	data_p = SOsdCtrl.reg[0];			//Only WIN0
	data_p += SPI_OSDWIN_DISPOFFSET;

	value  = (BYTE)(y >> 8);
	value <<=4;
	value |= (BYTE)(x >> 8);
	*data_p++ = value;
	*data_p++ = (BYTE)x;
	*data_p++ = (BYTE)y;
}



//-----------------------------------------------------------------------------
/**
* Description
*	clear HwBuff.
* @param
* @return
*/
//void OLD___SOsdHwBuffClean(void){	}
void SOsd_CleanLut(void)
{
	struct SOsdLut_s *pLut;
	BYTE win;

	for(win=0; win <= 8; win++) {
		pLut = &SOsdCtrl.lut[win];
		pLut->type = 0;
		pLut->offset = 0;
		pLut->size = 0;
		pLut->addr = 0;
		pLut->alpha = 0xFF;	//disable alpha
	}
}
void SOsd_CleanRlc(void)
{
	SOsdCtrl.rlc[0].win = 0;  SOsdCtrl.rlc[0].bpp = 0; SOsdCtrl.rlc[0].count = 0;
	SOsdCtrl.rlc[1].win = 0;  SOsdCtrl.rlc[1].bpp = 0; SOsdCtrl.rlc[1].count = 0;
}

//-----------------------------------------------------------------------------
/**
* Description
*	set LUT info to HwBuff.
* @param	winno
* @param	LutOffset
*	destination LUT location
* @param	size
* @param	address
*	image location on SpiFlash
* @return
*/
//void OLD___SOsdHwBuffSetLut(BYTE win, /*BYTE type,*/  WORD LutOffset, WORD size, DWORD address)	{}

void SOsd_SetLut(BYTE win, BYTE type, WORD LutOffset, WORD size, DWORD address, BYTE alpha_index)
{
	struct SOsdLut_s *pLut;

	pLut = &SOsdCtrl.lut[win];

	pLut->type = 0x80 | type; //with USE flag
	pLut->offset = LutOffset;
	pLut->size = size;
	pLut->addr = address;
	pLut->alpha = alpha_index;

}
 
//-----------------------------------------------------------------------------
/**
* Description
*	set RLE info to HwBuff
* @param
* @return
*/
//void OLD___SOsdHwBuffSetRle(BYTE win, BYTE bpp, BYTE count)	{}
void SOsd_SetRlc(BYTE win, BYTE bpp, BYTE count)
{
	struct SOsdRle_s *pRlc;

	if(win==0) {
		Printf("\n\rPlease use SOsd_CleanRlc()");
		return;
	}

	if(win==1 || win==2) 	pRlc = &SOsdCtrl.rlc[0];	//background group
	else					pRlc = &SOsdCtrl.rlc[1];	//foreground group
	pRlc->win   = win;
	pRlc->bpp   = bpp;
	pRlc->count = count;
}

//-----------------------------------------------------------------------------
/**
* Description
*	set Alpha to HwBuff
* @param
* @return
*/
//void OLD___SOsdHwBuffSetAlpha(BYTE win, WORD alpha_index){}
void SOsd_SetPixelAlphaIndex(BYTE win, BYTE alpha_index)
{
	struct SOsdLut_s *pLut;

	pLut = &SOsdCtrl.lut[win];
	pLut->alpha = alpha_index;
}



/*
example: volatile & memory register access
volatile BYTE XDATA mm_dev_R1CD	_at_ 0xC1CD;	//use 1 XDATA BYTE
//#define TW8835_R1CD	(*((unsigned char volatile xdata *) (0xc000+0x1CD)))
#define TW8835_R1CD	(*((unsigned char volatile xdata *) (REG_START_ADDRESS+0x1CD) ))
void Dummy_Volatile_memory_register_test(void)
{
	volatile BYTE mode;
	volatile BYTE XDATA *p; // = (BYTE XDATA *)0xC1CD;

	mode = *(volatile BYTE XDATA*)(0xC1CD);

	p = (BYTE XDATA *)0xC1CD;
	mode = *p;

	mode = mm_dev_R1CD;

	mode = TW8835_R1CD;
}
*/

//-----------------------------------------------------------------------------
/**
* Description
*	write H2Buff to real HW
* @param
* @return
*/

//void OLD___SOsdHwBuffWrite2Hw(void){}


//see SpiOsdLoadLUT() and SpiOsdIoLoadLUT().
void SOsd_UpdateLut(BYTE win, BYTE fAlpha)
{
	struct SOsdLut_s *pLut;
	BYTE bTemp;
	WORD wTemp;
//	DWORD dTemp;
	WORD LutOffset;
	BYTE type;
#ifdef DEBUG_SPIFLASH_TEST
	volatile BYTE B0,B;
#endif



#ifdef DEBUG_OSD
	Printf("\n\rSOsd_UpdateLut(%bd,%bd)",win,fAlpha);
	SOsd_dump();
#endif

	pLut = &SOsdCtrl.lut[win];

	//check Use flag.
	type = pLut->type;
	if((type & 0x80)==0)
		return;
	type &= LUTTYPE_MASK;

	//if LUTTYPE_ADDR, use IO method.
	if(type==LUTTYPE_ADDR) {
		SpiOsdIoLoadLUT(win, type, pLut->offset, pLut->size, pLut->addr, pLut->alpha);
		return;
	}


	LutOffset = pLut->offset;

	//select PCLK domain
	//TW8836:divider3, TW8835:divider2.
#ifdef DEBUG_SPIFLASH_TEST
	//win_lut_debug = 0;
	if(LutOffset) {
		WriteTW88(REG410,0x80/*0xa0*/);	//read B
		WriteTW88(REG411,0);		//addr 0
		//read twice
		B0=ReadTW88(REG412);		
		B0=(volatile)ReadTW88(REG412);		
		//win_lut_debug = 1;
	}
#endif

	bTemp = SOSD_LUT_WEN;								 
	if(type==LUTTYPE_ADDR)		bTemp |= SOSD_LUT_INC_ADDR;		 						
	else						bTemp |= SOSD_LUT_INC_COLM;		// (TW8835 & TW8836 prefer this mode)
	if(win == 1 || win == 2) 	bTemp |= SOSD_LUT_BGRP;
	else if(LutOffset & 0x0F00)	bTemp |= SOSD_LUT_HIGH256;	
	WriteTW88(REG410, bTemp );
	WriteTW88(REG411, (BYTE)LutOffset ); 		// LUT addr.

	wTemp = pLut->offset;
	if(type != LUTTYPE_ADDR)
		wTemp <<= 2;
	SpiFlash_Read_SOsd(pLut->addr, wTemp, pLut->size);

#ifdef DEBUG_SPIFLASH_TEST
	if(LutOffset) {
		WriteTW88(REG410,0x80/*0xa0*/);	//read B
		WriteTW88(REG411,0);		//addr 0
		//read twice
		B=ReadTW88(REG412);		
		B=ReadTW88(REG412);	

		if(B0 != B) {
			WriteTW88(REG411, 0);			//addr 0
//BK110809			WriteTW88(REG412, B0);	//overwrite
			Printf("\n***BUGBUG*** B0 %bx->%bx",B, B0); //--pls, use without EA
		}
	}
#endif

	/*updata alpha..*/
	if(fAlpha) {
		bTemp = pLut->alpha;
		if(bTemp != 0xFF) {
			LutOffset += bTemp;
			bTemp = SOSD_LUT_WEN | SOSD_LUT_INC_NO | SOSD_LUT_ATTR;
			if(win==1 || win==2)	bTemp |= SOSD_LUT_BGRP;	
			else if(LutOffset >> 8)	bTemp |= SOSD_LUT_HIGH256;

			WriteTW88(REG410, bTemp);
			WriteTW88(REG411, (BYTE)LutOffset ); 		// alpha index
			WriteTW88(REG412, 0x7F/*value*/ ); 			// alpha value
		}
	}
}

void SOsd_UpdatePixelAlpha(BYTE win)
{
	struct SOsdLut_s *pLut;
	BYTE bTemp;
	WORD LutOffset;
	BYTE alpha_index;
	BYTE type;
	
	pLut = &SOsdCtrl.lut[win];
	type = pLut->type & LUTTYPE_MASK;
	alpha_index = pLut->alpha;
	//updata alpha..
	if(alpha_index != 0xFF) {
		LutOffset = pLut->offset;
		LutOffset += alpha_index;

		bTemp = SOSD_LUT_WEN | SOSD_LUT_INC_NO; 
		if(win==1 || win==2)	bTemp |= SOSD_LUT_BGRP;
		else if(LutOffset >> 8)	bTemp |= SOSD_LUT_HIGH256;	

		WriteTW88(REG410, bTemp);
		WriteTW88(REG411, (BYTE)LutOffset ); 		// alpha index
		WriteTW88(REG412, 0x7F/*value*/ ); 			// alpha value

	}
}

//update everything.

//SOsd_menu_show
// when menu changes the page,
//	if the previous page uses rlc,
//  FW will update GroupB first, 
void SOsd_show(BYTE fClean)
{
	BYTE bTemp;
	BYTE win;
	WORD LutOffset;
	BYTE type;
	WORD wTemp;
//	DWORD dTemp;
	struct SOsdLut_s *pLut;


#ifdef DEBUG_OSD
	dPuts("\n\rSOsd_show.....");
	SOsd_dump();
#endif


	WaitVBlank(1);	
	//-----------------------------
	// time cirtical section start.
	//-----------------------------
	
	//-----------------------
	//disable all
	if(fClean) {
		WriteTW88(REG420, ReadTW88(REG420) & ~0x01);
		for(win=1; win <= 8; win++) {
			wTemp = REG430 + (win << 4);
			WriteTW88(wTemp, ReadTW88(wTemp) & ~0x01);
		}
	}
	//-----------------------
	// update Group B.
	for(win=1;win<=2;win++) {
		pLut = &SOsdCtrl.lut[win];
		type = pLut->type;
		if((type & 0x80)==0)
			continue;
		type &= LUTTYPE_MASK;
		LutOffset = pLut->offset;

	 	bTemp = SOSD_LUT_WEN | SOSD_LUT_BGRP;								 
		if(type==LUTTYPE_ADDR)		bTemp |= SOSD_LUT_INC_ADDR; 						
		else						bTemp |= SOSD_LUT_INC_COLM;

		WriteTW88(REG410, bTemp );
		WriteTW88(REG411, (BYTE)LutOffset ); 		// LUT addr.

		wTemp = pLut->offset;
		if(type != LUTTYPE_ADDR)
			wTemp <<= 2;
		SpiFlash_Read_SOsd(pLut->addr, wTemp, pLut->size);

		//Internal MCU is stopped until it finishes DMA.
		//So, we do not need to read back the done flag
		//    we do not need a busy check also.
		//If you are using an external MCU
		// you can use below trick to check DONE. BusyWait
		//while(ReadTW88Page() != PAGE4_SPI);			

		/*updata alpha..*/
		bTemp = pLut->alpha;
		if(bTemp != 0xFF) {
			LutOffset += bTemp;
			bTemp = SOSD_LUT_WEN | SOSD_LUT_INC_NO | SOSD_LUT_BGRP | SOSD_LUT_ATTR;

			WriteTW88(REG410, bTemp);
			WriteTW88(REG411, (BYTE)LutOffset ); 		// alpha index
			WriteTW88(REG412, 0x7F); 					// alpha value
		}
	}
	SOsd_UpdateRlcB();
	SOsd_UpdateReg(1,2);

	WaitVBlank(1);	
	//-----------------------
	// update Group A.
	for(win=0;win<=8;win++) {
		if(win==1 || win==2)
			continue;
		pLut = &SOsdCtrl.lut[win];
		type = pLut->type;
		if((type & 0x80)==0)
			continue;
		type &= LUTTYPE_MASK;
		LutOffset = pLut->offset;

	 	bTemp = SOSD_LUT_WEN;								 
		if(type==LUTTYPE_ADDR)	bTemp |= SOSD_LUT_INC_ADDR;	 						
		else					bTemp |= SOSD_LUT_INC_COLM;	
		if(LutOffset & 0x0F00)	bTemp |= SOSD_LUT_HIGH256;
		WriteTW88(REG410, bTemp );
		WriteTW88(REG411, (BYTE)LutOffset ); 		// LUT addr.

		wTemp = pLut->offset;
		if(type != LUTTYPE_ADDR)
			wTemp <<= 2;
		SpiFlash_Read_SOsd(pLut->addr, wTemp, pLut->size);

		//Internal MCU is stopped until it finishes DMA.
		//So, we do not need to read back the done flag
		//    we do not need a busy check also.
		//If you are using an external MCU
		// you can use below trick to check DONE. BusyWait
		//while(ReadTW88Page() != PAGE4_SPI);			

		/*updata alpha..*/
		bTemp = pLut->alpha;
		if(bTemp != 0xFF) {
			LutOffset += bTemp;								
			bTemp = SOSD_LUT_WEN | SOSD_LUT_INC_NO | SOSD_LUT_ATTR ; 
			if(LutOffset >> 8)	bTemp |= SOSD_LUT_HIGH256;

			WriteTW88(REG410, bTemp);
			WriteTW88(REG411, (BYTE)LutOffset ); 		// alpha index
			WriteTW88(REG412, 0x7F/*value*/ ); 			// alpha value
		}
	}
	SOsd_UpdateReg(3,8);
	SOsd_UpdateReg(0,0);
	SOsd_UpdateRlcA();
	//-----------------------------
	// time cirtical section end
	//-----------------------------
}



//-----------------------------------------------------------------------------
/**
* Description
* 	set alpha attribute on LUT (A or B).
* @param
* @return
*/
//BKTODO: If you donot using alpha, disable alpha.
//void SpiOsdPixelAlphaAttr(BYTE win, WORD lutloc, BYTE value)
//{
//	BYTE bTemp;
//
//	bTemp = SOSD_LUT_WEN | SOSD_LUT_INC_NO | SOSD_LUT_ATTR ; 
//	if(win==1 || win==2)	bTemp |= SOSD_LUT_BGRP;	
//	else if(lutloc >> 8)	bTemp |= SOSD_LUT_HIGH256;
//  
//	WriteTW88(REG410, bTemp);
//	WriteTW88(REG411, (BYTE)lutloc ); 	// LUT addr
//	WriteTW88(REG412, value ); 			// LUT addr
//}



//-----------------------------------------------------------------------------
/**
* Description
* 	download LUT
*
* NOTE BK110330:after we support 512 palette, we donot support the address method.
* We need a width and a height info. but RTL only supports a size info.
* So, if you want to use the address method, use a PIO method.
*
* NOTE Do not add WaitVBlank() here.
*
* @param type	
*	- 1:Byte pointer - LUTS type. LUTTYPE_BYTE
*	- 0:Address pointer - LUT type. LUTTYPE_ADDR
*	if 0, use LutOffset:0, size:0x400.
* @param alpha
*	- 0xFF: no alpha. 
*	- other: use alpha value 0x7F. location is LutOffset+alpha.
* @see SpiOsdIoLoadLUT
* @see McuSpiClkToPclk
* @see McuSpiClkRestore
* @see SpiFlashDmaStart
*/
//void OLD___SpiOsdLoadLUT(BYTE _winno, BYTE _type, WORD LutOffset, WORD size, DWORD address, BYTE alpha)	{}


//-----------------------------------
// RLC(RunLengthCompress) functions
//
//-----------------------------------

//-----------------------------------------------------------------------------
/**
* Description
* 	set RLC register
*
* @param winno win number.
*		winno 0 means disable.
* @param dcnt Data BPP
*	4:4bit, 6:6bit, others:8bit
*	7 means 8BPP with 128 color.	
* @param ccnt counter value.
*	- 4:4bit,5:5bit,..15:16bit, others:16bit
*/
//void OLD___SpiOsdRlcReg(BYTE winno,BYTE dcnt, BYTE ccnt) {}
void SOsd_UpdateRlc(void)
{
	BYTE i;
	BYTE bTemp;
	struct SOsdRle_s *pRlc;

	for(i=0; i < 2; i++) {
		pRlc = &SOsdCtrl.rlc[i];
		WriteTW88(REG404 + 2 - i*2, pRlc->win << 4);
		bTemp = pRlc->bpp;
		if(bTemp==7)	bTemp++;
		bTemp <<= 4;
		bTemp |= pRlc->count;
		WriteTW88(REG405 + 2 - i*2, bTemp);
	}
}
void SOsd_UpdateRlcB(void)
{
	BYTE bTemp;
	struct SOsdRle_s *pRlc;

	pRlc = &SOsdCtrl.rlc[0];
	WriteTW88(REG406, pRlc->win << 4);
	bTemp = pRlc->bpp;
	if(bTemp==7)	bTemp++;
	bTemp <<= 4;
	bTemp |= pRlc->count;
	WriteTW88(REG407, bTemp);
}
void SOsd_UpdateRlcA(void)
{
	BYTE bTemp;
	struct SOsdRle_s *pRlc;

	pRlc = &SOsdCtrl.rlc[1];
	WriteTW88(REG404, pRlc->win << 4);
	bTemp = pRlc->bpp;
	if(bTemp==7)	bTemp++;
	bTemp <<= 4;
	bTemp |= pRlc->count;
	WriteTW88(REG405, bTemp);
}


//-----------------------------------------------------------------------------
/**
* Description
* 	disable RLC	Register
* @param winno
* 	win0 means disable both RLE_A and RLE_B.
* @return
*/
//void OLD___SpiOsdDisableRlcReg(BYTE winno){	}



#endif //..SUPPORT_SPIOSD


#ifdef SUPPORT_SPIOSD
void SpiOsdIoLoadLUT(BYTE _winno, BYTE type, WORD LutOffset, WORD size, DWORD spiaddr, BYTE alpha)
{
	BYTE i,j,k;
	BYTE bTemp;
#ifdef DEBUG_OSD
	dPrintf("\n\rSpiOsdIoLoadLUT%s win:%bd, LutLoc:%d size:%d 0x%06lx", type ? "S":" ", _winno, LutOffset, size, spiaddr);
#endif

	//--- SPI-OSD config
	bTemp = SOSD_LUT_WEN;
	if(type==LUTTYPE_ADDR)		bTemp |= SOSD_LUT_INC_ADDR;
	else						bTemp |= SOSD_LUT_INC_COLM;	
	if(_winno==1 || _winno==2)	bTemp |= SOSD_LUT_BGRP;		
	else if(LutOffset >> 8)		bTemp |= SOSD_LUT_HIGH256; 


	if(type==LUTTYPE_ADDR) {
		//
		//ignore size. it is always 0x400.(256*4)
		//		
		for(i=0; i < 4; i++) {	 
			WriteTW88(REG410, bTemp | i );		//assign byte ptr	
			WriteTW88(REG411, (BYTE)LutOffset);	//reset address ptr.
			for(j=0; j<(256/64);j++) {
				SpiFlash_Read_XMem(spiaddr + i*256 + j*64, (WORD)SPI_Buffer, 64);
				//SpiFlashDmaRead2XMem(SPI_Buffer,spiaddr + i*256 + j*64,64, SPIFLASH_WAIT_READ);
				for(k=0; k < 64; k++)
					WriteTW88(REG412, SPI_Buffer[k]);		//write data
			}
		}
	}
	else {
		WriteTW88(REG410, bTemp);			
		WriteTW88(REG411, (BYTE)LutOffset);		//reset address ptr.

		for(i=0; i < (size / 64); i++ ) {		//min size is a 64(16*4)
			SpiFlash_Read_XMem(spiaddr + i*64, (WORD)SPI_Buffer, 64);
			//SpiFlashDmaRead2XMem(SPI_Buffer,spiaddr + i*64,64, SPIFLASH_WAIT_READ);
			for(k=0; k < 64; k++)
				WriteTW88(REG412, SPI_Buffer[k]);		//write data
		}
	}
	//pixel alpha
	if(alpha!=0xFF) {
		LutOffset += alpha;

		bTemp = SOSD_LUT_WEN | SOSD_LUT_INC_NO | SOSD_LUT_ATTR ; 
		if(_winno==1 || _winno==2)	bTemp |= SOSD_LUT_BGRP;	
		else if(LutOffset >> 8)		bTemp |= SOSD_LUT_HIGH256;
	
		WriteTW88(REG410, bTemp);
		WriteTW88(REG411, (BYTE)LutOffset );
		WriteTW88(REG412, 0x7F );
	}
}
#endif

/**
* description
*	read SpiOSD LUT and compare it with SpiFlash Data.
* winno: LUT A or LUT B
* type : LUT or LUTS
* offset: lut location
* size : lut size
* addr : address at SpiFlash
*
*/
#ifdef SUPPORT_SPIOSD
BYTE SpiOsdCheckLut(BYTE win, BYTE type, WORD offset, WORD size, DWORD addr)
{
	DWORD spiaddr;
	WORD lut_offset;
	BYTE r410;
	volatile BYTE rdata0, rdata1;
	BYTE errno;
	WORD nRead;
	BYTE i;
	BYTE bgra;

	ePrintf("\n\rSpiOsdCheckLut(%bd, %bd, %d, %d, 0x%lx)", win, type, offset, size, addr);
	spiaddr = addr;
	lut_offset = offset;

	errno = 0;
	while(size) {
		nRead = 64;	//min. color is 16.
		SpiFlash_Read_XMem(spiaddr, (WORD)SPI_Buffer, nRead); 
		//SpiFlashDmaRead2XMem(SPI_Buffer,spiaddr,nRead, SPIFLASH_WAIT_READ);

		for(i=0; i < (nRead >> 2); i++) {
			r410 = SOSD_LUT_WEN;
			if(win==1 || win==2)	r410 |= SOSD_LUT_BGRP;
			if(lut_offset >> 8)		r410 |= SOSD_LUT_HIGH256;

			for(bgra=0; bgra < 4; bgra++) {
				r410 &= 0xFC;
				r410 |= bgra; 
				WriteTW88(REG410, r410);
				WriteTW88(REG411, (BYTE)lut_offset);
	
				rdata0 = ReadTW88(REG412);	
				rdata1 = ReadTW88(REG412);	//read twice
				if(rdata1 != SPI_Buffer[i*4+bgra]) {
					delay1ms(100);
					rdata0 = ReadTW88(REG412);
					errno++;
					Printf("\n\r%04x",lut_offset);
					Printf("+%bx ", bgra);
					Printf("%02bx->%02bx,%02bx", SPI_Buffer[i*4+bgra], rdata1, rdata0 );
				}
			}
			lut_offset++;
		}
		if(errno)
			break;
		if(size >= nRead)	size -= nRead;
		else				size = 0;
		spiaddr += nRead;
	}
	if(errno==0) {
		ePrintf("\n\rPass."); 
		ePrintf("%04x",lut_offset);
	}
	return errno;
}
#endif //..SUPPORT_SPIOSD





