/**
 * @file
 * main.c 
 * @author Harry Han
 * @author YoungHwan Bae
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	main file
 * @section DESCRIPTION
 *	- CPU : DP80390
 *	- Language: Keil C
 *  - See 'Release.txt' for firmware revision history 
 ******************************************************************************
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
#include <intrins.h>
#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "Global.h"
#include "CPU.h"
#include "Printf.h"
#include "util.h"
#include "Monitor.h"

#include "I2C.h"
#include "SPI.h"

#include "main.h"
#include "misc.h"
#include "Remo.h"
#include "TouchKey.h"
#include "eeprom.h"
#include "e3prom.h"

#include "Settings.h"
#include "InputCtrl.h"
#include "decoder.h"
#include "Scaler.h"
#include "ImageCtrl.h"
#include "aRGB.h"
#include "DTV.h"
#include "measure.h"
#include "BT656.h"

#include "SOsd.h"
#include "FOsd.h"
#include "SpiFlashMap.h"
#include "SOsdMenu.h"
#include "Demo.h"
#include "Debug.H"


//-----------------------------------------------------------------------------
/**
* "system no initialize mode" global variable.
*
* If P1_5 is connected at the PowerUpBoot, 
*   it is a system no init mode (SYS_MODE_NOINIT).
* If the system is a SYS_MODE_NOINIT, 
*  FW will skips the initialize routine, 
*  and supports only the Monitor function.
*  and SYS_MODE_NOINIT can not support a RCDMode and a PowerSaveMode.
* But, if the system bootup with normal, 
*  the P1_5 will be worked as a PowerSave ON/OFF switch.
*/
//-----------------------------------------------------------------------------
BYTE CurrSystemMode;

//-----------------------------------------------------------------------------
// Interrupt Handling Routine Variables			                                               
//-----------------------------------------------------------------------------
BYTE SW_Video_Status;
BYTE SW_INTR_cmd;
#define SW_INTR_VIDEO_CHANGED	1

//-----------------------------------------------------------------------------
// Task NoSignal
//-----------------------------------------------------------------------------
#define TASK_FOSD_WIN	0
#define NOSIGNAL_TIME_INTERVAL	(10*100)

XDATA BYTE Task_NoSignal_cmd;		//DONE,WAIT_VIDEO,WAIT,RUN,RUN_FORCE
XDATA BYTE Task_NoSignal_count;		//for dPuts("\n\rTask NoSignal TASK_CMD_WAIT_VIDEO");

//-----------------------------------------------------------------------------
// MovingGrid TASK ROUTINES				                                               
//-----------------------------------------------------------------------------
XDATA BYTE Task_Grid_on;
XDATA BYTE Task_Grid_cmd;

//-----------------------------------------------------------------------------
// Freerun value				                                               
//-----------------------------------------------------------------------------
XDATA WORD global_Freerun_hTotal;
XDATA WORD global_Freerun_vTotal;

//-----------------------------------------------------------------------------
// Fast Toggle between CVBS & DTV(HDMI)	
// NOTE: You have to turn off SPIOSD.			                                               
//-----------------------------------------------------------------------------
#if defined(SUPPORT_FAST_INPUT_TOGGLE)
BYTE g_fast_toggle;
BYTE g_cvbs_checked;
BYTE g_hdmi_checked;
#endif

//-----------------------------------------------------------------------------
// MHL CEC				                                               
//-----------------------------------------------------------------------------
#if defined(SUPPORT_HDMI)
BYTE global_CEC_flag;
#endif

//-----------------------------------------------------------------------------
// MAIN LOOP				                                               
//-----------------------------------------------------------------------------
#define RET_MAIN_LOOP_PSM_BY_REMO	1
#define RET_MAIN_LOOP_PSM_BY_PORT	2

//=============================================================================
// Prototype				                                               
//=============================================================================
static void init_global_variables(void);
static void print_firmware_info(void);
void InitCore(BYTE fManual);
BYTE check_NoInit_mode_port(void);
BYTE InitSystem(BYTE _fPowerUpBoot);
static BYTE main_loop(void);
static void SystemPowerSave(void);
static void SystemPowerResume(void);
static BYTE WaitPowerOn(void);
static void InterruptPollingHandlerRoutine(void);
void CheckAndClearOSD(void);

//-----------------------------------------------------------------------------
// Task NoSignal
//-----------------------------------------------------------------------------
void NoSignalTask( void );
void NoSignalTaskOnWaitMode(void);
void TaskNoSignal_setCmd(BYTE cmd); 
BYTE TaskNoSignal_getCmd(void); 

//-----------------------------------------------------------------------------
// MovingGrid TASK ROUTINES				                                               
//-----------------------------------------------------------------------------
//extern void MovingGridTask( void );
void TaskSetGrid(BYTE onoff);
BYTE TaskGetGrid(void);
void TaskSetGridCmd(BYTE cmd);
BYTE TaskGetGridCmd(void);


//=============================================================================
// MAIN
//=============================================================================
/**
main & flow
+--init_global_variables
+--init_cpu
+--InitCore
|  +--init_spiflash_chip
+--InitSystem
|  +--E3P_Configure
|  +--Init8836AsDefault
|  +--InitGpioDefault
|  +--OutputEnablePin
|  +--InitClockAsDefault
|  +--FrontPanel_StartUP
|  +--FontOsdInit
|  +--SOsd_init
|  +--StartVideoInput
|  |  +--InputMainEE = GetInputMainEE()
|  |  +--ChangeInput( InputMainEE )
|  |     +--ChangeCVBS()
|  |     +--ChangeSVIDEO()
|  |     +--ChangeComponent()
|  |     +--ChangePC()
|  |     +--ChangeHDMI()
|  |     +--ChangeBT656Loop()
|  |     +--ChangeLVDSRx()
|  |     |
|  |     +--InitInputAsDefault
|  |     +--ret=CheckAndSet()
|  |     +--if(ret==ERR_SUCCESS)
|  |     +--  VInput_enableOutput
|  |     +--else
|  |     +--  VInput_gotoFreerun
|  |
|  +--InitLogo1
|  +--PowerUpLedBackLight
|  +--EnableRemoInt
|  +--InitAuxADC
|  +--RemoveLogoWithWait
|  +--EnableWatchdog
|
+--while(1) {
+----main_loop
|    +--Monitor
|    +--if(g_access==0) continue;
|    +--CheckKeyIn
|    +--CheckRemo
|    +--GetTouch2
|    +--MovingGridTask
|    +--InterruptPollingHandlerRoutine
|    +--NoSignalTask
|    +--I2CCMD_exec_main
|    
+----SystemPowerSave
+----WaitPowerOn
+----SystemPowerResume
+--}
*/
void main(void)
{
	BYTE ret;

#ifdef SUPPORT_WATCHDOG
	if (SFRB_WTRF) 
	{
        /* If WTRF was 1, system was reboot by Watchdog.
           Watchdog only reboot MCU, not VideoCore.
        */
#if 0   /* Easy & safe way..*/
        Puts("LV Reboot\n");
        LV_Reset_TW8836B();
        /* ByeBye */
#endif
        /* 
        If you want to use a customized start up, 
         copy START390.A51 from /Keil/C51/LIB to the project directory.
        If we skip the DATA/XDATA clear routine on the start up (START390.A51),
         We can use the old DATA/XDATA values these are not initialized by default value.
        Add below code before "IF IDATALEN <> 0" on your START390.A51.
            ;------------------------------
            ; if watchdog, skip initialize
            ;------------------------------
            	JNB WTRF,NORMAL
            	LJMP	?C_START
            NORMAL:	
            IF IDATALEN <> 0
        */
        init_cpu(1);        /* with Watchdog Reset Flag */
        EnableRemoInt();    /* enable remote controller */
        InitAuxADC();	    /* init touch */
    	EnableWatchdog(1);  /* enable watchdog */
        if (menu_on)
		{
            /* If memu was on, we have to recover focus and select.
               This code only recover focus */
            curr_menu->focus = curr_menu_watchdog_focus;
            curr_menu->select = curr_menu_watchdog_select;
        }
    }
    else
#endif 
    {
	    init_global_variables();    /*init global variable */			
		init_cpu(0);				/*init 8051 MCU*/
		print_firmware_info();		/*print MODEL VERSION*/
		InitCore(0);	

    	/*
    	if "access" becomes 0 in the InitCore(),
    	FW will be a no_init mode.
    	In this no_init mode, FW will support only the monitor.
    	*/  
    	if (g_access == 0)
		{
        	/* In this no_init mode, FW will support only the monitor.	*/  
    		Puts("***SKIP_MODE_ON***\r\n");
    		DebugLevel=3;
    		Puts("need **init core***init ee***init system***\r\n");
    		Puts("and **cache on**access on**task on**isr on\r\n*");
        }

		SetMonAddress(TW88I2CAddress);
    	Prompt(); /*first prompt*/

    	//==================================================
    	// Init System
    	//==================================================
        InitSystem(1);
    }

#ifdef DEBUG_TIME
	PrintSystemClockMsg("start loop");
#endif

	Prompt(); //second prompt

    /* Enable INT0: Chip Interrupt */
    SFRB_EX0  = 1;			

	while (1) 
	{
		//==================================================
		// MAIN LOOP
		//==================================================
		ret = main_loop();		
		
		Printf("main_loop() ret %bd\r\n", ret);		
		if (ret==RET_MAIN_LOOP_PSM_BY_REMO || ret==RET_MAIN_LOOP_PSM_BY_PORT)
		{
			SystemPowerSave();		//move to PowerSaveMode	
			WaitPowerOn();			//wait PowerOn input from keypad, Remo. not Touch
			SystemPowerResume();	//resume
		}
		//else..DO NOTHING
	}	
	//you can not be here...
}

//-----------------------------------------------------------------------------
#ifdef DEBUG_WATCHDOG
/*!
 * Debug function for RestartWatchdog.
 * It updates RWT, and print out SystemClock.
 * We can estimate the watchdog timer duration.
 */
static void DebugRestartWatchdog(void)
{
	static DWORD wdt_last=0;
	DWORD wdt_diff;
	DWORD curr_clock;

	F_watch = 0;

	/*refresh watchdog counter.*/
	RestartWatchdog();	

	SFRB_ET0 = 0;
	curr_clock = SystemClock;
	SFRB_ET0 = 1;

	wdt_diff = curr_clock - wdt_last;
	wdt_last = curr_clock;
	ePrintf("Watchdog Interrupt !!! %02bx  %ld.%ldsec", 
		SFR_WDCON, curr_clock/100, curr_clock%100);
	ePrintf(" diff:%ld.%02ldsec\r\n",wdt_diff/100, wdt_diff%100);

	/*refresh watchdog counter again, because, you are using a printf.*/
	RestartWatchdog();	
}
#endif


#ifdef SUPPORT_TOUCH
/* to reduce main_loop */
static BYTE IsTouchInput(void)
{
	BYTE tc,tp;
	BYTE ret;
	static BYTE savedCpuTouchChanged=0; 
	static BYTE savedCpuTouchPressed=0; 

	SFRB_ET1 = 0;
	tc = CpuTouchChanged;
	tp = CpuTouchPressed;
	SFRB_ET1 = 1;

	ret = 0;
	if(savedCpuTouchChanged != tc
	&& savedCpuTouchPressed != tp) {
		ret = 1; 	//something happen
	}
	savedCpuTouchChanged = tc;
	savedCpuTouchPressed = tp;
	return ret; 
}
#endif

//=============================================================================
// MAIN LOOP				                                               
//=============================================================================
/**
 * main_loop
 *
 * @param void
 * @return 
 *	0: 
 *	1:PowerSaveMode by Remo
 *	2:PowerSaveMode by Port
 *
 * @see ext0_int
 * @see	InterruptPollingRoutine
 * @see	InterruptHandlerRoutine
 * @see	I2CCMD_exec_main
 * @see Monitor
 * @see	CheckKeyIn
 * @see	CheckRemo
 * @see	GetTouch2
 * @see	ActionTouch
 * @see CheckAndClearOSD
 * @see	NoSignalTask
 * @see NoSignalTaskOnWaitMode
 *
*/
BYTE main_loop(void)
{
	BYTE ret;

	//---------------------------------------------------------------
	//			             Main Loop 
	//---------------------------------------------------------------
	while (1)
	{
		//-------------- Check Serial Port ---------------------
		if (RS_ready()) 
			Monitor();				// for new monitor functions

#ifdef SUPPORT_UART1
		if (RS1_ready())
			Monitor1();				// for UART1
#endif

		//-------------- Check Watchdog ------------------------
#ifdef DEBUG_WATCHDOG
		if (F_watch)
			DebugRestartWatchdog();
#elif defined(SUPPORT_WATCHDOG)
		/*refresh watchdog counter.*/
		RestartWatchdog();		
#endif

		//-------------- block access routines -----------------
		if (g_access == 0) 
			continue;		
		/* NOTE: If you donot have an access, You can not pass */

 		//-------------- Check Keypad input --------------------
		if ((CpuAUX3 >= 0x100) || SW_key)
		{
			ret = CheckKeyIn();
			if (ret == REQUEST_POWER_OFF)
			{
				Printf("\n\r===POWER SAVE===by Keypad");
				ret = RET_MAIN_LOOP_PSM_BY_REMO;
				break;
			}
		}
		
 		//-------------- Check Remote Controller ---------------
		if (RemoDataReady)
		{
			ret = CheckRemo();
			if (ret == REQUEST_POWER_OFF && CurrSystemMode==SYS_MODE_NORMAL)
			{
				ePrintf("\n\r===POWER SAVE===by Remo");
				ret = RET_MAIN_LOOP_PSM_BY_REMO;
				break;
			}
		}

		//-------------- Check Touch ---------------
#ifdef SUPPORT_TOUCH
#ifdef DEBUG_TOUCH_SW
		if (TraceAuto) 
			TraceTouch();
#endif
		if (IsTouchInput())
		{
			ret = GetTouch2();
			if (ret)
				ActionTouch();		
		}
#endif

 		//-------------- Check special port for RCD mode------
#ifdef SUPPORT_RCD
		if (ReadTW88(REG007) & 0x08)
		{
			;  //conflict with BT656Enc...just give up.
		}
		else
		{
			//It was using IsBackDrivePortOn().
			//Now, we are using EEPROM value.
			if (EE_Read(EEP_BOOTMODE) == 1)
			{
				if (CurrSystemMode == SYS_MODE_NORMAL)
				{
					//move to RCD mode.
					CurrSystemMode = SYS_MODE_RCD;
					InputMain = 0;	//dummy
					InitRCDMode(0);
				}
			}
			else
			{
				if (CurrSystemMode==SYS_MODE_RCD)
				{
					DWORD BootTime;
					
					BootTime = SystemClock;
					//move to normal mode.
					CurrSystemMode = SYS_MODE_NORMAL;
					//turn off a parkgrid task first
					TaskSetGrid(OFF);
					SOsd_Enable( 0, OFF );
					SOsd_Enable( 1, OFF );
					SOsd_UpdateReg(0, 1);
					SpiOsdEnable(OFF);
	
					InitSystem(0);
	
					BootTime = SystemClock - BootTime;
					ePrintf("\n\rBootTime(RCD2Normal):%ld.%ldsec", BootTime/100, BootTime%100 );
				}
			}
		}
#endif	
		
		//============== Task Section ==========================
		if (Task_Grid_on)
			MovingGridTask();

#ifdef SUPPORT_RCD
		//skip VideoISR on RCD.
		if (CurrSystemMode==SYS_MODE_RCD)
			continue;					
#endif

		//-------------- Check TW8836 Chip Interrupt -------------
		if (INT_STATUS || VH_Loss_Changed)
			InterruptPollingHandlerRoutine();

		if (INT_STATUS3)
		{
			extern DWORD ExtIntCount;
			Printf("\n\rINT_STATUS3:%bx  count: %ld",INT_STATUS3, ExtIntCount );
			INT_STATUS3 = 0;
		}

		//-------------- Check OSD timer -----------------------
#if 0
		if (OsdTime && OsdTimerClock)
		{
			Puts("O");
			CheckAndClearOSD();
		}
#endif
		//-------------- HDMI Section -------------------------

		//-------------- NoSignal Task-------------------------
		NoSignalTask();
		NoSignalTaskOnWaitMode(); //Check each input status if WAIT_MODE

		//-------------- I2CCMD -------------------------------
#if defined(SUPPORT_I2CCMD_SERVER)
		if (F_i2ccmd_exec)
		{
			F_i2ccmd_exec = 0;
			I2CCMD_exec_main();
		}
#endif

		//-------------- test MAIN loop speed ------------------
#ifdef DEBUG_MAIN_LOOP
        /* to measure, turn off BT656 output. */
        /* On 104MHz, it uses 20uS */
        /* On 52MHz, it use 40uS, */
        P1_6 = !P1_6;
#endif	

	} //..while(1)

	return ret;
}

//=============================================================================
// Init QuadIO SPI Flash			                                               
//=============================================================================


//check port 1.5. 
//if high, it is a skip(NoInit) mode.
//P1.5 pin is a multifunction pin that share with TCREV and GPIO41.
//If TCON, we can not read. Select TTL here. (REG007=0x02).
BYTE check_NoInit_mode_port(void)
{
	BYTE bTemp;
    BYTE ret;

#ifdef MODEL_TW8836DEMO
    return 0;
#endif

	bTemp = ReadTW88(REG007);
	WriteTW88(REG007, 0x02);
	if (PORT_NOINIT_MODE == 1) 
        ret = 1;   /* NoInitMode */
    else
        ret = 0;
	WriteTW88(REG007, bTemp);

    return ret;
}

//-----------------------------------------------------------------------------
/**
* init core
*
* prepare SPIFLASH QuadIO.
* enable chip interrupt.
*
* If you are using NOINIT, you should manually type "init core".
*/
void InitCore(BYTE fManual)
{
	BYTE bTemp;

	Puts("InitCore");

    if (!fManual)
	{
        bTemp = check_NoInit_mode_port();
        if (bTemp)
		{
            CurrSystemMode = SYS_MODE_NOINIT;
            //To turn on the SKIP_MODE, assign g_access as 0.
            g_access = 0;
            Puts("\n");
            return;
        }
    }

	/*start from 27MHz.*/
	//in normal routine, default is 27MHz.
    //If system starts up with a I2CSPI or a Watchdog, it can be other value.
	SpiClkRecover27MSource();

	/*turn on CACHE.*/
	Puts(" Cache\n");
	SFR_CACHE_EN = 0x01;	//cache ON.

	/* Initialize SpiFlash chip */
	bTemp = init_spiflash_chip();
	if (bTemp == ERR_FAIL)
	{
        //SPI DMA does not work.
		//we need a reboot.
		CurrSystemMode = SYS_MODE_NOINIT;
        Puts("Fail SPIFLASH\n");
		g_access = 0;
		
		return; // ERR_FAIL;
	}

    /* Enable Chip Interrupt. */
    WriteTW88(REG002, 0xFF);	// Clear Pending Interrupts
    WriteTW88(REG003, 0xEE);	// enable SW. enable SW INTR7FF
}

/**
* InitLVDS Tx
*
* REG640~REG647
*/
#ifdef PANEL_LVDS
void InitLVDSTx(void)
{
	WriteTW88(REG006, (ReadTW88(REG006) & 0xE0) | 0x06);

	WriteTW88(REG640, 0x0C);
	WriteTW88(REG641, 0x00);
	WriteTW88(REG642, 0x41);
	WriteTW88(REG643, 0x60);
	WriteTW88(REG644, 0x00);
	WriteTW88(REG647, 0x02);
}
#endif

#if defined(SUPPORT_I2C_MASTER)
void CheckGpioExpender(BYTE fPowerUpBoot)
{
	BYTE value;

	if(fPowerUpBoot) {
#if defined(DEBUG_I2C)
		Printf("\n\rdef: SDA:%bd, SCL:%bd", (BYTE)I2C_SDA, (BYTE)I2C_SCL);
		I2C_SCL = 1;
		I2C_SDA = 1;
		Printf("\n\rup:  SDA:%bd, SCL:%bd", (BYTE)I2C_SDA, (BYTE)I2C_SCL);
		I2C_SDA = 0;
		I2C_SCL = 0;
		Printf("\n\rdown:SDA:%bd, SCL:%bd", (BYTE)I2C_SDA, (BYTE)I2C_SCL);
		I2C_SCL = 1;
		I2C_SDA = 1;
		Printf("\n\rup:  SDA:%bd, SCL:%bd", (BYTE)I2C_SDA, (BYTE)I2C_SCL);
#endif

		//check I2C port.
		if(I2C_SCL == 0)
			I2C_SCL = 1;
		if(I2C_SCL == 0)
			Printf("\n\rWARN:I2C_SCL low");
		if(I2C_SDA == 0)
			I2C_SDA = 1;
		if(I2C_SDA == 0)
			Printf("\n\rWARN:I2C_SDA low");

		/*
		*	Check I2C GPIO Expendor.
		*	If I2C device holds SCL, it can hangup the system.
		*	0: success
		*	1: NAK
		*	2: I2C dead
		*/
		value=CheckI2C(I2CID_SX1504);
		switch(value) {
		case 0:		Puts("\n\rI2CID_SX1504:Pass");			break;
		case 1:		Puts("\n\rI2CID_SX1504:NAK");			break;
		case 2:		Puts("\n\rI2CID_SX1504:Dead");			break;
		default:	Printf("\n\rI2CID_SX1504:%bx",value);	break;
		}
#ifdef DEBUG_I2C
		if(value==0)
			//ok. print the chip default value
			Printf("\n\rI2CID_SX1504 %02bx %02bx %02bx %02bx",
				ReadI2CByte(I2CID_SX1504, 0), 
				ReadI2CByte(I2CID_SX1504, 1),
				ReadI2CByte(I2CID_SX1504, 2),
				ReadI2CByte(I2CID_SX1504, 3));
#endif
		if(g_access) {
			//turn off FPPWC & FPBias. make default
			//	0x40 R0 R1 is related with FP_PWC_OnOff
			WriteI2CByte( I2CID_SX1504, 1, 0xFF);		//RegDir:	input 
			WriteI2CByte( I2CID_SX1504, 0, 0xFF);		//RegData:	FPBias OFF. FPPWC disable.
			WriteI2CByte( I2CID_SX1504, 2, 0x00);		//disable PullUp
			WriteI2CByte( I2CID_SX1504, 3, 0x00);		//disable Pulldown
			//print the FW default value
			Printf("\n\rI2CID_SX1504 %02bx %02bx %02bx %02bx",
				ReadI2CByte(I2CID_SX1504, 0), 
				ReadI2CByte(I2CID_SX1504, 1),
				ReadI2CByte(I2CID_SX1504, 2),
				ReadI2CByte(I2CID_SX1504, 3));
		}
	}
}
#endif

/**
* Init Clock as Default
*
* @param mode
*	0:SYNC 1:ASYNC
*/

void InitClockAsDefault(BYTE mode)
{
	DWORD mcu_clk;
	DWORD dTemp = mode;
	BYTE bTemp;	

	//start from safe 27MHz.
    //start from HW default value.
	WriteTW88(REG4E1,0x06); 

	if(spiflash_chip == NULL)
		return;
	if(spiflash_chip->mid == 0)
		return;

	Puts("\nInitClockAsDefault");
	ePrintf(" %02bx:%02bx:%02bx ",
		spiflash_chip->sspll_0f8,
		spiflash_chip->sspll_0f9,
		spiflash_chip->sspll_0fa);
	/*set sspll1 for spi & mcu clock*/
	WriteTW88(REG0F8,spiflash_chip->sspll_0f8);
	WriteTW88(REG0F9,spiflash_chip->sspll_0f9);
	WriteTW88(REG0FA,spiflash_chip->sspll_0fa);
	WriteTW88(REG0FD,spiflash_chip->sspll_0fd);
 	WriteTW88(REG0FC, ReadTW88(REG0FC) & ~0x80); //SSPLL1 Power up

	/*sspll2 for pclk & pclko*/
#if defined(PANEL_1024X600)
	ePuts("SSPLL2 108M/1/2");
	Sspll2SetFreqReg(SSPLL_108M_REG);

 	WriteTW88(REG0EC, ReadTW88(REG0EC) & ~0x80);	//SSPLL2 Power up			
	WriteTW88(REG0ED,0x23); //POST=2(div1),VCO=54~108,ChargePump:3
#else
	ePuts("SSPLL2 72M/1/2");	
	Sspll2SetFreqReg(SSPLL_72M_REG);
		
 	WriteTW88(REG0EC, ReadTW88(REG0EC) & ~0x80);	//SSPLL2 Power up			
	WriteTW88(REG0ED,0x24); //POST=0,VCO=54~108,ChargePump:4
#endif
	//Errata150407
	//BK150409 restore Errara150407
	WriteTW88(REG0F6, 0x00);
	bTemp = ReadTW88(REG20D) & ~0x33;
	WriteTW88(REG20D, bTemp | 0x01);	//Div2, wothout Pol.

	//PLLClk for Spi & MCU
	PllClkSetSource(spiflash_chip->pllclk_s);
	PllClkSetDividerReg(spiflash_chip->pllclk_d);

	if(spiflash_chip->mcuclk_d) {
        /* ASYNC ? */
		SpiClk_SetAsync(spiflash_chip->mcuclk_d,async_wait_value[spiflash_chip->mcuclk_d], ON,ON);
		dTemp = spiflash_chip->typical_speed;
		mcu_clk = McuClkGetFreq(dTemp);

		I2C_delay_base = (mcu_clk -1) / 27;
		I2C_delay_base++;
	}
	else {
		SpiClk_SetSync();
		I2C_delay_base = (spiflash_chip->typical_speed -1) / 27;
		I2C_delay_base++;
	}
	DumpClock();
}

//=============================================================================
// InitSystem
//=============================================================================
//-----------------------------------------------------------------------------
/**
* initialize TW8836 System
*
*	CheckEEPROM
*	InitWithNTSC
*	Set default GPIO
*	Sspll1PowerUp
*	Startup DCDC
*	download Font
*	Start Video input
*	InitLogo1
*	Powerup LED
*	Remove Logo
*	Init Touch
* 	enable remocon
*
* @param bool fPowerUpBoot
* 	if fPowerUpBoot is true, 
*		download default value,
*		turn on Panel
* @return 0:success
* @see InitWithNTSC
* @see InitGpioDefault
* @see Sspll1PowerUp
* @see FrontPanel_StartUP
* @see FontOsdInit
* @see StartVideoInput
* @see InitLogo1
* @see PowerUpLedBackLight
* @see RemoveLogoWithWait
* @see OsdSetTime
* @see OsdSetTransparent
* @see BackLightSetRate
* @see MeasSetErrTolerance
* @see InitTouch
* @see UpdateOsdTimerClock
*/
BYTE InitSystem(BYTE _fPowerUpBoot)
{
	BYTE ee_mode;
	BYTE value;
	BYTE FirstInitDone;
	BYTE fPowerUpBoot;	//to solve the compiler bug, use a variable.

	if (g_access == 0)
		//do nothing.
		return 0;

	fPowerUpBoot = _fPowerUpBoot;

	//check EEPROM
#ifdef USE_SFLASH_EEPROM
	E3P_Configure();
#endif

#ifdef NO_EEPROM
	ee_mode = 0;
#else
	ee_mode = CheckEEPROM();
#endif

	if (ee_mode == 1)
	{
		//---------- if FW version is not matched, initialize EEPROM data -----------
		DebugLevel = 3;

		#ifdef USE_SFLASH_EEPROM
		E3P_Format();
		E3P_Init();
		#endif
	
		InputMain = 0xff;	// start with saved input
		InitializeEE();	 	//save all default EE values.
		InputMain = 0;
	
		DebugLevel = 0;
		SaveDebugLevelEE(DebugLevel);

		//check MAX SPI speed 108MHz~132MHz
		CheckSpiClock(32);	

		//it will become the NoInit mode
		Printf("\n\r=================================");
		Printf("\n\remergency recover...Please reboot");
		Printf("\n\r=================================");

		g_access = 0;
		DebugLevel = 0;

		return 0;
	}
	
#ifdef SUPPORT_RCD
	/* 
		TW8836B EVB does not have extra GPIO pin to check this mode.
		Previous FW using P1.6 to detect RCD activation,
		but, this P1.6 also used for BT656Encoder signal.
		Now, new FW using EE[0x0F] position to support RCD mode.
		If you want RCD, type "ee w f 1" and then reboot.

		Note: Current design have a conflict when it reboot by watchdog.
	*/
	if (EE_Read(EEP_BOOTMODE) == 1)
	{
		//If it is a RCD mode, FW will init only minimum routines for RCD.
		CurrSystemMode = SYS_MODE_RCD; 
		InputMain = 0;	//dummy
		InitRCDMode(1);
		return 0;
	}
#endif

	/* read debug level */
	DebugLevel = GetDebugLevelEE();
	if (DebugLevel != 0)
		ePrintf("\n\r===> Debugging is ON (%02bx)", DebugLevel);

	Printf("\n\rInitSystem(%bd)", fPowerUpBoot);

	if ((SFR_WDCON & 0x04))
	{
		Printf("WDCON: 0x%bx", SFR_WDCON);
		//EE_Increase_Counter_Watchdog();
    }

	/* read main input */
	InputMain  = GetInputMainEE();
	InputBT656 = GetInputBT656EE();

	//
	//set default setting.
	//
	if (fPowerUpBoot)
	{
		//Init HW with default.
		Init8836AsDefault(InputMain, 1); 
		I2C_delay_base = 1;
							
		//------------------
		//first GPIO position.
		//GPIO needs TRI_EN=0.
		InitGpioDefault();

		Puts("\nEnable OutputPin");
		OutputEnablePin(OFF, ON);		//Output enable. FP data: not yet

#if defined(SUPPORT_I2C_MASTER)
		if (DebugLevel >= DEBUG_WARN)
			CheckGpioExpender(fPowerUpBoot);
#endif

		InitClockAsDefault(0);	//0:SYNC, 1:ASYNC

#ifdef PANEL_LVDS
		InitLVDSTx();
#endif
	}

	//------------------------
	// init HDMI chip for EDID & HDCP.
#if  defined(SUPPORT_HDMI_TW8837)
	//Init_HdmiSystem();
#endif

	FirstInitDone = 0;

	//---------------------
	// turn on DCDC
	//---------------------
	if (fPowerUpBoot)
		FrontPanel_StartUP();

	//---------------
	//init FontOSD 
	FontOsdInit();
	FOsdSetDeValue();
	FOsdIndexMsgPrint(FOSD_STR1_TW8836);
	FOsdWinEnable(0, OFF);	//win0, disable..

	//---------------
	//Init SpiOSD data structire.
	SOsd_init();

	//------------------------
	//start with saved input
	//------------------------
	StartVideoInput();

#ifdef DEBUG_TIME
	PrintSystemClockMsg("Finish StartVideoInput");
#endif

	//-----------------------
	//draw Logo
	//
	if (FirstInitDone == 0)
	{
		if(InputMain == INPUT_HDMIPC || InputMain == INPUT_HDMITV || InputMain == INPUT_LVDS) 
		{
			Printf("\n\rSkip InitLogo1()");
		}
		else 
		{
			InitLogo1();
			FirstInitDone =1;
		}
	}
	
	//
	// Power Up FP(FrontPanel) LED.
	// Now you can see somthing on your panel.
	PowerUpLedBackLight();

	//enable human input. (remocon,Touch, and Keypad).
	EnableRemoInt();
	InitAuxADC();	

	//-----------------------
	//remove Logo
	//
	if (FirstInitDone == 1) 
	{
		FirstInitDone = 2;
		RemoveLogoWithWait(1);
		if (Task_NoSignal_cmd == TASK_CMD_DONE)
			FOsdWinEnable(0, OFF);	//win0, disable..
	}	
	
	//------------------------
	// setup eeprom effect
	//------------------------
	SetAspectHW(GetAspectModeEE());
	value = EE_Read(EEP_FLIP);	//mirror
	if (value)
	{
	    WriteTW88(REG201, ReadTW88(REG201) | 0x80);
	}

	OsdSetTime(EE_Read(EEP_OSD_TIMEOUT));
	OsdSetTransparent(EE_Read(EEP_OSD_TRANSPARENCY));
	BackLightSetRate(EE_Read(EEP_BACKLIGHT));

	//set the Error Tolerance value for "En Changed Detection"
	MeasSetErrTolerance(0x04);		//tolerance set to 32. 

#ifdef USE_SFLASH_EEPROM
	//to cleanup E3PROM
	//call EE_CleanBlocks();
#endif

	UpdateOsdTimerClock();

#if defined(SUPPORT_I2CCMD_SERVER)
	// init I2CCMD server.
	// 
	//Init_I2CCMD_Slave();
#endif

	// re calculate FOSD DE
	FOsdSetDeValue();

#ifdef SUPPORT_WATCHDOG
	/* enable watchdog */
	EnableWatchdog(1);
#endif

	return 0;
}

//=============================================================================
// Video TASK ROUTINES				                                               
//=============================================================================


//-----------------------------------------------------------------------------
/**
* set NoSignalTask status
*		
* @param  cmd
*	- TASK_CMD_DONE
*	- TASK_CMD_WAIT_VIDEO
*	- TASK_CMD_WAIT_MODE
*	- TASK_CMD_RUN
*	- TASK_CMD_RUN_FORCE
*/
void TaskNoSignal_setCmd(BYTE cmd) 
{ 	
	if (cmd == TASK_CMD_WAIT_VIDEO && MenuGetLevel())	
		Task_NoSignal_cmd = TASK_CMD_DONE;	
	else
		Task_NoSignal_cmd = cmd;

	if (cmd == TASK_CMD_RUN_FORCE)
		tic_task = NOSIGNAL_TIME_INTERVAL;	//right now

	Task_NoSignal_count = 0;
}

//-----------------------------------------------------------------------------
/**
* get NoSignalTask status
*
* @return Task_NoSignal_cmd
*/
BYTE TaskNoSignal_getCmd(void) 
{ 	
	return Task_NoSignal_cmd;	
}

//=============================================================================
// MovingGrid TASK ROUTINES				                                               
//=============================================================================

//-----------------------------------------------------------------------------
/**
 * on/off Grid task
 *
 * @param onoff
*/
void TaskSetGrid(BYTE onoff)  {	Task_Grid_on = onoff;	}	
//-----------------------------------------------------------------------------
/**
 * get Grid task status
 *
 * @return Task_Grid_on
*/
BYTE TaskGetGrid(void)		  {	return Task_Grid_on;    }
//-----------------------------------------------------------------------------
/**
 * set Grid task command
 *
 * @param cmd
*/
void TaskSetGridCmd(BYTE cmd) { Task_Grid_cmd = cmd;	}	
//-----------------------------------------------------------------------------
/**
 * get Grid task command
 *
 * @return Task_Grid_cmd
*/
BYTE TaskGetGridCmd(void)	  { return Task_Grid_cmd;   } 


//=============================================================================
// CheckAndSet LINK ROUTINES				                                               
//=============================================================================

//-----------------------------------------------------------------------------
/**
 * function pointer for CheckAndSetInput
 *
*/
BYTE (*CheckAndSetInput)(void);


//-----------------------------------------------------------------------------
/**
 * dummy CheckAndSet function
 *
*/
BYTE CheckAndSetUnknown(void)
{
	return ERR_FAIL;
}

//-----------------------------------------------------------------------------
/**
 * link CheckAndSetInput Routine
 *
 * @see CheckAndSetDecoderScaler
 * @see CheckAndSetComponent
 * @see CheckAndSetPC
 * @see CheckAndSetDVI
 * @see CheckAndSetHDMI
 * @see CheckAndSetBT656Loop
 * @see CheckAndSetUnknown
*/
void LinkCheckAndSetInput(void)
{
	switch(InputMain) {
#if defined(SUPPORT_CVBS) || defined(SUPPORT_SVIDEO)
	case INPUT_CVBS:
	case INPUT_SVIDEO:
		CheckAndSetInput = &CheckAndSetDecoderScaler;
		break;
#endif
#ifdef SUPPORT_COMPONENT
	case INPUT_COMP:
		CheckAndSetInput = &CheckAndSetComponent;
		break;
#endif
#ifdef SUPPORT_PC
	case INPUT_PC:
		CheckAndSetInput = &CheckAndSetPC;
		break;
#endif
#ifdef SUPPORT_DVI
	case INPUT_DVI:
		CheckAndSetInput = &CheckAndSetDVI;
		break;
#endif
#if defined(SUPPORT_HDMI)
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
		CheckAndSetInput = &CheckAndSetHDMI;
		break;
#endif
#ifdef SUPPORT_BT656_LOOP
	case INPUT_BT656:
		CheckAndSetInput = &CheckAndSetBT656Loop;
		break;
#endif
#ifdef SUPPORT_LVDSRX
	case INPUT_LVDS:
		CheckAndSetInput = &CheckAndSetLVDSRx;
		break;
#endif
	default:
		CheckAndSetInput = &CheckAndSetUnknown;
		break;
	}
	SW_Video_Status = 0;					//clear
	FOsdWinEnable(TASK_FOSD_WIN,OFF);		//WIN0, Disable
	TaskNoSignal_setCmd(TASK_CMD_DONE);		//turn off NoSignal Task
}



#if 0 //def SUPPORT_RCD
//-----------------------------------------------------------------------------
/**
 * check RCD(Rear Camera Display) port
 *
 * @return 0:No, 1:Yes
*/
BYTE IsBackDrivePortOn(void)
{
	//BT656_LOOP uses P1_6. FW can not support RCD on BT656 mode.
	//if(InputMain >= INPUT_DVI) 
	//	return 0;
	if(InputMain==INPUT_BT656)
		return 0;
	return (PORT_BACKDRIVE_MODE==0 ? 1 : 0);
}
#endif

//-----------------------------------------------------------------------------
/**
 * Update OSD Timer
 *
*/
void UpdateOsdTimerClock(void)
{
	OsdTimerClock = (DWORD)OsdGetTime() * 100;
	
	//Turn On FontOSD.
	FOsdOnOff(ON, 0);	//with vdelay 0
}

//-----------------------------------------------------------------------------
/**
 *	Check OSD Timer and clear OSD if timer is expired.
*/
#if 0
void CheckAndClearOSD(void)
{
	if(OsdGetTime()==0)
		return;

	if(OsdTimerClock==0) {
		if(MenuGetLevel())	
			MenuEnd();	
		
		//Turn OFF Font OSD
		//if(FOsdOnOff(OFF, 0))	//with vdelay 0
		//	dPuts("\n\rCheckAndClearOSD disable FOSD");
			
		if(getNoSignalLogoStatus())
			RemoveLogo();
	}
}
#endif

//================================
// Power Save & Resume
//================================
BYTE Buf_r003;
BYTE Buf_0B0;	// Touch
BYTE Buf_106;	// ADC
BYTE Buf_1E6;	// AFE mode
BYTE Buf_1CB;	// LLPLL, SOG
BYTE Buf_1E1;	// LLPLL GPLL
BYTE Buf_4E1;	// Clock selection

//-----------------------------------------------------------------------------
/**
* Go into Power Save Mode
*
* System PowerSave procedure
* ==========================
*
*	set all GPIOs as input mode
*	switch MCU clock to 27MKz
*	Powerdown all analog blocks
*	Power up RC oscillator
*	Switch MCU/SPI clock to RC oscillator
*	Power	down crysital oscillator
*	Now, it is a PowerSave Mode
*
* @see WaitPowerOn
* @see SystemPowerResume
*/
static void SystemPowerSave(void)
{
	BYTE i;

	Printf("\n\r----- SystemPowerSave -----");
	delay1ms(10);

	//set all GPIOs as input mode	
	//WriteTW88(REG08C, 0x00);
//#if 1 //131106
//	WriteTW88(REG084, ReadTW88(REG084) | 0x02); //GPIO41
//	WriteTW88(REG08C, 0x00);
//#endif


	FP_BiasOnOff(OFF);

	Buf_r003 = ReadTW88(REG003);
	Interrupt_enableVideoDetect(OFF);

	WriteTW88(REG008, ReadTW88(REG008) | 0x30);	// Tri-State All outputs & FPdata 

	FP_PWC_OnOff(OFF);

	//select TTL REG007. I need P1_5 to wakeup. 
	WriteTW88(REG007,(ReadTW88(REG007) & ~0x07) | 0x02);

	//switch MCU clock to 27MKz
	Buf_4E1 = ReadTW88(REG4E1);
	WriteTW88(REG4E1, 0x00);

	//----- Powerdown HDMI
#if defined(SUPPORT_HDMI)
	//power off HDMI device
#endif

	//----- Powerdown all analog blocks
	LvdsRxEnable(OFF);
	LvdsRxPowerDown(ON);

#ifdef PANEL_LVDS			   
	WriteTW88(REG640, ReadTW88(REG640) & ~0x04);
#endif

	SFRB_ET1 = 0;								// Disable Touch Timer
	WriteTW88(REG0B1, ReadTW88(REG0B1) & ~0x40);	//enable Pen INTR
	Buf_0B0 = ReadTW88(REG0B0);
	WriteTW88(REG0B0, Buf_0B0 | 0x80 );			// TSC_ADC			  	*** 0.2uA

	Buf_106 = ReadTW88(REG106);
	WriteTW88(REG106, Buf_106 | 0x0F );			// ADC
	Buf_1E6 = ReadTW88(REG1E6);
	WriteTW88(REG1E6, 0x00 );					// AFE Mode=low speed	*** 0.6uA

	Buf_1CB = ReadTW88(REG1CB);
	WriteTW88(REG1CB, Buf_1CB & 0x1F );			// SOG, LLPLL
	Buf_1E1 = ReadTW88(REG1E1);
	WriteTW88(REG1E1, Buf_1E1 | 0x20 );			// GenLock
	WriteTW88(REG007, ReadTW88(REG007) & ~0x08);	//BT656Enc

	//----- SSPLL power down
	Sspll1PowerUp(OFF);							// SSPLL
	Sspll2PowerUp(OFF);							// SSPLL

	//----- Switch MCU/SPI clock to RC oscillator
	WriteTW88(REG4E1, 0x10);	 				// SPI clock Source=32KHz, ...

	//----- Power down crysital oscillator
	WriteTW88(REG0D4, ReadTW88(REG0D4) | (0x80));	// Enable Xtal PD Control
	PORT_CRYSTAL_OSC = 0;						// Power down Xtal

	while( PORT_POWER_SAVE==1 );

	//----- Wait ~30msec to remove key bouncing
	for(i=0; i<100; i++);

	//
	//Now, it is a PowerSave Mode
	//
}

//-----------------------------------------------------------------------------
/**
* Resume from Power Save Mode				                                               
*
* System Resume procedure
* ========================
*
*	Power up crystal oscillator
*	wait until crystal oscillator stable
*	switch MCU/SPI clock to 27MHz
*	Power up all analog blocks
*	Set MCU clock mode back
*	Set GPIO mode back
*	Now, Normal Operation mode
*
* @see WaitPowerOn
* @see SystemPowerSave
*/
static void SystemPowerResume(void)
{
	BYTE i;

	SFRB_EA = 0;

	//recover REG007. it was changed to read P1.5.
#if defined(PANEL_SRGB)
	WriteTW88(REG007, (ReadTW88(REG007) & ~0x07) | 0x01);
#elif defined(PANEL_FP_LSB)
	WriteTW88(REG007, (ReadTW88(REG007) & ~0x07) | 0x02);
#else
	WriteTW88(REG007, ReadTW88(REG007) & ~0x07);
#endif		

	//----- Power up Xtal Oscillator
	PORT_CRYSTAL_OSC = 1;							// Power up Xtal
	WriteTW88(REG0D4, ReadTW88(REG0D4) & ~(0x80)); 	// Disable Xtal PD Control

	//----- Wait until Xtal stable (~30msec)
	for(i=0; i<100; i++);

	//----- switch MCU/SPI clock to 27MHz
	WriteTW88(REG4E1, 0x00);	 					// SPI clock Source=27MHz, ...

	//----- Power up SSPLL
	Sspll2PowerUp(ON);								// SSPLL
	Sspll1PowerUp(ON);								// SSPLL
	//DCDC data out needs 200ms.
	//GlobalBootTime = SystemClock;
	//PrintSystemClockMsg("Sspll1PowerUp");

	//----- Wait until SSPLL stable (~100usec)
	for(i=0; i<200; i++);
	for(i=0; i<200; i++);

	//----- Power up all analog blocks
	WriteTW88(REG1E1, Buf_1E1);					// GenLock
	WriteTW88(REG007, ReadTW88(REG007) | 0x08);	//BT656Enc. FYI...
	WriteTW88(REG1CB, Buf_1CB);					// LLPLL, SOG
	
	WriteTW88(REG106, Buf_106);					// ADC
	WriteTW88(REG1E6, Buf_1E6);					// AFE mode
	
	WriteTW88(REG0B0, Buf_0B0);					// Touch
	WriteTW88(REG0B1, ReadTW88(REG0B1) | 0x40);	//Disable Pen INTR

	//----- PowerUp HDMI
#if defined(SUPPORT_HDMI)
	//power up HDMI device
#endif

	//----- Set MCU clock mode back
	WriteTW88(REG4E1, Buf_4E1);	 				// Clock selection

	//----- Set GPIO mode back
	//WriteTW88(REG08C, 0x0C);

	SFRB_EA = 1;

	//
	// In case aRGB input, set Filter=0 and wait until stable and Filter=7
	//

#ifdef SUPPORT_TOUCH
	SFRB_ET1 = 1;									// Enable Touch Timer
#endif

	//
	//Now, Normal Operation mode
	//

	while( PORT_POWER_SAVE==1 );				// Wait untill button is released
	delay1ms(100);								// To remove key bouncing

	//LVDSRx
	if(InputMain==INPUT_LVDS) {
		LvdsRxEnable(ON);
		LvdsRxPowerDown(OFF);
	}

	//----- Power up Panel, Backlight

#ifdef PANEL_LVDS
	WriteTW88(REG640, ReadTW88(REG640) | 0x04);
#endif

	//Sspll1PowerUp needs 100ms before FW turns on the DataOut
	FrontPanel_StartUP();								// DCDC. it has WaitVBlank.
	PowerUpLedBackLight();								// LEDC


	Puts("\n\r----- SystemPowerResume -------");
	Interrupt_enableVideoDetect(ON);
	WriteTW88(REG003, Buf_r003);	//recover ISR mask
	if(DebugLevel)
		Prompt();
}

//-----------------------------------------------------------------------------
/**
* wait powerup condition on the power save state
*
* @return 1:by button, 2:by Touch
* @see SystemPowerSave
* @see SystemPowerResume
*/
static BYTE WaitPowerOn(void)
{
	BYTE i;

	while(1) {

//#if 0 //test only
//		//if PORT_POWER_SAVE has a problem, temporary, use a poll to check a suspend & resume.
//		for(ii=0; ii < 0x100; ii++) ;
//		return 1;
//#endif

		//----- Check Power Button
		if(PORT_POWER_SAVE==1) {
			for(i=0; i < 100; i++);
			if(PORT_POWER_SAVE==1) return 1;
		}
		//----- Check Touch
		if( P2_4==0 ) return 2;

		//----- Check Remote Control
		//if( P1_2==0 ) return 3;	// Need to confirm if it is by Power Button
	}
}


//=============================================================================
// RearCameraDisplayMode				                                               
//=============================================================================
#ifdef SUPPORT_RCD
//-----------------------------------------------------------------------------
/**
*	Turn On/Off Back drive grid SPIOSD image
*/
static void BackDriveGrid(BYTE on)
{
	if(on) {
		//draw parkgrid
		SOsd_CleanReg();

		//init DE
		SpiOsdSetDeValue();

		//init SOSD
		WaitVBlank(1);
		SpiOsdEnable(ON);
		SOsd_CleanRlc();
		SOsd_UpdateRlc();
		SOsd_SetSpiStartBit(1,0);
		SOsd_SetLutOffset( 1, 0 /*SOSD_WIN_BG,  WINBG_LUTLOC*/ );  //old: SpiOsdLoadLUT_ptr
//		SpiOsdWinFillColor( 1, 0 );

		MovingGridInit();
		//MovingGridDemo(0 /*Task_Grid_n*/);
		MovingGridTask_init();
		MovingGridLUT(3);	//I like it.

	}
	else {
		SpiOsdWinHWOffAll(0);	//without wait
		StartVideoInput();
	}
}

//-----------------------------------------------------------------------------
/**
* init RCD mode
*
* goto RCDMode (RearCameraDisplay Mode)
* and, prepare ParkingGrid.
* RCDMode does not support a video ISR.
* @return
*	0:success
*	other:error code
*/
extern code struct SCALER_TIME_TABLE_s SCALER_CVBS_OVERSCAN_time[];
BYTE InitRCDMode(BYTE fPowerUpBoot)
{
	BYTE ret;
	struct SCALER_TIME_TABLE_s *pScaler = &SCALER_CVBS_OVERSCAN_time[0];
	DWORD dTemp;

	Printf("\n\rInitRCDMode(%bd)",fPowerUpBoot);
	if(fPowerUpBoot==0) {
		if(MenuGetLevel()) {
			MenuQuitMenu();
			SpiOsdWinHWOffAll(1);	//with WaitVBlank
		}
		//FYI. I don't care demo page.

		//if logo was used, clear win1.
		WriteTW88(REG440, ReadTW88(REG440) & ~0x01);
	}

	//skip CheckEEPROM() and manually assign DevegLevel
	DebugLevel = 1;

	//set default setting.
	Init8836AsDefault(0/*InputMain*/, 1); //select CVBS
	I2C_delay_base = 1;

	InitGpioDefault();

	Puts("\nEnable OutputPin");
	OutputEnablePin(OFF,ON);		//Output enable. FP data: not yet

	InitClockAsDefault(0);			//0:SYNC, 1:ASYNC
	//Sspll1PowerUp(ON);
	//PrintSystemClockMsg("Sspll1PowerUp");
	//DCDC needs 100ms, but we have enough delay on...

#ifdef PANEL_LVDS
	InitLVDSTx();
#endif


	WriteTW88(REG040, ReadTW88(REG040) & ~0x10);   //scaler input clock pol.

	FrontPanel_StartUP();

#ifdef DEBUG_TIME
	PrintSystemClockMsg("before DecoderCheck");
#endif

//	InitInputAsDefault();
	Init8836AsDefault(0/*InputMain*/,0);
	InputSetSource(INPUT_PATH_DECODER,INPUT_FORMAT_YCBCR);	//InputSource  (InMux)
	AMuxSetInput(0/*InputMain*/);	//Analog Mux
	DecoderFreerun(DECODER_FREERUN_AUTO);	//Decoder freerun	
	//aRGB(VAdc)
	WriteTW88(REG1CB, (ReadTW88(REG1CB) & 0x1F));
	aRGBSetClockSource(1);			//select 27MHz. R1C0[0]
	//BT656 Output
	BT656EncOutputEnable(OFF,0);
	//DTV BT656Dec Freerun & clock
	BT656DecSetFreerun(OFF);
	//DTV
	//LVDSRx
		EnableExtLvdsTxChip(OFF);	//GPIO EXPANDER IO[4]
		LvdsRxEnable(OFF);
		LvdsRxPowerDown(ON);
	//scaler
	ScalerSetFreerunAutoManual(ON,OFF);
	ScalerWriteFreerunTotal(FREERUN_DEFAULT_HTOTAL, FREERUN_DEFAULT_VTOTAL);
	ScalerSetLineBufferSize(720/*hActive*/);
//	ScalerSetHScale(720/*hActive*/);
//	ScalerSetHScale_FULL(720);
		ScalerWriteLineBufferDelay(0x10/*SCALER_HDELAY2_BASE*/);
		ScalerPanoramaOnOff(OFF);

		dTemp = 720;
		dTemp *=  0x2000L;
		dTemp /= PANEL_H;
		ScalerSetHScaleReg(0x0400, dTemp);
		//ScalerSetLineBufferSize(Length);

//{720,240,60,  0,0, 0,0,0,	0x0,0x0000,	716,8,720,  238,21,  43, POL_PVH, 0x00000000, 0x00},	//F&P effect. 
//{720,288,50,  0,0, 0,0,0,	0x0,0x0000,	716,6,720,  285,27,  45, POL_PVH, 0x00000000, 0x00},	//F&P effect. 


	OverWriteScalerWithTable(pScaler,1,1);

//CheckAndSetDecoderScaler
//VInput_enableOutput

	//FW add a check routine because the customer wants a stable video..
	//Current code only check the NTSC. 
	//If you want to PAL, change REG11D value.
	//If we assign only NTSC, it uses a 300ms.
	//If we add all standard, it uses a 500ms.
	WriteTW88(REG11D, 0x01);

	//wait until we have a stable signal
	ret=DecoderCheckVDLOSS(100);
	if(ret) {
		ePuts("\n\rCheckAndSetDecoderScaler VDLOSS");
	}
	else {
		//get standard
		ret = DecoderCheckSTD(100);
		if ( ret == 0x80 ) {
		    ePrintf("\n\rCheckAndSetDecoderScaler NoSTD");
			//return( 2 );
		}
		else {
			ret >>= 4;
			//InputSubMode = mode;
			ePrintf("\n\rMode:%bx",ret);
		}
	}
#ifdef DEBUG_TIME
	PrintSystemClockMsg("after DecoderCheck");
#endif
	//?pal


	//disable interrupt.
	WriteTW88(REG003, 0xFE );	// enable only SW interrupt

	LedBackLight(ON);
	ScalerSetMuteManual(OFF);

	//---------------
	//init FontOSD 

	//---------------
	//Init SpiOSD data structire.
	SOsd_init();

	//draw parkgrid
	BackDriveGrid(ON);

	PowerUpLedBackLight();

	return ret;
}
#endif

//-----------------------------------------------------------------------------
/**
* print model, version, compile date
*
* example:
*	********************************************************
*	 TW8835 EVB 3.1 - 18:52:43 (May 14 2012) Server
*	********************************************************
*/
static void print_firmware_info(void)
{
	Puts("********************************************************\n");

	/* model */
#if defined(MODEL_TW8836B2)
	Puts(" TW8836B2");
#else
	Puts(" TW88XX");
#endif

	Puts(" ");

	/* board revision */
#if defined(MODEL_TW8836DEMO)
	Puts("Demo ");
#elif defined(EVB_11)
	Puts("EVB 1.1 - ");
#else
	Puts("XXX 0.0 - ");
#endif

	/*FW revision */
	Printf(" FW %bx.%bx -", (BYTE)(FWVER >> 8), (BYTE)FWVER);

	/*compiled data */
	Printf("%s (%s)", __TIME__, __DATE__);

	/* I2CCMD server mode*/
#ifdef SUPPORT_I2CCMD_SERVER
	Puts(" Server");
#endif

	/* RearCameraDisplay */
#ifdef SUPPORT_RCD
	Puts(" RCD");
#endif

	Puts("\n");

	//------------------------
	/* panel info */
	Printf(" Panel %dx%d ", (WORD)PANEL_H, (WORD)PANEL_V);

#ifdef PANEL_TCON
	Puts("TCON");
#elif defined(PANEL_SRGB)
	Puts("SRGB");
#elif defined(PANEL_FP_LSB)	
	Puts("FPLSB");
#elif defined(PANEL_LVDS)
	Puts("LVDS");
#else
	Puts("Unknown");	
#endif

	Puts("\n");
	Puts("********************************************************\n");
}

//=============================================================================
// INIT ROUTINES
//=============================================================================
//-----------------------------------------------------------------------------
/**
* init global variables
*/

static void init_global_variables(void)
{
	CurrSystemMode = SYS_MODE_NORMAL;
	DebugLevel = 3;
	g_access = 1;
	SW_key = 0;

	//--task variables
	Task_Grid_on = 0;
	Task_Grid_cmd = 0;
	Task_NoSignal_cmd = TASK_CMD_DONE;
	SW_INTR_cmd = 0;

	//search EOT to assign spiflash_chip->mid=0; 
	spiflash_chip = spiflash_chip_table;	
	while (spiflash_chip->mid)
		spiflash_chip++;
	SpiFlash4ByteAddr = 0;	//32bit address mode.

#ifdef USE_SFLASH_EEPROM
	e3p_spi_start_addr = E3P_SPI_SECTOR0;
#endif

	print_spiflash_status_register = &print_spiflash_status_register_default;

	OsdTime	= 0;

#ifdef DEBUG_REMO_NEC
	DebugRemoStep = 0;
#endif

#ifdef DEBUG_UART
	UART0_OVERFLOW_counter = 0;  //clear
	UART0_MAX_counter = 0; 		//clear
#endif

	global_Freerun_hTotal =	FREERUN_DEFAULT_HTOTAL;
	global_Freerun_vTotal = FREERUN_DEFAULT_VTOTAL;

#if defined(SUPPORT_HDMI)
	global_CEC_flag = 0;
#endif

	//timer2_owner = 0;
#if defined(SUPPORT_FAST_INPUT_TOGGLE)
	g_fast_toggle=1;
	g_cvbs_checked=0;
	g_hdmi_checked=0;
#endif

	/* menu */
    menu_on = 0;
    menu_level = 0;
}

//=============================================================================
//			                                               
//=============================================================================

//-----------------------------------------------------------------------------
/**
* start video with a saved input.
*
* @see ChangeInput
*/
void StartVideoInput(void)
{
	BYTE InputMainEE;
				
	ePrintf("\n\rStart with Saved Input: ");
	InputMainEE = GetInputMainEE();
	PrintfInput(InputMainEE,1);

	InputMain = 0xff;			// start with saved input						
	ChangeInput( InputMainEE );	
}




//=============================================================================
// Video Signal Task				                                               
//=============================================================================

//-----------------------------------------------------------------------------
/**
* do Video Signal Task Routine
*
*
* @see Interrupt_enableVideoDetect
* @see CheckAndSetInput
* @see VInput_enableOutput
*/
void NoSignalTask(void)
{	
	BYTE ret;
	BYTE r004;

	// MovingGridTask uses tic_task. 
    // It can not coexist with NoSignalTask.
	if(Task_Grid_on)
		return;

	if(Task_NoSignal_cmd==TASK_CMD_DONE)
		return;

	if(tic_task < NOSIGNAL_TIME_INTERVAL) 
		return;

	if(Task_NoSignal_cmd==TASK_CMD_WAIT_VIDEO) {
		FOsdWinToggleEnable(TASK_FOSD_WIN);
		if(Task_NoSignal_count < 3)
			Task_NoSignal_count++;
		tic_task = 0;

		return;
	}
	if(Task_NoSignal_cmd==TASK_CMD_WAIT_MODE)
		return;
 
	//--------------------------------------------
	//
	//--------------------------------------------

	//dPuts("\n\r***Task NoSignal TASK_CMD_RUN");
	//if(Task_NoSignal_cmd == TASK_CMD_RUN_FORCE)
	//	dPuts("_FORCE");

	r004 = ReadTW88(REG004);
	if(r004 & 0x01) {						
		ePrintf("..Wait...Video");

		tic_task = 0;
		return;
	}

	//turn off Interrupt.
	Interrupt_enableVideoDetect(OFF);

	//start negotition
	ret = CheckAndSetInput();

	//turn on Interrupt. 
	//if success, VInput_enableOutput() will be executed.
	Interrupt_enableVideoDetect(ON);

	if(ret==ERR_SUCCESS) {
		//dPuts("\n\r***Task NoSignal***SUCCESS");
		VInput_enableOutput(VH_Loss_Changed);
		FOsdWinEnable(TASK_FOSD_WIN,OFF); 	//WIN0, Disable


		if(getNoSignalLogoStatus()) {
			RemoveLogo();
		}

		//need SetBT656Output() with proper mode.....
		//SetBT656Output(GetInputBT656EE());
#if defined(SUPPORT_FAST_INPUT_TOGGLE)
		if(InputMain == INPUT_HDMIPC) {
			ReadTW88Reg2Buff(Fast_VIDEO_Toggle_info,Fast_DTV_Toggle_Buff);
			g_hdmi_checked = 1;
#ifdef DEBUG_TIME
			PrintSystemClockMsg("NoSignalTask INPUT_HDMIPC");
#endif
		}
		if(InputMain == INPUT_CVBS) {
			ReadTW88Reg2Buff(Fast_VIDEO_Toggle_info,Fast_CVBS_Toggle_Buff);
			g_cvbs_checked = 1;
#ifdef DEBUG_TIME
			PrintSystemClockMsg("NoSignalTask INPUT_CVBS");
#endif
		}
#endif
	}
#ifdef SUPPORT_PC
	else {
		//fail 
		if(InputMain==INPUT_PC) {
			if(ReadTW88(REG004) & 0x01)
				FOsdIndexMsgPrint(FOSD_STR2_NOSIGNAL);	//over write
			else
				FOsdIndexMsgPrint(FOSD_STR3_OUTRANGE);	//replace 
		}
	}
#endif

	//update tic_task.
	tic_task = 0;
}
//-----------------------------------------------------------------------------
/**
*  Check each input status
*
*  recover routine for unstable video input status.
*  only need it when user connect/disconnect the connector 
*  or, the QA toggles the video mode on the pattern generator.
*/
void NoSignalTaskOnWaitMode(void)
{
	BYTE ret;
	if((Task_NoSignal_cmd != TASK_CMD_WAIT_MODE))
		return;
	
	if(InputMain==INPUT_CVBS 
    || InputMain==INPUT_SVIDEO) {
		ret=DecoderReadDetectedMode();
		//only consider NTSC & PAL with an idle mode.
		if(ret == 0 || ret == 1) {
			if(InputSubMode != ret) {
				ScalerSetMuteManual( ON );

				SW_INTR_cmd = SW_INTR_VIDEO_CHANGED;
				dPrintf("\n\rRequest SW Interrupt cmd:%bd InputSubMode:%bd->%bd",SW_INTR_cmd, InputSubMode,ret);
				InputSubMode = ret;
				WriteTW88(REG00F, SW_INTR_VIDEO);	//SW interrupt.		
			} 
		}
	}
#ifdef SUPPORT_COMPONENT
	else if(InputMain==INPUT_COMP) {
		ret = aRGB_GetInputStatus();	//detected input.
		if(ret & 0x08) {			//check the compoiste detect status first.
			ret &= 0x07;
			if( (ret!=7) && (InputSubMode != ret) ) {
				ScalerSetMuteManual( ON );

				SW_INTR_cmd = SW_INTR_VIDEO_CHANGED;
				dPrintf("\n\rRequest SW Interrupt cmd:%bd InputSubMode:%bd->%bd",SW_INTR_cmd, InputSubMode,ret);
				InputSubMode = ret;
				WriteTW88(REG00F, SW_INTR_VIDEO);	//SW interrupt.		
			} 
		}
	}
#endif
}

//=============================================================================
// Interrupt Handling Routine			                                               
//=============================================================================
//-----------------------------------------------------------------------------
/**
* enable VideoDetect interrupt
*
* Turn off the SYNC Change(R003[2]) mask,
*               the Video Loss(R003[1]) mask,
*               the WirteReg0x00F(R003[0] mask.
* 
* Turn On  the Video Loss(R003[1]) mask,
*               the WirteReg0x00F(R003[0] mask. 
*
* I do not turn on the SYNC Change.
* if you want to turn on SYNC, You have to call Interrupt_enableSyncDetect(ON).
*
* @param bool fOn
* @see Interrupt_enableSyncDetect
*/
void Interrupt_enableVideoDetect(BYTE fOn)
{
#ifdef DEBUG_ISR
	WORD temp_VH_Loss_Changed;
	BYTE temp_INT_STATUS, temp_INT_STATUS2;
#endif

	if(fOn) {
		WriteTW88(REG002, 0xFF);	//clear
		WriteTW88(REG004, 0xFF);	//clear
		WriteTW88(REG003, 0xEC);	//release Video, but still block SYNC
	}
	else {
		WriteTW88(REG003, 0xEE);	//block.
		WriteTW88(REG002, 0xFF);	//clear
		WriteTW88(REG004, 0xFF);	//clear

#ifdef DEBUG_ISR
		//copy
		temp_INT_STATUS = INT_STATUS;
		temp_VH_Loss_Changed = VH_Loss_Changed;
		temp_INT_STATUS2 = INT_STATUS2;
#endif
		//clear
		INT_STATUS = 0;
		VH_Loss_Changed = 0;
		INT_STATUS2 = 0;
#ifdef DEBUG_ISR
		if(temp_INT_STATUS+temp_VH_Loss_Changed+temp_INT_STATUS2)
			dPrintf("\n\rclear INT_STATUS:%bx INT_STATUS2:%bx VH_Loss_Changed:%d",
                temp_INT_STATUS,temp_INT_STATUS2,temp_VH_Loss_Changed);
#endif
	}
}

//-----------------------------------------------------------------------------
/**
* Turn off/on SYNC Interrupt mask.
*
* @see Interrupt_enableVideoDetect
*/
void Interrupt_enableSyncDetect(BYTE fOn)
{
#ifdef DEBUG_ISR
	BYTE temp_INT_STATUS, temp_INT_STATUS2;
#endif
	BYTE bTemp;

	if(fOn) {
		WriteTW88(REG002, 0x04);	//clear
		WriteTW88(REG004, 0x06);	//clear
		bTemp =  ReadTW88(REG003) & ~0x04;
		WriteTW88(REG003, bTemp);	//release

		SW_Video_Status = 1;
		//dPrintf("\n\rSW_Video_Status:%bd",SW_Video_Status);
	}
	else {
		WriteTW88(REG003, ReadTW88(REG003) | 0x04);	//block
		WriteTW88(REG002, 0x04);	//clear
		WriteTW88(REG004, 0x06);	//clear

#ifdef DEBUG_ISR
		//copy
		temp_INT_STATUS = INT_STATUS;
		temp_INT_STATUS2 = INT_STATUS2;
#endif
		//clear
		INT_STATUS &= ~0x04;
		INT_STATUS2 &= ~0x06;
#ifdef DEBUG_ISR
		if( (temp_INT_STATUS != INT_STATUS) || (temp_INT_STATUS2 != INT_STATUS2))
			dPrintf("\n\rclear SYNC at INT_STATUS:%bx INT_STATUS2:%bx",temp_INT_STATUS,temp_INT_STATUS2);
#endif
	}
}

//-----------------------------------------------------------------------------
/**
* do interrupt polling
*
* Read interrupt global value that changed on interrupt service routine,
* and print status.
*
* @see ext0_int
* @see InterruptHandlerRoutine
*
*
* Interrupt Handler Routine 
* ==========================
* use InterruptPollingRoutine first
* @see ext0_int
* @see InterruptPollingRoutine
*
*/
static void InterruptPollingHandlerRoutine(void)
{
	BYTE temp_INT_STATUS_ACC;
	BYTE temp_INT_STATUS2_ACC;

	BYTE ret;
	BYTE r004;
	BYTE not_detected;

	WORD main_VH_Loss_Changed;
	BYTE main_INT_STATUS;
	BYTE main_INT_MASK;
	BYTE main_INT_STATUS2;

	//------------ Chip Interrupt --------------	
	SFRB_EX0 = 0; 	//disable INT0
	//--copy
	main_INT_STATUS  = INT_STATUS;
	main_INT_MASK    = INT_MASK;         
	main_INT_STATUS2 = INT_STATUS2;
	main_VH_Loss_Changed = VH_Loss_Changed;
	temp_INT_STATUS_ACC = INT_STATUS_ACC;
	temp_INT_STATUS2_ACC = INT_STATUS2_ACC;
	//--clear
	INT_STATUS = 0;			//can be removed
	INT_STATUS2 = 0;		//can be removed
	VH_Loss_Changed = 0;
	INT_STATUS_ACC = 0;
	INT_STATUS2_ACC = 0;
	SFRB_EX0 = 1;	//enable INT0

	//mask
	main_INT_STATUS &= ~main_INT_MASK;

	//-------------------------
	// print INT debug message
	//-------------------------
	if(main_INT_STATUS & 0x07) {
		ePrintf("\n\rInterrupt !!! R002[%02bx] ", main_INT_STATUS);
		//adjust from _ACC
		if(main_INT_STATUS != temp_INT_STATUS_ACC) {
			temp_INT_STATUS_ACC	&= ~main_INT_MASK;
			if(main_INT_STATUS != temp_INT_STATUS_ACC) {
				ePrintf(" [ACC:%02bx]", temp_INT_STATUS_ACC);
				if(temp_INT_STATUS_ACC & 0x01)
					main_INT_STATUS |= 0x01;				//NOTE
			}
		}
		ePrintf(" R003[%bx]",main_INT_MASK);
		ePrintf(" R004[%02bx] ", main_INT_STATUS2);
		//adjust from _ACC
		if(main_INT_STATUS2 != temp_INT_STATUS2_ACC) {
			ePrintf(" [ACC2:%02bx]", temp_INT_STATUS2_ACC);
			if((main_INT_MASK & 0x04) == 0) {
				main_INT_STATUS2 |= (temp_INT_STATUS2_ACC & 0x06);	//NOTE
				ePrintf("->[%02bx]",main_INT_STATUS2);	
			}
		}

	}
	//if( main_INT_STATUS & 0x80 ) ePrintf("\n\r   - SPI-DMA completion ");
	//if( main_INT_STATUS & 0x40 ) ePrintf("\n\r   - V display end ");
	//if( main_INT_STATUS & 0x20 ) ePrintf("\n\r   - Measurement Ready ");
	//if( main_INT_STATUS & 0x08 ) ePrintf("\n\r   - VSync leading edge ");
	//
	//if( main_INT_STATUS & 0x04 ) {
	//	ePrintf("\n\r   - Sync Changed ");
	//	if(main_INT_STATUS2 & 0x02) ePrintf(" - HSync changed ");
	//	if(main_INT_STATUS2 & 0x04) ePrintf(" - VSync changed ");
	//}

	if( (main_INT_STATUS & 0x04 ) && (main_INT_STATUS2 & 0x04)) {
		ePrintf("\n\r   - VSync Changed ");
	}			

	if(main_VH_Loss_Changed) {		//INT_STATUS[1] use accumulated VH_Loss_Changed value.
		//Video change happen.
		ePrintf("\n\r   - V/H Loss Changed:%d ", main_VH_Loss_Changed);
		if(main_INT_STATUS2 & 0x01)	ePrintf(" - Video Loss ");
		else						ePrintf(" - Video found ");
	}
	if(main_INT_STATUS & 0x01) {
		//Printf("\n\rR00F[%02bx]",ReadTW88(REG00F));
		if(SW_INTR_cmd == SW_INTR_VIDEO_CHANGED)
			dPrintf("\n\r*****SW_INTR_VIDEO_CHANGED");
#if defined(SUPPORT_I2CCMD_SERVER)
		else {
			BYTE cmd;
			cmd = ReadTW88(REG00F);
			if(cmd & SW_INTR_EXTERN) {
				if(cmd & I2CCMD_CHECK) {
					WriteTW88(REG009, 0xA1);
					//return;	
				}
				else if(cmd & I2CCMD_EXEC) {
					F_i2ccmd_exec = 1;	//request loop routine
					WriteTW88(REG009, 0xA1);
					//return;						
				}
			}	
		}		
#endif
	}

	//----------------------------------------
	// now, We uses 
	//	main_INT_STATUS
	//  main_INT_STATUS2
	//  main_VH_Loss_Changed


	//========================================
	// Handler routine
	//========================================

	if(main_INT_STATUS & 0x01) {
		if(SW_INTR_cmd == SW_INTR_VIDEO_CHANGED) {
			SW_INTR_cmd = 0;

			LedBackLight(OFF);
			ScalerSetMuteManual( ON );

			//start negotiation right now
			TaskNoSignal_setCmd(TASK_CMD_RUN_FORCE);		
		}
		else {
			//assume external MCU requests interrupts.
			//read DMA buffer registers that the external MCU write the commmand.
			//we need a pre-defined format
			//execute

		}
		//NOTE:TASK_CMD_RUN2 can be replaced on the following condition. LedBackLight(OFF) can make a problem.
	}
	//Check SYNCH first and than check VDLoss. VDLoss will have a high priority.
	if( main_INT_STATUS & 0x04 ) {
		//check only VSync.	I have too many HSync.
		//service SYNC only when we have a video.
		if(( (main_INT_STATUS2 & 0x05) == 0x04 ) && (Task_NoSignal_cmd==TASK_CMD_DONE || Task_NoSignal_cmd==TASK_CMD_WAIT_MODE)) {
			if(InputMain==INPUT_CVBS || InputMain==INPUT_SVIDEO) {
				//dPrintf("\n\r*****SYNC CHANGED");

				ret=DecoderReadDetectedMode();
				not_detected = ret & 0x08 ? 1 : 0;	//if not_detected is 1, not yet detected(in progress).
				ret &= 0x07;
				//dPrintf(" InputSubMode %bd->%bd",InputSubMode,ret);
				if(not_detected || (ret == 7)) {
					//dPrintf(" WAIT");
					TaskNoSignal_setCmd(TASK_CMD_WAIT_MODE);
				}
				else if(InputSubMode != ret) {
					//dPrintf(" NEGO");
					LedBackLight(OFF);
					ScalerSetMuteManual( ON );
	
					//start negotiation	right now
					TaskNoSignal_setCmd(TASK_CMD_RUN_FORCE);
				}
				//else
				//	dPrintf(" SKIP");
			}
#ifdef SUPPORT_COMPONENT
			else if(InputMain==INPUT_COMP) {
				//dPrintf("\n\r*****SYNC CHANGED");
	
				ret = aRGB_GetInputStatus();	//detected input.
				not_detected = ret & 0x08 ? 0:1;	 //if not_detected is 1, not yet detected.
				ret &= 0x07;
				//dPrintf(" InputSubMode %bd->%bd",InputSubMode,ret);
				if(not_detected || (ret == 7)) {
					//dPrintf(" WAIT");
					TaskNoSignal_setCmd(TASK_CMD_WAIT_MODE);
				}
				else if(InputSubMode != ret) {
					//dPrintf(" NEGO");
					LedBackLight(OFF);
					ScalerSetMuteManual( ON );
	
					//start negotiation right now
					TaskNoSignal_setCmd(TASK_CMD_RUN_FORCE);
				}
				//else
				//	dPrintf(" SKIP");
			}
#endif
#ifdef SUPPORT_PC
			else if(InputMain==INPUT_PC) {
				//Need to verify.
	
				//-------------------------------
				// Video Signal is already changed. I can not use a FreeRun with FOSD message.
				//WaitVBlank(1);
				LedBackLight(OFF);
				ScalerSetMuteManual( ON );
	
				//dPrintf("\n\r*****SYNC CHANGED");
				//dPrintf(" NEGO");

				//start negotiation	right now
				TaskNoSignal_setCmd(TASK_CMD_RUN_FORCE);
			}
#endif
			else {
				//Need to verify.

				//-------------------------------
				// Video Signal is already changed. I can not use a FreeRun with FOSD message.
				//WaitVBlank(1);
				LedBackLight(OFF);
				ScalerSetMuteManual( ON );

				//dPrintf("\n\r*****SYNC CHANGED");
				//dPrintf(" NEGO");

				//start negotiation	right now
				TaskNoSignal_setCmd(TASK_CMD_RUN_FORCE);
			}
		}
	}


	if(main_VH_Loss_Changed) {		//INT_STATUS[1] use accumulated VH_Loss_Changed value.
		//Video change happen.
		main_VH_Loss_Changed = 0;
		if(InputMain==INPUT_PC) {
			;
		}
		else {
			//------------------------
			//read INT_STATUS2 value from HW.
			r004 = ReadTW88(REG004);
			if(((main_INT_STATUS2 ^ r004) & 0x01) == 0x01) {						
				ePrintf("\n\rWarning SW replace Video Loss");
			//	main_INT_STATUS2 ^= 0x01;
			}
		}
		//--OK, what is a current status
		if(main_INT_STATUS2 & 0x01) {
			//Video Loss Happen.
			//turn on MuteManual first.
			//and then 
			if(SW_Video_Status) {

				ScalerSetMuteManual(ON);
				ScalerCheckAndSetFreerunManual();



				//dPuts("\n\rVideo Loss Happen");
				//turn off SYNC							
				SW_Video_Status = 0;
				//dPrintf("\n\rSW_Video_Status:%bd",SW_Video_Status);

				Interrupt_enableSyncDetect(OFF);
#ifdef SUPPORT_COMPONENT
				if(InputMain == INPUT_COMP)
					//Change to 0 for fast recover.
					aRGB_SetFilterBandwidth(0, 0);		
#endif
			
				//start "No Signal" blinking
				if(MenuGetLevel()==0) {
					FOsdIndexMsgPrint(FOSD_STR2_NOSIGNAL);
				}

				tic_task = 0;
				TaskNoSignal_setCmd(TASK_CMD_WAIT_VIDEO);	//block the negotiation until you have a Video Signal
			}
			else {
				//tic_task = 0;
				TaskNoSignal_setCmd(TASK_CMD_WAIT_VIDEO);	//block the negotiation until you have a Video Signal
			}
		}
		else {
			//Video Found Happen
			if(SW_Video_Status==0) {
				//dPuts("\n\rVideo found Happen");
				SW_Video_Status = 1;
				//dPrintf("\n\rSW_Video_Status:%bd",SW_Video_Status);

				//turn ON SYNC
				//Interrupt_enableSyncDetect(ON);  not yet. turn on it after it decide the video mode.
			}						

			if(Task_NoSignal_cmd==TASK_CMD_DONE) {
				//dPrintf("\n\r********RECHECK");
				tic_task = NOSIGNAL_TIME_INTERVAL;			//do it right now..	
			}
			else {
				tic_task=NOSIGNAL_TIME_INTERVAL - 500;		//wait 500ms. 100ms is too short.
			}
			//start negotiation
			TaskNoSignal_setCmd(TASK_CMD_RUN);	
		}
	}
}

//==<<DOXYGEN>>==============================================================================================
/*! \brief Brief description.
*
* Detail description starts here.
*/
/**
 * \brief
 * \note
 * \param
 * \param[out]
 * \param[in]
 * \see
 * \return
 * \result
*/

//
//int var; /*!< Detail description after the member */
//


