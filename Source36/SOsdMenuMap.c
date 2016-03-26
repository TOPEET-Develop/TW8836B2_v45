/**
 * @file
 * SOsdMenuMap.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	SpiOsd Menu Map
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
//									SOsdMenuMap.c
//
//*****************************************************************************
#include "config.h"

#if (PANEL_H==1280 && PANEL_V==800)
	#include "SOsdMenuMap_1024X600.c"
#elif (PANEL_H==1024 && PANEL_V==600)
	#include "SOsdMenuMap_1024X600.c"
#else
	#include "SOsdMenuMap_800X480.c"
#endif