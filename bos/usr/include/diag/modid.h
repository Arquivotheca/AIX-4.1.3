/* @(#)16       1.4  src/bos/usr/include/diag/modid.h, cmddiag, bos411, 9428A410j 3/23/94 06:44:29 */
/*
 *   COMPONENT_NAME: CMDDIAG
 *
 *   FUNCTIONS: IS_DESKTOP
 *		IS_HIGH
 *		IS_LOW
 *		IS_MED
 *		IS_R1
 *		IS_R2
 *		IS_RACK
 *		IS_RSC
 *		IS_TOWER
 *		IS_TURBO
 *		IsPowerPC
 *		IsPowerPC_SMP
 *		IsPowerPC_SMP_A
 *		
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 *
 */

 /* MACRO usage:  x = get_cpu_model(&y); */

#define IS_R1(x) ((x & 0xff000000) ? 0 : 1 )
#define IS_R2(x) ((x & 0xff000000) ? 1 : 0 )
#define IS_RSC(y) (((y >= 0x40) && (y <= 0x49)) ? 1 : 0 )
#define IsPowerPC(x) (((x & 0xff000000)  == 0x08000000 ) ? 1 : 0)
#define IsPowerPC_SMP(x) (((x & 0xff0000f0)  == 0x080000A0 ) ? 1 : 0)
#define IsPowerPC_SMP_A(x) (((x & 0xff0000ff)  == 0x080000A0 ) ? 1 : 0)


#define IS_LOW(y)   (((y & 0x0C) == 0x00) ? 1 : 0 )
#define IS_MED(y)   (((y & 0x04) == 0x04) ? 1 : 0 )
#define IS_HIGH(y)  (((y & 0x08) == 0x08) ? 1 : 0 )
#define IS_TURBO(y) (((y & 0x0C) == 0x0C) ? 1 : 0 )

#define IS_TOWER(y)   (((y & 0x03) == 0x00) ? 1 : 0 )
#define IS_DESKTOP(y) (((y & 0x01) == 0x01) ? 1 : 0 )
#define IS_RACK(y)    (((y & 0x02) == 0x02) ? 1 : 0 )

