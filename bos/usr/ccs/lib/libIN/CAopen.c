static char sccsid[] = "@(#)09	1.6  src/bos/usr/ccs/lib/libIN/CAopen.c, libIN, bos411, 9428A410j 6/10/91 10:14:08";
/*
 * LIBIN: CAopen
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
 * FUNCTION: Open string for reading or writing via Standard I/O.
 *
 * RETURN VALUE DESCRIPTION: 
 */

#include <stdio.h>

CAopen(str, len, stream)
 char *str;
 FILE *stream;
{
	stream->_flag = _IONBF;
	stream->_ptr = (unsigned char *)str;
	stream->_cnt = len;
	fileno(stream) = -1;
}
