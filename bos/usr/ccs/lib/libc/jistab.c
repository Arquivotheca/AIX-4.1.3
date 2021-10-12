static char sccsid[] = "@(#)28	1.1  src/bos/usr/ccs/lib/libc/jistab.c, libcnls, bos411, 9428A410j 2/26/91 17:42:39";
/*
 * COMPONENT_NAME: (LIBCGEN/KJI) Standard C Library Conversion Functions
 *
 * FUNCTIONS: _jistoatab, _atojistab
 *
 * ORIGINS: 10
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
/**********************************************************************/
/*								      */
/* SYNOPSIS							      */
/*	unsigned int _jistoatab[][91]				      */
/*								      */
/* DESCRIPTION							      */
/*	_jistoatab is used to convert characters from Shift-JIS to    */
/*	ASCII.  The table and functions which use the table make      */
/*	use of the fact that ASCII characters are between 8140 and    */
/*	8197 or between 824f and 829a (functions use the difference   */
/*	between a given value and 8140 or 8240 to calculate an offset */
/*	into the table).					      */
/*								      */
/**********************************************************************/
unsigned int _jistoatab[2][91] = {
       {0x20,	/* ' '	8140 */
	0x8141,
	0x8142,
	0x2c,	/* ,	8143 */
	0x2e,	/* .	8144 */
	0x8145,
	0x3a,	/* :	8146 */
	0x3b,	/* ;	8147 */
	0x3f,	/* ?	8148 */
	0x21,	/* !	8149 */
	0x814a,
	0x814b,
	0x814c,
	0x814d,
	0x814e,
	0x5e,	/* ^	814f */
	0x8150,
	0x5f,	/* _	8151 */
	0x8152,
	0x8153,
	0x8154,
	0x8155,
	0x8156,
	0x8157,
	0x8158,
	0x8159,
	0x815a,
	0x815b,
	0x815c,
	0x815d,
	0x2f,	/* /	815e */
	0x5c,	/* \	815f */
	0x7e,	/* ~	8160 */
	0x8161,
	0x7c,	/* |	8162 */
	0x8163,
	0x8164,
	0x60,	/* `	8165 */
	0x8166,
	0x8167,
	0x22,	/* "	8168 */
	0x28,   /* (    8169 */
	0x29,	/* )	816a */
	0x816b,
	0x816c,
	0x5b,	/* [	816d */
	0x5d,	/* ]	816e */
	0x7b,	/* {	816f */
	0x7d,	/* }	8170 */
	0x8171,
	0x8172,
	0x8173,
	0x8174,
	0x8175,
	0x8176,
	0x8177,
	0x8178,
	0x8179,
	0x817a,
	0x2b,	/* +	817b */
	0x2d,	/* -	817c */
	0x817d,
	0x817e,
	0x817f,
	0x8180,
	0x3d,	/* =	8181 */
	0x8182,
	0x3c,	/* <	8183 */
	0x3e,	/* >	8184 */
	0x8185,
	0x8186,
	0x8187,
	0x8188,
	0x8189,
	0x818a,
	0x818b,
	0x27,	/* '	818c */
	0xde,   /* "    818d */
	0x818e,
	0x818f,
	0x24,	/* $	8190 */
	0x8191,
	0x8192,
	0x25,	/* %	8193 */
	0x23,	/* #	8194 */
	0x26,	/* &	8195 */
	0x2a,	/* *	8196 */
	0x40,	/* @	8197 */
	0x8198,
	0x8199,
	0x819a},

       {0x8240,
	0x8241,
	0x8242,
	0x8243,
	0x8244,
	0x8245,
	0x8246,
	0x8247,
	0x8248,
	0x8249,
	0x824a,
	0x824b,
	0x824c,
	0x824d,
	0x824e,
	0x30,	/* 0	824f */
	0x31,	/* 1	8250 */
	0x32,	/* 2	8251 */
	0x33,	/* 3	8252 */
	0x34,	/* 4	8253 */
	0x35,	/* 5	8254 */
	0x36,	/* 6	8255 */
	0x37,	/* 7	8256 */
	0x38,	/* 8	8257 */
	0x39,	/* 9	8258 */
	0x8259,
	0x825a,
	0x825b,
	0x825c,
	0x825d,
	0x825e,
	0x825f,
	0x41,	/* A	8260 */
	0x42,	/* B	8261 */
	0x43,	/* C	8262 */
	0x44,	/* D	8263 */
	0x45,	/* E	8264 */
	0x46,	/* F	8265 */
	0x47,	/* G	8266 */
	0x48,	/* H	8267 */
	0x49,	/* I	8268 */
	0x4a,	/* J	8269 */
	0x4b,	/* K	826a */
	0x4c,	/* L	826b */
	0x4d,	/* M	826c */
	0x4e,	/* N	826d */
	0x4f,	/* O	826e */
	0x50,	/* P	826f */
	0x51,	/* Q	8270 */
	0x52,	/* R	8271 */
	0x53,	/* S	8272 */
	0x54,	/* T	8273 */
	0x55,	/* U	8274 */
	0x56,	/* V	8275 */
	0x57,	/* W	8276 */
	0x58,	/* X	8277 */
	0x59,	/* Y	8278 */
	0x5a,	/* Z	8279 */
	0x827a,
	0x827b,
	0x827c,
	0x827d,
	0x827e,
	0x827f,
	0x8280,
	0x61,	/* a	8281 */
	0x62,	/* b	8282 */
	0x63,	/* c	8283 */
	0x64,	/* d	8284 */
	0x65,	/* e	8285 */
	0x66,	/* f	8286 */
	0x67,	/* g	8287 */
	0x68,	/* h	8288 */
	0x69,	/* i	8289 */
	0x6a,	/* j	828a */
	0x6b,	/* k	828b */
	0x6c,	/* l	828c */
	0x6d,	/* m	828d */
	0x6e,	/* n	828e */
	0x6f,	/* o	828f */
	0x70,	/* p	8290 */
	0x71,	/* q	8291 */
	0x72,	/* r	8292 */
	0x73,	/* s	8293 */
	0x74,	/* t	8294 */
	0x75,	/* u	8295 */
	0x76,	/* v	8296 */
	0x77,	/* w	8297 */
	0x78,	/* x	8298 */
	0x79,	/* y	8299 */
	0x7a}	/* z	829a */
};


/**********************************************************************/
/*								      */
/*  NAME							      */
/*	_atojistab						      */
/*								      */
/* SYNOPSIS							      */
/*	unsigned short _atojistab[]				      */
/*								      */
/* DESCRIPTION							      */
/*	_atojistab is used to convert characters from ASCII to        */
/*	Shift-JIS.  The table and functions which use the table make  */
/*	use of the fact that ASCII characters below 0x20 have no      */
/*	Shift-JIS equivalent (functions use the difference between a  */
/*	given value and 0x20 to calculate an offset into the table).  */
/*								      */
/**********************************************************************/

unsigned short _atojistab[95] = {
	/* 20	' ' */	0x8140,
	/* 21	! */	0x8149,
	/* 22	" */	0x8168,
	/* 23	# */	0x8194,
	/* 24	$ */	0x8190,
	/* 25	% */	0x8193,
	/* 26	& */	0x8195,
	/* 27	' */	0x818c,
	/* 28	( */	0x8169,
	/* 29	) */	0x816a,
	/* 2a	* */	0x8196,
	/* 2b	+ */	0x817b,
	/* 2c	, */	0x8143,
	/* 2d	- */	0x817c,
	/* 2e	. */	0x8144,
	/* 2f	/ */	0x815e,
	/* 30	0 */	0x824f,
	/* 31	1 */	0x8250,
	/* 32	2 */	0x8251,
	/* 33	3 */	0x8252,
	/* 34	4 */	0x8253,
	/* 35	5 */	0x8254,
	/* 36	6 */	0x8255,
	/* 37	7 */	0x8256,
	/* 38	8 */	0x8257,
	/* 39	9 */	0x8258,
	/* 3a	: */	0x8146,
	/* 3b	; */	0x8147,
	/* 3c	< */	0x8183,
	/* 3d	= */	0x8181,
	/* 3e	> */	0x8184,
	/* 3f	? */	0x8148,
	/* 40	@ */	0x8197,
	/* 41	A */	0x8260,
	/* 42	B */	0x8261,
	/* 43	C */	0x8262,
	/* 44	D */	0x8263,
	/* 45	E */	0x8264,
	/* 46	F */	0x8265,
	/* 47	G */	0x8266,
	/* 48	H */	0x8267,
	/* 49	I */	0x8268,
	/* 4a	J */	0x8269,
	/* 4b	K */	0x826a,
	/* 4c	L */	0x826b,
	/* 4d	M */	0x826c,
	/* 4e	N */	0x826d,
	/* 4f	O */	0x826e,
	/* 50	P */	0x826f,
	/* 51	Q */	0x8270,
	/* 52	R */	0x8271,
	/* 53	S */	0x8272,
	/* 54	T */	0x8273,
	/* 55	U */	0x8274,
	/* 56	V */	0x8275,
	/* 57	W */	0x8276,
	/* 58	X */	0x8277,
	/* 59	Y */	0x8278,
	/* 5a	Z */	0x8279,
	/* 5b	[ */	0x816d,
	/* 5c	\ */	0x815f,
	/* 5d	] */	0x816e,
	/* 5e	^ */	0x814f,
	/* 5f	_ */	0x8151,
	/* 60	` */	0x8165,
	/* 61	a */	0x8281,
	/* 62	b */	0x8282,
	/* 63	c */	0x8283,
	/* 64	d */	0x8284,
	/* 65	e */	0x8285,
	/* 66	f */	0x8286,
	/* 67	g */	0x8287,
	/* 68	h */	0x8288,
	/* 69	i */	0x8289,
	/* 6a	j */	0x828a,
	/* 6b	k */	0x828b,
	/* 6c	l */	0x828c,
	/* 6d	m */	0x828d,
	/* 6e	n */	0x828e,
	/* 6f	o */	0x828f,
	/* 70	p */	0x8290,
	/* 71	q */	0x8291,
	/* 72	r */	0x8292,
	/* 73	s */	0x8293,
	/* 74	t */	0x8294,
	/* 75	u */	0x8295,
	/* 76	v */	0x8296,
	/* 77	w */	0x8297,
	/* 78	x */	0x8298,
	/* 79	y */	0x8299,
	/* 7a	z */	0x829a,
	/* 7b	{ */	0x816f,
	/* 7c	| */	0x8162,
	/* 7d	} */	0x8170,
	/* 7e	~ */	0x8160
};
