static char sccsid[] = "@(#)73 1.13 src/bos/usr/bin/sccs/lib/putline.c, cmdsccs, bos411, 9428A410j 2/1/94 17:06:13";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: putline, flushline, xrm
 *
 * ORIGINS: 3, 10, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */

# include       "defines.h"

/*
	Routine to write out either the current line in the packet
	(if newline is zero) or the line specified by newline.
	A line is actually written (and the x-file is only
	opened) if pkt->p_upd is non-zero.  When the current line from 
	the packet is written, pkt->p_wrttn is set non-zero, and
	further attempts to write it are ignored.  When a line is
	read into the packet, pkt->p_wrttn must be turned off.
*/

int	Xcreate;
FILE	*Xiop;


putline(pkt,newline)
register struct packet *pkt;
char *newline;
{
	static char obf[BUFSIZ];
	char *xf, *auxf();
	register char *p;
	FILE *fdfopen();

	char	*acl_get();
	char	*sptr;

	if(pkt->p_upd == 0) return;

	if(!Xcreate) {
		stat(pkt->p_file,&Statbuf);
		sptr = acl_get(pkt->p_file);
		xf = auxf(pkt->p_file,'x');
		Xiop = xfcreat(xf, 600); /*600 is changed by acl_put*/
		setbuf(Xiop,obf);
		acl_put(xf, sptr, 1);
		chown(xf,Statbuf.st_uid,Statbuf.st_gid);
	}
	if (newline)
		p = newline;
	else {
		if(!pkt->p_wrttn++)
			p = pkt->p_line;
		else
			p = 0;
	}
	if (p) {
		if(fputs(p,Xiop)==EOF)
			FAILPUT;
		if (Xcreate)
			while (*p)
				pkt->p_nhash += (signed char)*p++;
	}
	Xcreate = 1;
}


flushline(pkt,stats)
register struct packet *pkt;
register struct stats *stats;
{
	register char *p;
	char ins[6], del[6], unc[6], hash[6];

	if (pkt->p_upd == 0)
		return;
	putline(pkt,(char *) 0);
	rewind(Xiop);

        if ( stats && (stats->s_ins > 99999 || stats->s_del > 99999 || stats->s_unc > 99999) ) {
                fprintf (stderr, MSGCO(LCNTWARN, "WARNING - The number of lines inserted, deleted or unchanged\n\
\texceeds 99,999.  The header section of the s. file  will record 99999\n\
\tin the appropriate field.  This is only a warning.\n"));
		if ( stats->s_ins > 99999 )
			stats->s_ins = 99999;
		if ( stats->s_del > 99999 )
			stats->s_del = 99999;
		if (stats->s_unc > 99999 )
			stats->s_unc = 99999;
        }

	if (stats) {
		sprintf(ins,"%.05u",stats->s_ins);
		sprintf(del,"%.05u",stats->s_del);
		sprintf(unc,"%.05u",stats->s_unc);
		for (p = ins; *p; p++)
			pkt->p_nhash += (*p - '0');
		for (p = del; *p; p++)
			pkt->p_nhash += (*p - '0');
		for (p = unc; *p; p++)
			pkt->p_nhash += (*p - '0');
	}

	sprintf(hash,"%5u",pkt->p_nhash&0xFFFF);

	if( hash[0] == 0x20 ) hash[0] = 0x30;
	if( hash[1] == 0x20 ) hash[1] = 0x30;
	if( hash[2] == 0x20 ) hash[2] = 0x30;
	if( hash[3] == 0x20 ) hash[3] = 0x30;
	if( hash[4] == 0x20 ) hash[4] = 0x30;

	fprintf(Xiop,"%c%c%s\n",CTLCHAR,HEAD,hash);
	if (stats)
		fprintf(Xiop,"%c%c %s/%s/%s\n",CTLCHAR,STATS,ins,del,unc);
	if (fclose(Xiop)==EOF)
		FAILPUT;
}


/*ARGSUSED*/
xrm(pkt)
struct packet *pkt;
{
	if (Xiop)
		fclose(Xiop);
	Xiop = 0;
	Xcreate = 0;
}
