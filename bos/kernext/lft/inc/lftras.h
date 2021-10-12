/* @(#)90		1.1	src/bos/kernext/lft/inc/lftras.h, lftdd, bos410, 9415B410a 10/15/93 14:31:40 */
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _L_RAS
#define _L_RAS


/*************************************************************************
	ERROR LOGGING
 *************************************************************************/

/*------------
 Unique RAS codes used to identify specific error locations for error logging
 ------------*/
#define UNIQUE_1		"1"
#define UNIQUE_2		"2"
#define UNIQUE_3		"3"
#define UNIQUE_4		"4"
#define UNIQUE_5		"5"
#define UNIQUE_6		"6"
#define UNIQUE_7		"7"
#define UNIQUE_8		"8"
#define UNIQUE_9		"9"
#define UNIQUE_10		"10"
#define UNIQUE_11		"11"
#define UNIQUE_12		"12"
#define UNIQUE_13		"13"
#define UNIQUE_14		"14"
#define UNIQUE_15		"15"
#define UNIQUE_16		"16"
#define UNIQUE_17		"17"
#define UNIQUE_18		"18"
#define UNIQUE_19		"19"
#define UNIQUE_20		"20"
#define UNIQUE_21		"21"
#define UNIQUE_22		"22"
#define UNIQUE_23		"23"
#define UNIQUE_24		"24"
#define UNIQUE_25		"25"
#define UNIQUE_26		"26"
#define UNIQUE_27		"27"
#define UNIQUE_28		"28"
#define UNIQUE_29		"29"
#define UNIQUE_30		"30"
#define UNIQUE_31		"31"
#define UNIQUE_32		"32"
#define UNIQUE_33		"33"
#define UNIQUE_34		"34"
#define UNIQUE_35		"35"
#define UNIQUE_36		"36"
#define UNIQUE_37		"37"
#define UNIQUE_38		"38"
#define UNIQUE_39		"39"
#define UNIQUE_40		"40"
#define UNIQUE_41		"41"
#define UNIQUE_42		"42"
#define UNIQUE_43		"43"
#define UNIQUE_44		"44"
#define UNIQUE_45		"45"
#define UNIQUE_46		"46"
#define UNIQUE_47		"47"
#define UNIQUE_48		"48"
#define UNIQUE_49		"49"
#define UNIQUE_50		"50"

/*------------
	LFT device driver error codes (1000 to 1099)
 ------------*/
#define LFT_ALREADY_INIT		1000
#define LFT_INV_STATE			1001
#define LFT_ALLOC_FAIL			1002
#define LFT_USPACE			1003
#define LFT_COPYIN			1004 
#define LFT_INIT_FAIL			1005
#define LFT_DISP_DEVNO			1006
#define LFT_GET_PD			1007
#define LFT_NO_DISP			1008
#define LFT_DEF_DISP			1009
#define LFT_DEVSWQRY			1010
#define LFT_STREAMS_INIT		1011
#define LFT_PIN_FAIL			1012
#define LFT_TERM_FAIL			1013
#define LFT_BAD_CMD			1014

/*
	FIle system
*/
#define LFT_FP_OPENDEV			1020
#define LFT_FP_OPEN			1021
#define LFT_FP_FSTAT			1022
#define LFT_FP_LSEEK			1023
#define LFT_FP_READ			1024

/*
	swkbd
*/
#define LFT_NOSWKB_FILE			1030
#define LFT_SWKB_INIT			1031

/*
	font - fkproc
*/
#define LFT_NOFONT_FILE			1040
#define LFT_FONT_INIT			1041
#define LFT_FKPROC			1042
#define LFT_CREATE_FKPROC		1043
#define LFT_FKPROC_INITP		1044
#define LFT_FKPROC_DEQ			1045
#define LFT_FKPROC_BADCMD		1046
#define LFT_XMATTACH			1047
#define LFT_FKPROC_SHM			1048
#define LFT_FKPROC_NOTINIT		1049
#define LFT_FKPROC_Q_OVRFLW		1050
#define LFT_FKPROC_Q_NO_MEM		1051

/*
	vtt functions
*/

#define LFT_VTTINIT			1060
#define LFT_VTTACT			1061
#define LFT_VTTDACT			1062
#define LFT_VTTTERM			1063
#define LFT_VTTDEFC			1064

/*------------
	LFT config method (cfglft) error codes (1100 to 1199)
 ------------*/
#define CFG_ODM_INIT 			1100
#define CFG_ODM_CUAT			1101
#define CFG_ODM_PDAT			1102
#define CFG_ODM_CUDV			1103
#define CFG_ODM_PDDV			1104
#define CFG_NO_DISPS			1105
#define CFG_NO_MEM			1106
#define CFG_NO_FONTS			1107
#define CFG_BAD_DEVNO			1108
#define CFG_NO_SWKBD			1119
#define CFG_BAD_SP_FILE			1110
#define CFG_BAD_PARMS			1111
#define CFG_BAD_LNAME			1112
#define CFG_DEV_MISSING			1113
#define CFG_BAD_DDS			1114
#define CFG_BAD_LOAD			1115
#define CFG_BAD_MAJOR			1116
#define CFG_BAD_MINOR			1117
#define CFG_BAD_INIT			1118
#define CFG_BAD_TERM			1119
#define CFG_BAD_STREAM			1120
#define CFG_BAD_UNLOAD			1121
#define CFG_NO_KYBD			1122


/*------------
	LFT config rule (startlft) error codes (1200 to 1299)
 ------------*/
#define STARTLFT_ODM_INIT		1200
#define STARTLFT_ODM_PDDV		1201
#define STARTLFT_ODM_PDAT		1202
#define STARTLFT_ODM_CUDV		1203
#define STARTLFT_NO_DISPS		1204
#define STARTLFT_DEFINE			1205

/*------------
	LFT unconfig method (ucfglft) error codes (1300 to 1399)
 ------------*/
#define UCFG_LOADEXT			1300
#define UCFG_UNKNOWN_MOD		1301
#define UCFG_SYSCONFIG			1302
#define UCFG_OPEN_CUDV			1303
#define UCFG_NO_CUDV			1304
#define UCFG_BAD_CUDV			1305
#define UCFG_NO_PDDV			1306
#define UCFG_BAD_PDDV			1307
#define UCFG_NO_MAJOR			1308
#define UCFG_NO_MINOR			1309
#define UCFG_UNLOAD			1310
#define UCFG_CHNG_CUDV			1311
#define UCFG_ODMCLOSE			1312

/*------------
	LFT stream based driver error codes (2000 to 2999)
 ------------*/
#define LFT_STR_INSTALL			2000
#define LFT_STR_NOINIT			2001
#define LFT_STR_ERRMEM			2002
#define LFT_STR_KBDINIT			2003
#define LFT_STR_KBDTERM			2004
#define LFT_STR_NOMSG			2005
#define LFT_STR_INVIOCTL		2006
#define LFT_STR_HDLINIT			2007
#define LFT_STR_SETINVD			2008
#define LFT_STR_NODISP			2009
#define LFT_STR_BUSYD			2010
#define LFT_STR_NOSPC			2011
#define LFT_STR_UXPESCS			2012
#define LFT_STR_NOKBD			2013
#define LFT_STR_OPENDEV			2014
#define LFT_STR_IOCTL			2015
#define LFT_STR_CLOSE			2016
#define LFT_STR_UXPSCOD			2017

#endif /* _L_RAS */
