static char sccsid[] = "@(#)78	1.6  src/bos/usr/bin/errlg/errupdate/crc.c, cmderrlg, bos411, 9428A410j 6/15/90 21:08:11";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: crc
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Generate a 32 bit crc from a buffer.
 * The input is the poitner to the buffer and its byte length.
 * The return value is the crc.
 *
 * Note: This algorithm is the same as for calculating Alert ID numbers
 *       for the Generic Alert Architecture. However, its only use here is
 *       generate a "unique" 32 bit tag from some input, and there is no
 *       requirement to match a particular polynomial.
 *
 * The polynomial is:
 * x**32 + x**26 + x**23 + x**22 + x**16 + x**12 + x**11  + x**10 +
 *    + x**8 + x**7 + x**5 + x**4 + x**2 + x**1 + x**0
 */

union {
	unsigned char uc[4];
	unsigned int  ui;
} A;

static unsigned Tl[] = {
	0x00000000,
	0x04C11DB7,
	0x09823B6E,
	0x0D4326D9,
	0x130476DC,
	0x17C56B6B,
	0x1A864DB2,
	0x1E475005,

	0x2608EDB8,
	0x22C9F00F,
	0x2F8AD6D6,
	0x2B4BCB61,
	0x350C9B64,
	0x31CD86D3,
	0x3C8EA00A,
	0x384FBDBD
};

static unsigned Th[] = {
	0x00000000,
	0x4C11DB70,
	0x9823B6E0,
	0xD4326D90,
	0x34867077,
	0x7897AB07,
	0xACA5C697,
	0xE0B41DE7,

	0x690CE0EE,
	0x251D3B9E,
	0xF12F560E,
	0xBD3E8D7E,
	0x5D8A9099,
	0x119B4BE9,
	0xC5A92679,
	0x89B8FD09,
};

unsigned crc(buf,count)
unsigned char *buf;
{
	unsigned char w,m;
	
	A.ui = 0xFFFFFFFF;
	while(--count >= 0) {
		m = *buf++;
		w = m ^ A.uc[0];
		A.uc[0] = A.uc[1];
		A.uc[1] = A.uc[2];
		A.uc[2] = A.uc[3];
		A.uc[3] = 0;
		A.ui ^= Th[w/16];
		A.ui ^= Tl[w%16];
	}
	A.ui = ~A.ui;
	return(A.ui);
}

