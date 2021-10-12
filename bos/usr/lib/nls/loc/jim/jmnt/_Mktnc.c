static char sccsid[] = "@(#)26	1.4.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mktnc.c, libKJI, bos411, 9428A410j 7/23/92 03:23:46";
/*
 * COMPONENT_NAME :	(LIBKJI) Japanese Input Method (JIM)
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */

/*
 *      include Kanji Project.
 */
#include "kj.h"         /* Kanji Project Define File.                   */
#include "kcb.h"        /* Kanji Control Block Structure.               */


/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

extern unsigned short	_toJIS83();

/*
 *      JIS Kuten code to PC kanji code conversion.
 */
_Mktnc( jis,pkc, jis_mode )

uchar    *jis;       /* JIS KUTEN Code (I)                              */
uchar    *pkc;       /* PC  Kanji Code (O)                              */
char	 jis_mode;   /* JIS78, JIS83 Mode switch 			*/
{
extern  int     atoi();         /* convert string to integer.           */
        int     ret_code;       /* Return Code.                         */
        int     cc;             /* convert code.                        */
        int     j;              /* JIS KUTEN code input code. (integer) */
        int     h;              /* JIS KUTEN code high order byte.      */
        int     l;              /* JIS KUTEN code low  order byte.      */
        uchar   jisdata[6];     /* jis KUTEN code input buffer          */
	unsigned short jis_code, new_jis_code;    

/************************************************************************
 *      debug process routine.                                          */
CPRINT(START KMKTNC);

        /* 1.1 Convert input strings to integer.                        */
        memset(jisdata,'\0',6); /* Jis kuten code input area NULL clear.*/
        memcpy(jisdata,jis,5);  /* Jis kuten code input set.            */
        j = atoi(jisdata);      /* Jis kuten code convert integer.      */

        /* 1.2 Division JIS KUTEN code High order byte & Low order byte.*/
        h = j/100;     /* Jis kuten code High order byte.               */
        l = j%100;     /* Jis kuten code Low  order byte.               */

        /* 1.3 Check jis kuten code.                                    */
        /* Hight order byte is less than one,                           */
        /*                 ,or is more then one hundred nineteen.       */
        if ((h < 1) || (h > 120))
                /* Invalid input code.                                  */
                {
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.3 KMKTNC);

                /* Error of execution.Input code is not jis kuten code. */
                return( IMFAIL );
                }

        /* Low order byte is less than one, or is more then ninety four.*/
        if ((l < 1) || (l >  94))
                /* Invalid input code.                                  */
                {
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.3 KMKTNC);

                /* Error of execution.Input code is not jis kuten code. */
                return( IMFAIL );
                }

        /* 1.4 Convert PC Kanji code high byte.                         */
        if (h <= 62)
                cc = 0x81 + ((h - 1)/2);
        else
                cc = 0x81 + (0xdf - 0xa0 +1) + ((h - 1)/2);

        /* 1.5 Convert PC Kanji code Low order byte.                    */
        if (h%2 == 0)
            /* Hight one is even number.                                */
            {
                l = l + 0x9f - 1;
            }
        else
            /* Hight one is odd number.                                 */
            {
                if (l <= 63)
                        l = l + (0x40 - 1);
                else
                        l = l + (0x40 - 1) + 1;
            }

        /* 1.6 PC Kanji code set.                                       */
	if( jis_mode == K_JIS83 ) {
	    jis_code = ((((uchar)cc << 8 ) & 0xFF00 ) | 
		        (((uchar)l & 0x00FF )));
	    if( new_jis_code = _toJIS83( jis_code )) {
                pkc[0] = (uchar)( new_jis_code >> 8 ) & 0xFF ; /* high  */
                pkc[1] = (uchar)( new_jis_code & 0xFF );       /* low   */
	    }
	    else {
                pkc[0] = (uchar)cc;  /* high  order byte set.           */
                pkc[1] = (uchar)l;   /* low   order byte set.           */
	    }
	}
	else {
            pkc[0] = (uchar)cc;  /* high  order byte set.               */
            pkc[1] = (uchar)l;   /* low   order byte set.               */
	}

/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.6 KMKTNC);

        /* Success of execution.Input code is not jis kuten code.       */
        return ( IMSUCC );
}
