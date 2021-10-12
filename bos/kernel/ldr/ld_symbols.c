static char sccsid[] = "@(#)32	1.35  src/bos/kernel/ldr/ld_symbols.c, sysldr, bos41J, 9518A_all 4/28/95 14:30:25";
/*
 * COMPONENT_NAME: (SYSLDR) Program Management
 *
 * FUNCTIONS: ld_deferred(), ld_makedepend(), ld_resolveredo(),
 *            ld_resolve1(), ld_resolve2(), ld_resolve(), ld_relocate1(),
 *            ld_relocate(), ld_bind(), ld_redo()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/xcoff.h>
#include	<sys/seg.h>
#include	<sys/pseg.h>
#include 	<sys/errno.h>
#include 	<sys/lockl.h>
#include	<sys/uio.h>
#include	<sys/ldr.h>
#include	<sys/malloc.h>
#include	<sys/syspest.h>
#include	"ld_data.h"


static int
ld_defered(
struct sharedstuff	*sspointer,
struct loader_entry *le,
uint	i)
{
	struct loader_entry_extension	*lexpointer;
	struct loader_defered	*ld;
	extern void brkpoint();
	lexpointer = le->le_lex;
	ld = ld_ualloc(sspointer,sizeof(struct loader_defered));
	if (!ld) return ENOMEM;
	ld->next = le->le_defered;
	le->le_defered = ld;
	ld->symindex = i;
	lex.locs[i+3].v = 1;
	/* it turns out that if you export an import and the import/export
	 * list says syscall, DS gets converted to SYSCALL - even though
	 * this module is going into the library and the syscall will be
	 * ignored.  So treat it just like the DS it started out to be.
	 */
	ld->value = ( (XMC_DS == lex.ldsym[i].l_smclas) ||
		      (XMC_SV == lex.ldsym[i].l_smclas) ) ?
			  (ulong) brkpoint : -1;
        lex.locs[i+3].d = ld->value - (ulong)(lex.ldsym[i].l_value);
	ss.la->la_flags |= LA_DEFERED;
	return 0;
}

static int
ld_makedepend(
struct sharedstuff	*sspointer,
struct loader_entry *le,
struct loader_entry *rle)
{
	struct loader_entry	*tle;
	uint	i,j,k;
	
	while(1){
		for(i=0;i<le->le_ndepend;i++)
			if (le->le_depend[i] == rle) return 0;
		tle = le->le_next;
		if (!tle ||!(tle->le_flags & LE_EXTENSION))
			break;
		le = tle;
	}
	
	/* fall through if le does not already depend on rle.  In this case,
	 * le now names the last extension if any.  Extensions just carry
	 * additional depend pointers, all other fields are null
	 */

	if (le->le_ndepend < le->le_maxdepend)
		le->le_depend[le->le_ndepend++] = rle;
	else{	/* we must add an extension */
		uint	size;
		size= sizeof(struct loader_entry) +
		      7 * sizeof(struct loader_entry*);
		tle = ld_ualloc(sspointer,size);
		if (!tle)
			return ENOMEM;
		bzero(tle,size);
		tle->le_flags = LE_EXTENSION;
		tle->le_next = le->le_next;
		le->le_next = tle;
		tle->le_maxdepend = 8;
		tle->le_ndepend = 1;
		tle->le_depend[0] = rle;
	}
		
	rle->le_usecount += 1;
	return 0;
	
}

/* tries to resolve defered symbols for this le from list of already
 * loaded things
 */
static int
ld_resolveredo(
struct sharedstuff	*sspointer,
struct loader_entry	*le,
struct loader_entry	*first,
struct loader_entry	*pastlast)
{
        int i,j,k;
        uint	p,q;
	struct loader_entry_extension	*lexpointer;
	struct exp_index	*ex;
	int	rc;
        struct loader_defered	*ld,*pld,*nld;
        struct loader_entry	*rle;
        uint	size;
	
	rc = 0;
	lexpointer = le->le_lex;
        size = sizeof(struct locs)*(lex.ldhdr->l_nsyms+3);
        lex.locs = (void *)xmalloc(size,2,kernel_heap);
        if (lex.locs == NULL) return ENOMEM;
        bzero(lex.locs,size);
	ASSERT(le->le_defered);
	pld = NULL;
	for(ld=le->le_defered;ld;pld=ld,ld=nld){
		int	hashfindrc;
		nld = ld->next;
		i = ld->symindex;
		for(rle = first; rle != pastlast; rle = rle->le_next){
			if(!rle->le_exports) continue;
		        ex = ld_hashfind(rle->le_exports,lex.ldhdr,
				&lex.ldsym[i],&hashfindrc,le);
    		        if (ex && ex->exp_location != (void *)-1){
                   		lex.locs[i+3].d = (ulong) ex->exp_location;
                   		lex.locs[i+3].v = (ulong)ex;	/* non-zero - but
							   more useful */
            			if (LDR_EXPORT(lex.ldsym[i])){
					ld_updatehash(le->le_exports,lex.ldhdr,
					  &lex.ldsym[i],&lex.locs[i+3]);
	    			}
                   		lex.locs[i+3].d -= ld->value;
                   		lex.flags |= FOUNDDEFERED;
                   		/* remove defered block from chain */
                   		if(pld)
                   			pld->next = nld;
                   		else
                   			le->le_defered = nld;
                   		ld_ufree(sspointer,ld);
                   		ld = pld;
                   		if (rc=ld_makedepend(sspointer,le,rle))
                   			return rc;
                   		break;
                   	}
                   	else
			       /* hashfind error - probably type mismatch
			        * or insane offset in loader section
				*/
			       if (!ex && hashfindrc)	
                   		       return hashfindrc;
		} /* end of loop over new programs to resolve a defered
		     symbol */
	}/* end of loop over defered symbols*/
	return 0;
}

/* tries to resolve the symbols for this le.  If any symbols are left
 * unresolved, rc & 1 will be non-zero.  If any EXPORTED symbols where
 * resolved on this pass, rc&2 will be non-zero.
 * (rc is the function return value.)
 */
static int
ld_resolve1(
struct sharedstuff	*sspointer,
struct loader_entry	*le,
int	firsttime)	/*  1 for first time for this le,
			 *  0 for additional times
			 */
{
        int i,j,k;
        uint	p,q;
	struct loader_entry_extension	*lexpointer;
	struct loader_entry  **impid;
	struct exp_index	*ex;
	int	rc;
	int	redefine;
	
	rc = 0;
	lexpointer = le->le_lex;
	/* loop through symbols, computing a delta for each one.
         * sanity check has guaranteed that we can loop through the symbols
	 */
        if (firsttime) {
            lex.locs = (void *)
	      xmalloc(sizeof(struct locs)*(lex.ldhdr->l_nsyms+3),2,kernel_heap);
            if (lex.locs == NULL)
		 return ENOMEM+4;
	    lex.locs[0].d = lex.textreloc;
  	    lex.locs[1].d = lex.datareloc;
            lex.locs[0].v = lex.locs[1].v = 1;
	    lex.locs[2].d = lex.bssreloc;
	    lex.locs[2].v = 1;
        } /* end of if(firsttime)*/
        impid = lex.impid;           /* file name vector build by libraries */
        /* N.B. the first three "symbols" are not in the table
         * thus we fill them in above, and then in the loop that follow
         * we must be careful to add 3 each time we refer to locs
         * (PLEASE trust the compiler to get this without really adding 3
	 * each time!)
         */
        for(i = 0; i < lex.ldhdr->l_nsyms; i++) {
            if ( !firsttime && lex.locs[i+3].v )
	        continue;
            if (LDR_IMPORT(lex.ldsym[i])){
		int	hashfindrc, deferedrc;
		uint	ifile;
		ifile = lex.ldsym[i].l_ifile;
		ASSERT(ifile <= lex.nimpid);
		if (ifile > lex.nimpid )
			return ENOEXEC+4;
		/* binder did it wrong here - instead of passing zero ifile for
		 * defered, it made a null string entry in the name list!
		 */
		if (!ifile || !impid[ifile-1]){
			/* if "defered" symbol was imported at a fixed address
			 * just accept the definition as is */
			if (XMC_XO == lex.ldsym[i].l_smclas){
				lex.locs[i+3].d=0;
				lex.locs[i+3].v=1;
			}
			/* delayed resolution required */
			else if(deferedrc = ld_defered(sspointer,le,i))
				return deferedrc+4;
		}
		else{
			ex = ld_hashfind(impid[ifile-1]->le_exports,lex.ldhdr,
				 &lex.ldsym[i],&hashfindrc,le);
                	if (ex && (int)ex->exp_location != -1){
                 	/* TRICKY CODE - assigns delta AND test for delta
		        * non zero - if delta non zero then checks for import
		       	* at fixed location - in which case delta must be
			* zero. (Non zero means the exportor at a fixed address
                       	* has a different address than the importer at a
			* fixed address
                        */
		        	if ( (lex.locs[i+3].d = (ulong)ex->exp_location -
					   (ulong)(lex.ldsym[i].l_value)) &&
				             (XMC_XO == lex.ldsym[i].l_smclas) )
					   return ENOEXEC+4;
	                        lex.locs[i+3].v = (ulong)ex;
			}
			else
				if (!ex && hashfindrc)
					return hashfindrc + 4;
                        	else {
                         	          lex.locs[i+3].v = 0;
                          	         rc |= 1;
                         	}
		}
            }
            /* if this symbol is not imported, it is a location in text or data
             * thus, its relative relocation is the same as text or data
             */
            else {
                j = lex.ldsym[i].l_scnum - 1;  /* -1 because we keep values
						   zero based internally */
                if (j == lex.text ) lex.locs[i+3].d = lex.textreloc;
                else if (j == lex.data) lex.locs[i+3].d = lex.datareloc;
                else if (j == lex.bss) lex.locs[i+3].d = lex.bssreloc;
                else return ENOEXEC+4;
                lex.locs[i+3].v = 1;
            }
            if (LDR_EXPORT(lex.ldsym[i])){
		if (firsttime) lex.numexports += 1;  /* only used firsttime */
                if (lex.locs[i+3].v) {
			rc |= 2;
			if (!firsttime)
			    ld_updatehash(le->le_exports,lex.ldhdr,
					  &lex.ldsym[i],&lex.locs[i+3]);
		}
	    }
	}
        if (firsttime && lex.numexports) {
            ld_uinitheap(sspointer);
            le->le_exports = ld_buildhash(lex.ldhdr,lex.numexports,
				lex.locs,L_EXPORT,0,NULL,ss.la->la_data_heap,
				&redefine);
	    if(!le->le_exports) return u.u_error ? u.u_error+4:ENOMEM+4;
            le->le_exports->data = (void *)
			    (hdr.s[lex.data].s_vaddr + lex.datareloc);
        }
        return rc;
}

/* get the symbol associated with the i'th ldsym entry
 * ebuf is assumed to be 128 chars long */

ld_getname(
struct	loader_entry_extension	*lexpointer,
int	i,
char	*ebuf)
{
	char    *p,*t,maxlen;
	int	j;
	t = ebuf;
	if (0 == lex.ldsym[i].l_zeroes){
		maxlen=lex.ldhdr->l_stlen - lex.ldsym->l_offset;
		maxlen=MIN(maxlen,128);
		p = (char *) (lex.ldhdr) +
		    lex.ldhdr->l_stoff    +
		    lex.ldsym[i].l_offset;
		for(;(*t++=*p++) && t < ebuf+maxlen;);
		*(t-1) = 0;
	}
	else {
		p = &(lex.ldsym[i].l_name[0]);
		for(;(*t++=*p++)&& t < ebuf+8;);
		if (t == ebuf+8)
			*t++ = 0;
	}
}

/* resolve2 takes a pass over the symbol table looking for unresolved
 * symbols, i.e. locs.v = 0;
 * This pass is only for producing error messages.
 */
static int
ld_resolve2(
struct sharedstuff	*sspointer,
struct loader_entry	*le)
{
	struct loader_entry_extension	*lexpointer;
	int	i,rc,ifile;
	rc = 0;
	lexpointer = le->le_lex;
        for(i=0;i<lex.ldhdr->l_nsyms;i++)
		if (!lex.locs[i+3].v){
			char	ebuf[128];
			rc = 1;
			ld_getname(lexpointer,i,ebuf);
			/* we believe its more useful to display the file
			 * name of the program NEEDING the symbol, not
			 * the one its supposed to be in.  This latter
			 * information can be gotten from the loader data
			 * of the former file.  BUT, the le of the "target"
			 * can be gotten by the following lines if needed.
			 * ifile = lex.ldsym[i].l_ifile;
			 * ld_emess(L_ERROR_UNDEF,ebuf,lex.impid[ifile-1]);
			 */
			ld_emess(L_ERROR_UNDEF,ebuf,le);
		}
#if 0
	return 0;  /* for now accept and ignore unresolved */
#endif
	return rc;	
}

/* resolve loops over the loader entries from ss.la to end_of_new
 * for each it calls resolve1 with firstime = 1.
 * resolve1 returns 0, 4+errno if disaster, 1 if more needed, 2 if
 * something changed.
 */
int
ld_resolve(
struct sharedstuff	*sspointer)
{
	struct loader_entry	*le;
	int	notdone;
	int	rc;
	int	firsttime;
	firsttime = 1;
	notdone = 3;    /* 3 means there are symbols to resolve (1) and a
			 * chance they may be resolved if we try again (2)
			 */
	while (notdone == 3) {
		notdone = 0;
		for(le=ss.la->la_loadlist;le!=ss.end_of_new;le=le->le_next) {
			if ((le->le_flags & (LE_DATA|LE_USEASIS)) != LE_DATA)
			       continue;
			rc=ld_resolve1(sspointer,le,firsttime);
			if (rc > 4) return rc-4;
			notdone |= rc;
		}
		firsttime = 0;
	}
	if (notdone & 1) { /* there are unresolveds left over */
		notdone = 0;
		for(le=ss.la->la_loadlist;le!=ss.end_of_new;le=le->le_next) {
			if ((le->le_flags & (LE_DATA|LE_USEASIS)) != LE_DATA)
			       continue;
			notdone |= ld_resolve2(sspointer,le);
		}
	}
	return (notdone&1) ? ENOEXEC : 0;
}

static int
ld_relocate1(
struct sharedstuff	*sspointer,
struct loader_entry	*le)
{
	/* sanity check has guaranteed that we can loop over the rld's */
	int	i,j,k;
        uint    ri;
        ulong	rv;
        char	*rtarg;
        ulong	dsize;
        ulong	rsize,rbsize;
        char	*tdataloc;
        char	rtype;
        char	*dvaddr;		/*virtual origin of data*/
        uint	numesd;
        struct  loader_entry_extension *lexpointer;
        int	redo;

	lexpointer = le->le_lex;
	redo = lex.flags & FOUNDDEFERED;
        dsize = hdr.a.o_dsize + hdr.a.o_bsize; /* allow rld's in BSS*/
        dvaddr = (char *)hdr.s[lex.data].s_vaddr;
        numesd = lex.ldhdr->l_nsyms + 3;    /* 3 for text,data,bss */
        tdataloc = le->le_data;
        for(i = 0; i< lex.ldhdr->l_nreloc; i++){
            /* for now we do NOT relocate text or BSS */
            if ((lex.ldrel[i].l_rsecnm-1) != lex.data) {
            	char	dec[32];
            	sprintf(dec,"%d",i);
            	ld_emess(L_ERROR_RLDBAD,dec,le);
		return ENOEXEC;
	    }
            ri = lex.ldrel[i].l_symndx;       /* index to relocate by */
            /* N.B. ri is zero based where 0,1,2 are text, data,
	     *	     bss and 3 is first symbol
	     */
            if (ri >= numesd) {
            	char	dec[32];
            	sprintf(dec,"%d",i);
            	ld_emess(L_ERROR_RLDBAD,dec,le);
		return ENOEXEC;
	    }
            if (redo && !(lex.locs[ri].v))
            	continue;	              /* in redo, just skip invalids */
            if (! (lex.locs[ri].v)){
            	char	ebuf[128];
            	if (ri > 2){
            		ld_getname(lexpointer,ri-3,ebuf);
            		ld_emess(L_ERROR_UNDEF,ebuf,le);
            	}
            	else
            	 	ld_emess(L_ERROR_SYSTEM,"text or data unresolved",le);
		return ENOEXEC;
	    }
            /* locs table is zero based but contains extra entries
	     * for text, data, bss
	     */
            rv = lex.locs[ri].d;
            rtarg = (char *)(lex.ldrel[i].l_vaddr) - (uint)dvaddr;
            if ((ulong)rtarg >= dsize){
            	char	dec[32];
            	sprintf(dec,"%d",i);
            	ld_emess(L_ERROR_RLDBAD,dec,le);
		return ENOEXEC;
            }
            rbsize = lex.ldrel[i].l_rtype;
            rtype = rbsize & 0xff;
            rbsize = (rbsize>>8)& 0x1f; /*rbsize is rightmost 5 bits*/
            rsize = (rbsize+8)/8;      /* rbsize is size in bits -1 */
            if ((ulong)(rtarg + rsize) > dsize){
            	char	dec[32];
            	sprintf(dec,"%d",i);
            	ld_emess(L_ERROR_RLDBAD,dec,le);
		return ENOEXEC;
	    }
            rtarg = (ulong)rtarg + tdataloc;
            switch (rtype)
            {
                case (R_POS):  /* A (sym) */
                case (R_BA):
                case (R_RBA):
                case (R_REF):
                case (R_RL):
                case (R_TCL):
                case (R_GL):
                    break;
                case (R_NEG):
                    rv = -rv;  /* A(-sym) */
                    break;
                case (R_REL):  /* A(sym-*) */
                case (R_BR):
                case (R_RBR):
                case (R_TOC): /* A(sym-toc) */
                case (R_TRL):
                    rv = rv-lex.datareloc;
                    break;
                case (R_RTB):  /* A(sym-*)/2 */
                case (R_RRTBI):
                    rv = (rv - lex.datareloc)/2;
                    break;
                case (R_RBAC):   /* not relocated */
                case (R_RLA):
                case (R_RBRC):
                case (R_RRTBA):
                case (R_TRLA):
                    rv = 0;
                    break;
                default:{
            		char	dec[32];
            		sprintf(dec,"%d",i);
            		ld_emess(L_ERROR_RLDBAD,dec,le);
			return ENOEXEC;
		}
            } /* end switch */

            if ( ((uint)rtarg & 3)==0 && rbsize == 31 ) *(ulong*)rtarg += rv;
            else {
		/* full blown slow case - don't expect much of this for
		 * PIC code. When we add text relocation, we should get
		 * those special cases
		 */
                ulong word,mask,new_bits,old_bits;
                /* copy correct number of bytes of adcon to fix */
                rbsize++;
                bcopy(rtarg, (char *)&word+4-rsize , rsize);

                /* change only the appropriate bits within the adcon bytes */
                mask = (0xffffffff) << (rbsize);
                new_bits = (word + rv)  & (~mask);
                old_bits = word & mask;
                word = old_bits | new_bits;

                /* store back the corrected adcon bytes */
                bcopy(&word+4-rsize,rtarg,rsize);
                } /* end full blown case  */
            } /* end for loop over rlds */
            return 0;
}

int
ld_relocate(
struct	sharedstuff	*sspointer)
{
	struct loader_entry	*le;
	int	rc;
	for(le = ss.la->la_loadlist; le != ss.end_of_new; le=le->le_next) {
		if ((le->le_flags & (LE_DATA|LE_USEASIS)) != LE_DATA)
			continue;
		rc=ld_relocate1(sspointer,le);
		if (rc ) return rc;
	}
	return 0;
}

int
ld_bind(
struct sharedstuff *sspointer,
struct loader_entry *exporter,
struct loader_entry *le)    	/*importer*/
{
	struct loader_entry_extension loader_entry_extension,*lexpointer;
	int	rc;
	label_t jmpbuf;

	if ( ! le->le_defered)
		return 0;		/* nothing left to resolve */
	ASSERT((!le->le_lex));
	le->le_lex = lexpointer = &loader_entry_extension;
	lex.le = le;
	bzero(lexpointer,sizeof *lexpointer);
	lex.h = (void *)le->le_file;
	lex.filesize = le->le_filesize;
	if ((rc = setjmpx(&jmpbuf)) == 0) {
		/*
		 * establish exception handler to catch I/O errors in mapped
		 * files.  normal code path follows. in the exception code
		 * path,  rc will be set and returned to the caller.
		 *
		 */
		/* recompute needed values in lex */
		rc = ld_sanity(sspointer, lexpointer);

		if (!rc)
			rc = ld_resolveredo(sspointer, le, exporter,
					    exporter->le_next);

		if (!rc && (lex.flags & FOUNDDEFERED)) {
			ld_addressppseg(sspointer);
			/* perform relocation with per-process segment */
			ld_relocate1(sspointer,le);
			ld_restdataseg(sspointer);
		}

		clrjmpx(&jmpbuf);
	}
	if (lex.locs)
		xmfree(lex.locs,kernel_heap);
	le->le_lex = NULL;
	return rc;
}


/* defered resolutions are "redone" here if possible.  The set up is that
 * the le's from loadlist to end_of_new are "new" this load.  They may
 * define symbols which where unresolved previously.  The le's from
 * end_of_new to the end are the candidates to have their defered symbols
 * resolved.
 * N.B. this code is brute force - it counts on defered resolution being
 *      a relatively infrequent event for relatively few symbols.  If
 *      this proves not to be the case, a more complicated scheme may be
 *      needed. The overall flow is to pass over the "old" le's.  For each
 *      with defered symbols, we search ALL the new le's for each defered
 *      symbol, setting up a new locs vector.  At the end, we run relocate
 *      again.  The locs values of most symbols will be zero.  Those which
 *      were defered will have non zero value and be relocated.
 */
int
ld_redo(
struct sharedstuff *sspointer)
{
	struct loader_entry_extension loader_entry_extension,*lexpointer;
	struct loader_entry *le,*nle,*ple,*tle;
	int	rc;

	lexpointer = &loader_entry_extension;
	/* along the way, we recompute LA_DEFERED to be on only if
	 * defered's still remain after this redo
	 */
	ss.la->la_flags &= ~LA_DEFERED;

	/* first allocate lex structures for all le's with defered lists
	 * and relocate
	 */
	for(le = ss.end_of_new; le; le=le->le_next) {
		if (le->le_flags & LE_NOAUTODEFER || ! le->le_defered)
			continue;
		bzero(lexpointer,sizeof *lexpointer);
		ASSERT((!le->le_lex));
		le->le_lex = lexpointer;
		lex.le = le;
		lex.h = (void *)le->le_file;
		lex.filesize = le->le_filesize;
		/* recompute needed values in lex */
		rc = ld_sanity(sspointer, lexpointer);
		if (!rc)
			rc = ld_resolveredo(sspointer,le,ss.la->la_loadlist,
					    ss.end_of_new);
		if (0==rc && (lex.flags & FOUNDDEFERED)) {
			/* process the relocation entries associated with
			 * defered symbols.  Be sure to have the per-process
			 * segment addressable,  so that changes are made in
			 * the users segment.
			 */
			ld_addressppseg(sspointer);
			rc=ld_relocate1(sspointer,le);
			ld_restdataseg(sspointer);
		}
		if (lex.locs)
			xmfree(lex.locs,kernel_heap);
		if (rc)
			return rc;
		le->le_lex = NULL;
		if (le->le_defered)
			ss.la->la_flags |= LA_DEFERED;
	}
	if (!(ss.la->la_flags & LA_DEFERED))
		for(le = ss.la->la_loadlist; le != ss.end_of_new;le=le->le_next)
			if (le->le_defered){
				ss.la->la_flags |= LA_DEFERED;
				break;
			}
	return 0;
}
