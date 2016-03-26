/**
 * @file
 * SpiFlashMap.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	SpiFlash Map for images
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
//								SpiFlashMap.c
//
//*****************************************************************************
#include "Config.h"

#if (PANEL_H==1280 && PANEL_V==800)
	#include "SpiFlashMap_1024X600.c"
#elif (PANEL_H==1024 && PANEL_V==600)
	#include "SpiFlashMap_1024X600.c"
#else
	#include "SpiFlashMap_800X480.c"
#endif