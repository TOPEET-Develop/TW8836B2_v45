/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/

/****************************************************************/
/*                      TW88xx RAM FONT                         */
/****************************************************************/
/* <FILE TYPE>=TW8804 */
/* <NUMBER OF FONTS>=0x1E(30) */
/* <FONT WIDTH>=16 */

/* <FONT HEIGHT>=26 */

/* <2bit COLOR START>=0x000 */

/* <3bit COLOR START>=0x000 */

/* <4bit COLOR START>=0x01E */

/* <LOOK-UP TABLE 4 COLORS> */

/* <LOOK-UP TABLE 8 COLORS> */
code unsigned short FontGrayNumPalette[8] = {
     0xFFFF,0x0000,0xDEDB,0x9492,0x6B6D,0xB5B6,0x4A49,0x2124
};


/* <FONT DATA> */
code unsigned char FontGrayNum[][384] = {
    /* 3bit Color Font */
    /*0x00(  0)*/ {0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x80,0x00,0x42,0x00,0x44,0x40,0x88,0x10,0x00,0x50,0x00,0x10,0xA2,0x55,0x21,0x02,0xAA,0x55,0x80,0x08,0xAA,0x40,0x80,0x00,0x02,0x20,0x08,0x01,0x40,0x00,0x00,0x00,0x00,0x00,0x04,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /*0x01(  1)*/ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x04,0x02,0x00,0x22,0x00,0x24,0x40,0x04,0x08,0x31,0x22,0x14,0x88,0x84,0xA0,0x11,0x88,0x51,0xAA,0x11,0x1C,0x11,0xAA,0x04,0xC2,0x11,0x20,0x04,0x40,0x20,0x02,0x12,0x02,0x04,0x84},
    /*0x02(  2)*/ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF7,0xFC,0x80,0x31,0xCE,0x13,0x8C,0x73,0x80,0x01,0xEC,0x37,0x00,0x63,0x4C,0x33,0x31,0xCC,0x44,0x33,0x67,0x08,0x44},
    /* 3bit Color Font */
    /*0x03(  3)*/ {0x77,0x8C,0x00,0xEC,0x13,0x80,0x10,0x8C,0x01,0xFC,0xF3,0x08,0x00,0x03,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x13,0x00,0x00,0x03,0x11,0x00,0x00},
    /*0x04(  4)*/ {0x00,0x11,0x00,0x00,0x00,0x11,0x00,0x00,0x00,0x11,0x00,0x00,0x00,0x11,0x00,0x00,0x00,0x11,0x00,0x00,0x00,0x01,0x00,0x20,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /*0x05(  5)*/ {0x00,0x82,0x22,0x00,0x62,0x40,0x22,0x00,0x01,0x00,0x22,0x00,0x00,0x00,0x22,0x00,0x00,0x00,0x22,0x00,0x00,0x00,0x22,0x00,0x00,0x00,0x22,0x00,0x00,0x00,0x22,0x00,0x20,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 3bit Color Font */
    /*0x06(  6)*/ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x71,0xCC,0x00,0x11,0x8C,0xCC,0x00,0x02,0x00,0xCC,0x00,0x00,0x00,0xCC,0x00,0x00,0x00,0xCC,0x00,0x00,0x00,0xCC,0x00,0x00,0x00,0xCC,0x00,0x00,0x00,0xCC,0x00,0x30,0xF0,0xFC,0xE0,0x03,0x0F,0x0F,0x0E},
    /*0x07(  7)*/ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x10,0x00,0x12,0x00,0x04,0x08,0x00,0x00,0x02,0x04,0x00,0x00,0x20,0x04,0x00,0x00,0x04,0x88,0x00,0x10,0x29,0x00,0x00,0x02,0x84,0x00},
    /*0x08(  8)*/ {0x10,0x21,0x00,0x00,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x08,0x12,0x00,0x36,0x84,0x4C,0x40,0x00,0x00,0x20,0x40,0x00,0x00,0x62,0x40},
    /* 3bit Color Font */
    /*0x09(  9)*/ {0x00,0x00,0x80,0x08,0x00,0x01,0x28,0x00,0x00,0x40,0x84,0x00,0x00,0x39,0x00,0x00,0x40,0x06,0x00,0x20,0x04,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF7,0xEE,0x00},
    /*0x0A( 10)*/ {0x11,0x0C,0x33,0x80,0x00,0x00,0x31,0xCC,0x00,0x00,0x13,0x8C,0x00,0x00,0x77,0x80,0x00,0x00,0xC6,0x00,0x00,0x71,0x08,0x00,0x10,0xEE,0x00,0x00,0x33,0xFC,0xF0,0xE0,0x03,0x0F,0x0F,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /*0x0B( 11)*/ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x12,0x00,0x00,0x08,0x24,0x08,0x00,0x00,0x20,0x00,0x00,0x00,0x10,0x08,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x11,0x04,0x00,0x00,0x21,0x04,0x20,0x00,0x14,0x08,0x02,0x00,0x04,0x00,0x00,0x00,0x00,0x00},
    /* 3bit Color Font */
    /*0x0C( 12)*/ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x21,0x00,0x10,0x80,0x02,0x00,0x60,0x48,0x00,0x00,0x02,0x44,0x00,0x00,0x94,0x80,0x00,0x88,0x00,0x80,0x00,0x00,0x24,0x44,0x00,0x00,0x00,0x26,0x00,0x00,0x03,0x44},
    /*0x0D( 13)*/ {0x00,0x00,0x10,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0xFF,0xFE,0x00,0x01,0x08,0x13,0x80,0x00,0x00,0x13,0x88,0x00,0x00,0x73,0x00,0x00,0xFF,0xFC,0x00},
    /*0x0E( 14)*/ {0x00,0x00,0x17,0x88,0x00,0x00,0x00,0xCC,0x00,0x00,0x30,0xC8,0x10,0xF0,0xE3,0x00,0x01,0x0F,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x20,0x80,0x00},
    /* 3bit Color Font */
    /*0x0F( 15)*/ {0x00,0x50,0x00,0x00,0x00,0x20,0x00,0x00,0x21,0x44,0x00,0x00,0x02,0x08,0x00,0x00,0x80,0x00,0x00,0x10,0x08,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /*0x10( 16)*/ {0x00,0x00,0x00,0x00,0x00,0x11,0x08,0x88,0x00,0x02,0xC0,0x88,0x00,0xD4,0x4C,0x88,0x10,0x09,0x44,0x88,0x00,0x06,0x44,0x88,0x46,0x88,0x44,0x88,0x09,0x00,0x04,0x08,0x00,0x00,0x40,0x80,0x00,0x00,0x44,0x88,0x00,0x00,0x04,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /*0x11( 17)*/ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0xF7,0x00,0x00,0x11,0xFF,0x00,0x00,0x23,0x7F,0x00,0x00,0xCE,0x77,0x00,0x30,0xCC,0x77,0x00,0x71,0x00,0x77,0x00,0xF7,0xF0,0xF7,0xF0,0x0F,0x0F,0x7F,0x0F,0x00,0x00,0x77,0x00},
    /* 3bit Color Font */
    /*0x12( 18)*/ {0x00,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x88,0x00,0x44,0x00,0x00,0x00,0x44,0x00,0x00,0x00,0x44,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x24,0x44},
    /*0x13( 19)*/ {0x00,0x00,0x11,0x44,0x00,0x00,0x20,0x04,0x00,0x00,0x10,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x22,0x00,0x00,0xCC,0x22,0x00,0x00,0x00,0x22,0x00,0x00,0x00},
    /*0x14( 20)*/ {0x22,0x00,0x00,0x00,0x22,0x00,0x00,0x80,0x00,0x00,0x00,0x40,0x00,0x00,0x33,0x66,0x00,0x00,0x02,0x44,0x20,0x00,0x08,0x00,0x02,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 3bit Color Font */
    /*0x15( 21)*/ {0x11,0xFF,0xFF,0x88,0x11,0xCC,0x00,0x00,0x11,0xCC,0x00,0x00,0x11,0xCC,0x00,0x00,0x11,0xFF,0xFC,0x80,0x00,0x00,0x33,0x8C,0x00,0x00,0x00,0xCC,0x00,0x00,0x11,0xC8,0x30,0xF0,0xE7,0x00,0x03,0x0F,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /*0x16( 22)*/ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x44,0x00,0x08,0x00,0x00,0x00,0x80,0x00,0x00,0x02,0x88,0x10,0x00,0x00,0x40,0x00,0x40,0x00,0x00,0x00,0x22,0x00,0x88,0x00,0x22,0x02,0x08,0x11,0x00,0x00,0x00,0x02,0x80,0x00,0x04,0x02,0x00},
    /*0x17( 23)*/ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x41,0x00,0x00,0x10,0x20,0x08,0x00,0x22,0xC4,0x00,0x00,0x46,0x48,0x00,0x00,0x44,0x00,0x20,0x64,0x44,0x88,0x11,0x00,0x44,0x80,0x11,0x00},
    /* 3bit Color Font */
    /*0x18( 24)*/ {0x26,0x4C,0x00,0x22,0x12,0x02,0x06,0x04,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x71,0xFF,0xCC,0x10,0xE7,0x00,0x00,0x31,0x8C,0x00,0x00,0x31,0x70,0xE0,0x00},
    /*0x19( 25)*/ {0x33,0x8F,0x3F,0xC8,0x33,0x88,0x01,0xCE,0x33,0x00,0x00,0xCC,0x31,0xC8,0x01,0xCE,0x01,0xFE,0xF1,0x0C,0x00,0x03,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x44,0x00,0x00,0x22},
    /*0x1A( 26)*/ {0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x80,0x00,0x00,0x10,0x08,0x00,0x00,0xA9,0x00,0x00,0x11,0x40,0x00,0x00,0x22,0x00,0x00,0x00,0x44,0x08,0x00,0x00,0x91,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 3bit Color Font */
    /*0x1B( 27)*/ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x21,0x02,0x00,0x00,0x42,0x84,0x00,0x00,0x94,0x00,0x00,0x10,0xB0,0x00,0x00,0x30,0x62,0x00,0x00,0x60,0x44,0x00,0x00,0x00,0x80,0x00,0x00,0x08,0x00,0x00,0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x00},
    /*0x1C( 28)*/ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x77,0xFF,0xFF,0xEE,0x00,0x00,0x11,0x8C,0x00,0x00,0x33,0x08,0x00,0x00,0x67,0x08,0x00,0x00,0x6F,0x00,0x00,0x01,0xCE,0x00,0x00,0x13,0x8C,0x00,0x00,0x37,0x00,0x00},
    /*0x1D( 29)*/ {0x00,0x76,0x00,0x00,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x80,0x20,0x84,0x12,0x44,0x22,0x88,0x10,0x40,0x00,0x14,0x42,0x84,0x00,0x40,0x00,0x00},
};
