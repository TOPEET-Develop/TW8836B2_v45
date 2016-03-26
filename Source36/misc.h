/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/
#ifndef __MISC_H__
#define __MISC_H__

#if defined(SUPPORT_I2CCMD_SERVER)
extern bit F_i2ccmd_exec;				/*!< I2CCMD flag */
#define I2CCMD_CHECK	0x20
#define I2CCMD_EXEC		0x10
#endif

BYTE InitSystemForChipTest(BYTE fPowerUpBoot);
void Init_I2CCMD_Slave(void);
BYTE I2CCMD_exec_main(void);

#endif //__MISC_H__
