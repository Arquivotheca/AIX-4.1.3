static char sccsid[] = "@(#)04	1.68.1.4  src/bos/kernel/ldr/POWER/ld_memory.c, sysldr, bos41J, 9515A_all 4/11/95 11:04:28";
/*
 * COMPONENT_NAME: (SYSLDR) Program Management
 *
 * FUNCTIONS: ld_usecount(), ld_cleartext(), ld_ptrace(), ld_ptracefork(),
 *            ld_addresslib(), ld_libinit(), ld_cleanupi(), brk(),
 *            sbrk(), ld_unitheap(), ld_ufree(), ld_ualloc(), ld_dfree(),
 *            ld_readprotecttext(),  bssclear(), ld_mapdata1(),
 *	      ld_mapdata(), ld_addressppseg(), ld_restdataseg()
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
#include	<sys/errno.h>
#include	<sys/fp_io.h>
#include	<sys/intr.h>
#include	<sys/ldr.h>
#include	<sys/lockl.h>
#include	<sys/malloc.h>
#include	<sys/proc.h>
#include	<sys/pseg.h>
#include	<sys/seg.h>
#include	<sys/shm.h>
#include	<sys/systm.h>
#include	<sys/syspest.h>
#include	<sys/user.h>
#include	<sys/vmuser.h>
#include	<sys/xmalloc.h>
#include	<varargs.h>
#include	"ld_data.h"
#include	"ff_alloc.h"
#include	"ld_fflist.h"
#include	"vmm/vmsys.h"


void	ld_xatt(struct sharedstuff *sspointer),
	ld_xdet(struct sharedstuff *sspointer);

/* called by fork, ld_execld, and exit to deal with duplicating
 * or eliminating a process image
 */
void
ld_usecount(
int	dir)
{
	struct loader_anchor *a;
	struct loader_entry  *aloadlist,*le,*dle,*execle;
	struct loader_domain *ld;
	uint	i;
	vmhandle_t	srval;
	vmhandle_t	text_savesrval, data_savesrval;
	int	text_wasralloced, data_wasralloced;
	int	waslocked;         /*kernel lock recursion*/
	int	basesid, sharsrval, textsrval;
	struct file *textfp;
	int mapped;
	int gone_to_0 = 0;
	struct file *domfp = NULL;

	/*
	 * If these variables are filled in, delete the segments after 
	 * releasing the loader_anchor lock.
	 */
	basesid = sharsrval = textsrval = -1;
	textfp = NULL;
	mapped = 0;

	waslocked = lockl(&kernel_anchor.la_lock,LOCK_SHORT);   /* and this */

	ASSERT((dir==1) || (dir==-1));
	/* since we are called from outside the loader (fork, exit) we
	 * can't assume shared library addressability.  The correct shared
	 * library is the one associated with this process.
	 * First we ralloc the shared library area.  If it is already ralloced,
	 * we use it anyway be restore its old value on exit.
	 */
	if (text_wasralloced = vm_ralloc(SHTEXTORG))
		text_savesrval = vm_geth(SHTEXTORG);
	if (data_wasralloced = vm_ralloc(SHDATAORG))
		data_savesrval = vm_geth(SHDATAORG);

	/*
	* Set up loader to use global shared data and text segments.
	*/
	vm_seth(library_text_handle,SHTEXTORG);
	vm_seth(library_data_handle,SHDATAORG);


	/* a will be the process private loader anchor */
	a=(struct loader_anchor*)(U.U_loader);
	execle = a->la_execle;
	/* use aloadlist since at the bottom we walk the process
	 * load list one last time AFTER the anchor has been cleared */
	aloadlist = a->la_loadlist;

	/* Attach to overflow segment if necessary */
	if (a->la_flags & LA_OVFL_HEAP)
		ld_ovflatt(a);

	if (dir == -1){
	/* keep track of text segment if it exists
	 * text can either be copied into the text segment or can be an open
	 * file mapped at the textorg.  In the former case, we must free the
	 * segment when the last using process goes away.
	 * The segment register values are protected because we are single
	 * threaded here.
	 */

		if (a->la_flags&LA_PTRACE_TS)
			textsrval = as_getsrval(&U.U_adspace,TEXTORG);

		if (a->la_flags&LA_PTRACE_SL)
			sharsrval = as_getsrval(&U.U_adspace,SHTEXTORG);

		/* is this last use of this file via this file table entry?
		 * initial use comes from exec, other uses from fork.
		 * when all such uses have either exited or exec'd, we must
		 * free up segment.  To avoid deadlock, the last close must
		 * be done after the lock is released. If fp_opencount is > 1,
		 * fp_close() is safe; if fp_opencount==1, then
		 * we know this is the last use.  We then arrange for the
                 * code after the lock release to clean up
		 * N.B. the fp_close and check of fp_opencount must be
		 * in a critical section (i.e. holding the loader lock)
		 * so that exactly one process sees the value 1.
		 */

		if (execle) {
		  if (1 == fp_opencount(execle->le_fp)){
			textfp = execle->le_fp;
			if (a->la_flags & LA_TEXTALLOC)
				/* exec'd file copied into computational
				   segment this is last use */
				basesid = a->la_basesid;
			else
				/* exec'd file mapped and this is last use */
				mapped = 1;
		  }
		  else {
			/* opencount>1, so will only decrement open count */
			fp_close(execle->le_fp);
		  }
		}
	}

	for(le=aloadlist;le;le=le->le_next){
		if (le->le_flags & LE_DOMAIN_ENT) {
			assert(le->le_de);
			(le->le_de)->de_usecount += dir;
			if (!(le->le_de)->de_usecount)
				gone_to_0++;
		}

		if (dir==1 && le->le_fp)
			fp_hold(le->le_fp);

		for(i=0;i<le->le_ndepend;i++){
			dle=le->le_depend[i];
			/* only count up uses in le's which will be SHARED
			 * by both copies after the fork.  Remeber, fork
			 * is duplicating the private le's
			 */
			if (dle->le_flags & (LE_LIBRARY|LE_SYSCALLS))
				dle->le_usecount+=dir;
		}
	}

	/*
	 * adjust use counts in loader domains,  and clean up any that
	 * go to zero
	 */
	if (gone_to_0)
		ld_clean_domain(a->la_domain);
	if (ld = a->la_domain) {
		ld->ld_usecount += dir;
		if (ld->ld_usecount == 0) {
			/*
			 * save domain fp so it can be closed after
			 * releasing loader lock.
			 */
			domfp = ld->ld_fp;
			ld_unload_domains();
		}
	}

	/* in case this process gets killed again, leave things so we
	 * don't undo everything twice.  This could happen for example if
	 * exec got into real bad trouble between here and the end of
	 * allocating a new user data area
	 */
	if (dir==-1) {
		/* Mark process loader state as unsure so ld_ptrace() won't
		 * process.  Note that this must be done while holding the
		 * loader lock.
		 */
		a->la_flags = LA_UNSURE | (a->la_flags & (LA_OVFL_HEAP |
				LA_OVFLSR_ALLOCED | LA_OVFL_ATT));
		a->la_loadlist = NULL;
		a->la_execle = NULL;
		a->la_domain = NULL;
		as_seth(&U.U_adspace,NULLSEGVAL,TEXTORG);
		as_seth(&U.U_adspace,NULLSEGVAL,SHTEXTORG);
	}

		
	if (waslocked != LOCK_NEST) unlockl(&kernel_anchor.la_lock);
	(text_wasralloced) ? vm_seth(text_savesrval,SHTEXTORG)
			: vm_det(SHTEXTORG);
	(data_wasralloced) ? vm_seth(data_savesrval,SHDATAORG)
			: vm_det(SHDATAORG);

	/* when dir == -1, there may some deferred cleanup */
	if (dir==-1) {
		for(le=aloadlist;le;le=le->le_next){
			if (execle != le && le->le_fp)
				  fp_close(le->le_fp);
		}

		/* delete segments now, without lock */
		if (sharsrval != -1)
			vms_delete(SRTOSID(sharsrval));
		if (textsrval != -1)
			vms_delete(SRTOSID(textsrval));
		if (basesid != -1)
			vms_delete(SRTOSID(basesid));

		/* final close of the program text file */
		if (textfp) {
			/* if mapped, detach from it */
			if (mapped) fp_shmdt(textfp, SHM_RDONLY);
				fp_close(textfp);
		}

		/*
		 * delayed close of loader domain file
		 */
		if (domfp)
			fp_close(domfp);
	}

	/* If necessary,  detach from overflow segment */
	if (a->la_flags & LA_OVFL_ATT)
		ld_ovfldet(a);
}

int
ld_cleartext(
struct sharedstuff	*sspointer,
char   *textorg,
struct file	*fp)
{
	int	sid,rc;
	vmhandle_t	srval;
	if (
		fp_shmat(fp,NULL,SHM_RDONLY,&srval)){
        /* file can't be mapped so set up to copy it in */
        	u.u_error = 0;	/* fp_shmap error cleared */
		if (rc = vms_create(&sid,V_WORKING|V_UREAD,0,SEGSIZE,
		    SEGSIZE,SEGSIZE))
			return rc;
		ss.la->la_flags |= LA_TEXTALLOC;
		srval = SRVAL(sid,0,0);
	}
	else{
		/* shmat succeeded */
 		sid = SRTOSID(srval);
 		srval = vm_setkey(srval,VM_UNPRIV);
 	}
	vm_seth(srval,textorg);             /* kernel writable if TEXTALLOC */

	/* KLUDGE - check for mapped file with wrong size aouthdr */
	if ( ! (ss.la->la_flags & LA_TEXTALLOC) &&
		((struct filehdr *)textorg)->f_opthdr != sizeof(struct aouthdr) ){
	/* ok - this is an out of date module.  ld_textread may be able to cope
	 * with it - but not if its mapped - so undo everything and redo it
	 * UGH - this is a copy of the code above.  Its done this way to avoid
	 * lengthening the path of the normal case */
		fp_shmdt(fp,SHM_RDONLY);
		if (rc = vms_create(&sid,V_WORKING|V_UREAD,0,SEGSIZE,
		    SEGSIZE,SEGSIZE))
			return rc;
		ss.la->la_flags |= LA_TEXTALLOC;
		srval = SRVAL(sid,0,0);
		vm_seth(srval,textorg);	/* kernel writable if TEXTALLOC */
	} /* end of mapped obsolete file KLUDGE */

	ss.la->la_basesid = sid;
	as_seth(&U.U_adspace,vm_setkey(srval,VM_UNPRIV),textorg);
	return 0;
}

void
ld_readprotecttext(
char   *textorg)
{
	/* change kernel access to readonly */
	vm_seth(vm_setkey(vm_geth(textorg),VM_UNPRIV),textorg);
}

/* to convert a process to PTRACE mode, we create a new computational segment
 * and map the new one onto the old one.
 * We also create a new shared library text segment and map it onto the
 * existing one.
 * Note that this map is NOT an instantaneous copy.  Thus, we can't use the
 * ptrace copy to support subsequent load or unload system calls.  Rather,
 * we abandon the copy for each such call, make a new one, and expect the
 * debugger to re-install any breakpoints it needs.  Note that the debugger
 * will get a load signal each time this happens.
 *
 * N.B. if the vmmap of the shared libarary fails, we leave the ptrace copy of
 * text around anyway.  ld_usecount is programmed to correctly deal with this
 * case when is undoes the process later.
 *
 * The whole store is as follows:
 * ld_ptrace puts a process into ptrace mode by making private copies of
 *    text and library text
 *
 * it is called by ptrace
 *
 * when a process in ptrace exec's, ld_usecount(-1), called by exec,
 *    take the process out of ptrace mode, freeing the private copies.
 *
 * when a process calls load or unload, ld_addresslib gets rid of the private
 *    (writable) copys of the text and data.
 *
 * when a process forks, ld_ptracefork is called by fork with the pid of
 *    the child. The child is disconnected from the (parents) copies of
 *    text and library are reconnected to the base (readonly) copies.  If
 *    the mp ptrace bit is on, ld_ptrace is immediately called to make
 *    new ptrace copies for the child.
 *
 * N.B. in a nunmber of the above cases, break points "disappear" and must
 *    be restored by the debugger. In all these cases, the debugger gets
 *    a signal before the child process is allowed to continue.
 *
 * To make correctness more likely we determine if a PTRACE copy of a
 *    segment exists by the following:  For text, the flag LA_PTRACE_TS in
 *    the loader anchor is used.  For library, the flag LA_PTRACE_SL in
 *    the processes loader anchor is used.
 */
int
ld_ptrace(
pid_t	pid)
{
	struct loader_anchor *a;
	int	sid,rc,rc1,libsid;
	vmhandle_t	srval;
	void   *newtextorg;
	void	*textorg,*privorg;
	uint	filesize;
	struct	proc	*procp;
	struct	user	*user;
	struct	loader_entry	*le;
	ulong	privreloc;
        label_t jmpbuf;
	int	g_waslocked; /* global loader lock recursion */
	int	p_waslocked = LOCK_NEST; /* process loader lock recursion */
	int	ovfl_was_attached = 0;

	rc = 0; rc1 = 0;
	privorg = textorg = newtextorg = NULL;
	procp = (pid == -1) ? curproc : PROCPTR(pid);
	privorg = vm_att(procp->p_adspace,0);
	privreloc = (ulong)privorg - PRIVORG;
	user = PTRADD(&U,privreloc);
	a=PTRADD(U.U_loader,privreloc);

	/* Take per-process loader lock in a multi-threaded process */
	if (procp->p_active != 1)
		p_waslocked = lockl(&a->la_lock, LOCK_SHORT);

	/* Assure no one is changing anchor while we are accessing it */
	g_waslocked = lockl(&kernel_anchor.la_lock,LOCK_SHORT);
	if (a->la_flags & LA_UNSURE) {
		rc = EINVAL;
		goto exit;
	}

	/* Attach to overflow heap,  this may contain loader entries */
	assert(privorg != OVFLORG);	/* this would be trouble */
	ovfl_was_attached = ld_ovflatt(a);

	/* first make any privately loaded text read/write
	 * We do this every time, even if LA_PTRACE_* is on, because
	 * new programs may have been loaded.
	 * N.B. the relocation is subtle.  We relocate each le
	 * after we have tested for NULL!
	 */
	for(le = a->la_loadlist;le;le=le->le_next){
	  le = OVFL_PTRADD(le,privreloc);
	  if ( ((le->le_flags & (LE_TEXT | LE_TEXTRW)) == LE_TEXT) &&
	  (((int)le->le_file >> SEGSHIFT) == PRIVSEG) ) {
		vm_protect(PTRADD(le->le_file,privreloc), le->le_filesize,
			UDATAKEY);
		le->le_flags |= LE_TEXTRW;
	  }
	}
	/* end of making private text read/write */
	if (a->la_flags & LA_PTRACE_TS)	
		goto checklib;
	textorg = vm_att(SRVAL(a->la_basesid,0,0),0);
	le = PTRADD(a->la_execle,privreloc);
	filesize = le->le_filesize;
	/* create segment for copy of text segment that ptrace can write*/
	if (rc=vms_create(&sid,V_WORKING|V_UREAD,0,filesize,filesize,SEGSIZE))
		goto exit;
	srval = SRVAL(sid,0,0);
	newtextorg = vm_att(srval,0);
	/* map copy the base file or text segment*/
        if(( rc1 = setjmpx(&jmpbuf)) == 0)
        {
          rc = vm_map(textorg,newtextorg,filesize);
          clrjmpx(&jmpbuf);
	  if (rc) goto exit;
        }
        else
        {
          goto exit;
        }
        /* reset the user to use the copy - ptrace will use this too*/
        /* remember that we have recorded the sid of the base text segment in
         * the anchor - in la_basesid
         */
	as_seth(&(user->U_adspace),vm_setkey(srval,VM_UNPRIV),TEXTORG);
	a->la_flags |= LA_PTRACE_TS;
	vm_det(textorg);
	vm_det(newtextorg);
	textorg = newtextorg = NULL;

	/* create segment for copy of shared library segment */
checklib:
	/* first,  make sure a copy doesn't already exist */
	if (a->la_flags & LA_PTRACE_SL)
		goto exit;

	/* next,  make sure this process actually uses shared library seg */
	srval = as_getsrval(&(user->U_adspace),SHTEXTORG);
	if (srval == NULLSEGVAL)
		goto exit;

	/* create segment for copy of library segment that ptrace can write*/
	if (rc=vms_create(&sid,V_WORKING|V_UREAD,0,SEGSIZE,SEGSIZE,SEGSIZE))
		goto exit;
	/* Make it addressable */
	srval = SRVAL(sid,0,0);
	newtextorg = vm_att(srval,0);

	/* Make global shared library text segment addressable */
	textorg = vm_att(vm_setkey(library_text_handle,VM_PRIV),0);

	/* map copy the base file or text segment*/
        if((rc1 = setjmpx(&jmpbuf)) == 0)
        {
           rc = vm_map(textorg,newtextorg,SEGSIZE);
           clrjmpx(&jmpbuf);
           if (rc) goto exit;
        }
        else
        {
           goto exit;
        }

        /* reset the user to use the new copy - ptrace will use this too.
         */
	as_seth(&(user->U_adspace),vm_setkey(srval,VM_UNPRIV),SHTEXTORG);
	a->la_flags |= LA_PTRACE_SL;
exit:
	a = PTRADD(U.U_loader,privreloc);  /* a must be process anchor */
	if (ovfl_was_attached)
		ld_ovfldet(a);
	if (g_waslocked != LOCK_NEST) unlockl(&kernel_anchor.la_lock);
	if (p_waslocked != LOCK_NEST) unlockl(&a->la_lock);
	if (newtextorg) vm_det(newtextorg);
	if (privorg) vm_det(privorg);
	if (textorg) vm_det(textorg);
        return rc;
}

/* child has been created while in ptrace.  The ptrace copies of
 * text and library must not be shared.  So we delete them, then
 * if multiprocess debug, place child back into ptrace mode.
 */
int
ld_ptracefork(
pid_t	pid)
{
	struct loader_anchor *a;
	vmhandle_t	srval;
	void	*privorg;
	struct	proc	*procp;
	struct	user	*user;
	ulong	privreloc;

	/* if parent was never being debugged, then return immediately */
	if (! (((struct loader_anchor *)U.U_loader)->la_flags & LA_PTRACE_TS))
		return(0);

	procp = (pid == -1) ? curproc : PROCPTR(pid);
	privorg = vm_att(procp->p_adspace,0);
	privreloc = (ulong)privorg - PRIVORG;
	user = PTRADD(&U , privreloc);
	a=PTRADD(U.U_loader , privreloc);
	if (a->la_flags & LA_PTRACE_TS){
		srval = SRVAL(a->la_basesid,0,0);  /* original text segment */
		as_seth(&(user->U_adspace),vm_setkey(srval,VM_UNPRIV),TEXTORG);
		/* N.B. don't delete the ptrace copy - its still in use by
		 * parent! Do turn off PTRACE for the child
		 */
		a->la_flags &= ~LA_PTRACE_TS;
	}
		
	/* now check shared library */
	if (a->la_flags & LA_PTRACE_SL){
		as_seth(&(user->U_adspace),
		  vm_setkey(library_text_handle, VM_UNPRIV), SHTEXTORG);
		/* N.B. don't delete the ptrace copy - its still in use by
		 * parent!
		 */
		a->la_flags &= ~LA_PTRACE_SL;
	}

	vm_det(privorg);

	/* The STRC flag is only inherited, when the multi-process debug
	 * flag is set.  If child is being traced, make new copies.
	 */
        return (procp->p_flag & STRC) ? ld_ptrace(pid) : 0;
}


int
ld_addresslib(
struct sharedstuff	*sspointer)
{
	int	rc;

	/* kernel loads need only attach to overflow segment if necessary */
	if (ss.type == 'K') {
		ld_ovflatt(ss.la);
		return 0;
	}

	/* Set flag to indicate ld_addresslib was called */
	ss.flags |= SS_LD_ADDRESSLIB;

	/* save sr values is set */
	if (ss.m_data.libtext_ralloc_rc=vm_ralloc(SHTEXTORG))
	         ss.m_data.libtext_save_sr = vm_geth(SHTEXTORG) ;
	if (ss.m_data.libdata_ralloc_rc=vm_ralloc(SHDATAORG))
	         ss.m_data.libdata_save_sr = vm_geth(SHDATAORG) ;

	/* attach to secondary data segments, if any */
	ld_xatt(sspointer);

	/*
	* Set up loader to use global shared test and data segments.
	* This must be done in every case
	*/
	vm_seth(library_data_handle,SHDATAORG);
	vm_seth(library_text_handle,SHTEXTORG);

	/* attach to overflow segment if one exists.  this must be done
	 * after the global shared data segment is addressable for the
	 * case in which "ss.type == 'P " and "ss.la == library anchor"
	 */
	ld_ovflatt(ss.la);

	if (ss.type == 'E'){ /* execload */
		/* set up process to use the current shared library */
		/* don't check for error in user address space - just
		 * take segment
		 */
		as_ralloc(&U.U_adspace,SHTEXTORG);
		as_seth(&U.U_adspace,vm_setkey(library_text_handle,VM_UNPRIV),
			SHTEXTORG);
	}

	else { /* load, unload or loadbind */
		vmhandle_t srval;

		/* Determine if we are using a PTRACE copy of text segment */
		if (ss.la->la_flags & LA_PTRACE_TS){
			/* Must use original segment in kernel */
			srval = SRVAL(ss.la->la_basesid,VM_UNPRIV,0);
			vm_seth(srval,TEXTORG);
		}

	}
	return 0;
}

/* Initialize the shared library areas.
 */
int
ld_libinit()
{
	heapaddr_t 	text_heap, data_heap;
	struct heap	*heapptr;
	int		text_sid,  data_sid;

	/* We must insure that the la_ovfl_srval field(in the processes loader
	 * anchor) is in the pinned page of the u-block.  This is required
	 * because the field is referenced in the sys_timer() routine while
	 * running diabled at INTTIMER.  This check is made once during
	 * system initialization to catch anyone that may have changed the
	 * user structure.
	 */
	assert(((unsigned)(&__ublock) >> PGSHIFT )  == ((unsigned)
	  (&((struct loader_anchor *)U.U_loader)->la_ovfl_srval) >> PGSHIFT));

	if (vms_create(&text_sid,V_WORKING|V_UREAD|V_SHRLIB,0,SEGSIZE,SEGSIZE,
	SEGSIZE))
	{	u.u_error = ENOMEM;
		return (-1);
	}

	if (vms_create(&data_sid,V_WORKING|V_UREAD|V_SHRLIB,0,SEGSIZE,SEGSIZE,
	SEGSIZE))
	{	vms_delete(text_sid);
		u.u_error = ENOMEM;
		return (-1);
	}

	library_text_handle = SRVAL(text_sid,0,0);
	library_data_handle = SRVAL(data_sid,0,0);

	/* get addressability to new segments  */
	vm_seth(library_text_handle,SHTEXTORG);
	vm_seth(library_data_handle,SHDATAORG);

	/* initialize the segments as a giant heap.
	 * we actually remember the heap anchors in the library control block
	 * which is in the library segment.  The address of the library control
	 * block is the same for all shared libraries and is saved in the kernel
	 */

	/* Initialize the heap in the shared library data segment */
	data_heap = init_heap((char *)SHDATAORG,SEGSIZE,0);
	ASSERT(data_heap != (heapaddr_t)(-1));
	/* Set the vmrelflag in the heap structure so that pages are
	 * released when they are xmfree'ed
	 */
	heapptr = (struct heap *)data_heap;
	heapptr->vmrelflag = 1;

	
	/* Initialize the heap in the shared library text segment */
	text_heap = init_heap((char *)SHTEXTORG,SEGSIZE,0);
	ASSERT(text_heap != (heapaddr_t)(-1));
	heapptr = (struct heap *)text_heap;
	heapptr->vmrelflag = 1;

	/* Put the library anchor in the shared library data segment */
	library_anchor = xmalloc(sizeof(struct library_anchor),2,data_heap);
	ASSERT(library_anchor != NULL && library_anchor != (void*)-1);
	bzero(library_anchor,sizeof(struct library_anchor));
	lib.la.la_text_heap = text_heap;
	lib.la.la_data_heap = data_heap;
	lib.la.la_flags |= (LA_TEXT_HEAP | LA_DATA_HEAP);
	lib.la.la_lock = LOCK_AVAIL;
	/* initialize the field used to preallocate data areas to user private
	 * locations
	 */
	lib.la.sbreak = (char *)DATAORG;


	return 0;
}

void
ld_cleanup(
struct sharedstuff	*sspointer)
{
	/* detach form overflow segment if necessary.  must be done
	 * while global library data segment is addressable
	 */
	ld_ovfldet(ss.la);

	/* nothing for kernel load */
	if (ss.type == 'K')
		return;

	/* if ld_addresslib was not previously called,  then return */
	if (ss.flags & SS_LD_ADDRESSLIB)
		ss.flags &= ~SS_LD_ADDRESSLIB;
	else
		return;

	if (ss.m_data.libtext_ralloc_rc)
	         vm_seth(ss.m_data.libtext_save_sr,SHTEXTORG) ;
	else vm_det(SHTEXTORG);


	if (ss.m_data.libdata_ralloc_rc)
	         vm_seth(ss.m_data.libdata_save_sr,SHDATAORG) ;
	else vm_det(SHDATAORG);
	
	/* detach from secondary data segments, if any */
	ld_xdet(sspointer);
}


#define SUCCESS 0
#define ERROR -1
/* Alignment of brk value. */
#define BRK_ALIGN(x) ((((uint)x)+sizeof(ulong)-1)&((ulong)-sizeof(ulong)))


/*
 * NAME: brk
 *
 * FUNCTION:
 *	Sets the break value to the specified value.
 *
 * RETURN VALUE DESCRIPTION:
 *	0=The value was changed.
 *	-1=An error, errno is set by the sbreak call.
 *
 * All about sbreak, lastbreak, minbreak.  These are fields in the loader
 * anchor which keep track of the user data area memory.
 * minbreak is the address of the first free byte beyond the last currently
 *          loaded program.  It changes whenever load or unload happens.
 * sbreak is the address of the first completely free byte of the user area.
 *          sbreak is always greater than or equal to minbreak.
 * lastbreak is always between minbreak and sbreak. It represents the base
 *          of the memory the user has allocated (with brk calls).
 *          any storage below lastbreak is only in use if it is either
 *	    part of a loaded program or on the dinuse chain.  the dinuse
 *          chain records user storage which was "trapped" by a subsequent
 *	    load call which loaded a program past the in use user storage.
 */

int
brk(
void *endds)
{
	ulong a,mina;
	struct loader_anchor *la;
	struct dinuse	*dinuse;
	int waslocked = LOCK_NEST;
	int ovfl_was_attached;

	la=(struct loader_anchor*)(U.U_loader);

	/* get per-process loader lock in a multi-threaded process */
	if (curproc->p_active != 1)
		waslocked = lockl(&la->la_lock, LOCK_SHORT);

	a=BRK_ALIGN(endds);
	mina = MAX(a,BRK_ALIGN(la->minbreak));
	/* dinuse chain may exist if loads and unloads have been
	 * done. */
	if (a <= (ulong) la->lastbreak) {
		/* we are moving the break to the left far enough
		 * that elements on the dinuse chain may be affected*/
		ulong	newlast;
		newlast = BRK_ALIGN(la->minbreak);
		/* dinuse structures may be in the overflow segment */
		ovfl_was_attached = ld_ovflatt(la);
                for(dinuse=la->la_dinuse;dinuse;dinuse=dinuse->next){
                	if ((ulong)(dinuse->start) > a ) {
                	/* this dinuse is completely freed by the brk call*/
                		dinuse->start=NULL;
                		dinuse->length=0;
                	}
                	else{
				if((ulong)(dinuse->start+dinuse->length)>a){
				/* this dinuse is partially freed by the brk call*/
					dinuse->length = a-(ulong)(dinuse->start);
				}
				/*
				newlast=MAX(newlast,(ulong)dinuse->start+
                			dinuse->length);
				*/
				if ((ulong)dinuse->start+dinuse->length >
				   newlast)
					newlast = (ulong)dinuse->start
						+ dinuse->length;
                	}
                }
		if (ovfl_was_attached)	 /* if necessary,  detach overflow */
			ld_ovfldet(la);
                la->lastbreak=(void*)BRK_ALIGN(newlast);
                /* if newlast is to left of mina we can move
                 * the break point all the way to newlast since nothing
                 * between newlast and mina is in use
                 * (by the way, newlast <= mina here
                 *  and newlast >= la->minbreak here) */
               	mina = MIN(newlast,mina);
        }
	/* remember that sbreak is the address of the first unallowed
	 * byte - NOT the last useable byte.  That's why we subtract
	 * 1 in the check for break in the same page*/
	if (PAGEDOWN(la->sbreak-1)==PAGEDOWN(mina-1)||sbreak(mina)==0){
		la->sbreak = (void *)mina;
		if (waslocked != LOCK_NEST)
			unlockl(&la->la_lock);
		return(SUCCESS);
	}
	/* sbreak didn't like it. */
	if (waslocked != LOCK_NEST)
		unlockl(&la->la_lock);
	return(ERROR);
}

/*
 * NAME: sbrk
 *
 * FUNCTION:
 *	Returns the old break value, usually the pointer to
 *	the data area allocated, unless incr is negative.
 *	The parameter passed is an area size or increment.
 *
 * RETURN VALUE DESCRIPTION:
 *	a value other than -1=the old break value.
 *	-1=An error, errno is set by the sbreak call.
 */
void*
sbrk(
int incr)
{
	ulong oldend;
	struct loader_anchor *la;
	int waslocked = LOCK_NEST;

	la=(struct loader_anchor*)(U.U_loader);

	/* get per-process loader lock in a multi-threaded process */
	if (curproc->p_active != 1)
		waslocked = lockl(&la->la_lock, LOCK_SHORT);
	
	/* for first process */
	if (la->sbreak == 0) la->sbreak = U.U_dsize + (char *)BASE(DATASEG);

	/* Pick up the old value, then increment/decrement it. */
	oldend = (ulong)la->sbreak;
	if (brk((void *)(la->sbreak+incr))==0) {
		/* Return the original value. */
		if (waslocked != LOCK_NEST)
			unlockl(&la->la_lock);
		return((void *)oldend);
	}

	/* brk failed. */
	if (waslocked != LOCK_NEST)
		unlockl(&la->la_lock);
	return(ERROR);
}
			
/* the ld_u... services allocate priveleged storage in the appropriate place
 * based on sspointer.  In the kernel or library, xmalloc/xmfree are used.
 * In the user area, we try to allocate in a loader area near the u-block
 * the idea is that simple programs won't need the extra pages implied by
 * starting a full blown heap in the user segment.
 * This code allocates in that regeon using a moving pointer.  ld_free, if
 * written should test for storage in this area and just abandone it!
 * If the area fills up, we start a user region heap.  Once having paid the
 * price of initing that heap there is no point in doing any further
 * allocation from the loader area.  If the user region heap fills up,  we
 * create an overflow segment,  and start an overflow heap there.  Once we
 * pay the price to do this,  all allocations will out of the overflow heap.
 */
void
ld_uinitheap(
struct sharedstuff	*sspointer)
{
	uint	hstart,hlength;
	if (ss.la->la_flags &  LA_DATA_HEAP)
		return;
	hstart = PRIVORG+KHEAP;
	hlength = hstart+KHEAPSIZE;
	hstart = PAGEUP(hstart);
	hlength = hlength -hstart;
 	ss.la->la_data_heap = init_heap((void*)hstart,hlength,0);
 	ss.la->la_flags |= LA_DATA_HEAP;
}

/* IMPORTANT NOTE:  If the space to be free'ed is in the overflow heap,
 * the caller must insure addressability to the overflow segment.
 */
int
ld_ufree(
struct sharedstuff	*sspointer,
void * addr)
{
	if (ss.type == 'L' || ss.type == 'K' )
		return xmfree(addr,ss.la->la_data_heap);
	/* if the addr is not in the initial heap free it - else abandon
	 * the space
	 */
	if  ( ((uint)addr-(uint)ss.la)>= sizeof U.U_loader ){
		ASSERT( ((uint)ss.la->la_data_heap-(uint)ss.la) >
			sizeof U.U_loader );
		return xmfree(addr, IS_OVFLSEG(addr) ? pp_ovfl_heap :
			ss.la->la_data_heap);
	}
	return 0;
}

/* allocates privledged storage
 * IMPORTANT:  Please see the notes about calls to ld_ualloc and
 * addressability of overflow heap segments in this code.
 */
void *
ld_ualloc(
struct sharedstuff	*sspointer,
uint	size)
{
	caddr_t	p=NULL;
	int	vmid;

	/* If an overflow heap has been created, allocate there.
	 * NOTE: this code assumes that if an overflow heap has been created
	 * for this process,  then the segment which contains this overflow
	 * heap has already been attached to the current address space.
	 * This has been assured for all code that existed at the time this
	 * code was written.  New calls to ld_ualloc must follow this
	 * convention or modify this code
	 */
	if (ss.la->la_flags & LA_OVFL_HEAP)
		return(xmalloc(size,0,pp_ovfl_heap));

	/* If the initial heap has been created,  try to allocate there */
	if (ss.la->la_flags & LA_DATA_HEAP ){
		if (p = xmalloc(size,0,ss.la->la_data_heap))
			return p;
	}
	else {
	/* Space may still exist in the "u block",  try to allocate there */
		size = (size+3)&(-4); /* round up to word */
        	if ((uint)ss.la->la_data_heap-(uint)ss.la+size <=
		  sizeof U.U_loader ){
          		/* still in initial area and request fits */
			p=ss.la->la_data_heap;
			(ss.la->la_data_heap)=PTRADD(ss.la->la_data_heap,size);
			return p;
          	}

		/* Will not fit in "u block",  create the initial heap and
		 * try to allocate there.
		 */
	        ld_uinitheap(sspointer);
		if (p = xmalloc(size,0,ss.la->la_data_heap))
			return p;
	}

	/* We get here when the request will not fit in the initial heap,
	 * AND an overflow heap does not exits.  Create an overflow heap
	 * and allocate the space from there.
	 */
	if (VMS_CREATE(&vmid,V_WORKING|V_SYSTEM,0,0,SEGSIZE,SEGSIZE))
                return p;	/* failed to get a segment,  return null */

	/* Save value, set flag in loader anchor */
	ss.la->la_ovfl_srval = SRVAL(vmid, 0, 0);
	ss.la->la_flags |= LA_OVFL_HEAP;

	/* Attach to the segment so that the space returned can be used later.
	 * NOTE: this assumes that ld_cleanup(or some other routine that will
	 * call ld_ovfldet) will be called later in this path of execution.
	 * This has been assured for all code that existed at the time this
	 * code was added.  New calls to ld_ualloc must follow this convention.
	 */
        ld_ovflatt(ss.la);      /* attach to segment */

        /* Initialize the entire segment as a large heap.  Note that the
	 * global variable pp_ovfl_heap is the same for every process.
	 */
        pp_ovfl_heap = init_heap((char *)OVFLORG,(uint) SEGSIZE, 0);

	/* Now,  allocate space from the newly created overflow heap */
	return(xmalloc(size, 0, pp_ovfl_heap));
}

/* The ld_d... services allocate "non-priveledged" storage.  In the kernel
 * or library, there is no difference - xmalloc/xmfree are still used.
 * But in the user area, these services compete with sbrk for storage.
 */
void
ld_dfree(
struct sharedstuff	*sspointer,
void	*addr,
uint	size)
{
	int	rc;
	struct myxcoffhdr	*h;
	char	*ldraddr;
	extern	int vm_release();

	if (ss.type == 'K') {	
		h = (struct myxcoffhdr *) addr;

		/* Free loader section separately */
		ldraddr = (char *)addr + h->s[h->a.o_snloader - 1].s_scnptr;
		if (FF_ADDR(&pinned_fflist, ldraddr) ||
		    FF_ADDR(&unpinned_fflist, ldraddr))
			ff_free(ldraddr);
		else {
			rc = VM_RELEASE(ldraddr,
				h->s[h->a.o_snloader - 1].s_size);
			ASSERT(!rc);
			rc = xmfree((char *) ldraddr, kernel_heap);
			ASSERT(!rc);
		}
			
		if (FF_ADDR(&pinned_fflist, addr) ||
		    FF_ADDR(&unpinned_fflist, addr))
			ff_free(addr);
		else {
			rc = VM_RELEASE(addr, size);
			ASSERT(!rc);
			rc = xmfree(addr, kernel_heap);
			ASSERT(!rc);
		}
		return;
	}
	if (ss.type == 'L') {
		if ((((ulong)addr & 0xf0000000) >> SEGSHIFT) == SHTEXTSEG)
			rc = xmfree(addr,ss.la->la_text_heap);
		else if ((((ulong)addr & 0xf0000000) >> SEGSHIFT) == SHDATASEG)
			rc = xmfree(addr,ss.la->la_data_heap);
		else
			assert(0);
		ASSERT(!rc);
		return;
	}
	/* storage management in user data area is by keeping
	 * track of what's allocated - so we ignore d_free and
	 * simply release the pages
	 */
	rc = VM_RELEASE(addr, size);
	ASSERT (!rc);
	return;

	
}


BUGVDEF(ld_bddebug,1);

/* bit masks for the alloc field in an adspace_t */
static ulong ld_bits[] = {
	0x10000000, 0x18000000, 0x1c000000, 0x1e000000,
	0x1f000000, 0x1f800000, 0x1fc00000, 0x1fe00000 };

/* ld_crsegs - Create multiple segments for large data programs.  Called
 * from ld_assigndataexec.  Returns 0 on success or an errno on failure.
 */

ld_crsegs(
struct sharedstuff *sspointer,
uint o_maxdata,
uint *minbreakp)
{
	int nsegs, segno, rc, sid;
	uint minbreak;
	vmhandle_t srval;
	int vtype;

	BUGLPR(ld_bddebug,2, ("ld_crsegs: big data: %x\n",o_maxdata));

	/* compute new starting data address */
	minbreak = (BDATAORG | (*minbreakp&PAGESIZE-1));

	BUGLPR(ld_bddebug,2, ("ld_crsegs: new minbreak: %x\n",minbreak));

	nsegs = (o_maxdata + SEGSIZE-1) >> SEGSHIFT;

	BUGLPR(ld_bddebug,4,
		("ld_crsegs: creating %d additional segs\n",nsegs));

	if (nsegs > 8 || U.U_adspace.alloc & ld_bits[nsegs-1]
	||  u.u_save.as.alloc & ld_bits[nsegs-1])
		return ENOMEM;

	U.U_adspace.alloc |= ld_bits[nsegs-1];
	u.u_save.as.alloc |= ld_bits[nsegs-1];

        vtype = V_WORKING;
        if (curproc->p_flag & SPSEARLYALLOC)
                vtype |= V_PSEARLYALLOC;

	for( segno = BDATASEG; segno < nsegs+BDATASEG; segno++ )
	{
		/* create the segment */
		if ((rc = VMS_CREATE(&sid,vtype,0,0,0,0)))
		{
			BUGLPR(ld_bddebug,1,
				("ld_crsegs: vms_create failed (%d)\n", rc));
			
			freeuspace(NULL);
			return ENOMEM;
		}

		srval = vm_setkey(sid,VM_UNPRIV);

		BUGLPR(ld_bddebug,3, ("ld_crsegs: srval = 0x%x\n", srval));

		/* set up segstate */
		assert(U.U_segst[segno].segflag == 0);
		U.U_segst[segno].segflag = SEG_WORKING;
		U.U_segst[segno].num_segs = 1;
		U.U_segst[segno].ss_srval = srval;

		/* save srval in the user's address space */
		as_seth(&U.U_adspace,srval,segno<<SEGSHIFT);
		assert(U.U_adspace.srval[segno] == srval);	/* paranoia */

		/* load segment register for immediate use */
		srval = vm_setkey(sid,VM_PRIV);
		ldsr(segno,srval);

		if ((BDATASEGMAX - segno) == MIN_FREE_SEGS)
			ss.flags |= SS_RELEASE_SRS;
	}

	*minbreakp = minbreak;
	return 0;
}

/*
* ld_xatt - Attach to extra segments for programs with big data.
*
* Called from: ld_addresslib
*/

void
ld_xatt(struct sharedstuff *sspointer)
{
	int i;
	vmhandle_t srval;
	caddr_t ca;

	/* loop through segment numbers that might be used for big data */

	for (i = BDATASEG; i <= BDATASEGMAX; i++)
	{
		/* quit on first segment number that isn't working */
		if (!(U.U_segst[i].segflag & SEG_WORKING))
			break;

		/*
		* Compute new segment register value with privileged key
		* and attach to it.
		*/

		srval=vm_setkey(U.U_segst[i].ss_srval,VM_PRIV);
		ca = vm_att(srval,0);
		assert(ca == i<<SEGSHIFT);
	}

	/* note that i will be last bigdata seg + 1 */
	if ((BDATASEGMAX - i) < MIN_FREE_SEGS)
		ss.flags |= SS_RELEASE_SRS;
}

/*
* ld_xdet - Detach from extra segments for programs with big data.
*
* Called from: ld_cleanup
*/

void
ld_xdet(struct sharedstuff *sspointer)
{
	int i;

	/* loop through candidates */
	for (i = BDATASEG; i <= BDATASEGMAX; i++)
	{
		/* quit on first segment number that isn't working */
		if (!(U.U_segst[BDATASEG].segflag & SEG_WORKING))
			break;

		/* if it's allocated, then detach from it */
		if (u.u_save.as.alloc & ((unsigned)0x80000000>>i))
			vm_det(i<<SEGSHIFT);
	}

	ss.flags &= ~SS_RELEASE_SRS;
}

#ifdef notyet
static int
ld_mapin(int segno)
{
	uint srval;
	caddr_t ca;

	if (U.U_adspace.srval[segno] == NULLSEGVAL)
		return EINVAL;

	/* compute privileged srval */
	srval=vm_setkey(U.U_adspace.srval[segno],VM_PRIV);

	/* cram it into a segment register */
	ca = vm_att(srval,0);
	if (ca != (segno<<SEGSHIFT))
	{
		vm_det(ca);
		return EINVAL;
	}
	return 0;
}

/*
 * ld_addressu - Called by the loadat system call to gain addressability
 * to a shared memory segment.
 */
int
ld_addressu(char *uaddr, int mapflag)
{
	uint segno;

	if (mapflag)	/* map in */
	{
		/* validate the address */
		segno = (unsigned)uaddr >> SEGSHIFT;
		if (segno <= PRIVSEG || segno >= SHTEXTSEG)
			return EINVAL;

		if (!(U.U_segst[segno].segflag & SEG_SHARED))
			return EINVAL;

		return ld_mapin(segno);
	}
	else		/* map out */
		vm_det(uaddr);

	return 0;
}

/*
* ld_addressu2 - Get addressability to two user addresses
*
* ld_addressu2 is used by the loadbind system call to ensure addressability
* to the two modules so that they can be properly relocated.  Normally,
* the modules are loaded into the process private segment which is always
* addressable.  However, if the modules are loaded into shared memory
* segments, then they need to be mapped in.
*
* Return value: 0 indicates success, !0 indicates total failure.
*/

int
ld_addressu2(char *uaddr1, char *uaddr2, int mapflag)
{
	int segno1, segno2, rc;

	segno1 = (unsigned)uaddr1>>SEGSHIFT;
	segno2 = (unsigned)uaddr2>>SEGSHIFT;
	if (mapflag)
	{
		/* map in uaddr1 if necessary */
		if (segno1 != PRIVSEG)
			if (rc = ld_mapin(segno1))
				return rc;

		/* map in uaddr2, unless uaddr1 already did the job */
		if (segno2 != PRIVSEG && segno2 != segno1)
			if (rc = ld_mapin(segno2))
			{
				vm_det(uaddr1);
				return rc;
			}
	}
	else
	{
		if (segno1 != PRIVSEG)
			vm_det(uaddr1);

		if (segno2 != PRIVSEG && segno2 != segno1)
			vm_det(uaddr2);
	}
	return 0;
}
#endif


static void
bssclear(
struct sharedstuff	*sspointer,
char 	*start,
ulong	length)
{
	ulong	temp;
	extern int vm_release();

	if (!length)
		return;
	/* always clear remainder bss in first page */
	temp = MIN((char *)PAGEUP(start)-start,length);
	if (temp)
		bzero(start,temp);
	/* in execload case, all new storage is zero */
	if (ss.type == 'E')
		return;
	length = length-temp;
	start = start + temp;
	if (!length)
		return;
	temp = (char *)PAGEDOWN(start+length) - start;
	if (temp){
		VM_RELEASE(start,temp);
		start = start+temp;
		length = length-temp;
	}
	if (length)
		bzero(start,length);
	return;
}

static int
ld_mapdata1(
struct sharedstuff	*sspointer,
struct loader_entry	*le)
{
	int	rc,temp;
	uint i,j,k;
	caddr_t	p;
	struct loader_entry_extension 	*lexpointer;
	int vm_map();
	ulong	dataend;
	caddr_t bss_map_addr;
	int     bss_map_len;

	lexpointer = le->le_lex;
#ifdef	notyet
	if (ss.load_addr)
	{
		le->le_flags |= LE_LOADAT;
		goto skip_break;	/* XXX, to minimize diffs */
	}
#endif
	/* First determine if this is a pre-relocated instance of a
	 * library module.
	 */
	if (le->le_flags & LE_USEASIS){
		unsigned  datasize, bsssize;
		vmhandle_t data_srval, text_srval;
		struct myxcoffhdr *tmphdr = (struct myxcoffhdr *)le->le_file;
		/* the library copy has data and bss - so use datasize
		 * from o_dsize.  Don't forget relocation.
		 */	
		datasize = tmphdr->a.o_dsize;
		bsssize = tmphdr->a.o_bsize;

		/* Make the per-process shared library data segment
		 * addressable.  We use the SHTEXTSEG segment.
		 */
		data_srval = as_getsrval(&U.U_adspace, SHDATAORG);
		text_srval = vm_geth(SHTEXTORG);
		vm_seth(vm_setkey(data_srval,VM_PRIV), SHTEXTORG);

		if (VM_MAP(le->le_data, SEGOFFSET(le->le_data) + SHTEXTORG,
		  datasize)) {
			vm_seth(text_srval, SHTEXTORG);
			return ENOMEM;
		}

		bss_map_addr = SEGOFFSET(le->le_data) + SHTEXTORG + datasize;
		/* zero out the partial page after the initialized data
		 */
		if (datasize) {
			if (temp = MIN((char *) PAGEUP(bss_map_addr)
       		            - bss_map_addr, bsssize))
				bzero(bss_map_addr,temp);
		}
		/* special case where there is no data in library module.
		 * every page in per-process data segment must be part of
		 * a vm_map,  therefore vm_map beginning of bss.  this
		 * will zero page,  which is OK since there is no data.
		 */
		else if (bsssize && (PAGEUP(bss_map_addr) != bss_map_addr)) {
			if (VM_MAP(bss_map_addr, bss_map_addr, 1)) {
				vm_seth(text_srval, SHTEXTORG);
				return ENOMEM;
			}
		}

		/* the mapping length is to be full pages
		 * for bss
		 */ 
		bss_map_len = PAGEUP(bss_map_addr + bsssize)-
				PAGEUP(bss_map_addr);

		/* map the bss pages with the SAME virtual address for
		 * source and target addresses
		 */
		if (bss_map_len && VM_MAP(PAGEUP(bss_map_addr),
			PAGEUP(bss_map_addr), bss_map_len)) {
			vm_seth(text_srval, SHTEXTORG);
			return ENOMEM;
		}

		le->le_flags |= LE_DATAEXISTS|LE_DATAMAPPED;
		vm_seth(text_srval, SHTEXTORG);
		return 0;
	}

	/* make sure break is big enough and compute minbreak as we go */
	dataend = (ulong)le->le_data+le->le_datasize;
	/* all modules except the exec'd may be unloaded - which
	 * in general requires unmapping the data area.  Also,
	 * the data is copied by mapping.  Thus we page align the dataend*/
	if (!(le->le_flags & LE_EXECLE))
		dataend = PAGEUP(dataend);
	if ((ulong)ss.la->sbreak < dataend)
		if (BRK(dataend)) return ENOMEM;
	/*
	* Code temporarily removed until defect 22837 is fixed.
	*
	ss.la->minbreak = (char*)MAX((ulong)ss.la->minbreak,dataend);
	*/
	if ((ulong)ss.la->minbreak < dataend)
		ss.la->minbreak = (char *) dataend;

#ifdef	notyet
skip_break:	
#endif
	/* see if we can map copy the data */
	if ( (le->le_flags & LE_DATAMAPPED ) &&
	           (!VM_MAP(le->le_file +
		    hdr.s[lex.data].s_scnptr,le->le_data,hdr.a.o_dsize)) )
		   ;	
	else
		bcopy(le->le_file + hdr.s[lex.data].s_scnptr,	
		  le->le_data,hdr.a.o_dsize);	

	/* only need to zero rest of page - rest of bss is new pages */
	bssclear(sspointer,le->le_data + hdr.a.o_dsize,hdr.a.bsize);
	le->le_flags |= LE_DATAEXISTS;
	return 0;
}

/* ld_mapdata
 * 	maps all entries on load list down to end_of_new
 * If an entry is found that represents a pre-relocated instance of a
 * shared module(LE_USEASIS flag is set), we will create a per-process
 * shared library data segment.  The ld_mapdata1() routine will be
 * responsible for making this segment addressable.
 */
int
ld_mapdata(
struct	sharedstuff	*sspointer)
{
	struct loader_entry *le;
	int	rc;
        int	data_sid;
        vmhandle_t data_srval, save_h;
        caddr_t	copy_add;
        ulong	relocation;
        int	segment_exists=0;
	int     type = V_WORKING | V_SPARSE; 

	for(le=ss.la->la_loadlist;le != ss.end_of_new;le=le->le_next) {
	  if (LE_DATA == (le->le_flags &
	    (LE_DATA|LE_EXTENSION|LE_DATAEXISTS))) {

		/* Determine if we need to create a per-process
		 * shared library data segment.
		 */
		if (!segment_exists && (le->le_flags & LE_USEASIS)){
			/*
			 * See if segment is already allocated.
			 */
			if ( !( U.U_adspace.alloc &
			  ((unsigned)0x80000000>>SHDATASEG))) {
                                /* If this is an early allocation process,
                                 * set the proper segment attribute.
                                 */
				if (U.U_procp->p_flag & SPSEARLYALLOC)
					type |= V_PSEARLYALLOC;
				if (VMS_CREATE(&data_sid, type, 0,
					SEGSIZE, SEGSIZE, SEGSIZE)) {
					return ENOMEM;
				}
				/*
				 * Set the current process to use new segment
				 */
				assert(data_sid != 0);
				data_srval = vm_setkey(data_sid, VM_UNPRIV);
				as_ralloc(&U.U_adspace, SHDATAORG);
				as_seth(&U.U_adspace, data_srval, SHDATAORG);
			}
			segment_exists = 1;
		}
				
		/* Call ld_mapdata1() to do the actual mapping */
		if (rc = ld_mapdata1(sspointer, le)) {
			return rc;
		}
	  } /* if LE_DATA */
        } /* for */

        ss.la->lastbreak = ss.la->sbreak;
	return 0;
}

void
ld_addressppseg(
struct sharedstuff *sspointer)
/* Make the per-process shared library data segment addressable.  This is
 * done by putting the segment register value in the segment register
 * used for the global shared library data segment.  The global data segment
 * register value is saved in the shared stuff structure.
 */
{
	vmhandle_t data_srval;

	/* If no per-process segment,  nothing to do */
	if ( !( U.U_adspace.alloc & ((unsigned)0x80000000>>SHDATASEG)))
		return;

	/* get the segment register values */
	data_srval = as_getsrval(&U.U_adspace, SHDATAORG);
	ss.m_data.tmp_save_sr = vm_geth(SHDATAORG);

	/* Put per-process data in global data */
	vm_seth(vm_setkey(data_srval,VM_PRIV), SHDATAORG);
}

void
ld_restdataseg(
struct sharedstuff *sspointer)
/* Restore addressability to the global shared library data segment.  This
 * routine should only be called after a call to ld_addressppseg.
 */
{
	/* If no per-process segment,  nothing to do */
	if ( !( U.U_adspace.alloc & ((unsigned)0x80000000>>SHDATASEG)))
		return;

	/* Put per-process data in global data */
	vm_seth(ss.m_data.tmp_save_sr,SHDATAORG);
}

/*
 * NAME:	ld_filemap(sspointer, addr, size, *fp, pos)
 *
 * FUNCTION:	This routine is designed to simply map a file(from
 * 		its persistent segment) into the shared library text segment.
 *		The parameters specify the range of addresses in the
 *		mapping.  This routine assumes that the shared library
 *		text segment is addressable(in SHTEXTSEG) when called.
 *
 * PARAMETERS:	addr - Target address in the shared library text segment.
 *		size - Number of bytes to map.  The value passed MUST be a
 *		       multiple of PAGESIZE.
 *		fp   - File pointer to source file.
 *     		pos  - Position to start mapping within the source file.
 *
 * RETURN:	0  if success,
 *		!0 if failure
 *
 */
int
ld_filemap
(struct sharedstuff *sspointer,
caddr_t addr,
size_t size,
struct file *fp,
off_t pos)
{
	vmhandle_t	srval;
	int		sid, sidx, rc;
	void		*file_org;
	extern int	ps_not_defined;
	extern int	vm_writep();

	/* If the source pages cross a segment boundary then give up
	 * and let the file be read in.  
	 */
	 if (((unsigned)pos >> SEGSHIFT) != ((unsigned)(pos+size) >> SEGSHIFT))
		return(-1);

	/* Before page space is defined,  the loader will open files
	 * such that they can be written to.  Mapping a writable file
	 * would be a bad idea!  We check the O_RSHARE flag in the file
	 * structure to determine how the file was opened.
	 */
	if (!(fp->f_flag & O_RSHARE))
		return (-1);

	/* first thing to do is fp_shmat in order to get a sid */
	if (FP_SHMAT(fp,NULL,SHM_RDONLY,&srval))
		return (-1);

	/* We must make the segment addressable */
	sid = SRTOSID(srval);
	/* Take into account files > SEGSIZE */
	sidx = STOI(sid);
	sid = ITOS(sidx, (unsigned)pos >> L2PSIZE);
	file_org = vm_att(sid, 0);

	/* The value in pos is the offset from the beginning of the file.
	 * Change this to be the offset into a segment.
	 */
	pos = SEGOFFSET(pos);

	/* Write out all modified pages in the file before mapping.
	 * If a page is modified but not written,  VMM will copy the
	 * vm_mapp'ed page.  We want to avoid copying pages if possible.
         */
	rc = VM_WRITEP(sid, (unsigned)pos >> L2PSIZE, size >> L2PSIZE);

	/* Now vm_map from the persistent segment to the global shared
	 * library text segment.
	 */
	if (!rc)
		rc = VM_MAP((unsigned)file_org + pos, addr, size);

	/* we no longer need addressability to the persistent segment
	 * and we can fp_shmdt.  Note that the file will remain open
	 * until use count goes to zero.
	 */
	vm_det(file_org);
	FP_SHMDT(fp, SHM_RDONLY);
	return(rc);
}

/*
 * NAME:	ld_ovflatt(struct loader_anchor *la)
 *
 * FUNCTION:	Attach the segment that contains the overflow heap to
 *		the current address space.
 *
 * PARAMETERS:	la - loader anchor of a process
 *
 * RETURN:	!0 if attach was performed
 *		0 otherwise
 *
 */
int
ld_ovflatt(struct loader_anchor *la)
{
	/* Make sure this process has an overflow heap and it is not
	 * already attached.
	 */
	if (OVFL_EXISTS(la) && !(la->la_flags & LA_OVFL_ATT)) {
		/* Allocate segment register,  save if already allocated */
		if (vm_ralloc(OVFLORG)) {
			la->la_save_srval = vm_geth(OVFLORG);
			la->la_flags |= LA_OVFLSR_ALLOCED;
		}

		vm_seth(la->la_ovfl_srval, OVFLORG);
		la->la_flags |= LA_OVFL_ATT;

		return 1;
	}
	return 0;	/* did not do the attach */
}

/*
 * NAME:	ld_ovfldet(struct loader_anchor *la)
 *
 * FUNCTION:	Detach the segment that contains the overflow heap from
 *		the current address space.
 *
 * PARAMETERS:	la - loader anchor of a process
 *
 */
void
ld_ovfldet(struct loader_anchor *la)
{
	/* Make sure process has an overflow heap and it is attached */
	if (la->la_flags & LA_OVFL_ATT) {
		/* See if there is a saved segment reg value to restore */
		if (la->la_flags & LA_OVFLSR_ALLOCED) {
			vm_seth(la->la_save_srval, OVFLORG);
			la->la_save_srval = NULLSEGVAL;
		}
		else
			vm_det(OVFLORG);

		la->la_flags &= ~(LA_OVFL_ATT | LA_OVFLSR_ALLOCED);
	}
}


/*
 * NAME:	ld_srfreecall(fcn_ptr, va_list)
 *
 * FUNCTION:	This routine is designed to release a number of segment
 *		registers and then call the specified function.  There
 *		is a set of macros defined in 'ld_data.h' which determine
 *		if this routine should serve as a 'wrapper' for the
 *		specified function.
 *
 * PARAMETERS:	fcn_ptr	     - pointer to function that will be called.
 *		va_alist     - arguments to be passed to function.
 *
 * RETURN:	return code from specified function
 *
 */
int
ld_srfreecall(fcn_ptr, va_alist)	
int (*fcn_ptr)();
va_dcl
{
	va_list ap;
	int arg_count;
	unsigned inuse_mask = 0x00000000;
	unsigned arg_array[LD_MAX_ARGS];
	unsigned tmp_val;
	vmhandle_t save_srval[BDATASEGMAX-BDATASEG];
	int i, j, free;
	unsigned sr_alloc;
	int rc;
	extern	brk(), copyin(), copyinstr(), fp_close(), fp_fstat(),
		fp_open(), fp_read(), fp_shmat(), fp_shmdt(), vm_map(),
		vm_protect(), vm_release(), vm_writep(), vms_create(),
		vms_delete();
	
	/* determine number of arguments passed to function to be called */
	if (fcn_ptr == brk)			arg_count = 1;
	else if (fcn_ptr == copyin)		arg_count = 3;
	else if (fcn_ptr == copyinstr)		arg_count = 4;
	else if (fcn_ptr == fp_close)		arg_count = 1;
	else if (fcn_ptr == fp_fstat)		arg_count = 4;
	else if (fcn_ptr == fp_open)		arg_count = 6;
	else if (fcn_ptr == fp_read)		arg_count = 6;
	else if (fcn_ptr == fp_shmat)		arg_count = 4;
	else if (fcn_ptr == fp_shmdt)		arg_count = 2;
	else if (fcn_ptr == vm_map)		arg_count = 3;
	else if (fcn_ptr == vm_protect)		arg_count = 3;
	else if (fcn_ptr == vm_release)		arg_count = 2;
	else if (fcn_ptr == vm_writep)		arg_count = 3;
	else if (fcn_ptr == vms_create)		arg_count = 6;
	else if (fcn_ptr == vms_delete)		arg_count = 1;
	else	/* should never get here */
		ASSERT(0);
	/* extract the arguments */
	ASSERT(arg_count <= LD_MAX_ARGS);
	va_start(ap);
	for(i=0; i<arg_count; i++)
		arg_array[i] = va_arg(ap, unsigned);


	/* some functions may have their arguments in segments which
         * could potentially be released.  mark such segments here so
	 * that they will not be released.
	 */
	if (fcn_ptr == fp_read) {
		/* the second parameter contains the address of
		 * a buffer.  the third parameter is the number of
		 * bytes in the buffer.  check for crossing a
		 * segment boundary.
		 */
		tmp_val = arg_array[1];
		inuse_mask |= (unsigned)0x80000000 >> 
				(tmp_val>>SEGSHIFT);
		while ((tmp_val>>SEGSHIFT) !=
		      ((arg_array[1] + arg_array[2]) >> SEGSHIFT)) {
			tmp_val += SEGSIZE;
			inuse_mask |= (unsigned)0x80000000 >> 
					(tmp_val>>SEGSHIFT);
		}
	}
	else if (fcn_ptr == vm_map) {
		/* first and second parameters contain virtual addresses */
		inuse_mask |= (unsigned)0x80000000 >> 
				(arg_array[0]>>SEGSHIFT);
		inuse_mask |= (unsigned)0x80000000 >> 
				(arg_array[1]>>SEGSHIFT);
	}
	else if ((fcn_ptr == vm_protect) ||
		 (fcn_ptr == vm_release)) {
		/* first parameter is a virtual address */
		inuse_mask |= (unsigned)0x80000000 >> 
				(arg_array[0]>>SEGSHIFT);
	}
	inuse_mask &= BDATA_SEG_MASK;


	/* must free up some segment registers */
	free=0;
	sr_alloc = csa->as.alloc & BDATA_SEG_MASK;
	for(i=BDATASEGMAX-1; i >= BDATASEG;  i--){
		/* have we found all that we need ? */
		if (free > MIN_FREE_SEGS-1) {
			save_srval[i-BDATASEG]=-1;
		}
		/* need to find more segments that can be free'ed */
		else if (!(((unsigned)0x80000000>>i) & sr_alloc)) {
			/* if not allocated,  its free */
			save_srval[i-BDATASEG]=-1;
			free++;
		}
		/* check to see if allocated segment is used by call */
		else if (((unsigned)0x80000000>>i) & inuse_mask) {
			/* can't free segments that are in use */
			save_srval[i-BDATASEG]=-1;
		}
		else {	/* save srval and deallocate sr */
			save_srval[i-BDATASEG]=vm_geth(i<<SEGSHIFT);
			vm_det(i<<SEGSHIFT);
			free++;
		}
	}

	/* make the call */
	switch(arg_count) {
		case 1:	rc = (*fcn_ptr)(arg_array[0]);
			break;
		case 2:	rc = (*fcn_ptr)(arg_array[0],arg_array[1]);
			break;
		case 3:	rc = (*fcn_ptr)(arg_array[0],arg_array[1],
				arg_array[2]);
			break;
		case 4:	rc = (*fcn_ptr)(arg_array[0],arg_array[1],
				arg_array[2],arg_array[3]);
			break;
		case 5:	rc = (*fcn_ptr)(arg_array[0],arg_array[1],
				arg_array[2],arg_array[3], arg_array[4]);
		case 6:	rc = (*fcn_ptr)(arg_array[0],arg_array[1],
				arg_array[2],arg_array[3], arg_array[4],
				arg_array[5]);
			break;
	}

	/* restore segment registers */
	for(i=BDATASEG; i < BDATASEGMAX; i++){
		if (save_srval[i-BDATASEG] != -1) {
			vm_ralloc(i<<SEGSHIFT);
			vm_seth(save_srval[i-BDATASEG], i<<SEGSHIFT);
		}
	}

	va_end(ap);
	return rc;
}
