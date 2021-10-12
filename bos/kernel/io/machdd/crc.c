static char sccsid[] = "@(#)53	1.2  src/bos/kernel/io/machdd/crc.c, machdd, bos411, 9428A410j 11/20/92 10:02:45";
/*
 * COMPONENT_NAME: (MACHDD) Machine Device Driver
 *
 * FUNCTIONS: crc32
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <sys/types.h>

/*
 * NAME: crc32
 *
 * FUNCTION:
 *     crc32 generates a 32 bit "classic" CRC using the following
 *     CRC polynomial:
 *
 *   g(x) = x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8
 *               + x^7  + x^5  + x^4  + x^2  + x^1  + x^0
 *
 *   e.g. g(x) = 1 04c1 1db7
 *
 * EXECUTION ENVIRONMENT:  Interrupt and process; must be pinned.
 *
 * NOTES:
 *
 *    This function has been optimized for speed and size.  Two routines in
 *    this file, crchware() and mkcrctbl(), were used to construct crc32().
 *    crchware() simulates a crc hardware circuit in software.
 *    crchware() can be used to generate crc's, but the time overhead to
 *    use it is prohibitive.  By studying the CRC algorithm, we note that
 *    the data byte and the high byte of the accumulator are combined
 *    together to a value between 0 and 255 which can be precalculated in
 *    a table of the 256 possible values.  This table can further be
 *    collapsed by computing a table of values for the high nybble and a
 *    table of values for the low nybble, which are then XOR'ed into the
 *    accumulator.
 *
 *    Note further that if the computed CRC is appended to the end of the
 *    data and the crc function is called again, the result is zero if
 *    there are NO ERRORS in the data.
 *
 * RETURN VALUE:
 *
 *       0 : The last four bytes of the data contained the correct CRC
 *           for the data space.
 *
 *   accum : A four byte CRC for the data.
 *
 */

#define CRC_32    0x04c11db7

static ulong crctl[16] = {
    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
    0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
    0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD
};

static ulong crcth[16] = {
    0x00000000, 0x4C11DB70, 0x9823B6E0, 0xD4326D90,
    0x34867077, 0x7897AB07, 0xACA5C697, 0xE0B41DE7,
    0x690CE0EE, 0x251D3B9E, 0xF12F560E, 0xBD3E8D7E,
    0x5D8A9099, 0x119B4BE9, 0xC5A92679, 0x89B8FD09
};

ulong
crc32(char *pbuff, ulong len)
{
    ulong i;
    ulong accum;
    ulong temp;

    accum = 0;
    for (i=0; i < len; i++) {
        temp = (accum >> 24) ^ *pbuff++;
        accum <<= 8;
        accum ^= crcth[ temp/16 ];
        accum ^= crctl[ temp%16 ];
    }
    return accum;
}

#ifdef notdef

/*
 * NAME: crc16
 *
 * FUNCTION:
 *     crc16 generates a 16 bit "pseudo" CRC.
 *     It is believed that this function uses the following
 *     CRC-CCITT polynomial:
 *              g(x) = x^16 + x^12 + x^5 + x^0      e.g. g(x) = 1 1021
 *
 * NOTES:
 *     The exact origin of this function is not known.  It is believed to
 *     have come from the PC and similar code was used on the RT.  It does
 *     a reliable job of generating a unique "CRC" value.
 *
 *     Note further that if the computed CRC is BYTE REVERSED and appended
 *     to the end of the data and the crc function is called again, the
 *     result is zero if there are NO ERRORS in the data.
 *
 * RETURN VALUE:
 *
 *       0 : The last two bytes of the data contained the correct
 *           BYTE REVERSED CRC for the data space.
 *
 *  dvalue : A two byte CRC for the data.
 */

ushort
crc16(pbuff, length)
char *pbuff;
int length;
{
#define CRC_MASK 0xff07
#define COMBINE(x, y) (((x) << 8) | (y))

    struct bytes {
       char msb;
       char lsb;
    };
    union accum {
        ushort whole;
        struct bytes byte;
    } avalue, dvalue;

    char datav;
    int i;

    dvalue.whole = 0xffff;
    avalue.whole = 0;
    for(i=0; length > 0; i++, length--) {
        datav = *(pbuff+i);
        avalue.byte.lsb = (datav ^ dvalue.byte.lsb);
        dvalue.byte.lsb = avalue.byte.lsb;
        avalue.whole = ((avalue.whole * 16) ^ dvalue.byte.lsb);
        dvalue.byte.lsb = avalue.byte.lsb;
        avalue.whole <<= 8;
        avalue.whole >>= 1;
        avalue.byte.lsb ^= dvalue.byte.lsb;
        avalue.whole >>= 4;
        avalue.whole = COMBINE(avalue.byte.lsb, avalue.byte.msb);
        avalue.whole = ((avalue.whole & CRC_MASK) ^ dvalue.byte.lsb);
        avalue.whole = COMBINE(avalue.byte.lsb, avalue.byte.msb);
        avalue.byte.lsb ^= dvalue.byte.msb;
        dvalue.whole = avalue.whole;
    }
    return dvalue.whole;
}

	/* THIS CODE INCLUDED ONLY FOR EXPLANATORY PURPOSE ONLY */
/*
 * NAME: crchware
 *
 * FUNCTION:
 *    crchware() simulates a crc hardware circuit in software for any
 *    generator polynomial.
 *
 * NOTES:
 *    This function can be used for either 16 or 32 bit CRC's.  In fact it
 *    was orginally written for 16 bit CRC and I modified it for 32 bits.
 *    I hoped to show with the ifdef's what changes were needed to suit 
 *    your needs.
 * 
 *    This function is typically used to create a table of CRC combined values.
 *    e.g.    
 *            int i;
 *            for (i=0; i < 256; i++) 
 *                crctbl[i] = crchware(i, GPOLY, 0);
 *         where GPOLY is the generator polynomial (1021)
 *
 *    It could be called repeatedly to calculate a CRC.
 *    e.g.
 *            char *ptr;
 *            int i;
 *            ulong accum = 0;
 *            for (i=0; i < datalen; i++)
 *                  accum = crchware( *(ptr+i), GPOLY, accum);
 *
 * RETURN VALUE:
 *
 *       0 : The last two/four bytes of the data contained the correct 
 *           CRC for the data space.
 *
 *   accum : A two/four byte CRC for the data.
 */

ulong
crchware(data, genpoly, accum)
#ifdef 32bit
    ulong data, genpoly, accum;
#else /* 16bit */
    ushort data, genpoly, accum;
#endif
{
#ifdef 32bit
    #define SHIFTHIGH 24
    #define HIGHMASK  0x80000000
#else /* 16bit */
    #define SHIFTHIGH 8
    #define HIGHMASK  0x8000
#endif
    int i;

    data <<= SHIFTHIGH;
    for (i=8; i > 0; i--) {
	if ((data^accum) & HIGHMASK)
	    accum = (accum << 1) ^ genpoly;
	else
	    accum <<= 1;
	data <<= 1;
    }
    return accum;
}

/*
 * NAME: mkcrctbl
 *
 * FUNCTION:
 *     Generate a collapsed table of CRC combined values for the given
 *     CRC polynomial.
 *
 * DATA STRUCTURES:
 *     Fills static/global declared vars crctl[] and crcth[]
 *
 * RETURN VALUE:
 *     NONE
 */

mkcrctbl(poly)
ulong poly;
{
    for (i = 0; i < 16; i++) {
	crctl[i] = crchware(i, poly, 0);
	crcth[i] = crchware(i<<4, poly, 0);
    }
}

/* N.B.  THE FOLLOWING ROUTINE IS USEFUL IF YOU NEED TO COMPUTE A CRC AND
 *       COMPARE IT TO ONE GENERATED BY HARDWARE, E.G. TOKEN RING CARD.
 *
 * NAME: crcrevhware
 *
 * FUNCTION:
 *    crcrevhware() simulates a crc hardware circuit in software for any
 *    generator polynomial and is useful for computing a CRC for data that
 *    was computed by hardware.
 *
 * NOTES:
 *    This function can be used for either 16 or 32 bit CRC's.  In fact it
 *    was orginally written for 16 bit CRC and I modified it for 32 bits.
 *    I hoped to show with the ifdef's what changes were needed to suit 
 *    your needs.
 * 
 *    This function is typically used to create a table of CRC combined values.
 *    e.g.    
 *            int i;
 *            for (i=0; i < 256; i++) 
 *                crctbl[i] = crchware(i, RGPOLY, 0);
 *         where RGPOLY is the generator polynomial reversed (8408)
 *
 *    It could be called repeatedly to calculate a CRC.
 *    e.g.
 *            char *ptr;
 *            int i;
 *            ulong accum = ~0;
 *            for (i=0; i < datalen; i++)
 *                  accum = crchware( *(ptr+i), GPOLY, accum);
 *
 * RETURN VALUE:
 *
 *       0 : The last two/four bytes of the data contained the correct 
 *           CRC for the data space.
 *
 *   accum : A two/four byte CRC for the data.
 */

ulong
crcrevhware(data, genpoly, accum)
#ifdef 32bit
    ulong data, genpoly, accum;
#else /* 16bit */
    ushort data, genpoly, accum;
#endif
{
    #define HIGHMASK  0x0001
    int i;

    data >>= 1;
    for (i=8; i > 0; i--) {
	if ((data^accum) & HIGHMASK)
	    accum = (accum >> 1) ^ genpoly;
	else
	    accum >>= 1;
    }
    return accum;
}
	/* THIS CODE INCLUDED ONLY FOR EXPLANATORY PURPOSE ONLY */
#endif
