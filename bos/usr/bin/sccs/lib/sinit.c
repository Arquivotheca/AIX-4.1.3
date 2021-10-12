static char sccsid[] = "@(#)80 1.9 src/bos/usr/bin/sccs/lib/sinit.c, cmdsccs, bos411, 9428A410j 2/1/94 17:06:31";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: sinit
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

# include 	<errno.h>
# include       "defines.h"

/*
	Does initialization for sccs files and packet.
*/

int old_Fflags;		/* p46254 */
void reset_flags();	/* p46254 */

sinit(pkt,file,openflag)
register struct packet *pkt;
register char *file;
{
	extern	char	*satoi();
	register char *p;
	FILE *fdfopen();
	char *getline();
	extern char had_standinp;      /* p46254+ */
	extern int is_dir;
	int standinp_exist = 0;
	int xfd;      		       /* p46254. */

	zero((char *)pkt,sizeof(*pkt));
	if (size(file) > FILESIZE)
		fatal(MSGCO(PATHTOOLNG, "\nThe maximum path name is 510 characters. (co7)\n"));  /* MSG */
	if (!sccsfile(file))
		fatal(MSGCO(NOTSCCS, "\nSpecify an SCCS file with this command. (co1)\n"));  /* MSG */
	copy(file,pkt->p_file);
	pkt->p_wrttn = 1;
	pkt->do_chksum = 1;	/* turn on checksum check for getline */

	if (had_standinp)              				/* p46254+ */
	    standinp_exist = ((xfd=open(file,0)) < 0)
	 	? ( (errno != EACCES) ? 0 : 1 )
		: 1;                                            /* p46254. */
	if (openflag) {
	    if ((is_dir) || (standinp_exist)) {   		/* p46254+ */
			Ffunc = reset_flags;
                	old_Fflags = Fflags;
                	Fflags &= ~FTLMSG;
                	Fflags |= FTLFUNC; 
		}						/* p46254. */
		pkt->p_iop =xfopen(file,0);
	        if ((is_dir) || (standinp_exist)) 	   	/* p46254 */
			Fflags = old_Fflags;			/* p46254 */
		setbuf(pkt->p_iop,pkt->p_buf);
		fstat(fileno(pkt->p_iop),&Statbuf);
		if (openflag == 1 && Statbuf.st_nlink > 1)
			fatal(MSGCO(MRTHNONELNK, "\nSCCS files can have only one link or name \n\
\tUnlink the file, then create the SCCS file. (co3)\n"));  /* MSG */
		if ((p = getline(pkt)) == NULL || *p++ != CTLCHAR || *p++ != HEAD) {
			fclose(pkt->p_iop);
			fmterr(pkt);
		}
		p = satoi(p,&pkt->p_ihash);
		if (*p != '\n')
			fmterr(pkt);
	}
	pkt->p_chash = 0;
	pkt->p_7bchash = 0;
}

void			/* p46254+ */
reset_flags()
{
	extern int Fcnt;
	Fflags = old_Fflags;
	Fcnt--;
}			/* p46254. */
