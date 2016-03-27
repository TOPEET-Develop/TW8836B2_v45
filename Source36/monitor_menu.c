/**
 * @file
 * Monitor_MENU.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	Interface between TW_Terminal2 and Firmware.
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
//								Monitor_Menu.c
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
#include "monitor_MENU.h"
#include "reg_debug.h"

#include "i2c.h"
//#include "menu8835.h"
#include "Remo.h"

#include "SOsd.h"
#include "FOsd.h"
#include "SOsdMenu.h"

#include "Demo.h"
#include "SpiFlashMap.h"

#ifdef SUPPORT_FOSD_MENU
#include "FOsdMenu.h"
#endif

//temp..please move to .h file and remove it
extern 		void MenuCheckTouchInput(BYTE TscStatus,int x,int y);
extern		void proc_home_menu_start(void);

extern void DumpFontOsdPalette(void);
extern void Test16x32FontSet(void);
extern void TestUpper256Char(void);
extern void TestMultiBPP4(void);
extern void TestNum1BPP3BPP(void);

extern void TaskSetGridCmd(BYTE cmd);
extern BYTE TaskGetGrid(void);


//extern XDATA BYTE Task_Grid_on;
extern void MenuKeyInput(BYTE key);

//extern TestMainMenuImage(BYTE type);



//=============================================================================
//
//=============================================================================
/*
	menu start
	menu [up|down|right|left]
	menu enter
	menu exit
*/
void MonitorMenu(void)
{
//	int x, y;
	
	if (!stricmp(argv[1], "?"))
	{
		Puts("\n\r  === Help for MENU command ===");
		Puts("\n\rmenu start					;start main menu");
		Puts("\n\rmenu [up|down|right|left]	;menu arrow key");
		Puts("\n\rmenu enter					;enter key");
		Puts("\n\rmenu exit					;exit key");

		return;
	}

	if (argc != 3)
	{
		//Puts("\n\rmenu x y");

		if (TaskGetGrid())
		{
			if (!stricmp(argv[1], "up"))
			{
				TaskSetGridCmd(NAVI_KEY_UP);
			}
			else if (!stricmp(argv[1], "down"))
			{
				TaskSetGridCmd(NAVI_KEY_DOWN);
			}
			else if (!stricmp(argv[1], "left"))
			{
				TaskSetGridCmd(NAVI_KEY_LEFT);
			}
			else if (!stricmp(argv[1], "right"))
			{
				TaskSetGridCmd(NAVI_KEY_RIGHT);
			}
			else if (!stricmp(argv[1], "enter"))
			{
				TaskSetGridCmd(NAVI_KEY_ENTER);
			}

			return;
		}
		
		if (MenuGetLevel())
		{
			if (!stricmp(argv[1], "up"))
			{
				MenuKeyInput(NAVI_KEY_UP);
			}
			else if (!stricmp(argv[1], "down"))
			{
				MenuKeyInput(NAVI_KEY_DOWN);
			}
			else if (!stricmp(argv[1], "left"))
			{
				MenuKeyInput(NAVI_KEY_LEFT);
			}
			else if (!stricmp(argv[1], "right"))
			{
				MenuKeyInput(NAVI_KEY_RIGHT);
			}
			else if (!stricmp(argv[1], "enter"))
			{
				MenuKeyInput(NAVI_KEY_ENTER);
			}
		}

		if (!stricmp(argv[1], "start"))
		{
			Printf("\n\rcall MenuStart");
			//proc_home_menu_start();
			MenuStart();
		}
		else if (!stricmp(argv[1], "exit"))
		{
			Printf("\n\rcall MenuEnd");
			MenuEnd();
		}
		else if (!stricmp(argv[1], "test1"))
		{
			//draw 1bpp FOSD Palette
			//environmemt
			//	w ff 3
			//	b 8a 0c 66 40
			//	fosd dnfont 0
			//
			DumpFontOsdPalette();
		}
		else if (!stricmp(argv[1], "test2"))
		{
			//FOSD FIFO test.
			Test16x32FontSet();
		}
		else if (!stricmp(argv[1], "test3"))
		{
			TestUpper256Char();
		}
		else if (!stricmp(argv[1], "test4"))
		{
			TestMultiBPP4();
		}
		else if (!stricmp(argv[1], "test5"))
		{
			TestNum1BPP3BPP();
		}
		//else if(!stricmp( argv[1], "main1")) {
		//	TestMainMenuImage(1);
		//}
		//else if(!stricmp( argv[1], "main2")) {
		//	TestMainMenuImage(2);
		//}
		//else if(!stricmp( argv[1], "main3")) {
		//	TestMainMenuImage(3);
		//}

		return;
	}

//	x=a2h(argv[1]);
//	y=a2h(argv[2]);
//   	Printf("\n\r ECHO: menu x:%d y:%d", x,y);
//	MenuCheckTouchInput(x,y);
}

//=============================================================================
//
//=============================================================================
//FONT OSD
void MonitorFOsd(void)
{
	if(!stricmp( argv[1], "?")) {
		Puts("\n\r  === Help for FOSD command ===");
		Puts("\n\rFOSD BPP3			;calculate 3BPP alpha order");
		Puts("\n\rFOSD BPP2	[winno]	;draw 2BPP intersil icon");
		Puts("\n\rFOSD lutd			;dump 64 lut value");
		Puts("\n\rFOSD lutw	dat		;overwrite all 64 LUT with value");
		Puts("\n\rFOSD dnlut  [0|1|2]	;download LUT");
		Puts("\n\rFOSD info			;");
		Puts("\n\rFOSD dnfont	[n]		;download font");
		Puts("\n\rFOSD fontd			;dump downloaded font");
	}
	//=============================================
	// font
	//	font info
	// 	download font
	//	dump font
	// 
	//palette
	//	palette dump
	// 	download palette
	//
	//osdram
	//	
	//

	//-----------------------------------------------------
	// print Font information
	else if(!stricmp( argv[1], "info" )) {
		extern void FontInfoByNum(BYTE FontMode);
		
		//FOsdMon_info();
		Printf("\n\rFont");
		Printf("\n\r\t0: default_font 12x18");
		Printf("\n\r\t1: consolas 16x26");
		Printf("\n\r\t2: consolas graynum 16x26");
		Printf("\n\r\t3: con+graynum 16x26");
		Printf("\n\r\t4: def+kor 16x26");
		FontInfoByNum(0);
		FontInfoByNum(1);
		FontInfoByNum(2);
		FontInfoByNum(3);
	}
	//-----------------------------------------------------
	// download font
	else if(!stricmp( argv[1], "dnfont" )) {
		BYTE num;
		WORD loc;
		if(argc < 3) {
			num = 0;
			loc = 0;
		}
		else {
			num = a2h(argv[2]);
			if(argc < 4) {
				loc = 0;
			}
			else {
				loc = a2h(argv[3]);
			}
		}
		InitFontRamByNum(num, loc);
	}
	//-----------------------------------------------------
	// dump font.
	else if(!stricmp( argv[1], "fontd" )) {
		extern void DumpFont(void);
		DumpFont();
	}
	//-----------------------------------------------------
	// dump palette.
	else if(!stricmp( argv[1], "lutd")) {
		//dump palette table
		BYTE winno;

		winno=3;
		if(argc == 3) {
			winno = a2h(argv[2]);
			if(winno > 3)
				winno=3;
		}
		FOsdDumpPalette(winno);
	}
	else if(!stricmp( argv[1], "lutw")) {
		//fosd lutw 0 ffff  0
		BYTE loc;
		WORD color;
		if ( argc < 3 ) {
			Printf("\n\rflutw need a WORDSIZE value");
		}
		else {
			loc = a2h(argv[2]);
			color = a2h(argv[3]);
			FOsdSetPaletteColor(loc, color, 1, 0);
		}
	}
	else if(!stricmp( argv[1], "dnlut")) {
		//download palette table
		BYTE lut_num;

		lut_num = 0;
		if(argc == 3) {
			lut_num = a2h(argv[2]);
		}
		FOsdSetDefPaletteColor(lut_num);


	}
	else if(!stricmp( argv[1], "cpwin" )) {
		/*
		TW8836 has 8 FontOsdWindow. FW uses 4 windows for MENU.
		To test WIN5,6,7,8 copy WIN1,2,3,4 register to WIN5,6,7,8 
		and then adjust start x, start y.
		*/
		BYTE src,dest,i;

		if(argc >=4) {	
			src = a2h(argv[2]);
			dest = a2h(argv[3]);
			if(src >=8 || dest >=8)
				return;
			for(i=0; i < 0x10; i++) {
				WriteTW88(REG310+dest*0x10+i, ReadTW88(REG310+src*0x10+i));
			}
		}
	}
	else if ( !stricmp( argv[1], "test1" )) {
		//extern void FOsdDisplayInput(void);
		//FOsdDisplayInput();
		extern void FOsdTest_1(BYTE mode);
		FOsdTest_1(0);
	}
	else if ( !stricmp( argv[1], "test2" )) {
		extern void FOsdTest_1(BYTE mode);
		FOsdTest_1(1);
	}
	else if ( !stricmp( argv[1], "test3" )) {
#ifdef SUPPORT_FOSD_MENU
//		extern void FOsdMenuOpen(void);

		//InitFontRam(0, &default_font,"def");
		InitFontRamByNum(FONT_NUM_DEF12X18, 0);
		FOsdRamSetFifo(ON, 1);
		FOsdMenuOpen();
#endif
	}
	else if ( !stricmp( argv[1], "test4" )) {
#ifdef SUPPORT_FOSD_MENU
		//extern void FOsdDisplayInput(void);
		//FOsdDisplayInput();

		extern void FOsdDisplayInput(void);
//		extern BYTE CloseOSDMenu(void);
		extern void WriteStringToAddr(WORD addr, BYTE *str, BYTE cnt);
//		extern void OSDSelect(void);
		extern void DisplayPCInfo(BYTE CODE *ptr);
//		extern void OSDCursorMove(BYTE flag );



//		CloseOSDMenu();
		FOsdDisplayInput();
		WriteStringToAddr(0, "test", 4);
//		OSDSelect();
		DisplayPCInfo("CHECK 656 signal");
//		OSDCursorMove(0);
#endif
	}
#if defined(SUPPORT_I2C_MASTER)	
	else if ( !stricmp( argv[1], "test5" )) {
		extern void FOsdTest_5(BYTE);
		BYTE num;
		if(argc < 3)	num = 0;
		else			num = a2h(argv[2]);
		

		FOsdTest_5(num);
	}
#endif
	else if(!stricmp( argv[1], "BPP3")) {
		FOsdInitBpp3AlphaTable(1);
	}
	else if(!stricmp( argv[1], "BPP2")) {
		BYTE winno;

		winno=3;
		if(argc == 3) {
			winno = a2h(argv[2]);
			if(winno > 3)
				winno=3;
		}
//		FOsdIntersil(winno);
	}
	else
		Printf("\n\rInvalid command...");	
}

//=============================================================================
//
//=============================================================================
//SPI OSD
void MonitorSOsd(void)
{
	if(!stricmp( argv[1], "?")) {
		Puts("\n\r=== Help for SOSD command ===");
		Puts("\n\r\ton/off		;SOSD on/off");
		Puts("\n\r\t [rose|pigeon]");
		Puts("\n\r\t ??");
		Puts("\n\r\t lut img_n lut_n");
		Puts("\n\r\t img img_n lut_n win_n");
		Puts("\n\rAdd Your Test Function			;add comment");
 	}
	//-------<<on>>-----------------------------
	else if(!stricmp( argv[1], "on")) {
		SpiOsdEnable(ON);
	}
	//-------<<off>>-----------------------------
	else if(!stricmp( argv[1], "off")) {
		SpiOsdEnable(OFF);
	}
	//-------<<rose>>-----------------------------
	else if(!stricmp( argv[1], "rose")) {
		if(argc >= 3) {
			if(!stricmp( argv[2], "info")) {
				Printf("\n\rimg loc:%lx size:%lx lut loc:%lx size:%lx 400x400x10",ROSE_START,ROSE_LEN, ROSE_LUT_LOC,PIGEON_ROSE_LUT_LEN);
			}
			else
				Printf("\n\rInvalid command...");	
		}
		RoseDemo();
	}
	//-------<<pigeon>>-----------------------------
	else if(!stricmp( argv[1], "pigeon")) {
		if(argc >= 3) {
			if(!stricmp( argv[2], "info")) {
				Printf("\n\rimg loc:%lx size:%lx lut loc:%lx size:%lx 400x400x10",PIGEON_START,PIGEON_LEN, PIGEON_LUT_LOC,PIGEON_ROSE_LUT_LEN);
			}
			else
				Printf("\n\rInvalid command...");	
		}
		PigeonDemo();
	}
	//-------<<?? test image information>>-----------------------------
	else if(!stricmp( argv[1], "??")) {
		MonSOsdImgInfo();
	}
	//-------<<lut offset# addr#>>-----------------------------
	else if(!stricmp( argv[1], "lut")) {
		BYTE img_n;
		WORD lut;
		BYTE saved_DebugLevel;

		if(argc < 4)
			Printf("\n\rInvalid command...");		 
		else {
			saved_DebugLevel = DebugLevel;
			DebugLevel = 3;

			img_n=a2i(argv[2]);
			lut  =a2i(argv[3]);
			MonOsdLutLoad(img_n,3, lut); //I don't know winno. so tempolary assign 3.

			DebugLevel = saved_DebugLevel;	
		}	
	}
	//-------<<img0 win# lut#>>-----------------------------
	else if(!stricmp( argv[1], "img")) {
		BYTE img_n,winno;
		WORD lut;
		if(argc < 5)
			Printf("\n\rInvalid command...");		 
		else {
			img_n=a2i(argv[2]);
			lut  =a2i(argv[3]);
			winno = a2i(argv[4]);
			MonOsdImgLoad(img_n,winno,lut);
		}	
	}
	//-------<<img1 win# lut#>>-----------------------------
	else if(!stricmp( argv[1], "dnitem")) {
		BYTE img_n,winno;
		WORD lut;
		BYTE saved_DebugLevel;

		if(argc < 4) 
			Printf("\n\rInvalid command...");
		else {
			saved_DebugLevel = DebugLevel;
			DebugLevel = 3;
			if(!stricmp( argv[3], "lut")) {
				img_n = a2i(argv[2]);
				winno = ReadTW88(REG009) >> 5;
				winno++;	
				lut = ReadTW88(REG009) & 0x1F;
				lut <<= 4;
				MonOsdLutLoad(img_n,winno, lut);
			} 
			else if(!stricmp( argv[3], "img")) {
				img_n = a2i(argv[2]);
				winno = ReadTW88(REG009) >> 5;
				winno++;	
				lut = ReadTW88(REG009) & 0x1F;
				lut <<= 4;
				MonOsdImgLoad(img_n,winno,lut);
			}
			else
				Printf("\n\rInvalid command...");
			DebugLevel = saved_DebugLevel;	
		}
	}
	//-------<<sosd download lut>>-----------------------------
	else if(!stricmp( argv[1], "dnlut")) {
		//download palette table
		//format: sosd dnlut lut_offset image_address
		//step: read header	& download lut
		extern menu_image_header_t header_table;
		extern BYTE MenuReadRleHeader(DWORD spi_loc,struct RLE2_HEADER *header);
		extern void rle2_to_header(struct RLE2_HEADER *rle_header);
		#define MRLE_INFO_SIZE		0x10
		WORD lut_loc;
		BYTE lut_type;
		WORD nColor;
		DWORD image_loc;
		BYTE winno;		//TW8836 need.
		menu_image_header_t *header = &header_table;
		struct RLE2_HEADER rle_header;


		if(argc < 3) {
			Puts("\n\ruse: sosd dnlut ?");
			return;
		}
		else {
			if(argv[2][0]=='?') {
				Puts("\n\rFormat: sosd dnlut winno lut_type(0 or 1) lut_offset flash_addr");
				Puts("\n\r      : sosd dnlut winno lut_type(2 or 3) lut_offset flash_addr [nColor].");
				Puts("\n\r\twinno 0~8. TW8836 needs winno");
				Puts("\n\r\tlut_type 0: TW8832 style. 256 color.");
				Puts("\n\r\t         1: TW8835 menu style. image has a header+palette+data.");
				Puts("\n\r\t         2: LUT type(BBB...GGG..RRR...AAA...).");
				Puts("\n\r\t         3: LUTS type(BGRA BGRA BGRA...).");
				Puts("\n\r\tlut_offset 0~511 (TW8836 win1~win2:0~255, win3~win8 and win0:0~511)");
				Puts("\n\r\tflash_addr:flash address. if lut_type is 1, it has a header location");
				Puts("\n\r\tnColor:number of color. default:256");
				Puts("\n\rexample:");
				Puts("\n\r\t sosd dnlut 1 1 0 41e071      ; menu background");
				Puts("\n\r\t sosd dnlut 1 3 0 41e081 128  ; menu background");
				Puts("\n\r\t sosd dnlut 8 1 256 4336a7    ; menu video icon");
				return;
			}
			if(argc < 6) {
				Puts("\n\ruse: sosd dnlut ?");
				return;
			}
			winno = a2h(argv[2]);
			lut_type = a2h(argv[3]);
			lut_loc = a2i(argv[4]);
			image_loc = a2h(argv[5]);
			if(lut_type == 2 || lut_type == 3) {
				if(argc < 7) 
					nColor = 256;
				else
					nColor = a2i(argv[6]);
			}
		}
		
	
		//MenuPrepareImageHeader(image);	//update header_table
		if(lut_type==0) { //TW8832 style. only LUT. size 256*4. group style.
			header->lut_type = 0;
			header->lut_size = 0x400;
			header->lut_loc = image_loc;
		}
		else if(lut_type==1) { //TW8835 menu style
			MenuReadRleHeader(image_loc,&rle_header);		
			rle2_to_header(&rle_header);
			header->lut_loc = image_loc + MRLE_INFO_SIZE;
			header->image_loc = image_loc + header->lut_size + MRLE_INFO_SIZE;
		} 
		else if(lut_type==2) {
			header->lut_type = 0;
			header->lut_size = nColor*4;
			header->lut_loc = image_loc;
		}
		else if(lut_type==3) {
			header->lut_type = 1;
			header->lut_size = nColor*4;
			header->lut_loc = image_loc;
		}
		else {
			Printf("\n\rInvalid command...");	
			return;
		}	
		WaitVBlank(1);
		//SpiOsdLoadLUT(winno, header->lut_type, lut_loc, header->lut_size, header->lut_loc, 0xFF);
		SOsd_SetLut(winno, header->lut_type, lut_loc, header->lut_size, header->lut_loc, 0xFF);
		SOsd_UpdateLut(winno, 0);	

	}
	//-------<<sosd demo1>>-----------------------------
	else if(!stricmp( argv[1], "demo1")) {
		extern void SOsdDemo1(void);
		SOsdDemo1();
	}
	else if(!stricmp( argv[1], "demo2")) {
		extern void SOsdDemo2(void);
		SOsdDemo2();
	}
	//1280x800 test
	else if(!stricmp( argv[1], "demo3")) {
		extern void SOsdDemo3(void);
		SOsdDemo3();
	}



	//-------<<sosd rlut img_n lut_n>>----------------------
	// read back lut & compare
	else if(!stricmp( argv[1], "rlut")) {
	}
	//-------<<img5 win# lut#>>-----------------------------
	else if(!stricmp( argv[1], "??")) {
	}
	//-------<<img6 win# lut#>>-----------------------------
	else if(!stricmp( argv[1], "??")) {
	}
	//-------<<img7 win# lut#>>-----------------------------
	else if(!stricmp( argv[1], "??")) {
	}
	//-------<<??>>-----------------------------
	else if(!stricmp( argv[1], "??")) {
	}
	else
		Printf("\n\rInvalid command...");	
}
