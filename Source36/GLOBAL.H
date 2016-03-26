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
/*                                                                           */
/*              Grobal Variables                                             */
/*                                                                           */
/*****************************************************************************/
#ifndef __GLOBAL_H__
#define __GLOBAL_H__

//---watchdog was -------------------
//extern DATA BYTE watchdog_reset_flag_was;

//---------------------------------- CPU --------------------------------------
extern	DATA 	BYTE   	tm001;
extern	DATA 	WORD   	tm01;
extern	DATA 	BYTE   	tic01;
extern	DATA 	WORD   	tic_pc;
extern	DATA 	WORD   	tic_task;
extern	DATA 	DWORD	SystemClock;
extern 	DATA	DWORD	OsdTimerClock;

#ifdef DEBUG_ISR
extern XDATA WORD INTR_counter[];
#endif

#ifdef DEBUG_UART
extern XDATA BYTE UART0_OVERFLOW_counter;
extern XDATA BYTE UART0_MAX_counter;
#endif

//===== WatchDog ======
#ifdef DEBUG_WATCHDOG
extern			bit		F_watch;
#endif

//===== Serial0 ======
extern	DATA 	BYTE	RS_buf[];
extern	DATA 	BYTE	RS_in, RS_out;
extern			bit		RS_Xbusy;

//===== Serial1 ======
#ifdef SUPPORT_UART1
extern	DATA 	BYTE	RS1_buf[];
extern	DATA 	BYTE	RS1_in, RS1_out;
extern			bit		RS1_Xbusy;
#endif

#ifdef REMO_RC5
	extern	bit         RemoPhase1, RemoPhase2;
	extern	IDATA BYTE	RemoDataReady;
	extern	IDATA BYTE  RemoSystemCode, RemoDataCode;
	extern	IDATA BYTE  RemoSystemCode0, RemoDataCode0;
	extern	IDATA BYTE  RemoSystemCode1, RemoDataCode1;
	extern	IDATA BYTE  RemoSystemCode2, RemoDataCode2;
	extern	IDATA DWORD RemoReceivedTime;
	extern  IDATA BYTE  RemoCaptureDisable;
#elif defined REMO_NEC
	extern	bit			RemoPhase;
	extern	DATA BYTE	RemoStep;
	extern	DATA BYTE	RemoHcnt, RemoLcnt;
	extern	DATA BYTE	RemoData[4];
	extern	IDATA BYTE  RemoDataReady;
	extern	IDATA BYTE  RemoNum, RemoBit;
	//extern	IDATA BYTE  RemoDataCode;
#endif // REMO 
extern 	BYTE timer2_owner;
extern	BYTE timer2_tic;


extern	DATA	BYTE	INT_STATUS;
extern	DATA	BYTE	INT_MASK;
extern	DATA	BYTE	INT_STATUS2;
extern	DATA	BYTE	INT_STATUS3;
extern	DATA	BYTE	EXINT_STATUS;
extern  DATA	WORD	MCU_INT_STATUS;
extern	DATA	WORD	VH_Loss_Changed;
extern	DATA	BYTE	INT_STATUS_ACC;
extern	DATA	BYTE	INT_STATUS2_ACC;

extern			bit		RM_get;

//-------------------------- Freerun ------------------------------------------
extern XDATA WORD global_Freerun_hTotal;
extern XDATA WORD global_Freerun_vTotal;

//-------------------------- MHL CEC ------------------------------------------
extern BYTE global_CEC_flag;

//-------------------------- SPI ----------------------------------------------
// see SPI_Buffer[SPI_BUFFER_SIZE]
//-------------------------- FontOsd Message ----------------------------------
extern BYTE FOsdMsgBuff[];

//-------------------------- Debugging & Monitor ------------------------------

extern	XDATA	BYTE 	DebugLevel;
extern	XDATA	BYTE	MonAddress;	
extern	XDATA	WORD	MonIndex;
extern	XDATA	DWORD	MonWdata;
extern	XDATA	BYTE	monstr[];		// buffer for input string
extern	XDATA	BYTE 	*argv[];
extern	XDATA	BYTE	argc;
extern			bit		echo;
extern	XDATA	BYTE	SW_key;

extern	XDATA	BYTE 	*argv1[];
extern	XDATA	BYTE	argc1;

extern			BYTE 	*MonString;

extern	struct st_animation AnimationData[];
extern          bit 	AnimationON;
extern			bit		g_access;

//===================== KeyRemo =================================================
extern WORD		TouchX, TouchY;
extern bit		TouchAuto, TraceAuto;
extern bit		tsc_debug;
//--from CPU
extern  WORD	CpuTouchX, CpuTouchY, CpuZ1, CpuZ2;
extern 	BYTE /*bit*/		CpuTouchPressed;
extern  WORD	CpuAUX0;
extern  WORD	CpuAUX1;
extern  WORD	CpuAUX2;
extern  WORD	CpuAUX3;
extern  BYTE	CpuAUX0_Changed;
extern  BYTE	CpuAUX1_Changed;
extern  BYTE	CpuAUX2_Changed;
extern  BYTE	CpuAUX3_Changed;
extern  BYTE	CpuTouchStep, CpuTouchChanged;
extern  WORD 	CpuTouchSkipCount;


//===================== debug =================================================
//extern DWORD GlobalBootTime;
//extern BYTE FirstInitDone;

void WaitVBlank(BYTE cnt);		//@inputctrl.h
void WaitOneVBlank(void);

void UpdateOsdTimerClock(void);

//===================== fast toggle============================================
typedef struct REG_IDX_DATA_s {
	WORD idx;
	BYTE value;
} REG_IDX_DATA_t;

extern BYTE g_fast_toggle;
extern BYTE g_cvbs_checked;
extern BYTE g_hdmi_checked;
extern XDATA REG_IDX_DATA_t Fast_Cvbs_Buff[];
extern XDATA REG_IDX_DATA_t Fast_Hdmi_Buff[];

//===================== Image Control==========================================

extern BYTE OsdTime;

extern BYTE watchdog_focused_curr_menu;
extern BYTE watchdog_selected_curr_menu;


#endif // __GLOBAL_H__