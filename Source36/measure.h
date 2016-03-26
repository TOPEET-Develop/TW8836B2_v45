/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/
#ifndef  _MEASURE_H_
#define _MEASURE_H_


extern WORD	MeasVLen, MeasVStart; 
extern WORD MeasHLen, MeasHStart;
extern WORD	RGB_hStart;
extern BYTE	RGB_vDE;

//=====================================
// Level 0
//=====================================

void MeasSetWindow( WORD x, WORD y, WORD w, WORD h );
//void MeasSetField(BYTE field);
void MeasLockData(BYTE fLock);
BYTE MeasStartMeasure(void);
void Meas_StartMeasure(void);
BYTE Meas_IsMeasureDone(BYTE cnt);

//BYTE MeasureAndWait(BYTE field);
BYTE MeasSetInputClkForHPeriodReg(BYTE fOn);
void MeasSetNoiseMask(BYTE mask);
void MeasSetErrTolerance(BYTE value);
void MeasEnableChangedDetection(BYTE on);
void MeasEnableDeMeasure(BYTE fOn);
//void MeasSetThreshold(BYTE thold);

WORD MeasGetVPeriod( void );
WORD MeasGetHPeriodReg( void );
WORD MeasGetHTotal(BYTE hPol);
WORD meas_get_hperiod_reg( void );
BYTE MeasGetHFreq(void);
WORD MeasGetHSyncRiseToFallWidth(void);
WORD MeasGetHSyncRiseToHActiveEnd(void);
WORD MeasGetVSyncRiseToFallWidth(void);
WORD MeasGetVsyncRisePos(void);
WORD MeasGetHActive( WORD *start );
WORD MeasGetHActive2( void );
WORD MeasGetDviDivider(void);
BYTE MesaGetHPol(void);

WORD MeasGetVActive( WORD *start );
WORD MeasGetVActive2(void);
DWORD	MeasGetVPeriod27( void );
BYTE MeasGetVFreq(void);
BYTE MeasRoundDownVFreqValue(BYTE vfreq);


BYTE MeasCheckVPulse(void);
//=====================================
// Level 1
//=====================================

void AutoTunePhase(void);
void AutoTuneColor(void);

void CheckMeasure(void);

void Measure_VideoTiming(void);

#endif
