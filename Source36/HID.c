/**
 * @file
 * HID.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	Human Interface Device
 *	support Keypad, touch, remocon
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
#include "config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "global.h"
#include "CPU.h"
#include "printf.h"

#include "I2C.h"

#include "main.h"
#include "Remo.h"
#include "TouchKey.h"
#include "HID.h"

#include "SOsd.h"
#include "FOsd.h"
#include "SOsdMenu.h" //NAVI_KEY_
#include "eeprom.h"
#include "InputCtrl.h"

#ifdef SUPPORT_FOSD_MENU
#include "FOsdString.h"
#include "FOsdMenu.h"
#include "FOsdDisplay.h"
extern    bit     OnChangingValue;
#endif

#define CAPTURE_MAX		10

BYTE previous_key0 = 0;
BYTE previous_key1 = 0;

#if defined(SUPPORT_HDMI)
/*
*
* @return
*	if it ate, return 0xFF;
*	or, return _RemoDataCode.
*/
#define EAT_KEY		0xFF
BYTE ActionRemoHDMI(BYTE _RemoDataCode, BYTE AutoKey)
{
	BYTE value = AutoKey;
	BYTE ret = _RemoDataCode;

	if (global_CEC_flag == 0)
		return ret;
	
	value = ReadI2C_multi(0x68, 0x21, 0x0900);
	if ((value & 0x03) != 0x03)
	{
		Printf("\n\rglobal_CEC_flag ON, but SYS_Status:%bx", value);
		return ret;
	}

	switch(_RemoDataCode)
	{
	case REMO_SELECT:	//0x00;Select
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x00);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	case REMO_CHNUP: //0x01:Up
  		dPuts("\n\rREMO1_CHNUP pressed!!!");
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x01);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	case REMO_CHNDN: //0x02:Down
  		dPuts("\n\rREMO1_CHNDN pressed!!!");
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x02);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	case REMO_VOLDN: //0x03:Left
  		dPuts("\n\rREMO1_VOLDN pressed!!!");
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x03);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	case REMO_VOLUP: //0x04:Right
  		dPuts("\n\rREMO1_VOLUP pressed!!!");
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x04);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
 	//0x05:Right-Up
	//0x06:Right-Down
	//0x07:Left-Up
	//0x08:Left-Down
	//
	case REMO_EXIT:	//0x09:Root Menu
  		dPuts("\n\rREMO_EXIT pressed!!!");
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x09);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	//0x0A Setup Menu
	//0x0B: Contents Menu
	//0x0C: Favoriate Menu
	//0x0D: Exit
	case REMO_NUM0:
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x20);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	case REMO_NUM1:
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x21);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	case REMO_NUM2:
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x22);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	case REMO_NUM3:
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x23);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	case REMO_NUM4:
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x24);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	case REMO_NUM5:
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x25);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	case REMO_NUM6:
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x26);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	case REMO_NUM7:
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x27);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	case REMO_NUM8:
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x28);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	case REMO_NUM9:
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x29);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	//0x2A Dot
	//0x2B Enter
	//0x2C Clear
	//0x30 Channel Up
	//0x31 Channel Down
	//0x32 Previous Channel
	//0x33 Sound Select
	case REMO_INPUT: // 0x34 Input Select
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x34);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	case REMO_INFO:	   //0x35 Show Information
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x35);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	//0x36 Help
	//0x37 Page Up
	//0x38 Page Down
	//0x41 Volume Up
	//0x42 Volume Down
	case REMO_MUTE: //0x43 Mute
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x43);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	//0x44 Play
	//0x45 Stop
	//0x46 Pause
	//0x47 Record
	//0x48 Rewind
	//0x49 Fast Forward
	//0x4A Eject
	//0x4B Forward
	//0x4C Backward
	//0x50 Angle
	//0x51 Subtitle
	//0x06 Play_Function
	//0x61 Pause_Play_Function
	//0x62 Record_Function
	//0x63 Pause_Record_Function
	//0x64 Stop_Function
	//0x65 Mute_Function
	//0x66 Restore_Volume_Function
	//0x67 Tune_Function
	//0x78 Select_Media_Function
	case REMO_TTXCYAN: 	//0x71 F1 (Blue)
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x71);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	case REMO_TTXRED:	//0x72 F2 (Red)
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x72);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	case REMO_TTXGREEN:	 //0x73 F3 (Green)
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x73);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	case REMO_TTXYELLOW: //0x74 F4 (Yellow)
		WriteI2C_multi(0x68, 0x21, 0xA01, 0x74);
		WriteI2C_multi(0x68, 0x21, 0xA00, 0x10);
		ret = EAT_KEY;
		break;
	//0x75 F5
	//0x7E Vendor_Specific

 	//==============================================
	case REMO_STANDBY:				// power
		//return 0;					// power off
		break;
	case REMO_MENU:
		break;
	case REMO_PIPON:
#if defined(SUPPORT_HDMI)
		//dPuts("\n\rPIP Display mode change");
		dPuts("\n\rCEC_AUTO OFF");
		//WriteI2C_multi(0x68,0x21, 0x800, 0x00);
		global_CEC_flag = 0;

		ret = EAT_KEY;
#endif
		break;			 
	case REMO_STILL:
		break;
	case REMO_SWAP:
		//dPrintf("\n\r=====SWAP InputMode:%bd",InputMain);
		break;

	case REMO_PIPINPUT:
		dPuts("\n\rPIP input mode change");
		break;

	case REMO_AUTO:
		break;

	case REMO_ASPECT:
		break;

	}
	return ret;
}
#endif

/*
* @return
*	if it ate, return 0xFF;
*	or, return _RemoDataCode.
*/
BYTE ActionGridRemo(BYTE _RemoDataCode)
{
	if (_RemoDataCode == REMO_CHNUP)
	{
		TaskSetGridCmd(NAVI_KEY_UP);
		return 0xFF;
	}
	else if (_RemoDataCode == REMO_CHNDN)
	{
		TaskSetGridCmd(NAVI_KEY_DOWN);
		return 0xFF;
	}
	else if (_RemoDataCode == REMO_VOLDN)
	{
		TaskSetGridCmd(NAVI_KEY_LEFT);
		return 0xFF;
	}
	else if (_RemoDataCode == REMO_VOLUP)
	{
		TaskSetGridCmd(NAVI_KEY_RIGHT);
		return 0xFF;
	}
	else if (_RemoDataCode == REMO_MENU || _RemoDataCode == REMO_EXIT)
	{
		TaskSetGridCmd(NAVI_KEY_ENTER);
		return 0xFF;
	}
	
	return _RemoDataCode;
}

BYTE ActionRemo_SOsdMenu(BYTE _RemoDataCode, BYTE AutoKey)
{
	BYTE value=AutoKey;
	BYTE ret = _RemoDataCode;

	switch (_RemoDataCode)
	{
	case REMO_INPUT:	//ignore
		ret = 0xFF;
		break;
	case REMO_SELECT:
		MenuKeyInput(NAVI_KEY_ENTER);
		ret = 0xFF;	
		break;
	case REMO_CHNUP:
		MenuKeyInput(NAVI_KEY_UP);
		ret = 0xFF;	
		break;
	case REMO_CHNDN:
		MenuKeyInput(NAVI_KEY_DOWN);
		ret = 0xFF;	
		break;
	case REMO_VOLUP:
		MenuKeyInput(NAVI_KEY_RIGHT);
		ret = 0xFF;	
		break;
	case REMO_VOLDN:
		MenuKeyInput(NAVI_KEY_LEFT);
		ret = 0xFF;	
		break;
	case REMO_MENU:
		MenuKeyInput(NAVI_KEY_ENTER);	
		ret = 0xFF;	
		break;
	case REMO_EXIT:
		if(MenuGetLevel() > 1)
			proc_menu_list_return();	//only one level...
		else
			MenuEnd();
		ret = 0xFF;	
		break;
	}
	
	return ret;
}

/**
* action for remocon
*
* if press the menu keypad on the SUPPORT_FOSD_MENU,
*	it will starts the FontOSD menu.
* if press the menu remocon, it will start SpiOSD menu.
* if press the PIP_ON, it will toggle CEC_AUTO feature.
*/
BYTE ActionRemo(BYTE _RemoDataCode, BYTE AutoKey)
{
	BYTE value;

	UpdateOsdTimerClock();

	if (TaskGetGrid())
	{
		value = ActionGridRemo(_RemoDataCode);
		if (value == 0xFF)
			return 0;
		//else passthru..
	}

#if defined(SUPPORT_HDMI)
	//It have to work only when the CEC was enabled...
	if (MenuGetLevel() == 0 && _RemoDataCode != REMO_MENU)
	{
		value = ActionRemoHDMI(_RemoDataCode, AutoKey);
		if (value == 0xFF)
			return 0;
		//else passthru..
	}
#endif

	if (MenuGetLevel())
	{
		//SPI SETUP MENU
		value = ActionRemo_SOsdMenu(_RemoDataCode, AutoKey);
		if (value == 0xFF)
			return 0;
		//else passthru..
	}

	switch (_RemoDataCode)
	{
	case REMO_STANDBY:				// power
		if (AutoKey)				// need repeat key. 
			return REQUEST_POWER_OFF;
		return 0;					// power off

	case REMO_MUTE:
#ifdef SUPPORT_FOSD_MENU
		if (AutoKey)
			return 1;
//		ToggleAudioMute();
		if (IsAudioMuteOn())
			DisplayMuteInfo();
		else
		{						
			ClearMuteInfo();
			
			if (DisplayInputHold)
				FOsdDisplayInput();
		}
#endif
		break;

	case REMO_INPUT:
#ifdef SUPPORT_FOSD_MENU
		InputModeNext();
#else
		//increase input mode..
		InputModeNext();
#endif
		break;
	
	case REMO_INFO:
		if (MenuGetLevel() == 0)
		{
			//toggle current video information.
			if (TaskNoSignal_getCmd() == TASK_CMD_DONE)
			{
#ifdef SUPPORT_FOSD_MENU
				FOsdDisplayInput();	
#else
				if ((ReadTW88(REG310) & 0x80) == 0)
				{
					FOsdCopyMsgBuff2Osdram(OFF);
				}
				FOsdWinToggleEnable(0);	//win0
#endif
			}				
		}
		return 1;
	
	case REMO_NUM0:
	case REMO_NUM1:
	case REMO_NUM2:
	case REMO_NUM3:
	case REMO_NUM4:
	case REMO_NUM5:
	case REMO_NUM6:
	case REMO_NUM7:
	case REMO_NUM8:
	case REMO_NUM9:
		break;
	
	case REMO_SELECT:
#ifdef SUPPORT_FOSD_MENU
		if (AutoKey)
			return 1;
		if (DisplayedOSD & FOSD_MENU)
			FOsdMenuProcSelectKey();
#endif
		break;

	case REMO_CHNUP:
#ifdef SUPPORT_FOSD_MENU
		if (DisplayedOSD & FOSD_MENU)
			FOsdMenuMoveCursor(FOSD_UP);
#else
  		dPuts("\n\rREMO_CHNUP pressed!!!");
#endif
		break;
	
	case REMO_CHNDN:
#ifdef SUPPORT_FOSD_MENU
		if (DisplayedOSD & FOSD_MENU)
			FOsdMenuMoveCursor(FOSD_DN);
#else
  		dPuts("\n\rREMO_CHNDN pressed!!!");
#endif
		break;
	
	case REMO_VOLUP:
#ifdef SUPPORT_FOSD_MENU
		if (DisplayedOSD & FOSD_MENU)
		{
			if (OnChangingValue)
				FOsdMenuValueUpDn(FOSD_UP);
			else					
				FOsdMenuProcSelectKey();			
		}
		else 
		{
			ChangeVol(1);
			DisplayVol();
		}
#else
  		dPuts("\n\rREMO_VOLUP pressed!!!");
#endif
		break;
	
	case REMO_VOLDN:
#ifdef SUPPORT_FOSD_MENU
		if (DisplayedOSD & FOSD_MENU)
		{
			if (OnChangingValue)
				FOsdMenuValueUpDn(FOSD_DN);
			else
				FOsdHighMenu();	
		}
		else 
		{
			ChangeVol(-1);
			DisplayVol();
		}
#else
  		dPuts("\n\rREMO_VOLDN pressed!!!");
#endif
		break;
	
	case REMO_MENU:
  		dPuts("\n\rREMO_MENU pressed!!!");
//#ifdef SUPPORT_FOSD_MENU
//		if(DisplayedOSD & FOSD_MENU)
//			FOsdHighMenu();
//		else {
//			if(GetOSDLangEE()==OSDLANG_KR)
//				InitFontRamByNum(FONT_NUM_PLUS_RAMFONT, 0);
//			else 
//				InitFontRamByNum(FONT_NUM_DEF12X18, 0);	//FOsdDownloadFont(0);	//12x18 default font
//			FOsdRamSetFifo(ON, 1);
//			FOsdMenuOpen();		
//		}
//#else
		if (MenuGetLevel() == 0)
			MenuStart();	
//#endif
		break;

	case REMO_EXIT:
  		dPuts("\n\rREMO_EXIT pressed!!!");

#ifdef SUPPORT_FOSD_MENU
		if (DisplayedOSD & FOSD_MENU)		
			FOsdMenuDoAction(EXITMENU);
#else
#endif
		break;

	case REMO_PIPON:
		dPuts("\n\rPIP Display mode change");

#if defined(SUPPORT_HDMI)
		if (InputMain == INPUT_HDMIPC || InputMain == INPUT_HDMITV)
		{
			dPuts("\n\rCEC_AUTO ON");
			//BK130402
			//WriteI2C_multi(0x68,0x21, 0x800, 0x01);
			global_CEC_flag = 1;
		}
#endif
		break;
				 
	case REMO_STILL:
		break;
	
	case REMO_SWAP:
		//Printf("\n\r=====SWAP InputMode:%bd",InputMain);
		if (InputMain == INPUT_BT656)
		{
			value = ReadTW88(REG006);
			if (value & 0x40)
				WriteTW88(REG006, value & ~0x40);
			else
				WriteTW88(REG006, value | 0x40);
		}			
		break;

	case REMO_PIPINPUT:
		dPuts("\n\rPIP input mode change");
		break;

	case REMO_TTXRED:
		break;

	case REMO_TTXYELLOW:
		break;

	case REMO_TTXGREEN:	
		break;
	
	case REMO_TTXCYAN:		
		break;

	case REMO_AUTO:
		break;

	case REMO_ASPECT:
		break;
	}

	return 1;
}

#ifdef SUPPORT_TOUCH
//extern BYTE	TouchStatus;
extern WORD PosX,PosY;
/**
* action for Touch
*/
BYTE ActionTouch(void)
{
	BYTE TscStatus;
	BYTE ret;
	WORD	xpos, ypos;

#ifdef DEBUG_TOUCH_SW
	dPrintf("\n\r==>Tsc Action");
#endif

	UpdateOsdTimerClock();

	TscStatus = TouchStatus;
	xpos = PosX;
	ypos = PosY;

	if(MenuGetLevel()==0) {
 
		if(TaskGetGrid()) {
			if(TscStatus == TOUCHCLICK || TscStatus == TOUCHDOUBLECLICK)
			{
				TaskSetGridCmd(NAVI_KEY_ENTER);
				return 0;
			}
		}
		if(TscStatus == TOUCHDOUBLECLICK) {
			MenuStart();
			SetTouchStatus(TOUCHEND); //BK111108
		}
		else if(TscStatus == TOUCHPRESS || TscStatus >= TOUCHMOVE) {
		}
		else if(TscStatus == TOUCHMOVED) {
			SetTouchStatus(TOUCHEND); //END
		}
		else if(TscStatus == TOUCHCLICK) {
			SetTouchStatus(TOUCHEND); //END
		}

		return 0;	
	}

	//in the menu
	if(TscStatus == TOUCHPRESS || (TscStatus >= TOUCHMOVE)) {
		//serial input mode.
		ret = MenuIsTouchCalibMode();
		if(ret) {
			CalibTouch(ret-1);

			MenuKeyInput(NAVI_KEY_ENTER); //goto next "+"
			//wait until TOUCH_END
			WaitTouchButtonUp();
			SetTouchStatus(TOUCHEND);
		}
//		else if(MenuIsSlideMode()) {
//			//need MOVE for slider bar
//			MenuCheckTouchInput(TscStatus, xpos, ypos);
//			//note: do not call SetTouchStatus(0);
//		}
		else {
			//
			//update focus.
			//
			MenuCheckTouchInput(TscStatus, xpos, ypos);
			//note: do not call SetTouchStatus(0);
		}
		return 0;
	}
	//else if(TscStatus == TOUCHMOVE) {
	//	//if lost focus, do something.
	//}
	else if(TscStatus == TOUCHCLICK || TscStatus == TOUCHDOUBLECLICK || TscStatus == TOUCHLONGCLICK || TscStatus == TOUCHMOVED) {
		//action mode

		if(TscStatus == TOUCHLONGCLICK) {
			if(MenuGetLevel()==1 || MenuIsSystemPage()) {
				//special.
				//use default value.
				MenuTouchCalibStart();
				SetTouchStatus(TOUCHEND);
				return 0;
			}
		}

		if(TscStatus == TOUCHMOVED)
			TscStatus = TOUCHCLICK;

		MenuCheckTouchInput(TscStatus,xpos, ypos);
		SetTouchStatus(TOUCHEND);  //BK110601
		return 0;
	}
	ePrintf("\n\rUnknown TscStatus :%bd", TscStatus);

	return 1;
}
#endif


/**
* check Key input
*
* keypad: LEFT, RIGHT, DOWN, UP, MENU, MODE
*/
BYTE CheckKeyIn( void ) 
{
	BYTE	key;

//#ifdef SUPPORT_TOUCH
//	//if we support the touch, we need to init TscAdc again
//	InitAUX(3);
//#endif

	//key = ReadKeyPad();
	key = GetKey(1);
	if(key == 0) {
		//sw key
		if(SW_key) {
			key = SW_key;
			SW_key=0;
		}
		if( key == 0)
			return 0;
	}

	//if(key== KEY_MENU)				   //only for William
	//	return REQUEST_POWER_OFF;

	UpdateOsdTimerClock();

#ifdef DEBUG_KEYREMO
	dPrintf("\n\rGetKey(1):%02bx ",key);
	switch(key) {
  	case KEY_RIGHT:	dPuts("Right"); break;
	case KEY_UP:	dPuts("Up"); break;
	case KEY_LEFT:	dPuts("Left"); break;
	case KEY_DOWN:	dPuts("Down"); break;
	case KEY_INPUT:	dPuts("Input"); break;
	case KEY_MENU:	dPuts("Menu"); break;
	default:		dPuts("unknown"); break;
	}
#endif

	if(TaskGetGrid()) {
		switch(key) {
	  	case KEY_RIGHT:
			TaskSetGridCmd(NAVI_KEY_RIGHT);
			break;
		case KEY_UP:
			TaskSetGridCmd(NAVI_KEY_UP);
			break;
		case KEY_LEFT:
			TaskSetGridCmd(NAVI_KEY_LEFT);
			break;
		case KEY_DOWN:
			TaskSetGridCmd(NAVI_KEY_DOWN);
			break;
		case KEY_INPUT:
		case KEY_MENU:
			TaskSetGridCmd(NAVI_KEY_ENTER);
			break;
		}
		return 0;
	}


	switch ( key ) {
	  case 	KEY_RIGHT:
#ifdef SUPPORT_FOSD_MENU
		if(  DisplayedOSD & FOSD_MENU  ) {
			if( OnChangingValue ) 	FOsdMenuValueUpDn(FOSD_UP );
			else					FOsdMenuProcSelectKey();			
		}
		else 
#endif
#if defined(SUPPORT_SPIOSD)
		if(MenuGetLevel())
			MenuKeyInput(NAVI_KEY_RIGHT);
		else
#endif
		{
#if defined(SUPPORT_FOSD_MENU)
			ChangeVol(1);
			DisplayVol();
#endif
		}
    	break;

	  case 	KEY_UP:
#ifdef SUPPORT_FOSD_MENU
		if(DisplayedOSD & FOSD_MENU)
			FOsdMenuMoveCursor(FOSD_UP);
		else
#endif
#if defined(SUPPORT_SPIOSD)
		if(MenuGetLevel())
			MenuKeyInput(NAVI_KEY_UP);
		else
#endif
		{
			; //do nothing
		}
    	break;

	  case 	KEY_MENU:
#if defined(SUPPORT_FOSD_MENU) && defined(SUPPORT_SPIOSD)

		if((DisplayedOSD & FOSD_MENU)==0
		&& (MenuGetLevel()==0)) {
			//activate menu. but, which one ?
			//if key was UP=>DOWN=>MENU, i will executes FontOSD
			if(previous_key0 == KEY_UP
			&& previous_key1 == KEY_DOWN) {
				if(GetOSDLangEE()==OSDLANG_KR)
					InitFontRamByNum(FONT_NUM_PLUS_RAMFONT, 0);
				else 
					InitFontRamByNum(FONT_NUM_DEF12X18, 0);	//FOsdDownloadFont(0);	//12x18 default font
				FOsdRamSetFifo(ON, 1);
				FOsdMenuOpen();		
			}
			else {
				MenuStart();
			}
		}
		else if(DisplayedOSD & FOSD_MENU) {
			FOsdHighMenu();
		}
		else //assume  MenuGetLevel() != 0.
			MenuKeyInput(NAVI_KEY_ENTER);
#elif defined(SUPPORT_FOSD_MENU)
		if(DisplayedOSD & FOSD_MENU)
			FOsdHighMenu();	
		else {
			if(GetOSDLangEE()==OSDLANG_KR)
				InitFontRamByNum(FONT_NUM_PLUS_RAMFONT, 0);
			else 
				InitFontRamByNum(FONT_NUM_DEF12X18, 0);	//FOsdDownloadFont(0);	//12x18 default font
			FOsdRamSetFifo(ON, 1);
			FOsdMenuOpen();		
		}
#else
		if(MenuGetLevel()==0)
			MenuStart();	
		else
			MenuKeyInput(NAVI_KEY_ENTER);	
#endif
    	break;

	  case 	KEY_LEFT:
#ifdef SUPPORT_FOSD_MENU
		if(  DisplayedOSD & FOSD_MENU  ) {
			if( OnChangingValue ) FOsdMenuValueUpDn(FOSD_DN );
			else FOsdHighMenu();	
		}
		else 
#endif
#if defined(SUPPORT_SPIOSD)
		if(MenuGetLevel())
			MenuKeyInput(NAVI_KEY_LEFT);
		else
#endif
		{
#if defined(SUPPORT_FOSD_MENU)
			ChangeVol(-1);
			DisplayVol();
#endif
		}
    	break;

	  case 	KEY_DOWN:
#ifdef SUPPORT_FOSD_MENU
		if(DisplayedOSD & FOSD_MENU)
			FOsdMenuMoveCursor(FOSD_DN);
		else
#endif
#if defined(SUPPORT_SPIOSD)
		if(MenuGetLevel())
			MenuKeyInput(NAVI_KEY_DOWN);
		else
#endif
		{
			; //do nothing
		}
    	break;

	  case 	KEY_INPUT:
		if(MenuGetLevel())
			MenuEnd();
		else
			InputModeNext();
    	break;
	}
	previous_key0 = previous_key1;					   	
	previous_key1 = key;

	return 0; //for William
}


//=============================================================================
//		BYTE CheckHumanInputs( void )
//=============================================================================
/**
* check Human Inputs. (Keypad, touch, remocon, UART0)
*
* Just Check, NO Action.
* extern BYTE TouchChanged;
* @return
*	-0: No Input
*	-other: input type
*/
BYTE	CheckHumanInputs( BYTE skip_tsc )
{
#ifdef SUPPORT_TOUCH
#else
	BYTE temp = skip_tsc;
#endif

	//
	//check remo input
	//
	//if(CheckRemo())
	//	return 1;
#if 0
	if(RemoDataReady)
		return HINPUT_REMO;

#else
	BYTE AutoKey,  _RemoDataCode;
	BYTE ret=0;

	if( IsRemoDataReady(&_RemoDataCode, &AutoKey) )	{
		//ret = ActionRemo( _RemoDataCode, (BYTE)AutoKey) ;

//#ifdef DEBUG_KEYREMO
		//dPrintf("\n\r**** Remo: %02bx, %02bx ", _RemoDataCode, (BYTE)AutoKey);
		//dPrintf("\n\r%02bx,%02bx ", _RemoDataCode, AutoKey);
//#endif
		return HINPUT_REMO;
	}
#endif





	//
	//check keypad
	//
//#ifdef SUPPORT_ANALOG_SENSOR_NEED_VERIFY
//	//CheckKeyIn();
	if ( ReadKeyPad() ) {
		//dPuts("\n\rGet Key data");
		return ( HINPUT_KEY );		// input!!!
	}
//#endif
	//
	//check touch
	//	
#ifdef SUPPORT_TOUCH
	if(skip_tsc==0) {
		//it is too sensitive...use GetTouch2()
		//if ( CpuTouchPressed || CpuTouchChanged ) {
		//	dPrintf("\n\rSenseTouch CpuTouchPressed:%bd CpuTouchChanged:%bd",CpuTouchPressed,CpuTouchChanged);
		//	return ( HINPUT_TSC );		// input!!!
		//}
		ret = GetTouch2();
		if(ret)
			return ( HINPUT_TSC );		// input!!!
	}
#endif

	//
	//check RS232 monitor input
	//	
	if (RS_ready()) {
		//dPuts("\n\rGet serial data");
		return ( HINPUT_SERIAL );		// input!!!
	}
	return (HINPUT_NO);
}


