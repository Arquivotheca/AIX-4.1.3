static char sccsid[] = "@(#)68	1.2  src/bos/kernext/dlc/lan/dump.c, sysxdlcg, bos411, 9428A410j 3/13/91 12:27:38";
/*************************************************************************
 * COMPONENT_NAME: SYSXDLCG
 *
 * FUNCTIONS: Dump memory
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 ************************************************************************/

#include <stdio.h>
typedef unsigned char UCHAR;
/*  dump memory routine  */
/*  input : pointer and length */

char buf[4000];

dump(a,l)
char a[];
int l;
{
int t,i,j;
UCHAR *p;
char buf[17];
int offset;

	printf("DUMP addr=%x length=dec[%d] hex[%x]\n",a,l,l);
	if (l >= 16) {
		t = l / 16;
		/* t = number of whole lines */
		for(i=0; i<t; i++) {
			p = a;
			printf("  %03X - %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X   |",offset,
			*p++,*p++,*p++,*p++,*p++,*p++,*p++,*p++,*p++,*p++,*p++,*p++,*p++,*p++,*p++,*p++);
			for (j=0; j<16; j++) {
				buf[j] = *a++;
				if (buf[j] < ' ' || buf[j] > 'z')
					buf[j] = ' ';
			}
			buf[16] = 0x00;
			printf("%s|\n",buf);
			offset += 16;
        		l -= 16;
   		}
	}
	/* do the last line */
	if ((t=l%16) > 0) {
		printf("  %03X - ",offset);
		/* for all available bytes */
		for(i=0; i<t; i++) {
			buf[i] = a[i];
			if (buf[i] < ' ' || buf[i] > 'z')
				buf[i] = ' ';
			if (i > 0 && ((i % 4) == 0))
				printf(" ");
			printf("%02X",a[i]);
		}
		/* finish the line */
		for(i=t; i<16; i++) {
			buf[i] = ' ';
			if ((i % 4) == 0)
				printf(" ");
			printf("--");
		}
		buf[16] = 0x00;
		printf("   |%s|\n",buf);
	}
	return(0);
}
