/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/
#ifndef __MAIN__
#define __MAIN__


#define SYS_MODE_NORMAL		0
#define SYS_MODE_NOINIT		1
#define SYS_MODE_RCD		2
#define SYS_MODE_WATCHDOG	3	//
extern BYTE CurrSystemMode;

#define SW_INTR_VIDEO		0x80
#define SW_INTR_EXTERN		0x40

#define EXT_I2C_REQ_ANSWER	0x20
#define EXT_I2C_DONE		0x10
#define EXT_I2C_ACK1		0xA1
#define EXT_I2C_ACK2		0xA2
#define EXT_I2C_NAK2		0xB2	
		
void InitCore(BYTE fManual);
//void InitCore_on_watchdog_reset(void);
BYTE InitSystem(BYTE _fPowerUpBoot);
//BYTE InitSystem_on_watchdog_reset(void);
void StartVideoInput(void);


void TaskSetGrid(BYTE onoff);
BYTE TaskGetGrid(void);
void TaskSetGridCmd(BYTE cmd);
BYTE TaskGetGridCmd(void);

#define TASK_CMD_DONE			0
#define TASK_CMD_WAIT_VIDEO		1
#define TASK_CMD_WAIT_MODE		2
#define TASK_CMD_RUN			3
#define TASK_CMD_RUN_FORCE		4

void TaskNoSignal_setCmd(BYTE cmd);
BYTE TaskNoSignal_getCmd(void);

void Interrupt_enableVideoDetect(BYTE fOn);
void Interrupt_enableSyncDetect(BYTE fOn);

void LinkCheckAndSetInput(void);
extern BYTE (*CheckAndSetInput)(void);
BYTE CheckAndSetUnknown(void);
BYTE IsBackDrivePortOn(void);

void SystemPowerSave(void);
void SystemPowerResume(void);

void InitClockAsDefault(BYTE mode);
BYTE InitRCDMode(BYTE fPowerUpBoot);

#endif	// __MAIN__
