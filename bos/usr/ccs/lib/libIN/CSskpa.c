static char sccsid[] = "@(#)47	1.6  src/bos/usr/ccs/lib/libIN/CSskpa.c, libIN, bos411, 9428A410j 6/10/91 10:52:46";
/*
 * LIBIN: CSskpa
 *
 * ORIGIN: 9,10
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
 * CSskpa scans str until a character not in set or the NULL terminator of str
 * is encountered.
 *
 * RETURN VALUE DESCRIPTION: 
 * If str is NULL or set is NULL, str is returned.  Otherwise, the address in
 * str of the first character found (which is not in set) is returned; if str
 * consists entirely of chars in set, the address of the terminating NULL is
 * returned.
 */

#include <stdlib.h>
#include <sys/types.h>

char *
CSskpa(str, set)
register char *str, *set;
{
	register char *setp;
	wchar_t str_ch;
	wchar_t set_ch;
	int flag;
	int strcount, setcount;
	int mb_cur_max = MB_CUR_MAX;

	if (str && set) {
 	    if (mb_cur_max == 1) {
		for (; *str; str++) {
			flag = 0;
			for (setp = set; *setp; setp++)
				if (*str == *setp) {
					flag = 1;  /* char found in set */
					break;
				}
			if (flag == 0) /* if character not found in the set */
				return(str); /* then return its address */
		}
	    } else {
		for (; *str; str += strcount) {
			flag = 0;
			strcount = mbtowc(&str_ch, str, mb_cur_max);
			for (setp = set; *setp; setp += setcount) {
				setcount = mbtowc(&set_ch, setp, mb_cur_max);
				if (str_ch == set_ch) {
					flag = 1;  /* char found in set */
					break;
				}
			}
			if (flag == 0) /* if character not found in the set */
				return(str); /* then return its address */
		}
	    }
	}
	return(str);
}
