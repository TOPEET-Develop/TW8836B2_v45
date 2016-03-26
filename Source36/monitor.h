/* monitor.h */
/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/


void Mon_tx(BYTE ch);

void SoftReset(void);
void MonHelp(void);
BYTE MonGetCommand(void);
void Monitor(void);
void Monitor1(void);
void Prompt(void);
void Prompt1(void);
void WaitUserInput(void);

BYTE GetMonAddress(void);
void SetMonAddress(BYTE addr);
BYTE MonReadI2CByte(void);
BYTE MonWriteI2CByte(void);

void DeltaRGBPanelInit(void);

//=====monitor_mcu.c============
BYTE WriteSFR(BYTE addr, BYTE dat);
BYTE ReadSFR(BYTE addr);
void MonitorMCU(void);
void Monitor1MCU(void);
