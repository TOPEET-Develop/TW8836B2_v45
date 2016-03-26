/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/
//void	FontDMA( void );
void	FontDisplay( void );
void	FontDemo( void );
void 	PigeonDemo( void );
void 	RoseDemo( void );
void 	CarDemo( void );
void 	LogoDemo( void );
void 	GridDemo( void );
void 	CompassDemo( void );
void 	ComplexDemo( void );
void 	OsdWinOffAll(void);
void 	OsdDemoNext(void);
void 	MovingGridInit( void );
void 	MovingGridDemo( BYTE n );
void 	MovingGridAuto( void );
void	MovingGridTask_init(void);
void MovingGridTask( void );
void MovingGridLUT( BYTE n );

// test routine.
void ReadOutputPixel(WORD start_x, WORD start_y,WORD end_x, WORD end_y);

