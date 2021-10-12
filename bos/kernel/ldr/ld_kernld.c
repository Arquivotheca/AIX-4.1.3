static char sccsid[] = "@(#)28	1.32.1.5  src/bos/kernel/ldr/ld_kernld.c, sysldr, bos411, 9428A410j 3/4/94 17:36:25";
/*
 * COMPONENT_NAME: (SYSLDR) Program Management
 *
 * FUNCTIONS: pincode(), unpincode(), ld_syscall(), kmod_load(), kmod_entrypt,
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

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/limits.h>
#include	<sys/errno.h>
#include	<sys/file.h>
#include	<sys/fp_io.h>
#include	<sys/ldr.h>
#include	<sys/lockl.h>
#include	<sys/malloc.h>
#include	<sys/seg.h>
#include	<sys/syspest.h>
#include	<sys/user.h>
#include	<sys/vmuser.h>
#include	<sys/xcoff.h>
#include	"ld_data.h"

/* kmod_lock is a lock used to serialize access to the kernel
 * load list during loads/unloads.  One would think that the
 * lock in the kernel_anchor would accomplish this,  but this
 * is no longer the case.  The lock in the kernel_anchor is
 * released while we are performing file I/O.  This allows
 * someone else to modify the kernel load list while we are in
 * the process of adding entries to the list.  This will cause
 * serious problems.
 * IMPORTANT NOTE - processes will be holding both the kmod_lock
 * and kernel_anchor lock at the same time.  It is very important
 * that the kmod_lock always be acquired before acquiring the
 * kernel_anchor lock.  One should NEVER attempt to acquire the
 * kmod_lock while holding the kernel_anchor lock.
 */
lock_t kmod_lock = LOCK_AVAIL;

/* routine to pin a loaded program
 * multiple pin calls are ok - but caller must make matching unpin calls.
 * pin keeps use counts per page to get this right.
 */

int
pincode(
int (*func)())
{
	struct loader_entry	*le;
	int	rc;

	if (((uint)func >> SEGSHIFT) != KERNELSEG)
		return EINVAL;

	if (!(le=ld_functole(func)))
		return EINVAL;

	if(rc=ltpin(le->le_file,le->le_filesize))
		return rc;	/* pin the code */

	if(rc=ltpin(le->le_data,le->le_datasize)){
		ltunpin(le->le_file,le->le_filesize);
		return rc;
	}
	return 0;
}

/* routine to unpin a loaded program
 */

int
unpincode(
int (*func)())
{
	struct loader_entry	*le;
	int	rc1,rc2;

	if (((uint)func >> SEGSHIFT) != KERNELSEG)
		return EINVAL;

	if (!(le=ld_functole(func)))
		return EINVAL;

	rc1 = ltunpin(le->le_file,le->le_filesize);	/* unpin the code */
	rc2 = ltunpin(le->le_data,le->le_datasize);	/* unpin the data */
	return rc1 ? rc1 : rc2;
}

   /* install this module as a kernel extension
    * a kernel extension extends the name space defined by the anchors
    * syscall_exports and kernel_exports
    * kernel_exports will point to the loader entry for this module,
    * while syscall_exports will point to a second entry for the user form
    * of the symbols.
    * The le for syscalls ALWAYS immediately FOLLOWS the le for the kernel
    * extension it is part of on the load list.
    */
int
ld_syscall(
struct sharedstuff *sspointer,
struct loader_entry *le)
{

	uint	numsvcs, i, j, k;
	struct svc_table_entry *svc;
	struct loader_entry_extension *lexpointer;
	int redefine;

	lexpointer = le->le_lex;

	if (le->le_exports)
		xmfree(le->le_exports,kernel_heap);

	le->le_exports =
	      ld_buildhash(lex.ldhdr, lex.numexports, lex.locs,L_EXPORT, 0,
	                   kernel_exports, kernel_heap, &redefine);

	/* This flag will never be used in the case of the extension
	 * it will only be used in the SYSCALLS entry.  We set it here
	 * only for analysis.
	 */
	if (redefine)
		le->le_flags |= LE_REDEFINEEXP;

	if ( ! le->le_exports )
		return u.u_error ? u.u_error : ENOMEM;

        /*
	 * construct loader entry and hash for kernel svcs
	 */
	ss.sysle =  xmalloc(sizeof(struct loader_entry ) +
		    sizeof(struct loader_entry *), 0, kernel_heap);
	if (!ss.sysle)
		return ENOMEM;

	bzero(ss.sysle, sizeof(struct loader_entry ));

	/* chain these in order kernel exports then syscall exports
	 * we do this now and then are careful to keep all the le invarients
	 * true.
	 * if there is an error purgenew will clean up - if we've been
	 * careful here
	 */
	ss.sysle->le_next = le->le_next;
	le->le_next = ss.sysle;

	ss.sysle->le_flags = LE_SYSCALLS | LE_DATA| LE_DATAEXISTS;
	ss.sysle->le_depend[0] = syscall_exports;
	syscall_exports->le_usecount += 1;
	ss.sysle->le_depend[1] = le;
	le->le_usecount += 1;
	/*
	 * for the user, this name space is called /unix no matter what
	 * extensions have been loaded.
	 */
	ss.sysle->le_filename = kernel_filename;
	ss.sysle->le_ndepend = 2;
	ss.sysle->le_maxdepend = 2;

	/* must take a pass just to count svc's*/
	numsvcs = 0;
	for (i = 0; i < lex.ldhdr->l_nsyms; i++)
		if ((lex.ldsym[i].l_smtype & L_EXPORT) == L_EXPORT
	             && lex.ldsym[i].l_smclas == XMC_SV )
			numsvcs++;

        /*
	 * we make a new table even if no svcs just to make life simple for
	 * unload
	 */

	j = sizeof(struct svc_table_entry ) * numsvcs +
		     syscall_exports->le_filesize;

	/*N.B. use file field to remember table*/
	svc = xmalloc(j, PGSHIFT, kernel_heap);
	if ( !svc)
		return ENOMEM;
	vm_protect(svc, j, UTXTKEY);

	ss.sysle->le_file = (void * )svc;
	ss.sysle->le_filesize = j;
	bcopy(syscall_exports->le_file, svc, syscall_exports->le_filesize);

	j = syscall_exports->le_filesize / sizeof(struct svc_table_entry );
	numsvcs += j;  /* only for the ASSERT below */
	for (i = 0; i < lex.ldhdr->l_nsyms; i++)
		if ( (lex.ldsym[i].l_smtype & L_EXPORT) == L_EXPORT
			&& lex.ldsym[i].l_smclas == XMC_SV ) {
			svc[j].svc0 = svc_instruction;
			svc[j].index = j;
			svc[j].svc = (int (*)())(lex.ldsym[i].l_value +
					 lex.locs[i+3].d); /* "relocation
							       factor"*/
			lex.locs[i+3].d = (ulong) & (svc[j].svc0) -
					   lex.ldsym[i].l_value;
			j++;
		}

	/* TEMPORARY - for fetch-protect/store problem.
	 * This can be removed when we no longer need to support
	 * the h/w level with this problem.
	 *
	 * Copy the svc table to the user-kernel-shadow segment.
	 */
	ukerncopy(svc,ss.sysle->le_filesize);

	ASSERT(numsvcs == j);
	lex.numsvcs = j;
	ss.sysle->le_exports =
		 ld_buildhash(lex.ldhdr, lex.numexports, lex.locs,L_EXPORT, 1,
	                      syscall_exports, kernel_heap, &redefine);

	/* Set the flag in the LE if this redefines a symbol that was
	 * previously defined.  ld_libraries checks this when verifying
	 * dependencies.
	 */
	if (redefine)
		ss.sysle->le_flags |= LE_REDEFINEEXP;

	if (!ss.sysle->le_exports)
		return ENOMEM;
		
	/* if all is well we must chain everything up */
	kernel_exports->le_usecount -= 1;
	kernel_exports = le;
	kernel_exports->le_usecount += 1;
	le->le_flags |= LE_KERNELEX;
	syscall_exports->le_usecount -= 1;
	syscall_exports = ss.sysle;
	syscall_exports->le_usecount += 1;
	svc_table = (struct svc_table_entry *)ss.sysle->le_file;
	svc_table_entries = ss.sysle->le_filesize /
			      sizeof(struct svc_table_entry );

	return  0;
}


/* this is NOT a system call - called only by trusted routines */
/* returns 0 if ok and kmod is set, otherwise returns errno of failure */
int
kmod_load(
caddr_t		path,	/* name of file to load into kernel	*/
uint		flags,
caddr_t		libpath,
mid_t		*kmid)
{


	struct sharedstuff sharedstuff, *sspointer;
	int	rc = 0;		 			/*return code */
	int	i, j, k;
	struct loader_entry	*le;
	int	(*entrypoint)();



	rc = 0;
	sspointer = &sharedstuff;
	bzero(sspointer, sizeof sharedstuff);

	ld_emessinit();
	lockl(&kmod_lock,LOCK_SHORT);
	lockl(&kernel_anchor.la_lock,LOCK_SHORT); /*this form does not fail*/
	ss.type = 'K';			/*mark this as kernel load*/
	ss.end_of_new = kernel_anchor.la_loadlist;
	ss.la = &kernel_anchor;

	if (flags & (LD_SINGLELOAD|LD_QUERY)){
		for(le=ss.la->la_loadlist;le;le=le->le_next){
			char	*p;
			if (!(le->le_flags & LE_DATA))
				continue;
			if (!le->le_filename)
				continue;
			for(p=le->le_filename;*p++;);
			if (strncmp(p,path,PATH_MAX))
				continue;
			if (le->le_loadcount == 0)
				continue;
			if (! (flags & LD_QUERY)){
				le->le_loadcount += 1;
				le->le_usecount += 1;
			}
			*kmid =	(void *) ld_entrypoint(le,NULL);
			goto exit;;
		}
		if (flags & LD_QUERY){
			*kmid = NULL;
			goto exit;
		}	
	}

	if (entrypoint = ld_loadmodule(sspointer,path,flags,libpath)){
		*kmid =	entrypoint;
		rc = 0;
	}
	else
		rc = u.u_error;
	/* for now, we don't reuse code in kernel.  To do so, you have to
	 * change kernelld in ld_libld to work like libld - now it uses
	 * testreadbss to make the data in the loaded program the data
	 * actually used, rather than a copy.
	 * Here, we close all the files involved, since we can't reuse anyhow.
	 * Closing is what gurarantees no reuse.
	 */
	for(le=ss.la->la_loadlist;le != ss.end_of_new;le=le->le_next){
		if (le->le_fp) {
			fp_close(le->le_fp);
			le->le_fp = NULL;
		}
	}
	
exit:
	unlockl(&kernel_anchor.la_lock);
	unlockl(&kmod_lock);
	return rc;
}

int
kmod_unload(
mid_t	kmid,
uint	flags)                  	/* unload flags		*/

{
	int	rc = 0;		        /* return code		*/
        struct loader_entry	*le;
	struct	sharedstuff	sharedstuff,*sspointer;

	/* absolutely first - set up shared stuff addressability
	 * N.B. ss is really (*sspointer) !
	 */
	sspointer = &sharedstuff;
	bzero(sspointer,sizeof sharedstuff);

	ld_emessinit();
	lockl(&kmod_lock,LOCK_SHORT);
	lockl(&kernel_anchor.la_lock,LOCK_SHORT);/*this form does not fail*/

	ss.type = 'K';				/* mark this as kernel  */
	ss.la = &kernel_anchor;

	/* see if this is a generic garbage collect request -
	 * encoded by sending the address of the appropriate library anchor
	 * if so don't do the specific unload.
 	 */

	if(kmid != &kernel_anchor){
	
		if (&kernel_anchor != ld_functola(kmid))
			goto return_EINVAL;

   		if ( ! (le = ld_functole(kmid)) )
			goto return_EINVAL;

	 	if ( le->le_flags & (LE_KERNEL|LE_SYSCALLS))
			goto return_EINVAL;
		
		/*
		 * if load count, count down it and usecount.
		 * Purpose is to prevent counting down usecount not attributed
		 * to load commands
		 */
		if (le->le_loadcount){ 	
			le->le_loadcount -= 1;
			le->le_usecount -= 1;
		}
		else
			goto return_EINVAL;
	
		if (le->le_loadcount == 0){
			/* mark to prevent future reuse */
			le->le_flags |= LE_UNLOAD;
	
			/* check for active kernel extension */
	
			if (kernel_exports == le) {
				struct	loader_entry	*newle;
				kernel_exports-> le_usecount -= 1;
				syscall_exports->le_usecount -= 1;
				for(newle = syscall_exports->le_next; /* initial */
			            (newle->le_flags & (LE_UNLOAD|LE_KERNELEX)) !=
				       LE_KERNELEX;                   /* terminating */
			            newle = newle->le_next)           /* next */
					     assert(newle->le_next);
				kernel_exports = newle;
				for(newle = newle->le_next;newle->le_flags&LE_EXTENSION;
					newle=newle->le_next) assert(newle);
				syscall_exports = newle;
				ASSERT(syscall_exports->le_flags & LE_SYSCALLS);
				kernel_exports->le_usecount += 1;
				syscall_exports->le_usecount += 1;
				svc_table_entries = syscall_exports->le_filesize /
						sizeof(struct svc_table_entry );
				svc_table = (struct svc_table_entry *)
						syscall_exports->le_file;
			}
		}
			
	}/* end of specific unload case */

	ld_unload(sspointer);                            /* garbage collect */
	
exit:
	unlockl(&kernel_anchor.la_lock);
	unlockl(&kmod_lock);
	return u.u_error;

return_EINVAL:
	u.u_error = EINVAL;
	goto exit;
}   /* end kmod_unload */

void (*(
kmod_entrypt(
mid_t	kmid,
uint	flags)))()
{
	return kmid;

}  /* end kmod_entrypt */
