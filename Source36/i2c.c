/**
 * @file
 * i2c.c
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	a device driver for the iic-bus interface 
 ******************************************************************************
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
#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"
#include "global.h"

#include "printf.h"
#include "CPU.h"
#include "util.h"

#include "I2C.h"

#include "SOsd.h"

#include <intrins.h>

//------------------------
// only for FAST or Slow environment
//------------------------

//BYTE I2C_Bus;	//0:Bus0 for TW8835, 1:BUS1 for TW8809, 2:BUS2 for TW8832.
//BYTE I2C_Idx;	//I2C address index mode. 0:8bit 1:16bit.
//BYTE I2C_Speed=1;	//0:normal, 1:slow, 2:for HS_mode, but not implememted.

void dummy_i2c_code(void) 
{
    Puts("\dummy i2c_code\n");
}

/**
* I2C Buffer
*/
//BYTE I2C_Buffer[16];
/**
* I2C delay base value
* I2C uses a loop to make a delay.
* Its speed is depend on the MCU speed and cache.
* If MCU is 27MHz, use 1.
*/
DATA BYTE I2C_delay_base = 1;

#if defined(SUPPORT_I2C_MASTER)

//=============================
// I2C TIME CHART
//=============================
/**
* I2C TIME CHART
*
*
    -+      +----------+---------+...----+   ...-+     +-------+				
SDA  |      |          |         |       |       |     |       |
     +------+----------+---------+...    +---...-+-----+       +--
												
SCL ---+     +----+      +----+      +----+          +------------+
       |     |    |      |    |      |    |          |         	  |
       +-----+    +------+    +-..---+    +--......--+            +--
     | |     |    |    | |           |  |          | | |
     |1|  2  |  3 |5   |6|           | 4|          |9|7|   8   |
     |                                  |              |
     |                                  |              |
     +START                              +RESTART      +STOP
   (4 1 5) (6  3   5)                              (9 7 8)
																	   TW8835 Slave	
																	   MIN		MAX
1: Hold Time START Condition											74ns	-
2: Clock Low Time(5+6)
3: Clock High Time								i2c_delay_clockhigh		
4: Setup Time for a Repeated START Condition							370ns	-
5: Data Hold Time								i2c_delay_datahold		50ns	900ns
6: Data Setup Time								i2c_delay_datasetup		74ns	-
7: Set-up time for STOP condition										370ns	-
8: Bus Free Time between a STOP and a START Condition 					740ns	 -
9: prepare time for STOP condition
A: ack wait time


I2C SPEED
=========
	slow			depend on system.
	Standard		100kbps
	Fast mode		400kbps
	Fast mode plus	1Mbit
	High-Speed		3.4Mbit				HS-mode

	TW8836 uses about 35kHz.

*/

//-----------------------
// I2C DELAY
// Note: It depends on CACHE, MCUSPI clock, & SPIOSD.
//-----------------------

#pragma SAVE
#pragma OPTIMIZE(2,SIZE)
static void dd(BYTE delay)
{
	DATA BYTE i,j;
	j = I2C_delay_base;
	while(j--) {
		for(i=0; i < delay; i++);
	}
}
#pragma RESTORE

#if 0 //test
                                    //32kHz
#define I2CDelay_1		dd(5)		//13uS				
#define I2CDelay_2		dd(10)		//22uS		
#define I2CDelay_3		dd(5)		//9.6uS		
#define I2CDelay_4		dd(20)		//	
#define I2CDelay_5		dd(3)		//
#define I2CDelay_6		dd(2)		//
#define I2CDelay_7		dd(3)		//	
#define I2CDelay_8		dd(30)		//	
#define I2CDelay_9		dd(5)		//
#define I2CDelay_ACK	dd(2)		//
#else
                                    //32kHz
#define I2CDelay_1		dd(8)		//13uS				
#define I2CDelay_2		dd(13)		//22uS		
#define I2CDelay_3		dd(5)		//9.6uS		
#define I2CDelay_4		dd(37)		//	
#define I2CDelay_5		dd(5)		//
#define I2CDelay_6		dd(8)		//
#define I2CDelay_7		dd(37)		//	
#define I2CDelay_8		dd(74)		//	
#define I2CDelay_9		dd(10)		//
#define I2CDelay_ACK	dd(8)		//
#endif

//=============================================================================
/**
* I2C subroutines
*/
//=============================================================================
#pragma SAVE
#pragma OPTIMIZE(2,SIZE)
static BYTE I2C_SetSclWait(void)
{
//#if 0 
//	//It is fast, but, if your HW has a bug,
//	//it can hangup the system. 
//	I2C_SCL=1;		
//	while(I2C_SCL==0);
//	return 0;	
//#else
	BYTE i;
	I2C_SCL=1;		
	for(i=0; i < 250; i++) {
		dd(10);
		if(I2C_SCL != 0)
			break;
	}
	if(i>=250) {
		Printf("\n\rI2C_SetSclWait Fail");
		return 1;	//fail
	}
	return 0;
//#endif
}
#pragma RESTORE


//static void I2C_Start(void)
void I2C_Start(void)
{
	I2C_SDA = 1;	
	I2C_SCL = 1;	
							I2CDelay_4;
	I2C_SDA = 0;			I2CDelay_1;
	I2C_SCL = 0;			I2CDelay_5;
}

//static void I2C_Stop(void)
void I2C_Stop(void)
{
	I2C_SDA = 0;			I2CDelay_9;
	I2C_SetSclWait();		I2CDelay_7;
	I2C_SDA = 1;			I2CDelay_8;	
}

//static BYTE I2C_WriteData(BYTE value)
BYTE I2C_WriteData(BYTE value)
{
	BYTE error;
	BYTE i;

	for(i=0;i<8;i++) {
		if(value & 0x80)	I2C_SDA = 1;
		else				I2C_SDA = 0;
							I2CDelay_6;
		I2C_SCL = 1; 		I2CDelay_3;
		I2C_SCL = 0;		I2CDelay_5;

		value <<=1;
	}
	I2C_SDA = 1;			//listen for ACK
	                        
	I2C_SetSclWait();		I2CDelay_ACK;
	if(I2C_SDA)	error=1;
	else        error=0;
	                        
	I2C_SCL=0;				I2CDelay_5;

	return error;
#if 0 //only for test
	DATA BYTE i;

	for(i=0;i<8;i++) {
		if(value & 0x80)	I2C_SDA = 1;
		else				I2C_SDA = 0;
							I2CDelay_6;
		I2C_SCL = 1; 		I2CDelay_3;
		I2C_SCL = 0;		I2CDelay_5;

		value <<=1;
	}
	I2C_SDA = 1;			//listen for ACK
	                        
//	I2C_SetSclWait();		
	I2C_SCL=1;		
	for(i=0; i < 250; i++) {
		dd(2); //BK160218   dd(10);
		if(I2C_SCL != 0)
			break;
	}
                            I2CDelay_ACK;	                        
	I2C_SCL=0;				I2CDelay_5;

	return 0;
#endif
}
//static BYTE I2C_ReadData(BYTE fLast)
BYTE I2C_ReadData(BYTE fLast)
{
	BYTE i;
	BYTE val=0;

	for(i=0; i <8; i++) {
							I2CDelay_6;
		I2C_SetSclWait();	I2CDelay_3;
		val <<= 1;
		if(I2C_SDA)
			val |= 1;
		I2C_SCL=0;			I2CDelay_5;
	}
	if(fLast)	I2C_SDA = 1;	//last byte
	else		I2C_SDA = 0;

	I2C_SetSclWait();		I2CDelay_3;
	I2C_SCL=0;
	I2C_SDA=1;				I2CDelay_5;
	return val;
}



//=============================================================================
/**
* I2C global Functions
*
* BYTE CheckI2C(BYTE i2cid)
*
* BYTE WriteI2CByte(BYTE i2cid, BYTE index, BYTE val)
* void WriteI2C(BYTE i2cid, BYTE index, BYTE *val, WORD cnt)
* BYTE ReadI2CByte(BYTE i2cid, BYTE index)
* void ReadI2C(BYTE i2cid, BYTE index, BYTE *val, BYTE cnt)
*/	                                                                          
//=============================================================================
/**
* check I2C device
*
* I2C commands use a infinity loop. 
* Use CheckI2C() first before you use other I2C commands.
*
* @return
*	0: success
*	1: NAK
*	2: I2C dead
*
* NOTE: I am not using I2CSetSclWait().
*/
BYTE CheckI2C(BYTE i2cid)
{
	BYTE error;
	BYTE i;
	BYTE value;

	value = i2cid;
	//--SFRB_EA=0;
	I2C_Start();

	for(i=0;i<8;i++) {
		if(value & 0x80) I2C_SDA = 1;
		else             I2C_SDA = 0;
							I2CDelay_6;
		I2C_SCL = 1; 		I2CDelay_3;
		I2C_SCL = 0;		I2CDelay_5;

		value <<=1;
	}
	I2C_SDA = 1;			//listen for ACK.                      
	I2C_SCL=1; 				I2CDelay_ACK;
	dd(100);
	if(I2C_SCL==0)	error = 2;	//I2C dead
	else {
		if(I2C_SDA)	error=1;	//NAK
		else        error=0;	//ACK
	}                        
	I2C_SCL=0;				I2CDelay_5;

	//stop routine
	I2C_SDA = 0;		I2CDelay_9;                      
	I2C_SCL=1; 	 		I2CDelay_7;
	I2C_SDA = 1;		I2CDelay_8;	

	//--SFRB_EA=1;
	return error;
}
/**
* write one byte data to I2C slave device
*
* @param i2cid - 8bit.
* @param index - 8bit 
* @param data
*/
BYTE WriteI2CByte(BYTE i2cid, BYTE index, BYTE val)
{
	BYTE ret;

	//--SFRB_EA=0;
	I2C_Start();
	ret  = I2C_WriteData(i2cid);	ret <<=1;
	ret |= I2C_WriteData(index);	ret <<=1;
	ret |= I2C_WriteData(val);
	I2C_Stop();
	//--SFRB_EA=1;

#if defined(DEBUG_I2C)
	if(ret)
		Printf("\n\rWriteI2CByte[%bx:%bx,%bx] FAIL:%bx",i2cid, index,val, ret);
#endif
	return ret;
}


/**
* write data to I2C slave device
*
* @param i2cid - 8bit
* @param index - 8bit
* @param *val. NOTE: Do not use a CodeSegment
* @param count
*/
BYTE WriteI2CS(BYTE i2cid, BYTE index, BYTE *val, BYTE _cnt)
{
	WORD cnt, i;
	BYTE ret;

    cnt = _cnt;
    if(cnt==0) 
        cnt=256;

	//--SFRB_EA=0;
	I2C_Start();
	ret  = I2C_WriteData(i2cid);	ret <<=1;
	ret |= I2C_WriteData(index);	ret <<=1;
	for(i=0;i<cnt;i++) 
		ret |= I2C_WriteData(val[i]);
	I2C_Stop();
	//--SFRB_EA=1;
#if defined(DEBUG_I2C)
	if(ret)
		Printf("\n\rWriteBlock2C[%bx:%bx,%bx] FAIL:%bx",i2cid, index,val, ret);
#endif
	return ret;
#if 0 //only for test
	WORD cnt, i;

    cnt = _cnt;
    if(cnt==0) 
        cnt=256;

	//--SFRB_EA=0;
	I2C_Start();
	I2C_WriteData(i2cid);
	I2C_WriteData(index);
	for(i=0;i<cnt;i++) 
		I2C_WriteData(val[i]);
	I2C_Stop();
	//--SFRB_EA=1;
	return 0;
#endif
}

/**
* read only commnad
* It should be combined with WriteI2CS(i2cid,index,NULL,0).
*
* @param i2cid - 8bit
* @return data
*/
BYTE ReadI2C_Only(BYTE i2cid)
{
	BYTE value;

	//--SFRB_EA=0;
	I2C_Start();		  
	I2C_WriteData(i2cid | 0x01);
	value=I2C_ReadData(1);
	I2C_Stop();
	//--SFRB_EA=1;

	return value;
}


/**
* read one byte data from I2C slave device
*
* @param i2cid - 8bit
* @param index - 8bit
* @return data
*/
BYTE ReadI2CByte(BYTE i2cid, BYTE index)
{
	BYTE value;
	BYTE ret;

	//--SFRB_EA=0;
	I2C_Start();		  
	ret = I2C_WriteData(i2cid);		ret<<=1;
	ret +=I2C_WriteData(index);		ret<<=1;
	I2C_Start();
	ret +=I2C_WriteData(i2cid | 0x01);
	value=I2C_ReadData(1);
	I2C_Stop();
	//--SFRB_EA=1;

#if defined(DEBUG_I2C)
	if(ret)
		Printf("\n\rReadI2CByte[%bx:%bx] FAIL:%bx",i2cid,index, ret);
#endif
	return value;
}

/**
* read only command
* It should be combined with WriteI2CS(i2cid,index,NULL,0).
*
* @param i2cid - 8bit
* @param *val - read back buffer
* @param count
* @return NA
*/
BYTE ReadI2CS_Only(BYTE i2cid, BYTE *val, BYTE _cnt)
{
	WORD i;
	BYTE ret;
    WORD cnt;

    cnt = _cnt;
    if(_cnt==0)
        cnt=256;

	//--SFRB_EA=0;
	I2C_Start();
	ret |= I2C_WriteData(i2cid | 0x01);
	cnt--;
	for(i=0; i<cnt; i++){
		val[i]=I2C_ReadData(0);
	}
	val[i]=I2C_ReadData(1);
  
	I2C_Stop();
	//--SFRB_EA=1;

	return ret;
}


/**
* read data from I2C slave device
*
* @param i2cid - 8bit
* @param index - 8bit 
* @param *val - read back buffer
* @param count
* @return 0:success, other:fail
*/
BYTE ReadI2CS(BYTE i2cid, BYTE index, BYTE *val, BYTE cnt)
{
	BYTE i;
	BYTE ret;

	//--SFRB_EA=0;
	I2C_Start();
	ret  = I2C_WriteData(i2cid);		ret <<=1;
	ret |= I2C_WriteData(index);		ret <<=1;
	I2C_Start();
	ret |= I2C_WriteData(i2cid | 0x01);
	cnt--;
	for(i=0; i<cnt; i++){
		val[i]=I2C_ReadData(0);
	}
	val[i]=I2C_ReadData(1);
  
	I2C_Stop();
	//--SFRB_EA=1;
#if defined(DEBUG_I2C)
	if(ret)
		Printf("\n\rReadI2C[%bx:%bx] FAIL:%bx",i2cid,index, ret);
#endif
	return ret;
}


//=============================================================================
// I2C_multi functions                                                                          
//  DWORD ReadI2C_multi(BYTE i2cid, BYTE config, WORD index)
//  BYTE ReadI2CS_multi(BYTE i2cid, BYTE config, WORD index, void *value, BYTE count)
//  BYTE WriteI2C_multi(BYTE i2cid, BYTE config, WORD index, DWORD value)
//  BYTE WriteI2CS_multi(BYTE i2cid, BYTE config, WORD index, void *value, BYTE count)
//=============================================================================
DWORD ReadI2C_multi(BYTE i2cid, BYTE config, WORD index)
{
	BYTE size, data_size;
	BYTE ret;
	DWORD value;

	size = config >>4;			//index size
	data_size = config & 0x0F;	//data size


	//--SFRB_EA=0;
	I2C_Start();		  
	ret = I2C_WriteData(i2cid);			ret<<=1;
	if(size==2)
		ret+=I2C_WriteData(index>>8);	ret<<=1;
	ret +=I2C_WriteData(index);			ret<<=1;

	I2C_Start();
	ret +=I2C_WriteData(i2cid | 0x01);

	//I2C read MSB first.
	size = data_size;
	value = 0;
	while(size) {
		size--;	
		if(size) {
			value |= I2C_ReadData(0);
			value <<= 8;
		}
		else 
			value |= I2C_ReadData(1);
	}

	I2C_Stop();
	//--SFRB_EA=1;

	return value;
}
/**
* count is -1.
*/
BYTE ReadI2CS_multi(BYTE i2cid, BYTE config, WORD index, void *value, BYTE _count)
{
	BYTE *pBYTE;
	BYTE size, data_size;
	BYTE ret;
    WORD count;

    count = _count;
    if(_count==0)
        count=256;
	pBYTE = (BYTE *)value;

	size = config >>4;			//index size
	data_size = config & 0x0F;	//data size


	//--SFRB_EA=0;
	I2C_Start();		  
	ret = I2C_WriteData(i2cid);			ret<<=1;
	if(size==2)
		ret+=I2C_WriteData(index>>8);	ret<<=1;
	ret +=I2C_WriteData(index);			ret<<=1;

	I2C_Start();
	ret +=I2C_WriteData(i2cid | 0x01);

	//I2C read MSB first
    while(count--) {
		size = data_size;
		while(size) {
			size--;	
			if(count || size)
				*pBYTE++ = I2C_ReadData(0);
			else
				*pBYTE = I2C_ReadData(1); //last		
		}
	}

	I2C_Stop();
	//--SFRB_EA=1;

	return 0;
}

BYTE WriteI2C_multi(BYTE i2cid, BYTE config, WORD index, DWORD value)
{
	BYTE ret;
	BYTE bTemp;
	BYTE size;

	size = config >>4;	/*index size. 1 or 2 */
	//--SFRB_EA=0;
	I2C_Start();		  
	ret= I2C_WriteData(i2cid);				ret<<=1;
	if(size==2)
		ret+=I2C_WriteData(index>>8);		ret<<=1;
	ret+=I2C_WriteData((BYTE)index);		ret<<=1;

	size = config & 0x0F;	/*data size, 1,2,or 4*/
	while(size) {
		size--;
		bTemp = value >> (size*8);
		ret |= I2C_WriteData(bTemp);
	}
	I2C_Stop();
	//--SFRB_EA=1;
#if defined(DEBUG_I2C)
	if(ret)
		Printf("=>Fail ret:%bx",ret);
#endif

	return ret;
}

// config[7:4] index size
// config[3:0] data size
// count start from 0. If want to write 256, assign 255.    BKTODO::change it
/*
* count is -1
*/
BYTE WriteI2CS_multi(BYTE i2cid, BYTE config, WORD index, void *value, BYTE _count)
{
	BYTE *pBYTE;
	BYTE size, data_size;
	BYTE ret;
    WORD count;

    count=_count;
    if(_count==0)
        count=256;

	pBYTE = (BYTE *)value;

	size = config >>4;			//index size
	data_size = config & 0x0F;	//data size

	//--SFRB_EA=0;
	I2C_Start();		  
	ret= I2C_WriteData(i2cid);				ret<<=1;
	if(size==2)
		ret+=I2C_WriteData(index>>8);		ret<<=1;
	ret+=I2C_WriteData((BYTE)index);		ret<<=1;

	// write MSB first.
    while(count--) {
		size = data_size;
		while(size) {
			size--;
			ret |= I2C_WriteData(*pBYTE++);
		}
		if(ret)
			break;
	}

	I2C_Stop();
	//--SFRB_EA=1;

	return ret;
}

//=============================================================================
// Linux style functions                                                                          
//  WriteI2C_8A
//  ReadI2C_8A
//  WriteI2CS_8A
//  ReadI2CS_8A
//=============================================================================
BYTE WriteI2C_8A(BYTE index, BYTE value)
{
	//--SFRB_EA=0;
	I2C_Start();
	I2C_WriteData(0x8A);
	I2C_WriteData(index);
	I2C_WriteData(value);
	I2C_Stop();
	//--SFRB_EA=1;
	return 0;
}
BYTE ReadI2C_8A(BYTE index)
{
	BYTE value;

	//--SFRB_EA=0;
	I2C_Start();
	I2C_WriteData(0x8A);
	I2C_WriteData(index);
	I2C_Stop();
	//--SFRB_EA=1;

	//--SFRB_EA=0;
	I2C_Start();
	I2C_WriteData(0x8B);
	value = I2C_ReadData(1);
	I2C_Stop();
	//--SFRB_EA=1;
	return value;
}
BYTE WriteI2CS_8A(BYTE index, BYTE len, BYTE *buff)
{
	BYTE value;
	WORD i,n;

	if(len==0)	n=256;
	else		n=len;

	//--SFRB_EA=0;
	I2C_Start();
	I2C_WriteData(0x8A);
	I2C_WriteData(index);
	for(i=0; i < n; i++) {
		value = *buff++;
		I2C_WriteData(value);
	}
	I2C_Stop();
	//--SFRB_EA=1;
	return 0;
}
BYTE ReadI2CS_8A(BYTE index, BYTE len, BYTE *buff)
{
	BYTE value;
	WORD i,n;

	if(len==0)	n=256;
	else		n=len;

	//--SFRB_EA=0;
	I2C_Start();
	I2C_WriteData(0x8A); /* i2cid */
	I2C_WriteData(index);
	I2C_Stop();
	//--SFRB_EA=1;

	//--SFRB_EA=0;
	I2C_Start();
	I2C_WriteData(0x8B);
	for(i=0; i < n; i++) {
		if((i+1)==n)	value=I2C_ReadData(1);
		else			value=I2C_ReadData(0);
		*buff++ = value;
	}
	I2C_Stop();
	//--SFRB_EA=1;
	return 0;
}


#endif //..SUPPORT_I2C_MASTER
