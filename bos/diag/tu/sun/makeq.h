/* @(#)53	1.1  src/bos/diag/tu/sun/makeq.h, tu_sunrise, bos411, 9437A411a 3/28/94 17:48:18 */
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/**************************************************************************/
/*  File: MakeQ.h                                                         */
/**************************************************************************/
/*  C-Cube Microsystems                                                   */
/*  1778 McCarthy Blvd.                                                   */
/*  Milpitas, CA 95035  U.S.A.                                            */
/*  Tel: (408) 944-6300  FAX: (408) 944-6314                              */
/**************************************************************************/
/*
	History -
		Written by L.S.         May-June, 1992
		Looked at it again      February, 1993

	Description -
	This file contains data sets used in computing CL5xx quantizer tables.
*/

/* These are the default input quantizer sets for luminance (Y)
   and chromimance (C) components. They specify the JPEG quantizers
   in a non-zig-zag order for easy editing. These tables are called
   Visi_Tables. */

unsigned short Default_Y_Visi_Table[64] = {

      16, 11, 10, 16, 24, 40, 51, 61,
      12, 12, 14, 19, 26, 58, 60, 55,
      14, 13, 16, 24, 40, 57, 69, 56,
      14, 17, 22, 29, 51, 87, 80, 62,
      18, 22, 37, 56, 68,109,103, 77,
      24, 35, 55, 64, 81,104,113, 92,
      49, 64, 78, 87,103,121,120,101,
      72, 92, 95, 98,112,100,103, 99};

unsigned short Default_C_Visi_Table[64] = {

      17, 18, 24, 47, 99, 99, 99, 99,
      18, 21, 26, 66, 99, 99, 99, 99,
      24, 26, 56, 99, 99, 99, 99, 99,
      47, 66, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99};

/*  If this Visi table is used, all samples for the assigned component are
	reduced to zero (not visible). This can be used for converting color
	images to black and white, for example, by zeroing the chroma
	with maximum Q values.
*/
unsigned short Zero_Visi_Table[64] = {

	 255,255,255,255,255,255,255,255,
	 255,255,255,255,255,255,255,255,
	 255,255,255,255,255,255,255,255,
	 255,255,255,255,255,255,255,255,
	 255,255,255,255,255,255,255,255,
	 255,255,255,255,255,255,255,255,
	 255,255,255,255,255,255,255,255,
	 255,255,255,255,255,255,255,255};

/*******************************************************************/
/*
	The KFactor[] matrix values are precalculated DCT coefficients
	specific to the CL550. Of the 128 coefficients, the first 64
	are used to compute the compression Q table and the last 64
	are used to compute the decompression Q table for one set of
	64 input Quantization values (this is independent of pixel
	component type).
*/

static double CompKFactor[64] = {

	2048.0000000,11362.630859,5351.682129 ,9632.773438 ,
	2048.0000000,6436.413574 ,2216.739258 ,2260.167969 ,
	11362.630859,63041.683594,29691.984375,53444.160156,
	11362.630859,35710.246094,12298.823242,12539.772461,
	5351.682129 ,29691.984375,13984.619141,25171.648438,
	5351.682129 ,16819.158203,5792.619141 ,5906.103516 ,
	9632.773438 ,53444.160156,25171.648438,45307.769531,
	9632.773438 ,30273.685547,10426.438477,10630.706055,
	2048.000000 ,11362.630859,5351.682129 ,9632.773438 ,
	2048.000000 ,6436.413574 ,2216.739258 ,2260.167969 ,
	6436.413574 ,35710.246094,16819.158203,30273.685547,
	6436.413574 ,20228.230469,6966.723633 ,7103.210449 ,
	2216.739258 ,12298.823242,5792.619141 ,10426.438477,
	2216.739258 ,6966.723633 ,2399.381592 ,2446.388428 ,
	2260.167969 ,12539.772461,5906.103516 ,10630.706055,
		2260.167969 ,7103.210449 ,2446.388428 ,2494.316162 };

static double DecompKFactor[64] = {

	64.000000   ,88.770554   ,83.620033   ,75.256042   ,
	64.000000   ,50.284481   ,34.636551   ,17.657562   ,
	88.770554   ,123.128288  ,115.984314  ,104.383125  ,
	88.770554   ,69.746574   ,48.042278   ,24.491743   ,
	83.620033   ,115.984314  ,109.254837  ,98.326752   ,
	83.620033   ,65.699837   ,45.254837   ,23.070717   ,
	75.256042   ,104.383125  ,98.326752   ,88.491737   ,
	75.256042   ,59.128292   ,40.728275   ,20.763098   ,
	64.000000   ,88.770554   ,83.620033   ,75.256042   ,
	64.000000   ,50.284481   ,34.636551   ,17.657562   ,
	50.284481   ,69.746574   ,65.699837   ,59.128292   ,
	50.284481   ,39.508263   ,27.213764   ,13.873458   ,
	34.636551   ,48.042278   ,45.254837   ,40.728275   ,
	34.636551   ,27.213764   ,18.745169   ,9.556205    ,
	17.657562   ,24.491743   ,23.070717   ,20.763098   ,
	17.657562   ,13.873458   ,9.556205    ,4.871711    };

/* End of File */
