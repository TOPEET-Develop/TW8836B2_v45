/**
 * @file
 * SPI.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	a device driver for the spi-bus interface 
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
#include "SPI.h"
#include "Settings.h"
#include "SOsd.h"


/**
* SpiFlash command routine that using a ChipRegister 0x4D0.
*
* use SPI_CmdBuffer[] for Write & Read.
*
*	SpiFlashChipRegCmd(SPICMD_RDID,0,3)
*	SpiFlashChipRegCmd(SPICMD_WREN,0,0)
*
* need REG4C1[0] before you use.
* SPICMD_READ_QUAD_IO can read only 1 byte on the chip buffer. See TW8835 Errata.
* SPICMD_PP on SFLASH_VENDOR_MICRON_512, must use XMEM, not DMA_BUFF_REG. See TW8836 Errata. 
*/
/*
options
	REG4C3[7:6]	destination(source on write mode)
	REG4C3[5:4] index mode
	REG4C1[0:0] start mode
	REG4C4[2:2}	enable busy check
	REG4C4[1:1] dma(cmd) read/write mode
	REG4C5[7:4]	DMA read wait cycle
	REG4C5[3:0] SPI read/write wait cycle
others
	SPI read mode		REG4C0[2:0]
	DMA(cmd) start		REG4C4[0:0]
	cmd buff			REG4CA[]~REG4CE[]
	cmd count			REG4C3[3:0]
	buff index			REG4C6[7:0]REG4C7[7:0]
	data count			REG4DA[7:0]REG4C8[7:0]REG4C9[7:0]	
	busy check cmd		REG4D8[7:0]
	busy check bit		REG4D9[2:0]
	busy check pol		REG4D9[3:3]
	chip data buff		REG4D0[]~REG4D7[]


if cmd is FAST DUAL_O/DUAL_IO/QUAD_O/QUAD_IO READ, use XMDM.
chip buff can read only 1 byte only
the dummy cycle is fixed on TW8836.

extern
	SPI_Buffer[128]
	SPI_CmdBuffer[8]


wait done flag

external registers
	
*/


//----------------- SPI Mode Definition ---------------------
/*!
 * SPI_Buffer[] is located at 0 on XMem.
 * It can make some room for I2CSPI routines.
 * SPI_CmdBuffer[] can be 4bytes. You can reduce it.
 */
XDATA BYTE SPI_Buffer[SPI_BUFFER_SIZE] _at_ 0;
XDATA BYTE SPI_CmdBuffer[8] _at_ SPI_BUFFER_SIZE;

/*!
 * SPI Read Command & Bytes.
 * On TW8836B, we are using REG4C3[3:0]; cmd length to make a dummy.
 *
 * mode    			command  DummyCycle	cmd_length
 * 0:Slow(single)	0x03	(no dummy)	4
 * 1:Fast(1-1-1)	0x0B	8			5
 * 2:Dual(1-1-2)	0x3B    8			5
 * 3:Quad(1-1-4)	0x6B	8			5
 * 4:DualIO(1-2-2)	0xBB	4			5
 * 5:QuadIO(1-4-4)	0xEB	6 			7
 * 6:D-Edge			0xED				12
 */
BYTE SPICMD_x_READ;     /*=0x03 slow */
BYTE SPICMD_x_BYTES;    /*=4 without 4B */

/*!
 * Spi 4byte address mode for 256Mbit or bigger SpiFlash chip.
 * If FW executes SPICMD_EN4B, SpiFlash4ByteAddr will be '1'.
 * If SpiFlash4ByteAddr is '1', FW will add '1' when it uses SPICMD_x_BYTES.
 * Use SpiFlash_4B_DmaCmd(), SpiFlashSetAddress2CmdBuffer().
 */
BYTE SpiFlash4ByteAddr; /* =0 */

/* Wait for BUSY. (unit mS)
							MX25L25635F		N25Q256A
cmd							typ	Max 		typ	Max 	
SPICMD_WRSR	Write Status		40			1.3 8		
SPICMD_SE	SE(4K)			43	200			250	800		
SPICMD_BE	BE(64K)			340	2000		700	3000	
SPICMD_PP	PageProgram		0.6	3			0.15 5		
*/

/*
	TIME
	clock frequency for all command 	108MHz
	                    READ command	54MHz			 50MHz
	S# deselect time after a READ command	20:: ns
	WRITE STATUS register cycle time	:1.3:8 ms
	WRITE Volatile configuration 		:40: ns
	PageProgram(256)					:0.5:5 ms		=> 10mS
	SectorErase							:0.25:0.8s		=> 1000ms
	BlockErase							:0.7:3s			=> 2500ms

Write in progress bit					micron           macronix   
	WRITE STATUS REGISTER				x:1.3:8   ms					this is WRSR, not WREN
	WRITE NONVOLATILE CONFIG REGISTER	x:200:3000 ms
	PROGRAM								x:0.5:5   ms	  x:0.33:1.2
	ERASE(SE)							x:250:800 ms	  x:25:200
	ERASE(BE)							x:700:3000 ms     x:250:1000
These commands need a BUSY check.	
	
PP and Read those are using buff_len, need a wait_done() function.										
*/

#define SPIFLASH_WAIT_W_STATUS	4
#define SPIFLASH_WAIT_SE		20
#define SPIFLASH_WAIT_BE		250
#define SPIFLASH_WAIT_PP		20
#define SPIFLASH_WAIT_100MS		10
#define SPIFLASH_WAIT_200MS		20


/* Micron 151218
	(Good)		(Bad)
	25Q256A		25Q256A
	13E40		13E40  		65nm -40~85C STD(not Auto)
	(e4) PHL	(e4) CHN
	782505773	6053513LE2
						   	marketing part#:N25Q256A13ESF40F or G 
	Status Register
		[1] write enable
		[0] write in progress
	read: 0x05			SPICMD_RDSR
	write: 0x01			SPICMD_WRSR
	write enable:0x06	SPICMD_WREN


	NonVolatile Config Register (Low Byte is read first)
		[15:12] dummy			6=QuadIO, 8=QuadO
		[4]		Reset/hold		0=Disable
	read: 0xB5			SPICMD_RDNVREG
	write:0xB1			SPICMD_WDNVREG

	Volatile Config Register
		[7:4] dummy				6=QuadIO, 8=QuadO
	read: 0x85			SPICMD_RDVREG
	write: 0x81			SPICMD_WDVREG

	Enhanced Volatile Config Register
		[4] Reset/Hold			0=Disable
	read: 0x65 			SPICMD_RDVEREG
	write: 0x61			SPICMD_WDVEREG

	Flag Status Register
		[7] Program		0:Busy,1=Ready
		[6] Erase suspend	1=in effect
		[0] address		0=3byte, 1=4byte
	read: 0x70 			SPICMD_RDFREG
	clear(?): 0x50		SPICMD_CLRFREG
*/


//-----------------------------------------------
//internal prototype
//-----------------------------------------------


/*!
* wait REG4C4[0] done status.
*	This flag is cleared when HW executes SPI command.
*
* 	For WRSR,SE,BE,PP, it does not means the command is finished.
*	You have to assign SPI_CMD_OPT_BUSY(or _WRITE_BUSY), and executes SpiFlash_check_busy().
*	For more detail info, see SpiFlash_check_busy() function. 
*
*	For the read family commands, it is cleared when HW finishes the DMA.
*	On the internal MCU, you do not need to wait, because, the code fetch routine is blocked.
*   So, you do not need a SpiFlash_wait_done() on the internal MCU.
*   But, on the external CPU that using I2C, CPU have to wait and check this flag.
*
*	When TW8836B HW executes the DMA, it also blocks the I2C host.
*	so, the external CPU also can use ReadI2C(0xFF); Page register.
*
* @param wait - count
* @param unit - 1ms delay unit
* @return 0:success 1:fail
*/
BYTE SpiFlash_wait_done(BYTE wait, BYTE unit)
{
	BYTE i;
	volatile BYTE vdata;

	for (i=0; i<wait; i++)
	{
		vdata = ReadTW88(REG4C4);
		if ((vdata & 0x01) == 0)
			break;
		delay1ms((WORD)unit);
	}
	
	if (i == wait)
	{
		Printf("DmaFail cmd:%02bx loop:%bd unit:%bd\n", ReadTW88(REG4DA), wait, unit);

		return 1;
	}
	
	return 0;
}

/*!
* execute DMA command
*
* REG4DA is for pBuff length high bytes.
* this function has 16bit size parameter,
*  and do not control REG4DA register.
* If you need a 3Bytes size, assign REG4DA, execute it, and then clear REG4DA.
*  see, SpiFlash_FastRead_Fixed_ChipReg().
*
* @param cmd
* @param target_cmd_len 
*	[7:6} target, 
*	[5:4] access mode, 
*	[3:0] SPI_CmdBuffer[] length 
* @param pBuff
* @param buff_len
* @param option	
*	[2] busy check  
*	[1]:write. related with pBuff.
*
* if you are using "busy check", execute
*	SpiFlash_wait_done();
*   SpiFlash_check_busy();
*
* if buff_len is 0, we don't need a pBuff.
*	for example: SpiFlash_DmaCmd(cmd, DMA_TARGET_CHIP, 0, 0....	
*
* if ExtI2c, use REG4C1[0]=1, At Vertical Blank. 
*/
void SpiFlash_DmaCmd(BYTE cmd, BYTE target_cmd_len, WORD pBuff, WORD buff_len, BYTE option)
{
	BYTE i;
	BYTE cmd_buff_len;
	BYTE r4e1;

	cmd_buff_len = target_cmd_len & 0x0F;
	if(cmd_buff_len > 4) 	//for FastRead dummy control.
		cmd_buff_len = 4;

	WriteTW88(REG4C3, target_cmd_len +1);		// target & cmd_len that includes cmd.
	WriteTW88(REG4C6, (BYTE)(pBuff>>8));		// pBuff high
	WriteTW88(REG4C7, (BYTE)pBuff);				// pBuff low
	WriteTW88(REG4C8, (BYTE)(buff_len>>8));		// data Buff count middle
	WriteTW88(REG4C9, (BYTE)buff_len);			// data Buff count Lo
	//--WriteTW88(REG4DA, 0);					// data Buff count high
	WriteTW88(REG4CA, cmd);
	for(i=0; i < cmd_buff_len; i++)				// cmd buffer.
		WriteTW88(REG4CB+i, SPI_CmdBuffer[i]);	// normally address.
	
	if(spiflash_chip->pllclk_dma) {
		//for SHSL issue on Micron. only related with an internal MCU.
		r4e1 = ReadTW88(REG4E1);
		if((r4e1 & 0x30) == 0x20) {
			WriteTW88(REG4E1, (r4e1 & 0xF0) | spiflash_chip->pllclk_dma);
			WriteTW88(REG4C4, 0x01 | option);	
			WriteTW88(REG4E1, r4e1);
		}
		else
			WriteTW88(REG4C4, 0x01 | option);
	}
	else
		WriteTW88(REG4C4, 0x01 | option);
	return;
}

/*!
* BUSY check
*
* If you executes DMA_OPT_BUSY (or DMA_OPT_BUSY_WRITE), 
* TW8836B will clear REG4C4[0], and keep checks the BUSY status register
* with REG4D8 and REG4D9 registers that normally has SPICMD_RDSR.
* It means, the command is executed, but, it is on the processing state, and it does not finished.
* If you execute the other SPIFlash command without check the status register flag on SpiFlash,
* this new command should be buffered and blocked until the status register becomes the done state.
*
* we have 2 method.
* Method 1:
*	Do not use REG4C4[2];BUSY, and read the status register from SpiFlash unitl the WIP(WriteInProgress) bit is cleared.
*   It is a logically correct, and easy to understand, but...
*   This method have to use a several SpiFlash Commands and have to use a compare routine.
* Method 2:
*	Use REG4C4[2];BUSY, and execute any SpiFlash command, and then check REG4C4[0] flag.
*	TW8836B HW will executes SPICMD_RDSR and compare WIP.
*	It uses only one SpiFlash command, and several REG4C4 read routine.
*	It is good for the internal MCU.
*	
*/
BYTE SpiFlash_check_busy(BYTE wait, BYTE unit)
{
	BYTE i;
	BYTE ret;

	for(i=0; i < wait; i++) {
		SpiFlash_DmaCmd(SPICMD_RDSR, DMA_TARGET_CHIP, 0x04D0, (WORD)0, 0);
		ret=SpiFlash_wait_done(10,10);
		if(ret==0) 
			break;
		delay1ms((WORD)unit);
	}
	if(i==wait) {
		Printf("BUSY fail loop:%bd unit:%bd\n", wait, unit);
		return 1; /* failed */
	}
	return 0; /* success */
}

/*!
* WriteEnable
*/
BYTE SpiFlash_WriteEnable(void)
{
	BYTE ret;
	SpiFlash_DmaCmd(SPICMD_WREN, DMA_TARGET_CHIP, 0, 0, DMA_OPT_NONE);
	ret=SpiFlash_wait_done(2,5);
	return ret;
}

/**
* If you use SPICMD_EN4B, TW8836B will use 4Bytes address.
*/
BYTE SpiFlash_4B_DmaCmd(BYTE cmd)
{
	BYTE ret;

	if
		(cmd == SPICMD_EN4B)
		SpiFlash4ByteAddr = 1;
	else if (cmd == SPICMD_EX4B)
		SpiFlash4ByteAddr = 0;
	else
		return 1; /* fail */

	if (is_micron_256or512())
		SpiFlash_WriteEnable();

	SpiFlash_DmaCmd(cmd, DMA_TARGET_CHIP, 0, 0, DMA_OPT_NONE);
	ret = SpiFlash_wait_done(10, 10);
	
	return ret;
}

/**
*	set 4Bytes address mode to support more than 128Mbit.
*	call after SpiFlash_SetReadModeByRegister()
*
* #define SPICMD_EN4B				0xB7	//enter 4Byte mode
* #define SPICMD_EX4B				0xE9	//exit 4Byte mode
*
* SpiFlash4ByteAddr value will be used to check the 4Byte mode or not.
*/
void SpiFlash_Set4BytesAddress(BYTE fOn)
{	
	if (fOn)
	{
		if (SpiFlash4ByteAddr == 0)
			SpiFlash_4B_DmaCmd(SPICMD_EN4B);
	}
	else 
	{ 
		if (SpiFlash4ByteAddr)
			SpiFlash_4B_DmaCmd(SPICMD_EX4B);
	}
}

//-----------------------------------------------------------------------------
/**
* assign a SpiFlash address
*
* To support 4BYTE address mode,
* use extern: SpiFlash4ByteAddr.
*/
#if 0
BYTE SpiFlashSetAddress2Hw(DWORD addr)
{
	if(SpiFlash4ByteAddr) {
		WriteTW88(REG4CB, (BYTE)(addr >> 24));
		WriteTW88(REG4CC, (BYTE)(addr >> 16));
		WriteTW88(REG4CD, (BYTE)(addr >> 8));
		WriteTW88(REG4CE, (BYTE)(addr));
		return 4;
	}
	else {
		WriteTW88(REG4CB, (BYTE)(addr >> 16));
		WriteTW88(REG4CC, (BYTE)(addr >> 8));
		WriteTW88(REG4CD, (BYTE)(addr));
		return 3;
	}
}
#endif
BYTE SpiFlashSetAddress2CmdBuffer(DWORD spiaddr)
{
	if(SpiFlash4ByteAddr) {
		SPI_CmdBuffer[0] = spiaddr >> 24;
		SPI_CmdBuffer[1] = spiaddr >> 16;
		SPI_CmdBuffer[2] = spiaddr >> 8;
		SPI_CmdBuffer[3] = spiaddr;
		return 4;
	}
	else {
		SPI_CmdBuffer[0] = spiaddr >> 16;
		SPI_CmdBuffer[1] = spiaddr >> 8;
		SPI_CmdBuffer[2] = spiaddr;
		return 3;
	}
}

/*!
* SectorErase. 4KB
* SE need a BUSY check.
*/
BYTE SpiFlash_SectorErase(DWORD spiaddr)
{
	BYTE cmd_buff_len;
	BYTE ret;

	SpiFlash_WriteEnable();

	cmd_buff_len=SpiFlashSetAddress2CmdBuffer(spiaddr);

	SpiFlash_DmaCmd(SPICMD_SE, DMA_TARGET_CHIP | cmd_buff_len, 0x04D0, 0, SPI_CMD_OPT_BUSY);
	SpiFlash_wait_done(2, 5);
	ret=SpiFlash_check_busy(10,100);	/*20*50=1000ms*/
	return ret;
}

/*!
* BlockErase. 64KB
* BE need a BUSY check.
*/
BYTE SpiFlash_BlockErase(DWORD spiaddr)
{
	BYTE cmd_buff_len;
	BYTE ret;

	SpiFlash_WriteEnable();

	cmd_buff_len=SpiFlashSetAddress2CmdBuffer(spiaddr);

	SpiFlash_DmaCmd(SPICMD_BE, DMA_TARGET_CHIP | cmd_buff_len, 0x04D0, 0, SPI_CMD_OPT_BUSY);
	SpiFlash_wait_done(2, 5);
	ret=SpiFlash_check_busy(30,100); /*30*100=3000ms*/
	return ret;
}

/*! 
* PageProgram with ChipReg
* PP need a BUSY check.
*
* @param len - max 8.
*/
BYTE SpiFlash_PageProgram_ChipReg(DWORD spiaddr, BYTE *pBuff, BYTE len)
{
	BYTE cmd_buff_len;
	BYTE i;
	BYTE ret;

	if(len>8)
		return 1; //fail

	SpiFlash_WriteEnable();

	cmd_buff_len=SpiFlashSetAddress2CmdBuffer(spiaddr);
	for(i=0; i < len; i++)
		WriteTW88(REG4D0+i, *pBuff++);

	SpiFlash_DmaCmd(SPICMD_PP, DMA_TARGET_CHIP | cmd_buff_len, 0x04D0, len, SPI_CMD_OPT_WRITE_BUSY);
	SpiFlash_wait_done(2, 5);
	ret=SpiFlash_check_busy(5,50); /*5*50=250ms*/
	return ret;	
}

/*! 
* PageProgram with XMem
* PP need a BUSY check.
* PP need a REG4C4[1].
*/
BYTE SpiFlash_PageProgram_XMem(DWORD spiaddr, WORD xaddr, WORD xlen)
{
	BYTE cmd_buff_len;
	BYTE ret;

	SpiFlash_WriteEnable();
	cmd_buff_len=SpiFlashSetAddress2CmdBuffer(spiaddr);
	SpiFlash_DmaCmd(SPICMD_PP, DMA_TARGET_XMEM | cmd_buff_len, xaddr, xlen, SPI_CMD_OPT_WRITE_BUSY);
	SpiFlash_wait_done(2, 5);

	ret=SpiFlash_check_busy(5,50); /*5*50=250ms*/
	return ret;	
}


/*! FastRead with ChipReg.
*  The Read with ChipReg does not support multi data output lines Read Commands (FastQuad,..).
 * It can be SlowRead, but SlowRead has a maximum speed limitation about 50MHz.
 *
 *  Read need a wait.
 *  but, 100ms delay on SpiFlash_DmaCmd should be enough.
 *  if you using it for CRC, use a SpiFlash_SlowRead_Fixed_ChipReg(). 
*/
BYTE SpiFlash_FastRead_ChipReg(DWORD spiaddr, BYTE len) 
{
	BYTE cmd_buff_len;
	BYTE ret;

	if(len>8)
		len=8;

	cmd_buff_len=SpiFlashSetAddress2CmdBuffer(spiaddr);
	cmd_buff_len +=1; //for FastRead dummy
	SpiFlash_DmaCmd(SPICMD_READ_FAST, DMA_TARGET_CHIP | cmd_buff_len, 0x04D0, len, SPI_CMD_OPT_NONE);
	ret=SpiFlash_wait_done(2, 5);
	return ret;
}
/*!
* we using this command to generate CRC.
*   len can be big number, so we have to adjust wait.
*
* BKTODO - try with QuadO(or QuadIO) and then check CRC value.
*/
BYTE SpiFlash_FastRead_Fixed_ChipReg(DWORD spiaddr, DWORD dLen)
{
	BYTE cmd_buff_len;
	BYTE wait, unit;
	BYTE ret;

	if(dLen & 0xFF000000) {
		//TW8836B use 24bits register. REG4DA,REG4C8,REG4C9.
		Puts("sorry!! too big\n");
		return 1;
	}
	else if(dLen & 0x00FF0000) {
		wait = dLen >> 16;
		unit = 20; /* @27MHz */
	}
	else {
		wait = dLen >> 8;
		unit = 5;
	}
	if(wait==0)
		wait=1;

	cmd_buff_len=SpiFlashSetAddress2CmdBuffer(spiaddr);
	cmd_buff_len +=1; //for FastRead dummy
	WriteTW88(REG4DA, dLen >> 16); /* data Buff count high */
	SpiFlash_DmaCmd(SPICMD_READ_FAST, 
		DMA_TARGET_CHIP | DMA_ACCESS_FIX | cmd_buff_len, 
		0x04D0, (WORD)dLen, SPI_CMD_OPT_NONE);	/* dLen use 16bit */
	ret=SpiFlash_wait_done(wait, unit);
	WriteTW88(REG4DA, 0);	/* clean the data Buff count high */
	return ret;
}

/*!
* Read to XMEM (Max 2048,0x800).
* Internal MCU using 128Bytes.
*  Read need a wait.
*/
BYTE SpiFlash_Read_XMem(DWORD spiaddr, WORD xaddr, WORD xlen) 
{
	BYTE ret;
	BYTE cmd_buff_len;

	cmd_buff_len = SpiFlashSetAddress2CmdBuffer(spiaddr);	/* it includes SpiFlash4ByteAddr */
	cmd_buff_len += SPICMD_x_BYTES;							/* it includes the dummy cycle */
	cmd_buff_len -= 4;
	SpiFlash_DmaCmd(SPICMD_x_READ, DMA_TARGET_XMEM | cmd_buff_len, xaddr,xlen, SPI_CMD_OPT_NONE);
	ret = SpiFlash_wait_done(10,10);
	return ret;
}
//void SpiFlashDmaRead2XMem(BYTE * dest_loc, DWORD src_loc, WORD size, BYTE wait)
//{
//	BYTE bTemp = wait;
//	WORD dest_w_loc = (WORD)dest_loc;
//
//	SpiFlash_Read_XMem(src_loc, dest_w_loc, size);
//}

/*!
* @param spiaddr
* @param addr - target LUT index location.
*	LUTTYPE_ADDR
*		addr is (wTemp >> 8) and (wTemp & 0xFF); 
*	LUTTYPE_BYTE (Column first)
*		addr is (wTemp>>6) and (wTemp << 2)		   
*		background has 256 LUTs	(0x000~0x0FF)
*		foreground has 512 LUTS (0x000~0x1FF)          
*		If we do r-shift twice (<< 2), LUTTYPE_BYTE is becomes LUTTYPE_ADDR.
* @param len
*/
BYTE SpiFlash_Read_SOsd(DWORD spiaddr, WORD addr, WORD len)
{
	BYTE cmd_buff_len;
	BYTE ret;

	cmd_buff_len = SpiFlashSetAddress2CmdBuffer(spiaddr);	/* it includes SpiFlash4ByteAddr */
	cmd_buff_len += SPICMD_x_BYTES;							/* it includes the dummy cycle */
	cmd_buff_len -= 4;
	SpiFlash_DmaCmd(SPICMD_x_READ, DMA_TARGET_SOSD + cmd_buff_len, addr,len, SPI_CMD_OPT_NONE);
	ret = SpiFlash_wait_done(10,10);
	return ret;
}

/*!
* @param spiaddr
* @param addr - target FontRam index location.
* @param len
*/
BYTE SpiFlash_Read_FOsd(DWORD spiaddr, WORD addr, WORD len) 
{
	BYTE cmd_buff_len;
	BYTE ret;

	cmd_buff_len = SpiFlashSetAddress2CmdBuffer(spiaddr); /* it includes SpiFlash4ByteAddr */
	cmd_buff_len += SPICMD_x_BYTES;						  /* it includes the dummy cycle */
	cmd_buff_len -= 4;
	SpiFlash_DmaCmd(SPICMD_x_READ, DMA_TARGET_FOSD | cmd_buff_len, addr,len, SPI_CMD_OPT_NONE);
	ret = SpiFlash_wait_done(10,10);
	return ret;
}


/*
 SPIFLASH chip & speed.

On TW8836B, the SpiFlash is used for the MCU code memory and the SpiOSD data.
These two modules fetch the data when it turned on.

Below is TW8836B spec. 
					Recommand Spec.		Internal target.	
	SSPLL			150MHz				(more than 150MHz)
	SPICLK			120MHz				133Mhz
	MCUCLK on ASync	108MHz				110MHz
	MCUCLK on Sync	108MHz				(more than 110MHz)	
On the panel, SPICLK needs a below minimum value to display SpiOSD.
If you need remocon, I recommand to use 108MHz on 1024x600 panel.
					 1024x600	800x480 panel
	Minimum SPICLK 		94MHz	54MHz
And, we are usgin QuadIO or Quad mode with fixed dummy cycle. 
	Fast Read Mode		DummyCycle
	QuadIO(1-4-4 mode)	6
	QuadO(1-1-4 mode)	8
	Fast(1-1-1 mode)	8

When I test same SpiOSD image on Quad and on QuadIO,
Quad mode has about 6% overhead, but, Quad mode can be worked on higher speed.
we choose Quad mode, if it can support more speed.
	test micron MT25Q256 with 4Byte address

		Quad Output mode:	1-1-4 with dummy 8.
			minimum: 99.984MHz
			Spec:    115MHz
		
		Quad IO mode: 1-4-4 with dummy 6
			minimum: 94.133MHz
			Spec:	 86MHz

We are trying to follow up the recommand speed that is suggested by SpiFlash Vendor.
When I test the SpiFlash, "MX25L25635F" has a best performance.
"MX25L25635F" is working on 120MHz even its spec. is 104MHz on the Socket board.	

  Vendor	spec.	test result.
  Macromix	104MHz	Stable. 
  Micron	108Mhz	unstable
  Winbond	104MHz	Unstable
  GigaDev	120MHz	Unstable(even 108Mhz)

So, we are provide below code "AS-IS" for evaluation purpose only.
*/

							    //SSPLL,     Control
							//REG0F8~REG0FA	 REG0FD
#define SSPLL1_133M_REGS		0x02,0x76,0x85,0x34
#define SSPLL1_120M_REGS		0x02,0x38,0xE4,0x34
#define SSPLL1_115M_REGS		0x02,0x21,0x30,0x33
#define SSPLL1_108M_REGS		0x02,0x00,0x00,0x24
#define SSPLL1_105M_REGS		0x01,0xF1,0xC8,0x23
#define SSPLL1_104M_REGS		0x01,0xED,0x0A,0x23
#define SSPLL1_98M_REGS			0x01,0xD0,0x98,0x23
#define SSPLL1_96M_REGS			0x01,0xC7,0x1D,0x23
#define SSPLL1_95M_REGS			0x01,0xC2,0x5F,0x23
#define SSPLL1_94M_REGS			0x01,0xBD,0xA2,0x23
#define SSPLL1_90M_REGS			0x01,0xAA,0xAB,0x23
#define SSPLL1_86M_REGS			0x01,0x97,0xB5,0x23
#define SSPLL1_84M_REGS			0x01,0x8E,0x39,0x23
#define SSPLL1_80M_REGS			0x01,0x7B,0x43,0x23
#define SSPLL1_78M_REGS			0x01,0x71,0xC8,0x23
#define SSPLL1_72M_REGS			0x01,0x55,0x56,0x23
#define SSPLL1_70M_REGS			0x01,0x4B,0xDB,0x23
#define SSPLL1_54M_REGS			0x01,0x00,0x00,0x23
#define SSPLL1_50M_REGS			0x01,0xDA,0x13,0x63		
								//REG4E1[3:0]
#define PLL_CLK_DIV_1			0					//sameas PLLCLK_DIV_
#define PLL_CLK_DIV_1P5			1
#define PLL_CLK_DIV_2			2
								//REG4F0[7:4]
#define MCU_CLK_DIV_1			0					//rename as MCUCLK_DIV
#define MCU_CLK_DIV_1P5			1	//need wait 3
#define MCU_CLK_DIV_2			2	//need wait 5
#define MCU_CLK_DIV_3			4	//need wait 8
#define MCU_CLK_DIV_4			6	//need wait ?

/* wait value on ASYNC mode (when MCU_CLK_DIV is not 1) */
code BYTE async_wait_value[] = {
/*  1	1.5	2	2.5	3	3.5	4       divider  */
	0,	3, 	5,	7,	8,	9,	10  /*wait value */
};

/*
    SPIFLASH CHIP TABLE
    ===================
    Remove commnet if you want to use.
    But, do not use NA or NG column.
*/
code struct SPIFLASH_DIMEMSION spiflash_chip_table[] =
{
/* Macronix*/
//	{ 0xC2,0x20,0x16,	32,		SPI_READ_QUADIO,    104, SSPLL1_104M_REGS,  0,0,2,0,    "MX25L3233F"},	 /*6 dummy, SHSL:15*/
//
//	{ 0xC2,0x20,0x17,	64,		SPI_READ_QUADIO,    86, SSPLL1_86M_REGS,    0,0,2,0,    "MX25L6435E"},	 /*6 dummy, SHSL:15*/
//	{ 0xC2,0x20,0x17,	64,		SPI_READ_QUADIO,    104,SSPLL1_104M_REGS,   0,0,0,0,    "MX25L6435E"},	 /*NA:8 dummy*/
//
    { 0xC2,0x20,0x18,	128,    SPI_READ_QUAD,	    104,SSPLL1_104M_REGS,   0,0,0,0,    "MX25L12835F"},   
//  { 0xC2,0x20,0x18,	128,    SPI_READ_QUADIO,	84, SSPLL1_84M_REGS,    0,0,0,0,    "MX25L12835F"},   
//  { 0xC2,0x20,0x18,	128,    SPI_READ_QUADIO,	133,SSPLL1_133M_REGS,   0,0,0,0,    "MX25L12835F"},  /*NA:8 dummy */ 
//  { 0xC2,0x20,0x18,	128,    SPI_READ_QUADIO,	120,SSPLL1_120M_REGS,   0,0,0,0,    "MX25L12835F"},  /*overclock */ 
//  { 0xC2,0x20,0x18,	128,    SPI_READ_QUADIO,	70, SSPLL1_70M_REGS,    0,0,2,0,    "MX25L12835E"},  /* SHSL:15 */ 
//
//	{ 0xC2,0x20,0x19,	256,	SPI_READ_QUAD, 	    94, SSPLL1_94M_REGS,    0,0,0,0,	"MX25L25635F"},	 /*for SOCKET. Bad Quality*/
//	{ 0xC2,0x20,0x19,	256,	SPI_READ_QUAD, 	    104,SSPLL1_104M_REGS,   0,0,0,0,	"MX25L25635F"},	
//	{ 0xC2,0x20,0x19,	256,	SPI_READ_QUADIO,    84, SSPLL1_84_REGS,     0,0,0,0,	"MX25L25635F"},	 /*6 dummy, SHSL:7nS */
//	{ 0xC2,0x20,0x19,	256,	SPI_READ_QUADIO,    133,SSPLL1_133_REGS,    0,0,0,0,	"MX25L25635F"},	 /*NA:10 dummy */
	{ 0xC2,0x20,0x19,	256,	SPI_READ_QUADIO,    70, SSPLL1_70M_REGS,    0,0,2,0,	"MX25L25635E"},	 /* SHSL:15 */
//
//  { 0xC2,0x20,0x1A,	512,	SPI_READ_QUAD, 	    104,SSPLL1_104M_REGS,   0,0,0,0,	"MX66L51235F"},	    		
//  { 0xC2,0x20,0x1A,	512,	SPI_READ_QUADIO,    84, SSPLL1_84M_REGS,    0,0,0,0,	"MX66L51235F"}, /*6 dummy, SHSL:7nS */	    		
//  { 0xC2,0x20,0x1A,	512,	SPI_READ_QUADIO,    133,SSPLL1_133M_REGS,   0,0,0,0,	"MX66L51235F"},	/*NA:10 dummy */    		
//
// 	{ 0xC2,0x20,0x1B,	1024,	SPI_READ_QUAD, 	    108,SSPLL1_108M_REGS,   0,0,0,0,	"MX66L1G45G"},
// 	{ 0xC2,0x20,0x1B,	1024,	SPI_READ_QUADIO,    84, SSPLL1_84M_REGS,    0,0,0,0,	"MX66L1G45G"},  /*6 dummy, SHSL:7nS*/
// 	{ 0xC2,0x20,0x1B,	1024,	SPI_READ_QUADIO,    133,SSPLL1_133M_REGS,   0,0,0,0,	"MX66L1G45G"},  /*NA:10 dummy */
//
/*Eon #CS deselect time(SHSL):10nS*/
//	{ 0x1C,0x30,0x17,	64,		SPI_READ_QUADIO,    50, SSPLL1_50M_REGS,    0,0,0,0,	"EN25QH64"},		
//	{ 0x1C,0x30,0x17,	64,		SPI_READ_QUADIO,    80, SSPLL1_80M_REGS,    0,0,0,0,	"EN25QH64"},    /*NA:*/		
//
	{ 0x1C,0x30,0x18,	128,	SPI_READ_QUADIO,    50, SSPLL1_50M_REGS,    0,0,0,0,	"EN25QH128"},
//	{ 0x1C,0x30,0x18,	128,	SPI_READ_QUADIO,    104,SSPLL1_105M_REGS,   0,0,0,0,	"EN25QH128"},   /* NA: */
//
//	{ 0x1C,0x70,0x19,	256,	SPI_READ_QUADIO,    50, SSPLL1_50M_REGS,    0,0,0,0,	"EN25Q256A"},
//	{ 0x1C,0x70,0x19,	256,	SPI_READ_QUADIO,    104,SSPLL1_104M_REGS,   0,0,0,0,	"EN25Q256A"},   /* NA: */
//
/*Winbond*/
	{ 0xEF,0x40,0x18,	128,	SPI_READ_QUADIO,    104,SSPLL1_104M_REGS,   0,0,0,0,	"W25Q128FV"},
//	{ 0xEF,0x40,0x19,	256,	SPI_READ_QUADIO,    104,SSPLL1_104M_REGS,   0,0,0,0,	"W25Q256FV"},
//	{ 0xEF,0x40,0x1A,	512,	SPI_READ_QUADIO,    104,SSPLL1_104M_REGS,   0,0,0,0,	"W25M512JV"},
//
/*Micron & Nymonix  #CS deselect time(SHSL):20nS  */
/* N25Q & MT25Q series has same JEDEC ID. I can not distinglish */
#if 1 //OLD N25Q series
//	{ 0x20,0xBA,0x16,	32,		SPI_READ_QUAD,      108,SSPLL1_108M_REGS,   0,0,2,0,    "N25Q032A"}, /*spec*/    
//	{ 0x20,0xBA,0x16,	32,		SPI_READ_QUAD,      80, SSPLL1_80M_REGS,    0,0,2,0,    "N25Q032A"}, /*real */
//	{ 0x20,0xBA,0x16,	32,		SPI_READ_QUADIO,    80, SSPLL1_80M_REGS,    0,0,2,0,    "N25Q032A"}, /*6dummy:spec*/
//	{ 0x20,0xBA,0x16,	32,		SPI_READ_QUADIO,    78, SSPLL1_78M_REGS,    0,0,2,0,    "N25Q032A"}, /*6dummy:real*/
//	{ 0x20,0xBA,0x16,	32,		SPI_READ_QUADIO,    108,SSPLL1_108M_REGS,   0,0,2,0,    "N25Q032A"}, /*NA:10 dummy*/
//
//	{ 0x20,0xBA,0x17,	64,		SPI_READ_QUAD,	    108,SSPLL1_108M_REGS,   0,0,2,0,	"N25Q064A"},  /*spec*/
	{ 0x20,0xBA,0x17,	64,		SPI_READ_QUAD,	    104,SSPLL1_104M_REGS,   0,0,2,0,	"N25Q064A"}, /* real*/
//	{ 0x20,0xBA,0x17,	64,		SPI_READ_QUADIO,	78, SSPLL1_78M_REGS,    0,0,2,0,	"N25Q064A"}, /*6dummy*/
//	{ 0x20,0xBA,0x17,	64,		SPI_READ_QUADIO,	108,SSPLL1_108M_REGS,   0,0,2,0,	"N25Q064A"}, /*NA:10dummy*/
//
//	{ 0x20,0xBA,0x18,	128,	SPI_READ_QUAD,	    108,SSPLL1_108M_REGS,   0,0,2,0,	"N25Q128A"},  /*spec*/
	{ 0x20,0xBA,0x18,	128,	SPI_READ_QUAD,	    104,SSPLL1_104M_REGS,   0,0,2,0,	"N25Q128A"}, /* real*/
//	{ 0x20,0xBA,0x18,	128,	SPI_READ_QUADIO,	80, SSPLL1_80M_REGS,    0,0,2,0,	"N25Q128A"}, /*6dummy*/
//	{ 0x20,0xBA,0x18,	128,	SPI_READ_QUADIO,	108,SSPLL1_108M_REGS,   0,0,2,0,	"N25Q128A"}, /*NA:10dummy*/
//
//	{ 0x20,0xBA,0x19,	256,	SPI_READ_QUAD,	    108,SSPLL1_108M_REGS,   0,0,2,0,	"N25Q256A"},  /*spec*/
	{ 0x20,0xBA,0x19,	256,	SPI_READ_QUAD,	    84, SSPLL1_84M_REGS,    0,0,2,0,	"N25Q256A"}, /* real*/
//	{ 0x20,0xBA,0x19,	256,	SPI_READ_QUADIO,	78, SSPLL1_78M_REGS,    0,0,2,0,	"N25Q128A"}, /*6dummy*/
//	{ 0x20,0xBA,0x19,	256,	SPI_READ_QUADIO,	108,SSPLL1_108M_REGS,   0,0,2,0,	"N25Q256A"}, /*NA:10dummy*/
//
//	{ 0x20,0xBA,0x20,	512,	SPI_READ_QUAD,	    108,SSPLL1_108M_REGS,   0,0,2,0,	"N25Q512A"},  /*spec*/
	{ 0x20,0xBA,0x20,	512,	SPI_READ_QUAD,	    104,SSPLL1_104M_REGS,   0,0,2,0,	"N25Q512A"}, /* real*/
//	{ 0x20,0xBA,0x20,	512,	SPI_READ_QUADIO,	80, SSPLL1_80M_REGS,    0,0,2,0,	"N25Q512A"}, /*6dummy*/
//	{ 0x20,0xBA,0x20,	512,	SPI_READ_QUADIO,	108,SSPLL1_108M_REGS,   0,0,2,0,	"N25Q512A"}, /*NA:10dummy*/
//
//	{ 0x20,0xBA,0x21,	1024,	SPI_READ_QUAD,	    108,SSPLL1_108M_REGS,   0,0,2,0,	"N25Q00AA"},  /*spec*/
	{ 0x20,0xBA,0x21,	1024,	SPI_READ_QUAD,	    104,SSPLL1_104M_REGS,   0,0,2,0,	"N25Q00AA"}, /* real*/
//	{ 0x20,0xBA,0x21,	1024,	SPI_READ_QUADIO,	80, SSPLL1_80M_REGS,    0,0,2,0,	"N25Q00AA"}, /*6dummy*/
//	{ 0x20,0xBA,0x21,	1024,	SPI_READ_QUADIO,	108,SSPLL1_108M_REGS,   0,0,2,0,	"N25Q00AA"}, /*NA:10dummy*/
#else //New MT25Q series
//	{ 0x20,0xBA,0x17,	64,	    SPI_READ_QUAD,	    104,SSPLL1_104M_REGS,   0,0,2,0,	"MT25QL064"}, 
	{ 0x20,0xBA,0x18,	128,	SPI_READ_QUAD,	    104,SSPLL1_104M_REGS,   0,0,2,0,	"MT25QL128"}, 
	{ 0x20,0xBA,0x19,	256,	SPI_READ_QUAD,	    104,SSPLL1_104M_REGS,   0,0,2,0,	"MT25QL256"}, 
//	{ 0x20,0xBA,0x20,	512,	SPI_READ_QUAD,	    104,SSPLL1_104M_REGS,   0,0,2,0,	"MTN25QL512"},
//	{ 0x20,0xBA,0x21,	1024,	SPI_READ_QUAD,	    104,SSPLL1_104M_REGS,   0,0,2,0,	"MTN25QL01G"},
//	{ 0x20,0xBA,0x22,	2048,	SPI_READ_QUAD,	    108,SSPLL1_108M_REGS,   0,0,2,0,	"MT25QL02G"},
#endif
//
/*spansion #CS deselect time(SHSL):10nS*/
/* spansion does not have SPICMD_EN4B(B7h) command, do not try 256 or 512Mbit */
//	{ 0x01,0x02,0x17,	64,		SPI_READ_QUADIO,    80, SSPLL1_80M_REGS,    0,0,0,0,	"S25FL064P"},
//	{ 0x01,0x02,0x18,	128,	SPI_READ_QUADIO,    90, SSPLL1_90M_REGS,    0,0,0,0,	"S25FL128S"},
//	{ 0x01,0x02,0x19,	256,	SPI_READ_QUADIO,    90, SSPLL1_90M_REGS,    0,0,0,0,	"S25FL256S"},
//	{ 0x01,0x02,0x20,	512,	SPI_READ_QUADIO,    90, SSPLL1_90M_REGS,    0,0,0,0,	"S25FL512S"},
//	{ 0x01,0x02,0x1B,	1024,	SPI_READ_QUADIO,    90, SSPLL1_90M_REGS,    0,0,0,0,	"S70FL01GS"},
//
/*gigabyte #CS deselect time(SHSL):20nS*/
//	{ 0xC8,0x40,0x15,	16,		SPI_READ_QUADIO,    108,SSPLL1_108M_REGS,   0,0,2,0,	"GD25Q16B"},
//	{ 0xC8,0x40,0x16,	32,		SPI_READ_QUADIO,    108,SSPLL1_108M_REGS,   0,0,2,0,	"GD25Q32B"},
//	{ 0xC8,0x40,0x17,	64,		SPI_READ_QUADIO,    108,SSPLL1_108M_REGS,   0,0,2,0,	"GD25Q64C"},
	{ 0xC8,0x40,0x18,	128,	SPI_READ_QUADIO,    80, SSPLL1_80M_REGS,    0,0,2,0,	"GD25Q128C"},
	{ 0xC8,0x40,0x20,	512,	SPI_READ_QUADIO,    104,SSPLL1_104M_REGS,   0,0,2,0,	"GD25Q512MC"},
//
/*ISSI*/
/*
 tCEH CE# High Time 7nS
 tCS CE# Setup Time 6nS
 tCH CE# Hold Time 6nS
*/
	{ 0x9D,0x60,0x18,	128,	SPI_READ_QUADIO,    104,SSPLL1_104M_REGS,   0,0,0,0,	"IC25LP128"},
//  { 0x9D,0x60,0x18,	128,	SPI_READ_QUAD,      133,SSPLL1_104M_REGS,   0,0,0,0,	"IC25LP128"}, /*NG*/
//
	/* last item need this for unknown SpiFlash */
	{ 0x00,0x00,0x00,}
};
struct SPIFLASH_DIMEMSION *spiflash_chip;


//#if 0
//BYTE is_micron_512(void)
//{
//	if(spiflash_chip->mid==0x20 && spiflash_chip->did1==0x20)
//		return TRUE;
//	return FALSE;
//}
//#endif

BYTE is_micron_256or512(void)
{
	if(spiflash_chip->mid==0x20) {
		if(spiflash_chip->did1==0x19
		|| spiflash_chip->did1==0x20)
			return TRUE;
	}
	return FALSE;
}

//#if 0
//BYTE is_macronix_256(void)
//{
//	if(spiflash_chip->mid==0xC2 && spiflash_chip->did1==0x19)
//		return TRUE;
//	return FALSE;
//}
//#endif
//#if 0
//BYTE is_winbond_256(void)
//{
//	if(spiflash_chip->mid==0xEF && spiflash_chip->did1==0x19)
//		return TRUE;
//	return FALSE;
//}
//#endif

/*!
 * check_spiflash_status_register
 *
 * @return TRUE FALSE
 */
BYTE check_spiflash_status_register(BYTE cmd, BYTE rsize, BYTE mask0, BYTE check0, BYTE mask1, BYTE check1)
{
	BYTE bTemp, bTemp1;

	SpiFlash_DmaCmd(cmd, DMA_TARGET_CHIP, 0x4D0, (WORD)rsize, SPI_CMD_OPT_NONE);
	SpiFlash_wait_done(2, 5);

	/* read back status value from REG4D0 */
	bTemp = ReadTW88(REG4D0);

	/* mask and then check */
	if ((bTemp & mask0) != check0)
		return FALSE;

	if (rsize == 2)
	{
		bTemp1 = ReadTW88(REG4D1);
		if ((bTemp1 & mask1) != check1)
			return FALSE;
	}
	
	return TRUE;
}

/*!
* function pointer method on uVerion makes RECURSIVE FUNCTION CALL warning.
* DO NOT USE a function pointer method...
*
* @return TRUE FALSE
*/
BYTE quadio_check_all(void)
{
	BYTE ret;
	BYTE mid = spiflash_chip->mid;

	switch (mid)
	{
	case SPIFLASH_MID_EON:
	case SPIFLASH_MID_MX:
	case SPIFLASH_MID_ISSI:
		return check_spiflash_status_register(SPICMD_RDSR, 1, 0x40, 0x40, 0, 0);

	case SPIFLASH_MID_WB:
	case SPIFLASH_MID_SPANSION:
		return check_spiflash_status_register(SPICMD_RDSR2, 1, 0x02, 0x02, 0, 0);

	case SPIFLASH_MID_GIGA:
		if (spiflash_chip->did1 == 0x20)	//GD25Q512MC
			return check_spiflash_status_register(SPICMD_RDSR, 1, 0x40, 0x40, 0, 0);
		else
			return check_spiflash_status_register(SPICMD_RDSR2, 1, 0x02, 0x02, 0, 0);

	case SPIFLASH_MID_MICRON:
		/* check Non-volatile */
		if (spiflash_chip->fast_mode == 5) 	//QuadIO need 6 dummy. clear RESET/HOLD
			ret = check_spiflash_status_register(SPICMD_RDNVREG,2,0x10,0x00,0xF0,0x60);
		else								//QuadO need 8 dummy. clear RESET/HOLD
			ret = check_spiflash_status_register(SPICMD_RDNVREG,2,0x10,0x00,0xF0,0x80);
		// if is_micron_512, use SPICMD_RDFREG,0x06.
		//   (pol:low,bit:7.  try bit6)
		if (spiflash_chip->did1 == 0x20)
			/* read FlagStatus register. bit:6 Pol:Low */
			SpiFlashSetupBusyCheck(SPICMD_RDFREG, 0x06);
		return ret;
		
	default:
		return FALSE;
	}	
}

BYTE quadio_enable_all(void)
{
	BYTE mid = spiflash_chip->mid;
	BYTE ret;

	switch (mid)
	{
	case SPIFLASH_MID_EON:
	case SPIFLASH_MID_MX:
	case SPIFLASH_MID_ISSI:
		SpiFlash_WriteEnable();
		SPI_CmdBuffer[0] = 0x40; 
		SpiFlash_DmaCmd(SPICMD_WRSR, DMA_TARGET_CHIP | 0x01, 0, 0, SPI_CMD_OPT_BUSY);
		SpiFlash_wait_done(10, 10);
		ret = SpiFlash_check_busy(10, 10);
		break;

	case SPIFLASH_MID_WB:
		if (spiflash_chip->did1 == 0x18)
		{
			/* quadio_enable_winbond_64 */
			SpiFlash_WriteEnable();
			SPI_CmdBuffer[0]=0x00;
			SPI_CmdBuffer[1]=0x02;
			SpiFlash_DmaCmd(SPICMD_WRSR, DMA_TARGET_CHIP | 0x02, 0, 0, SPI_CMD_OPT_BUSY);
			SpiFlash_wait_done(10,10);
			ret=SpiFlash_check_busy(10,10);
		}
		else
		{
			SpiFlash_WriteEnable();
			SPI_CmdBuffer[0]=0x02;
			SpiFlash_DmaCmd(SPICMD_WRSR2, DMA_TARGET_CHIP | 0x01, 0, 0, SPI_CMD_OPT_BUSY);
			SpiFlash_wait_done(10,10);
			ret=SpiFlash_check_busy(10,10);
		}
	   	break;

	case SPIFLASH_MID_MICRON:
#if 0 //micron_NonVolatile:
		SpiFlash_WriteEnable();
		SPI_CmdBuffer[0]=0xEF;	//LSB. [4] is a disable Reset/Hold. DO not write 0xF7. It will be corruptted.
		SPI_CmdBuffer[1]=0x6F;	//MSB. [7:4] is dummy cycle
		SpiFlash_DmaCmd(SPICMD_WDNVREG, DMA_TARGET_CHIP | 0x02, 0, 0, SPI_CMD_OPT_BUSY);
		SpiFlash_wait_done(10,10);
		ret=SpiFlash_check_busy(10,10);
		Puts("\n\rNonVolatile set QuadIO and 6 dummy." );
		Puts("\n\r==========================================" );
		Puts("\n\r===!!!You have to reboot your system!!!===" );
		Puts("\n\r==========================================" );
 		break;
#else //micron_volatile
		SpiFlash_WriteEnable();
	
		if (spiflash_chip->fast_mode == 5)
		{
			/* for QuadIO 1-4-4 mode with 6 dummy cycle */	
			SPI_CmdBuffer[0] = 0x6B;
		}
		else
		{
			/* for Quad and Fast with 8 dummy cycle. do not try Dual. */
			SPI_CmdBuffer[0] = 0x8B;	
		}
		SpiFlash_DmaCmd(SPICMD_WDVREG, DMA_TARGET_CHIP | 0x01, 0, 0, SPI_CMD_OPT_BUSY);
		SpiFlash_wait_done(10, 10);
		ret=SpiFlash_check_busy(10, 10);
	
		SpiFlash_WriteEnable();
	
		//Micron has different register meaning depend on thier chip.
		//We used 0xCF, and tried 0xEF, and now, we are using 0xE7.
		SPI_CmdBuffer[0] = 0xE7;
		SpiFlash_DmaCmd(SPICMD_WDVEREG, DMA_TARGET_CHIP | 0x01, 0, 0, SPI_CMD_OPT_BUSY);
		SpiFlash_wait_done(10, 10);
		ret=SpiFlash_check_busy(10, 10);
		break;
#endif

	case SPIFLASH_MID_SPANSION:
		SpiFlash_WriteEnable();
		SpiFlash_DmaCmd(SPICMD_RDSR, DMA_TARGET_CHIP, 0x4D0, 1, DMA_OPT_NONE);
		SpiFlash_wait_done(10, 10);
		SPI_CmdBuffer[0] = ReadTW88(REG4D0);   /* Input Status Register-1 */

		SpiFlash_WriteEnable();
        //Configuration Register 1 (CR1)   P58
		SPI_CmdBuffer[1] = 0x02; /* Input Configuration Register */
		SpiFlash_DmaCmd(SPICMD_WRSR, DMA_TARGET_CHIP | 0x02, 0, 0, SPI_CMD_OPT_BUSY);
		SpiFlash_wait_done(10, 10);
		ret=SpiFlash_check_busy(10, 10);
		break;

	case SPIFLASH_MID_GIGA:
		if (spiflash_chip->did1==0x15 || spiflash_chip->did1==0x16)
		{	
			//giga_small
			BYTE status0, status1;

			SpiFlash_DmaCmd(SPICMD_RDSR, DMA_TARGET_CHIP, 0x4D0, 1, DMA_OPT_NONE);
			SpiFlash_wait_done(10, 10);
			status0 = ReadTW88(REG4D0);

			SpiFlash_DmaCmd(SPICMD_RDSR2, DMA_TARGET_CHIP, 0x4D0, 1, DMA_OPT_NONE);
			SpiFlash_wait_done(10, 10);
			status1 = ReadTW88(REG4D0);
		
			SpiFlash_WriteEnable();

			SPI_CmdBuffer[0] = status0;
			SPI_CmdBuffer[1] = status1 | 0x02;
			SpiFlash_DmaCmd(SPICMD_WRSR, DMA_TARGET_CHIP | 0x02, 0, 0, SPI_CMD_OPT_BUSY);
			SpiFlash_wait_done(10, 10);
			ret = SpiFlash_check_busy(10, 10);
			break;
		}
		else if (spiflash_chip->did1==0x17 || spiflash_chip->did1==0x18)	//GD25Q128C/GD25Q64C
		{
			BYTE status;

			SpiFlash_DmaCmd(SPICMD_RDSR2, DMA_TARGET_CHIP, 0x4D0, 1, DMA_OPT_NONE);
			SpiFlash_wait_done(10, 10);
			status = ReadTW88(REG4D0);

			SpiFlash_WriteEnable();

			SPI_CmdBuffer[0]= status | 0x02;
			SpiFlash_DmaCmd(SPICMD_WRSR2, DMA_TARGET_CHIP | 0x01, 0, 0, SPI_CMD_OPT_BUSY);
			SpiFlash_wait_done(10, 10);
			ret = SpiFlash_check_busy(10, 10);
			break;
		}
		else if (spiflash_chip->did1==0x19 || spiflash_chip->did1==0x20)	//GD25Q512MC/GD25Q256C
		{
			BYTE status;

			SpiFlash_DmaCmd(SPICMD_RDSR, DMA_TARGET_CHIP, 0x4D0, 1, DMA_OPT_NONE);
			SpiFlash_wait_done(10, 10);
			status = ReadTW88(REG4D0);

			SpiFlash_WriteEnable();

			SPI_CmdBuffer[0]= status | 0x40;
			SpiFlash_DmaCmd(SPICMD_WRSR, DMA_TARGET_CHIP | 0x01, 0, 0, SPI_CMD_OPT_BUSY);
			SpiFlash_wait_done(10, 10);
			ret = SpiFlash_check_busy(10, 10);
			break;		
		}
	}

	return 0;
}

BYTE check_4b_all(void)
{
	BYTE mid = spiflash_chip->mid;

	switch (mid)
	{
	case SPIFLASH_MID_MX:
		return check_spiflash_status_register(SPICMD_RDCR, 1, 0x20, 0x20, 0, 0);
	case SPIFLASH_MID_MICRON:
		return check_spiflash_status_register(SPICMD_RDFREG, 1, 0x01, 0x01, 0, 0);
	case SPIFLASH_MID_WB:
		return check_spiflash_status_register(SPICMD_RDSR3, 1, 0x01, 0x01, 0, 0);
	case SPIFLASH_MID_GIGA:	/* I have only 128, so I don't know */
	case SPIFLASH_MID_ISSI:	/* I have only 128, so I don't know */
		return FALSE;
	case SPIFLASH_MID_EON:
	case SPIFLASH_MID_SPANSION:
	default:
		return check_spiflash_status_register(SPICMD_RDINFO, 1, 0x04, 0x04, 0, 0);			
	}
}

/*!
 *	Read JEDEC id and search table.
 *  If fail, it will have a {0x00,0x00,0x00,} table.
 */
struct SPIFLASH_DIMEMSION *find_spiflash_chip(void)
{
	struct SPIFLASH_DIMEMSION *spiflash1_chip;
	BYTE mid,did0,did1;
	BYTE ret;
	
	SpiFlash_DmaCmd(SPICMD_RDID,DMA_TARGET_CHIP, 0x4D0, 3, DMA_OPT_NONE);
	ret = SpiFlash_wait_done(10, 10);
	if (ret) 
	{
		Puts(" SPICMD_RDID fail");
		mid = 0;
		did0 = 0;
		did1 = 0;
        /* RDID DMA was failed. */
        return NULL;
	}

	mid  = ReadTW88(REG4D0);	//manufacturer id
	did0 = ReadTW88(REG4D1);	//device id0
	did1 = ReadTW88(REG4D2);	//device id1
	Printf(" %02bx:%02bx:%02bx", mid, did0, did1);

	spiflash1_chip = spiflash_chip_table;	
	while (spiflash1_chip->mid)
	{
		if (spiflash1_chip->mid == mid)
		{
			if (spiflash1_chip->did0 == did0 && spiflash1_chip->did1 == did1)
			{
				/* found */
				Printf(" %s", spiflash1_chip->name);
				return spiflash1_chip;
			}
		}
		spiflash1_chip++;
	}
	Puts(" search fail");
	Puts("\n\rWarning:Unknown SPIFLASH. System can be corrupted");

	return spiflash1_chip; /*unknown chip. It has spiflash_chip->mid=0.*/
}

extern DWORD e3p_spi_start_addr;
/*!
 * initiaize spiflash
 */
BYTE init_spiflash_chip(void)
{
	BYTE ret;

	Puts("SpiFlash ");
	
	spiflash_chip = find_spiflash_chip();
	if (spiflash_chip == NULL)
	{
        /* something wrong...use NoINIT mode */
		return ERR_FAIL;
	}

	if (spiflash_chip->mid == 0)
	{
		//---------------------
        //FW failed to detect SPIFLASH.
		//FAST is better than single.
		SpiFlash_SetReadModeByRegister(SPI_READ_FAST);
		Puts("mid was 0. FW uses FAST mode\n");
        //if(fWatchdog)
        //    return ERR_FAIL;
		return ERR_SUCCESS;
	}

	/* Check fast read mode. Is it QuadO or QuadIO ? */
	ret = quadio_check_all();
	if (ret == 0)
	{
        /* Quad did not enabled, enable Quad */
		Puts(" EnQuad");
		ret = quadio_enable_all();
		Puts(".");
	}

	/* setup fast read mode; Quad or QuadIO */
	SpiFlash_SetReadModeByRegister(spiflash_chip->fast_mode);
	if (spiflash_chip->fast_mode == 3)
		Puts(" QuadO");
	else
		Puts(" QuadIO");		

	if (spiflash_chip->size > 128)
	{
        /* enable 4Byte address mode */
		SpiFlash_Set4BytesAddress(ON);
		if (SpiFlash4ByteAddr) 
			Puts(" En4B");
	}
	Puts("\n");

	if (spiflash_chip->mid == SPIFLASH_MID_SPANSION)
	{
		//S25FL 256 S AG M F I 0 0 1
		//                       |
		//                       +- Model Number (Sector Type)
		//TW8836 need 0. they call it as "Uniform 64-kB sectors"
		//    = A hybrid of 32 x 4-kB sectors with all remaining sectors being 64 kB, 
        //      with a 256B programming buffer.
		//
		/*
		The main flash array is divided into erase units called sectors. 
        The sectors are organized either as a hybrid combination of 4-kB and 64-kB sectors, 
        or as uniform 256-kbyte sectors. The sector organization depends on
		the device model selected, see Ordering Information on page 149. 

		If it is S25FL256S, I will use 0x01FE0000~0x01FFFFFF.
		*/
		if     (spiflash_chip->did1==0x17) e3p_spi_start_addr = 0x00800000 - 0x20000;	//64Mbit
		else if(spiflash_chip->did1==0x18) e3p_spi_start_addr = 0x01000000 - 0x20000;	//128Mbit
		else if(spiflash_chip->did1==0x19) e3p_spi_start_addr = 0x02000000 - 0x20000;	//256Mbit
		else if(spiflash_chip->did1==0x20) e3p_spi_start_addr = 0x04000000 - 0x20000;	//512Mbit
		else
		{
			Puts("\n\rError. Spansion (512 or bigger) does not support 4kByte Secotr");
		}
    }

	return ERR_SUCCESS;
}

/**
 *
 * param	cmd		status read command
 * param	control
 *			[3] Busy polarity	0:Low, 1:High
 *			[2:0]	Busy bit in the status
 * 
 * register 
 	REG4D8[7:0]	read status command
	REG4D9[3]	busy polarity
	REG4D9[2:0]	busy bit
	REG4C4[2]	enable busy check
 *			
 */
/*
normally the status read command is 0x05
status has below flags
[0] WIP(WriteInProgress)	1=write operation
So, the default is SpiFlashBusyCheck(0x05, 0x08);
but, N25Q512 has different.
	cmd: 0x05 Read Status Register
	cmd: 0x70 Read Flag Status Register

Status Register
[0] Write in progress	1:Busy. 

Flag Status Register
[7] Program or erase controller
[4] Program 1:Failure 
*/
#define STATUS_REG_READ_CMD	0x05
#define STATUS_REG_WIP_LOC	0
#define STATUS_REG_WIP_BUSY	1

void SpiFlashSetupBusyCheck(BYTE cmd, BYTE busy_control)
{
	WriteTW88(REG4D8,cmd);
	WriteTW88(REG4D9,busy_control);	
}


//=============================================================================
//
//=============================================================================
//-----------------------------------------------------------------------------
/**
* set SpiFlash ReadMode
*
* updata HW and, SPICMD_x_READ and SPICMD_x_BYTES.
*
* @param mode
*	- 0: slow	CMD:0x03	BYTE:4
*	- 1: fast	CMD:0x0B	BYTE:5
*	- 2: dual	CMD:0x3B	BYTE:5
*	- 3: quad	CMD:0x6B	BYTE:5
*	- 4: Dualo	CMD:0xBB	BYTE:5
*	- 5: QuadIo	CMD:0xEB	BYTE:7
*	- 6: Dedge  CMD:0xED	BYTE:12
*/
void SpiFlash_SetReadModeByRegister( BYTE mode )
{
	WriteTW88(REG4C0, (ReadTW88(REG4C0) & ~0x07) | mode);

	switch( mode ) {
		case 0:	//--- Slow
			//max speed is 50MHz.
			//but, normally, 54MHz is working.
			SPICMD_x_READ	= 0x03;	
			SPICMD_x_BYTES	= 4;	//(8+24)/8
			break;
		case 1:	//--- Fast
			SPICMD_x_READ	= 0x0b;	
			SPICMD_x_BYTES	= 5;   //(8+24+8dummy)/8.
			break;
		case 2:	//--- Dual
			SPICMD_x_READ	= 0x3b;
			SPICMD_x_BYTES	= 5;   //(8+24+8dummy)/8
			break;
		case 3:	//--- Quad
			SPICMD_x_READ	= 0x6b;	
			SPICMD_x_BYTES	= 5;   //(8+24+8dummy)/8
			break;
		case 4:	//--- Dual-IO
			SPICMD_x_READ	= 0xbb;	
			SPICMD_x_BYTES	= 5;  //(8+(12+4dummy)*2lines)/8
  
			break;
		case 5:	//--- Quad-IO. HW using fixed 6 dummy cycle.
			SPICMD_x_READ	= 0xeb;	 
			SPICMD_x_BYTES	= 7;   //(8+(6+6dummy)*4lines)/8
			break;

		case 6:	//--- DEdge. DTR(Double Transfer Rate). DO NOT USE.
			SPICMD_x_READ	= 0xed;	 
			SPICMD_x_BYTES	= 12;   
			break;

		default:
			//fast read mode can support Extender/Qual/Quad.
			SPICMD_x_READ	= 0x0b;	
			SPICMD_x_BYTES	= 5;   //(8+24+8)/8. 8 dummy
			ePrintf("Unknow SPI rmode:%bx",mode);
			ePrintf(" set as fast !!!\n");
			break;		
 	}
}


//*****************************************************************************
//
//		EEPROM Emulation
//
//*****************************************************************************
//	Format: For each 4 bytes [Index] [Index^FF] [Data] [Data^FF]
//
//
#ifdef USE_SFLASH_EEPROM
//moved to E3P_c24.lib or E3P_bank.lib
#endif

//=============================================================================
// Note
//=============================================================================
/*!
 * Micron SpiFlash

//BK131119...0xEF,0x6F is correct N25Q256. It uses a little-endian.
//micron datasheet description.
//N25Q256A ID:20_BA_19						//N25Q128A ID:20_BA_18
//[15:12] 	dummy cycles 	= 0110			//[15:12] 	dummy cycles 	= 0110
//[11:9]  	XIP 			= 111			//[11:9]  	XIP 			= 111
//[8:6] 	output 			= 111			//[8:6] 	output 			= 111
//[5] 		reserved		= 1				//[5] 		reserved		= 1
//[4] 		Reset/hold 		= 1				//[4] 		Reset/hold 		= 1
//[3] 		QuadIo 			= 0				//[3] 		QuadIo 			= 0
//[2] 		DualIO 			= 1				//[2] 		DualIO 			= 1
//[1]		128M segment	= 1				//[1]		Reserved		= 1
//[0] 		Address 		= 1				//[0] 		Lock 			= 1. DO NOT WRITE 0.
//
//----EF:6F is working. 
//----if you write F7:6F, N25Q128A & N25Q256A and it can not recover.
//validated register description.
//first BYTE
//[7:3]
//[4]		Disable Reset/Hold:0 to use DQ3.
//[3:0]
//second BYTE
//[7:4] dummy cycle:6=0110.
//[3:0]
*/

