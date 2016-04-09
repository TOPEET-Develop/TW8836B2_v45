/*
* I2C.H
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
#ifndef	__I2C_H__
#define	__I2C_H__

/**
* assign I2C port
*/
#define	I2C_SCL		P1_0
#define	I2C_SDA		P1_1

extern DATA BYTE I2C_delay_base;
//extern BYTE I2C_Buffer[16];

//for test
//void I2C_Start(void);
//void I2C_Stop(void);
//BYTE I2C_WriteData(BYTE value);
//BYTE I2C_ReadData(BYTE fLast);


BYTE CheckI2C(BYTE i2cid);
BYTE ReadI2CByte(BYTE i2cid, BYTE index);
BYTE ReadI2CS(BYTE i2cid, BYTE index, BYTE *buff, BYTE cnt);
BYTE WriteI2CByte(BYTE i2cid, BYTE index, BYTE val);
BYTE WriteI2CS(BYTE i2cid, BYTE index, BYTE *buff, BYTE cnt);

DWORD ReadI2C_multi(BYTE i2cid, BYTE config, WORD index);
BYTE ReadI2CS_multi(BYTE i2cid, BYTE config, WORD index, void *value, BYTE count);
BYTE WriteI2C_multi(BYTE i2cid, BYTE config, WORD index, DWORD value);
BYTE WriteI2CS_multi(BYTE i2cid, BYTE config, WORD index, void *value, BYTE count);

BYTE ReadI2C_Only(BYTE i2cid);
BYTE ReadI2CS_Only(BYTE i2cid, BYTE *val, BYTE _cnt);

//=================
// global macro
//=================


//=================
// I2C device
//=================
#define I2CID_TW9910		0x88
#define I2CID_SX1504		0x40	//4CH GPIO
#define I2CID_BU9969		0xE0	//Digital video encoder
#define I2CID_ADV7390		0xD6	//Digital video encoder. 12bit
#define I2CID_ADV7391		0x56	//Digital video encoder. 10bit

#define I2CID_ISL97671A		0x58	//LED BackLight

#define I2CID_SIL9127_DEV0	0x60	//SiliconImage HDMI receiver
#define I2CID_SIL9127_DEV1	0x68
#define I2CID_SIL9127_HDCP	0x74
#define I2CID_SIL9127_COLOR	0x64
#define I2CID_SIL9127_CEC	0xC0
#define I2CID_SIL9127_EDID	0xE0
#define I2CID_SIL9127_CBUS	0xE6

#define I2CID_ISL97901		0x50	//ISL RGB LED Driver
#define I2CID_ADC121C021	0xAC	//12bit Analog2Digital Converter
#define I2CID_E330_FLCOS	0x7C	//FLCOS	 
#define I2CID_ISL97671		0x58	//PWM Dimming


BYTE WriteI2C_8A(BYTE index, BYTE value);
BYTE ReadI2C_8A(BYTE index);
BYTE WriteI2CS_8A(BYTE index, BYTE len, BYTE *buff);
BYTE ReadI2CS_8A(BYTE index, BYTE len, BYTE *buff);

#endif	// __I2C_H__

