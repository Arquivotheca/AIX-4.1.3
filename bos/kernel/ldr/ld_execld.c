static char sccsid[] = "@(#)25	1.55.1.2  src/bos/kernel/ldr/ld_execld.c, sysldr, bos41J, 9513A_all 3/28/95 11:28:19";
/*
 * COMPONENT_NAME: (SYSLDR) Program Management
 *
 * FUNCTIONS: gettext(), ld_execload()
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
#include	<sys/adspace.h>
#include	<sys/fp_io.h>
#include	<sys/ldr.h>
#include	<sys/malloc.h>
#include	<sys/proc.h>
#include	<sys/pseg.h>
#include	<sys/seg.h>
#include	<sys/shm.h>
#include	<sys/syspest.h>
#include	<sys/systm.h>
#include	<sys/uio.h>
#include	<sys/user.h>
#include	<sys/vmuser.h>
#include	<sys/xcoff.h>
#include 	<sys/errno.h>
#include 	<sys/lockl.h>
#include	"ld_data.h"


	extern void bcopy();
	extern void bzero();

        /*
         * N.B. one could add text table logic to remember that a file
	 * has already been read into a text segment - since the loader
	 * DOES NOT modify the text segment. Although we do some work to
	 * avoid reading unneeded parts of the file - all parts of the file
	 * which are used are read into the same offsets in the text segment
	 * that they have in the file.
         * Once gettext is done the remainder of the loader does not care if
         * the text segment is mapped or copied
         *
         * filesize is a location in which the size of the file read or
	 * mapped is returned to help with later sanity checks
         */

static int
gettext(
struct sharedstuff	*sspointer,
struct loader_entry_extension *lexpointer,
struct file	*fp)
{
        int     	i,j,k;
        int		rc;


	/* initialize text segment
	 * maps file if possible, otherwise makes sure the text region
	 * is backed with computational memory
	 */
	if (rc=ld_cleartext(sspointer,(caddr_t)lex.h,fp)) return rc;
	
	if (ss.la->la_flags & LA_TEXTALLOC){

	        /* textread reads the right amount at textorg
	         * it computes lex.filehdr, lex.aouthdr, lex.scnhdr
	         * Last parameter is a flags field
	         */
	        if ( rc = ld_textread(sspointer,lexpointer,fp,NULL,(caddr_t)lex.h,
		     SEGSIZE,NULL,0) )
			return rc;
		ld_readprotecttext((caddr_t)lex.h);
	}
	else {
		/* text is mapped - so we need to set lex.filesize to the
		 * size of the file
		 */
		struct stat stat;
		if (rc=fp_fstat(fp,&stat,sizeof stat,FP_SYS)) return rc;
		lex.filesize = stat.st_size;
	}

        if ( rc=ld_sanity(sspointer, lexpointer)){
        	/* if gettext fails, no le is made so we must
        	 * do the shmdt now - ld_usecount won't do it */
		if (!(ss.la->la_flags & LA_TEXTALLOC))
			fp_shmdt(fp,SHM_RDONLY);
       	}

	/* Check to see if the user has marked the executable to privately
	 * load all libraries
	 */
	if (strncmp(hdr.a.o_modtype,"PL",2) == 0)
		ss.flags |= SS_PRIVATELOAD;

	return rc;
	
}


/*
 * NAME:        ld_get_domain(*sspointer, *dompath, *lexpointer)
 *
 * FUNCTION:    This routine will determine if a process should use
 *		a loader domain.  If it should use a loader domain,
 *		then global as well as process specific data structures
 *		are initialized or modified.
 *
 *		Upon entry to this routine,  the global loader lock
 *		must be held.
 *
 * PARAMETERS:  sspointer  - shared stuff(global variables)
 *		dompath - pointer to domapn path in LIBPATH env
 *		lexpointer - loader entry extension
 *
 * MODIFIED	Process anchor will point to loader domain.  Shared
 * DATA		Stuff will point to loader domain.  A new loader
 * 		domain may be added to global list.  Use count in
 *		loader domain will be incremented.
 *
 * RETURN:      0	Successful
 *		!0	Unsuccessful	
 *
 */
int
ld_get_domain(struct sharedstuff *sspointer,
char *dompath,
struct loader_entry_extension *lexpointer)
{
	char *p, *q;
	char path_buf[PATH_MAX];
	struct file *fp;
	struct stat stat;
	struct loader_domain *ld, *back_ld;
	struct domain_entry *de;
	struct domain_altname *da;
	uint fh;
	int can_write = 0;

	/* initialize loader domain field of loader anchor and ss */
	ss.la->la_domain = ss.ld = NULL;

	/*
	 * determine if a loader domain is specified.  check
	 * out the libpath information.
	 */
	if (dompath && *dompath)
		p = dompath;
	else
		p = (char *)lex.ldhdr+lex.ldhdr->l_impoff;
	q = path_buf;
	while(*p && *p != ':' && ((q-path_buf) < PATH_MAX))
		*q++ = *p++;
	*q = '\0';

	/* dompain path is > PATH_MAX,  we can't open */
	if ((q-path_buf) == PATH_MAX)
		return(0);

	/* no libpath information (not very likely/common) */
	if (q == path_buf)
		return(0);

	/*
	 * path_buf now contains the first pathname in the libpath
	 * information.  if this path represents a regular file which
	 * can at least be read by this process, then it will be used
	 * as a loader domain.
	 */
	unlockl(&kernel_anchor.la_lock);
	if (fp_open(path_buf, O_RDONLY, 0, 0, FP_SYS, &fp)) {
		lockl(&kernel_anchor.la_lock,LOCK_SHORT);
		return(0);			/* unable to open for reading */
	}

	if (fp_fstat(fp, &stat, sizeof(stat), FP_SYS)) {
		fp_close(fp);
		lockl(&kernel_anchor.la_lock,LOCK_SHORT);
		return(0);			/* unable to stat */
	}
		
	if (!S_ISREG(stat.st_mode)) {
		fp_close(fp);
		lockl(&kernel_anchor.la_lock,LOCK_SHORT);
		return(0);			/* not a regular file */
	}


	/*
	 * check the processes access to the file.  only if the
	 * process can write to the file,  can it add members
	 * to the associated loader domain.
	 */
	if (!fp_accessx(fp, W_OK, ACC_SELF))
		can_write++;


	/*
	 * fp is a file pointer to a file that is associated with
	 * the loader domain.  search the list of existing loader
	 * domains for a match.
	 */
	lockl(&kernel_anchor.la_lock,LOCK_SHORT);
	ld = lib.la.la_domain;
	fh = FPtoFH(fp);
	while (ld) {
		if (FPtoFH(ld->ld_fp) == fh)
			break;
		else
			ld = ld->ld_next;
	}

	if (ld) {	/* a match was found */
		unlockl(&kernel_anchor.la_lock);
		fp_close(fp);
		lockl(&kernel_anchor.la_lock,LOCK_SHORT);
	}

	/*
	 * no match was found.  if process has sufficient permission,
	 * create a new domain.
	 */
	else if (can_write) {
		uint alloc_size;

		/*
		 * Allocate space for the loader domain.
		 */
		alloc_size = (q-path_buf) + 1 +
			     sizeof(struct loader_domain);
		if (!(ld = xmalloc(alloc_size, 2, lib.la.la_data_heap))) {
			/* not enough space to create a domain */
			unlockl(&kernel_anchor.la_lock);
			fp_close(fp);
			lockl(&kernel_anchor.la_lock,LOCK_SHORT);
			return(ENOMEM);
		}

		/*
		 * Initialize fields in the domain
		 */
		bzero(ld, alloc_size);
		ld->ld_name = (char *)ld + sizeof(struct loader_domain);
		strcpy(ld->ld_name, path_buf);
		ld->ld_fp = fp;

		/* add new domain to global list */
		ld->ld_next = lib.la.la_domain;
		lib.la.la_domain = ld;
	}

	else {	/* insufficient permission to create a new domain */
		unlockl(&kernel_anchor.la_lock);
		fp_close(fp);
		lockl(&kernel_anchor.la_lock,LOCK_SHORT);
		ld_emess(L_ERROR_DOMCREAT, path_buf, NULL);
		return(EACCES);
	}

	/*
	 * set flag in loader anchor to indicate process can add entries
	 * to the loader domain
	 */
	if (can_write)
		ss.la->la_flags |= LA_DOMAIN_WRITE;

	/*
	 * update use count in global domain
	 */
	ld->ld_usecount++;
	ss.la->la_domain = ld;	
	ss.ld = ld;
	(void)ld_pathinit(sspointer);
	
	return(0);
}


int (*
ld_execload(
struct	file	*fp,
char	*libpath,
char	*dompath)
)()
{

	struct	sharedstuff	sharedstuff,*sspointer;
	/*N.B. see ldr.h for use of lexpointer*/
	struct	loader_entry_extension	loader_entry_extension,*lexpointer;
	struct  loader_entry	*le;
	int rc;
	int          (*entrypoint)();
	int	i;
	uint	temp;
	caddr_t	textorg;
	int	waslongjmpx;
	label_t	jbuf;


	/* absolutely first - set up shared stuff addressability
	 * N.B. ss is really (*sspointer) !
	 */
	sspointer = &sharedstuff;
	bzero(sspointer,sizeof sharedstuff);
	lexpointer = &loader_entry_extension;
	bzero(lexpointer,sizeof loader_entry_extension);

	u.u_error = 0;
	/* we load into the current data segment, which has already been
	 * cleared. However, the code distinquishes between the location
	 * of the data and its load origin since these same services are
	 * used to load data in the shared library.  Shared library data
	 * is loaded into the library but relocated to its target location
	 * in the data segment.
	 */
	ss.type = 'E';				/* mark this as execload */
	lex.h = (void *)TEXTORG;                /* text goes hear */


	ss.end_of_new = NULL;	/* for execload we are starting empty */
	ss.libpath_env = libpath;
	ss.maxbreak = (char *)DATAORG + U.U_dmax;
	U.U_dsize = 0;

	/* set up to catch I/O errors in mapped files */
	waslongjmpx = rc = setjmpx(&jbuf);
	if (rc) {
		struct loader_entry *prevle;

		/* Make sure that we have the kernel lock at this point */
                lockl(&kernel_anchor.la_lock,LOCK_SHORT);

		/* make sure everything is addressable */
		if (ss.flags & SS_LD_ADDRESSLIB) {
			/*
			 * In this case we know ld_addresslib
			 * was called,  but we still can't be sure
			 * the segment we need is addressable.  An
			 * easy way to do this is to cleanup and
			 * then call ld_addresslib again.
			 */
			ld_cleanup(sspointer);
			ld_addresslib(sspointer);
		}
		else
			ld_addresslib(sspointer);

		/*
		 * Look for a library entry marked IOPENDING by this thread.
		 * If we find one,  then it must be backed out.
		 */
		le = lib.la.la_loadlist;
		prevle = NULL;
		while (le) {
                        if ((le->le_flags & LE_IOPENDING) &&
                            (le->le_tid == curthread->t_tid)) {
				/* found one */
				if (prevle)
					prevle->le_next = le->le_next;
				else
					lib.la.la_loadlist = le->le_next;

				xmfree(le,lib.la.la_data_heap);
				/* wakeup anyone waiting on entry */
				e_wakeup(&ld_loader_read);
				break;
			}
			prevle = le;
			le = le->le_next;
                }
		unlockl(&kernel_anchor.la_lock);

		ld_emess(L_ERROR_SYSTEM,"EFAULT",NULL);
		goto exitrc;	/* fault occured - such as I/O error in
				   mapped file */
	}

	/* for now compute the address of the anchor this way for historical
	 * reasons - once things stabalize we will move the loader area into
	 * the u_block declare and fix this statement
	 */
	ss.la = (struct loader_anchor *)(U.U_loader) ;
	ss.la->la_loadlist = NULL;
	ss.la->la_execle = NULL;
	ss.la->la_text_heap = (caddr_t)(ss.la+1);
	ss.la->la_data_heap = (caddr_t)(ss.la+1);
	ss.la->la_lock = LOCK_AVAIL;
	ss.la->la_flags = LA_UNSURE;
	ss.la->minbreak = (caddr_t)DATAORG;
	ss.la->sbreak = (caddr_t)DATAORG;    /* data will be allocated here */
	ss.la->lastbreak = (caddr_t)DATAORG;
	ss.la->la_dinuse = NULL;		/* user data allocated */
	ss.la->la_emess = NULL;			/* error message chain */
	ss.la->la_ovfl_srval = NULLSEGVAL;
	ss.la->la_save_srval = NULLSEGVAL;
	ss.la->la_domain = NULL;

        /* get the executable into the text segment - either map it or
	 * read what we need
	 */
        if (rc=gettext(sspointer,lexpointer,fp)) {
        	if(rc == ENOEXEC)
        		ld_emess(L_ERROR_FORMAT,NULL,NULL);
		goto exitrc;
	}


	/* initialize loader entry loadlist */
	{
	     uint size;
            /* -1 because sizeof(struct loader_entry) contains space for
	     * one slot
	     */
            size = sizeof(struct loader_entry) +
		   sizeof(struct loader_entry*)*(lex.nimpid-1);
	    le = (struct loader_entry *) ld_ualloc(sspointer, size );
	} /* end of use of size */
	lex.le = le;
	ss.la->la_loadlist = le;
	ss.la->la_execle = le;
	le->le_next = NULL;
	le->le_usecount = 1;
	le->le_loadcount = 0;
	le->le_flags = LE_TEXT|LE_DATA|LE_THISLOAD|LE_EXECLE;
	le->le_fp = fp;
        fp_hold(fp);
	le->le_file = (void *)lex.h;	/* the execfile always goes here */
	le->le_filesize = lex.filesize;
	le->le_data = NULL;
	le->le_datasize = 0;
	le->le_exports = NULL;
	le->le_lex = lexpointer;
	le->le_defered = NULL;
	le->le_filename = U.U_comm;
	le->le_ndepend=0;
	le->le_maxdepend=lex.nimpid;



	
	/* librarys will find and if needed instantiate all the libraries
	 * used by this program.  In the process, it may allocate load list
	 * entries and chain them on the load anchor of the segment
	 * liblist is a vector parallel to the list of library file in
	 * the loader section.  Each entry points to a symbol resolution
	 * hash table
	 */


	lockl(&kernel_anchor.la_lock,LOCK_SHORT);/*this form does not fail*/

        /* establish address of library segment in privlege key
         */
        ld_addresslib(sspointer);

	/* check for/establish loader domain */
	if (rc = ld_get_domain(sspointer, dompath, lexpointer)){
		unlockl(&kernel_anchor.la_lock);
		goto exitrc;
	}

	/* before paging is on, we try to keep the shared library
	 * as small as possible - part of this is done be purging	
	 * all zero use count things before each load.  This is	
	 * a poor performer but is the best that can be easily done	
	 * for space.
	 * We also try to cleanup if the shared library segment is
	 * getting full.  When a request for space in the library
	 * segment fails,  shlibseg_full is set.
	 */

	if (ps_not_defined || shlibseg_full){
		struct	sharedstuff	sharedstuff,*sspointer;
		sspointer = &sharedstuff;
		bzero(sspointer,sizeof sharedstuff);
 		ss.la=&(lib.la);
 		ss.type='L';
 		ld_unload(sspointer);
		if (shlibseg_full)
			shlibseg_full = 0;
 	}

        rc = ld_libraries(sspointer,le,ld_create_initialcall);

	ld_freelex(lib.la.la_loadlist,NULL);

	unlockl(&kernel_anchor.la_lock);

	if (rc) goto exitrc;

	/* assign data locations for all private loaded programs.
	 * Note that the execloaded program is last on the list - so it
	 * gets the largest address
	 */

	if (rc=ld_assigndata(sspointer)) goto exitrc;

	/* resolve processes the symbol table
	 * it computes a vector of relocation factors for each symbol in
	 * the table and computes the number of exports (if any)
	 * If symbols are unresolved it relocates them to 0 or to a trap
	 * depending on type and constructs a list of all such symbols
	 */

	if (rc=ld_resolve(sspointer)) goto exitrc;

	/* map or copy data areas */

	if (rc=ld_mapdata(sspointer)) goto exitrc;

	/* relocate processes the rld information, relocating each symbol*/
	
	if (rc=ld_relocate(sspointer)) goto exitrc;

	/* Store start of the text and size of the text and data sections
	 * (in bytes) in the new u_block
	 */
	U.U_tstart = le->le_file + hdr.s[hdr.a.o_sntext-1].s_scnptr;
	U.U_tsize = hdr.a.tsize;
	if (rc=BRK(ss.la->sbreak))
		goto exitrc;
	ss.la->minbreak = ss.la->sbreak;

        /* return the entry point  */

	if (hdr.a.o_entry == -1 )
		rc = ENOEXEC;
	else {
		entrypoint = (int(*)())
		    (hdr.a.o_entry + (hdr.a.o_snentry == hdr.a.o_sntext ?
		     lex.textreloc	: lex.datareloc));

		rc = 0;
	}

exitrc:
	if (! waslongjmpx)
		clrjmpx(&jbuf);
	
	ld_freelex(ss.la->la_loadlist,NULL);
 	ld_pathclear(sspointer);
	/* platform specific cleanup - probably address space manipulation*/
	ld_cleanup(sspointer);
	u.u_error = rc;
	if (rc) {
		char errno[32];
		sprintf(errno,"%d",rc);
		ld_emess(L_ERROR_ERRNO,errno,NULL);
		return NULL;
	}
	else {
		ss.la->la_flags &= ~LA_UNSURE;
        	return entrypoint;
	}
}
