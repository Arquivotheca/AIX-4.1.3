/* @(#)72	1.18.1.10  src/bos/kernel/sys/POWER/vmker.h, sysvmm, bos411, 9436B411a 9/1/94 17:48:55 */
#ifndef _H_VMKER
#define _H_VMKER
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */
/*
 * vmkerdata contains the variables of the VMM which must be
 * kept in the kernel segment for addressabilty.  vmkerdata
 * is kept in pinned V=R storage.
 *
 * NOTE - changes made here should also be made to vmker.m4.
 */

#include <sys/lock_def.h>

struct vmkerdata
{
	uint	vmmsrval;	/* sreg value for vmmdseg */
	uint	ptasrval;	/* sreg value for ptaseg  */
	uint	dmapsrval;	/* sreg value for pg space disk maps */
	uint	ramdsrval;	/* sreg value for ram disk */
	uint	kexsrval;	/* sreg value for kernel extension segment */
	uint	iplcbptr;	/* virtual addr of ipl control block */
	uint	hashbits;	/* number of bits in sid/vpn hash */
	uint	stoibits;	/* hash shift amount for STOI/ITOS */
	int	psrsvdblks;	/* number of reserved paging space blocks */
	int	nrpages;	/* biggest page frame number + 1    */
	int	badpages;	/* number bad memory pages 	    */
	int	numfrb; 	/* number of pages on free list     */
	int	maxperm;	/* max number of frames non-working */
	int	numperm;  	/* number frames non-working segments */
	int	numpsblks;	/* number of paging space blocks */
	int	psfreeblks;	/* number of free paging space blocks */
	uint	bconfsrval;	/* sreg value for base config segment */
	int	pfrsvdblks;	/* number of reserved (non-pinable) frames */
	uint	nofetchprot;	/* no fetch protect allowed due to h/w prob */
	int	ukernsrval;	/* user's shadow of kernel srval      */
	int	numclient;	/* number of client frames */
	int	maxclient;	/* max number of client frames */
	int	kernsrval;	/* sreg value for kernel segment */
	int	stoimask;	/* STOI/ITOS mask	*/
	int	stoinio;	/* STOI/ITOS sid mask	*/
	int	maxpout;	/* non-fblru pageout up limit - i/o pacing */
	int	minpout;	/* non-fblru pageout down limit - i/o pacing */
	int	rptsize;	/* size of repaging table	*/
	int	rptfree;	/* next free repaging table entry */
	int	rpdecay;	/* decay rate for repaging values */
	int	sysrepage;	/* global repaging count	*/
	uint	swhashmask;	/* bit mask to use in sid/vpn s/w hash */
	uint	hashmask;	/* bit mask to use in sid/vpn hash */
	uint	cachealign;	/* alignment to avoid cache aliasing */
	uint	overflows;	/* number of page table overflows */
	uint	reloads;	/* number of page table reloads */
	caddr_t pmap_lock_addr; /* self explanatory */
	uint	numcompress;	/* number of frames in compressed segments */
	uint	noflush;	/* 0 => write compressed files at iclose */
	uint	iplcbxptr;	/* virt addr of extended ipl control block */
	uint	ahashmask;	/* bit mask to use in sid/vpn alias hash */
	Simple_lock vmkerlock;	/* MP lock for vmker fields and operations */
	uint	pd_npages;	/* max number of pages to delete in one c.s. */
};

extern struct vmkerdata vmker;

#define vmker_lock	vmker.vmkerlock


/*
 * Real address range mappings established during system initialization.
 * The VMM uses this array to establish mappings in the page tables and
 * to assign real memory at system initialization.
 * Real memory is allocated to all valid ranges except those already
 * backed with memory ('ros' is set) and those in I/O space ('io' is set).
 * Contiguous memory of 'size' bytes is allocated on an alignment specified
 * by 'align', is zeroed, and 'raddr' is set to the starting real address.
 * All valid ranges except those for which unique segments are allocated
 * ('seg' is set) are mapped into the page table starting at effective
 * address 'eaddr' with storage attributes 'wimg'.  This effective address
 * must be in global kernel memory -- the kernel segment (KERNSR) or kernel
 * extension segment (KERNXSR).
 * Note that in the current implementation, use of vmrmap_io implies that
 * the real address range does not correspond to physical memory.  Ranges
 * of physical memory that need to be reserved should use vmrmap_ros.
 */
#define RMAP_MAX	0x20  /* Maximum number of ranges */

struct vmranges {
	uint	_lastx;		/* Index of last valid entry */
	struct {
		struct {
			unsigned _valid	: 1;	/* Entry is valid */
			unsigned _ros	: 1;	/* Established by ROS */
			unsigned _holes	: 1;	/* May contain bad memory */
			unsigned _io	: 1;	/* Range is in I/O space */
			unsigned _seg	: 1;	/* Assign to its own segment */
			unsigned _wimg	: 4;	/* WIMG storage attributes */
			unsigned _rsvd	: 7;	/* Reserved */
			unsigned _id	:16;	/* Identifier for range */
		} s1;
		uint	_raddr;	/* Real address */
		uint	_eaddr;	/* Effective address */
		uint	_size;	/* Size of mapping in bytes */
		uint	_align;	/* Real memory alignment in bytes */
	} range[RMAP_MAX];
};

/*
 * Zeroed storage for the array is defined elsewhere.
 */
extern struct vmranges vmrmap;

/*
 * Macros to access fields in an rmap array entry.
 */
#define vmrmap_valid(x)	vmrmap.range[(x)].s1._valid
#define vmrmap_ros(x)	vmrmap.range[(x)].s1._ros
#define vmrmap_holes(x)	vmrmap.range[(x)].s1._holes
#define vmrmap_io(x)	vmrmap.range[(x)].s1._io
#define vmrmap_seg(x)	vmrmap.range[(x)].s1._seg
#define vmrmap_wimg(x)	vmrmap.range[(x)].s1._wimg
#define vmrmap_id(x)	vmrmap.range[(x)].s1._id
#define vmrmap_raddr(x)	vmrmap.range[(x)]._raddr
#define vmrmap_eaddr(x)	vmrmap.range[(x)]._eaddr
#define vmrmap_size(x)	vmrmap.range[(x)]._size
#define vmrmap_align(x)	vmrmap.range[(x)]._align

/*
 * Access to fixed-index entries in the array.
 */
#define RMAP_KERN		0x0001	/* Kernel */
#define RMAP_IPLCB		0x0002	/* IPL control block */
#define RMAP_MST		0x0003	/* Machine state save areas */
#define RMAP_RAMD		0x0004	/* RAM disk */
#define RMAP_BCFG		0x0005	/* Boot configuration data */
#define RMAP_HAT		0x0006	/* h/w hash anchor table */
#define RMAP_PFT		0x0007	/* h/w page frame table */
#define RMAP_PVT		0x0008	/* Physical-to-virtual table */
#define RMAP_PVLIST		0x0009	/* Physical-to-virtual list */
#define RMAP_SWPFT		0x000a	/* Software page table */
#define RMAP_SWHAT		0x000b	/* Software page table hash */
#define RMAP_APT		0x000c	/* Alias page table */
#define RMAP_AHAT		0x000d	/* Alias page table hash */
#define RMAP_RPT		0x000e	/* Repaging table */
#define RMAP_RPHAT		0x000f	/* Repaging table hash */
#define RMAP_PDT		0x0010	/* Paging device table */
#define RMAP_PTAR		0x0011	/* PTA segment XPT root */
#define RMAP_PTAD		0x0012	/* PTA segment direct XPTs */
#define RMAP_PTAI		0x0013	/* PTA segment initial root XPTs */
#define RMAP_DMAP		0x0014	/* Paging space disk maps */
#define RMAP_IPLCBX		0x0015	/* Extended IPL Control Block */
#define RMAP_LAST		0x0015	/* Index of last fixed-index entry */

/*
 * Access to variable-index entries in the array.
 * RMAP_INIT must be used once before initializing any variable-index entries.
 * RMAP_NEXT is used before a variable-index entry is initialized.
 * RMAP_X is used as the index.
 */
#define RMAP_X		vmrmap._lastx
#define RMAP_INIT	(RMAP_X = RMAP_LAST)
#define	RMAP_NEXT	{RMAP_X++; assert(RMAP_X < RMAP_MAX);}

/*
 * Identifiers for variable-index entries.
 */
#define RMAP_SYSREG		0x0100	/* System Registers */
#define RMAP_SYSINT		0x0101	/* System Interrupt Registers */
#define RMAP_NVRAM		0x0102	/* Non-Volatile RAM */
#define RMAP_TCE		0x0103	/* Translate Control Entries */
#define RMAP_MCSR		0x0104	/* MCSR */
#define RMAP_MEAR		0x0105	/* MEAR */
#ifdef _PEGASUS
#define RMAP_SYS_SPEC_REG	0x0106	/* System Specific Registers */
#define RMAP_APR		0x0107	/* APR */
#endif

/*
 * Commonly used storage control attributes.
 */
#define WIMG_M		0x2	/* W=0, I=0, M=1, G=0 */
#define WIMG_IG		0x5	/* W=0, I=1, M=0, G=1 */
#define WIMG_DEFAULT	WIMG_M


/*
 * Page intervals utilized by the VMM.
 */
#define VMINT_MAX	0x10	/* Maximum number of intervals per type */

struct vmintervals {
	uint	_num;		/* Number of intervals */
	struct {
		uint	_start;	/* Starting page number */
		uint	_end;	/* Ending page number */
	} bounds[VMINT_MAX];
};

/*
 * Zeroed storage for the array is defined elsewhere.
 */
extern struct vmintervals vmint[];

/*
 * Macros to access fields in an vmint array entry.
 */
#define vmint_num(x)		vmint[(x)]._num
#define vmint_start(x,y)	vmint[(x)].bounds[(y)]._start
#define vmint_end(x,y)		vmint[(x)].bounds[(y)]._end

/*
 * Interval types.
 */
#define VMINT_TYPES	0x4	/* Maximum interval types */
#define VMINT_BADMEM	0x0	/* Bad memory or memory holes */
#define VMINT_FIXMEM	0x1	/* Kernel memory ranges to page fix */
#define VMINT_RELMEM	0x2	/* Kernel memory ranges to release */
#define VMINT_FIXCOM	0x3	/* Kernel pinned common ranges */

#endif /* _H_VMKER */
