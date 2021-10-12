/* @(#)02       1.26  src/bos/kernel/sys/dump.h, sysdump, bos411, 9428A410j 3/3/94 17:42:05  */ 

/*
 * COMPONENT_NAME: sysdump 
 *
 * FUNCTIONS:  header file for system dumpfile formats
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_DUMP
#define _H_DUMP

/*
 * The d_segval member is used by machines such as the R2 and RT whose
 * address space can be described by a caddr_t and an int segment value.
 * It is used to hold the segment containing the proc table and user blocks.
 * The d_xmemp is a pointer to an xmem structure for mmu-implementation
 * independent address space transfers.
 * For RT and R2, passing the d_segval is more convenient than passing
 * a pointer to an xmem structure, since it is not necessary to allocate
 * space for the xmem structure.
 */

struct cdt_entry {				/* component dump table entry */
	char             d_name[8];	/* name of data area to be dumped */
	int              d_len;		/* length of data area to be dumped */
	char            *d_ptr;		/* offset of data area to be dumped */
	union {
		int         _d_segval;	/* segment value for xmem operations */
		struct xmem *d_xmemp;	/* for non RT/R2 systems */
	} _d;
};

#define d_xmemdp d_segval	/* temp. to make switchover to d_segval */
#define d_segval _d._d_segval

struct cdt_head {
	int  _cdt_magic;
	char _cdt_name[16];		/* component dump name */
	int  _cdt_len;			/* length of component dump table */
};
#define DMP_MAGIC  0xEEEEEEEE

struct cdt0 {						/* component dump table */
	struct cdt_head  _cdt_head;		/* minus the variable length table */
};

struct cdt {						/* component dump table */
	struct cdt_head  _cdt_head;
	struct cdt_entry  cdt_entry[1];	/* component dump table entries */
};
#define cdt_magic _cdt_head._cdt_magic
#define cdt_name  _cdt_head._cdt_name
#define cdt_len   _cdt_head._cdt_len

#define	NUM_ENTRIES(cp)	\
	(((cp)->cdt_len - sizeof(struct cdt_head)) / sizeof(struct cdt_entry))

#define	DMPIOC	('d' << 8)

#define DMPSET_PRIM  (DMPIOC|1)
#define DMPSET_SEC   (DMPIOC|2)
#define DMPNOW_PRIM  (DMPIOC|4)
#define DMPNOW_SEC   (DMPIOC|5)
#define DMPNOW_AUTO  (DMPIOC|6)
#define DMP_IOCINFO  (DMPIOC|16)	/* dump info for savecore */
#define DMP_IOCSTAT  (DMPIOC|17)	/* dump info for savecore */
#define DMP_IOCHALT  (DMPIOC|18)	/* arg == 0 means don't halt after start */
#define DMP_IOCSTAT2 (DMPIOC|19)	/* this flag is used for the new -z flag */
#define DMP_SIZE (DMPIOC|15)            /* this flag is used for -e flag*/

struct dmp_query {
	int min_tsize;
	int max_tsize;
};

#define DMPD_PRIM	1		/* must be 1 */
#define DMPD_SEC	2		/* must be 2 */
#define DMPD_AUTO	3
#define DMPD_PRIM_HALT	4
#define DMPD_SEC_HALT	5

/*
 * these definitions are mainly for formatting the raw dump data
*/

typedef unsigned char bitmap_t;

#define DMP_MAXPAGES  (256 * 1024 * 1024 / PAGESIZE)	/* 256 MB */

#define BITS_BM		(8 * sizeof(bitmap_t))	/* must be a power of 2 */
#define	DMP_BMBITS	(512 * BITS_BM)		/* must be a power of 2 */
#define	DMP_BMMASK	(512 * BITS_BM - 1)	/* so that this works   */

#define	ISBITMAP(bm,bit) \
	( bm[(bit)/BITS_BM] &  (1 << ((bit) % BITS_BM)) )

#define	SETBITMAP(bm,bit) \
	( bm[(bit)/BITS_BM] |= (1 << ((bit) % BITS_BM)) )

#define BTOP(addr) (((unsigned)(addr) & 0x0FFFFFFF) / PAGESIZE)

#define DMPLIM(ptr,len) \
	( DMPSEG((ptr)+(len)-1) == DMPSEG(ptr) ? \
		(unsigned)((ptr)+(len)-1) : 0x0FFFFFFF )

#define DMPSEG(x) ((unsigned)(x) & 0xF0000000)

#define NPAGES(ptr,len) \
	( (len) == 0 ? 0 : BTOP(DMPLIM(ptr,len)) - BTOP(ptr) + 1 )

/*
 * # of pages:  # of bitmap_t elements (bytes)
 *      0                0 
 *      1                1 
 *      8                1
 *      9                2
 *      16               2
 *      17               3
 */
#define BITMAPSIZE(ptr,len) \
	( (NPAGES(ptr,len) + BITS_BM - 1) / BITS_BM )

/*
 * Return codes from dmp_do.
 */
#define DMPDO_SUCCESS     0
#define DMPDO_DISABLED   -1
#define DMPDO_PART       -2
#define DMPDO_FAIL       -3


/*
 * dm_flags bitfield values.
 */
#define DMPFL_NEEDLOG	0x01		/* this dump needs to be errlogged */
#define DMPFL_NEEDCOPY	0x02		/* this dump needs to be copied    */

#define DMPTRC_DMPOPEN    1
#define DMPTRC_DMPCLOSE   2
#define DMPTRC_DMPIOCTL   3
#define DMPTRC_DMPDUMP    4
#define DMPTRC_DMPADD     5
#define DMPTRC_DMPADDEXIT 6
#define DMPTRC_DMPDEL     7
#define DMPTRC_DMPDELEXIT 8
#define DMPTRC_DMPDO      9
#define DMPTRC_DMPDOEXIT  10
#define DMPTRC_DMPWRCDT   11
#define DMPTRC_DMPOP      12
#define DMPTRC_DMPOPEXIT  13
#define DMPTRC_DMPNULL    14
#define DMPTRC_DMPFILE    15

#ifdef TRCHK
#define DMP_TRCHK(name,dw) \
	TRCHK(HKWD_RAS_DUMP  | (DMPTRC_##name << 8) | ((dw) & 0x0FF))

#define DMP_TRCHKL(name,dw,value2) \
	TRCHKL(HKWD_RAS_DUMP | (DMPTRC_##name << 8) | ((dw) & 0x0FF),value2)

#define DMP_TRCHKG(name,dw,v1,v2,v3,v4,v5) \
	TRCHKG(HKWD_RAS_DUMP | (DMPTRC_##name << 8) | ((dw) & 0x0FF),\
		v1,v2,v3,v4,v5)
#endif

/*
 * #include <sys/dump.h>
 *
 * fd = open("/dev/sysdump",0);
 * ioctl(fd,DMP_IOCINFO,&dumpinfo);
 *
 *
 * If dm_size > 0 and if dm_devicename[0] != '\0', there is a valid dump.
 * The device name is dm_devicename.
 * The size in bytes of the dump is dm_size.
 * The seconds (time()) timestamp of the dump is dm_timestamp.
 * The magic number is a 4 byte number in bytes 0-3 of the dump image
 *   and is define in DMP_MAGIC
 *
 * Call the ioctl once. After that, the dumpinfo is all 0's.
 */
#define DEVNAME_LEN 256
struct dumpinfo {
	char   dm_devicename[DEVNAME_LEN]; /*name of dump device (/dev/rhd9)*/
	dev_t  dm_mmdev;		/* major/minor of dump device */
	off_t  dm_size;			/* size in bytes of the dump */
	time_t dm_timestamp;		/* seconds timestamp */
	int    dm_type;			/* 1 = primary, 2 = secondary */
	int    dm_status;		/* DMPDO_XXXX */
	int    dm_flags;		/* DMPFL_XXXX */
	u_long dm_hostIP;   	        /* remote IP address               */
        char   dm_filehandle[32];
	int    dm_fill;	        /*	fill to 64 bytes */
};

struct dump_read {
	struct mbuf *dump_bufread;
	int    wait_time;
};	

struct dump_read *dump_readp;

#endif	/* _H_DUMP */

