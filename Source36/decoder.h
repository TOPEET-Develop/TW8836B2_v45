/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/

#ifndef _DECODER_H_
#define _DECODER_H_

BYTE DecoderCheckVDLOSS( BYTE n );
BYTE DecoderCheckSTD( BYTE n );
BYTE DecoderReadDetectedMode(void);
#define DECODER_FREERUN_AUTO	0
#define DECODER_FREERUN_60HZ	2
void DecoderFreerun(BYTE mode);
void DecoderSetOutputCrop(WORD hDelay, WORD hActive, WORD vDelay, WORD vActive);
WORD DecoderGet_vDelay(void);
WORD DecoderGet_vActive(void);
WORD DecoderGet_hDelay(void);
WORD DecoderGet_hActive(void);
BYTE CheckAndSetDecoderScaler( void );
BYTE ChangeCVBS( void );
BYTE ChangeSVIDEO( void );

#ifdef SUPPORT_FOSD_MENU
BYTE DecoderIsNoInput(void);
BYTE DecoderReadVInputSTD(void);
#endif

#endif
