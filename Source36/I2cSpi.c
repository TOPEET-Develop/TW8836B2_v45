/**
 * @file
 * 	I2cSpi.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2015~2015 Intersil Corporation
 * @section DESCRIPTION
 *	i2cspi dma.
 *  to support i2cspi, we needs two TW8836B board.
 *  And, this is for the master board.
 *  The master board uses i2c ports() to control the slave board.
 *  To upload the master board firmware, use ext_mcu_i2c() ports.
 *
 *  To save the slave's hex file, I am using 0x60000~0x7FFFF;(max 128K).
 *
 * @section DESCRIPTION
 *	- CPU : DP80390
 *	- Language: Keil C
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
/*
 at 104MHz clock
    530KHz on I2C_delay_base=0.
    84.4KHz on I2C_delay_base=1.
    32.6KHz on I2C_delay_base=4.
     
*/
#include <intrins.h>
#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "Global.h"
#include "CPU.h"
#include "Printf.h"
#include "util.h"
#include "Monitor.h"
#include "I2C.h"
#include "SPI.h"
#include "I2cSpi.h"
#include "main.h"
#include "Debug.H"


//#define USE_HW_RESET_PIN


/* defines for customer codes.
*/
#define uint32_t	DWORD
#define uint16_t	WORD
#define uint8_t		BYTE

#define FLASH_BLOCK_SIZE	0x10000 /* 64KBytes */
#define FLASH_SECTOR_SIZE	0x01000 /* 4KBytes */
#define XMEM_BUFF_SIZE		128		/* same as SPI_BUFFER_SIZE */

#define port_TW8836_reverse	P1_6	/*pin114:GPIO60/INT13/P1.6/VD0*/
#define port_TW8836_lock	P1_7	/*pin115:GPIO61/INT13/P1.7/VD1*/
#define pin_TW8836_RESET	62		/*pin116:GPIO62/Gate0/P3.2/VD2*/
#define pin_TW8836_MCU		63		/*pin117:GPIO63/Gate1/P3.3/VD3*/

//------------
//prototype
//------------

/* SPI timing
                        typ max
    WriteStatus         --  40ms
    SectorErase         43  200ms
    BlockErase          340 2000ms
    PageProgram         0.6 3ms
*/

BYTE I2CSPI_4B_mode;    /* SPI 4Byte address mode */
BYTE I2CSPI_mid;        /* Manufactory ID */
BYTE I2CSPI_size;       /* memory density */
WORD g_CRC;             /* CRC value */

/*CRC16  table
  (X^16 + X^12 + X^5 + 1) */
CODE WORD crctab[256] = {
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50A5,  0x60C6,  0x70E7,
    0x8108,  0x9129,  0xA14A,  0xB16B,  0xC18C,  0xD1AD,  0xE1CE,  0xF1EF,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52B5,  0x4294,  0x72F7,  0x62D6,
    0x9339,  0x8318,  0xB37B,  0xA35A,  0xD3BD,  0xC39C,  0xF3FF,  0xE3DE,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64E6,  0x74C7,  0x44A4,  0x5485,
    0xA56A,  0xB54B,  0x8528,  0x9509,  0xE5EE,  0xF5CF,  0xC5AC,  0xD58D,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76D7,  0x66F6,  0x5695,  0x46B4,
    0xB75B,  0xA77A,  0x9719,  0x8738,  0xF7DF,  0xE7FE,  0xD79D,  0xC7BC,
    0x48C4,  0x58E5,  0x6886,  0x78A7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xC9CC,  0xD9ED,  0xE98E,  0xF9AF,  0x8948,  0x9969,  0xA90A,  0xB92B,
    0x5AF5,  0x4AD4,  0x7AB7,  0x6A96,  0x1A71,  0x0A50,  0x3A33,  0x2A12,
    0xDBFD,  0xCBDC,  0xFBBF,  0xEB9E,  0x9B79,  0x8B58,  0xBB3B,  0xAB1A,
    0x6CA6,  0x7C87,  0x4CE4,  0x5CC5,  0x2C22,  0x3C03,  0x0C60,  0x1C41,
    0xEDAE,  0xFD8F,  0xCDEC,  0xDDCD,  0xAD2A,  0xBD0B,  0x8D68,  0x9D49,
    0x7E97,  0x6EB6,  0x5ED5,  0x4EF4,  0x3E13,  0x2E32,  0x1E51,  0x0E70,
    0xFF9F,  0xEFBE,  0xDFDD,  0xCFFC,  0xBF1B,  0xAF3A,  0x9F59,  0x8F78,
    0x9188,  0x81A9,  0xB1CA,  0xA1EB,  0xD10C,  0xC12D,  0xF14E,  0xE16F,
    0x1080,  0x00A1,  0x30C2,  0x20E3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83B9,  0x9398,  0xA3FB,  0xB3DA,  0xC33D,  0xD31C,  0xE37F,  0xF35E,
    0x02B1,  0x1290,  0x22F3,  0x32D2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xB5EA,  0xA5CB,  0x95A8,  0x8589,  0xF56E,  0xE54F,  0xD52C,  0xC50D,
    0x34E2,  0x24C3,  0x14A0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xA7DB,  0xB7FA,  0x8799,  0x97B8,  0xE75F,  0xF77E,  0xC71D,  0xD73C,
    0x26D3,  0x36F2,  0x0691,  0x16B0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xD94C,  0xC96D,  0xF90E,  0xE92F,  0x99C8,  0x89E9,  0xB98A,  0xA9AB,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18C0,  0x08E1,  0x3882,  0x28A3,
    0xCB7D,  0xDB5C,  0xEB3F,  0xFB1E,  0x8BF9,  0x9BD8,  0xABBB,  0xBB9A,
    0x4A75,  0x5A54,  0x6A37,  0x7A16,  0x0AF1,  0x1AD0,  0x2AB3,  0x3A92,
    0xFD2E,  0xED0F,  0xDD6C,  0xCD4D,  0xBDAA,  0xAD8B,  0x9DE8,  0x8DC9,
    0x7C26,  0x6C07,  0x5C64,  0x4C45,  0x3CA2,  0x2C83,  0x1CE0,  0x0CC1,
    0xEF1F,  0xFF3E,  0xCF5D,  0xDF7C,  0xAF9B,  0xBFBA,  0x8FD9,  0x9FF8,
    0x6E17,  0x7E36,  0x4E55,  0x5E74,  0x2E93,  0x3EB2,  0x0ED1,  0x1EF0
};



//for monitor
BYTE I2cSpiFlashChipRegCmd(BYTE cmd, BYTE cmd_buff_len, BYTE data_buff_len, BYTE dma_option, BYTE wait)
{
	BYTE i;
	volatile BYTE vdata;

#if 0
	if(cmd==SPICMD_PP 
	|| cmd==SPICMD_READ_DUAL_O
	|| cmd==SPICMD_READ_QUAD_O
	|| cmd==SPICMD_READ_DUAL_IO
	|| cmd==SPICMD_READ_QUAD_IO)
		return SpiFlashXMemCmd(cmd,cmd_buff_len, data_buff_len, dma_option, wait);
#endif
	
	WriteI2C_8A(0xFF,0x04);
	WriteI2C_8A(DMAREG_4F3,(DMA_DEST_CHIPREG << 6) | (1+cmd_buff_len)); //cmd+cmd_buff_len
	WriteI2C_8A(DMAREG_4F6, DMA_BUFF_REG_ADDR_PAGE);
	WriteI2C_8A(DMAREG_4F7, DMA_BUFF_REG_ADDR_INDEX);

	WriteI2C_8A(DMAREG_4F5, 0x00 );			// 0xDA or 0xF5 data Buff count high
	WriteI2C_8A(DMAREG_4F8, 0x00 );			// data Buff count middle
	WriteI2C_8A(DMAREG_4F9, data_buff_len);	// data Buff count Lo

	WriteI2C_8A(DMAREG_4FA, cmd );							// cmd
	if(cmd_buff_len)
		WriteI2CS_8A(DMAREG_4FB,cmd_buff_len,SPI_CmdBuffer);

	// DMA-start
	WriteI2C_8A(0xFF,0x04);
	WriteI2C_8A(DMAREG_4F4, 0x01 | dma_option);	
    if(wait) {
		//REG4F4[0] is a self clear flag register.
        for(i=0; i < wait; i++) {
			vdata = ReadI2C_8A(DMAREG_4F4);
			if((vdata & 0x01)==0)
				break;
			delay1ms(10);
        }
        if(i==wait) {
            ePrintf("\n\rDMA Busy.");
            //fail
            return 0xFF;
        }
    }
	if(cmd==SPICMD_EX4B) I2CSPI_4B_mode = 0;
	if(cmd==SPICMD_EN4B) I2CSPI_4B_mode = 1;

	//read
	for(i=0; i < data_buff_len; i++) 
		SPI_CmdBuffer[i] = ReadI2C_8A(0xD0+i);		

	return 0;	//success
}

//for monitor
void I2cSpiFlashSetAddress2CmdBuffer(DWORD spiaddr)
{
	if(I2CSPI_4B_mode) {
		SPI_CmdBuffer[0] = spiaddr >> 24;
		SPI_CmdBuffer[1] = spiaddr >> 16;
		SPI_CmdBuffer[2] = spiaddr >> 8;
		SPI_CmdBuffer[3] = spiaddr;
	}
	else 
	{
		SPI_CmdBuffer[0] = spiaddr >> 16;
		SPI_CmdBuffer[1] = spiaddr >> 8;
		SPI_CmdBuffer[2] = spiaddr;
	}
}


void I2CSPI_SW_reset(void)
{
	BYTE bTemp;

    WriteI2C_8A(0xFF,0x04);
	bTemp = ReadI2C_8A(REG4E1);
	if((bTemp & 0x30))
		WriteI2C_8A(REG4E1, bTemp & ~0x30);
    WriteI2C_8A(0xFF,0x00);
	WriteI2C_8A(REG006, ReadTW88(REG006) | 0x80);

	if(bTemp & 0x30) {
        WriteI2C_8A(0xFF,0x04);
		WriteI2C_8A(REG4E0, bTemp);
    }
}

void I2CSPI_LV_reset(void)
{
 	WriteI2C_8A(0xFF,0x00);
    WriteI2C_8A(REG0D4,0x01);
    WriteI2C_8A(REG0D4,0x00);
}


BYTE I2CSPI_mcu_halt_rerun(BYTE fHalt)
{
	BYTE i;
    volatile BYTE bTemp;

	WriteI2C_8A(0xFF,0x04);
	WriteI2C_8A(0xED,0x55);
	WriteI2C_8A(0xED,0xAA);
	if(fHalt==0) {
		/* rerun MCU */ 
		WriteI2C_8A(0xEC,0x01);
		return 0;
	}
	/* halt MCU */
	WriteI2C_8A(0xEC,0x00);
	
	/* wait max 500ms*/
	WriteI2C_8A(0xFF,0x04);		 
	for(i=0; i < 100; i++) {
		bTemp = ReadI2C_8A(0xC4);
		if((bTemp & 0x80)==0)
			return 0;
		delay1ms(5);
	}
	return 1; /* fail */
}

/* Note: Do not enable xmem access on SPIDMA */
BYTE I2CSPI_enable_xmem_access(void)
{
	volatile BYTE bTemp;
	BYTE i;

	/*enable XMEM access */
	WriteI2C_8A(0xFF,0x04);
	bTemp = ReadI2C_8A(0xC2);
	WriteI2C_8A(0xC2, bTemp | 0x01);
	//wait. 100*10ms
	for(i=0; i < 100; i++) {
		bTemp = ReadI2C_8A(0xC2);
		if(bTemp & 0x02)
			break;
		delay1ms(10);
	}
	if(i==100)
		return 1;
	return 0;
}
BYTE I2CSPI_disable_xmem_access(void)
{
	volatile BYTE bTemp;
	BYTE i;

	WriteI2C_8A(0xFF,0x04);
	bTemp = ReadI2C_8A(0xC2);
	WriteI2C_8A(0xC2, bTemp & ~0x01);
	//wait..100*10ms
	for(i=0; i < 100; i++) {
		bTemp = ReadI2C_8A(0xC2);
		if((bTemp & 0x02)==0)
			break;
		delay1ms(10);
	}
	if(i==100)
		return 1;
	return 0;
}

/*!
* Check REG4F4[0] register.
* It is DMA start command and it a self clear flag.
* If it is cleared, it means the DMA command is accepted.
* If we using erase / pp commands,
* we have to read the status register from SpiFlash to check the BUSY bit.
* If we using REG4F4[2], with erase / pp commands,
*  TW8836B will clear REG4F4[0] when it receives the erase / pp commands
*  and, keep checks the status register with REG4D8 and REG4D9.
* If you did not wait, and try to use the other DMA command,
*  this new DMA command will be blocked until the BUSY bit is cleared.
*
* method1:
*	do erase/pp without REG4F4[2].
*   read status register with SPICMD_RDSR until the BUSY bit is cleared.
*
* method2:
*	do erase/pp with REG4F4[2], and REG4D8,REG4D9.
*	executes SPICMD_RDSR. or SPICMD_WRDI
*	In this case, we do not need to compare the BUSY bit.
*   We only need to check REG4F4[0] with a enough big wait value.
*   This method looks strange but it is more faster then method1.
*
* Sector Erase(SE) needs 100~400ms
* Block Erase(BE) needs 120~1600mS
*
* If SE, use (,50,10,0)	to wait 500mS
* If BE, use (,200,10,0) to wait 2000mS
*
*
* If Read, I read
*   20:47 CE:E2 04:00.
*   04:80 with RDID. If "i2cspi mcu halt", it becomes 04:00
*/
BYTE I2CSPI_IsCmdAccepted(BYTE option, BYTE loop, BYTE delay, BYTE debug)
{
	BYTE i;
	volatile BYTE vdata;
	volatile BYTE vpage;

    if(debug) {
		Puts("\n");

    	WriteI2C_8A(0xFF,0x04);	
    	for(i=0; i < loop; i++) {
		    vpage = ReadI2C_8A(0xFF);
            vdata = ReadI2C_8A(DMAREG_4F4);
            Printf("%02bx:%02bx ",vpage,vdata);
            if(vpage==0x04) {
        		if((vdata & 0x7F) == option) {
                    Printf("CMD:%02bX i:%bd delay:%bd", 
                        ReadI2C_8A(DMAREG_4FA),i,delay);
    			    return i;
                }
            }
    		if(delay)
    			delay1ms((WORD)delay);
    	}
        Printf(" CMD:%02bX fail loop%bd delay:%bd\n", 
            ReadI2C_8A(DMAREG_4FA),i,delay);
    }
    else {
    	WriteI2C_8A(0xFF,0x04);	
    	for(i=0; i < loop; i++) {
    		vdata = ReadI2C_8A(0xFF);
            if(vdata==0x04) {
                vdata = ReadI2C_8A(DMAREG_4F4);
        		if((vdata & 0x7F) == option)
        			return i;
            }
    		if(delay)
    			delay1ms((WORD)delay);
	    }
        Printf(" CMD:%02bX fail loop%bd delay:%bd\n", 
            ReadI2C_8A(DMAREG_4FA),i,delay);
    }
	return loop;	//fail:busy
}

/* SPICMD_RDID:0x9F */
BYTE I2CSPI_ReadId_to_chipreg(void)
{
    BYTE bRet;

	WriteI2C_8A(0xFF,0x04);
	WriteI2C_8A(DMAREG_4F3,(DMA_DEST_CHIPREG << 6) | 1); 
	WriteI2C_8A(DMAREG_4F6, DMA_BUFF_REG_ADDR_PAGE);
	WriteI2C_8A(DMAREG_4F7, DMA_BUFF_REG_ADDR_INDEX);
	WriteI2C_8A(DMAREG_4F5, 0x00 );			// data Buff count high
	WriteI2C_8A(DMAREG_4F8, 0x00 );			// data Buff count middle
	WriteI2C_8A(DMAREG_4F9, 3);	            // data Buff count Lo
	WriteI2C_8A(DMAREG_4FA, SPICMD_RDID );	// cmd
	WriteI2C_8A(DMAREG_4F4, 0x01 | SPI_CMD_OPT_NONE);	
	bRet=I2CSPI_IsCmdAccepted(SPI_CMD_OPT_NONE, 10,0, OFF);
	if(bRet==10) {
		Puts("\nRDID fail");
		return 1;
	}
    //result is in REG4D0,REG4D1,REG4D2.
	WriteI2C_8A(0xFF,0x04);
    I2CSPI_mid = ReadI2C_8A(REG4D0);
    I2CSPI_size = ReadI2C_8A(REG4D2);

	return 0;
}

/* SPICMD_WREN:0x06 */
BYTE I2CSPI_WriteEnable(void)
{
	BYTE bRet;

	WriteI2C_8A(0xFF,0x04);
    bRet = ReadI2C_8A(DMAREG_4F4);
    if(bRet & 0x01) {
        Puts("\n oops..you need lv_reset");
    }
	WriteI2C_8A(DMAREG_4F3, (DMA_DEST_CHIPREG << 6) | 0x01); //cmd len:1
	WriteI2C_8A(DMAREG_4F5, 0);	//length high
	WriteI2C_8A(DMAREG_4F8,	0); //length middle
	WriteI2C_8A(DMAREG_4F9,	0); //length low
	WriteI2C_8A(DMAREG_4FA, SPICMD_WREN);
	WriteI2C_8A(DMAREG_4F4, 0x01 | SPI_CMD_OPT_NONE);	
	bRet=I2CSPI_IsCmdAccepted(SPI_CMD_OPT_NONE, 5,0, OFF);
	if(bRet==5) {
		Puts("\nWREN fail");
		return 1;
	}
	return 0;
}
/*
SPICMD_EX4B:0xE9
SPICMD_EN4B:0xB7
*/
BYTE I2cSpiFlash_4B_DmaCmd(BYTE cmd)
{
    BYTE ret;

    if(I2CSPI_size < 0x19) {
        I2CSPI_4B_mode = 0;
        return 0;
    }

	if     (cmd==SPICMD_EN4B) 	I2CSPI_4B_mode = 1;
	else if(cmd==SPICMD_EX4B) 	I2CSPI_4B_mode = 0;
	else
		return 1; /* fail */

    if(I2CSPI_mid==SPIFLASH_MID_MICRON) {
        ret=I2CSPI_WriteEnable();
        if(ret)
            return 1;
    }

	WriteI2C_8A(0xFF,0x04);
	WriteI2C_8A(DMAREG_4F3, (DMA_DEST_CHIPREG << 6) +1);
	WriteI2C_8A(DMAREG_4F5, 0);	//length high
	WriteI2C_8A(DMAREG_4F8,	0); //length middle
	WriteI2C_8A(DMAREG_4F9,	0); //length low
	WriteI2C_8A(DMAREG_4FA, cmd);
	WriteI2C_8A(DMAREG_4F4, 0x01 | SPI_CMD_OPT_NONE);
	ret=I2CSPI_IsCmdAccepted(SPI_CMD_OPT_NONE, 5,0, OFF);
	if(ret==5) {
		Printf("CMD:%02bx fail",cmd);
		return 1;		 
	}
    return 0;
}

/* called within SE,BE, or PP */
/* return	0:success, 1:Fail
	the status register value will be placed at REG4D0
*/
/* SPICMD_RDSR:0x05 */
BYTE I2CSPI_wait_busy_with_rdsr(BYTE old_cmd, BYTE loop, BYTE base, BYTE debug)
{
	BYTE bRet;

	WriteI2C_8A(0xFF, 4);
	WriteI2C_8A(DMAREG_4F3, (DMA_DEST_CHIPREG << 6) | 0x01);	//Cmd len:1
	WriteI2C_8A(DMAREG_4F5,	0x00);	//length high
	WriteI2C_8A(DMAREG_4F6,	0x04);	//pBuff high
	WriteI2C_8A(DMAREG_4F7,	0xD0);	//pBuff low
	WriteI2C_8A(DMAREG_4F8,	0x00);	//length middle
	WriteI2C_8A(DMAREG_4F9,	0x01);  //length low
	WriteI2C_8A(DMAREG_4FA, SPICMD_RDSR);
	WriteI2C_8A(DMAREG_4F4, 0x01 | SPI_CMD_OPT_NONE);

	bRet=I2CSPI_IsCmdAccepted(SPI_CMD_OPT_NONE, loop, base, debug);
	if(bRet==loop) {
        Printf("\nwait_busy fail. old_cmd:%02bx loop:%d delay:%bd", 
            old_cmd, (WORD)loop, base);
		return 1;
	}
	return 0;
}

/* SPICMD_SE:0x20 */
BYTE I2CSPI_SectorErase(DWORD spi_addr)
{
	BYTE ret;

	dPrintf("SE %07lx",spi_addr);
	ret=I2CSPI_WriteEnable();
    if(ret)
        return 1;

	WriteI2C_8A(0xFF,0x04);
	WriteI2C_8A(DMAREG_4F3, (DMA_DEST_CHIPREG << 6) +4 +I2CSPI_4B_mode);	//Cmd Len:1+3
	WriteI2C_8A(DMAREG_4F5, 0);	//length high
	WriteI2C_8A(DMAREG_4F8,	0); //length middle
	WriteI2C_8A(DMAREG_4F9,	0); //length low
	WriteI2C_8A(DMAREG_4FA, SPICMD_SE);
    if(I2CSPI_4B_mode) {
    	WriteI2C_8A(DMAREG_4FB, (BYTE)(spi_addr>>24));
    	WriteI2C_8A(DMAREG_4FC, (BYTE)(spi_addr>>16));
    	WriteI2C_8A(DMAREG_4FD, (BYTE)(spi_addr>>8));
    	WriteI2C_8A(DMAREG_4FE, (BYTE)spi_addr);
    }
    else {
    	WriteI2C_8A(DMAREG_4FB, (BYTE)(spi_addr>>16));
    	WriteI2C_8A(DMAREG_4FC, (BYTE)(spi_addr>>8));
    	WriteI2C_8A(DMAREG_4FD, (BYTE)spi_addr);
    }
	WriteI2C_8A(DMAREG_4F4, 0x01 | SPI_CMD_OPT_WRITE_BUSY);
	ret=I2CSPI_IsCmdAccepted(SPI_CMD_OPT_WRITE_BUSY, 5,0, OFF);
	if(ret==5) {
		Puts("\nSE fail");
		return 1;
	}
	/* SE with BUSY needs a command to check the done action */
	/* BKFYI:it use 200ms on Micron */
	ret=I2CSPI_wait_busy_with_rdsr(SPICMD_SE,100,10, OFF); /*250~800msec*/
	if(ret) {
		Puts("\nSE wait fail");
		return 1;
	}
	return 0;
}

/* SPICMD_BE:0xD8 */
BYTE I2CSPI_BlockErase(DWORD spi_addr)
{
	BYTE ret;

	dPrintf("BE %07lx",spi_addr);
	ret=I2CSPI_WriteEnable();
    if(ret)
        return 1;

	WriteI2C_8A(0xFF,0x04);
	WriteI2C_8A(DMAREG_4F3, (DMA_DEST_CHIPREG << 6) +4 +I2CSPI_4B_mode);	//Cmd Len:1+3
	WriteI2C_8A(DMAREG_4F5, 0);	//length high
	WriteI2C_8A(DMAREG_4F8,	0); //length middle
	WriteI2C_8A(DMAREG_4F9,	0); //length low
	WriteI2C_8A(DMAREG_4FA, SPICMD_BE);
    if(I2CSPI_4B_mode) {
    	WriteI2C_8A(DMAREG_4FB, (BYTE)(spi_addr>>24));
    	WriteI2C_8A(DMAREG_4FC, (BYTE)(spi_addr>>16));
    	WriteI2C_8A(DMAREG_4FD, (BYTE)(spi_addr>>8));
    	WriteI2C_8A(DMAREG_4FE, (BYTE)spi_addr);
    }
    else {
    	WriteI2C_8A(DMAREG_4FB, (BYTE)(spi_addr>>16));
    	WriteI2C_8A(DMAREG_4FC, (BYTE)(spi_addr>>8));
    	WriteI2C_8A(DMAREG_4FD, (BYTE)spi_addr);
    }
	WriteI2C_8A(DMAREG_4F4, 0x01 | SPI_CMD_OPT_WRITE_BUSY);
    //BKFYI: it will return 0 on normal. If I2C line is not stable, it will return error.
	ret=I2CSPI_IsCmdAccepted( SPI_CMD_OPT_WRITE_BUSY, 5,0, OFF);
	if(ret==5) {
		Puts("\nBE fail");
		return 1;
	}
	/* BE with BUSY needs a command to check the done action */
	/* BKFYI:it use 620ms on Micron. 49@10mSec */
	ret=I2CSPI_wait_busy_with_rdsr(SPICMD_BE,150,20, OFF); /*750~3000msec*/
	if(ret) {
		Puts("\nBE wait fail");
		return 1;
	}
	return 0;
}


/* send data from Master's XMEM to Slave's XMEM */
BYTE I2CSPI_download_master_to_xmem(DWORD spiaddr, WORD length)
{
	BYTE ret;
	WORD i;
	BYTE *p = &SPI_Buffer[0];
	BYTE bTemp;


	if(length > 128) {
		Puts("\n!!I only have 128...you are killing me!!");
	}
    /* Read data from Master's SpiFlash to Master's XMEM.  */
    /*
    You have to replace this code on the SOC environment.
    */
	SpiFlash_Read_XMem(spiaddr, (WORD)SPI_Buffer, length);
	dPrintf("\n0x%06lx 0x%x bytes use %bdx10ms",spiaddr, length, ret);

	/* Enable XMEM access */
	ret=I2CSPI_enable_xmem_access();
	if(ret) {
		Puts("Fail En XMEM");
		return 1;
	}

    /* send data from Master's XMEM to Slave's XMEM */
    /*
    You have to replace this code on the SOC environment.
    */
	WriteI2C_8A(0xFF,0x04);
	WriteI2C_8A(0xDB,0x00); /* XMEM index high */
	WriteI2C_8A(0xDC,0x00); /* XMEM index low */
	for(i=0; i <length; i++) {
		bTemp = *p++;
		WriteI2C_8A(0xDD, bTemp);
        /* update g_CRC */
		g_CRC = (g_CRC << 8) ^ crctab[(g_CRC >>8) ^ bTemp];
	}

	/* Disable XMEM access */
	I2CSPI_disable_xmem_access();

	return 0;
}

/* Page is 256 byte */
/* SPICMD_PP:0x02 */
BYTE I2CSPI_PageProgram_from_xmem(DWORD spiaddr, WORD length)
{
	BYTE ret;

	ret=I2CSPI_WriteEnable();
    if(ret)
        return 1;

	//PP(PageProgram)
	WriteI2C_8A(0xFF,0x04);
	WriteI2C_8A(DMAREG_4F3, (DMA_DEST_MCU_XMEM << 6) +4 +I2CSPI_4B_mode);	//XMEM,Increase,len:1+3
	WriteI2C_8A(DMAREG_4F6, 0x00);	//XMEM start 0
	WriteI2C_8A(DMAREG_4F7, 0x00);
	WriteI2C_8A(DMAREG_4F5, 0x00); /* max pageprogram is 256 bytes */
	WriteI2C_8A(DMAREG_4F8,	(BYTE)(length>>8));
	WriteI2C_8A(DMAREG_4F9,	(BYTE)length);
	WriteI2C_8A(DMAREG_4FA, SPICMD_PP);
    if(I2CSPI_4B_mode) {
    	WriteI2C_8A(DMAREG_4FB, (BYTE)(spiaddr>>24));
    	WriteI2C_8A(DMAREG_4FC, (BYTE)(spiaddr>>16));
    	WriteI2C_8A(DMAREG_4FD, (BYTE)(spiaddr>>8));
    	WriteI2C_8A(DMAREG_4FE, (BYTE)spiaddr);
    }
    else {
    	WriteI2C_8A(DMAREG_4FB, (BYTE)(spiaddr>>16));
    	WriteI2C_8A(DMAREG_4FC, (BYTE)(spiaddr>>8));
    	WriteI2C_8A(DMAREG_4FD, (BYTE)spiaddr);
    }
	WriteI2C_8A(DMAREG_4F4, 0x01 | SPI_CMD_OPT_WRITE_BUSY);
	ret=I2CSPI_IsCmdAccepted(SPI_CMD_OPT_WRITE_BUSY, 5,0, OFF);
	if(ret==5) {
		Puts("\nPP fail ");
		return 1;
	}

	ret=I2CSPI_wait_busy_with_rdsr(SPICMD_PP,10,2, OFF); /*0.5~5msec*/
	if(ret) {
		Puts("\nPP wait fail");
		return 1;
	}
	return 0;
}

/* max length is BLOCKSIZE(64K=0x10000). loop should be BYTE size */
/* SPICMD_READ_SLOW:0x03 */ 
BYTE I2CSPI_check_crc(DWORD spiaddr, DWORD length, 	BYTE loop, WORD crc)
{
	WORD readed;
	BYTE ret;

	WriteI2C_8A(0xFF,0x04);
	WriteI2C_8A(DMAREG_4F3, (DMA_DEST_CHIPREG << 6) + 0x20+ 4 + I2CSPI_4B_mode);	//Reg,Fix,len:1+3
	WriteI2C_8A(DMAREG_4F6, 0x04);	//Reg Buffer
	WriteI2C_8A(DMAREG_4F7, 0xD0);
	WriteI2C_8A(DMAREG_4F5, (BYTE)(length>>16));
	WriteI2C_8A(DMAREG_4F8,	(BYTE)(length>>8));
	WriteI2C_8A(DMAREG_4F9,	(BYTE)length);
	WriteI2C_8A(DMAREG_4FA, SPICMD_READ_SLOW);
    if(I2CSPI_4B_mode) {
    	WriteI2C_8A(DMAREG_4FB, (BYTE)(spiaddr>>24));
    	WriteI2C_8A(DMAREG_4FC, (BYTE)(spiaddr>>16));
    	WriteI2C_8A(DMAREG_4FD, (BYTE)(spiaddr>>8));
    	WriteI2C_8A(DMAREG_4FE, (BYTE)spiaddr);
    }
    else {
    	WriteI2C_8A(DMAREG_4FB, (BYTE)(spiaddr>>16));
    	WriteI2C_8A(DMAREG_4FC, (BYTE)(spiaddr>>8));
    	WriteI2C_8A(DMAREG_4FD, (BYTE)spiaddr);
    }
	WriteI2C_8A(DMAREG_4F4, 0x01 | SPI_CMD_OPT_NONE);

	ret=I2CSPI_IsCmdAccepted(SPI_CMD_OPT_NONE, loop,10, ON);
	if(ret==loop) {
		Printf("check_crc fail. length:0x%lx loop:%d base:10", length, loop);
		return 1;		 
	}

	WriteI2C_8A(0xFF,0x04);
	readed =  ReadI2C_8A(0xEE);
	readed <<= 8;
	readed |=  ReadI2C_8A(0xEF);
	if(crc != readed) {
		Printf("\nCRC fail. %04x->%04x ", crc, readed);
		return 2;
	}
	return 0;
}

/* Download Block size (Max 64Kbytes) data */
BYTE I2CSPI_download_Block(uint32_t src_addr, uint32_t dest_addr, uint32_t upload_len)
{
	WORD local_crc;
	DWORD remain;
	WORD data_size;
	DWORD src_spi_addr, dest_spi_addr;
    BYTE ret;
	WORD read_crc;

    Printf("\ndownload_Block(%lx,%lx,%lx)",
        src_addr,dest_addr,upload_len);

    if((dest_addr & 0xFFFF))
        return 1; /* Not Block Boundary: 64KByte */

    if(upload_len > FLASH_BLOCK_SIZE)
        return 2;

    /* BE */
    Printf("\nBE 0x%lx", dest_addr);
    ret=I2CSPI_BlockErase(dest_addr);
    if(ret)
        return 3;   /* fail Erase */

    Printf("\nPP:");
    local_crc = 0;
	remain = upload_len;
    src_spi_addr = src_addr;
    dest_spi_addr = dest_addr;
    while(remain) {    
        /* progress */
        if((dest_spi_addr & 0x03FF) ==0)
            Puts(".");

        /* read */
		if(remain >= XMEM_BUFF_SIZE) data_size = XMEM_BUFF_SIZE;
		else 						 data_size = remain;
		remain -= data_size;
        g_CRC = local_crc;
		ret=I2CSPI_download_master_to_xmem(src_spi_addr, data_size);
		if(ret) {
			Printf("FAIL:master2xmem %07lx size:%x", src_spi_addr, data_size);
            return 4; /* fail read */
		}
		local_crc = g_CRC;
        src_spi_addr +=data_size;
        
        /* write */
		ret=I2CSPI_PageProgram_from_xmem(dest_spi_addr, data_size);
		if(ret) {
			Printf("FAIL:xmem2spi %07lx size:%x", dest_spi_addr, data_size);
            return 5; /* fail write */
		}
		dest_spi_addr += data_size;
    }
    /*CRC */
    Printf("\nCRC start:%lx size:%lx [%04X]",dest_addr,upload_len,local_crc);       
    ret=I2CSPI_check_crc(dest_addr, upload_len, 100, local_crc); /* wait 100*10ms */
    if(ret) {
    	WriteI2C_8A(0xFF,0x04);
    	read_crc =  ReadI2C_8A(0xEE);
    	read_crc <<= 8;
    	read_crc |=  ReadI2C_8A(0xEF);

        Printf(" FAIL CRC[%04X]", read_crc);

         if(ret==1)
            return 6;

        /* if ret was 2 */
        //-----------------------
        Puts("\nRetry");
        //-----------------------
        ret=I2CSPI_check_crc(dest_addr, upload_len, 100, local_crc); /* wait 100*10ms */
        if(ret) {
        	WriteI2C_8A(0xFF,0x04);
        	read_crc =  ReadI2C_8A(0xEE);
        	read_crc <<= 8;
        	read_crc |=  ReadI2C_8A(0xEF);
    
            Printf(" FAIL CRC[%04X]", read_crc);
            return 7;
        }
    }
    return 0; /* success */
}

/* Download Sector size (Max 4Kbytes) data */
BYTE I2CSPI_download_Sector(uint32_t src_addr, uint32_t dest_addr, uint32_t upload_len)
{
	WORD local_crc;
	DWORD remain;
	WORD data_size;
	DWORD src_spi_addr, dest_spi_addr;
    BYTE ret;
	WORD read_crc;

    Printf("\ndownload_Sector(%lx,%lx,%lx)",
        src_addr,dest_addr,upload_len);

    if((dest_addr & 0x0FFF))
        return 1; /* Not Sector Boundary: 4KByte */

    if(upload_len > FLASH_SECTOR_SIZE)
        return 2;

    /* SE */
    Printf("\nSE 0x%lx", dest_addr);
    ret=I2CSPI_SectorErase(dest_addr);
    if(ret)
        return 3;   /* fail Erase */

    Printf("\nPP:");
    local_crc = 0;
	remain = upload_len;
    src_spi_addr = src_addr;
    dest_spi_addr = dest_addr;
    while(remain) {    
        /* read */
		if(remain >= XMEM_BUFF_SIZE) data_size = XMEM_BUFF_SIZE;
		else 						 data_size = remain;
		remain -= data_size;
        g_CRC = local_crc;
		ret=I2CSPI_download_master_to_xmem(src_spi_addr, data_size);
		if(ret) {
			Printf("FAIL:master2xmem %07lx size:%x", src_spi_addr, data_size);
            return 4; /* fail read */
		}
		local_crc = g_CRC;
        src_spi_addr +=data_size;
        
        /* write */
		ret=I2CSPI_PageProgram_from_xmem(dest_spi_addr, data_size);
		if(ret) {
			Printf("FAIL:xmem2spi %07lx size:%x", dest_spi_addr, data_size);
            return 5; /* fail write */
		}
		dest_spi_addr += data_size;

        /* progress */
        Puts(".");
    }
    /*CRC */
    Printf("\nCRC start:%lx size:%lx [%04X]",dest_addr,upload_len,local_crc);       
    ret=I2CSPI_check_crc(dest_addr, upload_len, 10, local_crc); /* wait 10*10ms */
    if(ret) {
    	WriteI2C_8A(0xFF,0x04);
    	read_crc =  ReadI2C_8A(0xEE);
    	read_crc <<= 8;
    	read_crc |=  ReadI2C_8A(0xEF);
        Printf(" FAIL CRC[%04X]", read_crc);
        
        if(ret==1)
            return 6;

        /* if ret was 2 */
        //-----------------------
        Puts("\nRetry");
        //-----------------------
        ret=I2CSPI_check_crc(dest_addr, upload_len, 10, local_crc); /* wait 10*10ms */
        if(ret) {
        	WriteI2C_8A(0xFF,0x04);
        	read_crc =  ReadI2C_8A(0xEE);
        	read_crc <<= 8;
        	read_crc |=  ReadI2C_8A(0xEF);
            Printf(" FAIL CRC[%04X]", read_crc);

            return 7;
        }
    }
    return 0; /* success */
}


BYTE I2CSPI_download_main(uint32_t src_addr, uint32_t dest_addr, uint32_t download_len)
{
	DWORD remain;
	DWORD data_len;
	DWORD src_spi_addr, dest_spi_addr;
    BYTE bTemp;
    BYTE ret;
    BYTE need_4b;

    BYTE old_I2C_delay_base  = I2C_delay_base;
    I2C_delay_base=0;

	/* check align */
	if(dest_addr & 0x0FFF) {
		Printf("dest_addr is not 4KB aligned");
        I2C_delay_base = old_I2C_delay_base;
		return 1;
	}

    /* check I2C connection */
    Puts("\nCheck TW8836 ID");
	WriteI2C_8A(0xFF,0x00);	
	bTemp = ReadI2C_8A(0x00);
	if(bTemp != 0x36) {
		Printf("=> No TW8836 ID:%02bx", bTemp);
		I2C_delay_base = old_I2C_delay_base;
        return 1;
	}

    /* check & disable SpiOSD */
	WriteI2C_8A(0xFF,0x04);	
	bTemp = ReadI2C_8A(0x00);
	if(bTemp & 0x04) {
		Printf("\nDisable SpiOSD");
        WriteI2C_8A(0x00, bTemp & ~0x04);
        delay1ms(18);
        //check
    	bTemp = ReadI2C_8A(0x00);
    	if(bTemp & 0x04) {
    		Printf("\nDisable SpiOSD");
            WriteI2C_8A(0x00, bTemp & ~0x04);
        	bTemp = ReadI2C_8A(0x00);
        	if(bTemp & 0x04) {
                Puts("=>Fail");
                return 1;
            }
        }
	}

    /* check & disable xmem access */
	WriteI2C_8A(0xFF,0x04);
	bTemp = ReadI2C_8A(0xC2);
	if(bTemp & 0x01) {
		Puts("\nDisable XMEM access");
		WriteI2C_8A(0xC2, 0x00);
	}

    /* check DMA */
	WriteI2C_8A(0xFF,0x04);
	bTemp = ReadI2C_8A(0xF4);
	if(bTemp & 0x01) {
		Puts("\nI2CSPI DMA was busy");
        /* you need HW reset */
        Puts("\nLV reset");
        I2CSPI_LV_reset();
        delay1ms(500);
    	WriteI2C_8A(0xFF,0x04);
    	bTemp = ReadI2C_8A(0xF4);
    	if(bTemp & 0x01) {
    		Puts("\nI2CSPI DMA still busy");
            return 1;
        }
	}
	WriteI2C_8A(0xFF,0x04);
	bTemp = ReadI2C_8A(0xC4);
	if(bTemp & 0x01) {
		Puts("\nSPI DMA was busy");
        /* you need HW reset */
        Puts("\nLV reset");
        I2CSPI_LV_reset();
        delay1ms(500);
    	WriteI2C_8A(0xFF,0x04);
    	bTemp = ReadI2C_8A(0xC4);
    	if(bTemp & 0x01) {
    		Puts("\nSPI DMA still busy");
            return 1;
        }
	}

    /* read SPIFLASH ID */
    ret = I2CSPI_ReadId_to_chipreg();
	if(ret) {
		Puts("\nJEDEC failed");
		I2C_delay_base = old_I2C_delay_base;
        return 1;
	}
    /* read ID */
	WriteI2C_8A(0xFF,0x04);
    I2CSPI_mid = ReadI2C_8A(REG4D0);
    I2CSPI_size = ReadI2C_8A(REG4D2);
	Printf("\nJEDEC %02bx %02bx %02bx ", I2CSPI_mid, ReadI2C_8A(REG4D1), I2CSPI_size);
    switch(I2CSPI_mid) {
    case 0:
    case 0xFF:
        /* something wrong. do not try */
        I2C_delay_base = old_I2C_delay_base;
        return 1;
    case SPIFLASH_MID_MX:
        /* MX 256 working */
        Puts("MX");
        break;
    case SPIFLASH_MID_EON:
        Puts("EON");
        break;
    case SPIFLASH_MID_WB:
        /* WB 128 working */
        Puts("WB");
        break;
    case SPIFLASH_MID_MICRON:
        /* 25Q256A is working*/
        Puts("MICRON");
        break;
    default:
        /* spansion, giga or ISSI */
        Puts("\nUnknown");
        I2CSPI_size = 0; /* make it as an unknow size */
        break;
    }
    /* is it need 4Byte address mode ? */
    need_4b=0;
	dest_spi_addr = dest_addr;
    dest_spi_addr += download_len;
    if(dest_spi_addr > 0x1000000) {
        if(I2CSPI_size <= 0x18) {
            Puts("\nYou need Bigger chip");
            I2C_delay_base = old_I2C_delay_base;
            return 1;
        }
        /* I need 4Byte Address mode */
        need_4b=1;
    }
        
	/* halt MCU */
	I2CSPI_mcu_halt_rerun(1);

    /* check MCU */
	WriteI2C_8A(0xFF,0x04);
	bTemp = ReadI2C_8A(0xC4);
	if(bTemp & 0x80) {
		Puts("\nMCU is running");
        I2C_delay_base = old_I2C_delay_base;
        return 1;
	}

    /* move to 27MHz */
	WriteI2C_8A(0xFF,0x04);
	bTemp = ReadI2C_8A(0xE1);
	if(bTemp & 0x30) {
		Puts("\n27MHz");
		WriteI2C_8A(0xE1, bTemp & ~0x30);
	}
    /* BKTODO160218 Is it need a stable clock ? */    

    /* check & change 4Byte addr mode */
    I2CSPI_4B_mode = 0;
    if(I2CSPI_size > 0x18) {
        if(need_4b) {
            Puts("\nEn4B");
            ret=I2cSpiFlash_4B_DmaCmd(SPICMD_EN4B);
            if(ret) {
                Puts("\nFail En4B");
                I2C_delay_base = old_I2C_delay_base;
                return 1;
            }   
        }
        else {
            Puts("\nEx4B");
            ret=I2cSpiFlash_4B_DmaCmd(SPICMD_EX4B);
            if(ret) {
                Puts("\nFail Ex4B");
                I2C_delay_base = old_I2C_delay_base;
                return 1;
            }   
        }
    }

    /* init address & size */
    src_spi_addr = src_addr;
	dest_spi_addr = dest_addr;
	remain = download_len;

    /* pre-sector */
	while(dest_spi_addr & 0x0000F000) {
		if(remain >= FLASH_SECTOR_SIZE) data_len=FLASH_SECTOR_SIZE;
        else                            data_len=remain;
        ret=I2CSPI_download_Sector(src_spi_addr, dest_spi_addr, data_len);
        if(ret) {
            Puts("\nretry");
            ret=I2CSPI_download_Sector(src_spi_addr, dest_spi_addr, data_len);
            if(ret) {
                I2C_delay_base = old_I2C_delay_base;
                return 1;
            }
        }
        remain -= data_len;
		src_spi_addr  += data_len;
		dest_spi_addr += data_len;
    }
    /* block */
	while(remain > (FLASH_BLOCK_SIZE - FLASH_SECTOR_SIZE)) {
		if(remain >= FLASH_BLOCK_SIZE) data_len=FLASH_BLOCK_SIZE;
        else                           data_len=remain;
        ret=I2CSPI_download_Block(src_spi_addr, dest_spi_addr, data_len);
        if(ret) {
            Puts("\nretry");
            ret=I2CSPI_download_Block(src_spi_addr, dest_spi_addr, data_len);
            if(ret) {
                I2C_delay_base = old_I2C_delay_base;
                return 1;
            }
        }
        remain -= data_len;
		src_spi_addr  += data_len;
		dest_spi_addr += data_len;
    }
    /* post-sector*/
	while(remain) {
		if(remain >= FLASH_SECTOR_SIZE) data_len=FLASH_SECTOR_SIZE;
        else                            data_len=remain;
        ret=I2CSPI_download_Sector(src_spi_addr, dest_spi_addr, data_len);
        if(ret) {
            Puts("\nretry");
            ret=I2CSPI_download_Sector(src_spi_addr, dest_spi_addr, data_len);
            if(ret) {
                I2C_delay_base = old_I2C_delay_base;
                return 1;
            }
        }
        remain -= data_len;
		src_spi_addr  += data_len;
		dest_spi_addr += data_len;
    }

	/* re-run MCU */
	I2CSPI_mcu_halt_rerun(0);

    I2C_delay_base = old_I2C_delay_base;
    return 0;
}


BYTE I2CSPI_xcopy_main(uint32_t src_addr, uint32_t download_len)
{
	DWORD remain;
	DWORD data_len;
    uint32_t dest_addr = src_addr;
	DWORD src_spi_addr, dest_spi_addr;
    BYTE bTemp;
    BYTE ret;
    BYTE need_4b;

    BYTE old_I2C_delay_base  = I2C_delay_base;
    I2C_delay_base=0;

	/* check align */
	if(dest_addr & 0x0FFF) {
		Printf("dest_addr is not 4KB aligned");
        I2C_delay_base = old_I2C_delay_base;
		return 1;
	}

    /* check I2C connection */
    Puts("\nCheck TW8836 ID");
	WriteI2C_8A(0xFF,0x00);	
	bTemp = ReadI2C_8A(0x00);
	if(bTemp != 0x36) {
		Printf("=> No TW8836 ID:%02bx", bTemp);
		I2C_delay_base = old_I2C_delay_base;
        return 1;
	}

    /* check & disable SpiOSD */
	WriteI2C_8A(0xFF,0x04);	
	bTemp = ReadI2C_8A(0x00);
	if(bTemp & 0x04) {
		Printf("\nDisable SpiOSD");
        WriteI2C_8A(0x00, bTemp & ~0x04);
        delay1ms(18);
        //check
    	bTemp = ReadI2C_8A(0x00);
    	if(bTemp & 0x04) {
    		Printf("\nDisable SpiOSD");
            WriteI2C_8A(0x00, bTemp & ~0x04);
        	bTemp = ReadI2C_8A(0x00);
        	if(bTemp & 0x04) {
                Puts("=>Fail");
                return 1;
            }
        }
	}

    /* check & disable xmem access */
	WriteI2C_8A(0xFF,0x04);
	bTemp = ReadI2C_8A(0xC2);
	if(bTemp & 0x01) {
		Puts("\nDisable XMEM access");
		WriteI2C_8A(0xC2, 0x00);
	}

    /* check DMA */
	WriteI2C_8A(0xFF,0x04);
	bTemp = ReadI2C_8A(0xF4);
	if(bTemp & 0x01) {
		Puts("\nI2CSPI DMA was busy");
        /* you need HW reset */
        Puts("\nLV reset");
        I2CSPI_LV_reset();
        delay1ms(500);
    	WriteI2C_8A(0xFF,0x04);
    	bTemp = ReadI2C_8A(0xF4);
    	if(bTemp & 0x01) {
    		Puts("\nI2CSPI DMA still busy");
            return 1;
        }
	}
	WriteI2C_8A(0xFF,0x04);
	bTemp = ReadI2C_8A(0xC4);
	if(bTemp & 0x01) {
		Puts("\nSPI DMA was busy");
        /* you need HW reset */
        Puts("\nLV reset");
        I2CSPI_LV_reset();
        delay1ms(500);
    	WriteI2C_8A(0xFF,0x04);
    	bTemp = ReadI2C_8A(0xC4);
    	if(bTemp & 0x01) {
    		Puts("\nSPI DMA still busy");
            return 1;
        }
	}

    /* read SPIFLASH ID */
    ret = I2CSPI_ReadId_to_chipreg();
	if(ret) {
		Puts("\nJEDEC failed");
		I2C_delay_base = old_I2C_delay_base;
        return 1;
	}
    /* read ID */
	WriteI2C_8A(0xFF,0x04);
    I2CSPI_mid = ReadI2C_8A(REG4D0);
    I2CSPI_size = ReadI2C_8A(REG4D2);
	Printf("\nJEDEC %02bx %02bx %02bx ", I2CSPI_mid, ReadI2C_8A(REG4D1), I2CSPI_size);
    switch(I2CSPI_mid) {
    case 0:
    case 0xFF:
        /* something wrong. do not try */
        I2C_delay_base = old_I2C_delay_base;
        return 1;
    case SPIFLASH_MID_MX:
        /* MX 256 working */
        Puts("MX");
        break;
    case SPIFLASH_MID_EON:
        Puts("EON");
        break;
    case SPIFLASH_MID_WB:
        /* WB 128 working */
        Puts("WB");
        break;
    case SPIFLASH_MID_MICRON:
        /* 25Q256A is working*/
        Puts("MICRON");
        break;
    default:
        /* spansion, giga or ISSI */
        Puts("\nUnknown");
        I2CSPI_size = 0; /* make it as an unknow size */
        break;
    }
    /* is it need 4Byte address mode ? */
    need_4b=0;
	dest_spi_addr = dest_addr;
    dest_spi_addr += download_len;
    if(dest_spi_addr > 0x1000000) {
        if(I2CSPI_size <= 0x18) {
            Puts("\nYou need Bigger chip");
            I2C_delay_base = old_I2C_delay_base;
            return 1;
        }
        /* I need 4Byte Address mode */
        need_4b=1;
    }
        
	/* halt MCU */
	I2CSPI_mcu_halt_rerun(1);

    /* check MCU */
	WriteI2C_8A(0xFF,0x04);
	bTemp = ReadI2C_8A(0xC4);
	if(bTemp & 0x80) {
		Puts("\nMCU is running");
        I2C_delay_base = old_I2C_delay_base;
        return 1;
	}

    /* move to 27MHz */
	WriteI2C_8A(0xFF,0x04);
	bTemp = ReadI2C_8A(0xE1);
	if(bTemp & 0x30) {
		Puts("\n27MHz");
		WriteI2C_8A(0xE1, bTemp & ~0x30);
	}
    /* BKTODO160218 Is it need a stable clock ? */    
    
    /* check & change 4Byte addr mode */
    I2CSPI_4B_mode = 0;
    if(I2CSPI_size > 0x18) {
        if(need_4b) {
            Puts("\nEn4B");
            ret=I2cSpiFlash_4B_DmaCmd(SPICMD_EN4B);
            if(ret) {
                Puts("\nFail En4B");
                I2C_delay_base = old_I2C_delay_base;
                return 1;
            }   
        }
        else {
            Puts("\nEx4B");
            ret=I2cSpiFlash_4B_DmaCmd(SPICMD_EX4B);
            if(ret) {
                Puts("\nFail Ex4B");
                I2C_delay_base = old_I2C_delay_base;
                return 1;
            }   
        }
    }

    /* init address & size */
    src_spi_addr = src_addr;
	dest_spi_addr = dest_addr;
	remain = download_len;

    /* pre-sector */
	while(dest_spi_addr & 0x0000F000) {
		if(remain >= FLASH_SECTOR_SIZE) data_len=FLASH_SECTOR_SIZE;
        else                            data_len=remain;
        ret=I2CSPI_download_Sector(src_spi_addr, dest_spi_addr, data_len);
        if(ret) {
            Puts("\nretry");
            ret=I2CSPI_download_Sector(src_spi_addr, dest_spi_addr, data_len);
            if(ret) {
                I2C_delay_base = old_I2C_delay_base;
                return 1;
            }
        }
        remain -= data_len;
		src_spi_addr  += data_len;
		dest_spi_addr += data_len;
    }
    /* block */
	while(remain > (FLASH_BLOCK_SIZE - FLASH_SECTOR_SIZE)) {
		if(remain >= FLASH_BLOCK_SIZE) data_len=FLASH_BLOCK_SIZE;
        else                           data_len=remain;
        ret=I2CSPI_download_Block(src_spi_addr, dest_spi_addr, data_len);
        if(ret) {
            Puts("\nretry");
            ret=I2CSPI_download_Block(src_spi_addr, dest_spi_addr, data_len);
            if(ret) {
                I2C_delay_base = old_I2C_delay_base;
                return 1;
            }
        }
        remain -= data_len;
		src_spi_addr  += data_len;
		dest_spi_addr += data_len;
    }
    /* post-sector*/
	while(remain) {
		if(remain >= FLASH_SECTOR_SIZE) data_len=FLASH_SECTOR_SIZE;
        else                            data_len=remain;
        ret=I2CSPI_download_Sector(src_spi_addr, dest_spi_addr, data_len);
        if(ret) {
            Puts("\nretry");
            ret=I2CSPI_download_Sector(src_spi_addr, dest_spi_addr, data_len);
            if(ret) {
                I2C_delay_base = old_I2C_delay_base;
                return 1;
            }
        }
        remain -= data_len;
		src_spi_addr  += data_len;
		dest_spi_addr += data_len;
    }

	/* re-run MCU */
	I2CSPI_mcu_halt_rerun(0);

    I2C_delay_base = old_I2C_delay_base;
    return 0;
}


