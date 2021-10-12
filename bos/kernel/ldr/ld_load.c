static char sccsid[] = "@(#)22	1.30.3.5  src/bos/kernel/ldr/ld_load.c, sysldr, bos41J, 9518A_all 4/28/95 14:29:44";
/*
 * COMPONENT_NAME: (SYSLDR) Program Management
 *
 * FUNCTIONS: ld_clearle(), ld_functola(), ld_functole(), ld_purgenew(),
 *            ld_loadmodule(), loadx(), loadbindx(), ld_unload(),
 *            unloadx(), ld_ptracecall()
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
#include	<sys/limits.h>
#include	<sys/ldr.h>
#include	<sys/malloc.h>
#include	<sys/priv.h>
#include	<sys/proc.h>
#include	<sys/pseg.h>
#include	<sys/seg.h>
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
 * NAME:	ld_ptracecall(struct sharedstuff *sspointer)
 *
 * FUNCTION:	This routine is designed to be a front end for the
 *		call to ld_ptrace().  In a multi-threaded process it
 *		is very important to consider the order of operations
 *		when updating the text segments.  This routine is
 *		responsible for getting those sequence of operations
 *		correct.
 *
 * PARAMETERS:  sspointer - shared stuff
 *
 * RETURN:	None
 *
 */
void
ld_ptracecall(
struct sharedstuff	*sspointer)
{
	vmhandle_t sl_srval = 0, ts_srval = 0;
	extern vms_delete();

	/* signal the process to stop it */
	pidsig(U.U_procp->p_pid, SIGTRAP);

	/*
	 * Call ld_ptrace to make new writable copies of the
	 * text segments.  After doing this we can safely delete
	 * the old segments.  The segment register values are
	 * protected by the global ptrace lock.
	 */
	if (ss.la->la_flags & LA_PTRACE_SL) {
		sl_srval = as_getsrval(&U.U_adspace,SHTEXTORG);
		ss.la->la_flags &= ~(LA_PTRACE_SL);
	}
	if (ss.la->la_flags & LA_PTRACE_TS) {
		ts_srval = as_getsrval(&U.U_adspace,TEXTORG);
		ss.la->la_flags &= ~(LA_PTRACE_TS);
	}

	/* make the new copies */
	assert (ld_ptrace(U.U_procp->p_pid) == 0);

	/* delete the old segments */
	if (sl_srval)
		VMS_DELETE(SRTOSID(sl_srval));
	if (ts_srval)
		VMS_DELETE(SRTOSID(ts_srval));

	/*
	 * set stopped in load/unload flag
	 */
	fetch_and_or(&U.U_procp->p_atomic, SLWTED);

	return;
}


/* N.B. this service is different from the dir=-1 ld_usecount.
 * When an le is cleared, all the use counts it generates are reduced by
 * one.  But when an le is duplicated by fork, ONLY the usecounts outside
 * the copied process image must be increased.  Thus you can't common this
 * code with the usecount loop
 * NOTE: Assumes overflow segment is addressable if necessary
 */
void
ld_clearle(
struct sharedstuff	*sspointer,
struct loader_entry	*le)
{
	uint	i;
	struct	loader_entry	*dle;
	struct  loader_defered  *ld;
	for(i=0;i<le->le_ndepend;i++){
		dle=le->le_depend[i];
		ASSERT(dle);
		dle->le_usecount -=1;
		}
	le->le_ndepend = 0;
	if (le->le_fp) FP_CLOSE(le->le_fp);
	le->le_fp = NULL;
	while(le->le_defered){
		ld=le->le_defered->next;
		ld_ufree(sspointer,le->le_defered);
		le->le_defered = ld;
	}
  			
	if (le->le_exports && !(le->le_flags&LE_LIBEXPORTS))
		ld_ufree(sspointer,le->le_exports);
	le->le_exports = NULL;
}

/* routine to convert loader handle to entry address - if it is passed an
 * anchor verifies that the handle is valid. Returns null if handle is
 * invalid or unloaded
 * NOTE: Assumes overflow segment is addressable if necessary
 */

int (*(
ld_entrypoint(
struct loader_entry *le,
struct loader_anchor *la)
)) ()
{
	struct loader_entry *p;
	struct myxcoffhdr	*h;
	if (la)	
	      for(p = la->la_loadlist; p != le; p =p ->le_next)
			if (!p) return NULL;
        /* we must compute the entry point without assuming that lex exists
	 * since this load may have been satisfied by a reusable already
	 * loaded program
	 * If a module doesn't have an entrypoint we return the address of
	 * its data section.  Hopefully, the user knows what he's doing.
	 * We must have a way to load "entrypointless" modules.  A better
	 * way would be to fabricate a procedure descriptor IN THE DATA AREA
	 * of the module which points to trap.
	 * N.B. the actual entrypoint pointer MUST be within the data
	 *      area since it serves as a module id.  If it doesn't,
	 *      the user can never unload.
         */


	h = (void*)le->le_file;
	if (h->a.o_entry == -1)
		return (void*)(le->le_data);
	else
		return (void*)
		       (h->a.o_entry + (h->a.o_snentry == h->a.o_sntext ?
			  (uint)le->le_file + h->s[h->a.o_sntext-1].s_scnptr
			                    - h->s[h->a.o_sntext-1].s_vaddr :
			  (uint)le->le_data - h->s[h->a.o_sndata-1].s_vaddr));

}

/*
 * NOTE: Assumes overflow segment is addressable if necessary
 */
struct loader_anchor *
ld_functola(
int (*func)())
{
	struct	loader_anchor	*la;
	switch((uint)func - SEGOFFSET((uint) func)){
		case KERNELORG:
			la=&kernel_anchor;
			break;
		case SHDATAORG:
			/* Note that a function in the shared library data
			 * segment should be assumed to be on the process
			 * private load list.  Only an exhaustive search of
			 * the load list(ld_searchll) can prove this.
			 */
		case PRIVORG  :
			la=(struct loader_anchor*)(U.U_loader);
			break;
		case SHTEXTORG:
			la=&(lib.la);
			break;
		default:
			if (U.U_segst[(uint)func>>SEGSHIFT].segflag
					& (SEG_WORKING|SEG_SHARED))
				la=(struct loader_anchor*)(U.U_loader);
			else
				return NULL;
	}
	
	return la;
}

/*
* ld_searchll - Search a load list for a loader entry that contains
*		the given function descriptor address.
* NOTE: Assumes overflow segment is addressable if necessary
*/

static
struct loader_entry *
ld_searchll(
struct loader_anchor *la,
int (*func)())
{
	struct loader_entry *le;

	for (le = la->la_loadlist; le; le = le->le_next)
		if ((le->le_flags & LE_DATAEXISTS) &&
			(le->le_data <= (char *)func) &&
			((le->le_data+le->le_datasize)>(char *)func))
				return le;

	return NULL;
}

struct loader_entry *
ld_functole(
int (*func)())
{
	struct	loader_anchor	*la;

	if (!(la = ld_functola(func)))
		return NULL;

	return ld_searchll(la,func);
}

/*
 * NOTE: Assumes overflow segment is addressable if necessary
 */
void
ld_purgenew(
struct sharedstuff	*sspointer)
{
	struct loader_entry	*le,*nextle;
	int	waslocked;
	extern int vm_release();

	waslocked = lockl(&kernel_anchor.la_lock,LOCK_SHORT);	
	/*
	 * assume freelex is already done.
	 * must loop threee times.  once with global shared library data
	 * segment addressable,  the other two with the per-process
	 * library segment addressable.
	 */

	/* First loop will decrement use counts */
	for(le = ss.la->la_loadlist; le != ss.end_of_new; le = le->le_next) {
		/* undoes file ref, use counts, defered,exports */
		ld_clearle(sspointer,le);
	}

	/*
	 * Second loop will free data and text
	 */
	if ((ss.type == 'E') || (ss.type == 'C')) {
		/* We must make the per-process shared library data segment
		 * for this process addressable.  We don't want to delete
		 * stuff in the global shared library data segment.
		 */
		ld_addressppseg(sspointer);
	}
	for(le = ss.la->la_loadlist; le != ss.end_of_new; le = le->le_next){
  		if (le->le_flags&LE_DATA && le->le_data &&
  		    (le->le_data < le->le_file ||
		     le->le_data >= (le->le_file+le->le_filesize)) )
  			ld_dfree(sspointer,PAGEDOWN(le->le_data),
				PAGEUP(le->le_data+le->le_datasize)-PAGEDOWN(le->le_data));
  		
  		if (le->le_flags&LE_TEXT){
  			uint	size;
			if (le->le_flags & LE_TEXTMAPPED)
				size = PAGEUP(le->le_file + le->le_filesize) -
					PAGEDOWN(le->le_file);
			else
  				size = le->le_filesize;
			if (le->le_flags & LE_SYSCALLS)
				/* re-fetch protect this kernel memory*/
				vm_protect(le->le_file,size,KERKEY);
			ld_dfree(sspointer,le->le_flags&LE_TEXTMAPPED ?
				(void *)PAGEDOWN(le->le_file):
				(void *)le->le_file, size);
#if defined(_KDB)
			if (__kdb())
				kdb_freedesc(le);
#endif /* _KDB */
  		}
	}
	if ((ss.type == 'E') || (ss.type == 'C')) {
		/* Restore addressability to the global shared
		 * library data segment
		 */
		ld_restdataseg(sspointer);
	}

	/*
	 * Third loop will free the loader entries themselves
	 */
	for(le=ss.la->la_loadlist;le!=ss.end_of_new;le=nextle){
		nextle = le->le_next;
		ld_ufree(sspointer,le);
	}

	ss.la->la_loadlist = ss.end_of_new;
	
	if (waslocked != LOCK_NEST) unlockl(&kernel_anchor.la_lock);
}

int (*
ld_loadmodule(
struct	sharedstuff	*sspointer,
char	*filenameparm,
uint	flags,
char	*libpathparm)
)()
{
	struct  loader_entry	*le;
	int rc;
	int         (*entrypoint)();
	extern int copyinstr(), copystr();
	int	i;
	uint	temp;
	caddr_t	textorg;
	struct myxcoffhdr *h;
	char	libbuf[PATH_MAX+1];
	char	filebuf[PATH_MAX+2];
	char	*filename,*libpath;
	int	length;
	volatile int	waslocked = LOCK_FAIL;
	int	waslongjmpx;
	label_t	jbuf;

	u.u_error = 0;
	
	waslongjmpx = rc = setjmpx(&jbuf);
	if (rc) {
		/*
		 * Most of the exception code should only be run if we
		 * were in the process performing a load into a process
		 * address space.  It should not be run if we were loading
		 * a kernel extension.
		 */
		if (ss.type != 'K') {
			struct loader_entry *prevle;

			/* Make sure that we have the kernel lock */
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
			 * Look for a library entry marked IOPENDING by this
			 * process. If we find one,  it must be backed out.
			 */
			le = lib.la.la_loadlist;
			prevle = NULL;
			while (le) {
				if ((le->le_flags & LE_IOPENDING) &&
				   (le->le_tid == curthread->t_tid)) {
					/* found one */
					if (prevle)
						prevle->le_next=le->le_next;
					else
						lib.la.la_loadlist=le->le_next;

					xmfree(le,lib.la.la_data_heap);
					/* wakeup anyone waiting on entry */
					e_wakeup(&ld_loader_read);
					break;
				}
				prevle = le;
				le = le->le_next;
			}
		} /* if not a kernel load */

                if (IS_LOCKED(&kernel_anchor.la_lock) &&
		    waslocked == LOCK_SUCC)
			unlockl(&kernel_anchor.la_lock);

		goto exitrc;	/* fault occured - such as I/O error in
				   mapped file */
	}

	/* copy in the library path if specified */
	if (libpathparm){
		if (flags & LD_USRPATH)
			rc=COPYINSTR(libpathparm,libbuf,sizeof libbuf,&length);
		else
			rc=copystr(libpathparm,libbuf,sizeof libbuf,&length);
		/* translate rc */
		if (rc == E2BIG) rc = ENAMETOOLONG;
		else if (rc) rc = EFAULT;
		if (rc) goto exitrc;
		libpath = libbuf;
	}
	else libpath = NULL;

	ss.libpath_env = libpath;

	/* copy in the filename */
	if (flags & LD_USRPATH)
		rc=COPYINSTR(filenameparm,filebuf+1,sizeof filebuf -1,&length);
	else
		rc=copystr(filenameparm,filebuf+1,sizeof filebuf -1,&length);
	/* translate rc */
	if (rc == E2BIG) rc = ENAMETOOLONG;
	else if (rc) rc = EFAULT;
	if (rc) goto exitrc;

	/* if libpath is specified, convert file name to correct form - namely
	 * path part null basename null.  If no libarary, then call with the
	 * use the file name as is form */

	if (libpath) {
		char	*p,*q;
		p = filebuf + length - 1;	/* last actual character */
		for(;p != filebuf && *p != '/' ;p--);	/* last "/" OR past
							   beginning */
		if ( p == filebuf ) {		/* no "/" found */
			filename = filebuf;
			*filename = 0;
		}
		else if ( p == filebuf+1 ) {	/* name is of form /foo */
			filename = filebuf;
	 		*filename = '/';
			*(filename+1) = 0;
		}
		else { 				/* name is of form ???/foo */
			*p = 0;
			filename = filebuf + 1;
		}
	}
	else {
		libpath = filebuf+1;
		filename = NULL;
	}

	/* set up privleged addressability to shared library */
	if (ss.type != 'K'){
		ld_addresslib(sspointer);
	}
	waslocked = lockl(&kernel_anchor.la_lock,LOCK_SHORT);	

        /* load the program*/
        le = ld_getlib(sspointer,libpath,filename,"\0",ld_create_initialcall);


	if (ss.type != 'K'){
 		ld_freelex(lib.la.la_loadlist,NULL);
	}
	
	if (waslocked != LOCK_NEST) unlockl(&kernel_anchor.la_lock);
	if (!le) {
		rc = u.u_error ? u.u_error : ENOEXEC;
		goto exitrc;
	}

	/* assign data locations for all private loaded programs.
	 */


	if (rc=ld_assigndata(sspointer)) goto exitrc;

	/* resolve processes the symbol table
	 * it computes a vector of relocation factors for each symbol in
	 * the table and computes the number of exports (if any)
	 * If symbols are unresolved it relocates them to 0 or to a
	 * trap depending on type and constructs a list of all such symbols
	 */

	if (rc=ld_resolve(sspointer)) goto exitrc;

	/* map or copy data areas */

	if (rc=ld_mapdata(sspointer)) goto exitrc;

	/* relocate processes the rld information, relocating each symbol*/
	
	if (rc=ld_relocate(sspointer)) goto exitrc;

	/* if previous load have left lists of undefinied symbols around,
	 * try to resolve any which have been defined by the new things
	 * just loaded
	 */
	if (ss.la->la_flags & LA_DEFERED)
		if (rc = ld_redo(sspointer))
			goto exitrc;

	/* if there are exports, we build an export hash table */
	/* N.B. for type 'K' lex always exist */
	if (ss.type == 'K' && le->le_lex->numexports && (flags & LD_KERNELEX)){
		if (rc = ld_syscall(sspointer,le))
			goto exitrc;

	}
	entrypoint = ld_entrypoint(le,NULL);

        le->le_usecount += 1;
	le->le_loadcount += 1;

	if (flags & L_NOAUTODEFER)
		le->le_flags |= LE_NOAUTODEFER;

exituerror:
	
	/* run freelex over the whole load list - previously loaded entries may
	 * have been marked THISLOAD if used in this load
	 */
	if (! waslongjmpx)
		clrjmpx(&jbuf);
	ld_freelex(ss.la->la_loadlist,NULL);
 	ld_pathclear(sspointer);
	if (rc ) { /*error cleanup*/
		ld_purgenew(sspointer);
	}
	/* platform specific cleanup - probably address space manipulation*/
	ld_cleanup(sspointer);
        return (rc) ? NULL : entrypoint;


exitrc:
	if (!u.u_error) u.u_error = rc;
	rc = -1;
	goto exituerror;
}

/*
 * NAME: load
 *
 * FUNCTION: Load system call
 *
 * EXECUTION ENVIRONMENT:  Called from process level only
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:  Pointer to the loaded function's entry point 
 *	or   NULL on failure.
 */
int (*
loadx(
    char	*filenameparm,
    uint	flags,
    char	*libpathparm)
)()
{

	struct	sharedstuff	sharedstuff,*sspointer;
	int	(*entrypoint)();
	int	unlock_pplock = 0;
	int	ovfl_was_attached;

	/* absolutely first - set up shared stuff addressability
	 * N.B. ss is really (*sspointer) !
	 */
	sspointer = &sharedstuff;
	bzero(sspointer,sizeof sharedstuff);

	ss.type = 'C';			/* mark this as load command  */

	ss.la = (struct loader_anchor *)(U.U_loader);

	/* get per-process loader lock in a multi-threaded process */
	if (curproc->p_active != 1) {
		lockl(&ss.la->la_lock, LOCK_SHORT);
		unlock_pplock++;
	}

	ss.maxbreak = PTRADD(DATAORG , U.U_dmax);	/* must have lock */
	ss.end_of_new = ss.la->la_loadlist;

	/* check for a loader domain in use */
	if ((ss.la)->la_domain)
		ss.ld = (ss.la)->la_domain;

        ld_emessinit();

	/* pathinit puts pathnames of already loaded programs into the
	 * path lookaside so new requests for the "same" files return the
	 * same fp - this prevents loading a new copy of libc.a (for example)
	 * because a new version was installed while the process was running
	 * This is a compromise - sometimes the programmer wants a new copy
	 * but othertimes, as with libc.a, a new copy is a disaster - you can't
	 * have two mallocs and two file stream i/o packages running in the
	 * same process.  A programmer can force a new copy by unloading
	 * all existing copies since unloaded programs do not contribute to
	 * the path lookaside. Also, this method can be "fooled" is the
	 * "same" file is loaded under a different name - but that can't
	 * be helped.
	 */
	/* NOTE - Names for loaded modules may be in the shared lib global
	 * 	data segment.  Therefore,  we must make this segment
	 *	addressable while calling ld_pathinit().  However,  the users
	 *	private	shared lib data segment may contain data we need to
	 *	access after the call to ld_pathinit().  This means the global
	 *	segment should be addressable only for the duration of this
	 *	call.
	 */
	ld_addresslib(sspointer);
	ld_pathinit(sspointer);
	ld_cleanup(sspointer);

	/* dinuse entries may be in the overflow segment,  therefore it must
	 * be made addressable while accessing list
	 */
	ovfl_was_attached = ld_ovflatt(ss.la);
	if (ss.la->lastbreak != ss.la->sbreak){
		struct	dinuse	*dinuse;
		if ( !(dinuse = ld_ualloc(sspointer,sizeof(struct dinuse))) ){
			errno = ENOMEM ;
			/* release per-process loader lock */
			if (unlock_pplock)
				unlockl(&ss.la->la_lock);
			if (ovfl_was_attached)
				ld_ovfldet(ss.la);
			return 0 ;
		}
		dinuse->next = ss.la->la_dinuse;
		ss.la->la_dinuse = dinuse;
		dinuse->start = ss.la->lastbreak;
		dinuse->length = (ulong)(ss.la->sbreak - ss.la->lastbreak);
		ss.la->lastbreak = ss.la->sbreak;
	}
	if (ovfl_was_attached)
		ld_ovfldet(ss.la);

	entrypoint = ld_loadmodule(sspointer,filenameparm,
				flags|LD_USRPATH, libpathparm);

	/* If tracing,  make new writable copies of the text segments */
	if (U.U_procp->p_flag & STRC)
		ld_ptracecall(sspointer);

	/* release per-process loader lock for multi-threaded process */
	if (unlock_pplock)
		unlockl(&ss.la->la_lock);
	
	return entrypoint;

}

int (*
load(
    char	*filenameparm,
    uint	flags,
    char	*libpathparm)
)()
{	/* temporary for compatibility */
	return loadx(filenameparm,flags,libpathparm);
}

/* loadbind syscall - resolves defered symbols needed by importer to
 * exported symbols of exporter
 */
int
loadbindx(int lflags,
void *exporter,
void *importer)
{
	struct	sharedstuff	sharedstuff,*sspointer;
	struct loader_entry *exle,*imle;
	int	unlock_pplock = 0;

	/* absolutely first - set up shared stuff addressability
	 * N.B. ss is really (*sspointer) !
	 */
	sspointer = &sharedstuff;
	bzero(sspointer,sizeof sharedstuff);
	u.u_error = 0;
	ss.type = 'C';			/* mark this as load command  */
	ss.la = (struct loader_anchor *)(U.U_loader);

	/* get per-process loader lock in a multi-threaded process */
	if (curproc->p_active != 1) {
		lockl(&ss.la->la_lock, LOCK_SHORT);
		unlock_pplock++;
	}

	ld_addresslib(sspointer);

	/* check the library anchor */

	if ( !(ss.la==ld_functola(importer))){
		u.u_error = EINVAL;
		goto exit;
	}

	if ( !(ss.la==ld_functola(exporter))){
		u.u_error = EINVAL;
		goto exit;
	}

	exle = ld_functole(exporter);

	imle = ld_functole(importer);
		
	if ( !exle || !imle ){
		u.u_error = EINVAL;
		goto exit;
	}

	u.u_error = ld_bind(sspointer,exle,imle);

exit:
	ld_cleanup(sspointer);

	/* If tracing,  make new writable copies of the text segments */
	if (U.U_procp->p_flag & STRC)
		ld_ptracecall(sspointer);

	/* release per-process loader lock for multi-threaded process */
	if (unlock_pplock)
		unlockl(&ss.la->la_lock);

	return u.u_error ? -1:0;
}

/*
 * NAME:        ld_clean_domain(ld)
 *
 * FUNCTION:    This routine is responsible for cleaning up any zero
 *		use count loader domain entries.  It will be called
 *		whenever a use count on a loader domain entry is
 *		decremented,  and the use count has gone to zero.
 *		It will traverse the list of domains entries in the
 *		specified domain,  searching for those which have a
 *		zero use count.  For those with a zero use count,  the
 *		domain entry structure will be deleted.
 *
 * PARAMETERS:	ld - loader domain to examine
 *
 * RETURN:      no return code
 *
 */
void
ld_clean_domain(
struct loader_domain *ld)
{
	struct domain_entry	*de,
				*prev_de,
				*tmp_de;
	struct domain_altname	*da,
				*tmp_da;

	de = ld->ld_entries;
	prev_de = NULL;

	/*
	 * scan list of domain entries
	 */
	while(de) {
		if (de->de_usecount == 0) {
			/* free any altname structures */
			da = de->de_altname;
			while (da) {
				tmp_da = da;
				da = da->da_next;
				xmfree(tmp_da, lib.la.la_data_heap);
			}

			/* unlink from list */
			if (prev_de)
				prev_de->de_next = de->de_next;
			else
				ld->ld_entries = de->de_next;

			/* free domain entry structure */
			tmp_de = de;
			de = de->de_next;
			xmfree(tmp_de, lib.la.la_data_heap);
		}
		else {
			prev_de = de;
			de = de->de_next;
		}
	}
}

/*
 * NAME:        ld_unload_domains()
 *
 * FUNCTION:    This routine is responsible for simply unloading loader
 *		domains.  It will traverse the list of domains searching
 *		for those which have a zero use count.  For those with
 *		a zero use count,  the loader entry structure will be
 *		deleted.
 *
 * RETURN:      no return code
 *
 */
void
ld_unload_domains()
{
	struct loader_domain *ld, *prev_ld, *tmp_ld, *next_ld;
	struct domain_entry *de, *tmp_de;
	struct domain_altname *da, *tmp_da;

	/* follow list starting at anchor */
	prev_ld = NULL;
	ld = lib.la.la_domain;
	while (ld) {
		next_ld = ld->ld_next;

		/* check for zero usecount */
		if (ld->ld_usecount == 0) {
			/*
			 * assert no zero use count loader domain contains
			 * no entries
			 */
			assert(ld->ld_entries == NULL);

			/* unlink domain from list */
			if (prev_ld)
				prev_ld->ld_next = next_ld;
			else
				lib.la.la_domain = next_ld;

			/*
			 * the fp_close operation must be performed
			 * while NOT holding the loader lock.  the
			 * calling routine must get the fp before
			 * calling this routine.  after releasing the
			 * loader lock it can call fp_close()
			 *
			 * if (ld->ld_fp)
			 *	fp_close(ld->ld_fp);
			 */
			xmfree(ld, lib.la.la_data_heap);
		}
		else	{
			prev_ld = ld;
		}

		/* examine next ld */
		ld = next_ld;
	}

	return;
}

/* this program garbage collects the load list anchored by shared stuff.
 * it assumes that the correct lock is already held, and that the heap
 * pointer found by following la_heap is correct.
 * The algorithm is to count down all the use counts implied by all
 * the elements on the list.  Then, iterate over the list looking for
 * non-zero use counts.  For each, count its dependencies back up
 * and mark as thisload.  In the end, any elements not marked thisload
 * must be discarded.
 * Note that the ss.type field must by K for the kernel, L for the library,
 * or other for the user.
 * NOTE: Assumes overflow segment is addressable if necessary
 */
void
ld_unload(
struct sharedstuff *sspointer)
{
	struct	loader_entry	*le,*ple,*nle;
	struct  loader_defered	*ld;
	uint	i,j,k;
	uint	progress;
	ulong	newbreak;
	struct	domain_entry *de;
	int	clean_up_domain = 0;
	extern	int vm_release();
	
	ASSERT(ss.la);

	/* count down phase - loaded entries are immediately skipped*/
	for(le=ss.la->la_loadlist;le;le=le->le_next){
		int loadcount;

		if (le->le_flags & LE_DOMAIN_ENT)
			clean_up_domain++;

		if (le->le_flags & LE_EXTENSION)
			continue;
		/* makesure to turn on thisload in extensions of loaded
		 * things */
		loadcount = le->le_loadcount;
		while(1){
			if (loadcount)
				le->le_flags |= LE_THISLOAD;
			else
				for(i=0;i<le->le_ndepend;i++)
					(le->le_depend[i])->le_usecount -= 1;
			if ( le->le_next &&
			     (LE_EXTENSION & le->le_next->le_flags) )
				le=le->le_next;
			else break;
		}
	}

	/* count up phase */
	progress = 1;
	while(progress){
		progress = 0;
		for(le=ss.la->la_loadlist;le;le=le->le_next){
			if ( (le->le_flags & (LE_THISLOAD|LE_EXTENSION)) ||
			     (0==le->le_usecount) )
				continue;

			while(1){
				le->le_flags |= LE_THISLOAD;
				for(i=0;i<le->le_ndepend;i++)
					(le->le_depend[i])->le_usecount += 1;
				if ( le->le_next &&
				     (LE_EXTENSION & le->le_next->le_flags) )
					le=le->le_next;
				else break;
			}

			progress = 1;
		}
	}

	/* delete phase */
	/* unload may have eliminated all defered - check */
	if ((ss.type == 'E') || (ss.type == 'C')) {
		/*
		 * first adjust loader domain use counts
		 */
		if (clean_up_domain) {
			clean_up_domain = 0;
			for(le=ss.la->la_loadlist;le;le=le->le_next){
				if ((le->le_flags &
				    (LE_DOMAIN_ENT|LE_THISLOAD)) ==
				    LE_DOMAIN_ENT) { 
					assert(le->le_de);
					(le->le_de)->de_usecount--;
					if (!(le->le_de)->de_usecount)
						clean_up_domain++;
				}
			}
		}

		/* We must make the per-process shared library data segment
		 * for this process addressable.  We don't want to delete
		 * stuff in the global shared library data segment.
		 */
		ld_addressppseg(sspointer);
	}
	ss.la->la_flags &= ~LA_DEFERED;
	ple = NULL;
	newbreak = 0;
	for(le=ss.la->la_loadlist;le;le=nle){
		nle = le->le_next;
		if (le->le_flags & LE_THISLOAD) {
			ulong temp;
			le->le_flags ^= LE_THISLOAD;
			if (le->le_defered)
				ss.la->la_flags |= LA_DEFERED;
			ple = le;
			if ((le->le_flags & (LE_DATA|LE_USEASIS|LE_SRO))
			     ==LE_DATA)
				temp=(ulong)(le->le_data)+le->le_datasize;
			else
				temp=0;
			if((le->le_flags&(LE_TEXT|LE_EXECLE))==LE_TEXT)
				temp=MAX((ulong)(le->le_file)+le->le_filesize,temp);
			if(!(le->le_flags & LE_EXECLE) )
				temp=PAGEUP(temp);

			newbreak = MAX(newbreak,temp);
			continue;
		}
		ASSERT(0==(le->le_usecount | le->le_loadcount));
		
  		if (le->le_fp)
			FP_CLOSE(le->le_fp);
  		
  		if (le->le_flags&LE_DATA && le->le_data &&
  		    (le->le_data < le->le_file ||
		     le->le_data >= (le->le_file+le->le_filesize)) )
  			ld_dfree(sspointer,PAGEDOWN(le->le_data),
				PAGEUP(le->le_data+le->le_datasize)-PAGEDOWN(le->le_data));
  		
  		if (le->le_flags&LE_TEXT){
  			uint	size;
			if (le->le_flags & LE_TEXTMAPPED)
				size = PAGEUP(le->le_file + le->le_filesize) -
					PAGEDOWN(le->le_file);
			else
  				size = le->le_filesize;
			if (le->le_flags & LE_SYSCALLS)
				/* re-fetch protect this kernel memory*/
				vm_protect(le->le_file,size,KERKEY);
			ld_dfree(sspointer,le->le_flags&LE_TEXTMAPPED ?
				(void *)PAGEDOWN(le->le_file):
				(void *)le->le_file, size);
#if defined(_KDB)
			if (__kdb())
				kdb_freedesc(le);
#endif /* _KDB */
  		}
  		
  			
  		if (le->le_exports && !(le->le_flags&LE_LIBEXPORTS))
  			ld_ufree(sspointer,le->le_exports);

		
		while(le->le_defered){
			ld=le->le_defered->next;
			ld_ufree(sspointer,le->le_defered);
			le->le_defered = ld;
		} 	
			
		if (ple)
			ple->le_next = nle;
		else
			ss.la->la_loadlist = nle;
		
		ld_ufree(sspointer,le);
	}
	if ((ss.type == 'E') || (ss.type == 'C')) {
		/* Restore addressability to the global shared
		 * library data segment
		 */
		ld_restdataseg(sspointer);
	}

	/*
	 * if necessary clean up loader domain structures now
	 */
	if (clean_up_domain) {
		ld_clean_domain(ss.la->la_domain);
	}

	if (ss.type != 'C')
		return;
	/* recompute break values.  At his point, newbreak is just beyond last
	 * loaded program.  This will be the new value of minbreak.  (minbreak is
	 * used to prevent sbreak() from moving the break lower than a loaded
	 * program.)
	 * if sbreak equals (old) lastbreak, then the only user allocated storage
	 * is represented on the dinuse chain.  In that case, the new sbreak will
	 * be at the end of that storage.  If sbreak > lastbreak (it can't be less)
	 * then sbreak will not be changed by this unload, since the user has
	 * storage allocated beween lastbreak and sbreak.
	 */
	if (ss.la->sbreak == ss.la->lastbreak){
		struct	dinuse	*dinuse;
		ulong	newsbreak;
		newsbreak=0;
		for(dinuse=ss.la->la_dinuse;dinuse;dinuse=dinuse->next)
			newsbreak=MAX(newsbreak,(ulong)(dinuse->start)+dinuse->length);
		ss.la->minbreak = (void *)newbreak;	/*reset now so brk works*/ 	
		BRK(MAX(newsbreak,newbreak));
	}
	else
		ss.la->minbreak = (void *)newbreak;
	
}
int
unloadx(
void	*entrypoint)
{
	struct	sharedstuff	sharedstuff,*sspointer;
	struct loader_entry *le;
	int	unlock_pplock = 0;

	/* absolutely first - set up shared stuff addressability
	 * N.B. ss is really (*sspointer) !
	 */
	sspointer = &sharedstuff;
	bzero(sspointer,sizeof sharedstuff);
	u.u_error = 0;
	ss.type = 'C';			/* mark this as load command  */
	ss.la = (struct loader_anchor *)(U.U_loader);

	/* get per-process loader lock in a multi-threaded process */
	if (curproc->p_active != 1) {
		lockl(&ss.la->la_lock, LOCK_SHORT);
		unlock_pplock++;
	}

	ld_emessinit();
	lockl(&kernel_anchor.la_lock,LOCK_SHORT);

	/* see if this is a generic garbage collect request -
	 * encoded by calling unload with L_PURGE
 	 */
 	if (entrypoint == L_PURGE){
 		if (u.u_error=privcheck(SYS_CONFIG))
 			goto exit;
 		ss.la=&(lib.la);
 		/* purge must purge every shared library if we ever support
 		 * more than one.  In that case, the following must loop
 		 * through all the active ones.  For now, just ask for
 		 * addressability to the current and only one. */
 		ss.type='P';	/* P is noticed by addresslib */
 		ld_addresslib(sspointer);
 		ss.type='L';
 		ld_unload(sspointer);
 		ss.la=&kernel_anchor;
 		ss.type='K';
 		ld_unload(sspointer);
		/*
		 * reset ss to point to per-process loader anchor before
		 * attempting to unlock
		 */
 		ss.type='P';
		ss.la = (struct loader_anchor *)(U.U_loader);
 		goto exit;
 	}

	ld_addresslib(sspointer);

	/* find the loader entry on the load list */
	if (!(le = ld_searchll(ss.la,entrypoint))) {
		u.u_error = EINVAL;
		goto exit;
	}

	/*
	 * if load count, count it down and usecount. Purpose is to
	 * prevent counting down usecount not attributed to load
	 * commands
	 */
	if (le->le_loadcount){ 		
		le->le_loadcount -= 1;
		le->le_usecount -= 1;
	}

	le->le_flags |= LE_UNLOAD;  /* mark to prevent future reuse */


	ld_unload(sspointer);                            /* garbage collect */
	
exit:
	ld_cleanup(sspointer);
	unlockl(&kernel_anchor.la_lock);

	/*
	 * If tracing,  make new writable copies of the text segments.
	 * Don't do this in special garbage collect case.
	 */
	if ((U.U_procp->p_flag & STRC) && (entrypoint != L_PURGE))
		ld_ptracecall(sspointer);

	/* release per-process loader lock for multi-threaded process */
	if (unlock_pplock)
		unlockl(&ss.la->la_lock);

	return u.u_error ? -1:0;
}


/*
 * NAME: loadquery
 *
 * FUNCTION: Loadquery system call
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 */
int
loadquery(
int	flags,
void	*buffer,
unsigned int	length)
{
	struct	sharedstuff	sharedstuff,*sspointer;
	register int            rc;
	int	unlock_pplock = 0;

	/* absolutely first - set up shared stuff addressability
	 * N.B. ss is really (*sspointer) !
	 */
	sspointer = &sharedstuff;
	bzero(sspointer,sizeof sharedstuff);

	ss.type = 'C';			/* mark this as load command  */
	ss.la = (struct loader_anchor *)(U.U_loader);

	/* get per-process loader lock in a multi-threaded process */
	if (curproc->p_active != 1) {
		lockl(&ss.la->la_lock, LOCK_SHORT);
		unlock_pplock++;
	}

	/* is this the getmessage call? */
	if (flags == L_GETMESSAGES)
		u.u_error=ld_emessdump(buffer,length,USER_ADSPACE);
	else if (flags == L_GETINFO)
		u.u_error = ld_getinfo_user(-1,length,buffer);
	else
		u.u_error = EINVAL;

	/* release per-process loader lock for multi-threaded process */
	if (unlock_pplock)
		unlockl(&ss.la->la_lock);

	return u.u_error ? -1:0;
}
