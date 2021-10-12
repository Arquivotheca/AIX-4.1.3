/* @(#)79	1.13  src/bos/usr/include/dbxstclass.h, cmdld, bos411, 9428A410j 7/26/93 15:55:01 */
#ifndef _H_DBXSTCLASS
#define _H_DBXSTCLASS
/*
 * COMPONENT_NAME: (CMDLD) xcoff object file format definition
 *
 * FUNCTIONS: dbxstclass.h 
 *
 * ORIGINS: 26, 3, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *	XCOFF STORAGE CLASSES AND STABSTRINGS DESIGNED SPECIFICALLY FOR DBX
 */
#define DBXMASK		0x80

#define C_GSYM		0x80
#define C_LSYM		0x81
#define C_PSYM		0x82
#define C_RSYM		0x83
#define C_RPSYM		0x84
#define C_STSYM		0x85
#define C_TCSYM		0x86
#define C_BCOMM		0x87
#define C_ECOML		0x88
#define C_ECOMM		0x89
#define C_DECL		0x8c
#define C_ENTRY		0x8d
#define C_FUN		0x8e
#define C_BSTAT		0x8f
#define C_ESTAT		0x90

#define TP_ARRAY {\
	"int:t-1=r-1;-2147483648;2147483647",\
	"char:t-2=@s8;r-2;0;255",\
	"short:t-3=@s16;r-3;-32768;32767",\
	"long:t-4=-1",\
	"unsigned char:t-5=@s8;r-5;0;255",\
	"signed char:t-6=@s8;r-6;-128;127",\
	"unsigned short:t-7=@s16;r-7;0;65535",\
	"unsigned int:t-8=r-8;0;4294967295",\
	"unsigned:t-9=-8",\
	"unsigned long:t-10=-8",\
	"void:t-11=r-11;0;0",\
	"float:t-12=g-12;4",\
	"double:t-13=g-12;8",\
	"long double:t-14=g-12;16",\
	"integer:t-15=-1",\
	"boolean:t-16=efalse:0,true:1,",\
	"shortreal:t-17=g-12;4",\
	"real:t-18=g-12;8",\
	"stringptr:t-19=N-19",\
	"character:t-20=@s8;r-20;0;255",\
	"logical*1:t-21=@s8;r-21;0;255",\
	"logical*2:t-22=@s16;r-22;0;65535",\
	"logical*4:t-23=r-23;0;4294967295",\
	"logical:t-24=-23",\
	"complex:t-25=c-25;8",\
	"double complex:t-26=c-25;16",\
	"integer*1:t-27=-6",\
	"integer*2:t-28=-3",\
	"integer*4:t-29=-1",\
	"wchar:t-30=@s16;r-30;0;65535", \
        "long long:t-31=r-31;-9223372036854775808;9223372036854775807", \
        "unsigned long long:t-32=r-32;0;18446744073709551615", \
        "logical*8:t-33=r-33;0;18446744073709551615",\
        "integer*8:t-34=-31",\
	}

#define TP_INT		(-1)
#define TP_CHAR		(-2)
#define TP_SHORT	(-3)
#define TP_LONG		(-4)
#define TP_UCHAR	(-5)
#define TP_SCHAR	(-6)
#define TP_USHORT	(-7)
#define TP_UINT		(-8)
#define TP_UNSIGNED	(-9)
#define TP_ULONG	(-10)
#define TP_VOID		(-11)
#define TP_FLOAT	(-12)
#define TP_DOUBLE	(-13)
#define TP_LDOUBLE	(-14)
#define TP_PASINT	(-15)
#define TP_BOOL		(-16)
#define TP_SHRTREAL	(-17)
#define TP_REAL		(-18)
#define TP_STRNGPTR	(-19)
#define TP_FCHAR	(-20)
#define TP_LOGICAL1	(-21)
#define TP_LOGICAL2	(-22)
#define TP_LOGICAL4	(-23)
#define TP_LOGICAL	(-24)
#define TP_COMPLEX	(-25)
#define TP_DCOMPLEX	(-26)
#define TP_INTEGER1	(-27)
#define TP_INTEGER2	(-28)
#define TP_INTEGER4	(-29)
#define TP_WCHAR	(-30)
#define TP_LLONG        (-31)
#define TP_ULLONG       (-32)
#define TP_LOGICAL8     (-33)
#define TP_INTEGER8     (-34)

#define TP_NTYPES	34	

#endif /* _H_DBXSTCLASS */
