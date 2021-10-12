/*
 * COMPONENT_NAME: CMDAOUT (libld.a)
 *
 * FUNCTIONS: ldgetname
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef	lint
static char *sccsid = "@(#)70  1.6  src/bos/usr/ccs/lib/libld/ldgetname.c, libld, bos411, 9428A410j 2/17/92 15:46:17";
#endif	lint

#include <stdio.h>
#include <filehdr.h>
#include <syms.h>
#include <ldfcn.h>
#include <dbxstclass.h>
#include <xcoff.h>
#include "takes.h"
#ifdef _IBMRT
#include <sys/types.h>
#endif

char *
ldgetname(ldptr, symentry)
register LDFILE	*ldptr;
register SYMENT	*symentry;
{

	static char buffer[BUFSIZ];
	extern int vldldptr();
	extern char *strncpy();
#ifdef FLEXNAMES
	long position;
	long toff;
	static offset = -1L;
	register char *p;
	register int c = ~0,
		     i;
#endif

	if (vldldptr(ldptr) != SUCCESS)
		return(NULL);
#ifdef FLEXNAMES
	if (symentry->n_zeroes == 0L) {	/* in string table or debug section */
		position = FTELL(ldptr);
		offset = OFFSET(ldptr);

		if (symentry->n_sclass & DBXMASK) {
			if ((toff = get_debug_offset(ldptr)) < 0)
				return (NULL);
			offset += toff;
		} else {
			offset += HEADER(ldptr).f_symptr + 
				(HEADER(ldptr).f_nsyms * SYMESZ);
		}

		if (!fseek(IOPTR(ldptr), offset + symentry->n_offset, 0)) {
			for (i = 0, p = buffer; i < BUFSIZ && c && 
				(c = getc(IOPTR(ldptr))) != EOF; ++i, ++p)
                			*p = c;

			(void)fseek(IOPTR(ldptr), position, 0); 
        		if (c == EOF) return(NULL);
			return(buffer);
		} else {
			(void)fseek(IOPTR(ldptr), position, 0); 
			return(NULL);
		}
	}
	else	/* still in old COFF location */
#endif
	{
		(void) strncpy(buffer, symentry->n_name, SYMNMLEN);
		buffer[SYMNMLEN] = '\0';
		return(buffer);
	}
}

long get_debug_offset(ldptr)
LDFILE *ldptr;
{
long position;
static int last_fnum_ = 0;
static long offset;
static long origin = -1L;
int i;
SCNHDR	scnhdr;	

/*
* Different archive members are noted by the OFFSET change.
* Otherwise, normal other ldptr's are distinguished by
* the different _fnum_'s.
*/
  if (origin != OFFSET(ldptr) || last_fnum_ != ldptr->_fnum_) {
	position = FTELL(ldptr);
	last_fnum_ = ldptr->_fnum_;
	origin = OFFSET(ldptr);
	if (fseek(IOPTR(ldptr),(origin + FILHSZ + HEADER(ldptr).f_opthdr),0)){
		(void) fseek(IOPTR(ldptr), position, 0);
		return(-1);
	} else {
		for(i=0; i < HEADER(ldptr).f_nscns; ++i) {
			if (!fread((void *) &scnhdr, (size_t)SCNHSZ, (size_t)1,
			    IOPTR(ldptr))) {
				(void) fseek(IOPTR(ldptr), position, 0);
				return(-1);
			} else {	
                        	if(scnhdr.s_flags & STYP_DEBUG) {
                                	offset= scnhdr.s_scnptr;
					(void) fseek(IOPTR(ldptr), position, 0);
                               		return(offset); 
                       		}
			}
		}			
		(void) fseek(IOPTR(ldptr), position, 0); 
		return(-1);
	}
  } else {
	return(offset);
  }
}
	
/* static char ID[] = "ldgetname.c: 1.2 2/16/83"; */
