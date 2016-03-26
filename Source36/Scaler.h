/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/
#ifndef __SCALER_H__
#define __SCALER_H__

#define VIDEO_ASPECT_NORMAL		0
#define VIDEO_ASPECT_ZOOM		1
#define VIDEO_ASPECT_FILL		2
#define VIDEO_ASPECT_PANO		3
extern BYTE VideoAspect;

#define HPOL_P		0x40	//Horizontal Polarity Positive. Active High.   _|-|___.
#define HPOL_N		0x00	//Horizontal Polarity Negative. Active Low.    -|_|---.

#define VPOL_P		0x80
#define VPOL_N		0x00

#define INTERLACED	0x01

#define P_HN_VN		HPOL_N | VPOL_N
#define P_HP_VP		HPOL_P | VPOL_P	
#define P_HN_VP		HPOL_N | VPOL_P
#define P_HP_VN		HPOL_P | VPOL_N
#define I_HN_VN		HPOL_N | VPOL_N | INTERLACED
#define I_HP_VP		HPOL_P | VPOL_P | INTERLACED	
#define I_HN_VP		HPOL_N | VPOL_P | INTERLACED
#define I_HP_VN		HPOL_P | VPOL_N | INTERLACED

struct DEC_VIDEO_TIME_TABLE_s {
	WORD hActive,vActive;
	BYTE vFreq;
	BYTE hDelay, vDelay, vZeroPointOffset;
	BYTE hOverScan, vOverScan;	//2 is enough value.
	BYTE pol;					//scaler input clock polarity
};
//extern code struct DEC_VIDEO_TIME_TABLE_s TW8836_DEC_TABLE[];


#define MEAS_COMP_MODE_480I			0
#define MEAS_COMP_MODE_576I			1
#define MEAS_COMP_MODE_480P			2
#define MEAS_COMP_MODE_576P			3
#define MEAS_COMP_MODE_1080I25		4
#define MEAS_COMP_MODE_1080I30		5
#define MEAS_COMP_MODE_720P50		6
#define MEAS_COMP_MODE_720P60		7
#define MEAS_COMP_MODE_1080P50		8
#define MEAS_COMP_MODE_1080P60		9
struct COMP_VIDEO_TIME_TABLE_s {
	BYTE id;		//DMT id. if bit7 is high, we donot support.
	WORD hActive;	//horizontal resoultion
	WORD vActive;	//vertical resoultion
	BYTE vFreq;		//vertical frequency

	BYTE pol;		//H & V Sync polarity + interlaced flag.

	WORD hTotal;	//horizontal period
	//WORD hStart;	//horizontal Sync width + Back Porch.
	WORD hSync;		//horizontal Synch width
	WORD hBPorch;	//horizontal Back Porch

	WORD vTotal;	//vertical period
	//BYTE vFPorch;	//vertical Front Porch
	BYTE vSync;		//vertical Synch Width
	BYTE vBPorch;	//vertical Back Porch

	BYTE ClampPos;	//Clamp position value for REG1D7.

	BYTE hOverScan;	//overscan pixels
	BYTE vOverScan;	//overscan lines

	BYTE pclko_pol;
};
//extern code struct COMP_VIDEO_TIME_TABLE_s TW8836_COMP_TABLE[];

struct DIGIT_VIDEO_TIME_TABLE_s {
	BYTE vid; 		//DMT id. if bit7 is high, we donot support.

	WORD hActive;	//horizontal resoultion
	WORD vActive;	//vertical resoultion
	BYTE vFreq;		//vertical frequency

	BYTE pol;		//hPol[7] vPol[6] IorP[0].

	WORD hTotal;	//horizontal period
	WORD hFPorch;	//horizontal Front Porch
	WORD hSync;		//horizontal Synch width
	WORD hBPorch;	//horizontal Back Porch

	WORD vTotal;	//vertical period
	BYTE vFPorch;	//vertical Front Porch
	BYTE vSync;		//vertical Synch Width
	BYTE vBPorch;	//vertical Back Porch

	char vOffset;	//-128 to 127. used for Big Panel
	//hFreq:kHz
	//PixelFreq:MHz

	//BYTE hOverScan;
	//BYTE vOverScan;
	//BYTE REMOVED__pclko_pol;
	DWORD pixelfreq;

};
//extern code struct VIDEO_TIME_TABLE_s TW8836_VESA_TABLE[];

//======================================================================
// CEA 861 Video Table
//======================================================================
#define SYNC_POL_ACTIVE_LOW		1
#define FREQ_60		60
#define FREQ_50		50
#define FREQ_30		30
#define FREQ_25		25
#define FREQ_24		24

//typedef struct s_DTV_table {
//	BYTE vid;
//	WORD hActive,vActive;		
//	BYTE vFreq;
//	//BYTE fIorP; //'I' has 1, 'P' has 0.
//	BYTE pol;	/* hPol | vPol */
//	WORD hTotal, hFPorch, hSync, hBPorch;  
//	WORD vTotal; 	
//	BYTE vFPorch, vSync, vBPorch;
//	DWORD pixelfreq;
//} t_DTV_table;


struct SCALER_TIME_TABLE_s {
    WORD hActive;	//horizontal resoultion
    WORD vActive;	//vertical resoultion
    BYTE vFreq;		//vertical frequency
	BYTE vClone;	//for 59&60Hz

	BYTE vSyncWidth;
	BYTE vBPorch;

    WORD hScale;        //h scale value depend on Up/DOWN.
    WORD vScale;        //v scale value.

    WORD hLen;          //crop h active length
    BYTE hSync;
    WORD hStart;        //crop h start(Positive:sync+bporch or Negative:bporch)
    WORD hBuff;         //scaler buffer length (max 1366)

    WORD vLen;          //crop v active length 
    BYTE vSync;
    BYTE vStart;        //crop v start(Positive:sync+bporch or Negative:bporch)
    WORD vDE;           //output vDE position					

    BYTE pol;           //[7] reserved
                        //[6] BT656Enc_output is I....
                        //[5] BT656Enc		REG040[5] 1: high active. (VDCLK_POL)
                        //[4] BT656Dec		REG047[5] 1:Invert. (DTVCKP)
                        //[3] input vSync. REG041[3] 1:falling	  
                        //[2] input hSync. REG041[2] 1:falling
                        //[1] input field. REG041[1] 1:inversion
                        //[0] input clock. REG040[4] 1:Inversion 						
						
	WORD sspll2;		//base 10kHz, if 552.750MHz, write 55275.
    BYTE div;           //[7:4] pclk divider => REG0F6[4]REG0F6[2:0]
                        //[3]   pclko polarity => REG20D[4]  1:Inversion =>BK141014. use table.
                        //[2:0] pclko divider =>ocnvert=> REG20D[5]REG20D[1:0]	
};


#define POL_INTERLACED		0x40
#define POL_VDCLK		0x20
#define POL_DTVCLK		0x10
#define POL_S_V			0x08
#define POL_S_H			0x04
#define POL_S_F			0x02
#define POL_S_C			0x01
#define POL_VH		POL_S_V | POL_S_H                       //0x0C			
#define POL_VHC		POL_S_V | POL_S_H | POL_S_C             //0x0D			
#define POL_VHF		POL_S_V | POL_S_H | POL_S_F	            //0x0E			
#define POL_VHFC	POL_S_V | POL_S_H | POL_S_F | POL_S_C   //0x0F			
#define POL_DTVCLK_I		POL_DTVCLK | POL_INTERLACED
#define POL_DTVCLK_I_F		POL_DTVCLK_I | 	POL_S_F		//0x52

//see settings.h
//#define PCLKO_DIV1		0x00
//#define PCLKO_DIV2		0x01
//#define PCLKO_DIV3		0x02
//#define PCLKO_DIV15		0x06






struct SCALER_TIME_TABLE_s *FindScalerTable(BYTE _InputMain,WORD hActive,WORD vActive,BYTE vFreq, BYTE vStart, BYTE vPol);
void OverWriteScalerWithTable(struct SCALER_TIME_TABLE_s *pScaler, BYTE hPol, BYTE vPol);

#if 0
//struct BT656ENC_INPUT_TIME_TABLE_s {
//	WORD hActive;	//horizontal resoultion
//	WORD hStart;	//crop h start(sync+bporch or bporch)
//	WORD hScale;	//h scale. base 0x400. 
//  
//	WORD vActive;	//vertical resoultion
//	BYTE vStart;	//crop v start(sync+bporch or bporch)
//	BYTE vScale;	//v scale. 0~9
//};
//struct BT656ENC_OUTPUT_TIME_TABLE_s {
//	WORD hActive;	//horizontal resoultion
//	WORD hStart;	//crop h start(sync+bporch or bporch)
//	BYTE pol;		//[6] BT656Enc		REG040[5] 1: high active. (VDCLK_POL)
//					//[5] BT656Dec		REG047[5] 1:Invert. (DTVCKP)
//};
//struct BT656DEC_SCALER_TIME_TABLE_s {
//	WORD hLen;		//crop h active length
//	WORD hStart;	//crop h start(sync+bporch or bporch)
//	WORD hBuff;		//scaler buffer length (max 1366)
//  
//	WORD vLen;		//crop v active length 
//	BYTE vStart;	//crop v start(sync+bporch or bporch)
//	WORD vDE;		//output vDE position					
//
//	WORD hScale;	//h scale. . 
//	WORD vScale;	//v scale.
//
//	BYTE pol;		//[4] pclko.       REG20D[4] 1:Inversion
//					//[3] input vSync. REG041[3] 1:falling	  
//					//[2] input hSync. REG041[2] 1:falling
//					//[1] input field. REG041[1] 1:inversion
//					//[0] input clock. REG040[4] 1:Inversion 
//	DWORD sspll2;	//sspll2 freq. (not a reg value)
//	BYTE div;		//[7:4] pclk divider
//					//[3:0] pclko divider	
//};
//struct BT656ENC_TIME_TABLE_s {
//	struct BT656ENC_INPUT_TIME_TABLE_s *in;	
//	struct BT656ENC_OUTPUT_TIME_TABLE_s *out;
//	struct BT656DEC_SCALER_TIME_TABLE_s *s;
//};
#endif

//#define POL_B1	0x40
//#define POL_D1	0x20
//#define 
struct BT656ENC_TIME_TABLE_s {
	//BT656ENC_INPUT_TIME_TABLE
	WORD i_hActive;	//horizontal resoultion
	WORD i_hStart;	//crop h start(sync+bporch or bporch)
	WORD i_hScale;	//h scale. base 0x400. 

	WORD i_vActive;	//vertical resoultion
	BYTE i_vStart;	//crop v start(sync+bporch or bporch)
	BYTE i_vSyncDelay; //add delay, if the source has too small vSync.
	BYTE i_vScale;	//v scale. 0~9

	BYTE i_genlock;	//[4] x8
					//[3:0] PreDiv REG077[3:0]

	//BT656ENC_OUTPUT_TIME_TABLE
	WORD o_hActive;	//horizontal resoultion
	WORD o_hStart;	//crop h start(sync+bporch or bporch)

	//BT656DEC_SCALER_TIME_TABLE
	WORD s_hLen;		//crop h active length
	WORD s_hStart;	//crop h start(sync+bporch or bporch)
	WORD s_hBuff;		//scaler buffer length (max 1366)

	WORD s_vLen;		//crop v active length 
	BYTE s_vStart;	//crop v start(sync+bporch or bporch)
	WORD s_vDE;		//output vDE position					

	WORD s_hScale;	//h scale. . 
	WORD s_vScale;	//v scale.

    BYTE pol;           //[7] reserved
                        //[6] BT656Enc_output is I....
                        //[5] BT656Enc		REG040[5] 1: high active. (VDCLK_POL)
                        //[4] BT656Dec		REG047[5] 1:Invert. (DTVCKP)
                        //[3] input vSync. REG041[3] 1:falling	  
                        //[2] input hSync. REG041[2] 1:falling
                        //[1] input field. REG041[1] 1:inversion
                        //[0] input clock. REG040[4] 1:Inversion 						
						
	WORD sspll2;		//base 10kHz, if 552.750MHz, write 55275.
    BYTE div;           //[7:4] pclk divider => REG0F6[4]REG0F6[2:0]
                        //[3]   pclko polarity => REG20D[4]  1:Inversion
                        //[2:0] pclko divider => REG20D[5]REG20D[1:0]
};
#define BT656SCALER_IDX_CVBS_NTSC			0
#define BT656SCALER_IDX_CVBS_PAL			1
#define BT656SCALER_IDX_CVBS_NTSC_2D		2
#define BT656SCALER_IDX_CVBS_PAL_2D			3

#define BT656SCALER_IDX_COMP_480I			4		/*002*/
#define BT656SCALER_IDX_COMP_576I			5		/*090*/
#define BT656SCALER_IDX_COMP_480P			6		/*057*/
#define BT656SCALER_IDX_COMP_576P			7		/*130*/
#define BT656SCALER_IDX_COMP_720P50			8		/*150*/
#define BT656SCALER_IDX_COMP_720P60			9		/*152*/
#define BT656SCALER_IDX_COMP_1080I50_QHD	10		/*160*/
#define BT656SCALER_IDX_COMP_1080I60_QHD	11		/*164*/
#define BT656SCALER_IDX_COMP_1080P50_QHD	12		/*174*/
#define BT656SCALER_IDX_COMP_1080P60_QHD	13		/*176*/

#define BT656SCALER_IDX_PC_0660				14
#define BT656SCALER_IDX_PC_0860				15
#define BT656SCALER_IDX_PC_1060				16
#define BT656SCALER_IDX_PC_126A_1024X640	17

#define BT656SCALER_IDX_DTV_480I			18
#define BT656SCALER_IDX_DTV_576I			19
#define BT656SCALER_IDX_DTV_480P			20
#define BT656SCALER_IDX_DTV_576P			21
#define BT656SCALER_IDX_DTV_720P50			22
#define BT656SCALER_IDX_DTV_720P60_480P		23
#define BT656SCALER_IDX_DTV_720P60			24
#define BT656SCALER_IDX_DTV_1080I50_QHD		25
#define BT656SCALER_IDX_DTV_1080I60_QHD		26
#define BT656SCALER_IDX_DTV_1080P50_QHD		27
#define BT656SCALER_IDX_DTV_1080P60_QHD		28
#define BT656SCALER_IDX_DTV_0660			29
#define BT656SCALER_IDX_DTV_0860			30
#define BT656SCALER_IDX_DTV_1060			31
#define BT656SCALER_IDX_DTV_126A_1024X640	32

#define BT656SCALER_IDX_PANEL_800x480		33
#define BT656SCALER_IDX_PANEL_1024x600		34
#define BT656SCALER_IDX_PANEL_1024x768		35
#define BT656SCALER_IDX_PANEL_1280x800		36



extern code struct BT656ENC_TIME_TABLE_s BT656SCALER_time[];
extern struct BT656ENC_TIME_TABLE_s *G_pBt656Scaler;
void OverWriteBt656ScalerTable(struct BT656ENC_TIME_TABLE_s *pBt656Scaler, BYTE step);


//typedef struct s_VESA_table {
//	BYTE vid;
//	WORD hDeWidth,vDeWidth;		
//	BYTE vfreq;
//	BYTE Pol;	/* hPol | vPol */
//	WORD hTotal; 	BYTE hFPorch, hSyncWidth, hBPorch;  
//	WORD vTotal; 	BYTE vFPorch, vSyncWidth, vBPorch;
//	DWORD pixelfreq;
//} t_VESA_table;

//18bytes
//typedef struct s_VIDEO_TIME {
//	WORD hDeWidth,vDeWidth;
//	BYTE fIorP;	//'I' has 1, 'P' has 0.
//	BYTE vfreq;	//59 or 60, 50,25,24,30.
//	BYTE Pol;	//[4]:hPol, [0]:vPol. if high->low->high, it has 1.	So, 480p will have 0x11.
//	WORD hTotal;	BYTE hFPorch, hSync, hBPorch;  
//	WORD vTotal; 	BYTE vFPorch, /*vPad, vSyncStart,*/ vSync, vBPorch;
//} t_VIDEO_TIME;



#define VID_720X480P_IDX	1
#define VID_1280X720P_IDX	2
#define VID_1920X1080I_IDX	3
#define	VID_720X480I_IDX	4
#define VID_720X576P_IDX	10
#define VID_720X576I_IDX	13

#define VID_800X600P_IDX	25
#define VID_800X480P_IDX	26

#define DTV_TABLE_MAX 22
#define DTV_TABLE_480P 1	//who is a default?

#define VESA_TABLE_MAX 22



extern code struct DEC_VIDEO_TIME_TABLE_s  TW8836_DEC_TABLE[];
extern code struct COMP_VIDEO_TIME_TABLE_s TW8836_COMP_TABLE[];
extern code struct DIGIT_VIDEO_TIME_TABLE_s TW8836_VESA_TABLE[];
extern code struct DIGIT_VIDEO_TIME_TABLE_s             CEA861_table[];


void PrintCbvsVideoTimeTable(struct DEC_VIDEO_TIME_TABLE_s *p);
void PrintCompVideoTimeTable(struct COMP_VIDEO_TIME_TABLE_s *p);
void PrintVesaVideoTimeTable(struct DIGIT_VIDEO_TIME_TABLE_s *p);
void PrintCEAVideoTimeTable(struct DIGIT_VIDEO_TIME_TABLE_s *p);
//void PrintMeasValue2(char *who, WORD hStart,WORD hActive,WORD vStart,WORD vActive, BYTE vFreq);
void PrintMeasValue(char *who);
void PrintMeasAdjValue(char *who, WORD _hStart,WORD _vStart);
void PrintScalerInputCrop(char *who,WORD hStart,WORD vStart,WORD hActive,WORD vActive);
void PrintDecoderOutputCrop(char *who, WORD hDelay, WORD hActive, WORD vDelay, WORD vActive);

struct DIGIT_VIDEO_TIME_TABLE_s *Find_CEA861_VideoTable(WORD hActive,WORD vActive,BYTE vFreq);
//struct DIGIT_VIDEO_TIME_TABLE_s *Find_CEA861_VideoTable(const WORD hActive,const WORD vActive,const BYTE vFreq);


void ScalerSetOutputFixedVline(BYTE onoff /*, WORD lines*/);


//-------------------------------------
// Scale ratio
void ScalerWriteXUpReg(WORD value);
void ScalerWriteXDownReg(WORD value);
WORD ScalerReadXUpReg(void);
WORD ScalerReadXDownReg(void);
void ScalerWriteVScaleReg(WORD value);
WORD ScalerReadVScaleReg(void);
void ScalerSetHScaleReg(WORD down, WORD up);

void ScalerSetHScale(WORD Length);
void ScalerSetHScaleWithRatio(WORD Length, WORD ratio);
void ScalerSetVScale(WORD Length);
void ScalerSetVScaleWithRatio(WORD Length, WORD ratio);

//-------------------------------------
// Panorama WaterGlass effect
void ScalerPanoramaOnOff(BYTE fOn);
void ScalerSetPanorama(WORD px_scale, short px_inc);

//-------------------------------------
// Line Buffer
void ScalerWriteLineBufferDelay(BYTE delay);
BYTE ScalerReadLineBufferDelay(void);
void ScalerSetLineBufferSize(WORD length);
WORD ScalerGetLineBufferSize(void);
void ScalerSetLineBuffer(BYTE delay, WORD length);
void ScalerSetFPHSOutputPolarity(BYTE fInvert);

void ScalerWriteOutputHBlank(WORD length);

//-------------------------------------
// HDE VDE
void ScalerWriteHDEReg(WORD pos);
WORD ScalerReadHDEReg(void);
WORD ScalerCalcHDE(void);

void ScalerWriteOutputWidth(WORD width);
WORD ScalerReadOutputWidth(void);

void ScalerSetOutputWidthAndHeight(WORD width, WORD height);

void ScalerWriteOutputHeight(WORD height);

void ScalerWriteVDEReg(WORD pos);
void ScalerSet_vDE_value(BYTE vStart);
WORD ScalerReadVDEReg(void);
WORD ScalerCalcVDE(void);
WORD ScalerCalcVDE2(WORD vStart, char vde_offset);
void ScalerSetVDEPosHeight(BYTE pos, WORD len);

void ScalerSetVDEMask(BYTE top, BYTE bottom);

//-------------------------------------
//HSYNC VSYNC

void ScalerSetHSyncPosLen(WORD pos, BYTE len);
void ScalerSetVSyncPosLen(WORD pos, BYTE len);


//-------------------------------------
// Freerun
WORD ScalerReadFreerunVtotal(void);
WORD ScalerReadFreerunHtotal(void);

//WORD ScalerCalcFreerunHtotal(void);
//WORD ScalerCalcFreerunVtotal(void);
void ScalerCalcFreerunValue(WORD *Freerun_hTotal, WORD *Freerun_vTotal);

//void ScalerWriteFreerunHtotal(WORD value);
//void ScalerWriteFreerunVtotal(WORD value);
void ScalerWriteFreerunTotal(WORD h, WORD v);


//-------------------------------------
// Scaler Freerun
void ScalerSetFreerunAutoManual(BYTE fAuto, BYTE fManual);
void ScalerSetMuteAutoManual(BYTE fAuto, BYTE fManual);
void ScalerSetMuteManual( BYTE on );




void ScalerSetFreerunValue(void);
void ScalerCheckPanelFreerunValue(void);
void ScalerCheckAndSetFreerunManual(void);
void ScalerSetDeOnFreerun(void);

#define ScalerReadHActive()	ScalerReadOutputWidth()
#define ScalerSetWidthAndHeight(w,h)	ScalerSetOutputWidthAndHeight(w,h)	
#define ScalerSetBlackScreen(on)	ScalerSetMuteManual(!on)

//void ScalerTest(BYTE mode);

#endif