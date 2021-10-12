static char sccsid[] = "@(#)76 1.11 src/bos/usr/bin/sccs/lib/dodelt.c, cmdsccs, bos41B, 9504A 12/9/94 09:59:54";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: dodelt, doixg, getadel
 *
 * ORIGINS: 3, 10, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 *
 */

# include       "defines.h"

# define ONEYEAR 31536000L

long	Timenow;

char	Pgmr[LOGSIZE];	/* for rmdel & chghist (rmchg) */
int	First_esc;
int	First_cmt;
int	CDid_mrs;		/* for chghist to check MRs */

char    *fmalloc();

struct idel *
dodelt(pkt,statp,sidp,type)
register struct packet *pkt;
struct stats *statp;
struct sid *sidp;
char type;
{
	extern  char	had[26];
	extern	char	*satoi();
	char *c, *getline();
	struct deltab dt;
	register struct idel *rdp;
	extern char *Sflags[];
	int n, founddel;
	long timediff;
	register char *p;

	pkt->p_idel = 0;
	founddel = 0;

	time(&Timenow);
	stats_ab(pkt,statp);
	while (getadel(pkt,&dt) == BDELTAB) {
		if (pkt->p_idel == 0) {
			if (pkt->p_verbose) {
				if (Timenow < dt.d_datetime)
					fatal(MSGCO(CLKWRNG, "\nThe creation date of the newest delta in the SCCS file is later than\n\
\tthe current time.\n\
\tMake sure that the system date is set correctly or\n\
\tuse the local problem reporting procedures. (co10)\n"));  /* MSG */
				timediff = Timenow - dt.d_datetime;
				if (timediff > ONEYEAR)
					fprintf(stderr,MSGCO(CLKMAYWRNG, "The difference between the current date and the creation date of\n\
\tthe newest delta in the SCCS file is greater than 1 year.\n\
\tMake sure that the system date is set correctly.\n\
\tThis message is only a warning. (co11)\n"));  /* MSG */
			}
			pkt->p_idel = (struct idel *)fmalloc(n=((dt.d_serial+1)*
							sizeof(*pkt->p_idel)));
			zero((char *)pkt->p_idel,n);
			pkt->p_apply = (struct apply *)fmalloc(n=((dt.d_serial+1)*
							sizeof(*pkt->p_apply)));
			zero((char *)pkt->p_apply,n);
			pkt->p_idel->i_pred = dt.d_serial;
		}
		founddel = 0;
		if (dt.d_type == 'D') {
			if (sidp && eqsid(&dt.d_sid,sidp)) {
				copy(dt.d_pgmr,Pgmr);	/* for rmchg */
				zero((char *)sidp,sizeof(*sidp));
				founddel = 1;
				First_esc = 1;
				First_cmt = 1;
				CDid_mrs = 0;
				for (p = pkt->p_line; *p && *p != 'D'; p++)
					;
				if (*p)
					*p = type;
			}
			else
				First_esc = founddel = First_cmt = 0;
			pkt->p_maxr = max(pkt->p_maxr,dt.d_sid.s_rel);
			rdp = &pkt->p_idel[dt.d_serial];
			rdp->i_sid.s_rel = dt.d_sid.s_rel;
			rdp->i_sid.s_lev = dt.d_sid.s_lev;
			rdp->i_sid.s_br = dt.d_sid.s_br;
			rdp->i_sid.s_seq = dt.d_sid.s_seq;
			rdp->i_pred = dt.d_pred;
			rdp->i_datetime = dt.d_datetime;
		}
		while ((c = getline(pkt)) != NULL)
			if (pkt->p_line[0] != CTLCHAR)
				break;
			else {
				switch (pkt->p_line[1]) {
				case EDELTAB:
				case COMMENTS:
				case MRNUM:
					if (founddel)
					{
						escdodelt(pkt);
					}
					if (pkt->p_line[1] == EDELTAB) break;
				continue;
				default:
					fmterr(pkt);
				case INCLUDE:
				case EXCLUDE:
				case IGNORE:
					if (dt.d_type == 'D')
						doixg(pkt->p_line,&rdp->i_ixg);
					continue;
				}
				break;
			}
		if (c == NULL || pkt->p_line[0] != CTLCHAR || getline(pkt) == NULL)
			fmterr(pkt);
		if (pkt->p_line[0] != CTLCHAR || pkt->p_line[1] != STATS)
			break;
	}
	return(pkt->p_idel);
}


static getadel(pkt,dt)
register struct packet *pkt;
register struct deltab *dt;
{
	if (getline(pkt) == NULL)
		fmterr(pkt);
	return(del_ab(pkt->p_line,dt,pkt));
}


doixg(p,ixgpp)
char *p;
struct ixg **ixgpp;
{
	int *v, *ip;
	int type, cnt, i;
	struct ixg *cur;
	int xtmp[MAXLINE];

	v = ip = xtmp;
	++p;
	type = *p++;
	NONBLANK(p);
	while (NUMERIC((int)(*p))) {
		p = satoi(p,ip++);
		NONBLANK(p);
	}
	cnt = ip - v;
	while ( cur = *ixgpp )
		ixgpp = &cur->i_next;
	*ixgpp = cur = (struct ixg *) fmalloc(sizeof(*cur) +
					(cnt-1)*sizeof(cur->i_ser[0]));
	cur->i_next = 0;
	cur->i_type = type;
	cur->i_cnt = cnt;
	for (i=0; cnt>0; --cnt)
		cur->i_ser[i++] = *v++;
}
