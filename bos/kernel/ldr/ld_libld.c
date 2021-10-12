static char sccsid[] = "@(#)29	1.63.1.3  src/bos/kernel/ldr/ld_libld.c, sysldr, bos41J, 9520A_all 5/16/95 16:46:23";
/*
 * COMPONENT_NAME: (SYSLDR) Program Management
 *
 * FUNCTIONS: ld_findfp(), ld_libgettext(),
 *            ld_provinstance(), ld_kernelld(), ld_prinld(), sortd(),
 *            extendd(), ld_allocd(), ld_getmap(), ld_assigndata(), ld_libld(),
 *            ld_checkdp(), ld_findle(), ld_getinstance(),
 *            ld_instance(), ld_getlib(), ld_libraries(), ld_undothis(),
 *            ld_getlibread(), ld_preread(), ld_findtextle(), ld_privgettext()
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
#include	<sys/fp_io.h>
#include	<sys/stat.h>
#include	<sys/ldr.h>
#include	<sys/malloc.h>
#include	<sys/pseg.h>
#include	<sys/seg.h>
#include	<sys/syspest.h>
#include	<sys/uio.h>
#include	<sys/vmuser.h>
#include	<sys/xcoff.h>
#include 	<sys/errno.h>
#include 	<sys/lockl.h>
#include	<sys/sleep.h>
#include	<sys/vfs.h>
#include	"ld_data.h"

/* predeclare - mutual recursion requires this */
struct loader_entry *
ld_findle(
struct sharedstuff *sspointer,
struct	file	*fp,
char	*member);

static struct loader_entry *
ld_instance(
struct sharedstuff	*sspointer,
struct loader_entry *lible);

void
prune_libpath(char *orig_libpath, char *new_libpath);

static void format_error(
char	*filename,
char	*member)
{
	char	fbuf[256];
	strncpy(fbuf,filename,128);
	if (member && *member){
		strncat(fbuf,"[",2);
		strncat(fbuf,member,(256-129));
		strncat(fbuf,"]",2);
	}
	ld_emess(L_ERROR_FORMAT,fbuf,NULL);
}

/* searches the load chain starting at le for an entry
 * which matches the file/member specification.
 * unloaded entries are ignored
 */
struct loader_entry *
ld_findfp(
struct loader_entry *le,
struct file	    *fp,
char	*member)
{
	uint	fh;
	fh=FPtoFH(fp);

        for(;le;le=le->le_next){
           if (le->le_flags & LE_UNLOAD) continue;
           if (FPtoFH(le->le_fp) != fh) continue;
           if (0==strcmp(le->le_filename,member)) return le;
	   /* special test for execed program whose filename is
	    * unfortunately represented differently.  It never has
	    * a member name */
	   if ( (le->le_flags & LE_EXECLE) && 
	       (!member || ! (*member))) return le;
           }
        return NULL;
}

static struct loader_entry *
ld_findtextle(
struct loader_entry *le,
struct file	    *fp,
char	*member)
{
	while(le=ld_findfp(le,fp,member)){
		/* want an entry which just represents the text
		 * (findfp skips unloaded entries).
		 */
		if((le->le_flags & (LE_DATA|LE_TEXT))==LE_TEXT) return le;
		le = le->le_next;
	}
	return NULL;
}

/* add a module to the shared library segment.  this module may then
 * be prerelocated, OR it may simply serve as the source for privinstance, OR
 * both.  modules are only put in the shared library if they are
 * read other.
 */

static struct loader_entry *
ld_libgettext(
struct sharedstuff *sspointer,
struct file	*fp,
char	*member)
{
	/*N.B. see ldr.h for use of lexpointer*/
	struct	loader_entry_extension	loader_entry_extension,*lexpointer;
        int	rc=0;		 			/*return code */
        int	i,j,k;
        struct	loader_entry *lible,*le,*prele;
        char	*filename;
        uint	size,membersize;
	uint	flag = 0;

	/* first see if this is already available */
	do {
		if ((le = ld_findtextle(lib.la.la_loadlist,fp,member))
		    && !(le->le_flags & LE_IOPENDING))
			return le;
		if (le)
			e_sleepl(&kernel_anchor.la_lock, &ld_loader_read,
				EVENT_SHORT);
	} while(le);

	ASSERT(ss.pre);


	lexpointer=&loader_entry_extension;
	bzero(lexpointer,sizeof *lexpointer);
	/* this loader entry represents the library - but not a resolved
	 * instance of the library - thus it has no dependencies
	 */
	filename = ld_fptopath(sspointer,fp);
	ASSERT(filename);
	if (!filename) filename = "";
	membersize = strlen(member);
	size = membersize+1+strlen(filename)+1+sizeof(struct loader_entry);
	lible = xmalloc(size,0,lib.la.la_data_heap);
	if (!lible){
		return NULL;
	}
	lible->le_usecount=1;	/* counted down after lock is reaquired */
	lible->le_loadcount = 0;
	lible->le_flags=LE_TEXT|LE_LIBRARY|LE_IOPENDING;
	lible->le_fp=fp;
	lible->le_exports=NULL;
	lible->le_lex = NULL;
	lible->le_defered = NULL;
	lible->le_filename = (char *)lible + sizeof(struct loader_entry);
	strcpy(lible->le_filename,member);
	strcpy(lible->le_filename+membersize+1,filename);
	lible->le_maxdepend=1;
	lible->le_ndepend=0;
	/* Note which thread is performing the I/O */
	lible->le_tid = curthread->t_tid;
	/* add it to the library load list*/
	lible->le_next = lib.la.la_loadlist;
	lib.la.la_loadlist = lible;
	unlockl(&kernel_anchor.la_lock);
	/* check for read/execute other permission -
	 * without that we can't put this into the library
	 */

	{
	         struct stat	stat;
	         if (rc = FP_FSTAT(fp,&stat,sizeof stat,FP_SYS))
			goto backout_le;
	         if ((stat.st_mode & S_IROTH) != S_IROTH)
		        goto backout_le;

		 /* Also determine of the file is in a local filesystem.
		  * If it is local,  ld_textread will attempt to map it.
		  */
		 if (!(stat.st_flag & FS_REMOTE))
			flag = LD_localfile;
	}
	
	
	/* textread will read the text in - it has to compute the
	 * length and then read the right amount
	 * a side effect is to set lex.h to the origin
	 * The second and third parameters are for execload, which
	 * always reads the program into a predetermined place
	 */
	
	u.u_error=ld_textread(sspointer,lexpointer,fp,member,NULL,0,
		lib.la.la_text_heap,flag);

	if (u.u_error)
		goto backout_le;

	/* remember that sanity initializes a number of values in lex*/
        if (u.u_error=ld_sanity(sspointer, lexpointer))
		goto backout_le;

	lockl(&kernel_anchor.la_lock,LOCK_SHORT);/*this form does not fail*/
	fp_hold(fp);
	ASSERT(lible->le_usecount == 1);
	lible->le_usecount = 0;
	lible->le_file =(void *)  lex.h;
	lible->le_filesize = lex.filesize;
	lible->le_data =  lible->le_file + hdr.s[lex.data].s_scnptr;
	lible->le_datasize = hdr.a.o_dsize;  /* N.B. don't include BSS */
	lible->le_flags &= ~LE_IOPENDING;
	if (hdr.f.f_flags & F_SHROBJ && strncmp(hdr.a.o_modtype,"RO",2) == 0
	&&  hdr.a.o_bsize == 0)
		lible->le_flags |= LE_SRO;
	if (strncmp(hdr.a.o_modtype,"PL",2) == 0)
		lible->le_flags |= LE_PRIVATELOAD;
	if (lex.flags & TEXTMAPPED)
		lible->le_flags |= LE_TEXTMAPPED;
	e_wakeup(&ld_loader_read);
	return lible;

backout_le:
	lockl(&kernel_anchor.la_lock,LOCK_SHORT);/*this form does not fail*/
	le = lib.la.la_loadlist;
	prele = NULL;
	while(le!= lible){
		prele = le;
		le = le->le_next;
	}
	if (prele)
		prele->le_next =le->le_next;
	else
		lib.la.la_loadlist = le->le_next;

	xmfree(le,lib.la.la_data_heap);
	e_wakeup(&ld_loader_read);
	return NULL;
}

static struct loader_entry *
ld_privgettext(
struct sharedstuff *sspointer,
struct file	*fp,
char	*member)
{
	struct loader_entry_extension loader_entry_extension,*lexpointer;
	struct loader_entry *le;
	int 	rc;
	char	*filename;
	uint	size,membersize;
	extern int vm_protect();

	
	if(le = ld_findtextle(ss.la->la_loadlist,fp,member))
		return le;

	lexpointer=&loader_entry_extension;
	bzero(lexpointer,sizeof *lexpointer);
	
	/* textread will read the text in - it has to compute the
	 * length and then read the right amount
	 * a side effect is to set lex.h to the origin
	 * an allocation map is passed in.  Note that we could improve this
	 * by keeping the allocation map up to date throughout.
	 */
	
	unlockl(&kernel_anchor.la_lock);

	rc = ld_textread(sspointer,lexpointer,fp,member,NULL,0,NULL,LD_allocd);

	lockl(&kernel_anchor.la_lock,LOCK_SHORT);/*this form does not fail*/

	/* remember that sanity initializes a number of values in lex*/
        if (!rc)
		rc = ld_sanity(sspointer, lexpointer);

	filename = ld_fptopath(sspointer,fp);
	ASSERT(filename);
	if (!filename) filename = "";

	if (rc) {	/* textread or sanity failed */
		if (rc == ENOEXEC)
			format_error(filename,member);
		u.u_error = rc;
		return NULL;
	}

	membersize = strlen(member);
	size = membersize+1+strlen(filename)+1+sizeof(struct loader_entry);
	le = ld_ualloc(sspointer,size);
	le->le_usecount=0;
	le->le_loadcount=0;
	le->le_flags=LE_TEXT;
	le->le_fp=fp;
	fp_hold(fp);
	le->le_file =(void *)  lex.h;
	le->le_filesize = lex.filesize;
	/* make base copy user read only - important part is text
	 * since the user actually executes here - but may as well	
	 *protect it all.  Is undone in unload*/
	VM_PROTECT(le->le_file,le->le_filesize,UTXTKEY);
	le->le_data =  le->le_file + hdr.s[lex.data].s_scnptr;
	le->le_datasize = hdr.a.o_dsize;  /* N.B. don't include BSS */
	le->le_exports=NULL;
	le->le_lex = NULL;
	le->le_defered = NULL;
	le->le_filename = (char *)le + sizeof(struct loader_entry);
	strcpy(le->le_filename,member);
	strcpy(le->le_filename+membersize+1,filename);
	le->le_maxdepend=1;
	le->le_ndepend=0;
	/* add it to the library load list*/
	le->le_next = ss.la->la_loadlist;
	ss.la->la_loadlist = le;
	return le;
}
/* privinstance - given a library loaded text, make a private instance.
 * this is needed if the prerelocated version can't be used for some reason, or
 * as the back end of privld
 */
static struct loader_entry *
ld_privinstance(
struct sharedstuff	*sspointer,
struct loader_entry	*lible)
{
	struct loader_entry	*le;
	struct loader_entry_extension	*lexpointer;
	uint size;
	/* we base a private instance on the unresolved version of the program.
	 * sometimes, privinstance is passed a prerelocated library module
	 * which for some reason can't be used - the following retrieves
	 * the base module
	 */
	while (! (lible->le_flags & LE_TEXT) )
		lible = lible->le_depend[0];
	lexpointer =
	     xmalloc(sizeof(struct loader_entry_extension),0,kernel_heap);
	assert(lexpointer);	/* if kernel heap is this close we're dead*/
	bzero(lexpointer,sizeof *lexpointer);
	lex.flags = XMALLOC;
	lex.h = (void *)lible->le_file;
	lex.filesize = lible->le_filesize;
	if (ld_sanity(sspointer, lexpointer)) panic("loader error");

        size = sizeof(struct loader_entry) +
	       sizeof(struct loader_entry*)*(lex.nimpid);
	le = (struct loader_entry *) ld_ualloc(sspointer, size );
	if (!le){
		u.u_error = ENOMEM;
		xmfree(lexpointer,kernel_heap);
		return NULL;
	}
	lex.le = le;

	le->le_lex = lexpointer;
	le->le_usecount = 0;
	le->le_loadcount = 0;
	le->le_flags = LE_DATA|LE_THISLOAD;
	le->le_fp = lible->le_fp;
	fp_hold(le->le_fp);
	le->le_file = lible->le_file;
	le->le_filesize = lible->le_filesize;
	le->le_data = NULL;   /* to be allocated later */
	le->le_datasize = hdr.a.o_dsize + hdr.a.o_bsize;
	le->le_exports = NULL;
	le->le_defered = NULL;
	le->le_filename = lible->le_filename;
        le->le_ndepend = 1;
        le->le_maxdepend = lex.nimpid+1;
        le->le_depend[0] = lible;
        lible->le_usecount += 1;
	le->le_next = ss.la->la_loadlist;    /* add to the process load list */
	ss.la->la_loadlist = le;
	/*
	 * now that this one is on the loadlist we can safely instantiate
	 * everything it depends on
	 */
	if (ld_libraries(sspointer,le,ld_create_create))
	        /* we don't clean up hear - we assume this is "curtains"
		 * for this load and higher level code will clean up
		 * everything or kill the process
		 */
		return NULL;
	return le;
}

/* kernel library load
 * called to load an instance into the kernel.  kernel programs are not reused
 * so we read in a copy which is executed as is.
 */
static struct loader_entry *
ld_kernelld(
struct sharedstuff  *sspointer,
struct	file		*fp,
char	*member)
{
	struct loader_entry_extension *lexpointer;
	struct loader_entry *le;
	int 	rc;
	char	*filename;
	uint	lesize,size,membersize;
	/* if called from library load refuse - this will cause library
	 * load to fail.  Ultimately will be called again to load the needed
	 * library under the original sharedstuff environment.
	 */
	if (ss.type != 'K') return NULL;


	lexpointer =
	       xmalloc(sizeof(struct loader_entry_extension),0,kernel_heap);
	assert(lexpointer);
	bzero(lexpointer,sizeof *lexpointer);
	lex.flags = XMALLOC;
	
	/* textread will read the text in and include room for bss
	 * thus this copy cannot be used as the asis for other instances.
	 * a side effect is to set lex.h to the origin
	 */

	rc = ld_textread(sspointer,lexpointer,fp,member,NULL,0,kernel_heap,LD_textreadbss);

	if (!rc)
		rc = ld_sanity(sspointer, lexpointer);

	filename = ld_fptopath(sspointer,fp);
	ASSERT(filename);
	if (!filename) filename = "";
	
	if (rc){ /*textread or sanity failed*/
		if (rc==ENOEXEC)
			format_error(filename,member);
		u.u_error = rc;
		xmfree(lexpointer,kernel_heap);
		return NULL;
	}
		
	membersize = strlen(member);
	lesize = sizeof(struct loader_entry) +
		 sizeof(struct loader_entry*)*(lex.nimpid-1);
	size = lesize+membersize+1+strlen(filename)+1;
	/*
	 * -1 because sizeof(struct loader_entry) contains space for one
	 * slot
	 */
	le = ld_ualloc(sspointer,size);
	assert(le);
	lex.le = le;
	le->le_usecount=0;
	le->le_loadcount=0;
	le->le_flags=LE_TEXT|LE_DATA|LE_DATAEXISTS|LE_THISLOAD|LE_DATAINTEXT;
	le->le_fp=fp;
	fp_hold(fp);
	le->le_file =(void *)  lex.h;
	le->le_filesize = lex.filesize;
	le->le_data =  le->le_file + hdr.s[lex.data].s_scnptr;
	le->le_datasize = hdr.a.o_dsize + hdr.a.o_bsize;
	le->le_exports=NULL;
	le->le_lex = lexpointer;
	le->le_defered = NULL;
	le->le_filename = (char *)le + lesize;
	strcpy(le->le_filename,member);
	strcpy(le->le_filename+membersize+1,filename);
	le->le_maxdepend=lex.nimpid;
	le->le_ndepend=0;
	le->le_next = ss.la->la_loadlist;    /* add to the kernel load list */
	ss.la->la_loadlist = le;
	/*
	 * now that this one is on the loadlist we can safely instantiate
	 * everything it depends on
	 */
#if defined(_KDB)
	if (__kdb())
		kdb_getdesc(le);
#endif /* _KDB */

	if (ld_libraries(sspointer,le,ld_create_create))
	        /* we don't clean up hear - we assume this is "curtains" i
		 * for this load and higher level code will clean up
		 * everything
		 */
		return NULL;
	return le;
}

/* private library load
 * called when we can't even put the code into the shared library,
 * because of permissions.
 * instead, we read a copy of the code into the data segment.  we then use this
 * as if it where shared, which it may become if there are multiple loads
 * of a one-use program.
 */
static struct loader_entry *
ld_privld(
struct sharedstuff  *sspointer,
struct	file		*fp,
char	*member)
{
	struct loader_entry *le;

	if (ss.type == 'K')
		return ld_kernelld(sspointer,fp,member);

	/* if called from library load refuse - this will cause library
	 * load to fail.  Ultimately will be called again to load the needed
	 * library under the original sharedstuff environment.
	 */
	if (ss.type == 'L') return NULL;


	/* if is possible to make a private instance of a sharable text file.
	 * we try that case first */
	le = ld_findtextle(lib.la.la_loadlist,fp,member);

	/* From the prepass we know a copy of the text already exists.
	 * Therefore an entry marked LE_IOPENDING is not the match we
	 * want so look to the private load list.
	 */
	if (!le || (le->le_flags & LE_IOPENDING))
	    le = ld_findtextle(ss.la->la_loadlist,fp,member);

	return le ? ld_privinstance(sspointer,le) : le;
}

/* assign data addresses for prerelocated programs.  The strategy
 * is to assign addresses which will at least work for the program
 * being loaded now.  To do this, we must figure out which data storage
 * is already spoken for.
 * We use a first fit algorithm UNLESS we can find an exact fit between two
 * other in use address ranges.
 */

static void
sortd(
struct d	*d,
uint	n)
{
	uint	i,j,k;
#define swapd(i,j)	{struct d tempd;tempd=d[i];d[i]=d[j];d[j]=tempd;}
	
	d = &(d[-1]);	/* convert to 1 based indexing */

	for(i=2;i<=n;i++){
		j = i;
		do {
			if (d[j/2].org < d[j].org) swapd(j/2,j)
			else break;
			j=j/2;
		} while(j > 1);
	}

	for(i=n;i>1;i--){
		swapd(1,i);
		j = 1;
		while(j*2 < i){
			if (j*2+1<i && d[j*2+1].org > d[j].org
				    && d[j*2+1].org > d[j*2].org){
				swapd(j,j*2+1);
				j = j*2+1;
			}
			else if (d[j*2].org > d[j].org){
				swapd(j,j*2);
				j = j*2;
			}
			else break;
		}
	}

	/* sorted by org - now make monotone non overlapping
	 * remember that array is 1 based here */
	
	for(i = 1;i<n;i++){
		if (d[i].end > d[i+1].end) d[i+1].end = d[i].end;
		if (d[i].end > d[i+1].org) d[i+1].org = d[i].end;
	}
}

#define	DSIZE	256	/* DSIZE must be a power of two */
static int
extendd(
struct d	**d,
uint		*maxd)
{
	struct d	*newd;
	uint 	newmaxd;
	/* N.B. checkd ASSUMES that d is a multiple of DSIZE elements long!*/
	newmaxd = *maxd+DSIZE;
	newd = xmalloc(sizeof(struct d)*newmaxd,0,kernel_heap);
	if (newd){
		bcopy(*d,newd,sizeof(struct d)*(*maxd));
		xmfree(*d,kernel_heap);
		*d = newd;
		*maxd = newmaxd;
		return 0;
	}
	/* no more room */
	xmfree(*d,kernel_heap);
	return ENOMEM;
}

static struct d *
ld_getmap(
struct sharedstuff	*sspointer)	/* this represents library */
{
	uint	i,j,k,l,m,maxd,nextd;
	struct loader_entry	*le;
	struct loader_entry_extension	*lexpointer;
	struct d	*d;
	struct	dinuse	*dinuse,*pdinuse,*ndinuse;

	if (ss.d) xmfree(ss.d,kernel_heap);

	/* first find all allocated locations used by this load.  We take
	 * the user load list into account as well to be robust in the
	 * face of future prerelocation at load as well as exec time */
	
	d = xmalloc(sizeof(struct d) * DSIZE,0,kernel_heap);
	if (!d) return NULL;

	maxd = DSIZE;

	nextd = 2;

	if (ss.type == 'L'){
	for(le=ss.la->la_loadlist;le;le=le->le_next){
		struct myxcoffhdr *myh;
		if ( (le->le_flags&(LE_DATAEXISTS|LE_DATA|LE_THISLOAD)) !=
		     (LE_DATA|LE_THISLOAD))
			continue; 	/* not a prerelocated instance or
					   data in library*/
		if ( ! le->le_exports )
			continue;       /* this is one we need to assign
					   data address to */
		if ( nextd >= maxd )
			if(extendd(&d,&maxd)) return NULL;
		myh = (void*)le->le_file;
		d[nextd].org = PAGEDOWN(le->le_exports->data);
		d[nextd].end =
		 PAGEUP(le->le_exports->data + myh->a.o_dsize + myh->a.o_bsize);
		nextd++;
	}
	sspointer=ss.topstuff;
	ASSERT(sspointer);
	}


	/* initialize array with dummy entries for beginning and end
	 * N.B. this is done here since sspointer is modified by the code above.
	 *     maxbreak is only valid in the base shared_stuff.  By now, sspointer
	 *     always points to the base
	 */
	d[0].org = 0;
	d[0].end =PRIVORG;
	d[1].org = (ulong)ss.maxbreak;
	d[1].end = -1;	/*fence post*/

	pdinuse = NULL;
	for(dinuse=ss.la->la_dinuse;dinuse;){
		if (dinuse->length == 0){
			ndinuse=dinuse->next;
			ld_ufree(sspointer,dinuse);
			if (pdinuse)
				pdinuse->next = ndinuse;
			else
				ss.la->la_dinuse = ndinuse;
			dinuse = ndinuse;
			continue;
		}

		if ( nextd >= maxd )
			if(extendd(&d,&maxd)) return NULL;
		d[nextd].org = (uint)(dinuse->start);
		d[nextd].end = (uint)(dinuse->start)+dinuse->length;
		nextd++;
		dinuse = dinuse->next;
	}

	for(le = ss.la->la_loadlist;le;le=le->le_next){
		/* if there is text and it is in the private region
		 * include it
		 */
		if( (le->le_flags & (LE_TEXT|LE_LIBRARY)) == LE_TEXT){
			if ( nextd >= maxd )
				if(extendd(&d,&maxd)) return NULL;
			d[nextd].org = PAGEDOWN(le->le_file);
			d[nextd].end = PAGEUP(le->le_file + le->le_filesize);
			nextd++;
		}
		/* if there is a data address consider it allocated.
		 * some of these addresses may be outside the user
		 * data area but including them does no harm */
		if (le->le_data){
			if ( nextd >= maxd )
				if(extendd(&d,&maxd)) return NULL;
			d[nextd].org = PAGEDOWN(le->le_data);
			d[nextd].end = PAGEUP(le->le_data + le->le_datasize);
			nextd++;
		}
	}

	sortd(d,nextd);
	return d;

}


/* allocates in units of pages from the user data area.  Uses
 * the results of a previous getmap.*/
ulong
ld_allocd(
struct sharedstuff	*sspointer,
ulong		datasize)
{
	uint	i,j,k,fit;
	ulong	data;
	struct	d	*d;

	if (!ss.d){
		ss.d = ld_getmap(sspointer);
		if (!ss.d)
			return 0;
	}
	d = ss.d;
	datasize=PAGEUP(datasize);
	/* scan for an exact fit, remembering the first fit as we go
         * (given the way sbreak works, first fit seems the best choice)*/
	fit = -1;
	for(i=0;d[i].end != -1;i++){
		long	fitsize;	/*N.B. SIGNED for test below */
		fitsize=PAGEDOWN(d[i+1].org) - PAGEUP(d[i].end);
		if (fitsize <= 0)
			continue;
		if (fitsize==datasize){
			fit = i;
			break;
		}
		if (fit == -1 && fitsize>datasize)
			fit = i;
	}
	
	if (fit == -1) return 0;

	data = PAGEUP(d[fit].end);
	d[fit].end = data+datasize;
	return data;
}


/* load an instance of a shared library program.
 * if necessary, the program is actually added to the shared library.
 * this involves loading the text, computing a target data location
 * and prerelocating the data.
 * This process may lead to other library loads etc ...
 * N.B. library load only uses other LIBRARY instances for resolution.
 * If a library program uses a non-sharable program, then the library
 * program is not prerelocated and only private instances are possible.
 * The loader entry stucture is as follows -
 * The entry for the base file, on the libarary load list, is:
 *	LE_TEXT and LE_LIBRARY but not LE_DATA.
 *	le_data and le_datasize represent the data section of the file
 *	there are no dependencies.
 * The entry for a prerelocated instance, on the library load list, is:
 *	LE_DATA and LE_LIBRARY but not LE_TEXT
 *	le_depend[0] is le of base file:
 *	le_data and le_datasize represent the location in the shared
 *              library region of the data instance.
 *              le_datasize does NOT include bss.
 *	le_exports->data is the target location of the data in the private
 *               region
 *	the prerelocated instances of other files used by this one are
 *               listed in ld_depend
 * The entry for an in use instance, on the process load list, is:
 *	LE_DATA and LE_USEASIS but not LE_TEXT and not LE_LIBRARY
 *	le_depend[0] is le of library instance
 *	le_data and le_datasize represent the location and size of data in
 *              private region including bss.
 *	the private instances of files used by this are listed in ld_depend
 */


static struct loader_entry *
ld_libld(
struct sharedstuff	*sspointer,
struct	file		*fp,
char	*member)
{
	/*N.B. see ldr.h for use of lexpointer*/
	struct	loader_entry_extension	*lexpointer;
        int	rc=0;		 			/*return code */
        int	i,j,k;
        struct	loader_entry *lible,*le;
        struct	sharedstuff	sharedstuff;
        char   *data;
        uint	datasize;
	int	recursion;
	label_t	jbuf;
	int	waslongjmpx;
	extern int vm_map();


	/* never use shared library code in kernel - just fail -
	 * privld does the work.  It is possible for the user to
	 * mark the executable PRIVATELOAD,  again privld will do it
	 */
	if (ps_not_defined || ss.type == 'K' || (ss.flags & SS_PRIVATELOAD))
		return NULL;

	/* now we try to make an instance of this library for sharing
	 */
	if (ss.type == 'L')
		recursion = 1;
	else {
		recursion = 0;
		bzero(&sharedstuff,sizeof sharedstuff);
		sharedstuff.topstuff = sspointer;
		sspointer = &sharedstuff;
		ss.type = 'L';
		ss.la = &(lib.la);
		ss.end_of_new = lib.la.la_loadlist;
		/* copy some stuff */
		ss.pl = ss.topstuff->pl; /*must be restored on exit*/
		ss.libpath = ss.topstuff->libpath;
		ss.libpath_env = ss.topstuff->libpath_env;
		ss.libpath_saved = ss.topstuff->libpath_saved;
		ss.d = ss.topstuff->d;
		ss.execle = ss.topstuff->execle;
		ss.ld = ss.topstuff->ld;	/* loader domain in use */
		ss.ldr = ss.topstuff->ldr;	/* loader domain reopen */
		ss.flags |= (ss.topstuff->flags & SS_RELEASE_SRS);
#if 0		/* if we ever map files into the library we may need this */
		waslongjmpx = rc = setjmpx(&jbuf); /* catch "i/o"
						      errors in mapped files */
		if (rc)	
			goto exitrc;
#endif
	}

		
	/* see if the library is instantiated in shared library segment*/
	if (le=ld_findle(sspointer,fp,member))
		goto exitle;
		

	/* Look for a copy of the text in the shared library segment */
	lible = ld_findtextle(lib.la.la_loadlist,fp,member);

	/* Note that we have already insured that the text is availiable
	 * during the prepass.  If the LE_IOPENDING flag is set,  then
	 * the entry found is not the one we want.  Return NULL so that
	 * we can search for the text on the process private load list.
	 */
	if (!lible || (lible->le_flags & LE_IOPENDING)) {
		if (u.u_error == ENOEXEC)
			format_error(ld_fptopath(sspointer,fp),member);
		goto returnNULL;
	}

	/* now see if this library is needed by execload or the load command.
	 * only if execload do we attempt to pre-relocate it.  In the case
	 * where the process has specified a laoder domain,  we allow the
	 * an explicit load operation to create a pre-relocation. It is also
	 * possible that the user could have marked the library PRIVATELOAD
	 * for compatability reasons.  If PRIVATELOAD,  we don't pre-relocate.
	 */
	if ((ss.topstuff->type != 'E' && !(lible->le_flags & LE_SRO) &&
	    !(ss.topstuff->la->la_domain)) ||
	    (lible->le_flags & LE_PRIVATELOAD)) {
		if (recursion)
			return NULL;	/* refuse to prerelocate*/
		/* restore topstuff entries which may have changed */
		ss.topstuff->pl = ss.pl;
		ss.topstuff->d = ss.d;
		ss.topstuff->ldr = ss.ldr;
		/*
		 * ss.libpath may have been allocated
		 */
		if (ss.flags & SS_LIBPATH_ALLOCED) {
			assert(!(ss.topstuff->flags & SS_LIBPATH_ALLOCED));
			ss.topstuff->libpath = ss.libpath;
			ss.topstuff->flags |= SS_LIBPATH_ALLOCED;
		}
		return ld_privinstance(ss.topstuff,lible);
	}

	lexpointer  =
	       xmalloc(sizeof(struct loader_entry_extension),0,kernel_heap);
	assert(lexpointer);
	bzero(lexpointer,sizeof *lexpointer);
	lex.flags = XMALLOC;
	lex.h = (void *)lible->le_file;
	lex.filesize = lible->le_filesize;
        if (ld_sanity(sspointer, lexpointer)) panic("shared library damaged");
	
	
        /* we do not prerelocate a program in the library unless its
	 * data alignment allows mapping - to map the data alignment
	 * in the file must satisfy the alignment requirements of the data.
         */
        {/*block allows local declare*/
                ulong 	align;		

                align = (1<<hdr.a.o_algndata)-1;
                if ((uint)(lible->le_data-hdr.s[lex.data].s_vaddr) & align) {
                          xmfree(lexpointer,kernel_heap);
                          goto returnNULL;
                 }
        }

	/* Allocate space for the BSS also */
	datasize = PAGEUP(lible->le_data + hdr.a.o_dsize + hdr.a.o_bsize) -
		   PAGEDOWN(lible->le_data);

	/* Since data is the shared library data segment is not shared(a
	 * copy must be created for each process using it),  we must allocate
	 * space for read only data in the shared library text segment.
	 */
	if (lible->le_flags & LE_SRO)
		data = xmalloc(datasize,PGSHIFT,lib.la.la_text_heap);
	else
		data = xmalloc(datasize,PGSHIFT,lib.la.la_data_heap);

	if (!data) {
#ifdef DEBUG
		/* panic("shared library is full"); */
#endif
		/* if no room, we give up on prerelocation - thus
		 * we will load and relocate in private.  Set a flag
		 * indicating library segment is full.  We will try
		 * to cleanup before next exec.
		 */
		shlibseg_full++;
                xmfree(lexpointer,kernel_heap);
                goto returnNULL;
	}

	/* we must get le_data and le_datasize "right" - this means data
	 * is no longer the value returned be xmalloc.
	 * In unload, we will use PAGEDOWN to get back to that value for xmfree
	 */
	data = data + (uint)lible->le_data - PAGEDOWN(lible->le_data);

        {
	         uint	size;

                 size = sizeof(struct loader_entry) +
			sizeof(struct loader_entry*)*(lex.nimpid);
	         le =  xmalloc(size,0,lib.la.la_data_heap);
		if (!le) {
#ifdef DEBUG
			/* panic("shared library is full"); */
#endif
			/* if no room, we give up on prerelocation - thus
			 * we will load and relocate in private.  Set a
			 * flag to indicate shared library seg is full.
			 * We will try to cleanup before next exec.
			 */
			shlibseg_full++;
	                xmfree(lexpointer,kernel_heap);
			if (lible->le_flags & LE_SRO)
	                	xmfree(PAGEDOWN(data),lib.la.la_text_heap);
			else
	                	xmfree(PAGEDOWN(data),lib.la.la_data_heap);
	                goto returnNULL;
		}

                 bzero(le,size);
	} /* end of use of size */
	lex.le = le;
	le->le_usecount = 0;
	le->le_loadcount = 0;
	le->le_flags = LE_LIBRARY|LE_DATA|LE_THISLOAD;
	if (lible->le_flags & LE_SRO)
		le->le_flags |= LE_SRO;
	le->le_fp = fp;
	fp_hold(fp);
	le->le_file = (void *)lex.h;
	le->le_filesize = lex.filesize;
	/* Map copy the pristine copy to the new instance */
 	if(VM_MAP(lible->le_data,data,lible->le_datasize))
		bcopy(lible->le_data,data,lible->le_datasize);
	le->le_data = data;
	/* This instance represents both the data and bss */
	le->le_datasize = hdr.a.o_dsize + hdr.a.o_bsize;
	le->le_exports=NULL;
	le->le_lex = lexpointer;
	le->le_defered = NULL;
	le->le_filename = lible->le_filename;
	le->le_maxdepend=1+lex.nimpid;
	le->le_ndepend=1;
	le->le_depend[0] = lible;
	lible -> le_usecount += 1;

	/* Set the data and bss relocation fields for this instance of the
	 * data.  These will be used when we resolve/relocate the data.
	 */
	lex.datareloc = le->le_data - hdr.s[lex.data].s_vaddr;
	lex.bssreloc =  le->le_data + hdr.a.o_dsize - hdr.s[lex.bss].s_vaddr;

	/* add this entry to the library load list NOW.  Note that this le will
	 * satisfy library request for this program EVEN though we are not
	 * finished resolving it.  See the test in checkdep -
	 * since LE_THISLOAD is set, this entry is always accepted
	 */

	le->le_next = ss.la->la_loadlist;
	ss.la->la_loadlist = le;

	/* resolve libraries for this le - create instances if necessary */
        if (rc = ld_libraries(sspointer,le,ld_create_create)){
        	/* we get here whenever one of the dependencies is
        	 * not shareable.  Also, if a real error happened.
        	 * In that case, uerror will be set */
        	le=NULL;
        	goto exitle;
        }

	/* if recursive call stop here - resolve can't be done
	 * till all libraries are found
	 */
	if (recursion) return le;
	
	if (rc=ld_resolve(sspointer)) goto exitrc;

	if (rc=ld_relocate(sspointer)) goto exitrc;
	
exitle:
	/* if inside a recursion let the top level clean up the mess */
	if (recursion) return le;
#if 0	/* see comment above on setjmpx */
	if (! waslongjmpx)
		clrjmpx(&jbuf);
#endif
	/* at this point, there are temporary data structures hanging from
	 * loader_entries in the library.  We leave them be - they will be
	 * cleaned up in one final pass over the load list at the very end.
	 * We do this so that the THISLOAD bits already set are preserved.
	 */
	/* restore pathname lookaside chain - it may have changed */
	ss.topstuff->ldr = ss.ldr;
	ss.topstuff->pl = ss.pl;
	if (ss.flags & SS_LIBPATH_ALLOCED) {
		assert(!(ss.topstuff->flags & SS_LIBPATH_ALLOCED));
		ss.topstuff->libpath = ss.libpath;
		ss.topstuff->flags |= SS_LIBPATH_ALLOCED;
	}
	if (!le) { /*error cleanup*/
		struct loader_entry *tle,*nle;
		ld_freelex(ss.la->la_loadlist,ss.end_of_new);
		/* discard all the new loader entries */
		for(tle=lib.la.la_loadlist;tle!=ss.end_of_new;){
			nle = tle->le_next;
			ld_clearle(sspointer,tle);
			if (tle->le_flags & LE_DATA && tle->le_data) {
				if (tle->le_flags & LE_SRO)
	           	     		xmfree((void *)PAGEDOWN(tle->le_data),
						lib.la.la_text_heap);
				else
					xmfree((void *)PAGEDOWN(tle->le_data),
						lib.la.la_data_heap);
			}
			if (tle->le_flags & LE_TEXT && tle->le_file)
				xmfree(tle->le_file,lib.la.la_text_heap);
			xmfree(tle->le_exports,lib.la.la_data_heap);
			xmfree(tle,lib.la.la_data_heap);
			tle = nle;
		}
		ss.la->la_loadlist = ss.end_of_new;
		/* N.B. we don't retain the ss.d structure on error
		 * path.  In general, we can always correctly
		 * compute ss.d - so when in doubt throw the old
		 * one away. */
		xmfree(ss.d,kernel_heap);
		ss.topstuff->d = NULL;
		return NULL;
	}
	ss.topstuff->d = ss.d;
	return ld_instance(ss.topstuff,le);
	
exitrc:
	u.u_error=rc;
	le = NULL;;
	goto exitle;

returnNULL:
	if ( !recursion){
#if 0	/* see setjmpx comment above */
		ASSERT(! waslongjmpx);
		clrjmpx(&jbuf);
#endif
		ss.topstuff->pl = ss.pl;
		ss.topstuff->d = ss.d;
		ss.topstuff->ldr = ss.ldr;
		if (ss.flags & SS_LIBPATH_ALLOCED) {
			assert(!(ss.topstuff->flags & SS_LIBPATH_ALLOCED));
			ss.topstuff->libpath = ss.libpath;
			ss.topstuff->flags |= SS_LIBPATH_ALLOCED;
		}
	}
	return NULL;
}

/* Called from ld_checkdep when dependencies for the library
 * do not match.  In this case we turn off the LE_THISLOAD
 * flag and turn on the LE_NOTTHISLOAD flag in the passed le.
 * We then do the same for all le's that depend on an le with
 * the LE_NOTTHISLOAD flag on. Multiple passes are made over
 * the load list until no le's are found that satisfy this
 * condition.
 */

static void
ld_undothis(
struct  sharedstuff     *sspointer,
struct  loader_entry    *le)
{
	int i, check_again;

	le->le_flags &= ~LE_THISLOAD;
	le->le_flags |= LE_NOTTHISLOAD;
	do {
		check_again = 0;
		for (le = ss.la->la_loadlist; le; le=le->le_next) {
			if (le->le_flags & LE_THISLOAD) {
				for (i=1; i < le->le_ndepend; i++) {
					if (le->le_depend[i]->le_flags &
						LE_NOTTHISLOAD) {
						le->le_flags &= ~LE_THISLOAD;
						le->le_flags |= LE_NOTTHISLOAD;
						check_again++;
						break;
					}
				}
			}
		}
	} while (check_again);
}

/* called with an LD_DATA type le. must resolve each lib path name
 * and see if that is the same file used to make this le.  If so,
 * return 0, else return 1
 * we expect sspointer to provide libpath and support for path name lookaside
 */

static int
ld_checkdep(
struct	sharedstuff	*sspointer,
struct	loader_entry	*le)
{

	le->le_flags |= LE_THISLOAD;	/* mark current this load for now */

	if ('L' != ss.type)
		return 0;

	/* inorder for a prerelocated library routine to be useful, it must
	 * match exactly - including dependencies*/
	if (ld_libraries(sspointer,le,ld_create_check)){
		/* the libraries required for this le are not the ones it was
		 * resolved against
		 */
		ld_undothis(sspointer,le);
		return 1;
	}
	/* if it is good, leave the thisload bit on */
	return 0;
}

/* search a loader chain for an entry which represents the exact file
 * named by fp - the file pointer to an open file.
 * we skip unloaded entries or entries without the LE_DATA flag on
 * first we search for a thisload entry - if we find one it must match.
 * then we search again looking for an entry whose file dependencies match.
 * during the check, we tentitively mark the entry being tested as thisload.
 * Thus, if a depends on b and b depends on a we can find a pair (a,b) whose
 * other dependencies are correct.
 */
static struct loader_entry *
ld_findle(
struct sharedstuff *sspointer,
struct	file	*fp,
char	*member)
{
	struct loader_entry *le,*firstle;
	le = firstle = ld_findfp(ss.la->la_loadlist,fp,member);
	while(le){
		if ( (le->le_flags & (LE_DATA|LE_THISLOAD)) ==
		     (LE_DATA|LE_THISLOAD))
			return le;
		le = ld_findfp(le->le_next,fp,member);
	}
	le = firstle;
	while(le){
		/* It is possible that the LE_THISLOAD flag was set due to a
		 * previous call to ld_checkdep().  In this case we know all
		 * the dependencies are satisfied,  so simply return the le.
		 */
		if ( (le->le_flags & (LE_DATA|LE_THISLOAD)) ==
		      (LE_DATA|LE_THISLOAD))
			return le;
		if ( ((le->le_flags & (LE_DATA|LE_NOTTHISLOAD)) == LE_DATA) &&
			( le->le_flags & LE_SRO ||
			! ld_checkdep(sspointer,le))) return le;
		/* when checkdep fails, we may have reserved memory
		 * we don't need - so force map to be recomputed */
		xmfree(ss.d,kernel_heap);
		ss.d=NULL;
		le = ld_findfp(le->le_next,fp,member);
	}
        return NULL;
}


/* called with an LD_DATA type loader entry - finds or creates a private
 * instance.  sspointer is the anchor for the private load
 */
static struct loader_entry *
ld_getinstance(
struct sharedstuff	*sspointer,
struct loader_entry	*lible)
{
	struct loader_entry	*le;
	ASSERT(ss.type != 'L');
	/* if this is readonly - such as the kernel - use it directly */
	if (lible->le_flags & (LE_USEASIS|LE_SYSCALLS))
		return lible;
	/* search for existing instance - this code must be upgraded to deal
	 * with one use programs
	 */
	for(le=ss.la->la_loadlist;le;le=le->le_next)
		if ( (le->le_flags & (LE_UNLOAD|LE_TEXT|LE_DATA))==LE_DATA &&
		     le->le_depend[0] == lible) return le;
	return ld_instance(sspointer,lible);
}

BUGVDEF(ld_srodbg,0);

static struct loader_entry *
ld_sroinstance(
struct sharedstuff	*sspointer,
struct loader_entry *lible)
{
	struct loader_entry *le;

	BUGLPR(ld_srodbg,2,("sro instance: lible = 0x%x\n", lible));

	if (lible->le_ndepend > 1) {
		return NULL;
	}

	le = ld_ualloc(sspointer,sizeof(struct loader_entry));

	if (!le) {
		u.u_error = ENOMEM;
		return NULL;
	}

	le->le_usecount = 0;
	le->le_loadcount = 0;
	le->le_flags =
	      LE_DATA|LE_DATAEXISTS|LE_LIBEXPORTS|LE_USEASIS|
	      LE_THISLOAD|LE_SRO;
	le->le_fp = lible->le_fp;
	fp_hold(le->le_fp);
	le->le_file = lible->le_file;
	le->le_filesize = lible->le_filesize;
	le->le_data = lible->le_data;   /* target location of data */
	le->le_datasize = lible->le_datasize;
	le->le_exports = lible->le_exports;
	le->le_lex = NULL;
	le->le_defered = NULL;
	le->le_ndepend = 1;
	le->le_maxdepend = lible->le_maxdepend;
        le->le_depend[0] = lible;
        le->le_filename = lible->le_filename;
	lible->le_usecount++;
	le->le_next = ss.la->la_loadlist;
	ss.la->la_loadlist = le;
	BUGLPR(ld_srodbg,2,("sro instance: newle = 0x%x\n", le));
	return le;
}

/*
 * NAME:        ld_domain_add(ld, le, can_add)
 *
 * FUNCTION:	This routine is designed to be called from ld_instance().
 *		It will be called when a new library entry is added to
 *		a processes load list.  This routine will check to see
 *		if the loader entry already exists in the loader domain.
 *		If it does not already exist,  then it is added to the
 *		domain(if the process has sufficient privledge).
 *
 * PARAMETERS:  ld      - pointer to loader domain structure
 *              le      - pointer to loader entry structure
 *		can_add - flag to indicate process can add entries to the
 *			  loader domain
 *
 * MODIFIED	A new domain entry may be created and added to the
 * DATA		specified loader domain
 *
 * RETURN:      de - pointer to global domain entry
 *
 */
struct domain_entry *
ld_domain_add(struct loader_domain *ld, struct loader_entry *le, int can_add)
{
	struct domain_entry *de;
	char *p;

	/* Search for loader entry within the domain */
	de = ld->ld_entries;
	while (de) {
		if (de->de_le == le)
			return(de);
		de = de->de_next;
	}
	
	/* 
	 * no domain entry for this loader entry
	 * add it if permissions allow
	 */
	if (!can_add) {
		ld_emess(L_ERROR_DOMADD, ld->ld_name, le);
		u.u_error = EACCES;
		return(NULL);
	}

	de = xmalloc(sizeof(struct domain_entry), 2, lib.la.la_data_heap);

	if (!de) {
		/*
		 * system is in serious trouble(out of page space),
		 * but we won't force the issue here
		 */
		u.u_error = ENOMEM;
		return(de);
	}

	/* initialize fields in domain_entry structure */
	de->de_le = le;
	de->de_usecount = 0;
	p = (de->de_le)->le_filename;
	while(*p++);	/* skip member name */
	de->de_fullname = p;
	de->de_altname = NULL;

	/* add loader entry to domain list */
	de->de_next = ld->ld_entries;
	ld->ld_entries = de;

	return(de);
}

/* instance creates a local instance of a library loaded module
 * all it does is create the loader entry and compute the data location
 * for the instance. Note that this is a USEASIS instance.  If ld_instance
 * can't to this, it calls privinstance to make a relocated private copy.
 */
static struct loader_entry *
ld_instance(
struct sharedstuff	*sspointer,
struct loader_entry *lible)
{
	struct loader_entry *le;
	struct domain_entry *de;
	struct loader_domain *ld;
	uint	datasize,i;
	
	/* if this is a shared library load, we already have the "instance"*/
	if (ss.type == 'L') return lible;

	if (lible->le_flags & LE_SRO) 
		return ld_sroinstance(sspointer,lible);
	
	le = ld_ualloc(sspointer,
		       sizeof(struct loader_entry) +
		       sizeof(struct loader_entry*)*(lible->le_maxdepend-1));
	/* N.B. we don't bzero - so must initialize all fields */
	if (!le){
		u.u_error = ENOMEM;
		 return NULL;
	}
	{	/* copy the defered chain for private use
		 * N.B. do this first since it can fail.
		 */
		struct loader_defered	*ld,*nld,*libld;
		nld = NULL;
		for(libld=lible->le_defered;libld;libld=libld->next){
			ld=ld_ualloc(sspointer,sizeof(struct loader_defered));
			if (!ld) {
				while(nld){
					ld=nld->next;
					ld_ufree(sspointer,nld);
					nld=ld;	
				}
				ld_ufree(sspointer,le);
				return NULL;
			}
			*ld = *libld;
			ld->next = nld;
			nld = ld;
		}
		if(le->le_defered = nld)
			ss.la->la_flags |= LA_DEFERED;
	}
		
	le->le_usecount = 0;
	le->le_loadcount = 0;
	le->le_flags =
	      LE_DATA|LE_DATAMAPPED|LE_LIBEXPORTS|LE_USEASIS|LE_THISLOAD;
	le->le_fp = lible->le_fp;
	fp_hold(le->le_fp);
	le->le_file = lible->le_file;
	le->le_filesize = lible->le_filesize;
	le->le_data = lible->le_data;		/* target location of data */
	le->le_datasize = lible->le_datasize;	/*N.B. this includes bss*/
	le->le_exports = lible->le_exports;
	le->le_lex = NULL;
        le->le_ndepend = 1;
        le->le_maxdepend = lible->le_maxdepend;
        le->le_depend[0] = lible;
        le->le_filename = lible->le_filename;
        lible->le_usecount += 1;
	le->le_next = ss.la->la_loadlist;    /* add to the process load list */
	ss.la->la_loadlist = le;
	/*
	 * if the process uses a loader domain,  create/add new loader
	 * domain entry if necessary.
         */
	if (ld = ss.la->la_domain) {
		de = ld_domain_add(ld, lible,
					ss.la->la_flags & LA_DOMAIN_WRITE);
		if (!de)
			return NULL;
		/*
		 * set flag in loader entry to indicate the loader entry
		 * corresponds to a domain entry
		 */
		le->le_flags |= LE_DOMAIN_ENT;
		le->le_de = de;
		de->de_usecount++;
	}

	/* now that this one is on the loadlist we can safely instantiate
	 * everything it depends on - note that previous checks guarantee
	 * that this will work
	 */
	for(i=1;i<lible->le_ndepend;i++){
		le->le_depend[i] =
		       ld_getinstance(sspointer,lible->le_depend[i]);
		ASSERT(le->le_depend[i]);
		if (!le->le_depend[i]) return NULL;
		le->le_depend[i]->le_usecount += 1;
		le->le_ndepend += 1;
	}
	return le;
}

/*
 * getlibread is the special version of getlib used during the
 * readin prepass.  Instead of looking for a "good" le, it looks for
 * any le for the requested text, and if none exists it reads one in.
 * It must look both in the private and library load lists.  If it doesn't
 * find an le it must read one into either the private or library text
 */
static struct loader_entry *
ld_getlibread(
struct sharedstuff	*sspointer,
struct file             *fp,
char			*member)	/*name of member or null string*/
{
	struct loader_entry *le,**newpre;
	ulong i;
	if ( ! (le = ld_libgettext(sspointer,fp,member)))
	    le = ld_privgettext(sspointer,fp,member);

	if (!le)
		return NULL;

	for(i=0;i<ss.precount;i++)
		if (ss.pre[i] == le)
			return le;

	if(ss.precount == ss.maxprecount){
		ss.maxprecount +=236; /* 236+20 is 256 - see intial value*/
		newpre = xmalloc(ss.maxprecount*(sizeof le),0,kernel_heap);
		bcopy(ss.pre,newpre,ss.precount*(sizeof le));
		if (ss.precount > 236)
			xmfree(ss.pre,kernel_heap);
		ss.pre = newpre;
	}	

	ss.pre[ss.precount++] = le;
	le->le_usecount += 1;
	return le;
}
/*
 * ld_preread is called at the first entry to the ld_libld package, which may 
 * either be at getlib or ld_libraries.  It implements the strategy for avoiding
 * I/O wait deadlocks and livelocks.  In otherwords, all I/O will now be done 
 * without holding the library lock.  To do this, a prepass is taken in which
 * opens and reads are completed but no further loading is done.  It is safe 
 * to release the loader lock during each I/O in the prepass.
 *
 * The way this works is that we call into the normal load logic.  However,
 * getlib will satisfy each request with any le, since if any le exists, the 
 * text must be available.  The first time getlib finds the le, it adds it 
 * to the pre vector and ups its usecount.  Thus, none of the things already 
 * found can disappear during lock release sequence.
 */

static int
ld_preread(
struct	sharedstuff	*sspointer,
struct	loader_entry	*le,
char			*path,	/*search path or NULL*/
char			*filename,	/*name of file*/
char			*member)	/*name of member or null string*/
{
	struct loader_entry *localpre[20];
	int rc;
	ulong i,nextle;
	struct loader_entry *getlib_le;
	if (ss.type == 'K')
		return 0;
	ss.execle = ss.la->la_execle;
	ss.pre = localpre;
	ss.precount = 0;
	ss.maxprecount = (sizeof localpre)/(sizeof le);
	nextle = 0;
	/* now call the same entry that just called preread, but
	 * specify the 3 mode, which is preread */
	
	if (le){
		ss.pre[0] = le;
		ss.precount = 1;
		le -> le_usecount += 1;  /* add one only so count down loop
					    below is uniform */
		nextle = 1;
		rc=ld_libraries(sspointer,le,ld_create_read);
	}
	else{
		getlib_le=ld_getlib(sspointer,path,filename,member,ld_create_read);
		rc = getlib_le == NULL;
	}

	while (!rc && nextle < ss.precount){
		rc = ld_libraries(sspointer,ss.pre[nextle],ld_create_read);
		nextle += 1;
	}

	/*
	 * The code in ld_libraries assumes that ss.libpath is initially
	 * NULL.  If ss.libpath is not null,  then it is prepended to
	 * the libpath information in the shared object.  Therefore,  we
	 * must insure that ss.libpath is NULL for the initial call.
	 * because of the prepass,  there really are 2 'initial' calls.
	 * save the value of ss.libpath in ss.libpath_saved,  this will
	 * be restored in ld_libraries.
	 */
	if (ss.libpath) {
		ss.libpath_saved = ss.libpath;
		ss.libpath = NULL;
        }
	
		
	/*
	 * clean up any loader domain reopen arrays.  this could
	 * possibly release locks.  therefore,  it must be done
	 * before decrementing use counts.
	 */
	if (ss.ldr) {
		ld_clean_reopen(sspointer, ss.ldr);
		ss.ldr = NULL;
	}

	for(i=0;i<ss.precount;i++)
		ss.pre[i]->le_usecount -= 1;

	if(ss.pre != localpre)
		xmfree(ss.pre,kernel_heap);
	ss.pre = NULL; /*this lets us have an assert later on to
		        make sure we don't do I/O after the preread*/

	/* If this is an explicit load call,  check the process private
	 * load list for unloaded entries.  For each unloaded entry mark
	 * the library entry as NOTTHISLOAD.  This will prevent reuse
	 * of an unloaded entry and force creation of a new one if
	 * needed.  ld_freelex() will reset the bits in the library
	 * loader entries.
	 */
	if (ss.type == 'C') {
		for(le=ss.la->la_loadlist; le; le=le->le_next) {
			if((le->le_flags & (LE_DATA|LE_USEASIS|LE_UNLOAD)) ==
			(LE_DATA|LE_USEASIS|LE_UNLOAD))
				le->le_depend[0]->le_flags |= LE_NOTTHISLOAD;
		}
	}

	return rc;
}


/* getlib processes a request for making a pathname(member) available.
 * If the request is for create, getlib actually calls the routines needed
 * to load the program.
 * create can be 0, 1, 2, or 3.  1 indicates the external call from ld_load.
 * 2 is an internal call to create the library.  0 is an internal call to 
 * verify that the library exists.  3 is a call to verify that the text of 
 * the library exists.
 */
struct loader_entry *
ld_getlib(
struct sharedstuff	*sspointer,
char			*path,	/*search path or NULL*/
char			*filename,	/*name of file*/
char			*member,	/*name of member or null string*/
int			create)		/*create libs or just check them*/
{
	struct file* fp;      /* file pointer to library */
	struct loader_entry *le;

	if (ld_create_initialcall == create){
		if (ld_preread(sspointer,NULL,path,filename,member))
			return NULL;
		create = ld_create_create;
	}

	if (! memcmp(filename,"/\0unix\0",7)&& !*member)
		return (ss.type == 'K')?kernel_exports:syscall_exports;

	fp=ld_pathopen(sspointer,path,filename,NULL); /* NULL for normal
						         case - see pathinit*/

	if (! fp)
		return NULL;
	/* if request if for the exec'd file avoid loading another
	 * copy in the shared library*/
	else if (ss.execle && FPtoFH(fp) == FPtoFH(ss.execle->le_fp))
		return ss.type=='L' ? NULL : ss.execle;
	else if (ld_create_read == create)
		return ld_getlibread(sspointer,fp,member);
	/* see if we have already instantiated this library in this process */
	else if (le=ld_findle(sspointer,fp,member));
	/* see if we are just checking - if so give up */
	else if ( ld_create_check == create ) /* do nothing */;
	/* see if we can load an instance into the shared library segment*/
	else if (le=ld_libld(sspointer,fp,member));
	/* is u_error is set something bad happened - give up */
	else if (u.u_error);
	/* see if we can load a private instance */
	else le=ld_privld(sspointer,fp,member);
        /* end of confusing else if sequence - le now has a good value
         *                                            (maybe NULL)
         * N.B. we don't close fp - the pathopen/pathclear mechanism takes
	 * care of that
	 */

        return le;
}

/* libraries is called to process an le which has been added to the load list
 * (private, library, or kernel - see ss.type).
 * it loops through the list of requred files.  For each one, it finds or
 * causes to be created an le ON THE SAME load list for that file
 * Note that the process of creating an entry may cause libraries to
 * be called recursively - so the code must be careful about leaving things
 * in a good state.
 * N.B. if the indirect result of this call is to add things to the library,
 *      those must be resolved and relocated.  HOWEVER, the le's added to
 *      the process load list are unresolved unless they are library copies
 *      mared USEASIS.  They will be resolved by the relocate and resolve
 *      loops called from execld.
 *
 * Path processing has been modified so that the concatenation of two
 * paths - the default and the module, are used.  This supports better
 * LIBPATH semantics, since LIBPATH adds to rather than replaces the
 * default.  Thus, you only need put the new stuff in LIBPATH.
 *
 * We do it by copying since this is simpler than passing two strings around
 * particularly since the pathlook must hash the whole path anyhow.
 *
 * create can be 0, 1, 2, or 3.  1 indicates the external call from ld_execld.
 * 2 is an internal call to create the library.  0 is an internal call to 
 * verify that the library exists.  3 is a call to verify that the text of 
 * the library exists.
 */
int
ld_libraries(
struct	sharedstuff	*sspointer,
struct	loader_entry	*le,
int	create)
{
	struct	loader_entry_extension	*lexpointer;
        int	i,j,k;
        uint	depend,nextdep,size;
        struct	loader_entry	*lible;
        char	*imp,*filename,*member,*libpath,*tend;
        char	pathbuffer[128];
	int	plen,mlen,tlen;
	int	libpathmalloced=0;
	int	rc = 0;

	if (ld_create_initialcall == create){
		if (rc = ld_preread(sspointer,le,NULL,NULL,NULL))
			return rc;
		create = ld_create_create;
	}

        /* N.B. in the non-create path, lexpointer may be null ! */
        lexpointer = le->le_lex;
        if (ld_create_create == create) {
		depend = lex.nimpid;
  		size = sizeof(struct loader_entry) +
		       sizeof(struct loader_entry*) * (depend);
		lex.impid =
		     (struct loader_entry **) xmalloc(size,0,kernel_heap);
		if (lex.impid == NULL) return ENOMEM;
		imp = (char *)lex.ldhdr+lex.ldhdr->l_impoff;
	}
	else {
		struct myxcoffhdr *h;
		struct ldhdr	*ldhdr;
		h = (void *)le->le_file;
		ldhdr = (void *)
		      ((char *)le->le_file + h->s[h->a.o_snloader-1].s_scnptr);
		depend = ldhdr->l_nimpid - 1;
		imp = (char *)ldhdr + ldhdr->l_impoff;
		if (le->le_flags & LE_TEXT) nextdep = 0;else nextdep = 1;
	}
	

	/* getlib returns the loader entry in the process private segment for
	 * this library the path name string is a series of entries, the
	 * first of which represents the LIBPATH of this bind.
	 * Each entry consists of three NULL terminated strings - the path,
	 * the basefilename, and the member.  Any of these can be the empty
	 * string.  depend is the number of strings.  The first actual
	 * library name is the second string - which is considered ordinal 1.
	 */
	
	tend = ss.type != 'K' ?
		le->le_file + le->le_filesize :
		(char *) &hdr + hdr.s[lex.loader].s_scnptr +
			hdr.s[lex.loader].s_size;
#if 0
	/*
	 * tend = le->le_file + le->le_filesize;
	 * is not enough because the loader section might not be
	 * contiguous with the rest of the file
	 */
	tend = (char *) &hdr + hdr.s[lex.loader].s_scnptr +
		hdr.s[lex.loader].s_size;
#endif

	/*
	 * ss.libpath is set first time through this code.  the ss.libpath
	 * string is used throughout the loader operation.  it is prepended
	 * to the libpath string contained in the loader section.
	 */
	if (!ss.libpath) {
		/*
		 * if there is a saved libpath from the prepass,  restore
		 * it now.
		 */
		if (ss.libpath_saved) {
			ASSERT(!ss.pre)	/* must not be in the prepass */
#ifdef DEBUG
			/*
			 * DEBUG code asserts that saved libpath is the same
			 * as what a newly constructed libpath would be.
			 */
			if (ss.libpath_env&&(0 != strcmp(ss.libpath_env,imp))){
				plen = strlen(ss.libpath_env);
				mlen = strlen(imp);
				tlen = plen+mlen+2;

				assert(!memcmp(ss.libpath_saved +
					(ss.ld ? tlen : 0), ss.libpath_env,
					plen));
				assert(!memcmp(ss.libpath_saved + plen +
					(ss.ld ? tlen : 0),":",1));
				assert(!memcmp(ss.libpath_saved + plen + 1 +
					(ss.ld ? tlen : 0), imp, mlen + 1));
			}
			else {
				mlen = strlen(imp) + 1;
				assert(!memcmp(ss.libpath_saved +
					(ss.ld ? mlen : 0), imp, mlen));
			}
#endif /* DEBUG */
			ss.libpath = ss.libpath_saved;
			ss.libpath_saved = NULL;
		}

		/*
		 * if LIBPATH environment variable was set,  then ss.libpath
		 * becomes LIBPATH:'libpath string in loader section'
		 */
		else if (ss.libpath_env && (0 != strcmp(ss.libpath_env,imp))) {
			plen = strlen(ss.libpath_env);
			mlen = strlen(imp);
			tlen = plen+mlen+2;
			/*
			 * if loader domain is specified,  build full libpath
			 * in second half of allocated space.  the full path
			 * is then pruned,  and the resulting libpath is placed
			 * in the first half of the allocated space.
			 */
			ss.libpath = xmalloc(tlen * (ss.ld ? 2 : 1),
					     0, kernel_heap);
			assert(ss.libpath);
                	memcpy(ss.libpath + (ss.ld ? tlen : 0),
			       ss.libpath_env, plen);
			ss.libpath[plen + (ss.ld ? tlen : 0)] = ':';
			memcpy(ss.libpath + plen + 1 + (ss.ld ? tlen : 0),
			       imp, mlen + 1);
			if (ss.ld) {
				ASSERT(ss.pre);	/* must be prepass */
				prune_libpath(ss.libpath + tlen, ss.libpath);
			}
			ss.flags |= SS_LIBPATH_ALLOCED;
		}
		else {
			/*
			 * ss.libpath is set to libpath string in loader
			 * section of the file
			 */
			if (!ss.ld)
				ss.libpath = imp;
			else {
				/*
				 * prune libpath if loader domain is in use
				 */
				mlen = strlen(imp) + 1;
				ss.libpath = xmalloc(mlen * 2, 0, kernel_heap);
				assert(ss.libpath);
				memcpy(ss.libpath + mlen, imp, mlen);
				ASSERT(ss.pre);	/* must be prepass */
				prune_libpath(ss.libpath + mlen, ss.libpath);
				ss.flags |= SS_LIBPATH_ALLOCED;
			}
		}
		libpath = ss.libpath;
	}
	else {
		/*
		 * this is not first time through code.  ss.libpath is
		 * prepended to libpath information in this file.
		 */
		libpath = ss.libpath;
		if (0 != strcmp(libpath,imp)){
			/* concatenate default and module - only if
			 * if makes a difference */
			plen = strlen(libpath);
			mlen = strlen(imp);
			if ((plen+mlen+2) > sizeof pathbuffer){
				libpath = xmalloc(plen+mlen+2,0,kernel_heap);
				assert(libpath);
				libpathmalloced = 1;
			}
			else
				libpath = pathbuffer;	/* normal case */
			memcpy(libpath,ss.libpath,plen);
			libpath[plen] = ':';
			memcpy(libpath+plen+1,imp,mlen+1);
		}
	}
	for(;imp<tend&&*imp++;);	/* find end of path */
	imp++;	/* skip of path part of filename - always null for
		  this entry */
	imp++;	/* move on to start of first file name entry */
	for(i=0;i<depend;i++){
		filename = imp;
		for(;imp<tend&&*imp++;);	/* find end of path part of
						   filename*/
		for(;imp<tend&&*imp++;);	/* find end of base part of
						   filename */
		member = imp;
		for(;imp<tend&&*imp++;);	/* find char beyond null at
						   end of member */
		if (imp > tend) {               /* if imp == text, null at
						   end of memberi */
			rc = ENOEXEC;
			goto exit;     /* is the very last charcter
						   in the file */
		}
			
		/* due to binder behavior, we must deal with completely null
		 * file name string to represent defered.  (It was supposed
		 * to be a zero file index in the symbol entry!.
		 * We catch that here and iterate.
		 * BE CAREFUL about this code.  The le_depends list is not
		 * one to one with the impid list - the le_depend list
		 * does NOT have an entry  corresponding to the defered
		 * file name.  To put a null entry in would cost testing
		 * for null all over the place.
		 * The actual test below depends on the fact that each string
		 * (path,base,member) is null terminated - so if all three are
		 * together only three characters long they MUST be three
		 * nulls.
		 */
		if (filename+3==imp){
			if (ld_create_create == create)
				lex.impid[i] = NULL;
			continue;
		}
	
		/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		 * rest of loop skipped for null case - be very careful
		 * about loop counters.  i counts entries in impid - but
		 * NOT entries in le_depend.
		 */
		
		lible = ld_getlib(sspointer,libpath,filename,member,create);

		if (!lible){
			/* error message only needed if this failure will
			 * really stop the load.  The 'L' path is attempting
			 * to pre-relocate - if it fails we try again with
			 * a private load.  The non-create path is verifying
			 * that the library can be used - again if this
			 * fails we will try to load a fresh copy
			 * The create call will fail if the needed library 
			 * was read but has a bad format.  The read call 
			 * will fail if the library can't be found.
			 */
			if ((ss.type != 'L') && ((ld_create_create==create)
			   ||(ld_create_read==create))){
                                char	*p,*q;
                                char	ebuf[128];
                                q = ebuf;
                                /* file name is in two parts - path and base
                                 * each a null terminated string.
                                 * -10 is to leave some room for member name */
                                for(p=filename;*p&&q<ebuf+sizeof ebuf-10;*q++=*p++);
                                if ( p != filename && *(p-1) != '/' )
                                	*q++='/';
                                for(p++;*p&&q<ebuf+sizeof ebuf-10;*q++=*p++);
                                /* member immediately follows file name.
                                 * it may be a null string. Right now,
                                 * p points to null at end of file base name,
                                 * q to char beyond last copied */
                                if (*++p)
                                	*q++=' ';	/* blank - then member */
                                for(;(*q++=*p++)&&q<ebuf+sizeof ebuf;);
                                *(q-1) = 0;	/* if ran out of space */
                                ld_emess(L_ERROR_NOLIB,ebuf,NULL);
			}
			rc = u.u_error ? u.u_error : ENOEXEC;
			goto exit;
		}
		
		if (ld_create_create == create) {
			ASSERT(le->le_ndepend<le->le_maxdepend);
			lex.impid[i] = lible;
			le->le_depend[le->le_ndepend++] = lible;
			lex.impid[i]->le_usecount += 1;
		}
		else if (ld_create_check == create) {
		  ASSERT(nextdep < le->le_maxdepend);
		  if (le->le_depend[nextdep] == lible) {
			nextdep += 1;
		  }
		  else {	/* verification failed */
		    if (ss.type != 'K' && !*member &&
		      !memcmp(filename,"/\0unix\0",7) ) {
			/* The verification failed because of a new kernel
			 * name space.  It is most often the case that
			 * updates to the kernel name space have no effect
			 * on the kernel symbols imported by this module.
			 * This is the case if this new version of the
			 * kernel name space depends on the version of the
			 * kernel name space this module depends on AND
			 * the new kernel name space does not redefine any
			 * symbols defined in the kernel name space this
			 * modules depends on.  If both of these conditions
			 * are met,  then we will say the old kernel name
			 * space can continue to be used.  The loop below
			 * checks all this.
			 */
			struct loader_entry *tmple = lible;

			/*
			 * verify should always pass if a loader domain
			 * was specified and kernel name space is in
			 * question.
			 */
			if (ss.ld)
				nextdep += 1;
			else {
			  while (tmple->le_ndepend) {
				if (tmple == le->le_depend[nextdep]) {
					/* This verifies use of this module */
					break;
				}
				else if (tmple->le_flags & LE_REDEFINEEXP) {
					/* can't use this module */
					rc = ENOEXEC;
					goto exit;
				}

				tmple = tmple->le_depend[0];
			  }

			  if (tmple == le->le_depend[nextdep])
				nextdep += 1;
			  else {
				/* If we get here it means the new kernel
				 * name space does not depend on the previous
				 * kernel name space. In this case verification
				 * must fail
				 */
				rc = ENOEXEC;
				goto exit;
			  }
			} /* else no domain */
		    } /* if kernel name space */
		    else {  /* not kernel name space and verification failed */
			rc = ENOEXEC;
			goto exit;
		    }
		  } /* else verification failed */
		} /*  ld_create_check */
	}

	rc = 0;
exit:
	if (libpathmalloced)
		xmfree(libpath,kernel_heap);
	return rc;
}


/*
 * prune_libpath
 *	prune all nondirectory paths from the libpath string.  two
 *	character pointers are passed to this routine:
 *	orig_libpath - original libpath string(null terminated).
 *	new_libpath  - space allocated for new libpath string.  this
 *		       space is at least as big as orig_libpath.
 *
 *	NOTE -	this routine is required to release the global loader
 *		lock while making calls into filesystem code.  therefore,
 *		this routine can only be called during the pre-pass.
 */
void
prune_libpath(char *orig_libpath, char *new_libpath)
{

	char *p, *q, *start_tpath;
	struct file *fp;
	struct stat stat;

	p = orig_libpath;
	q = new_libpath;

	/*
	 * must not be holding locks while performing open or
	 * stat operations
	 */
	unlockl(&kernel_anchor.la_lock);

	/*
	 * parse original string
	 */
	while (*p) {

		start_tpath = q;
		fp = NULL;

		/*
		 * libpath entries are separated by a ':',  note that
		 * "::" is a valid entry which will result in searching
		 * the current directory.
		 */
		while (*p && *p != ':')
			*q++ = *p++;

		*q = '\0';
		/*
		 * test if libpath entry is NULL(current directory) or
		 * a directory
		 */
		if ( (q == start_tpath) ||
		     ( (q - start_tpath <= PATH_MAX) &&
		       (!fp_open(start_tpath, O_RDONLY, 0, 0, FP_SYS, &fp)) &&
		       (!fp_fstat(fp, &stat, sizeof(stat), FP_SYS)) &&
		       (S_ISDIR(stat.st_mode)) ) ) {
			/*
			 * entry is a directory, add it to new string
			 */
			*q++=*p;
		}
		else {
			/*
			 * do not include entry in pruned libpath string
			 */
			q = start_tpath;
		}

		/*
		 * advance to next entry in original libpath string
		 */
		if (*p)
			p++;

		/*
		 * close file
		 */
		if (fp)
			fp_close(fp);
        }

	/*
	 * don't let new libpath end with a NULL path unless original
	 * libpath also ended with a NULL path.
	 */
	if ( (p != orig_libpath) && (*--p != ':') &&
	     (q != new_libpath) && (*--q == ':'))
		*q = '\0';

	/*
	 * be sure to reacquire lock
	 */
	lockl(&kernel_anchor.la_lock,LOCK_SHORT);
}
