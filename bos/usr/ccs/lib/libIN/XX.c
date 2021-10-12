static char sccsid[] = "@(#)32	1.6  src/bos/usr/ccs/lib/libIN/XX.c, libIN, bos411, 9428A410j 6/10/91 10:22:49";
/*
 * LIBIN: XXshort, XXlong
 *
 * ORIGIN: 9
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: 
 *	Re-order the bytes of data dependent on cpu type;
 *	the operation is self-inverting; i.e., it doesn't matter whether
 *	data is to be converted from host order to target order or v.v.
 *
 *	The types are:
 *
 *	     0:  lo byte is first memory-order byte, hi byte is last
 *	     1:  bytes are swapped relative to type 0
 *	     2:  words are swapped relative to type 0
 *	     3:  bytes and words are swapped relative to type 0
 * 
 * RETURN VALUE DESCRIPTION: 
 */

#define B(n)        ((unsigned char *)&data)[n]
#define S(a,b)      ( (a)<<8 | (b) )
#define L(a,b,c,d)  S(S((long)S(a,b),c),d)

long
XXlong ( data, cpu )
	long data;
{
	switch( cpu & 3 )
	{   default: return L( B(3), B(2), B(1), B(0) );
	    case 1:  return L( B(2), B(3), B(0), B(1) );
	    case 2:  return L( B(1), B(0), B(3), B(2) );
	    case 3:  return L( B(0), B(1), B(2), B(3) );
	}
}

short
XXshort( data, cpu )
	short data;
{
	if( cpu & 1 )
	    return S( B(0), B(1) );
	else
	    return S( B(1), B(0) );
}
