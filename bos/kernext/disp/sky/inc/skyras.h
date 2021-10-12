/* @(#)53	1.1  src/bos/kernext/disp/sky/inc/skyras.h, sysxdispsky, bos411, 9428A410j 10/28/93 08:51:21 */
/*
 *   COMPONENT_NAME: SYSXDISPSKY
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _SKY_RAS
#define _SKY_RAS


/*************************************************************************
	ERROR LOGGING
 *************************************************************************/

/*------------
 Unique RAS codes used to identify specific error locations for error logging
 ------------*/
#define SKY_UNIQUE_1		"1"
#define SKY_UNIQUE_2		"2"
#define SKY_UNIQUE_3		"3"
#define SKY_UNIQUE_4		"4"
#define SKY_UNIQUE_5		"5"
#define SKY_UNIQUE_6		"6"
#define SKY_UNIQUE_7		"7"
#define SKY_UNIQUE_8		"8"
#define SKY_UNIQUE_9		"9"
#define SKY_UNIQUE_10		"10"
#define SKY_UNIQUE_11		"11"
#define SKY_UNIQUE_12		"12"
#define SKY_UNIQUE_13		"13"
#define SKY_UNIQUE_14		"14"
#define SKY_UNIQUE_15		"15"
#define SKY_UNIQUE_16		"16"
#define SKY_UNIQUE_17		"17"
#define SKY_UNIQUE_18		"18"
#define SKY_UNIQUE_19		"19"
#define SKY_UNIQUE_20		"20"
#define SKY_UNIQUE_21		"21"
#define SKY_UNIQUE_22		"22"
#define SKY_UNIQUE_23		"23"
#define SKY_UNIQUE_24		"24"
#define SKY_UNIQUE_25		"25"
#define SKY_UNIQUE_26		"26"
#define SKY_UNIQUE_27		"27"
#define SKY_UNIQUE_28		"28"
#define SKY_UNIQUE_29		"29"
#define SKY_UNIQUE_30		"30"
#define SKY_UNIQUE_31		"31"
#define SKY_UNIQUE_32		"32"
#define SKY_UNIQUE_33		"33"
#define SKY_UNIQUE_34		"34"
#define SKY_UNIQUE_35		"35"
#define SKY_UNIQUE_36		"36"
#define SKY_UNIQUE_37		"37"
#define SKY_UNIQUE_38		"38"
#define SKY_UNIQUE_39		"39"
#define SKY_UNIQUE_40		"40"
#define SKY_UNIQUE_41		"41"
#define SKY_UNIQUE_42		"42"
#define SKY_UNIQUE_43		"43"
#define SKY_UNIQUE_44		"44"
#define SKY_UNIQUE_45		"45"
#define SKY_UNIQUE_46		"46"
#define SKY_UNIQUE_47		"47"
#define SKY_UNIQUE_48		"48"
#define SKY_UNIQUE_49		"49"
#define SKY_UNIQUE_50		"50"

/*  functinal error codes */
#define SKY_SETJMP		"1000"
#define SKY_XMALLOC		"1001"
#define SKY_INVALMODE		"1002"
#define SKY_NOTKSRMODE		"1003"
#define SKY_BADCMD		"1004"
#define SKY_TALLOC		"1005"
#define SKY_DEVADDFAIL		"1006"
#define SKY_DEVQFAIL		"1007"
#define SKY_COPYINFAIL		"1008"
#define SKY_COPYOUTNFAIL	"1009"
#define SKY_DMAFAIL		"1010"
#define SKY_INTRFAIL		"1011"
#define SKY_DEVOPEN		"1012"
#define SKY_DEVINUSE		"1013"
#define SKY_DEVDELFAIL		"1014"

#endif
