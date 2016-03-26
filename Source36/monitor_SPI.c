/**
 * @file
 * Monitor_SPI.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	Interface between TW_Terminal2 and Firmware.
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
//*****************************************************************************
//
//								Monitor_SPI.c
//
//*****************************************************************************
//
//
#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "Global.h"
#include "CPU.h"
#include "printf.h"
#include "util.h"
#include "monitor_SPI.h"

#include "i2c.h"
#include "SPI.h"

#include "eeprom.h"

static void SPI_Status(void);
void SPI_dump(DWORD spiaddr); 


//TW8835B SPIFlash Dongle Pin
/*
	3V	CS
	D2	D1
	D3	CLK
	#RST D0
	GND	GND


	VD0~VD3 has a pull-up.
	P1_6 P1_7 P3_2 P3_3
*/

#define SPI_S			P1_6	//VD00	
#define SPI_DQ1			P1_7	//VD01
#define SPI_C			P3_2	//VD02
#define SPI_DQ0			P3_3	//VD03
//#define SPI_DQ2			P1_4
//#define SPI_DQ3			P1_5

#pragma SAVE
#pragma OPTIMIZE(2,SIZE)
static void dd_spi(BYTE delay)
{
	BYTE i,j;
	j = I2C_delay_base;
	while(j--) {
		for(i=0; i < delay; i++);
	}
}
#pragma RESTORE

                                    	//32kHz
#define wait_DVCH		dd_spi(2)		//				
#define wait_CHDX		dd_spi(3)		//				
#define wait_SHSL2		dd_spi(50)		//				

/* emulate SPI Protocol */
/*
	generate clock with S and DQ0
*/
void SPI_Set_S_DATA(BYTE s, BYTE dq0)
{
	SPI_C=1;
	SPI_S = s ? 1:0;
	SPI_DQ0 = dq0 ? 1:0;
	SPI_C=0;
	wait_DVCH;	// >=2nS Data in setup time
	SPI_C=1;
	wait_CHDX; 	// >=3nS Data in hold time
}
void SPI_write_byte(BYTE value)
{
	BYTE mask;
	BYTE bTemp;

	mask=0x80;
	while(mask) {
		bTemp = value & mask ? 1:0;
		SPI_Set_S_DATA(0,bTemp);
		mask >>=1;	
	}
}
BYTE SPI_read_byte(void)
{
	BYTE value;
	BYTE i;

	value=0x00;
	for(i=0; i < 8; i++) {
		value <<=1;
		SPI_C=0;
		wait_DVCH;	// >=2nS Data in setup time
		SPI_C=1;
		value |= (SPI_DQ1 ? 1:0);
		wait_CHDX; 	// >=3nS Data in hold time
	}
	return value;
}

//0x66 0x99
void SPI_Reset_Command(void)
{
	//Reset Enable
	SPI_write_byte(0x66);
	SPI_Set_S_DATA(1,0);
	wait_SHSL2;	
	//Reset Memory
	SPI_write_byte(0x99);
	SPI_Set_S_DATA(1,0);
	wait_SHSL2;
	/* 90nS ~ 30uS */	
}
void SPI_9F_Command(void)
{
	BYTE value;

	//Read ID
	SPI_write_byte(0x9F);
	value=SPI_read_byte();
	Printf(" %02bx",value);
	value=SPI_read_byte();
	Printf(" %02bx",value);
	value=SPI_read_byte();
	Printf(" %02bx",value);
}


/* special pattern for Recovery Sequence */
void SPI_Set_sequence(BYTE n)
{
	BYTE i;
	for(i=0; i < n; i++)
		SPI_Set_S_DATA(0,1);
	SPI_Set_S_DATA(1,1);
	wait_SHSL2;	// >= 50nS S# deselect time after a nonREAD command				
}

void SPI_Recovery(void)
{
	SPI_S = 1;
	//SPI_DQ3 = 1; //keep 1
	SPI_C = 1;

	//first part
	SPI_Set_sequence(7);
	SPI_Set_sequence(9);
	SPI_Set_sequence(13);
	SPI_Set_sequence(17);
	SPI_Set_sequence(25);
	SPI_Set_sequence(33);
	//second part
	SPI_Set_sequence(8);
}



//=============================================================================
// help for MonitorSPI()
//=============================================================================
void HelpMonitorSPI(void)
{
	Printf("\n\r\n  === Help for SPI command ===");
	Printf("\n\rSPI D [addr]      ; dump");
	Printf("\n\rSPI sector [addr] ; sector erase");
	Printf("\n\rSPI block [addr]  ; block erase");
   	Printf("\n\rSPI rmode [mode]	; set the read mode");
   	Printf("\n\rSPI 4B [mode]		; set the 4Byte Addr mode");
	Printf("\n\rSPI EnQIO      		; enable Fast QuadIO --need to implement");

	//Printf("\n\rSPI W addr data   ; write 1 byte. Use PageProgram");
	Printf("\n\rSPI W addr data ...  ; write max 8 byte. Use PageProgram");
   	Printf("\n\rSPI copy [src] [dst] [cnt] ; copy");
	Printf("\n\rSPI status      	; status");

	Puts("\nBelow 3 commands need special connections and Groud,Power");
	Puts("\n\rSPI recovery [VD0:S        VD2:C VD3:D0 DQ3:high]");
	Puts("\n\rSPI reset    [VD0:S        VD2:C VD3:D0]");
	Puts("\n\rSPI id       [VD0:2 VD1:D1 VD2:C CD3:D0]");

	Printf("\n\r");
}

//=============================================================================
//
//=============================================================================
void MonitorSPI(void)
{
#ifdef USE_SFLASH_EEPROM
	BYTE j;
#endif
	if(argc < 2) {
		HelpMonitorSPI();
		return;
	}
	else if( !stricmp( argv[1], "?" ) ) {
		HelpMonitorSPI();
		return;
	}
	//---------------------- Dump SPI --------------------------
	else if( !stricmp( argv[1], "D" ) ) {
		static DWORD spiaddr = 0;

		if( argc>=3 ) spiaddr = a2h( argv[2] );

		SPI_dump(spiaddr); 

		spiaddr += 0x80L;
	}
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "status" ) ) {
		SPI_Status();
	}
//	else if( !stricmp( argv[1], "quad" ) ) {
//		SPI_quadio();
//	}
	//--------------------------------------------------------
#ifdef USE_SFLASH_EEPROM
	else if( !stricmp( argv[1], "t" ) ) {
	
		SpiFlash_SectorErase( E3P_SPI_SECTOR0 );
		
		for(j=0; j<128; j++) {
			SPI_Buffer[j] = j;
		}
		//SpiFlash_PageProgram_XMem( E3P_SPI_SECTOR0, (WORD)SPI_Buffer, 128 );
		SpiFlash_PageProgram_XMem( E3P_SPI_SECTOR0, (WORD)SPI_Buffer, 8 );
	}
#endif
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "Sector" ) ) {
		static DWORD spiaddr = 0x10000L;

		if( argc>=3 ) spiaddr = a2h( argv[2] );

		Printf("\n\rSector Erase = %06lx", spiaddr);

		SpiFlash_SectorErase(spiaddr);
		spiaddr += 0x1000L;	/* increase 4K */
	}
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "Block" ) ) {
		static DWORD spiaddr = 0x10000L;

		if( argc>=3 ) spiaddr = a2h( argv[2] );

		Printf("\n\rBlock Erase = %06lx", spiaddr);

		SpiFlash_BlockErase(spiaddr);
		spiaddr += 0x10000L; /* increase 64K */
	}
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "Copy" ) ) {
		DWORD source = 0x0L, dest = 0x10000L;
		DWORD cnt=0x100L;

		if( argc<4 ) {
			Printf("\n\rMissing Parameters !!!" );
			return;
		}
		source = a2h( argv[2] );
		dest   = a2h( argv[3] );
		cnt    = a2h( argv[4] );

		Printf("\n\rSPI copy from %06lx to %06lx", source, dest);
		Printf("  %03x bytes", (WORD)cnt);
		SpiFlash_Read_XMem(source, (WORD)SPI_Buffer, cnt);
		//SpiFlashDmaRead2XMem(SPI_Buffer,source,cnt, SPIFLASH_WAIT_READ);

		SpiFlash_PageProgram_XMem( dest, (WORD)SPI_Buffer, cnt );
	}
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "rmode" ) ) {
		BYTE mod;

		if(argc < 3) {
			mod = ReadTW88(REG4C0) & 0x07;

			SPICMD_x_READ	= 0x03;	
			SPICMD_x_BYTES	= 4;	//(8+24)/8

			Printf("\n\rSPI Read Mode = %bd READ:%02bx BYTES:%bd", mod, SPICMD_x_READ,SPICMD_x_BYTES);
			if(mod != 5) {
				Printf(". To select QuadIO, spi rmode 5");	
			}
			//mod = 0x05;		//QuadIO
			return;
		}
		else
			mod = a2h( argv[2] );

		SpiFlash_SetReadModeByRegister(mod);
		Printf("\n\rSPI Read Mode = %bd 0x%bx", mod, ReadTW88(REG4C0) & 0x07);
		Puts(" Please, set 4B value!!");
	}
	/* 4Byte address mode */
	else if( !stricmp( argv[1], "4B" ) ) {
		BYTE mod;
		BYTE ret;

		if(argc == 3) {
			mod = a2h( argv[2] );
			SpiFlash_Set4BytesAddress(mod);
		}
//		ret=check_4b_address_mode();
		ret=check_4b_all();
		Printf("\n\rSPI 4Byte Mode SW:%bd HW:%bx", SpiFlash4ByteAddr, ret==TRUE ? 1:0 );

	}
	//---------------------- Write SPI --------------------------
	else if( !stricmp( argv[1], "W" ) ) {
		static DWORD spiaddr = 0;
		BYTE *ptr = SPI_Buffer;
		DWORD size;
		BYTE i;

		if( argc<3 ) {
			Printf("\n\rMissing Parameters !!!" );
			return;
		}
		if( argc > 7 ) {
			Printf("\n\ronly support 4 bytes !!!" );
		}
		

		spiaddr = a2h( argv[2] );
		//only support eeprom area....please
		//if((spiaddr < EE_SPI_SECTOR0) || (spiaddr > EE_SPI_SECTOR0+0x010000)) {
		//	Printf("\n\rout of range %06lx~%06lx!!!", EE_SPI_SECTOR0, EE_SPI_SECTOR0+0x010000 );
		//	return;
		//}
		Printf("\n\rWrite SPI %06lx ", spiaddr);


		size=0;
		for(i=3; i < argc; i++) {
			*ptr++ = (BYTE)a2h(argv[i]);
			Printf(" %02bx ",(BYTE)a2h(argv[i]));
			size++;
		}	
		SpiFlash_PageProgram_XMem( spiaddr, (WORD)SPI_Buffer, (WORD)size);
	}
	/* new */
	//---------------------- Micron Recovery Sequency --------------------
	else if( !stricmp( argv[1], "recovery" ) )
		SPI_Recovery();
	else if( !stricmp( argv[1], "reset" ) )
		SPI_Reset_Command();
	else if( !stricmp( argv[1], "id" ) )
		SPI_9F_Command();
	//---------------------- old    --------------------
	else if(argc >= 2 && argv[1][0] >= '0' && argv[1][0] <= '9' ) {
		BYTE rcnt,wcnt;
		BYTE i;
		volatile BYTE vdata;
		//old spi command style
		if(argc < 3) {
			Puts("\n\rSPI nRead cmd...	;command old style.");
			return;
		}

		rcnt = a2h(argv[1]);

		wcnt = argc - 2;
		WriteTW88(REG4C3,(DMA_DEST_CHIPREG << 6) | wcnt);
	 	WriteTW88(REG4C6, 0x04 );						// data Buffer address 0x04D0
		WriteTW88(REG4C7, 0xD0 );						// data Buffer address
		WriteTW88(REG4DA, 0x00 );						// data Buff count high
		WriteTW88(REG4C8, 0x00 );						// data Buff count middle
		WriteTW88(REG4C9, rcnt );						// data Buff count Lo
		for(i=0; i < wcnt; i++) {
			WriteTW88(REG4CA+i, a2h(argv[2+i]) );
		}
		WaitVBlank(1);


		if(spiflash_chip->pllclk_dma) {
			BYTE r4e1;
			r4e1 = ReadTW88(REG4E1) & 0xF0;
			WriteTW88(REG4E1, r4e1 | spiflash_chip->pllclk_dma);
			if(rcnt)
				WriteTW88(REG4C4, 0x01 );						// DMA-Read
			else
				WriteTW88(REG4C4, 0x03 );						// DMA-Write
			WriteTW88(REG4E1, r4e1 | spiflash_chip->pllclk_d);
		}
		else {
			if(rcnt)
				WriteTW88(REG4C4, 0x01 );						// DMA-Read
			else
				WriteTW88(REG4C4, 0x03 );						// DMA-Write
		}




		for(i=0; i < 200; i++) {
			vdata = ReadTW88(REG4C4);
			if((vdata & 0x01)==0)
				break;
			delay1ms(10);
		}
		Printf("wait:%bd,%bx data:",i,vdata);

		if(rcnt) {
			Puts("\bSPI Read==> ");
			for(i=0; i<rcnt; i++) {
				Printf("%02bx ",ReadTW88(REG4D0+i));
			}
		}
	}
	//--------------------------------------------------------
	//--------------------------------------------------------
	else
		Printf("\n\rInvalid command...");	
}
//=============================================================================
//
//=============================================================================
void HelpMonitorSPIC(void)
{
	Printf("\n\r\n  === Help for SPIFlash command ===");
	Printf("\n\rSPICMD_RDID: read JEDEC id.	cmd:spic r3 9f");
	Printf("\n\rSPICMD_WREN: write enable. cmd:spic 6");
	Printf("\n\rSPICMD_WRDI: write disable. cmd:spic 4");
	Printf("\n\rSPICMD_RDSR: read status register.	cmd:spic r1 5");
	Printf("\n\rSPICMD_WRSR: write status register. cmd:spic 1 stat1 stat2");
	Printf("\n\rSPICMD_EN4B: enter 4Byte mode. cmd:spic b7");
	Printf("\n\rSPICMD_EX4B: exit 4Byte mode. cmd:spic e9");
	Printf("\n\rSPICMD_PP: PageProgram. (use XMem) cmd:spic 2 addr data...");
	Printf("\n\rSPICMD_SE: sector erase. cmd spic 20 addr");
	Printf("\n\rSPICMD_BE: block erase. cmd:spic d8 addr");
	Printf("\n\rSPICMD_READ_SLOW: read data. cmd:spic rn 3 addr");
	Printf("\n\rSPICMD_READ_FAST: fast read data. cmd:spic rn b addr");
	Printf("\n\rSPICMD_READ_QUAD_IO: QuadIO read data. (Use XMem). cmd:spic rn eb addr");
	Puts("\n\r");
	Printf("\n\rother command, use spic [rn] cmd [data..]");
	Puts("\n\r");
	Printf("\n\r special: read status: cmd: spic rs");

	Printf("\n\r");
}

void print_spiflash_status_register_macronix(void)
{
	Puts("\n");
	Puts("StatisRegister\n");
	Puts("[7]	SRWD (status register write protect)	1=status register write disable\n");		 
	Puts("[6]	QE(Quad Enable)						1=Quad Enable,\n"); 					
	Puts("[5:2] BP(level of protected block)\n");											
	Puts("[1]   WEL(write enable latch)				1=write enable.		volatile\n");
	Puts("[0]   WIP(write in progress bit)			1=write operation.	volatile\n");
	Puts("\n");
 	Puts("Configuration Register\n");
	Puts("[7:6]	DC(Dummy cycle)			00:6dummy on QuadIO(default:00)	volatile\n");
	Puts("[5]	4BYTE					1:4byte address mode(default:0)	volatile\n");
	Puts("[3]	TB(top/bottom select)	1:Bottom area protect(default:0)	OPT\n");
	Puts("[2:0]	ODS(output driver strength) volatile\n");
}
void print_spiflash_status_register_winbond(void)
{
	Puts("\n");
	Puts("StatisRegister\n");
	Puts("[7]	SRP status register protect\n");		 
	Puts("[6]	TB Top/Bottom Protest Bit\n"); 					
	Puts("[5:2] BP(Block Protest Bit)\n");											
	Puts("[1]   WEL  write enable latch(Status-Only)\n");
	Puts("[0]   BUSY Erase/Write In Progress(Status-Only)\n");
	Puts("\n");
 	Puts("Configuration Register\n");
}
void print_spiflash_status_register_eon_256(void)
{
	Puts("\n");
	Puts("StatisRegister\n");
	Puts("[7]	SRP (status register protect)	1=status register write disable\n");		 
	Puts("[6]	QE(Quad Enable)						1=Quad Enable,\n"); 					
	Puts("[5:2] BP(level of protected block)\n");											
	Puts("[1]   WEL(write enable latch)				1=write enable.		volatile\n");
	Puts("[0]   WIP(write in progress bit)			1=write operation.	volatile\n");
	Puts("\n");
 	Puts("Configuration Register\n");
}
void print_spiflash_status_register_micron(void)
{
	Puts("\n");
	Puts("StatisRegister\n");
	Puts("[7]	status register write enable		1=status register write disable\n");		 
	Puts("[6]	Block protection					\n"); 
	Puts("[5]   Top/Bottom\n");					
	Puts("[4:2] Block protection(with [6])\n");											
	Puts("[1]   write enable latch)					1=Set(default:0) volatile\n");
	Puts("[0]   write in progress bit				1=Busy.	volatile\n");
	Puts("\n");
 	Puts("Configuration Register\n");
}
void print_spiflash_status_register_default(void)
{
	Puts("\nfirst type 'spic r3 9f' command");
}
void print_spiflash_status_register_unknown(void)
{
	Puts("\n");
	Puts("unknown spiflash\n");
}

void (*print_spiflash_status_register)(void);



static void PrintSpiAddr(DWORD spiaddr)
{
	if(SpiFlash4ByteAddr)   Printf(" %08lx", spiaddr);
	else					Printf(" %06lx", spiaddr);
}
void MonitorSPIC(void)
{
	volatile BYTE cmd;
	BYTE dat0;
	BYTE i;
	BYTE cnt;
	DWORD spiaddr;
	BYTE ret;
	BYTE index;
	BYTE read_byte;

	if(argc < 2) {
		HelpMonitorSPIC();
		return;
	}

	index = 0;
	read_byte = 0;
	//---------------------
	//read 'r'
	//---------------------
	if(argv[1][0]=='r') {
		read_byte = argv[1][1] - 0x30;
		if(read_byte > 8) {
			//invalid read option
			HelpMonitorSPIC();
			return;
		}
		index = 1;
	}
	else if(argv[1][0]=='s' && argv[1][1]=='r') {
		//print status register help file
		print_spiflash_status_register();
		return;
	}

	//---------------------
	//read command
	//---------------------
	cmd = a2h( argv[index+1] );

	if(cmd == SPICMD_RDID) {
		if(read_byte != 3) {
			Puts("\nuse spic r3 9f");
			return;
		}
		Printf("\n\rRDID(JEDEC) ");
		SpiFlash_DmaCmd(SPICMD_RDID, DMA_TARGET_CHIP, 0x4D0, 3, DMA_OPT_NONE);
		ret=SpiFlash_wait_done(10,10);
		if(!ret) {
			SPI_CmdBuffer[0] = ReadTW88(REG4D0);
			SPI_CmdBuffer[1] = ReadTW88(REG4D1);
			SPI_CmdBuffer[2] = ReadTW88(REG4D2);

			Printf(" %02bx %02bx %02bx ", SPI_CmdBuffer[0], SPI_CmdBuffer[1],SPI_CmdBuffer[2]);
			if     (SPI_CmdBuffer[0]==SPIFLASH_MID_MX) {
				Puts("Macronix"); 	
				print_spiflash_status_register = &print_spiflash_status_register_macronix;
			}
			else if(SPI_CmdBuffer[0]==SPIFLASH_MID_EON) {
				Puts("EOn");
				print_spiflash_status_register = &print_spiflash_status_register_eon_256;
			}
			else if(SPI_CmdBuffer[0]==SPIFLASH_MID_WB) {
				Puts("Winbond");
				print_spiflash_status_register = &print_spiflash_status_register_winbond;
			}
			else if(SPI_CmdBuffer[0]==SPIFLASH_MID_MICRON) {
				Puts("Micron");
				print_spiflash_status_register = &print_spiflash_status_register_micron;
			}
			else if(SPI_CmdBuffer[0]==SPIFLASH_MID_SPANSION) {
				Puts("Spansion");
				print_spiflash_status_register = &print_spiflash_status_register_unknown;
			}
			else if(SPI_CmdBuffer[0]==SPIFLASH_MID_GIGA) {
				Puts("Giga");
				print_spiflash_status_register = &print_spiflash_status_register_unknown;
			}
			else {
				Puts("Unknown");
				print_spiflash_status_register = &print_spiflash_status_register_unknown;
			}
		}
	}
	else if(cmd == SPICMD_WREN 		/*spic 6 */
	     || cmd == SPICMD_WRDI) {	/*spic 4 */
		if(cmd == SPICMD_WRDI) Puts("\n\rWRDI ");
		else				   Puts("\n\rWREN ");
		SpiFlash_DmaCmd(cmd, DMA_TARGET_CHIP, 0x4D0, 0, DMA_OPT_NONE);
		ret=SpiFlash_wait_done(10,10);
	}
	else if(cmd == SPICMD_RDSR) {
		if(read_byte ==0) {
			Puts("\nuse spic r1 5");
			return;
		}
		SpiFlash_DmaCmd(cmd, DMA_TARGET_CHIP, 0x4D0, read_byte, DMA_OPT_NONE);
		ret=SpiFlash_wait_done(10,10);
		for(i=0; i < read_byte; i++)
			Printf(" %02bx", ReadTW88(REG4D0+i)); 
	}
	else if(cmd == SPICMD_WRSR) {
		if( argc< (index+4) ) {
			Printf("\n\rMissing Parameters !!!" );
			return;
		}
		SPI_CmdBuffer[0] = a2h(argv[index+2]);
		SPI_CmdBuffer[1] = a2h(argv[index+3]);

		SpiFlash_DmaCmd(cmd, DMA_TARGET_CHIP | 2, 0x4D0, 0, SPI_CMD_OPT_BUSY);
		ret=SpiFlash_wait_done(10,10);
		ret=SpiFlash_check_busy(10,10);
	}
	else if(cmd == SPICMD_EN4B
	||      cmd == SPICMD_EX4B) {
		ret = SpiFlash_4B_DmaCmd(cmd);
	}
	else if(cmd == SPICMD_PP) {
		if( argc< (index+4) ) {
			Printf("\n\rMissing Parameters !!!" );
			return;
		}
		if( argc > (index+11) ) {
			Printf("\n\ronly support 8 bytes !!!" );
			argc = 11;
		}
		spiaddr = a2h( argv[index+2] );
		Printf("\n\rPP ");
		PrintSpiAddr(spiaddr);



		//BKFYI140819. PP with QuadIO has a problem on REG4D0.
		//use XMEM
		for(i=3,cnt=0; i <argc; i++,cnt++) {
			dat0 = a2h(argv[index+i]);
			SPI_Buffer[cnt]=dat0;
		}
		ret=SpiFlash_PageProgram_XMem(spiaddr,0,argc);
	}
	else if(cmd == SPICMD_SE || cmd == SPICMD_BE) {
		if( argc< (index+3) ) {
			Printf("\n\rMissing Parameters !!!" );
			return;
		}
		spiaddr = a2h( argv[index+2] );
		if(cmd == SPICMD_BE) Puts("\n\rBE ");
		else				 Puts("\n\rSE ");
		PrintSpiAddr(spiaddr);

		if(cmd == SPICMD_BE)
			ret = SpiFlash_BlockErase(spiaddr);
		else
			ret = SpiFlash_SectorErase(spiaddr);
	}
	else if(cmd == SPICMD_READ_SLOW 
	     || cmd == SPICMD_READ_FAST
		 || cmd == SPICMD_READ_DUAL_O
		 || cmd == SPICMD_READ_QUAD_O 
		 || cmd == SPICMD_READ_DUAL_IO 
		 || cmd == SPICMD_READ_QUAD_IO ) {
		BYTE w_len;
		BYTE SPI_mode;
		BYTE SPI_mode_Reg;

		if( argc < 4 ) {
			Printf("\n\rMissing Parameters !!!" );
			return;
		}
		if(read_byte==0) {
			HelpMonitorSPIC();
			return;
		}
		//change SPI_mode
		SPI_mode_Reg = ReadTW88(REG4C0);
		if(cmd==SPICMD_READ_SLOW) {
			SPI_mode=0;
			w_len = 3;
			if((ReadTW88(REG4E1) & 0xC0)==0xC0) {
				Printf("\n\rEDGE CYCLE error. GiveUp!!");
				return;
			}		
		}
		else if(cmd==SPICMD_READ_FAST) {
			SPI_mode=1;
			w_len = 4;		
		}
		else if(cmd==SPICMD_READ_DUAL_O) {
			SPI_mode=2;
			w_len = 4;		
		}
		else if(cmd==SPICMD_READ_QUAD_O) {
			SPI_mode=3;
			w_len = 4;		
		}
		else if(cmd==SPICMD_READ_DUAL_IO) {
			SPI_mode=4;
			w_len = 4;		
		}
		else {
			//Printf("\n\rDualIO & QuadIO can read only 1 byte...");
			SPI_mode = 5;
			w_len = 6;		
		}


		if(SpiFlash4ByteAddr)
			 w_len++;

		spiaddr = a2h( argv[index+2] );
		PrintSpiAddr(spiaddr);

		//if(argc >= 4)
		//	cnt = a2h(argv[index+3]);
		//else cnt = 8;

		SpiFlashSetAddress2CmdBuffer(spiaddr); //SPI_CmdBuffer[0]~[2] or [0]~[3]

		WriteTW88(REG4C0, (SPI_mode_Reg & ~0x07) | SPI_mode);
		//ret = SpiFlashChipRegCmd(cmd, w_len, read_byte, SPI_CMD_OPT_BUSY, 200);
		SpiFlash_DmaCmd(cmd, DMA_TARGET_CHIP, 0x4D0, read_byte, DMA_OPT_NONE);
		ret=SpiFlash_wait_done(10,10);
		if(!ret) {
			if(cmd == SPICMD_READ_DUAL_O
			|| cmd == SPICMD_READ_QUAD_O
			|| cmd == SPICMD_READ_DUAL_IO
			|| cmd == SPICMD_READ_QUAD_IO) {
				for(i=0; i < read_byte; i++)
					Printf(" %02bx", ReadTW88(REG4D0+i));
			}
			else {
				for(i=0; i < read_byte; i++)
					Printf(" %02bx", SPI_CmdBuffer[i]);
			}
		}

		//restore SPI_Mode
		WriteTW88(REG4C0, SPI_mode_Reg);
	}
	else {
		BYTE w_len;
		if(read_byte) 
			w_len = 0; //read mode
		else {	
			w_len = argc - 2;	
		}
		for(i=0; i < w_len; i++)
			SPI_CmdBuffer[i] = a2h(argv[index+2]);
		
		//ret = SpiFlashChipRegCmd(cmd, w_len, read_byte, SPI_CMD_OPT_NONE, 200);
		SpiFlash_DmaCmd(cmd, DMA_TARGET_CHIP | w_len, 0x4D0, read_byte, DMA_OPT_NONE);
		ret=SpiFlash_wait_done(10,10);
		if(!ret) {
			for(i=0; i < read_byte; i++)
				Printf(" %02bx",ReadTW88(REG4D0+i));
		}
	}
}

#define BT656ENC_SRC_DEC			0
#define BT656ENC_SRC_ARGB			1
#define BT656ENC_SRC_DTV			2
#define BT656ENC_SRC_LVDS			3
#define BT656ENC_SRC_PANEL			4
#define BT656ENC_SRC_OFF			5
#define BT656ENC_SRC_AUTO			6

#define BT656_INPUT_CVBS		0
#define BT656_INPUT_SVIDEO		1
#define BT656_INPUT_COMP		2
#define BT656_INPUT_PC			3
#define BT656_INPUT_DVI			4
#define BT656_INPUT_HDMIPC		5	//RGB
#define BT656_INPUT_HDMITV		6	//YUV
#define BT656_INPUT_LVDS		7
#define BT656_INPUT_PANEL		8


void EE_DataInfomation(void)
{
	Puts("\n0x00~0x05	Tag & FW Rev. -DO NOT WRITE-");
	Puts("\n0x06		DebugLevel 0~3");
	Puts("\n0x0F		BootMode 0:Normal 1:RCD");
	Puts("\n0x10		Main Input 0:CVBS,2:COMP,3:PC,5:HDMI,6:BT656,7:LVDS");
	Puts("\n0x20		BT656Enc Input 0:CVBS,2:COMP,3:PC,5:HDMI,7:LVDS,8:Panel");
	Puts("\n0x21		BT656Enc CVBS 0:Normal 1:2DDI");
	Puts("\n0x22		BT656Enc COMP 0:Normal..");
	Puts("\n0x23		BT656Enc PC   0:Normal..");
	Puts("\n0x25		BT656Enc HDMI 0:Normal..");
	Puts("\n0x27		BT656Enc LVDS 0:Normal..");
	Puts("\n0x30		Boot Count");
	Puts("\n0x31		Watchdog Boot Count");
	Puts("\n0x40~0x6C	Video Image");
	Puts("\n0x80~0x93	Touch");
	Puts("\n0xA0~0xAF	Video Effect");
	Puts("\n0xB0~0xC0	OLD data");
	Puts("\n0xD0~0x1BF	PC Mode");
}
//=============================================================================
//
//=============================================================================
void HelpMonitorEE(void)
{
	Puts("\n\r\n  === Help for EE command ===");
	Puts("\n\rEE format         ; format and initialize");
	Puts("\n\rEE init           ; initialze internal variables");
	Puts("\n\rEE default        ; initialze default EE values");
	Puts("\n\rEE check          ; report map,dump,corrupt");
	Puts("\n\rEE clean          ; move & clean bank sector");
	Puts("\n\rEE repair         ; call when you have a movedone error");
	Puts("\n\rEE info           ; E3P information");
	Puts("\n\rEE w [idx] [dat]  ; write data");
   	Puts("\n\rEE r [idx]        ; read data");
	Puts("\n\rEE d              ; dump all data");
	Puts("\n\rEE data           ; data info");
	//Printf("\n\rFYI %bx:DebugLevel %bx:InputMain ",EEP_DEBUGLEVEL,EEP_INPUT_MAIN);
	Puts("\n\r");
}


//=============================================================================
//
//=============================================================================
//	Format is needed only once
//	Init is needed when starting program
#ifdef USE_SFLASH_EEPROM
void MonitorEE(void)
{
	BYTE dat; //, i, j;
	BYTE dat1;
	WORD index;

	if(argc < 2) {
		HelpMonitorEE();
		return;
	}
	else if( !stricmp( argv[1], "?" ) ) {
		HelpMonitorEE();
		return;
	}

	index = a2h( argv[2] );
	dat   = a2h( argv[3] );

	//--------------------------------------------------------
	if( !stricmp( argv[1], "format" ) ) {
		Printf("\n\rFormat EEPROM...");
		E3P_Format();
		return;
	}
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "init" ) ) {
		Printf("\n\rFind EEPROM variables...");
		//E3P_Init();
		E3P_Configure();
		return;
	}
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "repair" ) ) {
		Printf("\n\rRepair MoveDone error..call only when EE find have a MoveDone error");
		E3P_Repair();
		return;
	}
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "default" ) ) {
		Printf("\n\rEE initialize........");
		SaveFWRevEE( FWVER );
		ClearBasicEE();
		SaveDebugLevelEE(0);
		E3P_PrintInfo();
		return;
	}
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "check" ) ) {
		Printf("\n\rEE check");
		E3P_Check();
		return;
	}
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "info" ) ) {
		Printf("\n\rEE info");
		E3P_PrintInfo();
		return;
	}
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "clean" ) ) {
		Printf("\n\rEE clean blocks");
		E3P_Clean();
		return;
	}
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "W" ) ) {
		if( argc==4 ) {
			Printf("\n\rWrite EEPROM %03x:%02bx ", index, dat );
			EE_Write( index, dat );
			dat1 = EE_Read( index );  //BUG
			dat = EE_Read( index );
			Printf(" ==> Read EEPROM[%03x] = %02bx %02bx", index, dat1, dat );
		}
	}
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "R" ) ) {
		if( argc==3 ) {
			dat = EE_Read( index );
			Printf("\n\r ==> Read EEPROM[%03x] = %02bx ", index, dat );
		}
	}
	//--------------------------------------------------------
#ifdef USE_SFLASH_EEPROM
	else if( !stricmp( argv[1], "D" ) ) {
		Printf("\n\rDump EEPROM");
		E3P_DumpBlocks(0);
		//for(j=0; j<E3P_MAX_INDEX/16; j++) {
		//	Printf("\n\rEEPROM %02bx:", j*0x10);
		//	for(i=0; i<8; i++) Printf(" %02bx", EE_Read( j*16 + i ) );
		//	Printf("-");
		//	for(; i<16; i++) Printf("%02bx ", EE_Read( j*16 + i ) );
		//}
	}
#endif
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "data" ) ) {
		EE_DataInfomation();
		return;
	}
	//--------------------------------------------------------
	else
		Printf("\n\rInvalid command...");	
	
}
#endif

//=============================================================================
/**
* SPI Read Status
*
*/
static void SPI_Status(void)
{
	BYTE dat1;
	BYTE vid;
	BYTE cid;
	BYTE ret;

	//ret = SpiFlashChipRegCmd(SPICMD_RDID,0,3, SPI_CMD_OPT_NONE, 200);
	SpiFlash_DmaCmd(SPICMD_RDID, DMA_TARGET_CHIP, 0x4D0, 3, DMA_OPT_NONE);
	ret=SpiFlash_wait_done(10,10);
	SPI_CmdBuffer[0] = ReadTW88(REG4D0);
	SPI_CmdBuffer[1] = ReadTW88(REG4D1);
	SPI_CmdBuffer[2] = ReadTW88(REG4D2);

	if(ret)
		Puts("\n\rSPICMD_RDID fail");
	vid  = SPI_CmdBuffer[0];
	dat1 = SPI_CmdBuffer[1];
	cid  = SPI_CmdBuffer[2];

	Printf("\n\rJEDEC ID: %02bx %02bx %02bx", vid, dat1, cid );

	switch(vid) {
	case 0x1C:	 	Puts("\n\rEON");		break;
	case 0xC2:		Puts("\n\rMX");		break;
	case 0xEF:		Puts("\n\rWB");		if(cid == 0x18) Puts("128"); break;
	case 0x20:		Puts("\n\rMicron");	break;
	default:		Puts("\n\rUnknown");	break;
	}

	if (vid == 0xC2 || vid == 0x1c) {
		//ret=SpiFlashChipRegCmd(SPICMD_RDSR,0, 1, SPI_CMD_OPT_NONE, 200);
		SpiFlash_DmaCmd(SPICMD_RDSR, DMA_TARGET_CHIP, 0x4D0, 1, DMA_OPT_NONE);
		ret=SpiFlash_wait_done(10,10);
		SPI_CmdBuffer[0] = ReadTW88(REG4D0);
		Printf("	CMD:%02bx Data:%02bx",SPICMD_RDSR,SPI_CmdBuffer[0]);
		if(SPI_CmdBuffer[0] & 0x40)	Puts(" Quad Enabled");
	}
	else if (vid == 0xEF) {					// WB
		if(cid == 0x18) {				//Q128 case different status read command
			//ret=SpiFlashChipRegCmd(SPICMD_RDSR2,0, 1, SPI_CMD_OPT_NONE, 200);
			SpiFlash_DmaCmd(SPICMD_RDSR2, DMA_TARGET_CHIP, 0x4D0, 1, DMA_OPT_NONE);
			ret=SpiFlash_wait_done(10,10);
			SPI_CmdBuffer[0] = ReadTW88(REG4D0);
			Printf("	CMD:%02bx Data:%02bx",SPICMD_RDSR2,SPI_CmdBuffer[0]);
		}
		else {
			//ret=SpiFlashChipRegCmd(SPICMD_RDSR,0, 2, SPI_CMD_OPT_NONE, 200);
			SpiFlash_DmaCmd(SPICMD_RDSR, DMA_TARGET_CHIP, 0x4D0, 2, DMA_OPT_NONE);
			ret=SpiFlash_wait_done(10,10);
			SPI_CmdBuffer[0] = ReadTW88(REG4D0);
			SPI_CmdBuffer[1] = ReadTW88(REG4D1);
			Printf("	CMD:%02bx Data:%02bx,%02bx",SPICMD_RDSR,SPI_CmdBuffer[0],SPI_CmdBuffer[1]);
		}
	}
	else if(vid==0x20) {
		if(cid !=0x18) {
			Puts(" NEED 128b!!!");
			return;
		}
		// Volatile
 		//ret=SpiFlashChipRegCmd(SPICMD_RDVREG,0, 1, SPI_CMD_OPT_NONE, 200);	//cmd, read Volatile register
		SpiFlash_DmaCmd(SPICMD_RDVREG, DMA_TARGET_CHIP, 0x4D0, 1, DMA_OPT_NONE);
		ret=SpiFlash_wait_done(10,10);
		SPI_CmdBuffer[0] = ReadTW88(REG4D0);
		dPrintf("	Volatile Register: %02bx", SPI_CmdBuffer[0] );

		// non-Volatile
		//ret=SpiFlashChipRegCmd(SPICMD_RDNVREG, 0, 2, SPI_CMD_OPT_NONE, 200);	//cmd, read Non-Volatile register
		SpiFlash_DmaCmd(SPICMD_RDNVREG, DMA_TARGET_CHIP, 0x4D0, 2, DMA_OPT_NONE);
		ret=SpiFlash_wait_done(10,10);
		SPI_CmdBuffer[0] = ReadTW88(REG4D0);
		SPI_CmdBuffer[1] = ReadTW88(REG4D1);
		dPrintf("	Non-Volatile Register: %02bx, %02bx", SPI_CmdBuffer[0], SPI_CmdBuffer[1] );
	}
}

//=============================================================================
/**
* read and dump SPIFLASH data
*/
void SPI_dump(DWORD spiaddr) 
{
	BYTE *ptr = SPI_Buffer;
	DWORD cnt = 0x80L;
	BYTE i, j, c;

	SpiFlash_Read_XMem(spiaddr, (WORD)SPI_Buffer, cnt);
	//SpiFlashDmaRead2XMem(SPI_Buffer, spiaddr, cnt, SPIFLASH_WAIT_READ);  //same SpiFlashDmaRead 

	for (j=0; j<8; j++) {
		Printf("\n\rSPI %06lx: ", spiaddr + j*0x10);
		for(i=0; i<8; i++) Printf("%02bx ", SPI_Buffer[j*0x10+i] );
		Printf("- ");
		for(; i<16; i++) Printf("%02bx ", SPI_Buffer[j*0x10+i] );
		Printf("  ");
		for(i=0; i<16; i++) {
			c = SPI_Buffer[j*0x10+i];
			if( c>=0x20 && c<0x80 ) Printf("%c", c);
			else Printf(".");
		}
	}
}

