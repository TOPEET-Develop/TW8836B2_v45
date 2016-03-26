/*
 I2cSpi.h
*/

#define DMAREG_4D0  0xD0
#define DMAREG_4D8  0xD8
#define DMAREG_4D9  0xD9
#define DMAREG_4F0	0xF0
#define DMAREG_4F1	0xF1
#define DMAREG_4F2	0xF2
#define DMAREG_4F3	0xF3
#define DMAREG_4F4	0xF4
#define DMAREG_4F5	0xF5
#define DMAREG_4F6	0xF6
#define DMAREG_4F7	0xF7
#define DMAREG_4F8	0xF8
#define DMAREG_4F9	0xF9
#define DMAREG_4FA	0xFA
#define DMAREG_4FB	0xFB
#define DMAREG_4FC	0xFC
#define DMAREG_4FD	0xFD
#define DMAREG_4FE	0xFE
//-------------
	#define I2C8REG4F0	0xF0
	#define I2C8REG4F1	0xF1
	#define I2C8REG4F2	0xF2
	#define I2C8REG4F3	0xF3
	#define I2C8REG4F4	0xF4
	#define I2C8REG4F5	0xF5
	#define I2C8REG4F6	0xF6
	#define I2C8REG4F7	0xF7
	#define I2C8REG4F8	0xF8
	#define I2C8REG4F9	0xF9
	#define I2C8REG4FA	0xFA
	#define I2C8REG4FB	0xFB
	#define I2C8REG4FC	0xFC
	#define I2C8REG4FD	0xFD
	#define I2C8REG4FE	0xFE

extern BYTE I2CSPI_4B_mode;    /* SPI 4Byte address mode */
extern BYTE I2CSPI_mid;        /* Manufactory ID */
extern BYTE I2CSPI_size;       /* memory density */


//BYTE I2cSpiFlashDmaWait(BYTE wait);
//BYTE I2cSpiFlashDmaWait_loop(BYTE dma_option, BYTE wait5ms);
BYTE I2cSpiFlashChipRegCmd(BYTE cmd, BYTE cmd_buff_len, BYTE data_buff_len, BYTE dma_option, BYTE wait);
void I2cSpiFlashSetAddress2CmdBuffer(DWORD spiaddr);

//new
void I2CSPI_SW_reset(void);
void I2CSPI_LV_reset(void);
BYTE I2CSPI_mcu_halt_rerun(BYTE fHalt);

BYTE I2cSpiFlash_4B_DmaCmd(BYTE cmd);

BYTE I2CSPI_IsCmdAccepted(BYTE option, BYTE loop, BYTE delay, BYTE debug);
BYTE I2CSPI_wait_busy_with_rdsr(BYTE old_cmd, BYTE loop, BYTE base, BYTE debug);
BYTE I2CSPI_ReadId_to_chipreg(void);
BYTE I2CSPI_WriteEnable(void);
BYTE I2CSPI_SectorErase(DWORD spi_addr);
BYTE I2CSPI_BlockErase(DWORD spi_addr);

BYTE I2CSPI_check_crc(DWORD spiaddr, DWORD length, 	BYTE loop, WORD crc);
void I2CSPI_test_big_dma(void);

//BYTE I2CSPI_upload(DWORD src_addr, DWORD dest_addr, DWORD upload_len);
//BYTE I2CSPI_upload_faster(DWORD src_addr, DWORD dest_addr, DWORD upload_len);
BYTE I2CSPI_download_main(DWORD src_addr, DWORD dest_addr, DWORD upload_len);
BYTE I2CSPI_xcopy_main(DWORD src_addr, DWORD upload_len);



