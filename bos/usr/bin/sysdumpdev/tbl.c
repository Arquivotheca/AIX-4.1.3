static char sccsid[] = "@(#)99	1.16  src/bos/usr/bin/sysdumpdev/tbl.c, cmddump, bos411, 9428A410j 6/10/91 16:01:42";

/*
 * COMPONENT_NAME: CMDDUMP    system dump control and formatting
 *
 * FUNCTIONS: c_tableinit, da_tableinit
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Fill in the "table of contents" for a dump file.
 * There are two such tables.
 *   1. component_name/starting cdt offset (C_table)
 *   2. data_area_name/starting offset     (ZDa_table)
 *      One per component.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/dump.h>
#include <sys/dasd.h>
#include <sys/hd_config.h>
#include "dmpfmt.h"

#define TBL_INCR 16                     /* realloc increment */

extern char *malloc(), *realloc();

extern checkflg;                        /* exit with error if sync lost */
extern char *Dumpfile;                  /* name of the dump file */
extern int	comp_is_lvm;		/* special processing for lvm */

struct cdt0 Cdt0;                       /* current component dump table */

static struct c_table *C_table;         /* table of cdt headers */
static c_tbl_size;                      /* current size. grows by TBL_INCR */
static c_tbl_idx;                       /* current index */

struct da_table *ZDa_table;       	/* table of data areas */

struct da_table *kern_dt;		/* kernel da_table	    	   */
static da_tbl_size;                     /* current size. grows by TBL_INCR */
static da_tbl_idx;                      /* current index */

static BMoffset;                        /* offset to bitmap */

static struct c_table  *get_ct();       /* ensure space in C_table */
static struct da_table *get_da();       /* ensure space in ZDa_table */

static ce_read();

#ifdef _NO_PROTO
void setupvg();
struct pvol *getpvol();
struct lvol* getlvol();
int getwork_Q();
struct buf *getbpool();
static read_da();
int xread();
int readfrompage();
off_t dump_addr();
int ddistance ();
int getbitmap();
void rdpb();
#else
void setupvg( struct da_table *kdt, struct volgrp *vg );
struct pvol *getpvol( struct da_table *kdt, char *addr );
struct lvol *getlvol(struct da_table *kdt, char *addr);
int getwork_Q( struct da_table *kdt, struct buf **lvworkq, struct buf ***workq);
struct buf *getbpool( struct da_table *kdt, struct buf *bp );
static read_da( struct da_table *da, char *addr, char *buf, int len );
int xread( struct da_table *da, char *addr, char *buf, int len);
int readfrompage( int pagen, char *buf, int len );
off_t dump_addr( struct da_table *da, char *addr);
int ddistance ( struct da_table *da, char *from, char *to );
int getbitmap( struct da_table *da );
void rdpb( struct pbuf *ptr, struct pbuf *buf);
#endif

/*
 * NAME:     c_tableinit
 * FUNCTION: make a table of the names of components and their file offsets
 *           into the dump file
 * INPUTS:   none
 * RETURNS:  head of a table of c_table structures, terminated by
 *           an entry with c_offset = -1
 */
struct c_table *
c_tableinit()
{
    struct c_table *ctp;
    struct cdt0 h;
    int offsetsv;
    int offset;
    int n;
    int i;

    if(C_table) {               /* free previous table */
        free( (char *) C_table);
        C_table = 0;
    }
    offsetsv = jtell();         /* get current offset */
    for(;;) {
        offset = jtell();       /* start of this component dump table */
        if(jread(&h,sizeof(h)) != sizeof(h))
		break;

        if(offset == 0) {
		/* must have a magic number at offset 0 or 512 */
		if(h.cdt_magic != DMP_MAGIC) {
			offsetsv = offset = 512;
			if((jseek(offset) != 0) ||
					(jread(&h,sizeof(h)) != sizeof(h)) ||
						  (h.cdt_magic != DMP_MAGIC)) {
				cat_eprint(CAT_DMPMAGIC,
					"bad magic: %08x offset 0x%x file %s\n",
						h.cdt_magic,offset,Dumpfile);
				exit(1);
			}
		}
	}

        if(h.cdt_magic == 0)
            break;
        if(h.cdt_magic != DMP_MAGIC) {  /* check for loss of sync */
            cat_eprint(CAT_DMPMAGIC,
                "bad magic: %08x offset 0x%x file %s\n",
                h.cdt_magic,offset,Dumpfile);
            exit(1);
        }
        ctp = get_ct();         /* set a c_table entry */
        strncpy(ctp->c_name,h.cdt_name,C_NAMESIZE); /* fill in name */
        ctp->c_offset = offset;                     /* and offset */
        n = NUM_ENTRIES(&h);    /* number of data areas for this component */
        BMoffset = offset + sizeof(h) + n * sizeof(struct cdt_entry);
        for(i = 0; i < n; i++)  /* skip over each one to get to the */
            ce_read(0);         /* next component dump table */
        jseek(BMoffset);
    }
    jseek(offsetsv);            /* reset file pointer */
    ctp = get_ct();             /* "NULL" terminate the c_table list */
    ctp->c_name[0] = '\0';
    ctp->c_offset  = -1;        /* "NULL" is -1 */
    return(C_table);            /* return head of table */
}

/*
 * NAME:     da_tableinit
 * FUNCTION: make a table of the names of data areas and their file offsets
 *           into the dump file
 * INPUTS:   none
 * RETURNS:  head of a table of da_table structures, terminated by
 *           an entry with da_offset = -1
 */
struct da_table *
da_tableinit()
{
    int			offsetsv;
    int			n;
    int			i;
    struct cdt_entry	ce;
    struct da_table	*tp;
    static int		lvm_init_count;
    extern void		lvm_da_tableinit();

    Debug("da_tableinit: offset %x\n",jtell());
    if(ZDa_table && ZDa_table != (struct da_table*)(-1)) {     /* free previous table   */
        free( (char *) ZDa_table);
        ZDa_table = 0;
    }
    offsetsv = jtell();                             /*  save starting offset */
    if(jread(&Cdt0,sizeof(Cdt0)) != sizeof(Cdt0))   /* read cdt header	     */
	goto da_exit;

    if(offsetsv == 0) {
	/* must have a magic number at offset 0 or 512 */
	if(Cdt0.cdt_magic != DMP_MAGIC) {
		offsetsv = 512;
		if((jseek(offsetsv) != 0) ||
				(jread(&Cdt0,sizeof(Cdt0)) != sizeof(Cdt0)))
			goto da_exit;
	}
    }

    if(Cdt0.cdt_magic == 0)
        goto da_exit;
     if(Cdt0.cdt_magic != DMP_MAGIC) {
        cat_eprint(CAT_DMPMAGIC,
            "bad magic: %08x offset 0x%x file %s\n",
            Cdt0.cdt_magic,offsetsv,Dumpfile);
        goto da_exit;
    }
    n = NUM_ENTRIES(&Cdt0);
    BMoffset = offsetsv + sizeof(Cdt0) + n * sizeof(ce);
    bzero(&ce,sizeof(ce));
    for(i = 0; i < n; i++) {
        tp = get_da();
        tp->da_bmoffset = BMoffset;
        tp->da_offset   = tp->da_bmoffset + BITMAPSIZE(ce.d_ptr,ce.d_len);
	ce_read(&ce);	/* sets BMoffset to start of next bitmap */
        strncpy(tp->da_name,ce.d_name,DA_NAMESIZE);
        tp->da_ptr = (int)ce.d_ptr;
        tp->da_len = ce.d_len;
        Debug("da_tableinit: tp=%x bmoffset=%x offset=%x name='%s'\n",
            tp,tp->da_bmoffset,tp->da_offset,ce.d_name);
    }
    jseek(offsetsv);            /* restore file offset */
    tp = get_da();
    tp->da_name[0] = '\0';
    tp->da_offset  = -1;        /* "NULL" terminate */

    /*
     * if this is the LVM component,
     * use the tp's to find the vg's and refill in the component entry table.
     * Also, switch to the kernel dump to use the kernel segment for finding
     * the LVM structures.
     *
     * Set a kernel_segment_offset for use in displaying the lvm structures
     * in the display options.  Also, form a table of lbufs and associated
     * pbufs.
     */ 
    comp_is_lvm = FALSE;	/* lvm_da_tableinit not used */
    if (lvm_init_count++ == 0 && !strcmp(Cdt0.cdt_name, "lvm")) {
	lvm_da_tableinit();
    }
    lvm_init_count--;

da_exit:
    return(ZDa_table);
}

/*
 * NAME:     get_ct
 * FUNCTION: allocate a c_table structure
 * INPUTS:   none
 * RETURNS:  pointer to c_table structure.
 *
 * This routine internally allocates TBL_INCR structures at a time
 * to reduce the number of calls to realloc().
 */
static struct c_table *
get_ct()
{


    if(C_table == 0 || c_tbl_size == 0) {
        c_tbl_size = 0;
        c_tbl_idx  = 0;
        if((C_table = MALLOC(TBL_INCR,struct c_table)) == 0) {
            perror("malloc");
            exit(1);
        }
        c_tbl_size += TBL_INCR;
    } else if(c_tbl_idx == c_tbl_size) {
        c_tbl_size += TBL_INCR;
        if((C_table = REALLOC(C_table,c_tbl_size,struct c_table)) == 0) {
            perror("realloc");
            exit(1);
        }
    }
    memset(&C_table[c_tbl_idx],0,sizeof(C_table[0]));
    return(&C_table[c_tbl_idx++]);
}

/*
 * NAME:     get_da
 * FUNCTION: allocate a da_table structure
 * INPUTS:   none
 * RETURNS:  pointer to da_table structure.
 *
 * This routine internally allocates TBL_INCR structures at a time
 * to reduce the number of calls to realloc().
 */
static struct da_table *
get_da()
{
    if(ZDa_table == 0 || da_tbl_size == 0) {
        da_tbl_size = 0;
        da_tbl_idx  = 0;
        if((ZDa_table = MALLOC(TBL_INCR,struct da_table)) == 0) {
            perror("malloc");
            exit(1);
        }
        da_tbl_size += TBL_INCR;
    } else if(da_tbl_idx == da_tbl_size) {
        da_tbl_size += TBL_INCR;
        if((ZDa_table = REALLOC(ZDa_table,da_tbl_size,struct da_table)) == 0) {
            perror("realloc");
            exit(1);
        }
    }
    memset(&ZDa_table[da_tbl_idx],0,sizeof(ZDa_table[0]));
    return(&ZDa_table[da_tbl_idx++]);
}

/*
 * NAME:     ce_read
 * FUNCTION: read the next component dump entry structure
 * INPUTS:   cep   pointer to dump entry structure to fill
 *           if 0, allocate an internal structure.
 * RETURNS:  none
 *
 * The variable BMoffset is set to just after the last byte in the data area.
 */

static bitmap_t bm[DMP_MAXPAGES / sizeof(bitmap_t)];

static
ce_read(cep0)
struct cdt_entry *cep0;
{
    int i;
    int count;
    int tcount;
    int addr;
    int npages;
    int bitmapsize;
    int offsetsv;
    struct cdt_entry *cep;
    struct cdt_entry ce;

    cep = cep0 ? cep0 : &ce;
    Debug("ce_read: offset=%x\n",jtell());
    jread(cep,sizeof(*cep));
    offsetsv = jtell();
    bitmapsize = BITMAPSIZE(cep->d_ptr,cep->d_len);
    npages     = NPAGES(cep->d_ptr,cep->d_len);
    Debug("ce_read: cep->d_ptr=%x len=%x bmsize=%x npages=%x BMoffset=%x\n",
        cep->d_ptr,cep->d_len,bitmapsize,npages,BMoffset);
    jseek(BMoffset);
    jread(bm,bitmapsize);
    addr  = (int)cep->d_ptr;
    count = cep->d_len;
    BMoffset += bitmapsize;
    Debug("ce_read: offset=%x PAGESIZE=%x bm[0]=%x\n",jtell(),PAGESIZE,bm[0]);
    for(i = 0; i < npages; i++) {
        tcount = MIN(count,PAGESIZE-(unsigned)addr % PAGESIZE);
        Debug("tcount=%x addr=%x\n",tcount,addr);
        if(ISBITMAP(bm,i))
            BMoffset += tcount;
        else
            Debug("PAGE %d not in mem\n",i);
        count -= tcount;
        addr  += tcount;
    }
    Debug("return: BMoffset=%x\n",BMoffset);
    jseek(offsetsv);
}

int lvm_debug;
#define LVM_DEBUG(da)	if	(lvm_debug) {			\
	printf("LVM_DA_SET: da=%x bmoffset=%x offset=%x\n",	\
	      da, da->da_bmoffset, da->da_offset);		\
	printf("\tda_name=%s da_ptr=%x da_len=%x\n",		\
		da->da_name, da->da_ptr, da->da_len); }

/* set component dump table entries for LVM */
#define LVM_DA_SET(name, ptr, len)     da = get_da();			      \
				       strncpy(da->da_name,name,DA_NAMESIZE); \
				       da->da_bmoffset = kern_dt->da_bmoffset;\
				       da->da_offset   = kern_dt->da_offset;  \
    				       da->da_ptr      = ptr;		      \
				       da->da_len      = len;		      \
				       LVM_DEBUG(da)

static struct pbuf	*hd_pbuf;	/* anchor pbuf list for LVM */

/*
 * NAME:     lvm_da_tableinit
 *
 * FUNCTION: make a table of the names of data areas and their file offsets
 *           into the dump file.  This routine takes the lvm dump component
 *	     list of addresses of volgrp structures in the kernel segment
 *	     to construct a dump table of addresses into the kernel segment
 *	     for LVM data structures.  The lvm component table in the dump
 *	     is used to construct this more useful table for lvm dumps
 *	     unless the kernel component, with the kernel segment, was
 *	     not part of the dump.
 * INPUTS:   none
 *
 * RETURNS:  none
 */
void 
lvm_da_tableinit()
{
    int			i, n;
    struct c_table	*c;
    struct cdt_entry	 ce;
    struct da_table	*da;
    struct da_table	*ldt;		/* LVM data table		   */
    struct vglist {			/* vglist as in sysx/lvm/hd_top.c  */
	struct volgrp	*vgp;
	long		major_num;
    }			*vglistp;
    struct volgrp	*vg;		/* LVM volume group		   */
    off_t		 lvmoffsetv;	/* offset to LVM component	   */
    int			nvgs;		/* number of volume groups	   */
    int			lvm_comp_set;
    
    lvm_comp_set = FALSE;
    lvmoffsetv = jtell();		/* save offset to lvm component	   */

    if ((i = c_lookup("bos")) == -1)	/* find "bos" component		   */
	goto lvm_out;			/* kernel segment not in the dump  */
    c = &C_table[i];			/* get bos component dump table	   */
    jseek(c->c_offset);			/* seek to bos component dump table*/

    /* kerrnel segment is in the dump, form the LVM table */
    ldt = ZDa_table;			/* save lvm table		    */

    ZDa_table = 0;			/* da_tableinit would frees ZDa_table*/

    if (kern_dt == NULL) {
	kern_dt = da_tableinit();	/* bos component da_table entry	    */
	ZDa_table = 0;
    }
    if (kern_dt == NULL || strcmp("kernel", kern_dt->da_name))
	goto lvm_out;
    /* !!! i = Da_lookup("kernel") !!! */
    /* !!! kern_dt += i;	   !!! *//* select "kernel" da_table entry   */

    ZDa_table = 0;			/* so get_da allocates new table    */
    /* first entry in the LVM component is the address of the pbuf anchor */
    hd_pbuf = (struct pbuf *)ldt->da_ptr;
    LVM_DA_SET("dmpbuf", (int)hd_pbuf, sizeof(struct pbuf *));
    
    /* get volume group addresses to be looked up in the kernel segment */
    vglistp = (struct vglist *) malloc((ldt+1)->da_len);
    if (vglistp == NULL)
	goto lvm_out;

    if (read_da(ldt+1, (char *)(ldt+1)->da_ptr, (char *)vglistp, (ldt+1)->da_len) <= 0)
	goto lvm_out;

    /* scan array of vg pointers, lookup data in kernel segment */
    nvgs = (ldt+1)->da_len / sizeof(struct vglist);
    for ( ; nvgs > 0; nvgs--, vglistp++) {
	if (vglistp->vgp) {
	    setupvg(kern_dt, vglistp->vgp);
	}
    }

    da = get_da();
    da->da_name[0] = '\0';
    da->da_offset  = -1;		/* "NULL" terminate		*/

    free( (char *) ldt);				/* done with init LVM da_table	*/
    ldt = ZDa_table;
    ZDa_table = 0;
    lvm_comp_set = TRUE;

 lvm_out:
    /* efficiency is no concern here */
    jseek(lvmoffsetv);			/* reset offset to start of lvm */
    da_tableinit();			/* re-initialize Cdt0 to LVM	*/
    if (lvm_comp_set == TRUE)
	comp_is_lvm = TRUE;
    ZDa_table = ldt;			/* set ZDa_table to lvm		*/
    /* no need to reset Cdt0->cdt_len, it will not be used again */
}

/*
 * NAME:     setupvg
 *
 * FUNCTION: make a table of the names of data areas and their file offsets
 *           into the dump file for a volume gruop.
 *
 * INPUTS:   kdt (pointer to the kernel dump table), vg (volume group pointer)
 *
 * RETURNS:  none
 */
void
setupvg( struct da_table *kdt, struct volgrp *vg )
{
    int			 i, n, numpps, numbytes;
    struct da_table	*da;
    struct volgrp	 v;
    struct buf		*b, *pb;
    struct buf	       **workq;
    struct lvol		*lv;
    struct pvol		*pv;

    if (read_da(kdt, (char *)vg, (char *)&v, sizeof(struct volgrp)) 
	== sizeof(struct volgrp)) {
	LVM_DA_SET( "volgrp", (int)vg, sizeof(struct volgrp) );

	vg = &v;
	/* dump the physical volumes in the volume group */
	for (n = 0; n < MAXPVS; ++n) {		/* loop thru pvol structs */
	    if (vg->pvols[n] == NULL)
		continue;
	    pv = getpvol(kdt, (char *)vg->pvols[n]);
	    if (pv && pv->dev != HD_NODEVICE) {
		LVM_DA_SET( "pvol", (int)vg->pvols[n], sizeof(struct pvol) );
		if (pv->defect_tbl)
		    LVM_DA_SET( "dfct_tbl",	/* defect table entry     */
			       (int)pv->defect_tbl,
			       sizeof(struct defect_tbl));
	    }					/* PV dump info set       */
	}					/* end PVs for loop	  */
	
	/* dump the logical volumes in the volume group */
	for (n=0; n < MAXLVS; ++n) {		/* loop thru lvol structs */
	    if (vg->lvols[n]) {
		lv = getlvol(kdt, (char *)vg->lvols[n]); /* dump the LV */
		if (lv == NULL)
		    continue;
		LVM_DA_SET( "lvol", (int)vg->lvols[n], sizeof(struct lvol) );

		/* dump work_Q and buffers */
		if (lv->work_Q) {
		    LVM_DA_SET( "work_Q", (int)lv->work_Q,
			       (sizeof(struct buf *)) * WORKQ_SIZE);
		
		    if (getwork_Q(kdt, lv->work_Q, &workq) > 0)
			for (i = 0; i < WORKQ_SIZE; i++) {
			    if (pb = workq[i]) {
				LVM_DA_SET("workqbuf", (int)pb,
					   sizeof(struct buf));
				b = getbpool(kdt, pb);
				for (; b && b->av_back;) {
				     LVM_DA_SET("workqbuf", (int)b->av_back,
						sizeof(struct buf));
				     b = getbpool(kdt, b->av_back);
				}
			    }
			}
		}				/* end the work buf for loop */
	    
		/* dump the partitions for the mirrors */
		for (i = 0; i < 3; i++) {	/* up to 3 PPs per LP */
		    if (lv->parts[i]) {
		        numpps = BLK2PART(vg->partshift, lv->nblocks);
		        numbytes = numpps * sizeof(struct part);
		        LVM_DA_SET("parts",  (int)lv->parts[i], numbytes);
		    }
		}
	    }
	}					/* end LVs for loop	  */
    }
}

struct pvol *
getpvol( struct da_table *kdt, char *addr )
{
    static struct pvol pv;
    int	len = sizeof(struct pvol);
    
    return( read_da( kdt, addr, (char *)&pv, len ) == len ? &pv : NULL );
}

struct lvol * 
getlvol(struct da_table *kdt, char *addr)
{
    static struct lvol lv;
    int	len = sizeof(struct lvol);

    return ( read_da( kdt, addr, (char *)&lv, len) == len ? &lv : NULL );
}

getwork_Q( struct da_table *kdt, struct buf **lvworkq, struct buf ***workq)
{
    int n;
    
    n = WORKQ_SIZE * (sizeof(struct buf *));
    *workq = (struct buf **)malloc( n );
    if (workq == NULL)
	return (-1);
    if (read_da(kdt, (char *)lvworkq, (char *)*workq, n) != n) {
	free(workq);
	n = -1;
    }
    return(n);
}

struct buf *
getbpool( struct da_table *kdt, struct buf *bp )
{
    static struct buf b;
    int nbytes;
    
    nbytes = read_da( kdt, (char *)bp, (char *)&b, sizeof(struct buf));
    return (nbytes == sizeof(struct buf) ? &b : NULL);
}

/*
 * NAME:     read_da
 *
 * FUNCTION: used to emulate addressing virtual memory
 *
 * INPUTS:   da		- data area table of a component entry 
 *	     addr	- virtual memory address of dumped component
 *	     buf	- data copied to this buffer
 *	     len	- length of memory to copy to buf
 *
 * RETURNS:  0 (success), -1 failure
 *
 * BITMAPSIZE(ptr,len)   is the size in bytes of the bitmap.
 * NPAGES(ptr,len)       is the number of pages spanned by the virtual address
 *                       range of the data area.
 * ISBITMAP(n)  is true if relative page 'n' is in the bitmap.
 * These macros are also used in the dsp_da and wr_cdt() routine of the
 * dmp_do() kernel dump routine, and are defined in sys/dump.h .
 */

static bitmap_t bm[DMP_MAXPAGES / sizeof(bitmap_t)];

static
read_da( struct da_table *da, char *addr, char *buf, int len )
{
    off_t	data_offset;

    getbitmap(da);		/* bitmap indicates if page is in dump image */
    
    if ((data_offset = dump_addr(da, addr)) != -1)
	if (jseek(data_offset) != -1)
	    return( xread(da, addr, buf, len) );                 
    return ( -1 );
}

/* round down to block address of block containing address */
#define BALIGN(address)		( (unsigned) address & ~(PAGESIZE - 1) )
/* determine relative block number: how many blocks addr is from saddr */
#define PAGENUM(saddr, addr)	((BALIGN(addr) - BALIGN(saddr)) / PAGESIZE)

/*
 * NAME:     xread
 *
 * FUNCTION: read from a dumpfile as though reading from memory
 *
 * INPUTS:   saddr	- dumped memory address, (struct da_table)->da_ptr
 *	     addr	- read from this virtual memory address
 *	     buf	- copy into this buffer
 *	     len	- length of read to copy into buffer
 *
 * RETURNS:  number of bytes read
 */
xread( struct da_table *da,	/* data table in component		*/
       char *addr,		/* virtual memory address to read	*/
       char *buf,		/* read copies into buf			*/
       int len			/* length of memory to copy		*/ )
{
    int	pagen;			/* specifies page to read from		*/
    int offset_in_page;		/* address position in first page	*/
    int lastpage;		/* last page to read from		*/
    int	count;			/* count of data remaining to be read	*/
    int n;
    
    pagen = PAGENUM(da->da_ptr, addr);
    lastpage = pagen + NPAGES(addr, len);
    offset_in_page = (unsigned) addr % PAGESIZE;
    count = len;

    /* read first page */
    count -= readfrompage( pagen, buf, MIN(count, PAGESIZE - offset_in_page) );

    /* read more pages */
    for( pagen++; pagen <= lastpage; pagen++, buf += n, count -= n )
	n = readfrompage(pagen, buf, MIN(count, PAGESIZE));

    return ( len - count );
}

/*
 * NAME:     readfrompage
 *
 * FUNCTION: read from a dumpfile as though reading from a page in memory
 *
 * INPUTS:   pagen	- page number from start of dumped pages to be read
 *	     buf	- read into buf buffer
 *	     len	- length of read to copy into buffer
 *
 * RETURNS:  number of bytes read
 *
 * ISBITMAP(n) is true if relative page 'n' is in the bitmap.
 */
readfrompage( int pagen, char *buf, int len )
{
    int rc;
    
    if (len < 0 || len > PAGESIZE)
	rc = -1;			/* len must be 0 to PAGESIZE */
    else if ( ISBITMAP(bm, pagen) )
	rc = jread(buf, len);		/* read data */
    else {
	memset(buf, 0, len);		/* memory paged out or never used */
	rc = len;
    }

    return (rc);
}

/*
 * NAME:     dump_addr
 *
 * FUNCTION: return offset of addr in dump file.
 *
 * INPUTS:   tp		- da_table for a component dump table entry
 *	     addr	- address of memory within this dump table entry
 *
 * RETURNS:  offset of addr in the dump file.
 *
 * ISBITMAP(n) is true if relative page 'n' is in the bitmap.
 */
off_t
dump_addr( struct da_table *da, char *addr)
{
    off_t	offset;		/* offset in dump image */
    
    offset = ddistance ( da, (char *)da->da_ptr, addr );
    if (offset != -1)
	offset += da->da_bmoffset + BITMAPSIZE(da->da_ptr, da->da_len);
    return (offset);
}

/*
 * NAME:     ddistance
 *
 * FUNCTION: distance between two addresses in a dumpfile
 *
 * INPUTS:   da		- da_table for a component dump table entry
 *	     from	- first address
 *	     to		- second address
 *
 * RETURNS:  separation of addresses in dump file or -1 on failure
 *
 * ISBITMAP(n) is true if relative page 'n' is in the bitmap.
 */
ddistance ( struct da_table *da, char *from, char *to )
{
    int realdist, dumpdist;	/* real distance, distance in dumpfile	*/
    int pagen;			/* block dumped				*/
    int offset_in_page;		/* offset in a block			*/

    if ((realdist = to - from) == 0)
	dumpdist = 0;		/* same address				   */
    else if (realdist < 0)
	dumpdist = -1;		/* first address not before second address */
    else if ((int)from < da->da_ptr || (int)to >= (da->da_ptr + da->da_len))
	dumpdist = -1;		/* address(es) outside of dumped entry	   */
    else {
	/* determine distance from beginning of data to first address */
	dumpdist = 0;
	pagen    = PAGENUM(da->da_ptr, from);	/* blocks from start of data */
	offset_in_page = (unsigned) from % PAGESIZE; /* offset in first page */

	getbitmap( da );	/* initialize bit map for this da_table	*/
	
	/* first page may be a partial block */
	if (ISBITMAP(bm, pagen))
	    dumpdist += MIN(realdist, PAGESIZE - offset_in_page);
	realdist -= MIN(realdist, PAGESIZE - offset_in_page);

	/* more pages */
	for ( ; realdist > 0; pagen++, realdist -= PAGESIZE)
	    if ( ISBITMAP(bm, pagen) )
		dumpdist += MIN(realdist, PAGESIZE);
    }

    return (dumpdist);		/* separation of addresses in dumpfile */
}

/*
 * NAME:     getbitmap
 *
 * FUNCTION: get the bitmap for the requested da_table
 *
 * INPUTS:   da		- da_table for a component dump table entry
 *
 * RETURNS:  none
 */
getbitmap( struct da_table *da )
{
    static struct da_table *da_bm; /* indicates which da's bitmap is set */

    if (da_bm != da) {
	jseek(da->da_bmoffset);	/* seek to bitmap			 */
	jread(bm, BITMAPSIZE(da->da_ptr, da->da_len));	/* read bitmap	 */
	da_bm = da;		/* bitmap is set for this da_table       */
    }
}

/*
 * NAME:     dsplookuplbuf
 *
 * FUNCTION: given an lbuf address, display the corresponding pbuf(s) 
 *
 * INPUTS:   addr - address of an lbuf
 *
 * NOTE:     hd_pbuf anchors the pbufs.  The pbuf field pb_forw links pbufs.
 *	     The list is NULL terminated.
 *
 * MESSAGE NOTE:  This is called from xtr.c which is used in crash.  Will
 *		not be messagified/translated.
 * RETURNS:  none
 */
void
dsplookuplbuf(int addr)
{
    int			found=0;	/* pbuf found			    */
    struct pbuf		pb;		/* pbuf pointer			    */
    struct pbuf		*paddr;		/* pbuf address			    */
    static struct pbuf	*saddr;		/* start pbuf address		    */

    printf("%x: ", (unsigned int) addr);

    if (hd_pbuf && kern_dt) {
	if (saddr == NULL) {
	    if (read_da(kern_dt, (char *)hd_pbuf, (char *) &saddr,
			sizeof(struct pbuf *)) != sizeof(struct pubf *))
		saddr = NULL;
	}
    }
    if (saddr) {
	rdpb(paddr = saddr, &pb);
	do {
	    if ((int)pb.pb_lbuf == addr) {
		found = 1;
		printf("%x\t", (unsigned int) paddr);
	    }
	    /* Don't ever use a NULL pointer */
	    if( pb.pb_forw == NULL )
		break;

	    rdpb(paddr = pb.pb_forw, &pb);
	} while( paddr != saddr );

	if ( found )
	    printf("\n");
	else
	    printf("No pbufs found\n");
    }
}

/*
 * NAME:     rdpb
 *
 * FUNCTION: read a pbuf from the kernel dump image
 *
 * INPUTS:   ptr - address of a pbuf in the kernel dump image
 *	     buf - copy the pbuf into buf
 */
void
rdpb( struct pbuf *ptr, struct pbuf *buf)
{
    int rc;

    rc = read_da(kern_dt, (char *)ptr, (char *)buf, sizeof(struct pbuf));
    if (rc != sizeof(struct pbuf)) {
	buf->pb_lbuf = NULL;
	buf->pb_forw = NULL;
    }
}
