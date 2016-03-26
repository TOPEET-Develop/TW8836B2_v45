/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/
#ifndef __DEBUG__
#define __DEBUG__

#define DEBUG_ERR	1
#define DEBUG_WARN	2
#define DEBUG_INFO	3
#define DEBUG_BREAK	4

#define Pause(a)	{ Printf("\r\n"); Printf(a); while(!RS_ready()); RS_rx(); }
//#define assert(x)	
#define assert(x) if((x)==0) { Printf("\n\rAssertion failed: line:%d",__LINE__); }

#endif	// __DEBUG__
