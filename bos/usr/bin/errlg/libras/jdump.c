static char sccsid[] = "@(#)90	1.1  src/bos/usr/bin/errlg/libras/jdump.c, cmderrlg, bos411, 9428A410j 3/2/93 09:00:30";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: _jdump
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Debug routine to "dump" a hex buffer into the Debug trace file.
 * the format is similar to od -ch
 *
 * NAME:     _jdump
 * FUNCTION: Debug
 * INPUTS:   'str'   label string printed in Debug file before hex dump
 *           'cp'    pointer to buffer,
 *           'count' number of bytes in buffer
 * RETURNS:  NONE
 *
 * NOTE:     If 'Debugflg' is not on, _jdump returns.
 */

#include <libras.h>

static char *ctrlstr();

#define ITOC(ip) ((unsigned char *)(ip))
#define ROUND(x) ((int)((x)) & ~3)

_jdump(str,cp,count)
char *str;
unsigned char *cp;
{
	int i,j;
	int n;
	int isave;
	int *ip;

	if(!Debugflg)
		return;
	if(count > 128) {
		ip = (int *)ROUND(cp + count - 1);
		while(ITOC(ip) > cp && ip[-1] == 0)
			ip--;
		count = ITOC(ip) - cp;
	}
	Debug("JDUMP: %s\n",str ? str : "");
	if(count == 0) {
		Debug("JDUMP: count == 0\n");
		return;
	}
	isave = 0;
	for(i = 0; i < count; i++) {
		Debug("%02x ",cp[i]);
		if(i % 16 == 15) {
			Debug("   ");
			for(j = 0; j < 16; j++)
				Debug("%s",ctrlstr(cp[isave+j]));
			Debug("\n");
			isave = i+1;
		}
	}
	n = count % 16;
	if(n != 0) {
		for(j = n; j < 16; j++)
			Debug("   ");
		Debug("   ");
		for(j = 0; j < n; j++)
			Debug("%s",ctrlstr(cp[isave+j]));
	}
	Debug("\n");
}

static char *ctrlstr(c)
{
	static char buf[2];

	buf[0] = (' ' < c && c < 0x7F) ? c : '.';
	buf[1] = '\0';
	return(buf);
}

