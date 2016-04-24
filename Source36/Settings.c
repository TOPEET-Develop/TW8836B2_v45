/**
 * @file
 * settings.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	setup system 
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
//input			
//	CVBS	YIN0
//	SVIDEO	YIN1, CIN0
//	aRGB	G:YIN2 B:CIN0 R:VIN0 
//	aYUV	G:YIN2 B:CIN0 R:VIN0

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
#include "SpiFlashMap.h"

#if 0
//If we use SSPLL1, SW_Reset will hangup the system.
//we can not recover.
//TW8836B have to use 27MHz.
void SW_Reset_TW8836B(void)
{
	BYTE bTemp;

	bTemp = ReadTW88(REG4E1);
	if((bTemp & 0x30))
		WriteTW88(REG4E1, bTemp & ~0x30); /* I am not using 32KHz */
	WriteTW88(REG006, ReadTW88(REG006) | 0x80);	//SW RESET

	if(bTemp & 0x30)
		WriteTW88(REG4E0, bTemp);
}
#endif	
#if 0
//Low Voltage reset
//same as HW reset
void LV_Reset_TW8836B(void)
{
    WriteTW88(REG0D4,0x01);
    WriteTW88(REG0D4,0x00);
}
#endif

//=============================================================================
//	INPUT CLOCKS			                                               
//=============================================================================


//=============================================================================
// SSPLL
//=============================================================================

//-----------------------------------------------------------------------------
/**
* power up the SSPLL
* PLL108M(REG4E0[0]=1) also needs to turn on SSPLL1.
*/
void Sspll1PowerUp(BYTE fOn)
{
	if(fOn)	WriteTW88(REG0FC, ReadTW88(REG0FC) & ~0x80);
	else	WriteTW88(REG0FC, ReadTW88(REG0FC) |  0x80);
}
void Sspll2PowerUp(BYTE fOn)
{
	if(fOn)	WriteTW88(REG0EC, ReadTW88(REG0EC) & ~0x80);
	else	WriteTW88(REG0EC, ReadTW88(REG0EC) |  0x80);
}

//-----------------------------------------------------------------------------
/**
* SSPLL1 center frequency register value
*
*	register
*	R0F8[3:0]	FPLL[19:16]
*	R0F9[7:0]	FPLL[15:8]
*	R0FA[7:0]	FPLL[7:0]

*	2 80 00		135MHz
*	2 78 00		133.3125MHz
*	2 76 85		133MHz
*	2 71 C8		132MHz
*	2 70 00		131.625MHz
*	2 6D 0A		131MHz
*	2 68 4B		130MHz
*	2 60 00		128.25
*	2 40 00		121.5MHz
*	2 20 00		114.75MHz
*	2 00 00		108MHz
*	1 F1 C7		105Mhz
*	1 80 00		81MHz
*	1 55 55		72MHz
*	1 40 00		67.5MHz
*	1 20 00		60.75MHz
*	1 00 00		54MHz
*	0 80 00		27MHz
*/
void Sspll1SetFreqReg(DWORD fpll)
{
	WriteTW88(REG0FA_FPLL2, (BYTE)fpll );
	WriteTW88(REG0F9_FPLL1, (BYTE)(fpll>>8));
	WriteTW88(REG0F8_FPLL0, (BYTE)(fpll>>16));
}
/**
* SSPLL2 center frequency register value
*
*	register
*	R0E8[3:0]	FPLL[19:16]
*	R0E9[7:0]	FPLL[15:8]
*	R0EA[7:0]	FPLL[7:0]
*/
void Sspll2SetFreqReg(DWORD fpll)
{
	WriteTW88(REG0EA, (BYTE)fpll );
	WriteTW88(REG0E9, (BYTE)(fpll>>8));
	WriteTW88(REG0E8, (BYTE)(fpll>>16));
}

static DWORD Sspll1GetFreqReg(void)
{
	DWORD dFPLL;
	
	dFPLL = ReadTW88(REG0F8_FPLL0)&0x0F;
	dFPLL <<=8;
	dFPLL |= ReadTW88(REG0F9_FPLL1);
	dFPLL <<=8;
	dFPLL |= ReadTW88(REG0FA_FPLL2);

	return dFPLL;
}

static DWORD Sspll2GetFreqReg(void)
{
	DWORD dFPLL;
	
	dFPLL = ReadTW88(REG0E8)&0x0F;
	dFPLL <<=8;
	dFPLL |= ReadTW88(REG0E9);
	dFPLL <<=8;
	dFPLL |= ReadTW88(REG0EA);

	return dFPLL;
}

//-----------------------------------------------------------------------------
/**
* set SSPLL AnalogControl register
*
*	register
*	R0FD[7:6] POST
*	R0FD[5:4] VCO
*	R0FD[2:0] ChargePump
*/
static void Sspll1SetAnalogControl(BYTE value)
{
	WriteTW88(REG0FD_SSPLL_ANALOG, value );
}
static void Sspll2SetAnalogControl(BYTE value)
{
	WriteTW88(REG0ED, value );
}

//-----------------------------------------------------------------------------
/**
* get SSPLL Post value
*/
BYTE Sspll1GetPost(void)
{
	BYTE post;

	post = ReadTW88(REG0FD);
	return ((post>>6) & 0x03);
}
BYTE Sspll2GetPost(void)
{
	BYTE post;

	post = ReadTW88(REG0ED);
	return ((post>>6) & 0x03);
}

#if 1 //GOOD
//-----------------------------------------------------------------------------
/**
* get FPLL value from freq.
*
*
*	PLL Osc Freq = 108MHz * FPLL / 2^17 / 2^POST
*
*	FREQ			= 27000000 * 4 * FPLL / 2^17  / 2^POST
*
*   FPLL 			= FREQ *((2^15) * (2^POST)) / 27000000 			   			
*   FPLL 			= (FREQ / 1000) *((2^15) * (2^POST)) / 27000 			   			
*   FPLL 			= (FREQ / 1000) *((2^12) * (2^POST)) * (2^3  / 27000)
*   FPLL 			= (FREQ / 1000) *((2^12) * (2^POST)) / (3375) 			   			
*
* We divides 1000 first, the minimal offset is 1KHz unit.
*/
static DWORD SspllFREQ2FPLL(DWORD freq, BYTE post)
{
	DWORD fpll;
	fpll = freq/1000L;
	fpll <<= post;
	fpll <<= 12;
	fpll = fpll / 3375L;
	return fpll;
}
#endif
#if 0 //BAD
//-----------------------------------------------------------------------------
/*
	This method do multiply first. it can be overflow.
	and, divide by 421875. Big value means big lost.
    FPLL 			= FREQ *((2^15) * (2^POST)) / 27000000 			   			
	FPLL 			= FREQ *(32768 * (2^POST)) / 27000000
	FPLL 			= FREQ *(512 * (2^POST)) / 421875
    FPLL            = FREQ * 16 / 421875 * 32 * (2^POST)

static DWORD SspllFREQ2FPLL(DWORD FREQ, BYTE POST)
{
	DWORD FPLL;
	FPLL = FREQ * 16; 
	FPLL /= 421875;
	FPLL *= 32;
	FPLL <<= POST;
	return FPLL;
}
*/
#endif


static DWORD Sspll1ConvertFreq2FPLL(DWORD freq)
{
	BYTE post;
	post = Sspll1GetPost();
	return SspllFREQ2FPLL(freq,post);
}
static DWORD Sspll2ConvertFreq2FPLL(DWORD freq)
{
	BYTE post;
	post = Sspll2GetPost();
	return SspllFREQ2FPLL(freq,post);
}


//-----------------------------------------------------------------------------
/**
* Convert FPLL to Freq.
*
*	 Freq			= 27000000 * 4 * FPLL / 2^17  / 2^POST
*    Freq 			= 27000000 * FPLL / ( (2^15) * (2^POST) )
*    Freq 			= 27000000 * FPLL / ( (2^15) * (2^POST) )
*					= 421875 * 64 * FPLL / (64 * 2^9 *(2^POST))
*					= 421875 * FPLL / (512 *(2^POST))
*	   				= FPLL / 64 * 421875 / 8 / (2^POST)
    Simpilfied FREQ	= 824L * FPLL * 2^POST
*/
static DWORD SspllFPLL2FREQ(DWORD fpll, BYTE post)
{
	DWORD freq;
	freq = fpll >> 6; /* div 64 */
	freq *= 421875;
	freq >>= 3;       /* div 8 */
	freq >>= post;
	return freq;
}

static DWORD Sspll1ConvertFPLL2Freq(DWORD fpll)
{
	BYTE post;

	post = Sspll1GetPost();

	return SspllFPLL2FREQ(fpll, post);
}

static DWORD Sspll2ConvertFPLL2Freq(DWORD fpll)
{
	BYTE post;

	post = Sspll2GetPost();

	return SspllFPLL2FREQ(fpll, post);
}

//-----------------------------------------------------------------------------
/**
* set SSPLL freq and Pll
*
* @param _PPF ? What this measn ?
*			Freq
* @param fControl
*	1: update analog control registers
*/
void Sspll1SetFreq(DWORD freq, BYTE fControl)
{
	BYTE freq1m; 
	BYTE curr, vco, post;
	DWORD fpll;
	
	//dPrintf("\n\rSspll1SetFreq(%ld,%bd)",freq,fControl);
	freq1m = freq/1000000L;		//base:1MHz
	if (freq1m > 150)
		Puts("\nERROR:Max SSPLL speed is 150MHz");
	
	if (fControl == 0)
	{
		fpll = Sspll1ConvertFreq2FPLL(freq);	
		Sspll1SetFreqReg(fpll);
		
		return;
	}

	//----- Frequency Range --------------------
	if (freq1m < 27)
	{
		vco = 2;
		curr = 0;
		post = 2;
	}		// step = 0.5MHz
	else if (freq1m < 54)
	{
		vco = 2;
		curr = 1;
		post = 1;
	}		// step = 1.0MHz
	else if (freq1m < 108)
	{
		vco = 2;
		curr = 2;
		post = 0;
	}		// step = 1.0MHz
	else
	{
		vco = 3;
		curr = 3;
		post = 0;
	}		// step = 1.0MHz

	curr = vco + 1;	//BK110721. Harry Suggest.

	//----- Get pll register value from freq
	fpll = SspllFREQ2FPLL(freq, post);

	//----- Setting Registers
	Sspll1SetFreqReg(fpll);
	Sspll1SetAnalogControl((vco<<4) | (post<<6) | curr);
	//dPrintf("\n\rPOST:%bx VCO:%bx CURR:%bx",post, vco, curr);
}

void Sspll2SetFreq(DWORD freq, BYTE fControl)
{
	BYTE	freq1m; 
	BYTE    curr, vco, post;
	DWORD	fpll; //register value
	
	//dPrintf("\n\rSspll2SetFreq(%ld,%bd)",freq,fControl);
	freq1m = freq/1000000L;		//base:1MHz
	if(freq1m > 150)
		Puts("\nERROR:Max SSPLL2 speed is 150MHz");

	if(fControl==0) {
		fpll=Sspll2ConvertFreq2FPLL(freq);	
		Sspll2SetFreqReg(fpll);
		return;
	}

	//----- Frequency Range --------------------
	//this routine make FPLL as bigger than 54MHz and uses the post divider
	if     ( freq1m < 27 )  { vco=2; curr=0; post=2; }		// step = 0.5MHz
	else if( freq1m < 54 )  { vco=2; curr=1; post=1; }		// step = 1.0MHz
	else if( freq1m < 108 ) { vco=2; curr=2; post=0; }		// step = 1.0MHz
	else                    { vco=3; curr=3; post=0; }		// step = 1.0MHz
	curr = vco+1;	//BK110721. Harry Suggest.

	//----- Get pll register value from freq
	fpll = SspllFREQ2FPLL(freq, post);

	//----- Setting Registers
	Sspll2SetFreqReg(fpll);
	Sspll2SetAnalogControl((vco<<4) | (post<<6) | curr);
	//dPrintf("\n\rPOST:%bx VCO:%bx CURR:%bx",post, vco, curr);


	PclkSetDividerReg(0);	//div1:default
	
	if(freq1m > 108)
		Puts("\nERROR:Max PCLK speed is 108MHz");
		//You need 	PclkSetDividerReg(1);	//div2

}



//-----------------------------------------------------------------------------
/**
* get SSPLL Frequency.
*
*
*	FPLL = REG(0x0f8[3:0],0x0f9[7:0],0x0fa[7:0])
*	POST = REG(0x0fd[7:6])
*	PLL Osc Freq = 108MHz * FPLL / 2^17 / 2^POST
*/
DWORD Sspll1GetFreq(void)
{
	DWORD freq, fpll;

	//read PLL center frequency
	fpll = Sspll1GetFreqReg();
	//convert FPLL to Frequency value
	freq = Sspll1ConvertFPLL2Freq(fpll);
	//dPrintf("\r\n(GetPPF) :%ld", freq);

	return freq;
}

DWORD Sspll2GetFreq(void)
{
	DWORD freq, fpll;

	//read PLL center frequency
	fpll = Sspll2GetFreqReg();
	//convert FPLL to Frequency value
	freq = Sspll2ConvertFPLL2Freq(fpll);
	//dPrintf("\r\n(GetPPF) :%ld", freq);

	return freq;
}


//=============================================================================
// PCLK
//=============================================================================

//-----------------------------------------------------------------------------
/**
* set PCLK divider
*
* REG0F6[4]
* REG0F6[2:0]
* 
* parm:
*	divider
*	0:div1  	1=div1.5 	2=div2		3=div2.5	4=div3
*	5:div3.5	6:div4		7:div5		
*	8:div8		9:div16 	10:div32
* note:
*	REG0F6[3] always have 0.
*/
void PclkSetDividerReg(BYTE divider)
{
	BYTE bTemp;

	bTemp = ReadTW88(REG0F6) & ~0x1F;
	if(divider & 0x08) {
		divider &= 0x07;
		bTemp |= 0x10;
	}
	bTemp |= divider;
	WriteTW88(REG0F6, bTemp);
}
#if 0
/**
* REG0F6[3]
*/
void PclkSetPolarity(BYTE pol)
{
	BYTE bTemp;

	bTemp = ReadTW88(REG0F6);
	if(pol)	bTemp |=  0x08;
	else    bTemp &= ~0x08; 
	WriteTW88(REG0F6, bTemp);
}
#endif

//-----------------------------------------------------------------------------
/**
* get PCLK frequency
*
*/
DWORD PclkGetFreq(DWORD sspll)
{
	BYTE divider;
	DWORD temp32;
	BYTE bTemp;

	bTemp = ReadTW88(REG0F6);
	divider = bTemp & 0x03;
	if(bTemp & 0x10)
		divider |= 0x04;	

	switch(divider) {
	case 0:	temp32 = sspll;			break;
	case 1:	temp32 = sspll *2/3;	break; //div1.5
	case 2:	temp32 = sspll >>1;		break; //div2
	case 3:	temp32 = sspll *2/5;	break; //div2.5
	case 4:	temp32 = sspll /3;		break; //div3
	case 5:	temp32 = sspll *2/7;	break; //div3.5
	case 6:	temp32 = sspll >>2;		break; //div4
	case 7:	temp32 = sspll / 5;		break; //div5
	}
	return temp32;
}

//-----------------------------------------------------------------------------
/**
* get PCLKO frequency
*/
DWORD PclkoGetFreq(DWORD pclk)
{
	BYTE divider;
	DWORD temp32;
	BYTE bTemp;

	bTemp = ReadTW88(REG20D);
	divider = bTemp & 0x03;

	divider++;
	temp32 = pclk / divider;
	if(divider==3) {
		if(bTemp & 0x20)
			temp32 <<= 1;
	}
	return temp32;
}

//AN-TW8836B Scaler & TCON.
//only div2 and div1.5 have 0 value.
code BYTE pclok_pol_table[] = 
// 0    1    2    3    4    5    6      7
// div1 div2 div3 div4 div1 div2 div1.5 div4
{  1,   0,   1,   1,   1,   0,   0,     1};
#if 1
BYTE CheckPclko_polarity(void)
{
	BYTE pclko_div;
	pclko_div = PclkoReadDiv();
	return pclok_pol_table[pclko_div];
}
#endif
//-----------------------------------------------------------------------------
/**
* set PCLKO polarity
*
* @param pol - Pixel clock output polarity
*	-0:	no inversion
*	-1:	inversion
*	- 0xFF: do not change it. Use previous value
*/
void PclkoSetPolarity(BYTE pol)
{
	BYTE value;
	value = ReadTW88(REG20D);
	if(pol)	value |=  0x10;
	else	value &= ~0x10;
	WriteTW88(REG20D, value);
}

//-----------------------------------------------------------------------------
/**
* set PCLKO divider and CLK polarity
*
*	R20D[4]		Pixel clock polarity control.
*	R20D[5]		divider for 1.5 (Only work with div 3).
*	R20D[1:0]	Pixel clock output frequency division control.

* @param div - Pixel clock output frequency division
*	0:div 1,	1:div 2,	2:div 3,	3:div 4.
*	4:div 1,	5:div 2,	6:div 1.5,	7:div 4.
*
*/
void PclkoSetDiv(BYTE div)
{
	BYTE value;
	value = ReadTW88(REG20D) & ~0x23;
	if(div==6)
		value |= 0x20;
	div &= 0x03;
	value |= div;

	WriteTW88(REG20D, value);
}

void PclkoSetDiv_with_pol(BYTE div)
{
	BYTE value;
	BYTE pol;

	value = ReadTW88(REG20D) & ~0x33;
	if(div==6)
		value |= 0x20;
	div &= 0x03;
	value |= div;

	pol = pclok_pol_table[div];
	if(pol) 
		value |= 0x10;
	WriteTW88(REG20D, value);
}


BYTE PclkoReadDiv(void)
{
	BYTE value;
	value = ReadTW88(REG20D) & 0x23;
	if(value & 0x20) value |= 0x04;
	value &= 0x07;

	return value;
}

void PclkoPrintDiv(BYTE div)
{
	switch(div) {
	case 0: 
	case 4:	Puts("div1"); 	break;
	case 1:
	case 5: Puts("div2"); 	break;
	case 2: Puts("div3");	break;
	case 6: Puts("div1.5");	break;
	default: Puts("div4"); break;  //3,7
	}	
}



//=============================================================================
// PLLCLK
//=============================================================================

//-----------------------------------------------------------------------------
/**
* select PLLClk input
* @param 
*	0:SSPLL1 1:PLL108
*
* register
*	REG4E0[0]
*/
void PllClkSetSource(BYTE fPLL108)
{
	BYTE bTemp;

	bTemp = ReadTW88(REG4E0);

	if (fPLL108)
		bTemp |= 0x01;
	else
		bTemp &= ~0x01;

	WriteTW88(REG4E0, bTemp);
}

BYTE PllClkGetSource(void)
{
	BYTE bTemp;

	bTemp = ReadTW88(REG4E0);

	return (bTemp & 0x01);
}

//-----------------------------------------------------------------------------
/**
* set PLLClk divider
* only for monitor
* 
* register
*	REG4E1[3:0]
*/
void PllClkSetDividerReg(BYTE divider)
{
	WriteTW88(REG4E1, (ReadTW88(REG4E1) & 0xF0) | divider);	//CLKPLL Divider
}

//-----------------------------------------------------------------------------
/**
* set PLLClk input and divider
* 
* only for monitor
*/
//void PllClkSetSelDiv(BYTE ClkPllSel, BYTE ClkPllDiv)
//{
//	BYTE mcu_sel;
//	DWORD clkpll,spi_clk;
//	BYTE i=0;
//
//	//check & move MCU CLK source to 27M 
//	mcu_sel = McuSpiClkReadSelectReg();
//	if(mcu_sel==MCUSPI_CLK_PLLCLK) {
//		McuSpiClkSelect(MCUSPI_CLK_27M);
//		I2C_delay_base = 1;	
//	}
//	//
//	//Now, MCU uses 27M. You can change PLLClk register without a system hang.
//	//
//  
//	PllClkSetSource(ClkPllSel);
//	do {
//		PllClkSetDividerReg(ClkPllDiv);
//		ClkPllDiv++;
//		clkpll =PllClkGetFreq();
//		spi_clk=SpiClkGetFreq(clkpll);  //-->McuClkGetFreq
//		i++;
//	} while(spi_clk > 75000000L);	//MAX SPICLK
//	if(i!=1)
//		ePrintf("\n\rPllClkSetSelDiv div increased:%d",i-1);
//  
//	//restore MCU CLK source
//	if(mcu_sel==MCUSPI_CLK_PLLCLK)
//		McuSpiClkSelect(MCUSPI_CLK_PLLCLK);	
//}

//-----------------------------------------------------------------------------
/**
* get PLLClk frequency
*/
DWORD PllClkGetFreq(void)
{
	BYTE temp8;
	DWORD clkpll;
	DWORD temp32;

	//check PLLCLK_Sel.
	temp8 = PllClkGetSource();
	if (temp8)
		clkpll = 108000000L;		//comes from PLL108M.
	else
		clkpll = Sspll1GetFreq();	// comes from SSPLL1
		
	//read divider and calculate PLLCLK.
	temp8 = ReadTW88(REG4E1) & 0x0F;

	switch (temp8)
	{
	case 0:	temp32 = clkpll;		break;
	case 1:	temp32 = clkpll*2/3;	break; //1.5
	case 2:	temp32 = clkpll >> 1;	break; //2
	case 3:	temp32 = clkpll*2/5;	break; //2.5
	case 4:	temp32 = clkpll/3;		break; //3
	case 5:	temp32 = clkpll*2/7;	break; //3.5
	case 6:	temp32 = clkpll >> 2;	break; //4
	case 7:	temp32 = clkpll/5;		break; //5
	case 8:	temp32 = clkpll >> 3;	break; //8
	case 9:	temp32 = clkpll >> 4;	break; //16
	default:temp32 = clkpll >> 5;	break; //32
	}

	return temp32;
}

//=============================================================================
// MCUSPI
//=============================================================================

//-----------------------------------
// McuSpiClkToPclk & McuSpiClkRestore
//-----------------------------------

/*
result  register        stepA	stepB	stepC	stepD	stepE
------	--------		-----	-----	-----	-----	-----
fail	PCLK_SEL  		PLL				 		PCLK
		MCUSPI_CLK_SEL		PCLK	27M						PCLK
		MCUSPI_CLK_DIV		1		   		0				1

OK		PCLK_SEL  		PLL				 		PCLK
		MCUSPI_CLK_SEL		PCLK	27M						PCLK
		MCUSPI_CLK_DIV		1		   		 				1

OK		PCLK_SEL  		PLL				 		PCLK
		MCUSPI_CLK_SEL		PCLK
		MCUSPI_CLK_DIV		1
---------------------------------------------------------------
*/
//BYTE shadow_r4e0;
//BYTE shadow_r4e1;

//-----------------------------------------------------------------------------
/**
* select PCLK for McuSpi
*
* NOTE: SPIDMA needs a PCLK source
* @see McuSpiClkRestore
*
*
*/



//-----------------------------------------------------------------------------
/**
* restore MCUSPI clock
*
* @see McuSpiClkToPclk
*/


//-----------------------------------------------------------------------------
/**
* read SPI clock mode
*
*@return
*	0:27MHz
*	1:32KHz
*	2:PLLCLK
*
* register
*	REG4E1[5:4]
*/
BYTE SpiClkReadSource(void)
{
	BYTE value;
	value = ReadTW88(REG4E1) & 0x30;
	return (value >> 4);
}
void SpiClkSetSource(BYTE mode)
{
	BYTE value;
	BYTE cache;

	value = ReadTW88(REG4E1) & ~0x30;
	value |= mode;
	cache = SFR_CACHE_EN;
	SFRB_EA = 0;
	SFR_CACHE_EN=0;
	WriteTW88(REG4E1, value);
	SFR_CACHE_EN=cache;
	SFRB_EA = 1;
}

/*
* @param fPowerUpBoot
* @param fPrint
*/

void SpiClkRecover27MSource(void)
{
	BYTE temp;

    //Clock
	temp = ReadTW88(REG4E1);
	if (temp & 0x30)
	{
		Puts(" 27MHz");
		WriteTW88(REG4E1, temp & ~0x30);
	}

    //SPI Read Mode
	temp = ReadTW88(REG4C0);
	if (temp & 0x07)
	{
        Puts(" Slow");
		WriteTW88(REG4C0, temp & 0xF8);
		SPICMD_x_READ  = 0x03;	
		SPICMD_x_BYTES = 4;	//(8+24)/8
	}

	I2C_delay_base = 1;
}

//-----------------------------------------------------------------------------
/**
* Select McuSpi clock source
*
*	register
*	R4E1[5:4]
* @param McuSpiClkSel clock source
*	-0:27MHz
*	-1:32KHz
*	-2:CLKPLL. << (PLL Divider) << (PCLK or 108M) << (if PCLK, SSPLL with PCLK Divider)
*/
//void McuSpiClkSelect(BYTE McuSpiClkSel)
//{
//#if defined(PANEL_AUO_B133EW01) || defined(PANEL_TM070DDH01)
//	//I will use SSPLL. Do not change MCU clock.
//	BYTE value=McuSpiClkSel;
//#else
//	BYTE value;
//	BYTE cache;
//	cache =  SFR_CACHE_EN;
//	SFR_CACHE_EN = 0;
//	value = ReadTW88(REG4E1) & 0x0F;
//
//	WriteTW88(REG4E1, (McuSpiClkSel << 4) | value);
//	SFR_CACHE_EN = cache;
//#endif
//} 

//-----------------------------------------------------------------------------
/**
* get Spi clock frequency
*/
DWORD SpiClkGetFreq(void)
{
	BYTE temp8;
	DWORD temp32;

	temp8 = ReadTW88(REG4E1) >> 4;
	temp8 &= 0x03;

	switch (temp8)
	{
	case 0:  temp32 = 27000000L;		break;
	case 1:	 temp32 = 32000L;			break;
	case 2:	 temp32 = PllClkGetFreq();	break;
	default: temp32 = 27000000L;		break;	//unknown. bug.
	}

	return temp32;
}

/**
* Get MCU clock from SpiClk
* @param spiclk
*/
DWORD McuClkGetFreq(DWORD spiclk)
{
	BYTE temp;
	DWORD temp32;

	temp = ReadTW88(REG4F0) >> 4;
	switch (temp)
	{
	case 0:	temp32 = spiclk;		break;
	case 1: temp32 = spiclk *2/3;	break;
	case 2: temp32 = spiclk / 2;	break;
	case 3: temp32 = spiclk *2/5;	break;
	case 4: temp32 = spiclk / 3;	break;
	case 5: temp32 = spiclk *2/7;	break;
	case 6: temp32 = spiclk / 4;	break;
	case 7: temp32 = spiclk / 5;	break;
	}
	
	return temp32;
}

//=============================================================================
// SPI CLK
//=============================================================================

//-----------------------------------------------------------------------------
/**
* get SPI clock frequency
*/
//DWORD SpiClkGetFreq__OLD(DWORD mcu_clk)
//{
//	BYTE divider;
//	DWORD temp32;
//	divider = ReadTW88(REG0F6) >> 4;
//	divider++;
//	temp32 = mcu_clk / divider;
//	return temp32;
//}

//=============================================================================
// ASYNC/SYNC SPI CLK
//=============================================================================

void SpiClk_SetSync(void)
{ 	
	BYTE r4e1;

//Puts("\n SpiClk_SetSync");
	r4e1 = ReadTW88(REG4E1);	
	if (r4e1 & 0x20)				  					//I don't care 32K and unknown
		WriteTW88(REG4E1, r4e1 & ~0x30);			//select 27MHz first.
	
	WriteTW88(REG4F2, ReadTW88(REG4F2) & ~0x80);	//off ASYNC
	WriteTW88(REG4F0, ReadTW88(REG4F0) & 0x0F);		//set MCU divider as 1.

	//-----------
	// make effect.
	WriteTW88(REG4E1, r4e1 | 0x20);					//select PLLCLK.  
}

/**
* example
*	#define MCU_DIV_2		2
*	bWait = SpiClk_GetMinAsyncWaitValue(MCU_DIV_2);
*	SpiClk_SetAsync(MCU_DIV_2,bWait, ON,ON);
*/
void SpiClk_SetAsync(BYTE mcu_div, BYTE wait, BYTE fSpiOSD, BYTE fMcuFetch)
{
	BYTE r4e1;
	BYTE bTemp;

	r4e1 = ReadTW88(REG4E1);	
	if (r4e1 & 0x20)								//I don't care 32K and unknown
		WriteTW88(REG4E1, r4e1 & ~0x30);			//select 27MHz first.

	WriteTW88(REG4F2, ReadTW88(REG4F2) | 0x80);		//on ASYNC

	bTemp = ReadTW88(REG4E0) & 0x01;
	WriteTW88(REG4E0, bTemp | (wait <<1));			//set wait value

	bTemp = ReadTW88(REG4DF) & ~0x08;
	if (fSpiOSD)
		WriteTW88(REG4DF, bTemp | 0x08);			//set SPIOSD_WAIT_OFF
	
	bTemp = ReadTW88(REG4F0) & ~0x08;
	if (fMcuFetch)
		WriteTW88(REG4F0, bTemp | 0x08);			//set MCUFETCH_WAIT_OFF	
	
	bTemp = ReadTW88(REG4F0) & 0x0F;
	WriteTW88(REG4F0, bTemp | (mcu_div << 4));		//set MCU divider

	//-----------
	// make effect.
	WriteTW88(REG4E1, r4e1 | 0x20);					//select PLLCLK.  
}

BYTE SpiClk_GetMinAsyncWaitValue(BYTE mcu_div)
{
	BYTE ret;
	
	switch (mcu_div)
	{
	case 0:	ret = 1;	break;	//div1
	case 1: ret = 2;	break;	//div1.5
	case 2: ret = 3;	break;	//div2
	case 3: ret = 5;	break;	//div2.5
	case 4: ret = 6;	break;	//div3
	case 5: ret = 7;	break;	//div3.5
	case 6: ret = 9;	break;	//div4
	default: ret = 9;	break;
	}

	return ret;
}

//-----------------------------------------------------------------------------
/**
* Set SPI input clock latch.
* To support a high speed SPI clock, it needs a positive edge & 1 Cycle Delay.
*@param property
*	00b - TW8836 default.(SPIFLASH spec. for READ)
*   01b - one cycle delay.
*   10b - positive edge..
*   11b - positive edge, one cycle delay. for HighSpeed.
*
*	*-----+		+-----+		+-----+		+-----+
*	|	  |		|	  |		|	  |		|	  |
*   +	  +-----+	  +-----+	  +-----+	  +-----
*	        -----------	----------- -----------
*	       X    (A)    X    (B)	   X    (C)    X    SPI Flash SPEC.		  
*           -----------	-----------	-----------
*					  *           *					case 00b read position.		  							  
*                     |			  |                 	27M~72MHz success. 
*													    PCLK 108MHz working.~ 109Mhz.
*                                                   	109,265KHz failed.             
*					        *           * 			case 11b read position     
*					        |			|           	27M~72MHz Fail.    
*														108MHz success. 
*														133M success(2,118,132)
*													    133M is max on MX chip.
*                                                   
* <<assume the response is slow on the high speed clock>>
*	*-----+		+-----+		+-----+		+-----+
*	|	  |		|	  |		|	  |		|	  |
*   +	  +-----+	  +-----+	  +-----+	  +-----
*	        -----------	----------- -----------
*	       X    (A)    X    (B)	   X    (C)    X    SPI Flash SPEC.		  
*           -----------	-----------	-----------
*               \		\			\
*                \		 \			 \
*	                ----------- -----------
*	        XXXXXXXX   (A)     X	 (B)     X					  
*                   ----------- -----------									  
*
*
*	*-----+		+-----+		+-----+		+-----+
*	|	  |		|	  |		|	  |		|	  |
*   +	  +-----+	  +-----+	  +-----+	  +-----
*	        -----------	----------- -----------
*	       X    (A)    X    (B)	   X    (C)    X    SPI Flash SPEC.		  
*           -----------	-----------	-----------
*                *          *                    	case 10b read position.
*                |			|					    	27M~36MHz success. 	  
*														54MHz~108MHz fail	
*														PCLK 54MHz Fail.
*                                                       It is for DEdge.																			  
*							      *           *		case 01b read position  
*							      |           |     	always fail.      
*													
*
* To support 133MHz, 
* 	Select 27MHz.
*	Set SSPLL as  0x02,0x00,0x00.(108MHz)
*   Select PCLK with div 1.5.
*
*	Select CLKPLL, div1.0 + SpiInputClockLatch as 11b.(write 4E1=0xE0);
*	Set SSPLL as  0x02,0x76,0x84.
*
* To move back 27MHz
*	Set SSPLL as  0x02,0x00,0x00.(108MHz)
*	Select CLKPLL, div1.5 + SpiInputClockLatch as 00b.(write 4E1=0x21);
*	Select 27MHz(write 4E1=0x01).
*	
*/

/**
*
* REG4E1[7:6]
*/
BYTE SpiClk_overclocking(BYTE fOn)
{
	DWORD spiclk;
	BYTE r4e1;
	BYTE bTemp;

	if (spiflash_chip->mid != SPIFLASH_MID_MX)
		return 1; //0xFF;	//incorrect spiflash vendor

	bTemp = ReadTW88(REG4F2);
	if (bTemp & 0x80)
		return 2; //0xFE;	//async can not support

	spiclk = SpiClkGetFreq();
	if (spiclk < 72000000)
		return 3; //0xFD;	//to slow

	spiclk = SpiClkGetFreq();
	if (spiclk > 108000000)
		return 4; //0xFC;	//to high

	r4e1 = ReadTW88(REG4E1);
	if (fOn)
		WriteTW88(REG4E1, r4e1 |  0xC0);
	else
		WriteTW88(REG4E1, r4e1 & ~0xC0);
			
	return 0; //success	
}

//=============================================================================
// LLPLL
//=============================================================================

//-----------------------------------------------------------------------------
/**
* set LLPLL clock source
*
* use 27M OSD or PLL 
*/
void aRGBSetClockSource(BYTE use_27M)
{
	if (use_27M)
		WriteTW88(REG1C0, ReadTW88(REG1C0) | 0x01); 
	else
		WriteTW88(REG1C0, ReadTW88(REG1C0) & ~0x01);
}

//-----------------------------------------------------------------------------
/**
* print current clock info
*/
void DumpClock(void)
{
	DWORD sspll;
	DWORD pll_clk, spi_clk, mcu_clk;
	DWORD pclk, pclko;

	sspll  = Sspll1GetFreq();
	pll_clk = PllClkGetFreq();
	spi_clk = SpiClkGetFreq();
	mcu_clk = McuClkGetFreq(spi_clk);
	
    sspll += 500000;        sspll /= 1000000;
    pll_clk += 500000;      pll_clk /= 1000000;
    spi_clk += 500000;      spi_clk /= 1000000;
    mcu_clk += 500000;      mcu_clk /= 1000000;

	Printf("\n\rCLOCK SSPLL1:%ldM PLLCLK:%ldM SPI:%ldM MCU:%ldM I2C:%bd", sspll, pll_clk, spi_clk, mcu_clk, I2C_delay_base);

	sspll = Sspll2GetFreq();
	pclk   = PclkGetFreq(sspll);
	pclko  = PclkoGetFreq(pclk);
    sspll += 500000;        sspll /= 1000000;
    pclk += 500000;      pclk /= 1000000;
    pclko += 500000;      pclko /= 1000000;
	
	Printf("\n\r      SSPLL2:%ldM PCLK:%ldM PCLKO:%ldM", sspll, pclk, pclko);

	if (ReadTW88(REG20D) & 0x10)
		Puts(" Pol:1");
	else
		Puts(" Pol:0");
}

//=============================================================================
// InMux (Input Mux)
//=============================================================================

//-----------------------------------------------------------------------------
/**
* set input mux format
*
* register
*	R102 - input format.
*	R105.
*	R106.
* @param InputMode
*/
//-----------------------------------------------------------------------------
/**
* set input mux format for AnalogModule(DEC and aRGB)
*
* register
*	R102 - input format.
*	R105.
*	R106.
* @param InputMode
*/

/*
Set Analog Mux.
OLD R105 moved to R1E8.
*/
void AMuxSetInput(BYTE InputMode)
{
	BYTE r102, r106, r1E8;

	r1E8 = ReadTW88(REG1E8) & 0xF0;
	r106 = ReadTW88(REG106) & ~0x03;	//Do not change Y.
										
//	r1C0 = ReadTW88(REG1C0) & ~0x01; //ADC Clock
	
	switch(InputMode) {
	case INPUT_CVBS:
	case INPUT_BT656:		//it is for loopback from DEC->BT656Enc->DTV)BT656Dec.
		r102 = 0x40;		// 0x40 - FC27:27MHz, IFSEL:Composite, YSEL:YIN0 
		r1E8 |= 0x0F;		//decoder mode
		r106 |= 0x03;		// C,V adc in Power Down.
		break;
	case INPUT_SVIDEO:
		r102 = 0x58;		// 0x54	- FC27:27MHz, IFSEL:S-Video, YSEL:YIN2, CSEL:CIN0 
		r1E8 |= 0x0F;		//decoder mode
		r106 |= 0x01;		// V in PowerDown
		break;
	case INPUT_COMP:	//target r102:4A,r105:04 r016:00
						//     ->     4A      00      00		  
		r102 = 0x4E ;		// TW8836 EVB10 use Y3.
		r1E8 |= 0x20;		// disable YOUT. it shares pin with Y3.
		break;
	case INPUT_PC:	//target r102:4A r105:04 r106:00
		r102 = 0x4E ;		// TW8836 EVB10 use Y3.
		r1E8 |= 0x20;		// disable YOUT. it shares pin with Y3.
		break;
	case INPUT_DVI:			//target ? don't care
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
	case INPUT_LVDS:
		//digital. don't care.
		r102 = 0x00;
		break;
	}
	if(r102) {	//need update?
		WriteTW88(REG102, r102 );
		WriteTW88(REG1E8, r1E8 );
		WriteTW88(REG106, r106 );
	}
}



//=============================================================================
//				                                               
//=============================================================================
//global


/*
//==========================
// GPIO EXAMPLE
//==========================
//!GPIO_EN	Enable(active high)
//!GPIO_OE	Output Enable(active high)
//!GPIO_OD	Output Data
//!GPIO_ID	Input Data
//!
//!		GPIO_EN	GPIO_OE	GPIO_OD	GPIO_ID
//!GPIO0	R080	R088	R090	R098
//!GPIO1	R081	R089	R091	R099
//!GPIO2	R082	R08A	R092	R09A
//!GPIO3	R083	R08B	R093	R09B
//!GPIO4	R084	R08C	R094	R09C
//!GPIO6	R085	R08D	R095	R09D
//!GPIO7	R086	R08E	R096	R09E
//!
//!bit readGpioInputData(BYTE gpio, BYTE b)
//!{
//!	BYTE reg;
//!	reg = 0x98+gpio;
//!	value = ReadTW88(reg);
//!	if(value & (1<<b))	return 1;
//!	else				return 0;
//!}
//!bit readGpioOutputData(BYTE gpio, BYTE b)
//!{
//!	BYTE reg;
//!	reg = 0x90+gpio;
//!	value = ReadTW88(reg);
//!	if(value & (1<<b))	return 1;
//!	else				return 0;
//!}
//!void writeGpioOutputData(BYTE gpio, BYTE b, BYTE fOnOff)
//!{
//!	BYTE reg;
//!	reg = 0x90+gpio;
//!	value = ReadTW88(reg);
//!	if(fOnOff) value |= (1<<b);
//!	else       value &= ~(1<<b);
//!	WriteTW88(reg,value);
//!}
*/
	


//
//BKFYI110909.
//	We merge step0 and step1, and check the status only at step2.
//	
//	step0 check_status : OK
//	step1 check_status : OK
//	step2 check_status : OK
//
//  step0 & step1 check_status : fail 20%
//	step2         check_status : OK
//-------------------------------------- 

//=============================================================================
// DCDC
//=============================================================================


//-----------------------------------------------------------------------------
/**
* turn on DCDC
*/


//=============================================================================
// GPIO EXPANDER
//=============================================================================
#define SX1504_DATA_REG			0	//data register index
#define SX1504_DIR_REG			1	//direction register index
#define SX1504_DIR_IN			1	//input direction
#define SX1504_DIR_OUT			0	//output direction
#define SX1504_PIN1_FP_PWC		0
#define SX1504_PIN0_FP_BIAS		1
#define SX1504_PIN4_LVDSTX		4
#define	SX1504_PIN5_HDMI_RSTB 	5	//=>HDMI_TW8837.c

#if defined(SUPPORT_I2C_MASTER)
BYTE UpdateGpioExpanderPin(BYTE pin, BYTE dir, BYTE value)
{
	BYTE mask;
	BYTE bRegData;
	BYTE bRegDir;

	mask = 1;
	mask <<= pin;

	if (dir == SX1504_DIR_IN)
	{
		//read..
		bRegDir  = ReadI2CByte(I2CID_SX1504, SX1504_DIR_REG);
		bRegDir |= mask;
		WriteI2CByte(I2CID_SX1504, SX1504_DIR_REG, bRegDir);

		bRegData = ReadI2CByte(I2CID_SX1504, SX1504_DATA_REG);

		return bRegData & mask ? 1 : 0;
	}
	else
	{
		//write..
		bRegData = ReadI2CByte(I2CID_SX1504, SX1504_DATA_REG);
		bRegDir  = ReadI2CByte(I2CID_SX1504, SX1504_DIR_REG);
		bRegDir &= ~mask;

		if (value)
			bRegData |= mask;
		else
			bRegData &= ~mask;

		WriteI2CByte(I2CID_SX1504, SX1504_DIR_REG, bRegDir);	//write direction first,
		WriteI2CByte(I2CID_SX1504, SX1504_DATA_REG, bRegData);	//and then write data.
		
		return 0;
	}
}
#endif

//GPIO43 or expender GPIO[1]
//-----------------------------------------------------------------------------
/**
* set FP_Bias
*/
void FP_BiasOnOff(BYTE fOn)
{
	Printf("\n\rFP_Bias %s", fOn ? "On" : "Off");

#ifdef MODEL_TW8836DEMO
	Puts("  -->skip");
	return;
#endif

#if defined(SUPPORT_I2C_MASTER)
	UpdateGpioExpanderPin(SX1504_PIN0_FP_BIAS, SX1504_DIR_OUT, fOn ? 0 : 1);
#endif
}

//FrontPanel PowerControl ON - GPIO42. or expender GPIO[0]
//-----------------------------------------------------------------------------
/**
* set FP_PWC
*/
void FP_PWC_OnOff(BYTE fOn)
{
#ifdef MODEL_TW8836DEMO
	BYTE temp;
	Printf("\n\rFP_PWC %s",fOn ? "On" : "Off");
	temp = ReadTW88(REG008) & 0xF0;
	if(fOn) temp |= 0x0B; //<=0;
	else	temp |= 0x0C; //<=1 
	WriteTW88(REG008,  temp); 
	return;
#endif
	
	Printf("\n\rFP_PWC %s", fOn ? "On" : "Off");

#if defined(SUPPORT_I2C_MASTER)
	UpdateGpioExpanderPin(SX1504_PIN1_FP_PWC, SX1504_DIR_OUT, fOn ? 0 : 1);
#endif
}

//-----------------------------------------------------------------------------
/**
* set GPIO for FP
*/
//void FP_GpioDefault(void)
void InitGpioDefault(void)
{
	//IR uses PORT1_4(GPIO40). To disable TCPOLN output, we need to enable it as GPIO and uses it as input port with INT11.
	WriteTW88(REG084, 0x01);	//GPIO 4x Enable     	- GPIO40 enable
	WriteTW88(REG08C, 0x00);	//GPIO 4x direction		- GPIO40 input
	WriteTW88(REG094, 0x00);	//GPIO 4x output data	- GPIO40 outdata as 0.
#ifdef MODEL_TW8836DEMO
	//use GPIO41 for EN_LVDS_RX
	WriteTW88(REG084, 0x03);	//GPIO 4x Enable     	- GPIO41 enable
	WriteTW88(REG08C, 0x02);	//GPIO 4x direction		- GPIO41 Output
	WriteTW88(REG094, 0x00);	//GPIO 4x output data	- GPIO41 outdata as 0.
#endif
}

//Enable SN65LVDS93A by GPIO Expander IO[4].
void EnableExtLvdsTxChip(BYTE fOn)
{
#ifdef MODEL_TW8836DEMO
	Printf("\n\rExtLvdsTx %s", fOn ? "On" : "Off");

	//P1_5 = fOn;
	if (fOn)
		WriteTW88(REG094, ReadTW88(REG094) |  0x02);
	else
		WriteTW88(REG094, ReadTW88(REG094) & ~0x02);
	
	return;
#endif

	if (fOn)
		Printf("\n\rExtLvdsTx %s", fOn ? "On" : "Off");

#if defined(SUPPORT_I2C_MASTER)
	UpdateGpioExpanderPin(SX1504_PIN4_LVDSTX, SX1504_DIR_OUT, fOn ? 1 : 0);
#endif
}

//yes, it is gone on TW8836...but, FW miss it.
//==============================================================================
// void	LedBackLight( BYTE on )
//==============================================================================
/**
* control LEDC digital block
*
*	register
*	REG0E0[0]
*/
void LedBackLight( BYTE on )
{
	WaitVBlank(1);
	if ( on )	WriteTW88(REG0E0, ReadTW88(REG0E0 ) | 1 );
	else		WriteTW88(REG0E0, ReadTW88(REG0E0 ) & ~0x01 );
}


//-----------------------------------------------------------------------------
/**
* power up BackLight LED.
*/
void PowerUpLedBackLight(void)
{
	volatile BYTE bTemp;

	Printf("\n\rLED Powerup");
#if defined(SUPPORT_I2C_MASTER)
	//WaitVBlank(1);

	WriteI2CByte(I2CID_ISL97671A, 0x01, 0x05);
	bTemp = ReadI2CByte(I2CID_ISL97671A,0x01);
	if ((bTemp & 0x05) != 0x05)	{
		Printf(" ID:58 Idx:1 W:0x05 R:%bx", bTemp);

		//retry.
		WriteI2CByte(I2CID_ISL97671A, 0x01, 0x05);
		bTemp = ReadI2CByte(I2CID_ISL97671A,0x01);
		if ((bTemp & 0x05) != 0x05)	{
			Printf(" W:0x05 R:%bx", bTemp);
		}
	}
#endif
}

//-----------------------------------------------------------------------------
/**
* enable BT656 output
*/
void BT656EncOutputEnable(BYTE fOn, BYTE clear_port)
{
#ifdef SUPPORT_UART1
	Printf("\n\rBUGBUG:BT656 and UART1..");
#endif

	if(fOn) {
		WriteTW88(REG007, ReadTW88(REG007) | 0x08);	 
	}
	else {
		WriteTW88(REG007, ReadTW88(REG007) & ~0x08); 
		//clear port
		if(clear_port) {
#if 0
			if(P1_6 == 0)
				P1_6 = 1;
#endif
		}
	}
}


//-----------------------------------------------------------------------------
/**
* enable Output pin
*
* DataOut need EnableOutputPin(ON,ON)
* target R008 = 0x89
*/
void OutputEnablePin(BYTE fFPDataPin, BYTE fOutputPin)
{
	BYTE value;

	wPrintf("\n\rOutputPin:%s FP_Data:%s",fOutputPin ? "On" : "Off",fFPDataPin ? "On" : "Off");

	value = ReadTW88(REG008) & ~0x30;
	if(fFPDataPin==0) 		value |= 0x20;
	if(fOutputPin==0)		value |= 0x10;
	WriteTW88(REG008,  value);
}



//-----------------------------------------------------------------------------
/**
* init default NTSC value
*/

//-----------------------------------------------------------------------------
/**
* turn on DCDC
*
*	DCDC startup step
*	SSPLL ON
*	FP PWC ON
*	call DCDC_on(0) & DCDC_on(1)
*	delay
*	wait VBlank
*	Enable FP Data Out
*	delay
*	call DCDC_on(2)
*	FP Bias On
*
* DCDC data out needs more then 200ms delay after Sspll1PowerUp(ON).
*/
BYTE FrontPanel_StartUP_sub(void)
{
	BYTE ret;

	//-------------
	//FPPWC ON
	FP_PWC_OnOff(ON);

	WaitVBlank(1);
	//-------------
	//FP Data Out
	Puts("\nEnable PanelOutput");
	OutputEnablePin(ON,ON);		//Output enable. FP data: enable

	ret=ERR_SUCCESS;

	//-------------
	//FPBIAS ON 
	FP_BiasOnOff(ON);

	//disable Blank
	//WriteTW88(REG21E, ReadTW88(REG21E) & ~0x01);

	if(ret!=ERR_SUCCESS)
		Puts(" FAIL");
	return ret;
}

//-----------------------------------------------------------------------------
/**
* turn on DCDC
*
* @see FrontPanel_StartUP_sub
*/
BYTE FrontPanel_StartUP(void)
{
	BYTE ret;
	
	ret=FrontPanel_StartUP_sub();
	if(ret == ERR_SUCCESS)
		return ERR_SUCCESS;

	ret=FrontPanel_StartUP_sub();
	if(ret == ERR_SUCCESS)
		return ERR_SUCCESS;

	ret=FrontPanel_StartUP_sub();
	return ret;
}


#pragma SAVE
#pragma OPTIMIZE(2,SPEED)
/**
* this issue was happed on 1920x1080I@25.
* FW assign vFreerun as 624. 625 was good.
*/
WORD ReadStableFreerunHtotal(WORD threshold)
{
	BYTE i;
	WORD hTotal;
	for(i=0; i < 20; i++) {
		hTotal = ScalerReadFreerunHtotal();
		if(hTotal > threshold) 
			break;

		WaitVBlank(1);
	}
	if(i) {
		Printf("\n%bd ",i);
		Printf("hTotal:%d(0x%04x)", hTotal,hTotal);
	}
	return hTotal;
}

BYTE AdjustSSPLL_with_HTotal(void)
{
	WORD hTotal;
	DWORD sspll;
	DWORD target;
	WORD input_vlen;
	BYTE pclko_div;
	//int i;
	WORD htotal_check_value;

	Puts("\nAdjustSSPLL_with_HTotal");

	Printf("\n\tSSPLL2:%ld ",Sspll2GetFreq());
	PclkoPrintDiv(PclkoReadDiv());

	target = ScalerReadFreerunVtotal();	
	target *=PANEL_H_TYP;
	target *= MeasGetVFreq();
	Printf("\n\ttarget %ldHz",target);
	//htotal_check_value = target;

	input_vlen = InputGetVLen();
	if(input_vlen >= 1024) {
		target *= 2;
		pclko_div = 1;	//div2
	}
	else if(input_vlen >= 720) {
		target /= 2;
		target *= 3;
		pclko_div = 6; //1.5	
	}
	else if(input_vlen >= 540) {
		target /= 2;
		target *= 3;
		pclko_div = 6; //1.5	
	}
	else {
		pclko_div = 0;	//div1
	}
	if(pclko_div) {
		Printf("->%ldHz ",target);
		PclkoPrintDiv(pclko_div);
	}

	Sspll2SetFreq(target,ON); //BK131120
	PclkoSetDiv(pclko_div);
	Printf(" %02bx",ReadTW88(REG0E8) & 0x0F);
	Printf(" %02bx",ReadTW88(REG0E9));
	Printf(" %02bx",ReadTW88(REG0EA));
	Puts("  ");
	PclkoPrintDiv(PclkoReadDiv());
	WaitVBlank(2);
	//WaitVBlank(2);	//for 1080i@50

	//read HTotal and compare it with Panel Spec.
	htotal_check_value = PANEL_H_MIN - 100;
	hTotal = ReadStableFreerunHtotal(htotal_check_value);
	//Printf("\n%bd hTotal:%d(0x%x)", i, hTotal,hTotal);
	//Printf("\n%bd ",i);
	Printf("\n\thTotal:%d(0x%04x)", hTotal,hTotal);
	Printf(" MIN:%d TYP:%d MAX:%d",PANEL_H_MIN,PANEL_H_TYP,PANEL_H_MAX);
	if(hTotal >= PANEL_H_MIN && hTotal <= PANEL_H_MAX)
		return 0;

	Printf("...checking");
	if(hTotal < PANEL_H_MIN) {
		//increase SSPLL
		//Printf("+...");
		Puts("+");
		while(1) {
			//reas SSPLL and increase.
			sspll = Sspll2GetFreqReg();
			sspll += 0x100;
			Sspll2SetFreqReg(sspll);
			WaitVBlank(2);
			//read HTotal and compare it with PANEL_H_TYP
			//hTotal = ScalerReadFreerunHtotal();
			hTotal = ReadStableFreerunHtotal(htotal_check_value);
			//Printf("\n %ld %d",sspll,hTotal);
			Puts(".");					
			if(hTotal >= PANEL_H_TYP)
				break;
		}
	}
	else {
		//decrease SSPLL
		//Printf("-...");
		Puts("-");
		while(1) {
			//reas SSPLL and decrease.
			sspll = Sspll2GetFreqReg();
			sspll -= 0x100;
			Sspll2SetFreqReg(sspll);
			WaitVBlank(2);
			//read HTotal and compare it with PANEL_H_MAX
			//hTotal = ScalerReadFreerunHtotal();
			hTotal = ReadStableFreerunHtotal(htotal_check_value);
			//Printf("\n %lx %d",sspll,hTotal);		
			Puts(".");
			if(hTotal < PANEL_H_MAX)
				break;	
		}
	}

	//Print new SSPLL & PCLKO value.
	Printf("=>%ld",Sspll2GetFreq());
	return 0;
}
#pragma RESTORE

/**
* check SpiClock for overclocking..
*
* It will destory the normal clock setting.
* You have to reset HW if it is failed.
*/
BYTE CheckSpiClock(BYTE max)
{
	DWORD clock;
	BYTE i,j;
	BYTE good;
	DWORD spiaddr;
	WORD size;
	BYTE saved_DebugLevel;
	BYTE ret;	

	saved_DebugLevel = DebugLevel;
	DebugLevel = 0;

	Puts("\nCheck spi clock..");
	InitClockAsDefault(0);	//select SYNC+SSPLL
	
	delay1ms(500);

	spiaddr = 0x60000;	//use blank area
	size = 8;

	SpiFlash_SectorErase(spiaddr);

	SPI_Buffer[0] = 0x01;
	SPI_Buffer[1] = 0x23;
	SPI_Buffer[2] = 0x45;
	SPI_Buffer[3] = 0x67;
	SPI_Buffer[4] = 0x89;
	SPI_Buffer[5] = 0xAB;
	SPI_Buffer[6] = 0xCD;
	SPI_Buffer[7] = 0xEF;
	SpiFlash_PageProgram_XMem( spiaddr, (WORD)SPI_Buffer, (WORD)size);

	PllClkSetDividerReg(0);
	ret = SpiClk_overclocking(ON);
	if(ret) {
		Printf("fail(%bx)",ret);
		return ret;
	}

	clock = 108000000L;
	good = 0;
	Puts("\n");
	for(i=8; i <= max; i++) {
		Puts("\r                                        ");
		Printf("\r%dMHz..", (WORD)(clock / 1000000L));
		Sspll1SetFreq(clock,1);

		//XMDM DMA test
		TWmemset(SPI_Buffer, 0x00, 8); //clear buffer
		SpiFlash_Read_XMem(spiaddr, (WORD)SPI_Buffer, 8/*cnt*/);
		//SpiFlashDmaRead2XMem(SPI_Buffer, spiaddr, 8/*cnt*/, SPIFLASH_WAIT_READ); 
		if(SPI_Buffer[0] != 0x01
		|| SPI_Buffer[1] != 0x23
		|| SPI_Buffer[2] != 0x45
		|| SPI_Buffer[3] != 0x67
		|| SPI_Buffer[4] != 0x89
		|| SPI_Buffer[5] != 0xAB
		|| SPI_Buffer[6] != 0xCD
		|| SPI_Buffer[7] != 0xEF
		) {
			break;
		}
		Puts("XMEM ");

		//SPIOSD LUT test
		SpiOsdEnable(ON);
		for(j=0; j<3; j++) {
			ret=MonOsdLutLoad(j/*img_n*/,3, 0/*lut*/); //WIN3,LUT:0
			if(ret)
				break;
		}
		SpiOsdEnable(OFF);
		if(j!=3)
			break;
		Puts("SOSD ");
		

		Puts("..checked");
		good = i;

		clock += 1000000L;	//add 1MHz
	}
	Puts("\n");
	Sspll1SetFreq(clock,1);
	SpiClk_overclocking(OFF);
	InitClockAsDefault(0);	//select SYNC+SSPLL

	if(good) {
		//if(good > 32)
		//	good = 32;
		EE_Write(EEP_AUTODETECT, good);	
	}
	DebugLevel = saved_DebugLevel;


	return 0;
}

/* eOn SPI Flash
	minimum for Input Page	61.383MHz	1 23 00 	
	maximum for Main Page	68.976MHz	1 47 00	
*/