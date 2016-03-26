/**
 * @file
 * DemoMain.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	Demo
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
//								Monitor_OSD.c
//
//*****************************************************************************
//
//
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
#include "Demo.h"
#include "Remo.h"
#include "InputCtrl.h"

#include "SOsd.h"
#include "FOsd.h"
#include "SpiFlashMap.h"


//#include "SOsdMenu.h"

BYTE	OsdDemoMode = 0;


XDATA BYTE Task_Grid_dt;
XDATA BYTE Task_Grid_lut;
XDATA BYTE Task_Grid_direction;
XDATA BYTE Task_Grid_n;
XDATA WORD Task_Grid_timer;
//XDATA BYTE Task_Grid_on;
extern void TaskSetGrid(BYTE onoff);
extern BYTE TaskGetGrid(void);
extern void TaskSetGridCmd(BYTE cmd);	
extern BYTE TaskGetGridCmd(void);



#define offsetd_grid DGRID_IMG_START //offset address for SPI writing
 
// typedef struct {    
//     DWORD start;    
//     DWORD length;   
// } SLIDEIMAGE;
#if 0
FAR CONST SLIDEIMAGE d_grid_IMG[] = { 
    { offsetd_grid+0x000000, 0x002AF0},    // Grid01 
    { offsetd_grid+0x002AF0, 0x00294B},    // Grid02 
    { offsetd_grid+0x00543B, 0x002934},    // Grid03 
    { offsetd_grid+0x007D6F, 0x002D1C},    // Grid04 
    { offsetd_grid+0x00AA8B, 0x003010},    // Grid05 
    { offsetd_grid+0x00DA9B, 0x00318F},    // Grid06 
    { offsetd_grid+0x010C2A, 0x00323F},    // Grid07 
    { offsetd_grid+0x013E69, 0x0032D4},    // Grid08 
    { offsetd_grid+0x01713D, 0x00339D},    // Grid09 
    { offsetd_grid+0x01A4DA, 0x00340C},    // Grid10 
    { offsetd_grid+0x01D8E6, 0x00347B},    // Grid11 
    { offsetd_grid+0x020D61, 0x00294C},    // Grid12 
    { offsetd_grid+0x0236AD, 0x002935},    // Grid13 
    { offsetd_grid+0x025FE2, 0x002D1D},    // Grid14 
    { offsetd_grid+0x028CFF, 0x003010},    // Grid15 
    { offsetd_grid+0x02BD0F, 0x00318F},    // Grid16 
    { offsetd_grid+0x02EE9E, 0x00323F},    // Grid17 
    { offsetd_grid+0x0320DD, 0x0032D4},    // Grid18 
    { offsetd_grid+0x0353B1, 0x00339D},    // Grid19 
    { offsetd_grid+0x03874E, 0x003408},    // Grid20 
    { offsetd_grid+0x03BB56, 0x00347B},    // Grid21 
};
#else 
//16 bytes aligned
FAR CONST SLIDEIMAGE d_grid_IMG[] = { 
    { offsetd_grid+0x000000, 0x002AF0},    // Grid01 
    { offsetd_grid+0x002af0, 0x00294B},    // Grid02 
    { offsetd_grid+0x005440, 0x002934},    // Grid03 
    { offsetd_grid+0x007d80, 0x002D1C},    // Grid04 
    { offsetd_grid+0x00aaa0, 0x003010},    // Grid05 
    { offsetd_grid+0x00dab0, 0x00318F},    // Grid06 
    { offsetd_grid+0x010c40, 0x00323F},    // Grid07 
    { offsetd_grid+0x013e80, 0x0032D4},    // Grid08 
    { offsetd_grid+0x017160, 0x00339D},    // Grid09 
    { offsetd_grid+0x01a500, 0x00340C},    // Grid10 
    { offsetd_grid+0x01d910, 0x00347B},    // Grid11 
    { offsetd_grid+0x020d90, 0x00294C},    // Grid12 
    { offsetd_grid+0x0236e0, 0x002935},    // Grid13 
    { offsetd_grid+0x026020, 0x002D1D},    // Grid14 
    { offsetd_grid+0x028d40, 0x003010},    // Grid15 
    { offsetd_grid+0x02bd50, 0x00318F},    // Grid16 
    { offsetd_grid+0x02eee0, 0x00323F},    // Grid17 
    { offsetd_grid+0x032120, 0x0032D4},    // Grid18 
    { offsetd_grid+0x035400, 0x00339D},    // Grid19 
    { offsetd_grid+0x0387a0, 0x003408},    // Grid20 
    { offsetd_grid+0x03bbb0, 0x00347B},    // Grid21 
};
#endif

#if 0
 0:0x000000  0x000000:0x002af0
 1:0x002af0  0x002af0:0x00294b padd:5
 2:0x005440  0x00543b:0x002934 padd:c
 3:0x007d80  0x007d6f:0x002d1c padd:4
 4:0x00aaa0  0x00aa8b:0x003010 
 5:0x00dab0  0x00da9b:0x00318f padd:1
 6:0x010c40  0x010c2a:0x00323f padd:1
 7:0x013e80  0x013e69:0x0032d4 padd:c
 8:0x017160  0x01713d:0x00339d padd:3
 9:0x01a500  0x01a4da:0x00340c padd:4
10:0x01d910  0x01d8e6:0x00347b  padd:5
11:0x020d90  0x020d61:0x00294c  padd:4
12:0x0236e0  0x0236ad:0x002935  padd:b
13:0x026020  0x025fe2:0x002d1d  padd:3
14:0x028d40  0x028cff:0x003010 
15:0x02bd50  0x02bd0f:0x00318f  padd:1
16:0x02eee0  0x02ee9e:0x00323f  padd:1
17:0x032120  0x0320dd:0x0032d4  padd:c
18:0x035400  0x0353b1:0x00339d  padd:3
19:0x0387a0  0x03874e:0x003408  padd:8
20:0x03bbb0  0x03bb56:0x00347b  padd:5
#endif







FAR CONST BYTE d_grid_rle[] = {
    0x89,	// Grid01
    0x89,	// Grid02
    0x89,	// Grid03
    0x89,	// Grid04
    0x89,	// Grid05
    0x89,	// Grid06
    0x89,	// Grid07
    0x89,	// Grid08
    0x89,	// Grid09
    0x88,	// Grid10
    0x88,	// Grid11
    0x89,	// Grid12
    0x89,	// Grid13
    0x89,	// Grid14
    0x89,	// Grid15
    0x89,	// Grid16
    0x89,	// Grid17
    0x89,	// Grid18
    0x89,	// Grid19
    0x88,	// Grid20
    0x88,	// Grid21
};


//=============================================================================
//				   void FontDMA( void )
//=============================================================================
#if 0 //move to OSDFont
void FontDMA( void ) {}
#endif

//=============================================================================
//				   void FontDisplay( void )
//=============================================================================
//Desc: Display font set. from 0x00 to (0xC0-1)
#if 0
void FontDisplay( void )
{...}
#endif


//=============================================================================
//		void FontDemo( void )
//=============================================================================
#if 0
void FontDemo( void )
{...}
#endif

//=============================================================================
//		void RoseDemo( void )
//=============================================================================
#if 1
void PigeonDemo( void )
{
	SOsd_CleanReg();		//ClearSpiOsdWinBuffer();

	//WindowRoseInit();
	// window 0 is pigeon demo
	SOsd_SetSpiStartOffset( 0, PIGEON_START );
	SOsd_SetImageWidthHeight( 0, 120, 140 );
	SOsd_SetWin0ImageOffsetXY( 0, 0 );
#if (PANEL_H==1024 && PANEL_V==600)	//sorry! still I have a small image. 
	SOsd_SetScreen( 0, SOSD_X_START+112, SOSD_Y_START+0, 120, 140 );
#else
	SOsd_SetScreen( 0, SOSD_X_START+0, SOSD_Y_START+0, 120, 140 );
#endif
	SOsd_SetWin0Animation( 2/*3*/, 0, 7, 3);

	SOsd_SetPixelWidth(0, 8);

	SOsd_SetGlobalAlpha( 0, 0 );
	SOsd_Enable( 0, ON );
	dPuts("\n\rFinished Pigeon window init");
	SOsd_SetLutOffset(0,0);

	WaitVBlank(1);
	//SpiOsdLoadLUT( 0, 0/*1*/, 0,0x400, PIGEON_LUT_LOC, 0xFF );	//win0
	SOsd_SetLut(0, 0/*1*/, 0,0x400, PIGEON_LUT_LOC, 0xFF);
	SOsd_UpdateLut(0, 0);	


	SOsd_UpdateReg(0, 0);
	dPuts("\n\rFinished LUTRoseDMA");
}
#endif
#define ROSE_WIN	0
void RoseDemo( void )
{
	SOsd_init();


	SOsd_CleanReg();
	SOsd_CleanLut();	
	SOsd_CleanRlc();

	SpiOsdEnable(ON);
	SOsd_UpdateRlc();
	SOsd_UpdateReg(0,8);


	SOsd_SetSpiStartOffset( ROSE_WIN, ROSE_START );
	SOsd_SetImageWidthHeight( ROSE_WIN, 400, 400 );
	SOsd_SetWin0ImageOffsetXY( ROSE_WIN, 0 );
#if (PANEL_H==1024 && PANEL_V==600)	//sorry! still I have a small image. 
	SOsd_SetScreen( ROSE_WIN, SOSD_X_START+200+112, SOSD_Y_START+40, 400, 400 );
#else
	SOsd_SetScreen( ROSE_WIN, SOSD_X_START+200, SOSD_Y_START+40, 400, 400 );
#endif
	SOsd_SetWin0Animation( 2/*3*/, 0, 9, 3);

	SOsd_SetPixelWidth(ROSE_WIN, 8);

	SOsd_SetGlobalAlpha( ROSE_WIN, 0 );
	SOsd_Enable( ROSE_WIN, ON );
	dPuts("\n\rFinished Rose window init");
	SOsd_SetLutOffset(ROSE_WIN,0);

	WaitVBlank(1);
	//SpiOsdLoadLUT(0, 0/*1*/, 0, 0x400, ROSE_LUT_LOC, 0xFF);	//win0
	SOsd_SetLut(ROSE_WIN, 0/*1*/, 0, 0x400, ROSE_LUT_LOC, 0xFF );
	SOsd_UpdateLut(ROSE_WIN, 0);	

	SOsd_UpdateReg(ROSE_WIN, 0);
	dPuts("\n\rFinished LUTRoseDMA");

}

//=============================================================================
//		void CarDemo( void )
//=============================================================================
#if 0
void CarDemo( void )  {}
#endif

//=============================================================================
//		void LogoDemo( void )
//=============================================================================
#if 0
void LogoDemo( void ) {}
#endif

//=============================================================================
//		void GridDemo( void )
//=============================================================================
#if 0
void GridDemo( void ) {}
#endif

//=============================================================================
//		void MovingGridInit( void )
//=============================================================================
//WIN1: Grid Image
//WIN2: Grid Message


//We only have one RLE window. WIN_GRID_IMG use more size. I will use it as a RLE.
//If the WIN_GRIP_MSG can not use a RLE, we can use it as a Animation.

#define WIN_GRID_IMG	3	//with RLE. BKTODO Win0 and Win1 use selerate RLE. If we use WIN3, we can share it.
//#define WIN_GRID_IMG	1	//with RLE. BKTODO Win0 and Win1 use selerate RLE. If we use WIN3, we can share it.
#define WIN_GRID_MSG	0	//with Animation

void MovingGridInit( void )
{
	SpiOsdEnable(ON);

	//dPuts("\n\rMovingGridInit - Start");
	SOsd_SetSpiStartOffset( WIN_GRID_IMG, DGRID_IMG_START );
	SOsd_SetImageWidthHeight( WIN_GRID_IMG, DGRID_H, DGRID_V );
	//if(WIN_GRID_IMG==0)
	//	SOsd_SetWin0ImageOffsetXY( 0, 0 );
#if (PANEL_H==1024 && PANEL_V==600)	//sorry! still I have a small image. 
	SOsd_SetScreen( WIN_GRID_IMG, SOSD_X_START+35+112, SOSD_Y_START+150 /*170*/, DGRID_H, DGRID_V );
#else
	SOsd_SetScreen( WIN_GRID_IMG, SOSD_X_START+35, SOSD_Y_START+150 /*170*/, DGRID_H, DGRID_V );
#endif
	//if(WIN_GRID_IMG==0)
	//	SOsd_SetWin0Animation( 1, 0, 0, 0);
	SOsd_SetPixelAlpha( WIN_GRID_IMG, ON );
	SOsd_SetPixelWidth(WIN_GRID_IMG, 8);
	//need RLE
	SOsd_SetRlc( WIN_GRID_IMG, d_grid_rle[0]>>4,d_grid_rle[0]&0x0F); //(curr_bg_info->rle >> 4), curr_bg_info->rle & 0x0F);
	SOsd_UpdateRlc();
	SOsd_SetLutOffset(WIN_GRID_IMG,0);

	SOsd_Enable( WIN_GRID_IMG, ON );

	//MovingGrid Message
	SOsd_SetSpiStartOffset( WIN_GRID_MSG, DGRID_MSG_START );
	SOsd_SetImageWidthHeight( WIN_GRID_MSG, 800, 60 ); //BUG: MAX 800?? -> FIxed 11/18/2010.
	if(WIN_GRID_MSG==0)		 //BK110330
		SOsd_SetWin0ImageOffsetXY( 0, 0 );
#if (PANEL_H==1024 && PANEL_V==600)	//sorry! still I have a small image. 
	SOsd_SetScreen( WIN_GRID_MSG, SOSD_X_START+112, SOSD_Y_START+410/*420*/, 800, 60 );
#else
	SOsd_SetScreen( WIN_GRID_MSG, SOSD_X_START+0, SOSD_Y_START+410/*420*/, 800, 60 );
#endif
	if(WIN_GRID_MSG == 0)
		SOsd_SetWin0Animation( 2/*3*/, 0, 3, 100/*50*/);
	dPuts("\n\rFinished GRID window init");
	SOsd_SetPixelAlpha( WIN_GRID_MSG, ON );
	SOsd_SetPixelWidth(WIN_GRID_MSG, 8); 
	SOsd_Enable( WIN_GRID_MSG, ON );
	SOsd_SetLutOffset(WIN_GRID_MSG,0x00 );  //If use a correct LUT, 0x0B is a better value.


	WaitVBlank(1);
	//SpiOsdLoadLUT(WIN_GRID_MSG, LUTTYPE_ADDR, 0, 0x400,  DGRID_LUT_START, 0xFF ); //win ?0 we have a alpha ready LUT.
	SOsd_SetLut(WIN_GRID_MSG, LUTTYPE_ADDR, 0, 0x400,  DGRID_LUT_START, 0xFF );
	SOsd_UpdateLut(WIN_GRID_MSG, 0);	
	if(WIN_GRID_IMG==1 || WIN_GRID_IMG==2) {
		//SpiOsdLoadLUT(WIN_GRID_IMG, LUTTYPE_ADDR, 0, 0x400,  DGRID_LUT_START, 0xFF ); //win ?1 TW8836 uses two LUT
		SOsd_SetLut(WIN_GRID_IMG, LUTTYPE_ADDR, 0, 0x400,  DGRID_LUT_START, 0xFF );
		SOsd_UpdateLut(WIN_GRID_IMG, 0);	
	}
	WaitVBlank(1);
	SOsd_UpdateReg(WIN_GRID_MSG, WIN_GRID_MSG);
	//test only IMG
	SOsd_UpdateReg(WIN_GRID_IMG, WIN_GRID_IMG);

	dPuts("\n\rFinished LUT GRID DMA");
	//dPuts("\n\rMovingGridInit - End");
}

//=============================================================================
//		void MovingGridDemo( BYTE n )
//=============================================================================
void MovingGridDemo( BYTE n )
{
	DWORD	offset;
	BYTE rle_byte;

	//dPrintf("\n\rMovingGridDemo(%02d)", (WORD)n);
	offset = d_grid_IMG[n].start;
	rle_byte = d_grid_rle[n];	
	//dPrintf("  offset:%06lx", offset);

	SOsd_SetSpiStartOffset( WIN_GRID_IMG, offset );

	WaitVBlank(1);
	SOsd_SetRlc( WIN_GRID_IMG, rle_byte >> 4, rle_byte & 0x0F);
	SOsd_UpdateRlc();
	SOsd_UpdateReg(WIN_GRID_IMG, WIN_GRID_IMG);
	//dPuts("\n\rFinished LUT GRID DMA");

//dPrintf("  0x40F:0x%02bx -demo", ReadTW88(REG40F));	

}

//=============================================================================
//		void MovingGridLUT( BYTE n )
//=============================================================================
void MovingGridLUT( BYTE n )
{
	WORD	offset;

	offset = n;
	offset *= 1024;
	WaitVBlank(1);
	//SpiOsdLoadLUT( WIN_GRID_MSG ,LUTTYPE_ADDR, 0, 0x400, DGRID_LUT_START + offset, 0xFF );	//win ?0
	SOsd_SetLut(WIN_GRID_MSG ,LUTTYPE_ADDR, 0, 0x400, DGRID_LUT_START + offset, 0xFF);
	SOsd_UpdateLut(WIN_GRID_MSG, 0);	
	if(WIN_GRID_IMG==1 || WIN_GRID_IMG==2) {
		//SpiOsdLoadLUT( WIN_GRID_IMG ,LUTTYPE_ADDR, 0, 0x400, DGRID_LUT_START + offset, 0xFF );	//win ?1
		SOsd_SetLut(WIN_GRID_IMG ,LUTTYPE_ADDR, 0, 0x400, DGRID_LUT_START + offset, 0xFF);
		SOsd_UpdateLut(WIN_GRID_IMG, 0);	
	}
#ifdef DEBUG_OSD
	dPuts("\n\rFinished LUT GRID LUT change");
#endif
}

//=============================================================================
//		void MovingGridAuto( void )
//=============================================================================

#if 0
//description:
//	This function do not receive the control input from TW-Terminal.
void MovingGridAuto( void )
{...}
#endif



void MovingGridTask_init(void)
{
	TaskSetGrid(1);	//Task_Grid_on = 1;			//global
	Task_Grid_direction = 0;	//static
	Task_Grid_dt = 10;			//static
	Task_Grid_n = 0;			//static
	Task_Grid_lut = 0;			//static
	Task_Grid_timer = Task_Grid_dt;	  Task_Grid_timer *= 100;
								//Task_Grid_timer 	  static
								//Task_Grid_dt	  	  static
								//Task_Grid_timer 	  static
}

#define NAVI_KEY_NONE	0
#define NAVI_KEY_ENTER	1
#define NAVI_KEY_UP		2
#define NAVI_KEY_DOWN	3
#define NAVI_KEY_LEFT	4
#define NAVI_KEY_RIGHT	5

void MovingGridTask( void )
{
	BYTE GridCmd;

	if(tic_task > Task_Grid_timer) 
	{
		GridCmd = TaskGetGridCmd();
		if( GridCmd )	{
			dPrintf("\n\r**** GridCmd:%bx", GridCmd);	
			if ( GridCmd == NAVI_KEY_ENTER ) {
				TaskSetGrid(0);		//Task_Grid_on = 0;	//OFF
				TaskSetGridCmd(0);	//clear
				WaitVBlank(1);
				SOsd_Enable( WIN_GRID_MSG, OFF );
				SOsd_UpdateReg(WIN_GRID_MSG, WIN_GRID_MSG);
				SOsd_Enable( WIN_GRID_IMG, OFF );
				SOsd_UpdateReg(WIN_GRID_IMG, WIN_GRID_IMG);
				SpiOsdEnable(OFF);

				return;
			}
			if ( GridCmd == NAVI_KEY_UP ) {
				if ( Task_Grid_dt ) {
					Task_Grid_dt--;					   		//fast up
					Task_Grid_timer = Task_Grid_dt;
					Task_Grid_timer *= 100;
				}								  
			}
			if ( GridCmd == NAVI_KEY_DOWN ) {
				if ( Task_Grid_dt < 20 ) {					//slow down
					Task_Grid_dt++;
					Task_Grid_timer = Task_Grid_dt;
					Task_Grid_timer *= 100;
				}
			}
			else if ( GridCmd == NAVI_KEY_RIGHT ) { 		//right
				Task_Grid_lut++;
				Task_Grid_lut %= 4;
				MovingGridLUT( Task_Grid_lut );
			}
			else if ( GridCmd == NAVI_KEY_LEFT ) {  		//left
				Task_Grid_lut +=3;
				Task_Grid_lut %= 4;
				MovingGridLUT( Task_Grid_lut );
			}

		 	TaskSetGridCmd(0);	//clear
		}

		if ( Task_Grid_direction ) {		// move to RIGHT
			if ( Task_Grid_n > 10 ) {
				Task_Grid_n++;
			}
			else {
				if(Task_Grid_n)
					Task_Grid_n--;
				else
					Task_Grid_n = 11;
			}
			if ( Task_Grid_n == 20 )
				Task_Grid_direction = 0;	// move to LEFT						
		}
		else {				   				// move to LEFT
			if ( Task_Grid_n < 10 ) {
				Task_Grid_n++;
			}
			else {
				Task_Grid_n--;
				if ( Task_Grid_n == 10 )
					Task_Grid_n = 0;
			}
			if ( Task_Grid_n == 10 )
				Task_Grid_direction = 1;	// move to RIGHT
		}
		MovingGridDemo(Task_Grid_n);
		//dPrintf("  tic_my:%d Task_Grid_timer:%d", tic_task, Task_Grid_timer);
		tic_task = 0;		//reset wait counter.

	}
}


//=============================================================================
//		void CompassDemo( void )
//=============================================================================
#if 0
void CompassDemo( void ) {}
#endif

//=============================================================================
//		void ComplexDemo( void )
//=============================================================================
#if 0
void ComplexDemo( void ) {}
#endif

//=============================================================================
//		void OsdWinOffAll( void )
//=============================================================================
#if 0
void OsdWinOffAll(void)	{}
#endif

#if 1	//save data memory and use code segment
#else
void (*f[])(void) = { (void *)OsdWinOffAll, (void *)RoseDemo, (void *)CarDemo, (void *)CompassDemo, (void *)GridDemo, (void *)LogoDemo, (void *)FontDemo, (void *)ComplexDemo };
#endif


//=============================================================================
//		void OsdDemoNext( void )
//=============================================================================
#if 0
void OsdDemoNext(void)	{}
#endif

//=============================================================================
// Read Output Pixel
// this feature is a junk.
// It is related with PCLKO and VBlank.
// If it is a VBlank area, we can not read.
// I can find a VBlank start position, but, I can not find the non-vblank area.
// Giveup and do not try to use it.
//=============================================================================
#undef CHECK_USEDTIME
void ReadOutputPixel(WORD start_x, WORD start_y,WORD end_x, WORD end_y)
{
	WORD x,y,max_x;
	BYTE y_high;
	BYTE bTemp;
#ifdef CHECK_USEDTIME
	DWORD UsedTime, curr_clock;
#endif
	volatile BYTE r,g,b;


	Printf("\n\r--Start Read Pixel %dx%d to %dx%d",start_x,start_y,end_x,end_y);
	if(end_y >= PANEL_V)
		end_y = PANEL_V -1;

	if(end_x >= PANEL_H)
		end_x = PANEL_H - 1;
	max_x = end_x; 
	bTemp = ReadTW88(REG20D) & 0x23;

	max_x *= ((bTemp & 0x03) +1);
	if(bTemp & 0x20)
		max_x >>= 1;

	Printf("  PCLKO:");
	if(bTemp == 0x22)	Printf("1.5");
	else {
		bTemp &= 0x03;
		Printf("%bd", bTemp+1);
	}
			


#ifdef CHECK_USEDTIME
	SFRB_ET0 = 0;
	UsedTime = SystemClock;
	SFRB_ET0 = 1;
#endif
	WaitVBlank(1);
	delay1ms(8);
	for(y=start_y; y <= end_y; y++) {		//vertical
		Printf("\n\rLine:%d Pixels:%d\n",y,max_x);
		for(x=start_x; x < max_x; x++) {		//horizontal
			//position
			WriteTW88(REG2F0, (BYTE)x);
			WriteTW88(REG2F1, (BYTE)y);
			y_high = (y >> 8) << 4; 
			WriteTW88(REG2F2, y_high | (x >> 8));
			//Read R G B
			r=ReadTW88(REG2F3);
			g=ReadTW88(REG2F4);
			b=ReadTW88(REG2F5);

			Printf("%02bx-%02bx-%02bx ", r,g,b);
		}
	}
#ifdef CHECK_USEDTIME
	SFRB_ET0 = 0;
	curr_clock = SystemClock;
	SFRB_ET0 = 1;
	UsedTime = curr_clock - UsedTime;
	Printf("\n\r--END UsedTime:%ld.%ldsec", UsedTime/100, UsedTime%100 );
#else
	Printf("\n\r--END");
#endif
}
#undef CHECK_USEDTIME
