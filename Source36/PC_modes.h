/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/

#ifndef __PC_MODE_DATA__
#define __PC_MODE_DATA__

extern code struct _PCMODEDATA PCMDATA[];
extern CONST struct _PCMODEDATA DVIMDATA[] ;

DWORD sizeof_PCMDATA(void);
DWORD sizeof_DVIMDATA(void);

void PC_PrepareInfoString(BYTE mode);
void YUV_PrepareInfoString(BYTE mode);
void DVI_PrepareInfoString(WORD han, WORD van, BYTE vfreq);

#endif

