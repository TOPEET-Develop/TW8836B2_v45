/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/
#ifndef __SPI_H__
#define __SPI_H__

//-----------------------------------------------------------------------------
// SPI FLASH Command
//-----------------------------------------------------------------------------
#define SPICMD_WRSR				0x01	//write status register
#define SPICMD_PP				0x02	//Page program
#define SPICMD_READ_SLOW		0x03	//Read data
#define SPICMD_WRDI				0x04	//write disable
#define SPICMD_RDSR				0x05	//read status register
#define SPICMD_WREN				0x06	//write enable
#define SPICMD_READ_FAST		0x0B	//fast read data
#define SPICMD_FASTDTRD			0x0D	//fast DT read
#define SPICMD_RDCR				0x15	//read configuration register(Macronix)
#define SPICMD_RDSR3			0x15	//read status3 register(WB). dat[0]:4B
#define SPICMD_SE				0x20	//sector erase
#define SPICMD_RDINFO			0x2B	//read information register. S[2]=1:4byte mode 
#define SPICMD_RDSCUR			0x2B	//read security register
#define SPICMD_WRSCUR			0x2F	//write security register
#define SPICMD_CLSR				0x30	//clear SR fail flags
#define SPICMD_WRSR2			0x31	//write status register2(WB) 
#define SPICMD_RDSR2			0x35	//read status2 register(WB). dat[1]:QE
#define SPICMD_SBLK				0x36	//single block lock
#define SPICMD_4PP				0x38	//quad page program
#define SPICMD_SBULK			0x39	//single block unlock
#define SPICMD_READ_DUAL_O		0x3B	
#define SPICMD_RDBLOCK			0x3C	//block protect read
#define SPICMD_EXPLM			0x45	//exit parallel mode
#define SPICMD_CLRFREG			0x50	//clear flag status register(micron)
#define SPICMD_BE32K			0x52	//block erase 32KB
#define SPICMD_ENPLM			0x55	//enter parallel mode
#define SPICMD_CE				0x60	//chip erase. 0x60 or 0xC7
#define SPICMD_WDVEREG			0x61	//write volatile enhanced register(micron)
#define SPICMD_RDVEREG			0x65	//read volatile enhanced register(micron)
#define SPICMD_ENHBL			0x67	//enter high bank latch mode
#define SPICMD_WPSEL			0x68	//write protection selection
#define SPICMD_READ_QUAD_O		0x6B
#define SPICMD_RDFREG			0x70	//read flag status register(micron)
#define SPICMD_ESRY				0x70	//enable SO to output RY/BY#
#define SPICMD_GBLK				0x7E	//gang block lock
#define SPICMD_DSRY				0x80	//disable SO to output RY/BY#
#define SPICMD_WDVREG			0x81	//write volatile register(micron)
#define SPICMD_RDVREG			0x85	//read volatile register(micron)
#define SPICMD_REMS				0x90	//read electronic manufacturer & device ID
#define SPICMD_EXHBL			0x98	//exit high bank latch mode
#define SPICMD_GBULK			0x98	//gang block unlock
#define SPICMD_RDID				0x9F	//read identification.
#define SPICMD_HPM				0xA3	//high performance enable mode
#define SPICMD_RDP				0xAB	//Release from deep power down
#define SPICMD_RES				0xAB	//read electronic ID
#define SPICMD_CP				0xAD	//continusly program mode
#define SPICMD_WDNVREG			0xB1	//write non-volatile register(micron)
#define SPICMD_ENSO				0xB1	//enter secured OTP
#define SPICMD_RDNVREG			0xB5	//read non-volatile register(micron)
#define SPICMD_EN4B				0xB7	//enter 4Byte mode
#define SPICMD_DP				0xB9	//Deep power down
#define SPICMD_READ_DUAL_IO		0xBB	//2x I/O read command
#define SPICMD_2DTRD			0xBD	//dual I/O DT Read
#define SPICMD_EXSO				0xC1	//exit secured OTP
#define SPICMD_REMS4D			0xCF	//read ID for 4x I/O DT mode
#define SPICMD_BE				0xD8	//block erase 64KB
#define SPICMD_REMS4			0xDF	//read ID for 4x I/O mode
#define SPICMD_RDLOCK			0xE8	//read Lock register(micron)
#define SPICMD_EX4B				0xE9	//exit 4Byte mode
#define SPICMD_READ_QUAD_IO		0xEB	//4x I/O read command
#define SPICMD_4DTRD			0xED	//Quad I/O DT Read
#define SPICMD_REMS2			0xEF	//read ID for 2x I/O mode

//-----------------------------------------------------------------------------
//		SPI Read Mode
//-----------------------------------------------------------------------------
//SPICMD_READ summary
//			cmd		format	length                            dummy	speed on Micron
//_SLOW		0x03	1-1-1 	cmd:1 addr:3(4) extra:0 total:4(5)
//_FAST		0x0B	1-1-1 	cmd:1 addr:3{4} extra:1 total:5(6) 	8 	(133/108)
//_DUAL_O	0x3B	1-1-2 	cmd:1 addr:3(4) extra:1 total:5(6) 	8 	(133/108)
//_QUAD_O	0x6B	1-1-4 	cmd:1 addr:3(4) extra:1 total:5(6) 	8 	(133/108)
//_DUAL_IO	0xBB	1-2-2 	cmd:1 addr:3(4) extra:1 total:5(6) 	4 	(97/90)
//_QUAD_IO	0xEB	1-4-4 	cmd:1 addr:3(4) extra:3 total:7(8) 	6 	(86/80)
//-----------------------------------------------------------------------------
//                                 command   TW8836 dummy	cmd_bytes(cmd+addr+dummy)
#define	SPI_READ_SLOW		0		//0x03					4
#define SPI_READ_FAST 		1		//0x0B		8			5
#define SPI_READ_DUAL	 	2		//0x3B		8			5
#define SPI_READ_QUAD  		3		//0x6B		8			5
#define SPI_READ_DUALIO		4		//0xBB		4			5
#define SPI_READ_QUADIO		5		//0xEB		6			7
#define SPI_READ_DEDGE		6		//QUADIO with DTR(Double Transfer Rate) mode for micron

//--------------------
// SPI FLASH Vendor	and ID
//--------------------
#define SPIFLASH_MID_MX			0xC2
#define SPIFLASH_MID_EON		0x1C
#define	SPIFLASH_MID_WB			0xEF
#define SPIFLASH_MID_MICRON		0x20
#define SPIFLASH_MID_SPANSION	0x01
#define SPIFLASH_MID_GIGA		0xC8
#define SPIFLASH_MID_ISSI 		0x9D

struct SPIFLASH_DIMEMSION {
	BYTE	mid;
	BYTE	did0;
	BYTE	did1;

	WORD	size;
	BYTE	fast_mode; 		//0:single,1:fast,3:Quad,5:QuadIO
	BYTE	typical_speed;	//fast read mode speed

	BYTE sspll_0f8;
	BYTE sspll_0f9;
	BYTE sspll_0fa;
	BYTE sspll_0fd;

	BYTE pllclk_s;			//select PLLCLK	REG4E0[0]
	BYTE pllclk_d;			//divider for PLLCLK REG4E1[3:0].
	BYTE pllclk_dma;		//divider for tSHSL.
	BYTE mcuclk_d;			//divider for MCUCLK REG4F0[7:4]. if not 0, use async.
	char name[16];
};
extern struct SPIFLASH_DIMEMSION *spiflash_chip;
extern code BYTE async_wait_value[];
extern code struct SPIFLASH_DIMEMSION spiflash_chip_table[];




//----------------------------------------------
//SPI_Buffer[]
//Note: SPI_Buffer[] is a largest buffer in TW8836 FW.
//		some other modules use this SPI_Buffer[].
//      ex:measure.c. settings.c
#define SPI_BUFFER_SIZE		128
extern  XDATA BYTE SPI_Buffer[SPI_BUFFER_SIZE];
extern  XDATA BYTE SPI_CmdBuffer[8];

extern BYTE SPICMD_x_READ;
extern BYTE SPICMD_x_BYTES;
extern BYTE SpiFlash4ByteAddr;	


BYTE SpiFlash_check_busy(BYTE wait, BYTE unit);

//BYTE SpiFlash_wait_done(BYTE cmd, BYTE wait, BYTE unit);
//void SpiFlash_DmaCmd(BYTE cmd, BYTE target_cmd_len, WORD pBuff, WORD buff_len, BYTE option);
BYTE SpiFlash_4B_DmaCmd(BYTE cmd);
BYTE SpiFlash_WriteEnable(void);
BYTE SpiFlash_SectorErase(DWORD spiaddr);
BYTE SpiFlash_BlockErase(DWORD spiaddr);
BYTE SpiFlash_PageProgram_XMem(DWORD spiaddr, WORD xaddr, WORD xlen);
BYTE SpiFlash_PageProgram_ChipReg(DWORD spiaddr, BYTE *pBuff, BYTE len);

BYTE SpiFlash_FastRead_ChipReg(DWORD spiaddr, BYTE len); 
BYTE SpiFlash_FastRead_Fixed_ChipReg(DWORD spiaddr, DWORD dLen);
BYTE SpiFlash_Read_XMem(DWORD spiaddr, WORD xaddr, WORD xlen);
 
BYTE SpiFlash_Read_FOsd(DWORD spiaddr, WORD addr, WORD len); 
BYTE SpiFlash_Read_SOsd(DWORD spiaddr, WORD addr, WORD len);





BYTE init_spiflash_chip(void);

//==============================================
//==============================================
BYTE is_macronix_256(void);
BYTE is_micron_512(void);
BYTE is_micron_256or512(void);
BYTE is_winbond_256(void);

//extern BYTE (*check_4b_address_mode)(void);
extern BYTE check_4b_all(void);


//void SpiFlashDmaStop(void);
void SpiFlashSetReadMode(BYTE mode);
BYTE SpiFlashGetReadMode(void);

#define DMA_DEST_FONTRAM			0
#define DMA_DEST_CHIPREG			1
#define DMA_DEST_SOSD_LUT			2
#define DMA_DEST_MCU_XMEM			3

//new:151215...
#define DMA_TARGET_FOSD			0x00
#define DMA_TARGET_CHIP			0x40
#define DMA_TARGET_SOSD			0x80
#define DMA_TARGET_XMEM			0xC0

#define DMA_ACCESS_INC			0x00
#define DMA_ACCESS_DEC			0x10
#define DMA_ACCESS_FIX			0x20

#define DMA_OPT_NONE			0x00
#define DMA_OPT_READ			0x00
#define DMA_OPT_WRITE			0x02
#define DMA_OPT_BUSY			0x04
#define DMA_OPT_BUSY_WRITE		0x06

BYTE SpiFlash_wait_done(BYTE wait, BYTE unit);
void SpiFlash_DmaCmd(BYTE cmd, BYTE target_cmd_len, WORD pBuff, WORD buff_len, BYTE option);


#define	SPIDMA_READ					0
#define SPIDMA_WRITE				1
#define SPIDMA_BUSYCHECK			1
BYTE SpiFlashDmaStart(BYTE dma_option, BYTE wait);

#define DMA_BUFF_REG_ADDR_PAGE		0x04
#define DMA_BUFF_REG_ADDR_INDEX		0xD0
#define DMA_BUFF_REG_ADDR			0x04D0
void SpiFlashDmaBuffAddr(WORD addr);
void SpiFlashDmaReadLen(DWORD len);
void SpiFlashDmaReadLenByte(BYTE len_l);
//BYTE SpiFlashSetAddress2Hw(DWORD addr);
BYTE SpiFlashSetAddress2CmdBuffer(DWORD spiaddr);

#define SPIFLASH_WAIT_READ		200
void SpiFlashDmaRead(BYTE dest_type,WORD dest_loc, DWORD src_loc, WORD size, BYTE wait);
//void SpiFlashDmaRead2XMem(BYTE * dest_loc, DWORD src_loc, WORD size, BYTE wait);
void SpiFlashCmdRead(BYTE dest);
void SpiFlashSetupBusyCheck(BYTE cmd, BYTE busy_control);
BYTE SpiFlashBusyCheck(WORD wait);
#define SPI_CMD_OPT_NONE		0x00
#define SPI_CMD_OPT_BUSY		0x04
#define SPI_CMD_OPT_WRITE		0x02
#define SPI_CMD_OPT_WRITE_BUSY	0x06
#define SPI_CMD_OPT_WRITE_BUSY_AUTO	0x16
//BYTE SpiFlashChipRegCmd(BYTE cmd, BYTE w_len, BYTE r_len, BYTE opt, BYTE wait);

//==============================================
//==============================================
//SPI Mode:	0=Slow, 1=fast, 2=dual, 3=quad, 4=dual-io, 5=quad-io
void SpiFlash_SetReadModeByRegister(BYTE mode);
void SpiFlash_Set4BytesAddress(BYTE fOn);

//void SPI_WriteEnable(void);
//void SPI_SectorErase( DWORD spiaddr );
//void SPI_BlockErase( DWORD spiaddr );
//void SPI_PageProgram( DWORD spiaddr, WORD x_addr, WORD cnt );

void print_spiflash_status_register_default(void);
extern void (*print_spiflash_status_register)(void);

#endif // __SPI_H__
