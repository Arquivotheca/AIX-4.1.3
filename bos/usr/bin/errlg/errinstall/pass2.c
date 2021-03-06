static char sccsid[] = "@(#)60        1.11.1.3  src/bos/usr/bin/errlg/errinstall/pass2.c, cmderrlg, bos411, 9428A410j 9/16/93 17:48:42";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: pass2, genmsgfile 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:     pass2
 * FUNCTION: Second pass of errinstall
 *           The "work" is in a list of 'md' structures created by pass1.
 *           Remove duplicates and sort the installed set/codepoint data
 *              (md structures) and put in a codept file.
 *              
 * INPUTS:   NONE (input is from global list 'md')
 * RETURNS:  NONE (error code from global Errflg)
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/err_rec.h>
#include <codepoint.h>
#include "errinstall.h"

extern FILE *Undofp;

FILE *Codeptfp;

struct cp_hdr    C_hdr;		/* in-core file header */
struct cp_entry *C_entry;	/* in-core cp_entry portion of file */
char            *C_text;	/* in-core text portion of file */
int              C_offset;	/* starting offset of C_text */
int 		 Mdcount;


pass2()
{
	int i;
	struct cp_hdr   cp_hdr;
	struct cp_entry cp_entry;
	struct md *mdp;
	int offset;
	int count;

	for(i = 1; i <= NERRSETS; i++)
		genmsgfile(i);
	if(checkflg || Mdcount == 0)
		return(0);
	
	if((Codeptfp = fopen(Codeptfile,"w+")) == 0) {
		perror(Codeptfile);
		return(-1);
	}

	/* Create the new codepoint.cat file using the list of 
	   md structures created by pass1().  */

	count = 0;
	for(i = 1; i <= NERRSETS; i++) {
		next_mdinit(1,i);
		while(next_md())
			count++;
	}

	/* write the new codepoint.cat header */
	strncpy(cp_hdr.cp_magic,MAGIC,sizeof(cp_hdr.cp_magic));
	cp_hdr.cp_count = count;
	time(&cp_hdr.cp_fill);
	cwrite(&cp_hdr,sizeof(cp_hdr));

	/* write the new message id's, sets, and text pointers */
	offset = sizeof(cp_hdr) + count * sizeof(cp_entry);
	for(i = 1; i <= NERRSETS; i++) {
		next_mdinit(1,i);
		while(mdp = next_md()) {
			cp_entry.ce_alertid = mdp->md_alertid;
			cp_entry.ce_set     = mdp->md_set;
			cp_entry.ce_offset  = offset;
			offset += strlen(mdp->md_text) + 1;
			cwrite(&cp_entry,sizeof(cp_entry));
		}
	}
	/* write the new message text */
	for(i = 1; i <= NERRSETS; i++) {
		next_mdinit(1,i);
		while(mdp = next_md())
			cwrite(mdp->md_text,strlen(mdp->md_text) + 1);
	}
	fclose(Codeptfp);

	return(0);
}

/* 
 * NAME:	genmsgfile()
 * FUNCTION:    Deletes each mdp entry from cdp table.  If
 *		length of text > 0, add/update, and reinstall
 *		in cdp table.
 * RETURNS:	None
 */

static genmsgfile(set)
{
	struct md *mdp,*cdp;
	int count;

	count = 0;
	next_mdinit(0,set);
	for(mdp = next_md(); mdp; mdp = next_md(), count++) {
		cdp = c_lookup(set,mdp->md_alertid);
		if(!forceflg && cdp){
			cat_warn(M(CAT_INS_EXIST),
				"The message id %04X is already defined in message set %s.\n",
				mdp->md_alertid,fsettocode(set));
			Warnflg++;
			continue;
		}
		if(Undofp)
		{
			if(count == 0)
				fprintf(Undofp,"SET %s\n",fsettocode(set));

			fprintf(Undofp,"%04X \"%s\"\n",
			      mdp->md_alertid,cdp ? cdp->md_text : "");
		}
		Mdcount++;
		if(cdp)
			c_delete(set,mdp->md_alertid);
		if(mdp->md_text)
			c_install(set,mdp->md_alertid,mdp->md_text);
	}
}
