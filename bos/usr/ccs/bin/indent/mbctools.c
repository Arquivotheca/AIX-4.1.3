static char sccsid[] = "@(#)52  1.2  src/bos/usr/ccs/bin/indent/mbctools.c, cmdprog, bos411, 9428A410j 6/21/91 15:17:59";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: nextmbc, prevmbc, putmbc, cpymbc, rtrimmbc, widthmbc
 *
 * ORIGINS: 26; 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*-
 * FILE NAME:
 *      mbctools.c
 * PURPOSE:
 *      Contains routines to handle general capabilities.
 * GLOBALS:
 *      None
 * FUNCTIONS:
 *      nextmbc
 *      prevmbc
 *      putmbc
 *      cpymbc
 *      rtrimmbc
 *      widthmbc
 *
 */

#include "indent_msg.h"
#include "ind_globs.h"
#include <stdlib.h>

/*
 * NAME: nextmbc
 *
 * FUNCTION: Find the first byte of the next [multibyte] character in the
 * buffer to aid in looking for 7-bit ASCII characters.
 *
 * ALGORITHM: If the MB_CUR_MAX is 1 then just the next byte;  otherwise skip
 * the number of bytes which comprise the current [multibyte] character.
 *
 * RETURNS: Pointer to the next apporpriate byte in the buffer;  if the current
 * character is invalid, a pointer to the next byte is returned.
 *
 * HISTORY: initial coding      April 1991 M S Flegel of IBM(ACTC)
 */
char *nextmbc (p_buff)

char *p_buff;                                   /* buffer to skip through */
{
char *p;
                                                /* not a MB codepage */
    if ((indent_mb_cur_max <= 1) || !(*p_buff & 0x80))
        return (p_buff+1);

    if ((p = mbsadvance (p_buff)) <= 0)         /* possible multibyte */
        return (p_buff+1);

    return (p);
}

/*
 * NAME: prevmbc
 *
 * FUNCTION: Find the first byte of the previous [multibyte] character in the
 * buffer to aid in looking for 7-bit ASCII characters.
 *
 * ALGORITHM: If the MB_CUR_MAX is 1 then just the previous byte; otherwise,
 * start from MB_CUR_MAX back into the buffer, and look for the latest complete
 * [multi-byte] character, if none exist, then return one byte back
 *
 * RETURNS: Pointer to the previous appropriate byte in the buffer;  if there are
 * no previous valid characters, then a pointer to the previous byte is returned.
 *
 * NOTES: It is assumed that there IS a previous character!
 *
 * HISTORY: initial coding      April 1991 M S Flegel of IBM(ACTC)
 */
char *prevmbc (p_buff)

  char *p_buff;                                 /* buffer to skip through */
{
char *p, *q;
int i;

    if (indent_mb_cur_max <= 1)                 /* one byte codepage */
        return ((p_buff-1));

    p = p_buff - indent_mb_cur_max;             /* start from MB_CUR_MAX back */
    do
    {
        q = p;
        i = mblen (p, indent_mb_cur_max);
        p += (i > 0) ? i : 1;
    } while (p < p_buff);
    return(q);
}

/*
 * NAME: putmbc
 *
 * FUNCTION: Output the bytes of a [multibyte] character in accordance with the
 * algorithm defined in "nextmbc".
 *
 * ALGORITHM: See "nextmbc".
 *
 * RETURNS: None.
 *
 * HISTORY: initial coding      April 1991 M S Flegel of IBM(ACTC)
 */

void putmbc (p_buff, p_fp)

char *p_buff;                   /* buffer containing MBc */
FILE *p_fp;                     /* output file */
{
int   i, l;
                                /* not an MB codepage */
    if ((indent_mb_cur_max <= 1) || !(*p_buff & 0x80))
        putc (*p_buff, p_fp);
    else                        /* entire MB, or one byte */
    {
        l = mblen (p_buff, indent_mb_cur_max);
        i = 0;
        do
        {
            putc (p_buff[i++], p_fp);
        } while (i < l);
    }
}

/*
 * NAME: cpymbc
 *
 * FUNCTION: Copy an MB character from a source into a target.
 *
 * ALGORITHM: See "nextmbc".
 *
 * RETURNS:  Number of bytes copied.
 *
 * NOTES: If the character is invalid, then one byte is copied
 *
 * HISTORY: initial coding      April 1991 M S Flegel of IBM(ACTC)
 */

int cpymbc (p_target, p_source)

char *p_target;                 /* target buffer */
char *p_source;                 /* source buffer */
{
int   i, l;
                                /* not an MB codepage */
    if ((indent_mb_cur_max <= 1) || !(*p_source & 0x80))
    {
        *p_target = *p_source;
        return (1);
    }
    else
    {
        l = mblen (p_source, indent_mb_cur_max);
        i = 0;
        do
        {
            p_target[i] = p_source[i];
            i += 1;
        } while (i < l);
        if (l <= 0)                             /* invalid MB copied */
        {
            p_target[i] = 0;                    /* ...NULL out the next byte */
            return(1);
        }
        return (l);
    }
}

/*
 * NAME: rtrimmbc
 *
 * FUNCTION: Trim the specified characters off of the rightside of a string.
 *
 * ALGORITHM: While the end char is one of the trimming chars, backup.
 *
 * RETURNS: Pointer to the last character trimmed.
 *
 * NOTES: A null set of trim characters implies everything which is not-printable.
 *
 * HISTORY: initial coding      April 1991 M S Flegel of IBM(ACTC)
 */

char *rtrimmbc (p_s, p_e, p_trim)

char *p_s;                                      /* start of buffer */
char *p_e;                                      /* end of buffer */
char *p_trim;                                   /* chars to trim */
{
char *p;

    if (p_trim != NULL)
    {
        for (p = prevmbc (p_e); p > p_s && strchr (p_trim, *p); p = prevmbc (p))
            ;                                   /* Empty */
    }
    else
    {
        for (p = prevmbc (p_e); p > p_s && *p < 040; p = prevmbc (p))
            ;                                   /* Empty */
    }

    if (p == p_s)                               /* ran out */
        return (p_s);

    return (nextmbc(p));
}

/*
 * NAME: widthmbc
 *
 * FUNCTION: Figure out the display width of a multibyte character.
 *
 * ALGORITHM: If the __max_disp_width global is 1, then simply return 1;
 * otherwise, use "wcwidth" to determine its width.
 *
 * RETURNS: >0 => number of columns required to display
 *           0 => error
 *
 * HISTORY: initial coding      April 1991 M S Flegel of IBM(ACTC)
 */

int widthmbc (p_s)

char *p_s;                                      /* pointer to multibyte character */
{
wchar_t wc;
int     i;

    if (p_s == NULL)                            /* nothing to calculate */
        return (0);

    if (__max_disp_width <= 1)                  /* at most one wide */
        return (1);

    if (mbtowc (&wc, p_s, indent_mb_cur_max) <= 0)      /* invalid - nothing to calculate */
        return (0);

    i = wcwidth (wc);                           /* convert it */
    return ((i > 0) ? i : 0);
}
