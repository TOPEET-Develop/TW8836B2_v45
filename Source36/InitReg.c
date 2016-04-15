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

#include "Global.h"
#include "CPU.h"
#include "Printf.h"
#include "util.h"
#include "Monitor.h"

#include "I2C.h"
#include "SPI.h"

#include "main.h"
#include "SOsd.h"
#include "FOsd.h"
#include "Scaler.h"
#include "decoder.h"
#include "InputCtrl.h"
#include "EEPROM.h"
#include "ImageCtrl.h"
#include "decoder.h"
#include "InputCtrl.h"
#include "Settings.h"
#include "measure.h"
#include "BT656.h"


//typedef struct REG_IDX_DATA_s {
//	WORD idx;
//	BYTE value;
//} REG_IDX_DATA_t;

/*=====================================================
*	Default HW register value.
*   !!!DO NOT CHANGE.!!!
*======================================================*/
code REG_IDX_DATA_t InitHwDefault_Table[] = {
					//-----------------
					//clock
					//use 27MHz. SYNC.
					//-----------------

	{0x4DF, 0x00},	//ASync control

	{0x4E0, 0x00},	//ASyncWait:0,PLLCLK:0:SSPLL
	{0x4E1, 0x06},	//-Edge,0_delay,27MHz,PLLCLK_DIV:6:div4.

	{0x4F0, 0x00},	//MCU_DIV:0:div1, MCU_WAIT_OFF:0:Note
	{0x4F2, 0x00},	//MCU_SYNC,DMA_SYNC

					//-----------------
					//SSPLL2
	{0x0E7, 0x16},	//
	{0x0E8, 0x01},	//60.750MHz
	{0x0E9, 0x20},	//
	{0x0EA, 0x00},	//
	{0x0EB, 0x40},	//FSS2	
	{0x0EC, 0xB0},	//PD_SSPLL2.Note
	{0x0ED, 0x11},	//POST:0,VCO:27~54,pump:2.5

					//-----------------
	{0x0F6, 0x04},	//PCLK_DIV:4:div3.

					//-----------------
					//SSPLL1
	{0x0F7, 0x16},	//
	{0x0F8, 0x01},	//60.750MHz
	{0x0F9, 0x20},	//
	{0x0FA, 0x00},	//
	{0x0FB, 0x40},	//FSS
	{0x0FC, 0xB0},	//PD_SSPLL1.Note
	{0x0FD, 0x11},	//POST:0,VCO:27~54,pump:2.5

	//FYI. before 131114, FW used below values.
	//{0x0F6, 0x00},
	//{0x0FC, 0x23},



	//REG000		ID def:36		TW8836
	//REG001		REV def:11		B2		    
					//-----------------
					//interrupt
#if 0 //InitCore already updates it. Do not check this two registers.
	{0x003, 0xFF},	//irq mask first.  def:FF
	{0x002, 0xFF},  //and then clean.  def:CA
#endif


	//REG004		STATUS readonly
	//REG005		IRQ&MASK def:F0
	{0x006, 0x00},	//SRST def:00
					//[7] SRST. SW chip reset. Do not execute it with SSPLL1.
					//[6] SwapBT656_OUT 1:swap
					//[5] i2c auto idx increase. 1:disable
					//[4:0] TCon output
					//BKTODO131125 :move to InitSwDefault_Table[] 
#ifdef	PANEL_FP_LSB
	{0x007,	0x02},	//
#else
	{0x007,	0x00},	//OUTPUT CTRL I. def:00
#endif
					//[7] FP data port swap R<->B
					//[6] swap MSB and LSB
					//[5:4]	FP data port bit shifting
					//[3] En BT656Enc interface
					//[2:0] TCON pin output mode
	{0x008,	0x30},	//Output CTL II def:30
					//[7:6] TCCLK driver strength. TCON uses 8mA. cfg file error
					//		00:0 01:4mA 10:8mA 11:12mA
					//      TCON only.
					//[5] TRI_FPD 	1:Tristate all FP data pins
					//[4] TRI_EN	1:Tristate all output pins
					//[3:0] GPO pin mode
					//		TW8836 EVB uses GPOSEL=tcpol, because REMO uses INT11(TCPOLP).
	//REG009		  for FW I2C Server
	//REG00F		  INT0 Write Port
	{0x01F, 0x00},	//test mode def:00

					//-----------------
	{0x040,	0x00},	//input control I. def:00
					//[7:6] hStart[9:8]
					//[5]	VDCLK_POL
					//[4]	Scaler Input Clk Pol
					//[3]   En DTVDE
					//[2]   En DTVCLK2 for BT656Loop
					//[1:0] Input Selection
	{0x041,	0xC0},	//input control II, def:C0 FW uses 0x00
					//[7] DTV input threshold 0:1.8V 1:3.3V DEF:1 
					//[6] BT656 input threshold 0:1.8V 1:3.3V DEF:1
					//[5]	Field control for progressive input
					//[4] Implicit DE => Explicit DE
					//[3] vSync Pol
					//[2] hSync Pol
					//[1] field pol
					//[0] data format 0:YCbCr 1:RGB


					//-----------------
					//input crop
					//def 	hStart 0x020 = 32
					//		hLen   0x2D0 = 720
					//		vStart 0x20	 = 32
					//		vLen   0x0F0 = 240
	{0x042,0x02},	//	  def:02
	//				  [7] Enable P10 pin(#60) as MUTE pin input
	//                [6:4] vLen[10:8]
	//				  [3:0] hLen[11:8]
	//0x043			  [7:0] vStart[7:0]
	//0x044			  [7:0] vLen[7:0], with REG042[6:4]
	//0x045			  [7:0] hStart[7:0], with REG040[7:6]
	//0x046			  [7:0] hLen[7:0], with REG042[3:0]
	{0x047, 0x00},	//BT656Dec CTRL I def:00
					//[7] internal pattern
					//[6] 1=50Hz
					//[5] DTV input clock control.
					//[4:0] BT656Dec input V delay.
	//REG048          BT656 Dec CTRL II def:00
	//REG04B		  VSYNC input cnotrol def:00
					//[7] En delay. BKTODO:Unknown.
					//[6] UART1_SWAP. swap UART1 pins form 120,121 to 60,61.
					//[5] SPI clk PLL select. 0:SSPLL1, 1:SSPLL2
					//[4] PCLK PLL select. 0:SSPLL2,1:SSPLL1
					//[3:0] VSync input delay
					//-----------------
					//DTV
	//REG050~REG056
	{0x057, 0x00},	//SEQUENCUAL RGB def:00
					//-----------------
	{0x05F, 0x00},	// test pattern def:00

	//REG060~REG061
	//REG062~REG06F BT656
	//REG077~REG07F

					//-----------------
					//GPIO
	{0x080, 0x00},
	{0x081, 0x00},
	{0x082, 0x00},
	{0x083, 0x00},
	{0x084, 0x00},
	{0x085, 0x00},
	{0x086, 0x00},
	{0x088, 0x00},
	{0x089, 0x00},
	{0x08A, 0x00},
	{0x08B, 0x00},
	{0x08C, 0x00},
	{0x08D, 0x00},
	{0x08E, 0x00},
	{0x090, 0x00},
	{0x091, 0x00},
	{0x092, 0x00},
	{0x093, 0x00},
	{0x094, 0x00},
	{0x095, 0x00},
	{0x096, 0x00},
					//-----------------
	{0x0A0, 0x00},	//MBIST
					//-----------------
					//TSC
	{0x0B0, 0x87},	//TSC CTRL I. def:87
					//[7] PD
					//[2:0] TSC mode
	{0x0B1, 0x00},	//TSC CTRL II. def:00
					//[7] 1:disable Ready Interrupt
					//[6] 1:disable Pen Interrupt
					//[5:3] Tsc sensitivity R selection.
					//		FYI. InitAux() updates it as 10K.
					//[2:0] ADC test mode
	//REG0B2 		  TSC DATA HI
	//				  [7:0] TSC_ADOUT[11:4]
	//REG0B3 		  TSC DATA LO
	//				  [3:0] TSC_ADOUT[3:0]
	{0x0B4, 0x00},	//TSC ADC SAMPLE & Clock. def:00
					//[7:4] reserved
					//[3] 1:continus sampling.
					//[2:0] TSC_ADC clock
					//	    FYI. InitAux() updates [3] and [2:0]. and uses div8.
					//-----------------
	{0x0D4, 0x00},	//LOPOR def:00
					//-----------------					
	{0x0D6, 0x00},	//PWM def:00
	//REG0D7		  def:00			
	//REG0D8		  def:80			
	//REG0D9		  def:00			
	//REG0DA		  def:80			
	//REG0DB		  def:55			
	//REG0DC		  def:00			
	//REG0DD		  def:80			
	//REG0DE		  def:00			
	//REG0DF		  def:80			

	//				SSPLL2
	//					SSPLL2:60.75Mhz,PCLK:20.25MHz
	//REG0E7		  def:16
	//REG0E8		  def:01
	//REG0E9		  def:20
	//REG0EA		  def:00
	//REG0EB		  def:40
	//REG0EC		  def:B0
	//REG0ED		  def:11
	//REG0F6		CLOCK_DIV def:04
	//				SSPLL1
	//					SSPLL1:60.75MHz,PLLCLK:15.1875MHz,SPICLK:27MHz,MCUCLK:27MHz
	//REG0F7		  def:16
	//REG0F8		  def:01
	//REG0F9		  def:20
	//REG0FA		  def:00
	//REG0FB		  def:40
	//REG0FC		  def:B0
	//REG0FD		  def:11

					//-----------------
					//decoder
	//REG101		  CSTATUS readonly
	{0x102, 0x40},	//INFORM. def:40
					//[7] CSEL_HI
					//[6] 1:27MHz 0:Square pixel, 60Hz:24.54MHz, 50Hz:29.5MHz.
					//[5:4]
					//[3:2]
					//[1] CSEL_LO
					//[0] VSEL
	//REG103		  LLC mode. def:00 read:24
	//				  [5] 1:Enable LLCMode
	{0x104, 0x00},	//hSync Delay  def:00
					//[6:5] Color Killer time constant. 0:Fast..3:Slow
	{0x105, 0x00},	//BT656Enc output ctrl II. def:00
	{0x106, 0x00},	//ACNTL def:00
					//..
					//[2] Y_PDN
					//[1] C_PDN
					//[0] V_PDN
					//BKTODO131125. def was inforrect. and disable [2] on HDMI.
					//---------------------------
					//decoder output crop				Normal			OverScan		
					//					  def   table	NTSC	PAL		NTSC	PAL
					//    hDelay: 0x00A  = 10	10		10 		7		8		6
					//    hActive:0x2D0	= 720	720		720		720		716		716
					//    vDelay: 0x012	= 18	18		20		24		21		27
					//    vActive:0x120 = 288	240		241		288		238		285
//	{0x107, 0x12},	//DEC Crop Hi. def:12
//					//[7:6] vDelay_hi
//					//[5:4] vActive_hi
//					//[3:2] hDelay_hi
//					//[1:0] hActive_hi
	{0x108, 0x12},	//vDelay def:12
//	{0x109, 0x20},	//vActive def:20
//	{0x10A, 0x0A},  //hDelay def:0A
	{0x10B, 0xD0},  //hActive def:D0

	{0x10C, 0xCC},	//CTRL1 def:CC
	{0x10D, 0x00},	//SS/WSS CTRL. def:15 read:00
					//[4:0] closed caption decoding line number on odd field.
					//color
	{0x110, 0x00},	//Brightness def:00
	{0x111, 0x5C},	//contrase def:5C
	{0x112, 0x11},	//sharpness def:11
	{0x113, 0x80},	//chroma SAT_U def:80
	{0x114, 0x80},	//chroma SAT_V def:80
	{0x115, 0x00},  //hue def:00
	{0x117, 0x80},	//vertical peak def:40 read:80
					//[7:4] coring for sharpness control
					//[2:0] vertical peaking control
					//FYI FW uses 0x30
	{0x118, 0x44},	//corning def:44
	{0x11A, 0x00},	//SS_STATUS def:00 read:0x10
					//[6]
					//[5]
					//other readonly
	//REG11B		CC_DATA readonly
	{0x11C, 0x07},	//SDT  def:07
					//[3]
					//[2:0]
					//other readonly
	{0x11D, 0x7F},	//SDTR def:7F
	{0x11E, 0x00},	//comp video format def:00
					//[3:0]
					//other readonly
	{0x120, 0x50},	//def:50
	{0x121, 0x22},	//def:22
	{0x122, 0xF0},	//def:F0
	{0x123, 0xD8},	//def:D8
	{0x124, 0xBC},	//def:BC
	{0x125, 0xB8},	//def:B8
	{0x126, 0x44},	//def:44
	{0x127, 0x38},	//def:38
	{0x128, 0x00},	//def:00
	{0x129, 0x00},	//def:00
	{0x12A, 0x78},	//def:78
	{0x12B, 0x44},	//def:44
	{0x12C, 0x30},	//def:30
	{0x12D, 0x14},	//def:14
	{0x12E, 0xA5},	//def:A5
	{0x12F, 0xE0},	//def:E0
	//REG131 readonly
	//REG132 readonly
	{0x133, 0x05},  //def:05
	{0x134, 0x1A},	//def:1A
	{0x135, 0x00},  //def:00
	//REG140 readonly
	//REG141 readonly
	//REG142 readonly

					//-----------------
				 	//LLPLL
	{0x1C0, 0x00},	//LLPLL input ctrl def:00	
	//REG1C1		  LLPLL input detection, readonly
	{0x1C2, 0x01},	//LLPLL CTRL def:01. FW uses D2
	{0x1C3, 0x03},	//LLPLL DIV def:03
	{0x1C4, 0x5A},	//LLPLL DIV def:5A
	{0x1C5, 0x00},	//LLPLL Clock Phase def:00
	{0x1C6, 0x20},	//LLPLL Loop ctrl def:20
	{0x1C7, 0x04},	//LLPLL vco def:04
	{0x1C8, 0x00},	//LLPLL vco def:00
	{0x1C9, 0x06},	//LLPLL PreCoast def:06
	{0x1CA, 0x06},	//LLPLL PostCoast def:06
	{0x1CB, 0x30},	//SOG Threshold def:30
					//	COMP starts from 0xD6
					//	PC   starts from 0x56
					//[7] SOG power down. 1:Power up
					//[6] PLL power down. 1:Power up
					//[5] PLL coast control. 1:Enable
					//[4:0] SOG slicer threshold
	{0x1CC, 0x00},	//Scaler Sync Selection. def:00
	{0x1CD, 0x54},	//PLL init. def:54

	{0x1D0, 0x00},	//CLAMP gain control def:00
	{0x1D1, 0xF0},	//Y gain def:F0
	{0x1D2, 0xF0},	//C gain def:F0
	{0x1D3, 0xF0},	//V gain def:F0
	{0x1D4, 0x00},	//clamp mode control def:00 FW uses 0x20. rising.
					//[5] ref edge
	{0x1D5, 0x00},	//clamp start position def:00
	{0x1D6, 0x10},	//clamp stop position def:10
	{0x1D7, 0x70},	//clamp master location def:0x70 FW starts from 0x00 and changes that depend on mode.
	{0x1D8, 0x00},	//ADC Test def:00
	{0x1D9, 0x04},	//Y clamp ref. def:04 FW uses 0x02
	{0x1DA, 0x80},	//C clamp ref. def:80
	{0x1DB, 0x80},	//V clamp ref. def:80
	{0x1DC, 0x20},	// def:0x20	FW uses 0x10.
					//[7] Edge Select
					//[5:0] Output HS width in number of output clocks
	{0x1E0, 0x00}, 	//LLPLL CTRL def:00
	{0x1E1, 0x05},	//LLPLL CTRL def:05 Note:1E1[5]:GPLL_PD
	{0x1E2, 0xD9},	//ADC CTRL I def:59 or D9
	{0x1E3, 0x07},	//ADC CTRL II def:87 read:0x07 DEC:0x07, aREG:0x37. error on datasheet
					//[7] Bias current 0:Normal 1=half
					//[6:4] Bias Control 0=10uA(Dec) 3=40uA(RGB)
					//[3:0]	Clamp current control
	{0x1E4, 0x33},	//ADC CTRL III def:33 DEC:0x33, aREG:0x55
	{0x1E5, 0x31},	//ADC CTRL IV	def:31 DEC:0x33, aRGB:0x55
	{0x1E6, 0x00},	//ADC CTRL V	def:00 DEC:0x00, aRGB:0x20
	{0x1E7, 0x2A},	//ADC CTRL VI	def:2A
	{0x1E8, 0x01},	//ADC CTRL VII	def:01	DEC:0x0F, aRGB:0x00
	{0x1E9, 0x00},	//CLOCK CTRL I	def:00
	{0x1EA, 0x03},	//CLOCK CTRL II	def:03

	{0x1F6, 0x30},  //DIFF CLAMP CTRL I		def:30 On CM_SLEEP.
	//REG1F7		  DIFF CLAMP CTRL II	def:00
	//REG1F8		  DIFF CLAMP CTRL III	def:00
	//REG1F9		  DIFF CLAMP CTRL IV	def:00
	//REG1FA		  DIFF CLAMP CTRL V		def:38
					//-----------------
					//scaler
	{0x201, 0x00},	//general scaler ctrl	def:00	
	{0x202, 0x20},	//SCALING OFFSET		DEF:20
	{0x203, 0x00},	//XScale_Lo				def:00
	{0x204, 0x20},	//XScale_hi				def:20
	{0x205, 0x00},	//YScale_lo				def:00
	{0x206, 0x20},	//YScale_hi				def:20
	{0x207, 0x80},	//PXScale				def:80 FW uses 0x40
	{0x208, 0x10},	//PXInc_Lo				def:10 FW uses 0x20
	{0x209, 0x00},	//HDScale_Lo			def:00
	{0x20A, 0x04},	//HDScale_hi			def:04
	{0x20B, 0x30},	//HDelay2				def:30 FW uses 0x10(16).
	{0x20C, 0xD0},	//HACTIVE2 withREG20E[6:4]		def:D0	
	{0x20D, 0x00},	//LNTT_HI def:00		see InitSwDefault_Table[]
	{0x20E, 0x20},	//HPADJ_HI		def:20
					//[7] 
					//[6:4] HACTIVE2_HI
					//[3:0]	HPADJ_HI
	{0x20F, 0x00},	//HPADJ_LO  def:00 FW uses 0x02

	{0x210, 0x10},	//HA_POS_LO with REG221[5:4] def:10 FW uses 0x30. it should be REG20B[]+0x20.
	{0x211, 0x00},	//HA_LEN_LO with def:00. depend on PANEL_H
	{0x212, 0x03},	//HA_LEN_Hi def:03
	{0x213, 0x10},	//HA_POS_Lo with def:10
	{0x214, 0x20},	//HS_LEN def:20
	{0x215, 0x20},	//def:20
	{0x216, 0x00},  //def:00
	{0x217, 0x03},  //def:03
	{0x218, 0x00},	//def:00
	{0x219, 0x00},	//def:00
	{0x21A, 0x00},	//def:00
	{0x21B, 0x00},	//def:00
	{0x21C, 0x00},  //def:40 read:0x00  BUGBUG130327 need manual.
	{0x21D, 0x00},	//def:00
	{0x21E, 0x00},	//def:00
	{0x220, 0x00},	//def:00
	{0x221, 0x00},	//def:00

					//-----------------
					// scaler output timing. for No video at first time.
					//-----------------

					//TCON
	{0x240, 0x10},	//def:10
	{0x241, 0x00},	//def:00
	{0x242, 0x01},	//def:01
	{0x243, 0x00},	//def:00
	{0x244, 0x00},	//def:00
	{0x245, 0x01},	//def:01
	{0x246, 0x00},	//def:00
	{0x247, 0x00},	//def:00
	{0x248, 0x01},	//def:01
	{0x249, 0x10},	//def:10
	{0x24A, 0x00},	//def:00
	{0x24B, 0x00},	//def:00
	{0x24C, 0x10},	//def:10
	{0x24D, 0x80},	//def:80
	{0x24E, 0x00},	//def:00
	//REG24F		  def:00
	//REG250		  def:00
	//REG251		  def:00
					//-----------------
					//Image Adjustment
	{0x280, 0x20},	//def:20
	{0x281, 0x80},	//def:80
	{0x282, 0x80},	//def:80
	{0x283, 0x80},	//def:80
	{0x284, 0x80},	//def:80
	{0x285, 0x80},	//def:80
	{0x286, 0x80},	//def:80
	{0x287, 0x80},	//def:80
	{0x288, 0x80},	//def:80
	{0x289, 0x80},	//def:80
	{0x28A, 0x80},	//def:80
	{0x28B, 0x30},	//def:30  FW uses 0x40
	{0x28C, 0x00},	//def:00
					//-----------------
					//Test Pattern
	{0x2BF, 0x00},
					//-----------------
					//Gamma
	{0x2E0, 0x00}, 
					//-----------------
					//Dither option
	{0x2E4, 0x00},	//def:00  FW starts from 0x21
					//-----------------
					//8bit PANEL Interface
	{0x2F8, 0x00},
	{0x2F9, 0x80},
					//-----------------
					//FOSD
// Font download have to be exec when 
//	{0x30C, 0x40},
	{0x30C, 0x00},	//turn on FOSD
	{0x310, 0x00},	//diable WIN1..WIN8
	{0x320, 0x00},
	{0x330, 0x00},
	{0x340, 0x00},
	{0x350, 0x00},
	{0x360, 0x00},
	{0x370, 0x00},
	{0x380, 0x00},
					//-----------------
					//SOSD
	{0x400, 0x00},
					//-----------------
					//LVDS Tx
	{0x640, 0x00},
	{0x641, 0x00},
	{0x642, 0x00},
	{0x643, 0x00},
	{0x644, 0x00},
	{0x647, 0x00},
					//-----------------
					//LVDS Rx
	{0x648, 0x00},	//def:01, read:00
	{0x649, 0x00},
	{0x64A, 0x00},
	{0x64B, 0x00},
	{0x64C, 0x00},
	{0x64D, 0x00},
	{0x64E, 0x00},
	{0x64F, 0x00},
					//-----------------
	{0xFFF, 0xFF}	//EOF
};


/*=====================================================
*	Software Default register value.
*   If you need to change the default, modify below table.
*======================================================*/

#if defined(PANEL_1280X800) && defined(PANEL_LVDS)
code REG_IDX_DATA_t InitSwDefault_Table[] = {

	{0x007, 0x10},				//PANEL_AUO_B133EW01
								//It is a LVDS, so dont care Output.
								//but, it needs 2bit shift down.
	{0x040, 0x20},  //VDCLK_POL=1, High Active.
	{0x041,	0x00},	//input control II, def:C0
	{0x0B1, 0xC0},	//TSC CTRL II. def:00
	{0x106, 0x03},	//ACNTL def:00
	{0x107, 0x02},	//DEC Crop. vActive 288 to 240
	{0x109, 0xF0},	
	{0x10A, 0x0B},  //hDelay def:0A
	{0x117, 0x30},	//vertical peak def:40 read:80, FW uses 0x30
	{0x1C2, 0xD2},	//LLPLL CTRL def:01
 	{0x1CB, 0x16},	//SOG Threshold def:30 COMP:0xD6 PC:0x56
	{0x1D4, 0x20},	//clamp mode control def:00 FW uses 0x20. rising.
	{0x1D7, 0x00},	//clamp master location def:0x70 FW starts from 0x00.
	{0x1D9, 0x02},	//Y clamp ref. def:04 FW uses 0x02
	{0x1DC, 0x10},	// def:0x20	FW uses 0x10.
	{0x1E3, 0x37},	//ADC CTRL II def:87 read:0x07 DEC:0x07, aREG:0x37. error on datasheet
	{0x1E4, 0x55},	//ADC CTRL III def:33 DEC:0x33, aREG:0x55
	{0x1E5, 0x55},	//ADC CTRL IV	def:31 DEC:0x33, aRGB:0x55	FW BUG.DEC:0x31	BKTODO131126
	{0x1E6, 0x20},	//ADC CTRL V	def:00 DEC:0x00, aRGB:0x20
	{0x1E8, 0x20},	//ADC CTRL VII	def:01	DEC:0x0F, aRGB:0x00
	{0x1F6, 0xB0},  //DIFF CLAMP CTRL I		def:30 On CM_SLEEP.


	{0x205, 0x00},	//??
	{0x206, 0x10},	//it assume 480i ???

	{0x207, 0x40},	//PXScale		def:80 FW uses 0x40
	{0x208, 0x20},	//PXInc_Lo		def:10 FW uses 0x20
	{0x20B, 0x10},	//HDelay2		def:30 FW uses 0x10(16).

	{0x20D, 0x92},	//temp....
	

	{0x20F, 0x02},	//HPADJ_LO  	def:00 FW uses 0x02

	{0x210, 0x30},	//HA_POS_LO with REG221[5:4] def:10 FW uses 0x30. it should be REG20B[]+0x20.
	{0x211, 0x00},	//(PANEL_H==1280)
	{0x212, 0x05},
	{0x213, 0x00},	//HA_POS_Lo with def:10
	{0x214, 0x0A},	//HS_LEN def:20
	{0x216, 0x20},	//(PANEL_V==800)
	{0x217, 0x03},
	{0x219, 0x27},	//def:00

	{0x21C, 0x42},  //def:40 read:0x00  BUGBUG130327 need manual.
	{0x21D, 0x3E},	//def:00

	{0x21E, 0x02},	//def:00

					//TCON
	{0x240, 0x10},	//def:10
	{0x241, 0x00},	//def:00
	{0x242, 0x05},	//def:01
	{0x243, 0x01},	//def:00
	{0x244, 0x64},	//def:00
	{0x245, 0xF4},	//def:01
	{0x246, 0x00},	//def:00
	{0x247, 0x0A},	//def:00
	{0x248, 0x36},	//def:01
	{0x249, 0x10},	//def:10
	{0x24A, 0x00},	//def:00
	{0x24B, 0x00},	//def:00
	{0x24C, 0x00},	//def:10
	{0x24D, 0x44},	//def:80
	{0x24E, 0x04},	//def:00
	//REG24F		  def:00
	//REG250		  def:00
	//REG251		  def:00

	{0x28B, 0x40},	//def:30  FW uses 0x40
//#if defined(PANEL_FORMAT_666)
	{0x2E4, 0x21},	//def:00  FW starts from 0x21
//#endif
	{0x648, 0x01},	//def:01, read:00

	{0xFFF, 0xFF}	//EOF
};
#elif defined(PANEL_1024X600) && defined(PANEL_LVDS)
code REG_IDX_DATA_t InitSwDefault_Table[] = {
#ifdef MODEL_TW8836DEMO
	{0x007, 0x02},				//It is a LVDS, so dont care Output.
								//but, it needs to remove TCON to use TCREV.			
	{0x008,	0xBC},	//4mA. TRI_FPD, TRI_EN.	GPOSEL=1 for FP_PWC off
#endif
	{0x040, 0x20},  //VDCLK_POL=1, High Active.
	{0x041,	0x00},	//input control II, def:C0
	{0x0B1, 0xC0},	//TSC CTRL II. def:00
	{0x106, 0x03},	//ACNTL def:00
	{0x107, 0x02},	//DEC Crop. vActive 288 to 240
	{0x109, 0xF0},	
	{0x10A, 0x0B},  //hDelay def:0A
	{0x117, 0x30},	//vertical peak def:40 read:80, FW uses 0x30
	{0x1C2, 0xD2},	//LLPLL CTRL def:01
 	{0x1CB, 0x16},	//SOG Threshold def:30 COMP:0xD6 PC:0x56
	{0x1D4, 0x20},	//clamp mode control def:00 FW uses 0x20. rising.
	{0x1D7, 0x00},	//clamp master location def:0x70 FW starts from 0x00.
	{0x1D9, 0x02},	//Y clamp ref. def:04 FW uses 0x02
	{0x1DC, 0x10},	// def:0x20	FW uses 0x10.
	{0x1E3, 0x37},	//ADC CTRL II def:87 read:0x07 DEC:0x07, aREG:0x37. error on datasheet
	{0x1E4, 0x55},	//ADC CTRL III def:33 DEC:0x33, aREG:0x55
	{0x1E5, 0x55},	//ADC CTRL IV	def:31 DEC:0x33, aRGB:0x55	FW BUG.DEC:0x31	BKTODO131126
	{0x1E6, 0x20},	//ADC CTRL V	def:00 DEC:0x00, aRGB:0x20
	{0x1E8, 0x20},	//ADC CTRL VII	def:01	DEC:0x0F, aRGB:0x00
	{0x1F6, 0xB0},  //DIFF CLAMP CTRL I		def:30 On CM_SLEEP.

	{0x205, 0xCD},	//(PANEL_H==1024 && PANEL_V==600)
	{0x206, 0x0C},	

	{0x207, 0x40},	//PXScale		def:80 FW uses 0x40
	{0x208, 0x20},	//PXInc_Lo		def:10 FW uses 0x20
	{0x20B, 0x10},	//HDelay2		def:30 FW uses 0x10(16).

	{0x20D, 0x90},	//if PANEL_1024X600

	{0x20F, 0x02},	//HPADJ_LO  	def:00 FW uses 0x02
	{0x210, 0x30},	//HA_POS_LO with REG221[5:4] def:10 FW uses 0x30. it should be REG20B[]+0x20.

	{0x211, 0x00},	//(PANEL_H==1024)
	{0x212, 0x04},

	{0x213, 0x00},	//HA_POS_Lo with def:10
	{0x214, 0x0A},	//HS_LEN def:20

	{0x216, 0x58},	//(PANEL_V==600)
	{0x217, 0x02},

	{0x219, 0x27},	//def:00

 					//(PANEL_H==1024)
					//need freerun value depend on PANEL_H.
	{0x21C, 0x42},	//AUTO_ON + Htotal value(1144) 
	{0x21D, 0x78},

	{0x21E, 0x02},	//def:00

					//TCON
	{0x240, 0x10},	//def:10
	{0x241, 0x00},	//def:00
	{0x242, 0x05},	//def:01
	{0x243, 0x01},	//def:00
	{0x244, 0x64},	//def:00
	{0x245, 0xF4},	//def:01
	{0x246, 0x00},	//def:00
	{0x247, 0x0A},	//def:00
	{0x248, 0x36},	//def:01
	{0x249, 0x10},	//def:10
	{0x24A, 0x00},	//def:00
	{0x24B, 0x00},	//def:00
	{0x24C, 0x00},	//def:10
	{0x24D, 0x44},	//def:80
	{0x24E, 0x04},	//def:00
	//REG24F		  def:00
	//REG250		  def:00
	//REG251		  def:00

	{0x28B, 0x40},	//def:30  FW uses 0x40
//	{0x2E4, 0x21},	//def:00  FW starts from 0x21

	{0x648, 0x01},	//def:01, read:00

	{0xFFF, 0xFF}	//EOF
};
#else //TCON 
code REG_IDX_DATA_t InitSwDefault_Table[] = {
	{0x006, 0x06},	//SRST def:00
	{0x008, 0xB6},	//Output CTL II def:30
	{0x040, 0x20},  //VDCLK_POL=1, High Active.
	{0x041,	0x00},	//input control II, def:C0
	{0x0B1, 0xC0},	//TSC CTRL II. def:00
	{0x106, 0x03},	//ACNTL def:00
	{0x107, 0x02},	//DEC Crop. vActive 288 to 240
	{0x109, 0xF0},	
	{0x10A, 0x0B},  //hDelay def:0A
	{0x117, 0x30},	//vertical peak def:40 read:80, FW uses 0x30
	{0x1C2, 0xD2},	//LLPLL CTRL def:01
 	{0x1CB, 0x16},	//SOG Threshold def:30 COMP:0xD6 PC:0x56
	{0x1D4, 0x20},	//clamp mode control def:00 FW uses 0x20. rising.
	{0x1D7, 0x00},	//clamp master location def:0x70 FW starts from 0x00.
	{0x1D9, 0x02},	//Y clamp ref. def:04 FW uses 0x02
	{0x1DC, 0x10},	// def:0x20	FW uses 0x10.
	{0x1E3, 0x37},	//ADC CTRL II def:87 read:0x07 DEC:0x07, aREG:0x37. error on datasheet
	{0x1E4, 0x55},	//ADC CTRL III def:33 DEC:0x33, aREG:0x55
	{0x1E5, 0x55},	//ADC CTRL IV	def:31 DEC:0x33, aRGB:0x55	FW BUG.DEC:0x31	BKTODO131126
	{0x1E6, 0x20},	//ADC CTRL V	def:00 DEC:0x00, aRGB:0x20
	{0x1E8, 0x20},	//ADC CTRL VII	def:01	DEC:0x0F, aRGB:0x00
	{0x1F6, 0xB0},  //DIFF CLAMP CTRL I		def:30 On CM_SLEEP.

	{0x205, 0x00},
	{0x206, 0x10},	//it assume 480i ???

	{0x207, 0x40},	//PXScale		def:80 FW uses 0x40
	{0x208, 0x20},	//PXInc_Lo		def:10 FW uses 0x20
	{0x20B, 0x10},	//HDelay2		def:30 FW uses 0x10(16).

	//{0x20D, 0x92},
	{0x20D, 0x81}, //BK140902, 72MHz/1/2 Pol:0

	{0x20F, 0x02},	//HPADJ_LO  	def:00 FW uses 0x02
	{0x210, 0x30},	//HA_POS_LO with REG221[5:4] def:10 FW uses 0x30. it should be REG20B[]+0x20.
			  
	{0x211, 0x20},	//(PANEL_H==800)
	{0x212, 0x03},

	{0x213, 0x00},	//HA_POS_Lo with def:10
	{0x214, 0x0A},	//HS_LEN def:20
					//(PANEL_V==480)
	{0x216, 0xE0},  //def:00
	{0x217, 0x01},  //def:03

	{0x219, 0x27},	//def:00

	{0x21C, 0x42},  //def:40 read:0x00  BUGBUG130327 need manual.
	{0x21D, 0x3E},	//def:00

	{0x21E, 0x02},	//def:00

					//TCON
	{0x240, 0x10},	//def:10
	{0x241, 0x00},	//def:00
	{0x242, 0x05},	//def:01
	{0x243, 0x01},	//def:00
	{0x244, 0x64},	//def:00
	{0x245, 0xF4},	//def:01
	{0x246, 0x00},	//def:00
	{0x247, 0x0A},	//def:00
	{0x248, 0x36},	//def:01
	{0x249, 0x10},	//def:10
	{0x24A, 0x00},	//def:00
	{0x24B, 0x00},	//def:00
	{0x24C, 0x00},	//def:10
	{0x24D, 0x44},	//def:80
	{0x24E, 0x04},	//def:00
	//REG24F		  def:00
	//REG250		  def:00
	//REG251		  def:00

 	{0x28B, 0x40},	//def:30  FW uses 0x40
					//(PANEL_FORMAT_666)
	{0x2E4, 0x21},	//def:00  FW starts from 0x21

	{0x648, 0x01},	//def:01, read:00

	{0xFFF, 0xFF}	//EOF
};
#endif


code REG_IDX_DATA_t Recover_Decoder[] = {
	{0x10C, 0xCC},	//DEC:0xDC,	aRGB:0xCC
	{0x1E3, 0x07},	//DEC:0x07, aREG:0x37
	{0x1E4, 0x33},	//DEC:0x33, aREG:0x55
	{0x1E5, 0x33},	//DEC:0x33, aRGB:0x55
	{0x1E6, 0x00},	//DEC:0x00, aRGB:0x20
	{0x1E8, 0x2F},	//DEC:0x0F, aRGB:0x00
	{0xFFF, 0xFF}	//EOF
};


code REG_IDX_DATA_t Recover_aRGB[] = {
	{0x10C, 0xCC},	//DEC:0xCC,	aRGB:0xCC
	{0x1E3, 0x37},	//DEC:0x07, aREG:0x37
	{0x1E4, 0x55},	//DEC:0x33, aREG:0x55
	{0x1E5, 0x55},	//DEC:0x33, aRGB:0x55
	{0x1E6, 0x20},	//DEC:0x00, aRGB:0x20
	{0x1E8, 0x20},	//DEC:0x0F, aRGB:0x00
	{0x1EA, 0x03},
	{0xFFF, 0xFF}	//EOF
};

#ifdef SUPPORT_COMPONENT
CODE REG_IDX_DATA_t DataInit_Component_Init1080p60_step1[] = {
//{0x102, 0x2a},    //+ Mux sel.
  {0x105, 0x20},    //* AFE Mode, AAF on/off
//{0x106, 0x00},    //+ Enable ADC

//----- LLPLL
//{0x1c0, 0x10},    //+ Input config.
  {0x1C2, 0xE2},    //* VCO, Charge Pump
  {0x1C3, 0x08},    //* Divider H
  {0x1C4, 0x97},    //* Divider L
  {0x1C5, 0x00},    //* Phase
//{0x1c6, 0x20},    //+ Loop Filter 0, Change to 7 later...
//{0x1c7, 0x04},    //- VCO Norminal freq.
//{0x1c8, 0x00},    //- VCO Norminal freq.
//{0x1c9, 0x00},    //+ Pre Coast
//{0x1ca, 0x00},    //+ Post Coast
//{0x1cb, 0xce},    //+ Power, SOG Threshold
//{0x1cc, 0x02},    //+ Sync Output sel.
//{0x1cd, 0x54},    //. Charge Pump Fine Control
//----- ADC Gain
  {0x1D0, 0x07},    //*
  {0x1D1, 0x10},    //*
  {0x1D2, 0x10},    //*
  {0x1D3, 0x10},    //*
//----- Clamp
//{0x1d4, 0x00},    //+ Mode
//{0x1d5, 0x00},    //+ Start
//{0x1d6, 0x10},    //+ Stop
  {0x1D7, 0x0E},    //* Master Location
//{0x1d8, 0x00},    //- Debug off
//{0x1d9, 0x01},    //+ G Level
//{0x1da, 0x80},    //+ B Level
//{0x1db, 0x80},    //+ R Level
//----- LLPLL Misc.
//{0x1dc, 0x10},    //+ HS width
//{0x1e0, 0x00},    //- Test
//{0x1e1, 0x05},    //+ GPLL Pd.
//----- AFE
  {0x1E2, 0x59},    //* Bais, VREF
  {0x1E3, 0x17},    //* Bias
  {0x1E4, 0x34},    //* Bias
  {0x1E5, 0x33},    //* Bias
  {0x1E6, 0x20},    //* Bias
  {0x1E7, 0x2A},    //* AAF  

  {0xFFF, 0xFF}	//EOF
};

CODE REG_IDX_DATA_t DataInit_Component_Init1080p50_H56_step1[] = {
//{0x102, 0x2a},    //+ Mux sel.
  {0x105, 0x20},    //* AFE Mode, AAF on/off
//{0x106, 0x00},    //+ Enable ADC

//----- LLPLL
//{0x1c0, 0x10},    //+ Input config.
  {0x1C2, 0xF2},    //* VCO, Charge Pump
  {0x1C3, 0x0A},    //* Divider H
  {0x1C4, 0x4F},    //* Divider L
  {0x1C5, 0x03},    //* Phase
//{0x1c6, 0x20},    //+ Loop Filter 0, Change to 7 later...
//{0x1c7, 0x04},    //- VCO Norminal freq.
//{0x1c8, 0x00},    //- VCO Norminal freq.
//{0x1c9, 0x00},    //+ Pre Coast
//{0x1ca, 0x00},    //+ Post Coast
//{0x1cb, 0xce},    //+ Power, SOG Threshold
//{0x1cc, 0x02},    //+ Sync Output sel.
//{0x1cd, 0x54},    //. Charge Pump Fine Control
//----- ADC Gain
  {0x1D0, 0x07},    //*
  {0x1D1, 0x10},    //*
  {0x1D2, 0x10},    //*
  {0x1D3, 0x10},    //*
//----- Clamp
//{0x1d4, 0x00},    //+ Mode
//{0x1d5, 0x00},    //+ Start
//{0x1d6, 0x10},    //+ Stop
  {0x1D7, 0x0E},    //* Master Location
//{0x1d8, 0x00},    //- Debug off
//{0x1d9, 0x01},    //+ G Level
//{0x1da, 0x80},    //+ B Level
//{0x1db, 0x80},    //+ R Level
//----- LLPLL Misc.
//{0x1dc, 0x10},    //+ HS width
//{0x1e0, 0x00},    //- Test
//{0x1e1, 0x05},    //+ GPLL Pd.
//----- AFE
  {0x1E2, 0x59},    //* Bais, VREF
  {0x1E3, 0x17},    //* Bias
  {0x1E4, 0x34},    //* Bias
  {0x1E5, 0x33},    //* Bias
  {0x1E6, 0x20},    //* Bias
  {0x1E7, 0x2A},    //* AAF  
  
  {0xFFF, 0xFF}	//EOF
};


CODE REG_IDX_DATA_t DataInit_Component_Init1080i30_step1[] = {
//{0x102, 0x2a},    //+ Mux sel.
  {0x105, 0x20},    //* AFE Mode, AAF on/off
//{0x106, 0x00},    //+ Enable ADC

//----- LLPLL
//{0x1c0, 0x10},    //+ Input config.
  {0x1C2, 0xE2},    //* VCO, Charge Pump
  {0x1C3, 0x08},    //* Divider H
  {0x1C4, 0x97},    //* Divider L
  {0x1C5, 0x00},    //* Phase
//{0x1c6, 0x20},    //+ Loop Filter 0, Change to 7 later...
//{0x1c7, 0x04},    //- VCO Norminal freq.
//{0x1c8, 0x00},    //- VCO Norminal freq.
//{0x1c9, 0x00},    //+ Pre Coast
//{0x1ca, 0x00},    //+ Post Coast
//{0x1cb, 0xce},    //+ Power, SOG Threshold
//{0x1cc, 0x02},    //+ Sync Output sel.
//{0x1cd, 0x54},    //. Charge Pump Fine Control
//----- ADC Gain
  {0x1D0, 0x07},    //*
  {0x1D1, 0x10},    //*
  {0x1D2, 0x10},    //*
  {0x1D3, 0x10},    //*
//----- Clamp
//{0x1d4, 0x00},    //+ Mode
//{0x1d5, 0x00},    //+ Start
//{0x1d6, 0x10},    //+ Stop
  {0x1D7, 0x18},    //* Master Location
//{0x1d8, 0x00},    //- Debug off
//{0x1d9, 0x01},    //+ G Level
//{0x1da, 0x80},    //+ B Level
//{0x1db, 0x80},    //+ R Level
//----- LLPLL Misc.
//{0x1dc, 0x10},    //+ HS width
//{0x1e0, 0x00},    //- Test
//{0x1e1, 0x05},    //+ GPLL Pd.
//----- AFE
  {0x1E2, 0x59},    //* Bais, VREF
  {0x1E3, 0x17},    //* Bias
  {0x1E4, 0x34},    //* Bias
  {0x1E5, 0x33},    //* Bias
  {0x1E6, 0x20},    //* Bias
  {0x1E7, 0x2A},    //* AAF  

  {0xFFF, 0xFF}	//EOF
};


CODE REG_IDX_DATA_t DataInit_Component_Init1080i25_H28_step1[] = {

//{0x102, 0x2a},    //+ Mux sel.
  {0x105, 0x20},    //* AFE Mode, AAF on/off
//{0x106, 0x00},    //+ Enable ADC

//----- LLPLL
//{0x1c0, 0x10},    //+ Input config.
  {0x1C2, 0xF2},    //* VCO, Charge Pump
  {0x1C3, 0x0A},    //* Divider H
  {0x1C4, 0x4F},    //* Divider L
  {0x1C5, 0x0F},    //* Phase
//{0x1c6, 0x20},    //+ Loop Filter 0, Change to 7 later...
//{0x1c7, 0x04},    //- VCO Norminal freq.
//{0x1c8, 0x00},    //- VCO Norminal freq.
//{0x1c9, 0x00},    //+ Pre Coast
//{0x1ca, 0x00},    //+ Post Coast
//{0x1cb, 0xce},    //+ Power, SOG Threshold
//{0x1cc, 0x02},    //+ Sync Output sel.
//{0x1cd, 0x54},    //. Charge Pump Fine Control
//----- ADC Gain
  {0x1D0, 0x07},    //*
  {0x1D1, 0x10},    //*
  {0x1D2, 0x10},    //*
  {0x1D3, 0x10},    //*
//----- Clamp
//{0x1d4, 0x00},    //+ Mode
//{0x1d5, 0x00},    //+ Start
//{0x1d6, 0x10},    //+ Stop
  {0x1D7, 0x28},    //* Master Location
//{0x1d8, 0x00},    //- Debug off
//{0x1d9, 0x01},    //+ G Level
//{0x1da, 0x80},    //+ B Level
//{0x1db, 0x80},    //+ R Level
//----- LLPLL Misc.
//{0x1dc, 0x10},    //+ HS width
//{0x1e0, 0x00},    //- Test
//{0x1e1, 0x05},    //+ GPLL Pd.
//----- AFE
  {0x1E2, 0x59},    //* Bais, VREF
  {0x1E3, 0x17},    //* Bias
  {0x1E4, 0x34},    //* Bias
  {0x1E5, 0x33},    //* Bias
  {0x1E6, 0x20},    //* Bias
  {0x1E7, 0x2A},    //* AAF  

  {0xFFF, 0xFF}	//EOF
};


CODE REG_IDX_DATA_t DataInit_Component_Init1080i25_H31_step1[] = {

//{0x102, 0x2a},    //+ Mux sel.
  {0x105, 0x20},    //* AFE Mode, AAF on/off
//{0x106, 0x00},    //+ Enable ADC

//----- LLPLL
//{0x1c0, 0x10},    //+ Input config.
  {0x1C2, 0xF2},    //* VCO, Charge Pump
  {0x1C3, 0x09},    //* Divider H
  {0x1C4, 0x47},    //* Divider L
  {0x1C5, 0x0F},    //* Phase
//{0x1c6, 0x20},    //+ Loop Filter 0, Change to 7 later...
//{0x1c7, 0x04},    //- VCO Norminal freq.
//{0x1c8, 0x00},    //- VCO Norminal freq.
//{0x1c9, 0x00},    //+ Pre Coast
//{0x1ca, 0x00},    //+ Post Coast
//{0x1cb, 0xce},    //+ Power, SOG Threshold
//{0x1cc, 0x02},    //+ Sync Output sel.
//{0x1cd, 0x54},    //. Charge Pump Fine Control
//----- ADC Gain
  {0x1D0, 0x07},    //*
  {0x1D1, 0x10},    //*
  {0x1D2, 0x10},    //*
  {0x1D3, 0x10},    //*
//----- Clamp
//{0x1d4, 0x00},    //+ Mode
//{0x1d5, 0x00},    //+ Start
//{0x1d6, 0x10},    //+ Stop
  {0x1D7, 0x28},    //* Master Location
//{0x1d8, 0x00},    //- Debug off
//{0x1d9, 0x01},    //+ G Level
//{0x1da, 0x80},    //+ B Level
//{0x1db, 0x80},    //+ R Level
//----- LLPLL Misc.
//{0x1dc, 0x10},    //+ HS width
//{0x1e0, 0x00},    //- Test
//{0x1e1, 0x05},    //+ GPLL Pd.
//----- AFE
  {0x1E2, 0x59},    //* Bais, VREF
  {0x1E3, 0x17},    //* Bias
  {0x1E4, 0x34},    //* Bias
  {0x1E5, 0x33},    //* Bias
  {0x1E6, 0x20},    //* Bias
  {0x1E7, 0x2A},    //* AAF  

  {0xFFF, 0xFF}	//EOF
};

CODE REG_IDX_DATA_t DataInit_Component_Init720p60_step1[] = {
//{0x102, 0x2a},    //+ Mux sel.
  {0x105, 0x20},    //* AFE Mode, AAF on/off
//{0x106, 0x00},    //+ Enable ADC

//----- LLPLL
//{0x1c0, 0x10},    //+ Input config.
  {0x1C2, 0xE2},    //* VCO, Charge Pump
  {0x1C3, 0x06},    //* Divider H
  {0x1C4, 0x71},    //* Divider L
  {0x1C5, 0x09},    //* Phase
//{0x1c6, 0x20},    //+ Loop Filter 0, Change to 7 later...
//{0x1c7, 0x04},    //- VCO Norminal freq.
//{0x1c8, 0x00},    //- VCO Norminal freq.
//{0x1c9, 0x00},    //+ Pre Coast
//{0x1ca, 0x00},    //+ Post Coast
//{0x1cb, 0xce},    //+ Power, SOG Threshold
//{0x1cc, 0x02},    //+ Sync Output sel.
//{0x1cd, 0x54},    //. Charge Pump Fine Control
//----- ADC Gain
  {0x1D0, 0x07},    //*
  {0x1D1, 0x10},    //*
  {0x1D2, 0x10},    //*
  {0x1D3, 0x10},    //*
//----- Clamp
//{0x1d4, 0x00},    //+ Mode
//{0x1d5, 0x00},    //+ Start
//{0x1d6, 0x10},    //+ Stop
  {0x1D7, 0x26},    //* Master Location
//{0x1d8, 0x00},    //- Debug off
//{0x1d9, 0x01},    //+ G Level
//{0x1da, 0x80},    //+ B Level
//{0x1db, 0x80},    //+ R Level
//----- LLPLL Misc.
//{0x1dc, 0x10},    //+ HS width
//{0x1e0, 0x00},    //- Test
//{0x1e1, 0x05},    //+ GPLL Pd.
//----- AFE
  {0x1E2, 0x59},    //* Bais, VREF
  {0x1E3, 0x17},    //* Bias
  {0x1E4, 0x34},    //* Bias
  {0x1E5, 0x33},    //* Bias
  {0x1E6, 0x20},    //* Bias
  {0x1E7, 0x2A},    //* AAF  

  {0xFFF, 0xFF}	//EOF
};

CODE REG_IDX_DATA_t DataInit_Component_Init720p50_step1[] = {

//{0x041, 0x3E,	

//{0x102, 0x2a},    //+ Mux sel.
  {0x105, 0x20},    //* AFE Mode, AAF on/off
//{0x106, 0x00},    //+ Enable ADC

//----- LLPLL
//{0x1c0, 0x10},    //+ Input config.
  {0x1C2, 0xE2},    //* VCO, Charge Pump
  {0x1C3, 0x07},    //* Divider H
  {0x1C4, 0xBB},    //* Divider L
  {0x1C5, 0x00},    //* Phase
//{0x1c6, 0x20},    //+ Loop Filter 0, Change to 7 later...
//{0x1c7, 0x04},    //- VCO Norminal freq.
//{0x1c8, 0x00},    //- VCO Norminal freq.
//{0x1c9, 0x00},    //+ Pre Coast
//{0x1ca, 0x00},    //+ Post Coast
//{0x1cb, 0xce},    //+ Power, SOG Threshold
//{0x1cc, 0x02},    //+ Sync Output sel.
//{0x1cd, 0x54},    //. Charge Pump Fine Control
//----- ADC Gain
  {0x1D0, 0x07},    //*
  {0x1D1, 0x10},    //*
  {0x1D2, 0x10},    //*
  {0x1D3, 0x10},    //*
//----- Clamp
//{0x1d4, 0x00},    //+ Mode
//{0x1d5, 0x00},    //+ Start
//{0x1d6, 0x10},    //+ Stop
  {0x1D7, 0x26},    //* Master Location
//{0x1d8, 0x00},    //- Debug off
//{0x1d9, 0x01},    //+ G Level
//{0x1da, 0x80},    //+ B Level
//{0x1db, 0x80},    //+ R Level
//----- LLPLL Misc.
//{0x1dc, 0x10},    //+ HS width
//{0x1e0, 0x00},    //- Test
//{0x1e1, 0x05},    //+ GPLL Pd.
//----- AFE
  {0x1E2, 0x59},    //* Bais, VREF
  {0x1E3, 0x17},    //* Bias
  {0x1E4, 0x34},    //* Bias
  {0x1E5, 0x33},    //* Bias
  {0x1E6, 0x20},    //* Bias
  {0x1E7, 0x2A},    //* AAF  

  {0xFFF, 0xFF}	//EOF
};

CODE REG_IDX_DATA_t DataInit_Component_Init576p_step1[] = {
//{0x041, 0x3E,	

//{0x102, 0x2a},    //+ Mux sel.
  {0x105, 0x20},    //* AFE Mode, AAF on/off
//{0x106, 0x00},    //+ Enable ADC

//----- LLPLL
//{0x1c0, 0x10},    //+ Input config.
  {0x1C2, 0xE2},    //* VCO, Charge Pump
  {0x1C3, 0x03},    //* Divider H
  {0x1C4, 0x5F},    //* Divider L
  {0x1C5, 0x0E},    //* Phase
//{0x1c6, 0x20},    //+ Loop Filter 0, Change to 7 later...
//{0x1c7, 0x04},    //- VCO Norminal freq.
//{0x1c8, 0x00},    //- VCO Norminal freq.
//{0x1c9, 0x00},    //+ Pre Coast
//{0x1ca, 0x00},    //+ Post Coast
//{0x1cb, 0xce},    //+ Power, SOG Threshold
//{0x1cc, 0x02},    //+ Sync Output sel.
//{0x1cd, 0x54},    //. Charge Pump Fine Control
//----- ADC Gain
  {0x1D0, 0x07},    //*
  {0x1D1, 0x10},    //*
  {0x1D2, 0x10},    //*
  {0x1D3, 0x10},    //*
//----- Clamp
//{0x1d4, 0x00},    //+ Mode
//{0x1d5, 0x00},    //+ Start
//{0x1d6, 0x10},    //+ Stop
  {0x1D7, 0x3A},    //* Master Location
//{0x1d8, 0x00},    //- Debug off
//{0x1d9, 0x01},    //+ G Level
//{0x1da, 0x80},    //+ B Level
//{0x1db, 0x80},    //+ R Level
//----- LLPLL Misc.
//{0x1dc, 0x10},    //+ HS width
//{0x1e0, 0x00},    //- Test
//{0x1e1, 0x05},    //+ GPLL Pd.
//----- AFE
  {0x1E2, 0x59},    //* Bais, VREF
  {0x1E3, 0x37},    //* Bias
  {0x1E4, 0x14},    //* Bias
  {0x1E5, 0x42},    //* Bias
  {0x1E6, 0x20},    //* Bias
  {0x1E7, 0x2A},    //* AAF  

  {0xFFF, 0xFF}	//EOF
};


CODE REG_IDX_DATA_t DataInit_Component_Init576i_step1[] = {

//{0x102, 0x2a},    //+ Mux sel.
  {0x105, 0x2E},    //* AFE Mode, AAF on/off
//{0x106, 0x00},    //+ Enable ADC

//----- LLPLL
//{0x1c0, 0x10},    //+ Input config.
  {0x1C2, 0xC1},    //* VCO, Charge Pump
  {0x1C3, 0x03},    //* Divider H
  {0x1C4, 0x5F},    //* Divider L
  {0x1C5, 0x00},    //* Phase
//{0x1c6, 0x20},    //+ Loop Filter 0, Change to 7 later...
//{0x1c7, 0x04},    //- VCO Norminal freq.
//{0x1c8, 0x00},    //- VCO Norminal freq.
//{0x1c9, 0x00},    //+ Pre Coast
//{0x1ca, 0x00},    //+ Post Coast
//{0x1cb, 0xce},    //+ Power, SOG Threshold
//{0x1cc, 0x02},    //+ Sync Output sel.
//{0x1cd, 0x54},    //. Charge Pump Fine Control
//----- ADC Gain
  {0x1D0, 0x07},    //*
  {0x1D1, 0x80},    //*
  {0x1D2, 0x80},    //*
  {0x1D3, 0x80},    //*
//----- Clamp
//{0x1d4, 0x00},    //+ Mode
//{0x1d5, 0x00},    //+ Start
//{0x1d6, 0x10},    //+ Stop
  {0x1D7, 0x80},    //* Master Location
//{0x1d8, 0x00},    //- Debug off
//{0x1d9, 0x01},    //+ G Level
//{0x1da, 0x80},    //+ B Level
//{0x1db, 0x80},    //+ R Level
//----- LLPLL Misc.
//{0x1dc, 0x10},    //+ HS width
//{0x1e0, 0x00},    //- Test
//{0x1e1, 0x05},    //+ GPLL Pd.
//----- AFE
  {0x1E2, 0x59},    //* Bais, VREF
  {0x1E3, 0x37},    //* Bias
  {0x1E4, 0x14},    //* Bias
  {0x1E5, 0x42},    //* Bias
  {0x1E6, 0x20},    //* HS PGA
  {0x1E7, 0x2A},    //* AAF  
  {0x1E8, 0x2E},

  {0xFFF, 0xFF}	//EOF
};

CODE REG_IDX_DATA_t DataInit_Component_Init480p_step1[] = {
//{0x041, 0x3E,	

//{0x102, 0x2a},    //+ Mux sel.
  {0x105, 0x20},    //* AFE Mode, AAF on/off
//{0x106, 0x00},    //+ Enable ADC

//----- LLPLL
//{0x1c0, 0x10},    //+ Input config.
  {0x1C2, 0xE2},    //* VCO, Charge Pump
  {0x1C3, 0x03},    //* Divider H
  {0x1C4, 0x59},    //* Divider L
  {0x1C5, 0x00},    //* Phase
//{0x1c6, 0x20},    //+ Loop Filter 0, Change to 7 later...
//{0x1c7, 0x04},    //- VCO Norminal freq.
//{0x1c8, 0x00},    //- VCO Norminal freq.
//{0x1c9, 0x00},    //+ Pre Coast
//{0x1ca, 0x00},    //+ Post Coast
//{0x1cb, 0xce},    //+ Power, SOG Threshold
//{0x1cc, 0x02},    //+ Sync Output sel.
//{0x1cd, 0x54},    //. Charge Pump Fine Control
//----- ADC Gain
  {0x1D0, 0x07},    //*
  {0x1D1, 0x90},    //*
  {0x1D2, 0x90},    //*
  {0x1D3, 0x90},    //*
//----- Clamp
//{0x1d4, 0x00},    //+ Mode
//{0x1d5, 0x00},    //+ Start
//{0x1d6, 0x10},    //+ Stop
  {0x1D7, 0x40},    //* Master Location
//{0x1d8, 0x00},    //- Debug off
//{0x1d9, 0x01},    //+ G Level
//{0x1da, 0x80},    //+ B Level
//{0x1db, 0x80},    //+ R Level
//----- LLPLL Misc.
//{0x1dc, 0x10},    //+ HS width
//{0x1e0, 0x00},    //- Test
//{0x1e1, 0x05},    //+ GPLL Pd.
//----- AFE
  {0x1E2, 0x59},    //* Bais, VREF
  {0x1E3, 0x37},    //* Bias
  {0x1E4, 0x14},    //* Bias
  {0x1E5, 0x42},    //* Bias
  {0x1E6, 0x00},    //* Bias
  {0x1E7, 0x2A},    //* AAF  

  {0xFFF, 0xFF}	//EOF
};


CODE REG_IDX_DATA_t DataInit_Component_Init480i_step1[] = {

//{0x102, 0x2a},    //+ Mux sel.
  {0x105, 0x20},    //* AFE Mode, AAF on/off
//{0x106, 0x00},    //+ Enable ADC

//----- LLPLL
//{0x1c0, 0x10},    //+ Input config.
  {0x1C2, 0xE2},    //* VCO, Charge Pump
  {0x1C3, 0x03},    //* Divider H
  {0x1C4, 0x59},    //* Divider L
  {0x1C5, 0x00},    //* Phase
//{0x1c6, 0x20},    //+ Loop Filter 0, Change to 7 later...
//{0x1c7, 0x04},    //- VCO Norminal freq.
//{0x1c8, 0x00},    //- VCO Norminal freq.
//{0x1c9, 0x00},    //+ Pre Coast
//{0x1ca, 0x00},    //+ Post Coast
//{0x1cb, 0xce},    //+ Power, SOG Threshold
//{0x1cc, 0x02},    //+ Sync Output sel.
//{0x1cd, 0x54},    //. Charge Pump Fine Control
//----- ADC Gain
  {0x1D0, 0x07},    //*
  {0x1D1, 0x10},    //*
  {0x1D2, 0x10},    //*
  {0x1D3, 0x10},    //*
//----- Clamp
//{0x1d4, 0x00},    //+ Mode
//{0x1d5, 0x00},    //+ Start
//{0x1d6, 0x10},    //+ Stop
  {0x1D7, 0x80},    //* Master Location
//{0x1d8, 0x00},    //- Debug off
//{0x1d9, 0x01},    //+ G Level
//{0x1da, 0x80},    //+ B Level
//{0x1db, 0x80},    //+ R Level
//----- LLPLL Misc.
//{0x1dc, 0x10},    //+ HS width
//{0x1e0, 0x00},    //- Test
//{0x1e1, 0x05},    //+ GPLL Pd.
//----- AFE
  {0x1E2, 0x59},    //* Bais, VREF
  {0x1E3, 0x17},    //* Bias
  {0x1E4, 0x34},    //* Bias
  {0x1E5, 0x33},    //* Bias
  {0x1E6, 0x20},    //* HS PGA
  {0x1E7, 0x2A},    //* AAF  
  {0x1E8, 0x2E},

  {0xFFF, 0xFF}	//EOF
};

#endif





/**
* Description: download indexed register value
* @param
*	pTbl : REG_IDX_DATA pointer.
*/
static void Init8836Register(REG_IDX_DATA_t *pTbl, BYTE fCheck)
{
	BYTE bTemp;

	if(fCheck) {
		while(1) {
			if(pTbl->idx==0xFFF)
				break;
			bTemp = ReadTW88(pTbl->idx);
			//these two regs complain the readonly area. let's mask it.
			if(pTbl->idx == 0x11A)
				bTemp &= 0xE0;
			else if(pTbl->idx == 0x11C)
				bTemp &= 0x0F;

			if(bTemp != pTbl->value) {
				wPrintf("\n\rREG%03X R:%02bx W:%02bx", pTbl->idx, bTemp,pTbl->value); 
				WriteTW88(pTbl->idx,pTbl->value);
			}
			pTbl++;
		}
	}
	else {
		while(1) {
			if(pTbl->idx==0xFFF /*&& pTbl->value==0xFF*/)
				break;
			WriteTW88(pTbl->idx,pTbl->value);
			pTbl++;
		}
	}
}

/**
* Description: initialize TW8836 Default value
* @param
*	InputMain : Input Type
*	fPowerOn 1:Powerup boot, 0:recover for InputMain.
*		if fPowerUpBoot==1, it will ignore InputMain value.
*/
//void Init8836AsDefault_for_watchdog(void)
//{
//	Init8836Register(InitHwDefault_Table, 0);
//}
void Init8836AsDefault(BYTE _InputMain, BYTE _fPowerOn)
{
	if (_fPowerOn)
	{
		//If power on, download HW default & SW default.
		Puts("\n\rInit8836AsDefault");
		Init8836Register(InitHwDefault_Table, 0);
		Init8836Register(InitSwDefault_Table, 0);

		return;
	}
	
	//--- recover
	Puts("\n\rRecover8836AsDefault(");
	PrintfInput(_InputMain, 0);
	Puts(")");

	//mute scler output.
	WriteTW88(REG21E, 0x03);		
 
	//Recover only the selected input.
	if (_InputMain == INPUT_CVBS 
	|| _InputMain == INPUT_SVIDEO 
	|| _InputMain == INPUT_BT656)
		Init8836Register(Recover_Decoder, 0);
	else if (_InputMain == INPUT_COMP 
	     || _InputMain == INPUT_PC)
		Init8836Register(Recover_aRGB, 0);
	//else
	//	INPUT_HDMI
	//	INPUT_LVDS

#if defined(PANEL_AUO_B133EW01) || defined(PANEL_1024X600)
#else
	//BK130307. PC->CVBS needs to recover SSPLL 
	//start from SSPLL 108MHz.
	
//	if(SpiFlashVendor==SFLASH_VENDOR_EON 
//	|| SpiFlashVendor==SFLASH_VENDOR_EON_256)
//		Sspll1SetFreqReg(SSPLL_105M_REG);
//	else	
//		Sspll1SetFreqReg(SSPLL_108M_REG);
	//BKTODO150122    --- please remove	
//	if(spiflash_chip->mid == SPIFLASH_MID_EON)
//		Sspll1SetFreqReg(SSPLL_105M_REG);
//	else
//		Sspll1SetFreqReg(SSPLL_108M_REG);						
#endif			
}

/*
*	BT656 External Encoder table.
*   TW8836 EVB only supports 8Bit SD.
*/
code BYTE BT656_table_8BIT_525I_YCBCR_TO_CVBS[] = {
	I2CID_ADV7390, 0x00,
	0x17, 0x02,
	0x00, 0x1C,
	0x01, 0x00,
	0x80, 0x10,
	0x82, 0xCB,	
	0xFF,0xFF	
};
code BYTE BT656_table_8BIT_625I_YCBCR_TO_CVBS[] = {
	I2CID_ADV7390, 0x00,
	0x17,0x02,
	0x00,0x1c,
	0x01,0x00,
	0x80,0x11,
	0x82,0xC3,
	0x8C,0xCB,
	0x8D,0x8A,
	0x8E,0x09,
	0x8F,0x2A,	
	0xFF,0xFF	
};
code BYTE BT656_table_8BIT_525P_YCBCR_TO_YPBPR[] = {	 //=>YPbPr
	I2CID_ADV7390, 0x00,
	0x17,0x02,
	0x00,0x1c,
	0x01,0x20,
	0x30,0x04,
	0x31,0x01,
	0xFF,0xFF	
};
code BYTE BT656_table_8BIT_625P_YCBCR_TO_YPBPR[] = {
	I2CID_ADV7390, 0x00,
	0x17,0x02,
	0x00,0x1c,
	0x01,0x20,
	0x30,0x1C,
	0x31,0x01,
	0xFF,0xFF	
};
#if 0
code BYTE BT656_table_8BIT_720P_YCBCR_TO_YPBPR[] = {
	I2CID_ADV7390, 0x00,
	0x17,0x02,
	0x00,0x1c,
	0x01,0x20,
	0x30,0x2C,
	0x31,0x01,
	0xFF,0xFF	
};
code BYTE BT656_table_8BIT_1080I_YCBCR_TO_YPBPR[] = {
	I2CID_ADV7390, 0x00,
	0x17,0x02,
	0x00,0x1c,
	0x01,0x20,
	0x30,0x6C,
	0x31,0x01,
	0xFF,0xFF	
};
#endif

//-----------------------------------------------------------------------------
/*
* init BT656 encoder
*
* TW8835 EVB2.1 and EVB3.1 use ADV7390.
* TW8835 EVB2.0 and EVB3.0 use BU9969.
*
* @param mode
*	0: BT656_8BIT_525I_YCBCR_TO_CVBS	
*	1: BT656_8BIT_625I_YCBCR_TO_CVBS	
*	2: BT656_8BIT_525P_YCBCR_TO_YPBPR	
*	3: BT656_8BIT_625P_YCBCR_TO_YPBPR	
*	4: BT656_8BIT_720P_YCBCR_TO_YPBPR	
*	5: BT656_8BIT_1080I_YCBCR_TO_YPBPR
*/
void BT656_InitExtEncoder(BYTE mode)
{
	BYTE *p;
	BYTE addr;

	switch(mode) {
	case BT656_8BIT_525I_YCBCR_TO_CVBS:	
		p = BT656_table_8BIT_525I_YCBCR_TO_CVBS; 
		break;
	case BT656_8BIT_625I_YCBCR_TO_CVBS:
		p = BT656_table_8BIT_625I_YCBCR_TO_CVBS; 
		break;
	case BT656_8BIT_525P_YCBCR_TO_YPBPR:
		p = BT656_table_8BIT_525P_YCBCR_TO_YPBPR; 
		break;
	case BT656_8BIT_625P_YCBCR_TO_YPBPR:
		p = BT656_table_8BIT_625P_YCBCR_TO_YPBPR; 
		break;
#if 0
	case BT656_8BIT_720P_YCBCR_TO_YPBPR:
		p = BT656_table_8BIT_720P_YCBCR_TO_YPBPR; 
		break;
	case BT656_8BIT_1080I_YCBCR_TO_YPBPR:
		p = BT656_table_8BIT_1080I_YCBCR_TO_YPBPR; 
		break;
#endif
	default:
		//unknown.
		return;
	}

	addr = p[0]; //ignore counter, p[1].
	p+=2;

#if defined(SUPPORT_I2C_MASTER)
	while (( p[0] != 0xFF ) || ( p[1]!= 0xFF )) {			// 0xff, 0xff is a end of data
		WriteI2CByte(addr, p[0], p[1]);
		p+=2;
	}
#endif
}

/**
* download component default registers
*
* @param mode
*	0: 480i
*	1: 576i
*	2: 480p
*	3: 576p
*	4: 1080i@25_H28
*	5: 1080i@30
*	6: 720p@50
*	7: 720p@60
*	8: 1080p@50
*	9: 1080p@60
*/
#ifdef SUPPORT_COMPONENT
void InitComponentReg(BYTE mode)
{
	WORD w_page=0;
	REG_IDX_DATA_t *p;
//	BYTE index,value;

	switch(mode) {
	case 0:	p=DataInit_Component_Init480i_step1;	break;
	case 1:	p=DataInit_Component_Init576i_step1;	break;
	case 2:	p=DataInit_Component_Init480p_step1;	break;
	case 3:	p=DataInit_Component_Init576p_step1;	break;
	case 4:	//we can't distinglish H28 and H31. Please select one.
			p=DataInit_Component_Init1080i25_H28_step1;	break;
			//p=DataInit_Component_Init1080i25_H31_step1;	break;
	case 5:	p=DataInit_Component_Init1080i30_step1;		break;
	case 6:	p=DataInit_Component_Init720p50_step1;	 	break;
	case 7:	p=DataInit_Component_Init720p60_step1;		break;
	case 8:	p=DataInit_Component_Init1080p50_H56_step1;	break;
	case 9:	p=DataInit_Component_Init1080p60_step1;	 	break;
	default:
		return;
	}

	Init8836Register(p,0);
}
#endif

//-----------------------------------------------------------------------------
/**
* print Input string
*/
void PrintfInput(BYTE Input, BYTE debug)
{
	if(debug <= DebugLevel) {
		switch(Input) {
		case INPUT_CVBS: 	Printf("CVBS"); 				break;
		case INPUT_SVIDEO: 	Printf("SVIDEO"); 				break;
		case INPUT_COMP: 	Printf("Component"); 			break;
		case INPUT_PC: 		Printf("PC"); 					break;
		case INPUT_DVI: 	Printf("DVI"); 					break;
		case INPUT_HDMIPC: 	Printf("HDMIPC"); 				break;
		case INPUT_HDMITV: 	Printf("HDMITV"); 				break;
		case INPUT_BT656: 	Printf("BT656");				break;
		case INPUT_LVDS: 	Printf("LVDSRx");				break;
		default: 			Printf("unknown:%02bd",Input); 	break;
		}
	}
}


#if	defined(DEBUG_BT656)
void PrintfBT656Input(BYTE Input, BYTE debug)
{
	if(debug > DebugLevel)
		return;

	switch(Input) {
	case BT656ENC_SRC_DEC: 	 Printf("DEC"); 				break;
	case BT656ENC_SRC_ARGB:  Printf("ARGB"); 				break;
	case BT656ENC_SRC_DTV: 	 Printf("DTV"); 				break;
	case BT656ENC_SRC_LVDS:  Printf("LVDS"); 				break;
	case BT656ENC_SRC_PANEL: Printf("PANEL"); 				break;
	case BT656ENC_SRC_OFF: 	 Printf("OFF"); 				break;
	case BT656ENC_SRC_AUTO:  Printf("AUTO"); 				break;
	default: 				 Printf("unknown:%02bd",Input); break;
	}
}
#endif
