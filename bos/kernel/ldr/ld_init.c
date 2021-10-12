static char sccsid[] = "@(#)27	1.28.1.2  src/bos/kernel/ldr/ld_init.c, sysldr, bos411, 9428A410j 2/11/94 15:57:33";
/*
 * COMPONENT_NAME: (SYSLDR) Program Management
 *
 * FUNCTIONS: init_ldr()
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
#include	<sys/ldr.h>
#include	<sys/xcoff.h>
#include	<sys/malloc.h>
#include	<sys/lockl.h>
#include	<sys/user.h>
#include	<sys/errno.h>
#include	<sys/seg.h>
#include	<sys/syspest.h>
#include	<sys/vmuser.h>
#include	"ld_data.h"


/* initialize the loader structures for the kernel
 * this code must build that, including the hash table for the kernel
 * if must also construct the svc's including a proper loader section
 * and hash table
 */

/* top of kernel load module is copied here by start before relocating kernel
 * this preserved the xcoff loader header smax is wild overestimate of # of
 * sections.
 * The build defines loader_sect_start - but this may be unreliable since it is
 * really the last byte of the TOC.
 */

struct myxcoffhdr kernel_header = {0};  /* init so its in data - must
					   not be in common */


long kernel_header_length=sizeof(kernel_header);

#define	filehdr	kernel_header.f
#define	aouthdr	kernel_header.a
#define	scnhdr	kernel_header.s



void
init_ldr()
{
    int			   i,j,k;
    uint		   numexports,numsvcs;
    char        *text,*data;	/*sections of kernel load module*/
    struct ldhdr	   *ldhdr;
    struct ldsym	   *ldsym;
    int			   khead_length;
    struct locs            *locs;
    struct loader_entry    *le;
    struct svc_table_entry *svc;

    /* create the initial shared library segment
     * which will be propagated by fork to all processes
     * eventually we may need a mechanism for created a new shared
     * library segment but for now, there is just one
     */

    if (ld_libinit()) panic("shared library init failed");

    data = (char *)scnhdr[aouthdr.o_sndata-1].s_vaddr;
    /* following computes address of loader section IN MEMORY
     * the loader section scnptr is the offset of the loader
     *   section in the module
     * the module has been shifted in memory (see low.s) so that the
     *   text section starts at 0.
     * by subtracting the module offset of the text section from the module
     *   offset of the loader section, we thus get the correct address
     * N.B. do not make the mistake of thinking the text section immediately
     *      follows the module header in the module - it may not!
     * N.B. do not make the mistake of thinking the loader section
     *      immediately follows the data - that is is co-incident with
     *      the first word of common - this is not guaranteed
     * N.B. the loader section is in "common".  We place a large (1 meg)
     *      variable in common which is never used exactly to prevent
     *      real common being on top of the loader section.  The loader
     *      section is used AFTER initialization!
     */

    ldhdr = (struct ldhdr*)(scnhdr[aouthdr.o_snloader-1].s_scnptr
                            - scnhdr[aouthdr.o_sntext-1].s_scnptr);
    ldsym = (struct ldsym*)(&ldhdr[1]);
    text = (char *)0;
    j = sizeof(struct locs)*(3+ldhdr->l_nsyms);
    locs = xmalloc(j,2,kernel_heap);
    /* since kernel is fully relocated, locs will be uniformly zero*/
    numexports = numsvcs = 0;

    /* Initialize kernel load list anchor */
    kernel_anchor.la_lock = LOCK_AVAIL;
    kernel_anchor.la_text_heap = kernel_heap;
    kernel_anchor.la_data_heap = kernel_heap;
    kernel_anchor.la_flags |= (LA_TEXT_HEAP | LA_DATA_HEAP);

    /* construct loader entry and hash for kernel segment*/
    for(i=0;i<ldhdr->l_nsyms;i++) {
        locs[i+3].d = 0; 	/*assume all kernel is fully relocated*/
        locs[i+3].v = 1;
        if ( ldsym[i].l_smtype &L_EXPORT ) numexports++;
        if ( ldsym[i].l_smtype &L_EXPORT && ldsym[i].l_smclas == XMC_SV )
	        numsvcs++;
    }

    le = kernel_anchor.la_loadlist = kernel_exports
       = xmalloc(sizeof(struct loader_entry),2,kernel_heap);
    bzero(le,sizeof(struct loader_entry));
    le->le_usecount=2; /*one because loaded,
			 one because kernel_exports names it*/
    le->le_loadcount = 1;
    le->le_flags=LE_TEXT | LE_KERNEL | LE_KERNELEX | LE_DATA | LE_DATAEXISTS;
    /*
     *N.B. this entry violates otherwise universal rule that
     *     text is contiguous starting with header
     */
    le->le_file = (char *)&kernel_header;
    le->le_exports =
	     ld_buildhash(ldhdr,numexports,locs,L_EXPORT, 0,NULL,kernel_heap,
		NULL);
    le->le_ndepend=0;
    le->le_filename = kernel_filename;

    /* construct loader entry and hash for kernel svcs */
    ASSERT(numsvcs != 0);
    le = kernel_anchor.la_loadlist->le_next = syscall_exports
       = xmalloc(sizeof(struct loader_entry),2,kernel_heap);
    bzero(le,sizeof(struct loader_entry));
    le->le_usecount = 2;
    le->le_loadcount = 1;
    le->le_flags=LE_SYSCALLS | LE_DATA | LE_DATAEXISTS;
    j = sizeof(struct svc_table_entry)*numsvcs;
    /*
     *N.B. use file field to remember table
     */
    svc = svc_table = xmalloc(j,PGSHIFT,kernel_heap);
    vm_protect(svc,j,UTXTKEY);
    le->le_file = (void*)svc;
    le->le_filesize = j;
    le->le_filename = kernel_filename;

    le->le_depend[0] = kernel_exports;
    le->le_ndepend = 1;
    svc_table_entries = numsvcs;
    j = 0;
    for(i = 0;i < ldhdr->l_nsyms; i++)
        if (ldsym[i].l_smtype &L_EXPORT && ldsym[i].l_smclas == XMC_SV ) {
            svc[j].svc0 = svc_instruction;
            svc[j].index = j;
            svc[j].svc = (int (*)())ldsym[i].l_value;
	    /* "relocation factor"*/
            locs[i+3].d = (ulong)&(svc[j].svc0)-ldsym[i].l_value;
            j++;
        }

    /* TEMPORARY - for fetch-protect/store problem.
     * This can be removed when we no longer need to support
     * the h/w level with this problem.
     *
     * Copy the svc table to the user-kernel-shadow segment.
     */
    ukerncopy(svc,le->le_filesize);  

    le->le_exports =
	   ld_buildhash(ldhdr,numexports,locs,L_EXPORT,1,NULL,kernel_heap,
		NULL);
    xmfree(locs,kernel_heap);
}
