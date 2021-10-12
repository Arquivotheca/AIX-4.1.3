static char sccsid[] = "@(#)26	1.78.1.4  src/bos/kernel/ldr/ld_files.c, sysldr, bos41J, 9520A_all 5/16/95 16:45:47";
/*
 * COMPONENT_NAME: (SYSLDR) Program Management
 *
 * FUNCTIONS: ld_assigndata1(), ld_assigndata(), oldar(), newar(),
 *            ld_textread(), ld_sanity(), ld_freelex(), ld_pathinit(),
 *            ld_pathclear(),ld_pathopen(), ld_fptopath(), knlist(),
 *            ld_getinfo(), ld_getinfo_user()
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
#include	<ar.h>
#include	<sys/adspace.h>
#include	<sys/errno.h>
#include	<sys/fp_io.h>
#include	<sys/ldr.h>
#include	<sys/lockl.h>
#include	<sys/malloc.h>
#include	<nlist.h>
#include	<sys/pin.h>
#include	<sys/priv.h>
#include	<sys/pseg.h>
#include	<sys/seg.h>
#include	<sys/systm.h>
#include	<sys/uio.h>
#include	<sys/xcoff.h>
#include	<sys/syspest.h>
#include	<unistd.h>
#include	<sys/statfs.h>
#include	<sys/vnode.h>
#include	<sys/vfs.h>
#include	<sys/vattr.h>
#include	"ld_data.h"
#include	"ff_alloc.h"
#include	"ld_fflist.h"

int	firstfitsize = 30 * 1024 * 1024;
int	firstfitmax = 300 * 1024;
FF_DECLARE(pinned_fflist);
FF_DECLARE(unpinned_fflist);

#define memcpy(t,s,l) bcopy(s,t,l)
/* compute data locations for exec'd program.  This is special
 * because it must come at the end.  This code assumes sbreak
 * had been updated.
 */
static int
ld_assigndataexec(
struct sharedstuff *sspointer,
struct loader_entry	*le,
ulong	minbreak)
{
	struct loader_entry_extension	*lexpointer;
        ulong 	align,temp,dbreak;
        int	rc;

        lexpointer = le->le_lex;
        ASSERT(lexpointer);
        ASSERT(!le->le_data);
	/* round to next page boundary */
	dbreak = minbreak = PAGEUP(minbreak);

        /* If alignment of data in memory is as required we can page map
	 * copy it. To do this, we must compute an origin which has the
	 * same page offset as the data in memory and which is on a new page.
	 * Start is the location in the first page where data must start.
	 * If we pagemap, it is the page offset of the data in the file.
	 * If we copy, it is the offset at which the ldr relocated (vaddr)
	 * masked by align.
         */
        align=(1<<hdr.a.o_algndata)-1;
        if (((ulong)
	     ( le->le_file + hdr.s[lex.data].s_scnptr -
		   hdr.s[lex.data].s_vaddr) & align & (PAGESIZE-1) ) == 0){
		le->le_flags |= LE_DATAMAPPED;
		minbreak += (ulong)(le->le_file + hdr.s[lex.data].s_scnptr)&(PAGESIZE-1);
        }
	
        /* general fix up of address is to take the difference of where the
         * code wants to be and the current break mod the alignment requirement
         * and add that to the break.  Note that taking mod by anding the
         * mask is correct mathematically - that is the result is always
         * positive even if the difference is negative
         * N.B. we do this EVEN when we have already page aligned above - this
	 * will only make a difference if the required alignment is STRONGER
	 * than PAGESIZE
         */
        minbreak += align  & (hdr.s[lex.data].s_vaddr - minbreak);

	if (hdr.a.o_maxdata)
	{
		struct	dinuse	*dinuse;

		/* adjust break before changing minbreak */
		if (BRK(dbreak))
			return ENOMEM;

		U.U_sdsize = dbreak - PRIVORG;

		/* create additional segments required by the program */
		if (rc = ld_crsegs(sspointer,hdr.a.o_maxdata,&minbreak))
			return rc;

		/*
		* Mark the chunk from dbreak to the end of the segment
		* as being in use.  This forces load(s) into segment 3.
		*/
		dinuse = ld_ualloc(sspointer,sizeof(struct dinuse));
		dinuse->next = ss.la->la_dinuse;
		ss.la->la_dinuse = dinuse;
		dinuse->start = dbreak;
		dinuse->length = (ulong)(SEGSIZE - (dbreak - PRIVORG));
	}
	else
		U.U_sdsize = 0;

        le->le_data = (void *)(minbreak);
        lex.datareloc = (uint)minbreak - hdr.s[lex.data].s_vaddr;
       	lex.bssreloc =	(uint)minbreak + hdr.a.o_dsize - hdr.s[lex.bss].s_vaddr;

        temp=hdr.a.o_dsize + hdr.a.o_bsize;
        le->le_datasize = temp;
	/* set break here as a performance optimization - it gets set	
	 * for good in mapdata1 if necessary */
        return BRK(minbreak+temp) ? ENOMEM : 0;
}

/* compute pirvate area data locations for execloaded, loaded, or
 * private libarary loaded programs, but NOT the exec'd program.
 */
static int
ld_assigndata1(
struct sharedstuff *sspointer,
struct loader_entry	*le)
{
	struct loader_entry_extension	*lexpointer;
        ulong 	align,temp,size,start;
        int	rc;

        lexpointer = le->le_lex;
        ASSERT(lexpointer);
        if ( le->le_flags & LE_DATAEXISTS) {
        	/* just compute relocation factors in case needed */
        	lex.datareloc = (uint)le->le_data - hdr.s[lex.data].s_vaddr;
       		lex.bssreloc =	(uint)le->le_data + hdr.a.o_dsize - hdr.s[lex.bss].s_vaddr;
		return 0;
        }
        	
        ASSERT(!le->le_data);
        /* If alignment of data in memory is as required we can page map
	 * copy it. To do this, we must compute an origin which has the
	 * same page offset as the data in memory and which is on a new page.
	 * Start is the location in the first page where data must start.
	 * If we pagemap, it is the page offset of the data in the file.
	 * If we copy, it is the offset at which the ldr relocated (vaddr)
	 * masked by align.   Note that we only support alignments up to a page
	 * here.  To do better, allocd must be extended to support alignment.
         */
        align=(1<<hdr.a.o_algndata)-1;
        if (((ulong)
	     ( le->le_file + hdr.s[lex.data].s_scnptr -
		   hdr.s[lex.data].s_vaddr) & align & (PAGESIZE-1)) == 0){
		le->le_flags |= LE_DATAMAPPED;
		start = (ulong)(le->le_file + hdr.s[lex.data].s_scnptr)&(PAGESIZE-1);
        }
        else
        	start = (ulong)(hdr.s[lex.data].s_vaddr)&align;
        /* now compute the size required for this data area.  Size
         * is an integral number of pages.  Take alignment into account.
         */
        le->le_datasize = hdr.a.o_dsize + hdr.a.o_bsize;
	size = PAGEUP(start+hdr.a.o_dsize + hdr.a.o_bsize);

#ifdef	notyet
	if (ss.load_addr && ss.type == 'C')
		temp = ss.load_addr;
	else
#endif
		if (!(temp = ld_allocd(sspointer,size)))
			return ENOMEM;
	
        le->le_data = (void *)(temp+start);
        lex.datareloc = (uint)(le->le_data) - hdr.s[lex.data].s_vaddr;
       	lex.bssreloc =	(uint)(le->le_data) + hdr.a.o_dsize - hdr.s[lex.bss].s_vaddr;
	return 0;
}


int
ld_assigndata(
struct sharedstuff	*sspointer)
{
	struct loader_entry	*le,*execle;
	int	rc;
	ulong minbreak;

	minbreak=DATAORG;
	execle=NULL;
	for(le=ss.la->la_loadlist;le != ss.end_of_new;le=le->le_next){
		/* USEASIS means this data is a copy of pre-relocated data
		 * thus its location has already been determined*/
		if ( (le->le_flags & (LE_DATA|LE_USEASIS|LE_EXECLE)) == LE_DATA){
			if (rc = ld_assigndata1(sspointer,le)) return rc;
		}
		else
			if ((le->le_flags & LE_EXECLE))
				execle = le;
		/* DATAEXISTS means this data is in the library */
		if ((le->le_flags &(LE_DATA|LE_DATAEXISTS|LE_USEASIS))==LE_DATA)
			minbreak = MAX(minbreak,(ulong)le->le_data+le->le_datasize);
		if (le->le_flags & LE_TEXT)
			minbreak = MAX(minbreak,(ulong)le->le_file+le->le_filesize);
	}
	if (execle)
		return ld_assigndataexec(sspointer,execle,minbreak);
	return 0;
}

static ulong
oldar(
struct sharedstuff *sspointer,
struct file	*fp,
char		*member)
{
/* TEMPORARY - old style ar.h */
/* ar.h	5.1 - 86/12/09 - 06:03:39 */

/*		COMMON ARCHIVE FORMAT
*
*	ARCHIVE File Organization:
*	_______________________________________________
*	|__________ARCHIVE_MAGIC_STRING_______________|
*	|__________ARCHIVE_FILE_MEMBER_1______________|
*	|					      |
*	|	Archive File Header "ar_hdr"          |
*	|.............................................|
*	|	Member Contents			      |
*	|		1. External symbol directory  |
*	|		2. Text file		      |
*	|_____________________________________________|
*	|________ARCHIVE_FILE_MEMBER_2________________|
*	|		"ar_hdr"		      |
*	|.............................................|
*	|	Member Contents (.o or text file)     |
*	|_____________________________________________|
*	|	.		.		.     |
*	|	.		.		.     |
*	|	.		.		.     |
*	|_____________________________________________|
*	|________ARCHIVE_FILE_MEMBER_n________________|
*	|		"ar_hdr"		      |
*	|.............................................|
*	|		Member Contents 	      |
*	|_____________________________________________|
*
*/

#define old_ARMAG	"!<arch>\n"
#define old_SARMAG	8
#define old_ARFMAG	"`\n"

struct ar_hdr		/* archive file member header - printable ascii */
{
	char	ar_name[16];	/* file member name - `/' terminated */
	char	ar_date[12];	/* file member date - decimal */
	char	ar_uid[6];	/* file member user id - decimal */
	char	ar_gid[6];	/* file member group id - decimal */
	char	ar_mode[8];	/* file member mode - octal */
	char	ar_size[10];	/* file member size - decimal */
	char	ar_fmag[2];	/* ARFMAG - string to end header */
};

		struct ar_hdr	ar;
		char	armag[old_SARMAG];
		ulong	incr;
		ulong	memberoffset,member_name_length;
		int	count;

		member_name_length = strlen(member);
		(void)fp_lseek(fp, 0, SEEK_SET);
		if (FP_READ(fp, armag, old_SARMAG, 0, UIO_SYSSPACE, &count) ||
				(count != old_SARMAG))
			return 0;
		if (0 != memcmp(armag,old_ARMAG,8) ) return 0;
		memberoffset = 8;
		while(1) {
			(void)fp_lseek(fp,memberoffset,SEEK_SET);
			if (FP_READ(fp, &ar, sizeof ar, 0, UIO_SYSSPACE,
				    &count) ||
					(count != sizeof ar))
				return 0;
			if ( (ar.ar_name[member_name_length] == '/' )  &&
			   (memcmp(member,ar.ar_name,member_name_length) == 0))
				 break;
			incr = atoi(ar.ar_size);
			incr += incr & 1;	/* round up to even number */
			memberoffset += sizeof ar + incr;
		}
   		memberoffset += sizeof ar;
   		return memberoffset;
}


/*
 * NAME: newar
 *
 * FUNCTION: Find the offset of a given member in an archive file
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 *           The member table looks as follows: (offset to member table
 *           is fl_memoff)
 *
 *                    fl_hdr
 *                    +------------------------------------+
 *                    |                 +---------------+  |
 *                    |	                | fl_memoff     |-------------+
 *                    |                 +---------------+  |          |
 *                    +------------------------------------+          |
 *                                                                    |
 *                                                                    |
 *                                                                    |
 *             +------------------------------------------------------+
 *             |
 *             |          MEMBER TABLE STRUCTURE
 *             |
 *             |      +------------------------------------+
 *             +----->| ar.hdr                             |
 *                    +------------------------------------+
 *                    | number of members (n)              |
 *                    +------------------------------------+
 *                    | offset to member 1 (12 characters) |
 *                    +------------------------------------+
 *                    | offset to member 2 (12 characters) |
 *                    +------------------------------------+
 *                    | offset to member 3 (12 characters) |
 *                    +------------------------------------+
 *                           .......
 *                           .......
 *                    +------------------------------------+
 *                    | offset to member n (12 characters) |
 *                    +------------------------------------+
 *                    | name of member 1 (ends with a null)|
 *                    +------------------------------------+
 *                    | name of member 2 (ends with a null)|
 *                    +------------------------------------+
 *                    | name of member 3 (ends with a null)|
 *                    +------------------------------------+
 *                           .......
 *                           .......
 *                    +------------------------------------+
 *                    | name of member n (ends with a null)|
 *                    +------------------------------------+
 *
 * RECOVERY OPERATION:
 *	     None
 *
 * DATA STRUCTURES:
 *	     None
 *
 * RETURNS:
 *           0 - member not found in archive file
 *           offset to member in archive file.
 */

static ulong
newar(
struct sharedstuff *sspointer,
struct file	*fp,
char		*member)
{
                #define FIELD_LEN_MEMBER_ENTRY 12
		

		struct  fl_hdr	fl_hdr;
		struct  ar_hdr	ar_hdr;
		struct  member_offset_entry {
		       char offset_ascii[FIELD_LEN_MEMBER_ENTRY];
                };
                struct  member_offset_entry *entry_offset_addr;

		ulong	memberoffset,member_name_length,tableoffset;
		ulong	size;
		char	*table;            /* local copy of member table */
		char	*mnames;           /* pointer to member name     */
		char	*start_mnames;     /* pointer to  start of 'member
					      names' section in member
					      table                      */
		uint	nummem;            /* number of members          */
		ulong	rc = 0;
		uint    len;
		int	count;             /* count from fp_read()	 */

		member_name_length = strlen(member);
		fp_lseek(fp,0,SEEK_SET);

		(void)FP_READ(fp, &fl_hdr, sizeof fl_hdr, 0, UIO_SYSSPACE,
			      &count);
		if (count != sizeof fl_hdr)
		    return 0;
		if (0 != memcmp(fl_hdr.fl_magic,AIAMAG,sizeof fl_hdr.fl_magic))
		    return 0;
		
		/* get to member table */
		if ( ! (tableoffset = atoi(fl_hdr.fl_memoff)) )
		   return 0;


		/*
		 * read in ar.hdr to get the size of the member table
		 * and read in member table. (see NOTES section for a
		 * description).
		 */

		(void)fp_lseek(fp,tableoffset,SEEK_SET);
		(void)FP_READ(fp, &ar_hdr, sizeof ar_hdr, 0, UIO_SYSSPACE,
			      &count);
		if (count != sizeof ar_hdr)
		    return 0;
		size = atoi(ar_hdr.ar_size);

		if (! (table=xmalloc(size+1,0,kernel_heap)) ) return 0;
		
		(void)FP_READ(fp, table, size, 0, UIO_SYSSPACE, &count);
		if (count != size) goto errexit;

		/*
		 * since each member name ends with a 'null' (i.e. 0),	
		 * setting the last byte to null will guarrantee a
		 * name match.
		 */
		((char *)table)[size] = 0;	
		memberoffset = 0;
		nummem = atoi(table);         /* set number of members    */

		/*
		 * get the starting addresses of member offset entries, and
		 * member name entries.
		 */
		entry_offset_addr = (struct member_offset_entry *)
	                        (table + sizeof(struct member_offset_entry));
		start_mnames =  ((char *) entry_offset_addr) +
				  (nummem * sizeof(struct member_offset_entry));

                /*
		 * now need to find the member name in the 'member names'
		 * section and its relative offset in the ar file.
		 *
		 * NOTE: When entry_offset_addr  = start_mames then all the
		 *       member names have been processed.
		 *
		 *       When the offset for a member is null, that
		 *       member is not in the archive file.
		 *
		 * If start_mnames is is too small, because nummen was absurd,
		 * the loop below terminates at once and no harm is done.
		 * The check for mnames protects against start_mnames too big
		 * and not enough strings in the name part of the table.
		 */
                for(mnames = start_mnames;
		   (char *) entry_offset_addr < start_mnames && mnames < table+size;
		    mnames += len + 1) {
		        len = strlen(mnames);
		        if ( len == member_name_length &&
		             0 == memcmp(member,mnames,len) )  {
			     memberoffset =
			            atoi(entry_offset_addr->offset_ascii);
			     break;
		         }
			 entry_offset_addr++;
		}

		
		/*
		 * if the member was found, then go ahead and read the
		 * ar.hdr for that member.
		 */
		if(memberoffset) {
			(void)fp_lseek(fp,memberoffset,SEEK_SET);
			(void)FP_READ(fp, &ar_hdr, sizeof ar_hdr, 0,
				      UIO_SYSSPACE, &count);
			if (count != sizeof ar_hdr)
				goto errexit;
			len = atoi(ar_hdr.ar_namlen);
			len = len + (len & 1);	/* round up to even */
			memberoffset = memberoffset + sizeof ar_hdr + len;
		}

		rc = memberoffset;
errexit:
		xmfree(table,kernel_heap);
		return rc;
}

/* this routine reads text into memory.  It is used whenever the text cannot be mapped */
int
ld_textread(
struct sharedstuff *sspointer,
struct loader_entry_extension	*lexpointer,
struct file	*fp,
char	*member,
char   *origin,
uint	length,
heapaddr_t	heap,
uint	flags)
{
        uint	hdrsize,startofrest,whdrsize;
        uint	lix,tix,dix;	/* must be unsigned - see comments below */
        int	rc;
        int     i,j,k;
        uint 	bssroom,filesize;
        uint	memberoffset;
        struct 	myxcoffhdr	temphdr;
	char	*loader_section;

	lex.heap = heap;

	/* if this is an archive, find the right member */
	if (member && *member) {
		struct ar_hdr	ar;
		char	armag[SAIAMAG];
		ulong	incr;

		(void)fp_lseek(fp,0,SEEK_SET);
		(void)FP_READ(fp, armag, SAIAMAG, 0, UIO_SYSSPACE, &rc);
		if (rc != SAIAMAG )
			memberoffset = 0;
		else if (0 != memcmp(armag,AIAMAG,8) )
			memberoffset = oldar(sspointer,fp,member);
		else
			memberoffset = newar(sspointer,fp,member);
		if (!memberoffset) {
			if (ss.type != 'L')
				ld_emess(L_ERROR_MEMBER,member,NULL);
			return ENOENT;
		}
	}
	else memberoffset = 0;

	lex.h = &temphdr;  	/* N.B. this so hdr. names the local storage */
	/* read in the file header.*/

	(void)fp_lseek(fp,memberoffset,SEEK_SET);
	/* read the headers into local storage */
	(void)FP_READ(fp, &temphdr, sizeof hdr, 0, UIO_SYSSPACE, &rc);
	hdrsize = sizeof(struct filehdr) + hdr.f.f_opthdr
                  + hdr.f.f_nscns*sizeof(struct scnhdr);
	if ( rc < hdrsize || hdr.f.f_opthdr > sizeof(struct aouthdr))
		return ENOEXEC;

	if (hdr.f.f_opthdr != sizeof(struct aouthdr)){
	/*
	 * the following code contains a major KLUDGE.  We needed to make the
	 * aouthdr larger.  In the transition, we wanted old modules to work.
	 * The approach is to supress mapping them and then fix up here.
	 * The fix up involves modifying the header so it appears to be in the
	 * new format.  This moves the section tables and everything past away from
 	 * the header to leave space.  This code only works for modules whose
 	 * header is too small.  It should be left in in case we need to do this
 	 * again.  There is essentially no cost in the normal path
 	 * DBX may have trouble with this.
 	 *
 	 * hdrsize is size of header in the file.
 	 * sizeof hdr is size of current header
 	 * startofrest is the offset in memory of the rest of the data.  This
 	 * equals hdrsize in the normal case, is larger otherwise.
 	 */
 		uint	aoutdelta,restdelta;
 		uint	i,j,k;
 		char	*p;
 		aoutdelta = sizeof(struct aouthdr) - hdr.f.f_opthdr;
 		whdrsize = hdrsize + aoutdelta;
 		/* first relocate the section headers this aoutdelta down */
 		j = sizeof(struct filehdr) + hdr.f.f_opthdr;
 		p = (char *)&temphdr;
 		for(i=hdrsize-1;i>=j;p[i+aoutdelta]=p[i],i--);
 		for(i=j;i<j+aoutdelta;p[i++]=0);
 		hdr.f.f_opthdr = sizeof(struct aouthdr);

		/* now compute startofrest.  It is end of relocated sections
		 * rounded up match original alignment */
		
		i = MAX(hdr.a.o_algntext,hdr.a.o_algndata);
		i = (1<<i)-1;
		restdelta = (aoutdelta+i)&(~i);
		startofrest = hdrsize + restdelta;
		for(i=0;i<hdr.f.f_nscns;i++){
		/* adjust filepointers in sections because of motion */
			hdr.s[i].s_scnptr += restdelta;
		}
 		
 	
	} /*end compatability KLUDGE*/
	else {
		startofrest = whdrsize = hdrsize;
	}

	/* In an attempt to save I/O we compute the maximum needed number
	 * of bytes. The idea is to avoid reading in most of the "section 4"
	 * information
	 */

        /* you may be concered that f_nscns is garbage.  But above, we read
	 * in enough bytes to cover all the sections it asserts.  If it is
	 * really crazy, the read would have failed!
	 *   (N.B. f_nscsn is unsigned)
         * Because ix is unsigned, this catches zero, which becomes "-1"
         */
        if ((tix=hdr.a.o_sntext-1) >= hdr.f.f_nscns ) return ENOEXEC;
        lex.filesize = hdr.s[tix].s_size + hdr.s[tix].s_scnptr;
        if ((dix=hdr.a.o_sndata-1) >= hdr.f.f_nscns ) return ENOEXEC;
        lex.filesize = MAX(lex.filesize,hdr.s[dix].s_size +
			   hdr.s[dix].s_scnptr);
        if ((lix=hdr.a.o_snloader-1) >= hdr.f.f_nscns ) return ENOEXEC;
	if (ss.type != 'K') 
		lex.filesize = MAX(lex.filesize,hdr.s[lix].s_size +
				   hdr.s[lix].s_scnptr);
	else {
		/*
		 * For kernel extensions don't count
		 * the loader section as part of the file.
		 * Instead check that the loader section 
		 * is the last section in the file.
		 */
		if (lix < tix || lix < dix)
			goto errenoexec;
	}

	/*
	 * The text and data sections must be aligned in the file
	 * according to the alignment information in the header.
	 * Enforcing the alignment from the first fit allocator
	 * is reduced to allocate memory aligned to the max of
	 * the text and data aligments.
	 * Formal proof:
	 *	align_text = 2 ^ r
	 *	align_data = 2 ^ s
	 *	align_text >= align_data 
	 *		(otherwise exchange text/data everywhere)
	 *		==>> align_text = align_data * t
	 *		(trivial, t = 2 ^ (r - s))
	 *	off_text = align_text . k 	(aligned in file)
	 *	off_data = align_data . l	(aligned in file)
	 *	off_mem  = align_text . m	(mem aligned to align_text)
	 *	off_mem + off_text = align_text . m + align_text . k =
	 *		align_text ( m + k )
	 *		==>> off_mem + off_text is aligned to align_text
	 *	off_mem + off_data = align_text . m + align_data . l =
	 *		align_data . t . m + align_data . l =
	 *		align_data ( t . m + l )
	 *		==>> off_mem + off_data is aligned to align_data
	 * XXX
	 * XXX The check for data alignment below is not correct.  First,
	 * XXX for user loads (exec, load, shared objects), the data gets
	 * XXX copied.  The file alignment only affects whether or not the
	 * XXX data can be map copied.  For kernel extensions, strictly
	 * XXX speaking, the check is correct, however; for binary 
	 * XXX compatibility, we must continue to load them incorrectly
	 * XXX at the very least.  The proper solution would be to allocate
	 * XXX the data in a separate chunk with the required alignment.
	 * XXX Text alignment is checked in ld_sanity.
	 * XXX
	if (((1 << hdr.a.o_algndata) - 1) & hdr.s[dix].s_scnptr ||
	    ((1 << hdr.a.o_algntext) - 1) & hdr.s[tix].s_scnptr)
		goto errenoexec;
	 */

        filesize = lex.filesize;
       	if ( flags & LD_textreadbss ){
		bssroom=((ulong)hdr.a.o_bsize+3)&(~3);
		lex.filesize = lex.filesize + bssroom;
	}

        {
	uint	needed;	
	needed = lex.filesize;
	if (flags & LD_allocd){
		if(!(lex.h = (void *)ld_allocd(sspointer,needed)))
			return ENOMEM;
		if ((ulong)ss.la->sbreak < (ulong)lex.h+needed)
			if (BRK((ulong)lex.h+needed)) return ENOMEM;
		ss.la->minbreak=(char*)MAX((ulong)ss.la->minbreak,(ulong)lex.h+needed);
	}
        else if (origin){
        	if (length < needed)
			return ENOMEM;
		lex.h = (struct myxcoffhdr *)origin;
	}
        else {
		lex.h = NULL;

		/*
		 * For small kernel modules use a first
		 * fit allocator to avoid fragmentation.
		 */
		if (ss.type == 'K' && needed <= firstfitmax) {
#if 0
			/*
			 * Modules that will be pinned allocate their
			 * memory from the pinned_fflist the others get
			 * it from the unpinned_fflist.
			 * This code uses the sticky bit as a hint that
			 * indicates that the extension will probably be
			 * pinned.
			 */
			struct stat	stat;
			if (FP_FSTAT(fp, &stat, sizeof(stat), FP_SYS) == -1)
				return ENOEXEC;
			if (lex.h = (struct myxcoffhdr *)
			    ff_alloc(stat.st_mode & ISVTX ?
				&pinned_fflist : &unpinned_fflist, needed,
				MAX(temphdr.a.o_algndata,
				    temphdr.a.o_algntext)))
					lex.flags |= TEXTFFALLOCED;
#else
			/*
			 * Assume that all will be pinned until a
			 * place for the hint is decided.
			 */
			if (lex.h = (struct myxcoffhdr *)
			    ff_alloc(&pinned_fflist, needed,
				MAX(temphdr.a.o_algndata,
				    temphdr.a.o_algntext)))
					lex.flags |= TEXTFFALLOCED;
#endif
		}

		/* If the file is local,  and going into the shared library
		 * segment,  we will try to map it.  The only thing
		 * preventing this would be improper alignment.
		 */
		if (!lex.h && (flags & LD_localfile)) {
			/* First check the alignment */
			unsigned data_offset, text_offset;
			unsigned data_align, text_align;
			text_offset = memberoffset + temphdr.s[tix].s_scnptr
				- temphdr.s[tix].s_vaddr;
			text_align = (1 << temphdr.a.o_algntext) - 1;
			data_offset = memberoffset + temphdr.s[dix].s_scnptr
				- temphdr.s[dix].s_vaddr;
			data_align = (1 << temphdr.a.o_algndata) - 1;
			if (((text_offset & text_align) == 0) &&
			((data_offset & data_align) == 0)) {
			  /* File can be mapped,  allocate storage from heap,
			   * this may require additional storage.
			   */
				size_t size=PAGEUP(memberoffset+lex.filesize)
					- PAGEDOWN(memberoffset);
				if (!(lex.h = (struct myxcoffhdr *)
				  xmalloc(size, PGSHIFT, heap)))
					return ENOMEM;

				/* Now map the file into the shared segment */
				if (ld_filemap(sspointer, lex.h, size, fp,
				  PAGEDOWN(memberoffset))) {
					/* if we are unable to map,  fall
					 * through and attempt to read.
					 */
					xmfree(lex.h, heap);
					lex.h = NULL;
				}
				else {  /* set flag so we know text is
					 * already in file
					 */
					lex.flags |= (TEXTMAPPED|TEXTMALLOCED);
					lex.h = (struct myxcoffhdr *)
					   ((unsigned)lex.h + (memberoffset-
					   PAGEDOWN(memberoffset)));
				}
			} /* if aligned */
		}

		/*
		 * If not a kernel module or a large kernel module or
		 * the first fit allocator failed, then use xmalloc.
		 */
		if (!lex.h) {
			if (!(lex.h = (struct myxcoffhdr *)
			    xmalloc(needed,PGSHIFT,heap)))
				return ENOMEM;
			lex.flags |= TEXTMALLOCED;
		}

		if (ss.type == 'K') {
			/*
			 * The loader section memory is not allocated as part
			 * of the memory allocated for the rest of the file.
			 * The memory used for the loader section comes
			 * from the kernel_heap so that it is pageable
			 * but still available to the loader.
			 * There is code that assumes that the loader section
			 * gets allocated contiguously with the rest of
			 * the file. That code gets to the loader section
			 * by adding the section pointer in the header to
			 * the address of the rest of the file. That code
			 * continues to work by forcing the loader section
			 * pointer in the header to be such that when added
			 * to the memory of the file a pointer to the memory
			 * of the loader section is reached.
			 */

			/*
			 * Allocate memory for the loader section from
			 * the first fit unpinned pool if possible
			 */
			if ((temphdr.s[lix].s_size <= firstfitmax)
			  && (loader_section = (char *)
			  ff_alloc(&unpinned_fflist,
			  temphdr.s[lix].s_size, (int)2))) {
				lex.flags |= LOADERFFALLOCED;
			}
			else {	/* ffalloc won't work,  try xmalloc */
			  if (loader_section = (char *)
			    xmalloc(temphdr.s[lix].s_size,
			    PGSHIFT, kernel_heap)) {
				lex.flags |= LOADERMALLOCED;
			  }
			  else {
				goto errenoexec;
			  }
			}

			/*
			 * Read the loader section.
			 */
			(void) fp_lseek(fp, temphdr.s[lix].s_scnptr, SEEK_SET);
			(void) FP_READ(fp, loader_section,
					temphdr.s[lix].s_size,
					0, UIO_SYSSPACE, &rc);
			if (rc != temphdr.s[lix].s_size)
				goto errenoexec;
			(void) fp_lseek(fp, memberoffset + sizeof hdr,
					SEEK_SET);

			/*
			 * Fix the offset to the loader section
			 */
			temphdr.s[lix].s_scnptr =
				loader_section - (char *) lex.h;
		}

	}
	} /*end of needed scope*/
	
	/* No need to read rest of file or copy header if file was mapped. */
	if (!(lex.flags & TEXTMAPPED)) {
	  /* copy header from temporary area - it may have been modified in
	   * KLUDGE mode
	   */
	  bcopy(&temphdr,lex.h,whdrsize);
	  (void)fp_lseek(fp,memberoffset+hdrsize,SEEK_SET);

	  if (flags & LD_textreadbss) {
		/* leave a hole between the data section and anything
		 * that follows it
		 */
		uint dataend;
		/* N.B. lex.h was reassigned just above - but we haven't
		 * read into it yet.  Take care if KLUDGE is operating.
		 * dataend is the offset is the "fixed up file image", not
		 * in the file.  By subtracting startofrest we get the right
		 * length even in this case.   (in the normal case,
		 * hdrsize==startofrest)
		 */
		dataend=temphdr.s[dix].s_size+temphdr.s[dix].s_scnptr;
		(void)FP_READ(fp,(char *)lex.h+startofrest,dataend-startofrest,
				0, UIO_SYSSPACE, &rc);
		if (rc != (dataend-startofrest)) goto errenoexec;
		/* in KLUDGE case, following two uses of dataend are relative
		 * to the remapped memory image, not the original file image
		 */
		bzero((char *)lex.h+dataend,bssroom);

		/*
		 * Read other sections that need to be after the bss.
		 * For regular executables the sections are in this
		 * order:
		 *	[pad] text [pad] data bss [pad] loader
		 * For those executables this code only reads in:
		 *	[pad] loader
		 * There could be bizare executables with this order:
		 *	[pad] data bss [pad] text [pad] loader
		 * for those, this code would read:
		 *	[pad] text [pad] loader
		 * In any case the secton pointers are adjusted to account
		 * for the space allocated for the bss.
		 * For kernel extensions the loader section was read above
		 * into a different piece of memory and filesize as been
		 * adjusted so that this code does not read the loader section.
		 */
		(void)FP_READ(fp, (caddr_t)lex.h+dataend+bssroom,
			      filesize-dataend, 0, UIO_SYSSPACE, &rc);
		if (rc != filesize-dataend) goto errenoexec;
		/* section offset fix ups - must fix up real header,
		 * not our copy
		 */
		if (hdr.s[tix].s_scnptr >= dataend)
		     hdr.s[tix].s_scnptr+=bssroom;
		if (ss.type != 'K') {
			if (hdr.s[lix].s_scnptr >= dataend)
			     hdr.s[lix].s_scnptr+=bssroom;
		}
	  }

          else {
		(void)FP_READ(fp,(char *)lex.h+startofrest,
			      lex.filesize-startofrest, 0, UIO_SYSSPACE, &rc);
	 	if (rc != (lex.filesize-startofrest))
		     goto errenoexec;
	  }

#ifdef	_POWER
	  vm_cflush(lex.h,lex.filesize);
#endif
	} /* if file was mapped */
	return 0;

errenoexec:
	/* Note that we will NEVER get here with the TEXTMAPPED flag set */
	if (lex.flags & TEXTMALLOCED)
		xmfree(lex.h,heap);
	if (lex.flags & TEXTFFALLOCED)
		ff_free (lex.h);
	if (lex.flags & LOADERFFALLOCED)
		ff_free (loader_section);
	if (lex.flags & LOADERMALLOCED)
		xmfree (loader_section, kernel_heap);
	lex.flags &= ~(TEXTFFALLOCED | LOADERMALLOCED | LOADERFFALLOCED);

	lex.h = NULL;	/* just so no-one trys to free it again!*/
	return ENOEXEC;
}

/* sanity initializes values in lex and validity checks the module so that
 * subsequent code can just use values without checking
 * N.B. this code only assumes that textorg and filesize are set
 * this appears to repeat checks already made in textread -
 * but if the text is mapped, the normal case, these checks must be done
 * values set are:
 *
 * lex.h
 * lex.ldhdr
 * lex.ldsym
 * lex.ldrel
 * lex.textreloc
 * lex.nimpid
 *
 * N.B. sanity assumes that lex.h and lex.filesize are correctly set on entry!
 *	If sanity fails and TEXTMALLOCED is set in lex.flags, it frees the
 *      memory in which the text was allocated.
 */

int
ld_sanity(
struct sharedstuff *sspointer,
struct loader_entry_extension	*lexpointer)
{
        int     i,j,k;
        int	rc;
	char	*ldrscn = (char *) lex.h + hdr.s[hdr.a.o_snloader-1].s_scnptr;
	char	*ldrscnend;
	ulong	ldrscnsz;
	extern	int vm_release();

#if   	defined(_POWER)
	if ( hdr.f.f_magic != U802TOCMAGIC  ) goto errenoexec;
#elif	defined(_IBMRT)
	if ( hdr.f.f_magic != U800TOCMAGIC  ) goto errenoexec;
#else	
	/* only compile this if you haven't added your machine to the list!*/
	goto errenoexec;
#endif

        if (hdr.f.f_opthdr != sizeof(struct aouthdr)) goto errenoexec;


        lex.f_nscns = hdr.f.f_nscns;

        /*
	 * N.B. because text etc. are unsigned, this check catches zero
	 * for free
	 */
        if ((lex.text = hdr.a.o_sntext-1) >= lex.f_nscns) goto errenoexec ;
        if ((lex.data = hdr.a.o_sndata-1) >= lex.f_nscns) goto errenoexec ;
        if ((lex.bss = hdr.a.o_snbss-1) >= lex.f_nscns) goto errenoexec;
        if ((lex.loader = hdr.a.o_snloader-1) >= lex.f_nscns) goto errenoexec;

        if ((hdr.s[lex.text].s_scnptr + hdr.s[lex.text].s_size) > lex.filesize)
	     goto errenoexec;
        if ((hdr.s[lex.data].s_scnptr + hdr.s[lex.data].s_size) > lex.filesize)
	     goto errenoexec;
        /*
	 * N.B. don't repeat this "check" for bss - bss is not a real
	 * section in the file
	 */
#if 0
	/* XXX this check commented out to avoid breaking the build
	 * XXX the code will go back in when the shared stuff is passed
	 * XXX in from all ld_sanity callers. this is done in this way
	 * XXX to prevent lots of re-merge on an unrelated defect that
	 * XXX had lots of files locked at this time.
	 * XXX THIS COMMENT SHOULD GO AWAY WHEN THE CODE GOES BACK IN
	 */
	if (ss.type != 'K' &&
	    hdr.s[lex.loader].s_scnptr + hdr.s[lex.loader].s_size >
	    lex.filesize)
		goto errenoexec;
#endif
	ldrscnsz = hdr.s[lex.loader].s_size;
	ldrscnend = ldrscn + ldrscnsz;

	if ((hdr.a.dsize != hdr.s[lex.data].s_size)) goto errenoexec;
	if ((hdr.a.bsize != hdr.s[lex.bss].s_size)) goto errenoexec;
	if ((hdr.a.tsize != hdr.s[lex.text].s_size)) goto errenoexec;

	lex.ldhdr = (struct ldhdr*)((char *)lex.h+hdr.s[lex.loader].s_scnptr);
        lex.ldsym = (struct ldsym*)&(lex.ldhdr[1]);

	/*
	 * Validate that the number of symbols will not couse us to
	 * step memory beyond the memory allocated for the loader
	 * section.
	 */
	if ((char *) &lex.ldsym[lex.ldhdr->l_nsyms] < ldrscn ||
	    (char *) &lex.ldsym[lex.ldhdr->l_nsyms] > ldrscnend)
		goto errenoexec;

        lex.ldrel = (struct ldrel*)&lex.ldsym[lex.ldhdr->l_nsyms];

	/*
	 * Validate that the number of relocation entries will not
	 * couse us to step memory beyond the memory allocated for
	 * the loader section.
	 */
	if ((char *) &lex.ldrel[lex.ldhdr->l_nreloc] < ldrscn ||
	    (char *) &lex.ldrel[lex.ldhdr->l_nreloc] > ldrscnend)
		goto errenoexec;

	if ( lex.ldhdr->l_impoff >= ldrscnsz ||
	     lex.ldhdr->l_impoff + lex.ldhdr->l_istlen > ldrscnsz ||
	     lex.ldhdr->l_stoff >= ldrscnsz ||
	     lex.ldhdr->l_stoff + lex.ldhdr->l_stlen > ldrscnsz)
		 goto errenoexec;

        /* compute the text segment relocation.  Note that even though the
	 * text has been relocated, if it is in fact position independent
	 * there will be no RLD's pointing to it.  However, textreloc may be
         * applied to data section values, particularly in the TOC.
         * Relocation factor is computed as the file location in memory
	 * (textorg) plus the text location in the file (s_scnptr) minus
	 * the location to which the text was relocated (s_vaddr)
         */

	lex.textloc = (char *)lex.h + hdr.s[lex.text].s_scnptr;
        lex.textreloc = (uint)lex.textloc - hdr.s[lex.text].s_vaddr;

	/* since we never move the text again, it must have the same alignment
	 * in the file as it requires.  We check this now.
	 * algntext is definied as the power of two to which the alignment
	 * of vaddr must be respected.  Iff textreloc is zero mod 2**aligntext
	 * will this be true
	 */
	{
		ulong	align;
		align = (1 << hdr.a.o_algntext) - 1;
		if (lex.textreloc & align) {
			ld_emess(L_ERROR_ALIGN,NULL,NULL);
			goto errenoexec;
		}
	}

        if ( ! lex.ldhdr->l_nimpid) goto errenoexec;

	/* count in header includes the path name string - so we reduce
	 * it by 1
	 */
	lex.nimpid = lex.ldhdr->l_nimpid - 1;

        return 0;

errenoexec:
	if (lex.flags & TEXTMAPPED) {
		VM_RELEASE(PAGEDOWN(lex.h), PAGEUP((unsigned)lex.h +
			lex.filesize) - PAGEDOWN(lex.h));
		lex.h = PAGEDOWN(lex.h);	/* will be xmfree'ed below */
	}
	if (lex.flags & TEXTMALLOCED)
		xmfree(lex.h,lex.heap);   	/* lex.heap set in textread */
	if (lex.flags & TEXTFFALLOCED)
		ff_free ((void *) lex.h);
	if (lex.flags & LOADERFFALLOCED)
		ff_free (ldrscn);
	if (lex.flags & LOADERMALLOCED)
		xmfree (ldrscn, kernel_heap);
	lex.flags &= ~(TEXTFFALLOCED | LOADERMALLOCED | LOADERFFALLOCED |
			TEXTMAPPED);

	lex.h = NULL;	/* just so no-one trys to free it again!*/
	return ENOEXEC;
}

/* loop through the list of loader entries for this load freeing load time only
 * data structures
 */
#define free_if_is(p) if ( p != NULL ) xmfree(p,kernel_heap)
void
ld_freelex(
struct loader_entry *firstle,
struct loader_entry *lastle)
{
	struct loader_entry *le;
	struct loader_entry_extension *lexpointer;
	for(le=firstle;le!=lastle;le=le->le_next){
		le->le_flags &= ~(LE_THISLOAD | LE_NOTTHISLOAD);
		if (lexpointer = le->le_lex) {
			free_if_is(lex.locs);
			free_if_is(lex.impid);
			if (XMALLOC&lex.flags) xmfree(lexpointer,kernel_heap);
			le->le_lex = NULL;
		}
	}
}

/*initialize the path structures corresponding to the already loaded
 *modules so new requests for those names will get the same answer
 */
ld_pathinit(
struct sharedstuff	*sspointer)
{
	struct loader_entry	*le;
	char	*filename;
	struct loader_domain *ld;
	struct domain_entry *de;
	struct domain_altname *da;

	for(le=ss.la->la_loadlist;le;le=le->le_next){
		if (le->le_flags & LE_UNLOAD) continue;
		if (!le->le_fp) continue;
		if (!(filename = le->le_filename)) continue;
		if (le != ss.la->la_execle)
			for(;*filename++;);	/*skip member name*/
		if ('/' == *filename){
			/* absolute path name - so put it into the lookaside*/
			ld_pathopen(sspointer,filename,NULL,le->le_fp);
		}
	}

	/*
	 * if process has specified a loader domain,  prime the pathanme
	 * lookaside with path names in the loader domain.
	 */
	if (ld = ss.ld) {
		for (de = ld->ld_entries; de; de = de->de_next) {
			if (!(filename = de->de_fullname))
				continue;

			if ('/' == *filename) {
				ld_pathopen(sspointer, filename, NULL,
				   (de->de_le)->le_fp);
			}

			/*
			 * also initialize pathname structures for all
			 * alternate names this file is known by.
			 */
			for(da = de->de_altname; da; da = da->da_next) {
			   if (!(filename = da->da_fullname))
				continue;

			   if ('/' == *filename) {
				ld_pathopen(sspointer, filename, NULL,
				   (de->de_le)->le_fp);
			   }
			}

		}
	}
}

/*deallocate a pathname lookaside and closes the file
 *(of course, file will really stay open if it has been loaded
 *since load did an fp_hold when it copied the fp into the loader_entry
 *Also clean up other sharedstuff allocated storage.
 */
void
ld_pathclear(
struct  sharedstuff	*sspointer)
{
	struct	pathlook	*p,*np;
	if (ss.d){
		xmfree(ss.d,kernel_heap);
		ss.d = NULL;
	}
	for(p=ss.pl;p;){
		np=p->next;
		if (p->type == 'F' && p->val)
			FP_CLOSE(p->val);
		xmfree(p,kernel_heap);
		p=np;
	}
	ss.pl = NULL;

	/* clean up any allocated libpath string */
	if (ss.flags & SS_LIBPATH_ALLOCED) {
		if (ss.libpath)
			xmfree(ss.libpath, kernel_heap);
		if (ss.libpath_saved)
			xmfree(ss.libpath_saved, kernel_heap);
		ss.flags &= ~SS_LIBPATH_ALLOCED;
	}
	ss.libpath = NULL;
	ss.libpath_saved = NULL;
}

/*
 * clean up and toss ld_reopen array
 */
void
ld_clean_reopen(
struct  sharedstuff	*sspointer,
struct ld_reopen *ldr)
{
	struct ld_reopen *t_ldr;
	int cleanup = 0;
	int had_lock = 1;
	int waslocked;


	/*
	 * acquire global loader lock if necessary
	 */
	waslocked = lockl(&kernel_anchor.la_lock,LOCK_SHORT);
		
	/*
	 * first decrement use counts
	 */
	t_ldr = ldr;
	while (t_ldr->ldr_de) {
		(t_ldr->ldr_de)->de_usecount--;
		if (!(t_ldr->ldr_de)->de_usecount)
			cleanup++;
		((t_ldr->ldr_de)->de_le)->le_usecount--;
		t_ldr++;
	}

	/*
	 * if necessary clean up zero use count entries in the domain
	 */
	if (cleanup)
		ld_clean_domain(ss.ld);

	/*
	 * release global loader lock and close files
	 */
	unlockl(&kernel_anchor.la_lock);
	t_ldr = ldr;
	while (t_ldr->ldr_de) {
		if (t_ldr->ldr_fp)
			FP_CLOSE(t_ldr->ldr_fp);
		t_ldr++;
	}

	/*
	 * reacquire lock if necessary
	 */
	if (waslocked == LOCK_NEST)
		lockl(&kernel_anchor.la_lock,LOCK_SHORT);

	/*
	 * release virtual space for array
	 */
	xmfree(ldr, kernel_heap);
}

/*
 * NAME:        ld_domain_reopen(sspointer)
 *
 * FUNCTION:    This routine is designed to reopen all pathnames contained
 *		in a loader domain.  It will build an array of ld_reopen
 *		structures that correspond to the state of the loader domain
 *		at the time of the call.  Since files will be opened in this
 *		routine,  the global loader lock must be released.  When the
 *		global loader lock is released,  the state of the loader
 *		domain can change.  Therefore,  we must be very careful to
 *		get an array of reopen structures that match the state of
 *		the loader domain.
 *
 *		This routine will increnet use counts in both domain entries
 *		and loader entries,  that will not be cleaned up until a call
 *		to ld_clean_reopen() is made at the end of the pre-pass.
 * 		
 * PARAMETERS:  sspointer - shared stuff(global data)
 *
 * RETURN:      pointer to array of ld_reopen structures(or possibly NULL).
 *
 */
static struct ld_reopen *
ld_domain_reopen(
struct sharedstuff *sspointer)
{
	int before_count, after_count, cleanup;
	struct domain_entry *de;
	struct ld_reopen *ldr, *t_ldr;

	/*
	 * if reopen array already exists,  then determine if it matches
	 * the current state of the loader domain.  note that because
	 * use counts are incremented for the duration of the pre-pass
	 * entries can not be deleted from the domain.
	 */
	if (ldr = ss.ldr) {
		t_ldr = ldr;
		de = ss.ld->ld_entries;
		while(t_ldr->ldr_de && de) {
			t_ldr++;
			de = de->de_next;
		}

		/*
		 * no match,  toss the current reopen array
		 */
		if (t_ldr->ldr_de || de) {
			ld_clean_reopen(sspointer, ldr);
			ldr = ss.ldr = NULL;
		}

		/*
		 * if there is a match,  ldr will remain set and we
		 * will bypass the loop below and simply return ldr.
		 */
	}
		
	/*
	 * keep trying until we get a vaild reopen array
	 */
	while (!ldr) {
		/*
		 * count the entries in loader domain
		 */
		before_count = 0;
		for(de = ss.ld->ld_entries; de; de = de->de_next)
			before_count++;
		if (!before_count)
			return(NULL);

		/*
		 * allocate space for ld_reopen array
		 */
		ldr = (struct ld_reopen *)
			xmalloc((before_count+1) * sizeof(struct ld_reopen),
			0, kernel_heap);
		if (!ldr)
			return(NULL);

		/*
		 * initialize ld_reopen array,  and increment use counts in
		 * domain entries and loader entries to prevent their removal.
		 */
		t_ldr = ldr;
		for(de = ss.ld->ld_entries; de; de = de->de_next) {
			t_ldr->ldr_de = de;
			de->de_usecount++;
			(de->de_le)->le_usecount++;
			t_ldr++;
		}
		t_ldr->ldr_de = NULL;

		/*
		 * release lock,  and reopen all files in the domain
		 */
		unlockl(&kernel_anchor.la_lock);
		t_ldr = ldr;
		while (t_ldr->ldr_de) {
			if (FP_OPEN((t_ldr->ldr_de)->de_fullname, O_RDONLY,
			    0, 0, FP_SYS, &(t_ldr->ldr_fp)))
				t_ldr->ldr_fp = NULL;
			t_ldr++;
		}

		/*
		 * reacquire lock,  and count entries in the loader domain
		 * to see if anything new was added.  we know nothing could
		 * have been deleted.
		 */
		lockl(&kernel_anchor.la_lock,LOCK_SHORT);
		after_count = 0;
		for(de = ss.ld->ld_entries; de; de = de->de_next)
			after_count++;

		/*
		 * if after_count != before_count,  then we know the domain
		 * changed when the lock was released.  therefore,  the array
		 * of reopen structures we have is invalid.  throw them away
		 * and try again.
		 */
		if (before_count != after_count) {
			ld_clean_reopen(sspointer, ldr);
			ldr = NULL;
		}
	}

	return(ldr);
}


/*
 * create a domain_altname structure for the passed pathname.  space
 * for this structure will be allocated from the shared library data
 * segment.
 * return - pointer to newly created domain_altname structure
 */
static struct domain_altname *
ld_create_dom_altname(char *pathname)
{
	struct domain_altname *da;

	da = xmalloc(sizeof(struct domain_altname) + strlen(pathname) + 1,
		2, lib.la.la_data_heap);

	if (da) {
		da->da_fullname = (char *)da + sizeof(struct domain_altname);
		strcpy(da->da_fullname, pathname);
	}

	return da;
}

/*
 * NAME:        ld_domain_open(sspointer, pathname, fp)
 *
 * FUNCTION:    This routine is designed to search a loader domain
 *		for a match based on the pathname and fp parameters.
 *		This routine should only be called from ld_pathopen(),
 *		AFTER fp_open() is called to open the file,  and the
 *		process has specified a loader domain.  At this point
 *		it is known there was no match in the pathanme lookaside
 *		structures.
 *
 *		This routine will first search for a match based on
 *		the fp by doing gnode comparisons.  If no match is
 *		found this way,  then all files in the domain are
 *		"reopen'ed".  A match is then searched for by doing
 *		gnode comparisons on the reopen'ed fps.  If a match
 *		is found in either case,  then an entry is added to
 *		the alternate name list for the domain entry.
 *
 * PARAMETERS:  sspointer - shared stuff(global data)
 *              pathname  - full pathname of library file
 *		fp	  - open file pointer to library file
 *
 * RETURN:      fp -	the original passed fp is returned if a match
 *			is found based on the gnode comparisons,  OR
 *			no match was found
 *		fp -	the fp from the matched loader entry in the
 *			domain is returned if a match was found.
 */
static struct file *
ld_domain_open(
struct sharedstuff *sspointer, 
char *pathname, 
struct file *fp)
{
	struct loader_domain *ld;
	struct domain_entry *de;
	struct domain_altname *da, *tmp_da;
	struct ld_reopen *ldr;
	uint fh;

	fh = FPtoFH(fp);

	/*
	 * first search the domain for a match based on gnode number
	 */
	ld = ss.ld;
	de = ld->ld_entries;
	while (de) {
		if (FPtoFH((de->de_le)->le_fp) == fh) {
			/*
			 * gnodes match.  this means the file was
			 * opened by another name(otherwise the
			 * lookaside mechanism would have found
			 * the match).  create an alternate name
			 * structure for this domain entry.
			 */
			da = ld_create_dom_altname(pathname);
			if (!da)
				return(fp);

			/* link new structure into chain */
			da->da_next = de->de_altname;
			de->de_altname = da;

			/* return passed fp */
			return fp;
		}
		de = de->de_next;
        }

	/*
	 * no gnode match found
	 * reopen all files in the domain,  ld_domain_reopen will build
	 * such an array
	 */
	ss.ldr = ld_domain_reopen(sspointer);

	/*
	 * now search this reopen array for a gnode match
	 */
	ldr = ss.ldr;
	if (ldr) {
	  while(ldr->ldr_de) {
		if (FPtoFH(ldr->ldr_fp) == fh) {
			/*
			 * gnodes match.
			 * create altname structure as above
			 */
			da = ld_create_dom_altname(pathname);
			if (!da)
				return(fp);

			/* link new structure into chain */
			da->da_next = (ldr->ldr_de)->de_altname;
			(ldr->ldr_de)->de_altname = da;

			/*
			 * return fp from domain
			 *
			 * close the file pointer that was passed in.  be
			 * sure to keep the open counts correct.  the global
			 * loader lock must be released before making the
			 * filesystem calls.
			 */
			unlockl(&kernel_anchor.la_lock);
			FP_CLOSE(fp);
			fp_hold(((ldr->ldr_de)->de_le)->le_fp);
			lockl(&kernel_anchor.la_lock,LOCK_SHORT);

			fp = ((ldr->ldr_de)->de_le)->le_fp;
			return fp;
		}
		ldr++;
	  }
	}

	/* no match found,  return original fp */
	return(fp);
}

struct file *
ld_pathopen(
struct	sharedstuff	*sspointer,
char	*path,
char	*filename,
struct	file	*initfp)
{
	uint	hash, mode;
	uint	lpath,ltemp,lfilename;
	struct	pathlook	*p;
	struct	pathlook	*relative_p = NULL;
	char	*x,*y,*z;
	struct	file	*fp;
	char	*realfilename;
	extern int ps_not_defined;
	extern char *kern_getcwd();

	lpath = lfilename = 1<<28;
	/* to pass a simple pathname, pass it in path, with filename NULL*/
	if (!filename) {
		filename = path;
		path = "";
		lpath = 0;
		hash = 0;
	}
	
	/* filename points to two successive null terminated strings
	 * the first is everything except the basename with the seperating
	 * slash removed - the second is the basename
	 * Thus, if the first string is non-null we use it as the path,
	 * else we use libpath
	 */
	else if (*filename){
		path = filename;
		hash = ld_hash(filename,&lpath);
		filename += lpath+1;
	}
	else {
		hash = ld_hash(path,&lpath);
		filename += 1;
	}
	hash ^= ld_hash(filename,&lfilename); /* ^ is bitwise exclusive or */
	for(p=ss.pl;p;p=p->next){
		if (hash != p->hash) continue;
		if (p->type != 'F') continue;
		if ((lpath + lfilename) != p->length) continue;
		if (lpath && memcmp(&(p->name[0]),path,lpath)) continue;
		if (memcmp(&(p->name[lpath]),filename,lfilename)) continue;
		u.u_error = 0;
		return p->val;
	}
	p = xmalloc(sizeof(struct pathlook) +
	    2 * (lpath + lfilename)+1,0,kernel_heap);
	assert(p);	/* if we can't get this much system is dead*/
	
	/* if path points to null string we really open - otherwise
	 * call pathopen recursively so we remember the base pathname as
	 * well as the
	 * libpath / basename pair */
	x=path;
	realfilename = &(p->name[lpath+lfilename]);
	while(1) {
		char last='/';
		z=realfilename;
		for(;*x && *x != ':';last=*z++=*x++); /* scan for null or : */
		if('/' != last) *z++ = '/';      /* needed for case of '/' */
		strcpy(z,filename);
		if (0 != lpath){
			/* for now initfp is always NULL on this path*/
			fp=ld_pathopen(sspointer,realfilename,NULL,initfp);
			if (fp) {
				fp_hold(fp);
				*realfilename = '\0';
			}
		}

		else if(initfp){
			/* this occurs during a load command when we
			 * "prime" the lookaside with the paths
			 * which are already known*/
			fp_hold(initfp);
			fp=initfp;
		}
		else {
		/* N.B. library load files need only have read permission
		 * O_RSHARE requests that no writer is allowed.
		 * before paging space is defined the libraries are 
		 * opened so that they can be truncated to conserve 
		 * space. only those libraries which stay in use or 
		 * will be never needed afterwards can be truncated.
		 */

	/*		ASSERT(ss.pre);		*/
			unlockl(&kernel_anchor.la_lock);
			u.u_error = 0;	/* we don't trust fp_open */
			mode = ps_not_defined ? O_RDONLY : O_RDONLY | O_RSHARE;
			u.u_error = FP_OPEN(realfilename,mode,0,0,FP_SYS, &fp);
			if (u.u_error)
				fp = NULL;

			/*
			 * if a loader domain was specified,  and a relative
			 * path name was used,  construct the absolute path
			 * name for the file just opened file.
			 */
			if (fp && ss.ld && *realfilename != '/') {
				char *cwd, *cwd_buf;

				/*
				 * first built a relative pathname lookaside
				 * structure using the parameters passed to
				 * this invocation of the routine.
				 */
				p->hash = hash;
				p->length = lpath + lfilename;
				p->type = 'F';
				if (lpath) memcpy(&(p->name[0]),path,lpath);
				memcpy(&(p->name[lpath]),filename,lfilename);
				p->name[lpath + lfilename] = '\0';

				/*
				 * allocate buffer for cwd string,  and
				 * call routine to determine cwd
				 */
				cwd_buf = xmalloc(PATH_MAX, 0, kernel_heap);
				assert(cwd_buf);
				cwd = kern_getcwd(cwd_buf, PATH_MAX);
				if (cwd) {
					struct pathlook *p2;
					int lcwd;
					/*
					 * fix up local variables to reflect
					 * the absolute full pathname rather
					 * than the relative one.  first
					 * construct the absolute path.
					 */
					lcwd = strlen(cwd);
					p2 = xmalloc(sizeof(struct pathlook) +
						2 * (lcwd + lfilename + 1),
						0, kernel_heap);
					assert(p2);
					realfilename = &(p2->name[lcwd +
						lfilename + 1]);
					memcpy(realfilename, cwd, lcwd);
					memcpy(realfilename + lcwd, "/", 1);
					memcpy(realfilename + lcwd + 1,
					       filename, lfilename + 1);
					/*
					 * recompute hash
					 */
					filename = realfilename;
					lfilename = 1<<28;
					hash = ld_hash(filename, &lfilename);
					/*
					 * p points to pathlook structure with
					 * absolute pathname.  relative_p
					 * points to structure with relative
					 * pathname.
					 */
					relative_p = p;
					p = p2;
				}
				xmfree(cwd_buf, kernel_heap);
			}

			/* this form of lock does not fail */
			lockl(&kernel_anchor.la_lock,LOCK_SHORT);

			/*
			 * if a loader domain was specified,  look for
			 * a match in the loader domain.  note that this
			 * call may change the value of fp if a match is
			 * found.  if ld_domain_open() changes the value of
			 * the fp,  then it will keep open counts correct.
			 */
			if (fp && ss.ld)
				fp = ld_domain_open(sspointer,realfilename,fp);

		}
		if (fp) break;
		if (0==*x) break;
		x++;
	}

	/* only use files - not directories, specials, etc */

	if ((!initfp) && fp) {
		struct stat stst;
				
		if (FP_FSTAT(fp, &stst, sizeof(stst),FP_SYS) == -1 ||
		     ! S_ISREG(stst.st_mode) ){
			/* not a regular file - its a directory
			 * or other bad thing
			 */
			FP_CLOSE(fp);
			u.u_error = EACCES;
			fp = NULL;
		}
	}

	if (fp) {
		/*
		 * if there is a pathname lookaside structure for a relative
		 * pathname,  add it to the list.
		 */
		if (relative_p) {
			fp_hold(fp);	/* so close count is correct */
			relative_p->val = fp;
			relative_p->next = ss.pl;
			ss.pl = relative_p;
		}
		p->next = ss.pl;
		ss.pl = p;
		p->hash = hash;
		p->val = fp;
		p->length = lpath + lfilename;
		p->type = 'F';
		if (lpath) memcpy(&(p->name[0]),path,lpath);
		memcpy(&(p->name[lpath]),filename,lfilename);
	}
	else {
		xmfree(p,kernel_heap);
		if (relative_p)
			xmfree(relative_p,kernel_heap);
	}
	return fp;
}

char *
ld_fptopath(
struct	sharedstuff	*sspointer,
struct	file	*fp)
{
	struct	pathlook	*p;
	for(p=ss.pl;p;p=p->next){
		if (p->type != 'F') continue;
		if (p->val != fp) continue;
		if (!(p->name[p->length])) continue;
		return &(p->name[p->length]);
	}
	return NULL;
}

/*
 * NAME: knlist()
 *
 * FUNCTION: Translate names to addresses in the running system.
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine may page fault
 *
 * NOTES: The returned "nlist" array is consistent per entry, but the entire
 *        array is not guaranteed to be consistent due to possible kernel
 *        updates concurrent with this command.
 *
 * RETURN VALUES:
 *	 0 = successful completion - symbol type & value set if found, else 0
 *      -1 = failed, errno indicates cause of failure
 */

int
knlist(
struct  nlist *nl,		/* pointer to array of nlist struct	*/
int	nelem,			/* number of user nlist struct		*/
int	sizelem)		/* size of expected nlist structure	*/
{
	extern	int	copyout();	/* copies from kernel to user	*/
	extern	int	copyin();	/* copies from user to kernel	*/
	extern	int	fubyte();	/* returns byte from user	*/
	extern	int     priv_chk();	/* performs privilege check     */


	register int ret;		/* remember return code		*/
	struct nlist nltmp;		/* area for temporary nlist	*/
	uint	elem;
#define	MAX_EXT 32
	char symbuf[MAX_EXT+1];	/* temporary symbol storage	*/
	struct exp_index *ex;
	uint	actual;
	int	waslocked;

	ret = 0;
	/* parameter ok and number of symbols is 1 or more	*/
	if (sizelem != sizeof(struct nlist) || nelem < 1)
	{
		u.u_error = EINVAL;
		return(BAD);
	}
	/* while elements repeat search, zero tmp, increment user ptr	*/
	for (elem=0;elem<nelem;elem++){
		/* copyin() n_name pointer and symbol if necessary	*/
		if (copyin(&nl[elem],&nltmp,sizeof nltmp) != GOOD)
		{
	       		u.u_error = EFAULT;
			return(BAD);
		}
		/* while n_name pointer is not NULL & non-NULL n_name	*/
		if (nltmp.n_name == NULL)
			continue;

		/* strcpy routine for symbol name from user to temp	*/
		if (copyinstr(nltmp.n_name,symbuf,sizeof symbuf,&actual))
		{
	       		u.u_error = EFAULT;
			return(BAD);
		}
		/*
		 * Call the xcoff loader routine to look for symbols.
		 * If symbol found, load tmp nl array with values from
		 *	ldsymp structure returned from loader:
		 *	n_value, n_scnum, n_type, n_sclass, n_numaux
		 */
		/* grab loader lock					*/
		waslocked = lockl(&kernel_anchor.la_lock, LOCK_SHORT);
		
		ex = ld_symfind(kernel_exports->le_exports,symbuf);

		if (ex){
			nltmp.n_value = (long)ex->exp_location;
		}
		else {	
			char *stmp = nltmp.n_name;
			u.u_error = EFAULT;
			ret = -1;
			bzero(&nltmp,sizeof nltmp);
			nltmp.n_name = stmp;
		}
			
		if (waslocked != LOCK_NEST)	/* release loader lock	*/
			unlockl(&kernel_anchor.la_lock);

		
		/* protected copy all nlist to user space, checks perm	*/
		if (copyout(&nltmp,&nl[elem], sizelem))
		{
       			u.u_error = EFAULT;
			return(BAD);
		}
	}
	/* successful search, return GOOD value	*/
	return(ret);
}

/* ld_getinfo - Get list of loader entries for the specified process
 * 		
 * Input parameters
 *	pid       - Process id to query or -1 to specify current
 *		    process
 *	length    - Length of callers buffer
 *	ld_info   - Pointer to callers buffer
 *	userspace - only specified on calls to ld_getingo_all
 *		    a nonzero value indicates callers buffer is
 *		    in a processes address space
 * Return Values
 *	0	Success
 *	!0	Failure
 *
 * N.B. it is assumed that either the caller is the process
 * being queried OR that the process being queried is stopped and
 * NOT in the loader.  This is true for ptraced processes when
 * the debugging process is running and the ptrace process is stopped.
 *
 * Note that the real workhorse routine is ld_getinfo_all().  There
 * are two separate paths to this routine.  ld_getinfo is used if the
 * buffer is in the kernel domain.  ld_getinfo_user is used if the
 * buffer is in the user address space.
 */
int ld_getinfo(
pid_t pid,
unsigned int length,
struct ld_info *ld_info)
{
	/* call ld_getinfo_all indicating buffer is in kernel space */
	return(ld_getinfo_all(pid, length, ld_info, 0));
}

int ld_getinfo_user(
pid_t pid,
unsigned int length,
struct ld_info *ld_info)
{
	/* call ld_getinfo_all indicating buffer is in user space */
	return(ld_getinfo_all(pid, length, ld_info, 1));
}

static int
ld_getinfo_all(
pid_t pid,
unsigned int length,
struct ld_info *ld_info,
int userspace)
{
	struct loader_anchor *a;
	struct loader_entry  *le,*execle,*nextle;
	int	sid,libsid;
	int	rc = 0;
	vmhandle_t	srval;
	void	*liborg,*privorg;
	struct	proc	*procp;
	struct	user	*user;
	ulong	privreloc,libreloc,namereloc;
	void	*ldmax;
	char 	*p,*q;
	uint	ldinfo_next;
	char	tmp_buf[sizeof(struct ld_info)+PATH_MAX+PATH_MAX];
	struct	ld_info *tmp_ld_info = (struct ld_info *) tmp_buf;
	uint	tmp_ld_info_length;
	int	ovfl_was_attached;


	/* Establish addressability to the process private segment.
	 * Remember that the process whose load list we are traversing
	 * may not be the current process.
	 */
	privorg = liborg = NULL;
	procp = ((pid == -1) ? curproc : PROCPTR(pid));
	privorg = vm_att(procp->p_adspace,0);
	privreloc = (ulong)privorg - PRIVORG;
	user = PTRADD(&U,privreloc);

	/* make sure this process has the goods - kprocs don't*/
	a=PTRADD(&(U.U_loader[0]),privreloc);
	if (a->la_execle==NULL || a->la_loadlist==NULL){
		rc = EINVAL;
		goto exit;
	}

	/* make global library data segment addressable */
	liborg = vm_att(vm_setkey(library_data_handle,VM_PRIV),0);
	libreloc = (ulong)liborg - SHDATAORG;

	/* Attach to overflow segment if necessary */
	assert((privorg != OVFLORG) && (liborg != OVFLORG));
	ovfl_was_attached = ld_ovflatt(a);

	
	/* ldmax is address of first byte past the end of passed buffer.
	 * Its value does not change.  ldinfo_next is the address in the
	 * user buffer where the next ldinfo structure will be put.
	 */
	ldmax = PTRADD(ld_info,length);
	ldinfo_next = ld_info;

	/* in following, we process execle first - that is how users can
	 * tell it from loaded programs.  We start with le pointing to
	 * the execle. At the bottom of the loop where we update le to
	 * point to the next le,  a special check is made in the case of
	 * the execle so we start at the beginning of the list.  Yuck!!!
	 */
	le = nextle = execle = OVFL_PTRADD(a->la_execle,privreloc);
	while (le) {

		/* Find the next le in the load list.  Note that when
		 * we get here 'le == nextle && nextle != NULL'.
		 */
		do {
			if (nextle->le_next == NULL)
				/* go to beginning of load list */
				nextle = OVFL_PTRADD(a->la_loadlist,
					privreloc);
			else
				nextle = OVFL_PTRADD(nextle->le_next,
					privreloc);

			/* If 'nextle == execle' we know that we have
			 * looked at every entry in the list.  Set
			 * nextle to NULL to indicate the end of the
			 * list
			 */
			if (nextle == execle)
				nextle = NULL;

		/* quit when we have looked at every entry in the list
		 * OR we have found the next entry.  Upon exit of this
		 * loop,  nextle will point to the next valid entry in
		 * the load list(possibly NULL).
		 */
		} while( nextle &&
		   (nextle->le_flags&(LE_DATA|LE_UNLOAD)) != LE_DATA);

		/* Build up temporary structure,  this will be copied to
		 * the buffer specified by the calling routine
		 */
		tmp_ld_info->ldinfo_fp = le->le_fp;
		tmp_ld_info->ldinfo_textorg = le->le_file;
		tmp_ld_info->ldinfo_textsize = le->le_filesize;
		tmp_ld_info->ldinfo_dataorg = le->le_data;
		tmp_ld_info->ldinfo_datasize = le->le_datasize;
		
		/* copy filename/member if it exists
		 * Note that in the loader entry the member name comes
		 * first and is followed by the member name.  In the
		 * ldinfo structure the file name comes first and is
		 * followed by the member name.  Hence,  the following
		 * code must reverse their order.  Yuck!!!
		 */
		p = tmp_ld_info->ldinfo_filename;
		if(le->le_filename){
		  namereloc = ((uint)le->le_filename >= SHDATAORG &&
			(uint)le->le_filename < SHDATAORG+(SEGSIZE-1)) ?
			libreloc: IS_OVFLSEG(le->le_filename) ? 0 : privreloc;
		  q = le->le_filename + namereloc;
		  /* for technical reasons the filename field of the
		   * execd entry points to u_comm. This is a file
		   * name, with no member. Fix up hear so interface
		   * outside is uniform. (Sorry about that - MAA).
		   */
		  if (le != execle)
			/* skip past member name, it comes first here */
			for(;*q++;);

		  /* copy file name */
		  while (*p++ = *q++);

		  /* now copy member name */
		  if (le == execle) {
			/* remember - execd has no member */
			*p++ = 0;
		  }
		  else{
			q = le->le_filename + namereloc;
			/* copy member name */
			while (*p++ = *q++);
		  }/*end member exists*/
		}/*end le_filename exists*/
		else {
			/* If no file name put out two zero length
			 * strings
			 */
			*p++ = 0;
			*p++ = 0;
		}

		/* Check to see if this structure will fit in the
		 * callers buffer
		 */
		tmp_ld_info_length = (uint)p - (uint)tmp_ld_info;
		if ((ldinfo_next + tmp_ld_info_length) >= ldmax) {
			rc = ENOMEM;
			goto exit;
		}

		/* Determine if there is another entry in list */
		if (nextle) {
			/* Calculate position of next entry in users
			 *  list.  Round up to a word boundary.
			 */
			p = (void *)(((uint)p+sizeof(int)-1) & 
				~(sizeof(int)-1));
			/* The current entry will point to next */
			tmp_ld_info->ldinfo_next =
				(uint)p - (uint) tmp_ld_info;
		}
		else {
			/* This is the last entry in the list */
			tmp_ld_info->ldinfo_next = 0;
		}

		/* Copy the temporary structure to the buffer
		 * specified by the caller
		 */
		if (userspace) {
			if (rc = copyout(tmp_ld_info, ldinfo_next,
			  tmp_ld_info_length))
				goto exit;
		}
		else {
			bcopy (tmp_ld_info, ldinfo_next,
			  tmp_ld_info_length);
		}

		/* Update ldinfo_next pointer for next loop iteration
		 * Grab size of the current structure out of the entry
		 * we just completed(this includes the necessary
		 * alignment)
		 */
		ldinfo_next += tmp_ld_info->ldinfo_next;

		/* process next entry */
		le = nextle;
	}

exit:
	if (ovfl_was_attached) ld_ovfldet(a);
	if (privorg) vm_det(privorg);
	if (liborg) vm_det(liborg);
        return rc;
}

/* chain an error message on the error chain.  The storage ALWAYS
 * comes from the user process kernel area.  If this is an execload
 * the errors will be passed to /etc/execerror.  If this is a load,
 * the user can call loadquery with the errorget option to retrieve the
 * messages */
void
ld_emess(
int	errorid,
char	*errordata,
struct	loader_entry	*le)
{
	struct sharedstuff *sspointer,sharedstuff;
	struct emess *e,*ep;
	int	mlen,flen,elen;
	char	fbuf[128],ebuf[32];
	char	*mp;
	int	ovfl_was_attached;
	
	/* set up a sufficient ss so that ualloc will work.  These call
	 * must always manipulate the process anchor and memory.  It may
	 * be called while the "real" sharedstuff names process, kernel, or
	 * library */
	sspointer = &sharedstuff;
	bzero(&sharedstuff,sizeof(sharedstuff));
	ss.type = 'C';
	ss.la = (struct loader_anchor *)(U.U_loader) ;

	/* Attach overflow segment if needed */
	ovfl_was_attached = ld_ovflatt(ss.la);

	sprintf(ebuf,"%d",errorid);
	elen=strlen(ebuf);
	/* if an le is passed, the file name associated with is is added to
	 * to the message */
	if (le){
		char	*file,*member,*t;
		file = le->le_filename;
		member = file;
		if (file && (le->le_flags & LE_EXECLE))
			member = "\0";
		else if (file)
			for(;*file++;);
		else
			file = member = "?";
		t = fbuf;
		for(;(*t++ = *file++) && t < fbuf+sizeof fbuf - 2;);
		if(*member) {
			*(t-1) = ' ';
			for(;(*t++ = *member++) && t < fbuf+sizeof fbuf - 1;);
		}	
		else
			*(t-1) = 0;
		flen = t-fbuf;
	}
	else flen=0;
	mlen = strlen(errordata);
	e = ld_ualloc(sspointer,elen+flen+mlen + sizeof(struct emess));
	if (!e) goto exit;
	memcpy(e->errordata,ebuf,elen);
	mp = e->errordata+elen;
	*mp++ = ' ';
	memcpy(mp,errordata,mlen+1);
	mp+=mlen;	/* points to the null which was last char copied*/
	if (flen){
		*mp++ = ' ';
		memcpy(mp,fbuf,flen); /* N.B. flen includes the null at end of fbuf */
	}
	e->next = NULL;
	if ( ss.la->la_emess ) {
		int count=100;
		for(ep=ss.la->la_emess;count-- && ep->next;ep=ep->next);
		if (count)
			ep->next = e;
		else  {
			sprintf(ep->errordata,"%d",L_ERROR_TOOMANY);
			ld_ufree(sspointer,e);
		}
	}
	else
		ss.la->la_emess = e;

exit:
	/* detach from overflow segment if necessary */
	if (ovfl_was_attached)
		ld_ovfldet(ss.la);
}

void
ld_emessinit()
{
	struct sharedstuff *sspointer,sharedstuff;
	struct emess *e,*ep;
	int	mlen;
	int	ovfl_was_attached;
	
	/* set up a sufficient ss so that ualloc will work.  These call
	 * must always manipulate the process anchor and memory.  It may
	 * be called while the "real" sharedstuff names process, kernel, or
	 * library */
	sspointer = &sharedstuff;
	bzero(&sharedstuff,sizeof(sharedstuff));
	ss.type = 'C';
	ss.la = (struct loader_anchor *)(U.U_loader) ;

	/* Attach to overflow heap if necessary */
	ovfl_was_attached = ld_ovflatt(ss.la);

	while (e = ss.la->la_emess) {
		ss.la->la_emess = e->next;
		ld_ufree(sspointer,e);
	}

	if (ovfl_was_attached)
		ld_ovfldet(ss.la);
}

int
ld_emessdump(
	char	*buf,
	int	blen,
	int	adspace)
{
	struct loader_anchor *la;
	struct emess *e;
	char	*pp,*mp;
	int	n, mlen;
	int	rc=0;
	int	ovfl_was_attached;

	if (blen < sizeof(void *))
		return ENOMEM;
	
	/* The head of the list of error messages is in the loader anchor */
	la = (struct loader_anchor *)(U.U_loader) ;

	/* attach to overflow segment if necessary */
	ovfl_was_attached = ld_ovflatt(la);

	/* count the messages */
	n = 0;
	for(e = la->la_emess;e;e=e->next)
		n++;
	
	/* For each error message:
	 *	- provide a pointer to the string within the buffer itself
	 *	- copy the error message string to the buffer
	 */
	pp = buf;
	mp = buf + sizeof(void *)*(n+1);
	for(e=la->la_emess;e;e=e->next){
		mlen = strlen(e->errordata)+1;
		if(mp+mlen > buf+blen){
			rc = ENOMEM;
			goto exit;
		}

		/* First copy the pointer to string */
		if (adspace==SYS_ADSPACE)
			bcopy(&mp,pp,sizeof(void*));
		else {
			if (rc = copyout(&mp,pp,sizeof(void *)))
				goto exit;
		}

		/* Then copy the string itself */
		pp += sizeof(void *);
		if (adspace==SYS_ADSPACE)
			bcopy(e->errordata,mp,mlen);
		else {
			if (rc = copyout(e->errordata,mp,mlen))
				goto exit;
		}

		mp += mlen;
	}

	/* Terminate the list of message pointers with a NULL pointer */
	mp = NULL;
	if (adspace == SYS_ADSPACE)
		bcopy(&mp,pp,sizeof(void *));
	else
		rc = copyout(&mp,pp,sizeof(void *));

exit:
	if (ovfl_was_attached)
		ld_ovfldet(la);
	return rc;
}

/*
 * NAME:        char *kern_getcwd(char *buf, int buf_size)
 *
 * FUNCTION:    This routine is designed to get the path name of the
 *		current directory.  It is identical in function to the
 *		library routine getcwd().  However,  kern_getcwd() is
 * 		designed to run in the kernel environment.
 *
 * PARAMETERS:  buf      - Points to a buffer that will contain the path
 *			   name.  This string must be addressable(in the
 *			   kernel).
 *              buf_size - Size(in bytes) of the buffer.
 *
 * RETURN:      pointer path name of current directory,  or NULL in case
 *		of an error.
 *
static char *
 */
char *
kern_getcwd(char *buf, int buf_size)
{
	struct file *cur_fp;
	struct file *par_fp;
	ulong_t cur_vfs;
	ino_t cur_ino;
	ulong_t root_vfs;
	ino_t root_ino;
	struct stat s;
	char curdir[PATH_MAX];
	char *curptr;
	char *pnptr;
	char *pbufptr;
	struct vattr vat;
	struct vattr *vattrp = &vat;
	struct ucred *crp;


	/*
	 * initialize path pointers.  note that pnptr points to the next
	 * character to be written.  it does not point to a valid
	 * character.  
	 */
	if (!buf || buf_size < 2)
		return(NULL);
	pnptr = buf + buf_size - 1;
	*pnptr-- = '\0';
	pbufptr = buf;
	curptr = curdir;

	/*
	 * get information about root directory,  so we know where to stop
	 */
	crp = crref();
	if (VNOP_GETATTR(rootdir, vattrp, crp))
		return(NULL);		/* unable to stat */
	crfree(crp);
	root_vfs = rootdir->v_vfsp->vfs_number;
	root_ino = vat.va_serialno;

	/*
	 * get information about current directory
	 */
	strcpy(curptr, "./");
	curptr += 2;
	if (fp_open(curdir, O_RDONLY, 0, 0, FP_SYS, &cur_fp))
		return(NULL);
	if (fp_fstat(cur_fp, &s, sizeof(s), FP_SYS)) {
		fp_close(cur_fp);
		return(NULL);                      /* unable to stat */
	}
	cur_vfs = s.st_vfs;
	cur_ino = s.st_ino;

	/*
	 * loop until we reach the root directory
	 */
	while (cur_vfs != root_vfs || cur_ino != root_ino) {
		/*
		 * open parent directory
		 */
		if (((curptr + 3) - curdir)  > PATH_MAX) {
			fp_close(cur_fp);
			return(NULL);
		}
		strcpy(curptr, "../");
		curptr += 3;
		if (fp_open(curdir, O_RDONLY, 0, 0, FP_SYS, &par_fp)) {
			fp_close(cur_fp);
			return(NULL);
		}
		if (fp_fstat(par_fp, &s, sizeof(s), FP_SYS) ||
		    !S_ISDIR(s.st_mode)) {
			fp_close(cur_fp);
			fp_close(par_fp);
			return(NULL);		/* unable to stat or not dir */
		}

		/*
		 * search for name associated with curdir in parent directory
		 */
	 	if (find_dir_name(par_fp, cur_fp, cur_vfs, cur_ino,
		                  &pnptr, &pbufptr)) {
			fp_close(cur_fp);
			fp_close(par_fp);
			return(NULL);
		}

		/*
		 * current directory is now parent directory
		 */
		cur_vfs = s.st_vfs;
		cur_ino = s.st_ino;
		fp_close(cur_fp);
		cur_fp = par_fp;
	}

	fp_close(cur_fp);
	pnptr++;
	return(pnptr);
}

/*
 * search for the name associated with the current directory in the parent
 * directory.  this is done by reading the directory entries in the parent
 * directory,  while looking for an inode match.  when a match is found the
 * name in the direcotry entry is used.
static
 */
find_dir_name(
	struct file *dir_fp,
	struct file *cur_fp,
	ulong_t cur_vfs,
	ino_t cur_ino,
	char **pnptr,
	char **pbufptr)
{
	struct vnode *dir_vp;
	ulong_t dir_vfs;
	struct iovec aiov;
	struct uio auio;
	char dir_buf[PAGESIZE];
	struct dirent *dirp;
	int found = 0;
	int bytes_read;
	int rc;

	/*
	 * determine if we are crossing a vfs
	 */
	dir_vp = dir_fp->f_vnode;
	if ((dir_vp->v_vfsp)->vfs_number != cur_vfs) {
		struct vnode *mnt_vp;
	        struct vattr vat;
		struct vattr *vattrp = &vat;

		/*
		 * if crossing a vfs,  get the inode of the 'mounted over'
		 * vnode.  this is what we want to match below.
		 */
		if (!((cur_fp->f_vnode)->v_flag & V_ROOT))
			return(-1);
		mnt_vp = ((cur_fp->f_vnode)->v_vfsp)->vfs_mntdover;

		if ((mnt_vp->v_vfsp)->vfs_number !=
		    (dir_vp->v_vfsp)->vfs_number)
			return(-1);

		if (VNOP_GETATTR(mnt_vp, vattrp, dir_fp->f_cred))
			return(-1);

		cur_ino = vat.va_serialno;
	}

	/*
	 * read directory entries block by block,  until the directory
	 * name is found or we reach the end of the directory.
	 */
	do {
		/*
		 * contruct iovec structure
		 */
		aiov.iov_base = dir_buf;
		aiov.iov_len = PAGESIZE;
		auio.uio_iov = &aiov;
		auio.uio_iovcnt = 1;
		auio.uio_fmode = dir_fp->f_flag & FMASK;
		auio.uio_segflg = UIO_SYSSPACE;
		auio.uio_resid = PAGESIZE;
		auio.uio_offset = dir_fp->f_offset;

		/*
		 * read a block of directory entries
		 */
		if (VNOP_READDIR(dir_vp, &auio, dir_fp->f_cred) == 0)
			dir_fp->f_offset = auio.uio_offset;
		else
			break;

		/*
		 * check for end of file/directory
		 */
		if (!(bytes_read = PAGESIZE - auio.uio_resid))
			break;

		/*
		 * we now scan the directory entries just read for the
		 * name of the file associated with the passed current
		 * directory.
		 */
		dirp = (struct dirent *)dir_buf;
		while (dirp < (struct dirent *)(dir_buf + bytes_read)) {
			if (dirp->d_ino == cur_ino) {
				/* found match */
				found++;
				break;
			}
			dirp = (struct dirent *)
				((char *)dirp + dirp->d_reclen);
		}

		
	} while (!found);

	if (found)
		rc = prepend(dirp->d_name, dirp->d_namlen, pnptr, pbufptr);
	else
		rc = -1;

	return(rc);
}

/*
 * prepend the directory name to the existing path name
static
 */
prepend(char *node, int node_len, char **pnptr, char **pbufptr)
{
	if ((*pnptr - node_len - 1) < *pbufptr)
		return(-1);

	node += (node_len - 1);

	while(node_len) {
		*(*pnptr)-- = *node--;
		node_len--;
	}

	*(*pnptr)-- = '/';

	return(0);
}
