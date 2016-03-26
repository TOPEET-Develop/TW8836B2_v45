/* 
DebugMsg.h
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

#if  defined(SUPPORT_HDMI_EP907M)

void DBG_PrintAviInfoFrame(void);
void DBG_PrintTimingRegister(void);
void DBG_DumpControlRegister(void);

#else 
void Dummy_DebugMsg_func(void);
#endif

void DumpDviTable(WORD hActive,WORD vActive);

void DbgMsg_EEP_Corruptted(void);

