/* @(#)24	1.49.1.2  src/bos/kernel/ldr/ld_data.h, sysldr, bos41J, 9520A_all 5/16/95 16:45:11 */

#ifndef _H_LD_DATA
#define _H_LD_DATA

/*
 * COMPONENT_NAME: (SYSLDR) Program Management
 *
 * FUNCTIONS: loader data
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

#include	<sys/ldr/m_ld_data.h>

/* emess structure - chain of messages */
struct emess{
	struct	emess	*next;
	char	errordata[4];
};

/* elements used to keep track of user allocated storage */
struct dinuse{
	struct	dinuse	*next;
	char	*start;
	uint	length;
};

/* One loader anchor per loader domain					*/
/*  Kernel Domain  - la_text_heap and la_data_heap  addr of kernel	*/
/*		     heap anchor					*/
/*  Library Domain - la_text_heap addr of heap anchor in the shared	*/
/*		     library text segment				*/
/*		   - la_data_heap addr of heap anchor in the shared	*/
/*		     library data segment				*/
/*  Process Domain - la_text_heap and la_data_heap  addr w/in kheap	*/
/*		     for allocation					*/

struct  loader_anchor {
    struct     loader_entry    *la_loadlist;
    struct     loader_entry    *la_execle; 	/*le of originial execed file*/
    heapaddr_t la_text_heap;	/* heap for text allocation */
    heapaddr_t la_data_heap;	/* heap for data allocation */
    int        la_lock;
    int        la_flags;        /* see heading "flags for la_flags"  */
    uint       la_basesid;	/* if PTRACE & TEXTALLOC this is the sreg val
				   of the text segment register */
    char	*sbreak;        /* the sbreak value */
    char	*minbreak;	/* minimum value break is allowed to take
    				 * break is never allowed to be less than the
    				 * data of some loaded program*/
    char	*lastbreak;	/* region from lastbreak to sbreak is in use
    				 * region from minbreak to lastbreak is free */
    struct	dinuse	*la_dinuse;	/* anchor of chain of descriptors
				 *of user allocated storage in data area */
    struct	emess	*la_emess;	/* emess chain for private */
    vmhandle_t	la_ovfl_srval;	/* srval for segment used when per-process
				   kernel heap overflows */
    vmhandle_t	la_save_srval;	/* saved srval when we need to load a sr to
				   access overflow heap */
    struct  loader_domain *la_domain; /* loader domain */
} ;

/*
 * flags for la_flags
 */

#define    LA_TEXTALLOC  0x01   /* the text segment is a computational
				   segment */
#define    LA_PTRACE_TS  0x02   /* the loader has made a ptrace copy of
				   the processes text segment */
#define	   LA_DEFERED    0x04   /* there are (or once where) outstanding
				   defered resolutions */
#define    LA_DALLOC     0x08   /* the "d" allocator has been initialized -
				   see ld_memory */
#define    LA_UNSURE     0x20   /* unsure about state of anchor,  process is
				   being exec loaded */
#define    LA_TEXT_HEAP	 0x10	/* la_text_heap is a real heap - always on
				   except for private */
#define    LA_DATA_HEAP  0x40	/* la_data_heap is a real heap - always on
				   except for private */
#define    LA_OVFL_HEAP  0x80	/* An overflow heap has been allocated for
				   this process(srval in la_ovfl_srval) */
#define    LA_OVFLSR_ALLOCED 0x100 /* segment register we need to access the
				   overflow segment was already in use (the
				   previous value is saved in la_save_srval) */
#define    LA_OVFL_ATT   0x200	/* The overflow segment is attached to the
				   current address space */
#define    LA_PTRACE_SL  0x400  /* the loader has made a ptrace copy of shared
				   library text for this process */
#define  LA_DOMAIN_WRITE 0x800	/* the process can add entries to the loader
				   domain it has specified */


/*
 * Definitions dealing with the overflow segment
 */
#define	OVFLSEG	TEMPSEG
#define	OVFLORG	TEMPORG
#define	OVFL_EXISTS(la)		((la)->la_flags & LA_OVFL_HEAP)
#define	IS_OVFLSEG(add)		(((unsigned)(add) >> SEGSHIFT) == OVFLSEG)
#define	IS_PRIVSEG(add)		(((unsigned)(add) >> SEGSHIFT) == PRIVSEG)
/* In OVFL_PTRADD,  the order of the arguments is important!  The first
 * argument MUST be the address that MAY be in the overflow segment.
 */
#define	OVFL_PTRADD(x,y)	(IS_OVFLSEG(x) ? (void*)(x) : PTRADD(x,y))
extern heapaddr_t pp_ovfl_heap;

struct	loader_defered{
	struct loader_defered	*next;
	uint	symindex;	/* symbol index in ldsym 0 based - add
				   3 to get rld index */
	ulong	value;		/* value it has been temporarily set to */
};

/*
 * loader domains list a set of library loader entries.  A loader domain
 * is a subset of all the loader entries available in the shared library
 * segments.
 */
struct domain_altname {			/* alternate name structure */
	char	*da_fullname;		/* full pathname */
	struct	domain_altname *da_next;/* pointer to next */
};
struct domain_entry {			/* loader domain entry structure */
	struct	domain_entry *de_next;	/* next entry in chain */
	uint	de_usecount;		/* use count for domain entry */
	struct	loader_entry *de_le;	/* pointer to loader entry */
	char	*de_fullname;		/* full pathname */
	struct	domain_altname *de_altname; /* alternate names for entry */
};
struct loader_domain {			/* loader domain structure */
	struct	loader_domain *ld_next;	/* next domain in list */
	uint	ld_usecount;		/* processes using this domain */
	struct	file *ld_fp;		/* open domain file */
	char	*ld_name;		/* domain name */
	struct	domain_entry *ld_entries; /* list of domain  */
};
struct ld_reopen {
	struct domain_entry *ldr_de;	/* associated domain entry */
	struct file *ldr_fp;		/* reopen'ed file pointer */
};

/*
 * Loader entries describe modules loaded in a segment.
 * They are allocated from either the loader region or the kheap
 * Loader entries are actually allocated with the correct number of
 * depends slots - declare shows 1 but may be greater
 */

struct   loader_entry  {
    struct	loader_entry    *le_next;    /* next entry */
    ushort	le_usecount;                 /* number of references from
						other le's plus number of
						loads */
    ushort	le_loadcount;                /* number of explicit loads */
    uint	le_flags;                    /* see heading
						"flags for le_flags" heading */
    struct      file           *le_fp;
    char       *le_file;                     /* file is contigous starting with
                                                header */
    unsigned    le_filesize;                 /* amount actually read in or
						mapped */
    union {
    char       *le_data;                     /* data may be in the file or
						be a copy this is where it is.
						See loader_exports.data
				                for relocation origin */
    tid_t	le_tid;			     /* thread which has I/O pending
					        for this loader entry */
    } _le_data_union;
    unsigned	le_datasize;	             /* data plus bss size */
    struct      loader_exports  *le_exports;
    struct	loader_entry_extension	*le_lex; /* only valid while this
						    entry is being loaded */
    struct	loader_defered	*le_defered;
    char	*le_filename;	             /* member name, then path name
						used to when this was loaded */
    ushort	le_ndepend;                  /* index of first free slot */
    ushort	le_maxdepend;	             /* number of slots
				                i.e. one more than max index */
    struct	domain_entry	*le_de;      /* pointer to domain entry.  only
						valid for process private le */
    struct      loader_entry    *le_depend[1];
}  ;

#define	le_data	_le_data_union.le_data
#define	le_tid	_le_data_union.le_tid

/*
 * flags for le_flags
 */
#define    LE_UNLOAD        0x00000001  /* unload has been issued to
					   this entry */
#define    LE_TEXT          0x00000002  /* text "belongs" to this entry */
#define    LE_KERNEL        0x00000004  /* only on for the entry describing
					   the boot kernel*/
#define    LE_SYSCALLS      0x00000008  /* this le represents a syscall
					   table for a kernel extension */
#define    LE_KERNELEX      0x00000010  /* this le represents a kernel
					   extension */
#define	   LE_DATAINTEXT    0x00000020  /* data and bss are imbedded in
					   the text - e.g. kernel */
#define	   LE_DATA	    0x00000040	/* this entry represents a resolved
					   module - as opposed to an archive
					   or unresolved shared library
					   image */
#define	   LE_LIBRARY       0x00000080  /* this entry is in the shared
					   library */
#define    LE_LIBEXPORTS    0x00000100  /* the le_exports pointer points
					   to a shared library table */
#define	   LE_DATAEXISTS    0x00000200  /* data is already mapped
					   e.g.  RO in shared library */
#define	   LE_USEASIS       0x00000400  /* this copy needs no relocation -
					   no le_lex exists */
#define    LE_TEXTMAPPED    0x00000800  /* text is page mapped rather than
					   copied */
#define    LE_DATAMAPPED    0x00001000  /* data is page mapped rather than
					   copied */
#define	   LE_NOAUTODEFER   0x00002000	/* don't autoresolve defered from
					   this module */
#define	   LE_EXECLE	    0x00004000	/* this is the loader entry for
					   the exec'd module */
#define    LE_IOPENDING     0x10000000	/* this le is being preread */
#define	   LE_SRO	    0x00008000	/* describes a shared read-only
					   module */
#define	   LE_LOADAT	    0x00010000	/* module was loaded at a specific
					   address */
#define    LE_NOTTHISLOAD   0x20000000	/* this le will NOT be loaded now */
#define    LE_EXTENSION     0x40000000  /* continuation list of depends */
#define	   LE_THISLOAD      0x80000000  /* on for entries being loaded now */
#define    LE_TEXTRW        0x08000000	/* privately loaded text has been
					   made read/write by a ptrace call */
#define    LE_REDEFINEEXP   0x01000000	/* redefines a previously defined
					   exported symbol.  only set in
					   SYSCALL entries		*/
#define    LE_PRIVATELOAD   0x02000000	/* the data for this library module
					   MUST be privately loaded.  */
#define    LE_DOMAIN_ENT    0x04000000	/* this loader entry corresponds to a
					   domain entry */


/*
 * anchors
 */

extern struct loader_anchor	kernel_anchor;  /* anchor of the list of
						   loaded kernel extensions */


/*
 * name spaces
 */

extern struct loader_entry  *kernel_exports;   /* current kernel name spaces */
extern struct loader_entry  *syscall_exports;  /* current syscall name spaces */

/*
 * global flags
 */

extern	uint	shlibseg_full;		/* flag to indicate that library
					 * segment has been overflowed */
extern char *kernel_filename;		/* special file name for kernel */

extern int ld_loader_read;		/* event list for pending I/O */

/* Hash table and address table for a module
 * Table is a SINGLE data structure which should be freed with one xmfree
 * Structure of table for n symbols is:
 *  loader_exports header
 *  An index pointer vector, size the next power of two larger than n
 *  An index (see exp_index) vector, n long
 *
 *  To find a symbol, compute its hash (ld_hash), mask the hash with hash_mask
 *     and use the result as an index into hashes.  If the value in hashes
 *     is notNULL, follow it to the first of a chain of exp_index blocks
 *     Check each one first for a match of the full hash, then for a match
 *     of the symbol.
 *
 *  The exp_location field contains the current value of the symbol found.
 *
 */
struct  loader_exports {
    struct exp_index       **hashes;         /* address of hash index table */
    struct exp_index       *indexes;         /* address of the hash values */
    unsigned               numexports;       /* number of symbols */
    unsigned               hash_mask;        /* power of two mask to convert
					        hash to hat index */
    char    	  	   *data;            /* address data relocated to */
};

struct  exp_index {
    char                  *typecheck;        /* pointer to type check hash */
    char                  *sym;              /* pointer to symbol string */
    uint                   syml;             /* length */
    char                   *exp_location;    /* relocated value of this
						symbol */
    struct exp_index       *next_index;      /* next symbol index with the
						same hash */
    unsigned               fullhash;         /* full word hash of this
						symbol */
};

/* kernel svc table - built by loader, storage recorded as section 1 on
 * the fabricated loader entry which is on the load list and pointed to
 * by syscall_exports each entry contains a fake descripor which points
 * to the svc instruction and has, as its toc value, the index of the
 * entry (zero based) the final word of the entry is a pointer to the
 * real internal descriptor for this svc
 * the address of the table and its number of entries are in svc_table
 * and svc_table_entries which are defined by the svc0 handler in low memory
 */


struct svc_table_entry{
	int		(*svc)();    	/* must be first - svc handler
					   assumes this */
	char		*svc0;          /* these two words serve as a
					   procedure */
	int		index;          /* descriptor */
};

/* these are definied in the svc handler - they are
 * svc_table - initialized by the loader to point to the svc table
 * svc_table_entries  - initialized by the loader to the number of entries
 * svc_instruction - initialized by the assembler to contain the address of
 * an svc instruction
 */

extern struct svc_table_entry	*svc_table;
extern int			svc_table_entries;
extern char			*svc_instruction;

#define SMAX 32	/* wild overestimate of max number of sections in a program */

struct myxcoffhdr {
	struct filehdr f;
	struct aouthdr a;
	struct scnhdr  s[SMAX];
};

struct locs {
	ulong	d;	/* relocation factor - diff of current and target
			   values */
	ulong	v;	/* valid - true if this d is valid */
};

/* additional information about a module needed while module is being loaded.
 * if several modules are loaded at once - for example a program and libraries,
 * there is one such data structure for each module.  Its address is found in
 * the loader entry
 */

#define lex (*lexpointer)
#define hdr (*(lex.h))
struct loader_entry_extension {
	struct  loader_entry	*le;	/* entry for which this is the
					   extension */
	uint	flags;                  /* see header "loader_entry_extension"
					   flags */
	uint	filesize;	        /* size in bytes of module file */
	uint	f_nscns;	        /* copy of field from file header */
	struct  myxcoffhdr *h;          /* address of text of this file */
	struct ldhdr	*ldhdr;         /* address of loader section */
        struct ldsym	*ldsym;         /* address of sym vector in loader
					   section */
        struct ldrel	*ldrel;         /* address of rld vector in loader
					   section */
	uint	text;  		        /* zero based indexes to loader
					   section headers */
	uint	data;		        /* these will be sanity checked and
					   can be used */
	uint	bss;		
	uint	loader;                 /* N.B. values fetched from loader
					       section are one based */
	char   *textloc;	        /* address of text at execution time */
	char   *entryloc;               /* address of entry point at execution
					   time */
	uint	textreloc;	        /* differences between the bind time
					   and final addresses */
	uint	datareloc;	        /* of the text and data segments.  */
	uint	bssreloc;               /* binder treats bss seperately - so
					   we do also */
	struct loader_entry	**impid;/* vector of loader_entryies for
					   imports */
	uint	nimpid;		        /* number of entries in impid vector */
	struct locs *locs;	        /* vector of deltas from resolve */
	ulong	numexports;	        /* number of exports - computed by
					   resolve */
	uint	numsvcs;
	heapaddr_t	heap;		/* heap to use for this execution -
					   kernel, library,user */
};

/*
 * flags for "loader_entry_extension.flags"
 */

#define	XMALLOC		0x40000000	/* loader extension needs to be
					   xfreed */
#define FOUNDDEFERED	0x20000000	/* used in redo */
#define TEXTMALLOCED	0x10000000	/* used so sanity can know when
					   to free text on error */
#define TEXTFFALLOCED	0x08000000	/* used so ld_freelex and ld_sanity can
					   know when to fffree text */
#define LOADERMALLOCED	0x04000000	/* used so ld_freelex and ld_sanity can
					   know when to xmfree loader section */
#define LOADERFFALLOCED	0x02000000	/* used so ld_freelex and ld_sanity can
					   know when to fffree loader section */
#define TEXTMAPPED	0x01000000	/* identifies a module whose text has
					   been mapped */

/* path name lookaside table entry */

struct pathlook {
	struct  pathlook *next;	/* chain pointer */
	uint	hash;  		/* composit hash of path, filename,
				   member name */
	void	*val;		/* value - either a file pointer or a
				   loader_entry pointer */
	uint	length;		/* length of name field - field may contain
				   embedded nulls */
	char	type;		/* type - 'F' for file, 'L' for loader entry */
	char	name[1];	/* beginning of actual value */
};

struct	d{
	ulong	org;
	ulong	end;
};

#define ss (*sspointer)

struct sharedstuff {
	int	myretcode;
	ushort	flags;
	char	type;		               /* E for exec,
						  K for kernel,
						  L for library */
	struct  loader_anchor	*la;
	struct  loader_entry	*end_of_new;
	struct  loader_entry	*sysle;
	struct	loader_entry	*execle;
	struct	pathlook	*pl;	      /* anchor for pathname lookaside
						 chain */
	char	*libpath;	              /* library path for library
						 search */
	char	*maxbreak;	              /* maximum data address - in
						 bytes */
	struct	sharedstuff	*topstuff;    /* in libld - original load ss */
	struct	d	*d;	              /* anchor for (temp) storage
						 map */
	char	*load_addr;		      /* load address, if any */
	struct	loader_entry	**pre;        /* array of le's already found */
	ulong	precount;		      /* number of entries in pre */
	ulong	maxprecount;
	struct	m_data  m_data;	              /* platform dependent stuff */
	char	*libpath_env;		      /* libpath string specified in
						 LIBPATH environment variable
						 or load parameter */
	struct	loader_domain	*ld;          /* loader domain in use */
	struct	ld_reopen *ldr;               /* array of reopened fps for
						 loader domain entry matching
					       */
	char	*libpath_saved;		      /* libpath string saved from the
						 prepass */
};

/*  flag values for flags field of sharedstuff structure */
#define	SS_PRIVATELOAD		0x0001	      /* Indicates that all data be
						 privately loaded.  Only
						 applies to exec_load */
#define	SS_LD_ADDRESSLIB	0x0002	      /* Indicated that ld_addresslib
						 has been called */
#define	SS_RELEASE_SRS		0x0004	      /* Indicates that segment regs
						 must be releases before
						 making calls outside loader */
#define	SS_LIBPATH_ALLOCED	0x0008	      /* Indicates the string pointed
						 to by ss.libpath has been
						 allocated from kernel_heap */

/*  control structure for shared library segment.
 *  This structure is at the same virtual location in every shared library
 *  segment - it is established by ld_libinit.
 */

struct library_anchor{
	struct	loader_anchor	la;
};
extern struct library_anchor *library_anchor;

extern vmhandle_t library_data_handle;	/* handle for data library segment */
extern vmhandle_t library_text_handle;	/* handle for text library segment */

#define lib (*library_anchor)
#define FPtoFH(fp)	(uint)(((fp)->f_vnode)->v_gnode)
#define VtoFH(vn)	(uint)((vn)->v_gnode)
#define PAGEDOWN(p)     ((uint)(p) & (-PAGESIZE))
#define PAGEUP(p)       ( ( (uint)(p) + PAGESIZE -1) & (-PAGESIZE) )
#define PTRADD(x,y)	((void*)((ulong)(x) + (ulong)(y)))

/* ld_textread() flags */
#define LD_textreadbss	0x00000001
#define LD_allocd	0x00000002
#define LD_localfile	0x00000004

/* The following macros are used for possible calls to the
 * ld_srfreecall() routine.  See this routine for details.
 */
#define BDATA_SEG_MASK  0x1FE00000
#define LD_MAX_ARGS     6
#define MIN_FREE_SEGS   3
#define BRK(endds) \
	(ss.flags & SS_RELEASE_SRS ? \
		ld_srfreecall(brk, (endds)) : \
		brk((endds)))
#define COPYIN(src, dest, count) \
	(ss.flags & SS_RELEASE_SRS ? \
		ld_srfreecall(copyin, (src), (dest), (count)) : \
		copyin((src), (dest), (count)))
#define COPYINSTR(from, to, max, actual) \
	(ss.flags & SS_RELEASE_SRS ? \
		ld_srfreecall(copyinstr, (from),(to),(max),(actual)) : \
		copyinstr((from), (to), (max), (actual)))
#define FP_CLOSE(fp) \
	(ss.flags & SS_RELEASE_SRS ? \
		ld_srfreecall(fp_close, (fp)) : \
		fp_close((fp)))
#define FP_FSTAT(fp, statp, len, seg) \
	(ss.flags & SS_RELEASE_SRS ? \
		ld_srfreecall(fp_fstat, (fp), (statp), (len), (seg)) : \
		fp_fstat((fp), (statp), (len), (seg)))
#define FP_OPEN(path, oflags, mode, ext, fpflag, fpp) \
	(ss.flags & SS_RELEASE_SRS ? \
		ld_srfreecall(fp_open, (path) ,(oflags) , \
			(mode) ,(ext) ,(fpflag) ,(fpp)) : \
		fp_open((path) ,(oflags), (mode), (ext), (fpflag), (fpp)))
#define FP_READ(fp, buf, nbytes, ext, seg, countp) \
	(ss.flags & SS_RELEASE_SRS ? \
		ld_srfreecall(fp_read, (fp), (buf), (nbytes) , \
			(ext), (seg), (countp)) : \
		fp_read((fp), (buf), (nbytes), (ext), (seg), (countp)))
#define FP_SHMAT(fp, addr, request, hp) \
	(ss.flags & SS_RELEASE_SRS ? \
		ld_srfreecall(fp_shmat, (fp), (addr), (request), (hp)) : \
		fp_shmat((fp), (addr), (request), (hp)))
#define FP_SHMDT(fp, flag) \
	(ss.flags & SS_RELEASE_SRS ? \
		ld_srfreecall(fp_shmdt, (fp), (flag)) : \
		fp_shmdt((fp), (flag)))
#define VM_MAP(saddr, taddr, nbytes) \
	(ss.flags & SS_RELEASE_SRS ? \
		ld_srfreecall(vm_map, (saddr), (taddr), \
			(nbytes)) : \
		vm_map((saddr), (taddr), (nbytes)))
#define VM_PROTECT(vaddr,nbytes,key) \
	(ss.flags & SS_RELEASE_SRS ? \
		ld_srfreecall(vm_protect, (vaddr), (nbytes), (key)) : \
		vm_protect((vaddr), (nbytes), (key)))
#define VM_RELEASE(vaddr, nbytes) \
	(ss.flags & SS_RELEASE_SRS ? \
		ld_srfreecall(vm_release, (vaddr), (nbytes)) : \
		vm_release((vaddr), (nbytes)))
#define VM_WRITEP(sid, pfirst, npages) \
	(ss.flags & SS_RELEASE_SRS ? \
		ld_srfreecall(vm_writep, (sid), (pfirst), (npages)) : \
		vm_writep((sid), (pfirst), (npages)))
#define VMS_CREATE(sid, type, device, size, uplim, downlim) \
	(ss.flags & SS_RELEASE_SRS ? \
		ld_srfreecall(vms_create, (sid), (type), (device), \
			(size), (uplim), (downlim)) : \
		vms_create((sid),(type),(device),(size),(uplim),(downlim)))
#define VMS_DELETE(sid) \
	(ss.flags & SS_RELEASE_SRS ? \
		ld_srfreecall(vms_delete, (sid)) : \
		vms_delete((sid)))

#ifndef _NO_PROTO
/* declare all functions used to get enhanced error checking */
void
ld_addressppseg(struct sharedstuff *sspointer);

ulong
ld_allocd(struct sharedstuff *sspointer,ulong  datasize);

int
ld_assigndata(struct sharedstuff *sspointer);

struct loader_exports *
ld_buildhash(
	struct ldhdr *ldhdr,
	int          numexports,
	struct locs  *locs,
	char         type,
	int          kernel,
	struct loader_entry	*le,
	heapaddr_t   heap,
	int	*redefine);

int
ld_cleartext(struct sharedstuff *sspointer,char *textorg,struct file *fp);

void
ld_clearle(struct sharedstuff *sspointer,struct loader_entry *le);

int
ld_clearlib(struct sharedstuff *sspointer);

void
ld_cleanup(struct sharedstuff *sspointer);

void
ld_dfree(struct sharedstuff *sspointer,void *addr,uint size);

void
ld_emess(int errorid,char *errordata,struct loader_entry *le);

void
ld_emessinit();

int
ld_emessdump(char *buf, int blen, int adspace);

int (*(
ld_entrypoint(struct loader_entry *le,struct loader_anchor *la))) ();

int
ld_filemap(struct sharedstuff *sspointer, caddr_t addr, size_t size,
	struct file *fp, off_t pos);

void
ld_freelex(struct loader_entry *firstle,struct loader_entry *lastle);

char *
ld_fptopath(struct sharedstuff *sspointer,struct file *fp);

struct loader_entry *
ld_functole(int (*func)());

struct loader_anchor *
ld_functola(int (*func)());

struct loader_entry *
ld_getlib(struct sharedstuff *sspointer,char *path,char *filename,
	  char *member,int create);

/* values for the create parameter */
#define	ld_create_check		0
#define	ld_create_initialcall	1
#define	ld_create_create	2
#define	ld_create_read		3

struct d *
ld_getmap(struct sharedstuff *sspointer);

struct exp_index *
ld_hashfind(struct loader_exports *lx,struct ldhdr *ldhdr,
	    struct ldsym *ldsym,int  *rc,struct loader_entry *le);

int
ld_libraries(struct sharedstuff	*sspointer,struct loader_entry *le,int create);

int (*
ld_loadmodule(struct sharedstuff *sspointer,char *filenameparm,uint flags,
	      char *libpathparm))();

int
ld_mapdata(struct sharedstuff *sspointer);

int
ld_ovflatt(struct loader_anchor *la);

void
ld_ovfldet(struct loader_anchor *la);

struct file *
ld_pathopen(struct sharedstuff *sspointer,char *path,char *filename,
            struct file *initfp);

void
ld_pathclear(struct sharedstuff *sspointer);

void
ld_readprotecttext(char *textorg);

int
ld_resolve(struct sharedstuff	*sspointer);

int
ld_relocate(struct sharedstuff *sspointer);

void
ld_restdataseg(struct sharedstuff *sspointer);

int
ld_sanity(struct sharedstuff *sspointer,
struct loader_entry_extension	*lexpointer);

int
ld_syscall(struct sharedstuff *sspointer,struct loader_entry *le);

struct exp_index *
ld_symfind(struct loader_exports *lx,char   *symstring);

int
ld_textread(
	struct	sharedstuff	*sspointer,
	struct loader_entry_extension	*lexpointer,
	struct file	*fp,
	char	*member,
	char   *origin,
	uint	length,
	heapaddr_t	heap,
	uint	flags);

void *
ld_ualloc(struct sharedstuff	*sspointer,uint	size);

void
ld_uinitheap(struct sharedstuff *sspointer);

void
ld_updatehash(struct loader_exports *lx,struct ldhdr *ldhdr,
	      struct ldsym *ldsym,struct locs *locs);

int
ld_ufree(struct sharedstuff *sspointer,void * addr);

#else /* ! _NO_PROTO */

void
ld_addressppseg();

ulong
ld_allocd();

int
ld_assigndata();

struct loader_exports *
ld_buildhash();

int
ld_cleartext();

void
ld_clearle();

int
ld_clearlib();

void
ld_cleanup();

void
ld_dfree();

void
ld_emess();

void
ld_emessinit();

int
ld_emessdump();

int (*
ld_entrypoint()) ();

int
ld_filemap();

void
ld_freelex();

char *
ld_fptopath();

struct loader_entry *
ld_functole();

struct loader_anchor *
ld_functola();

struct loader_entry *
ld_getlib();

struct d *
ld_getmap();

struct exp_index *
ld_hashfind();

int
ld_libraries();

int (*
ld_loadmodule())();

int
ld_mapdata();

int
ld_ovflatt();

void
ld_ovfldet();

struct file *
ld_pathopen();

void
ld_pathclear();

void
ld_readprotecttext();

int
ld_resolve();

int
ld_relocate();

void
ld_restdataseg();

int
ld_sanity();

int
ld_syscall();

struct exp_index *
ld_symfind();

int
ld_textread();

void *
ld_ualloc();

void
ld_uinitheap();

void
ld_updatehash();

int
ld_ufree();

#endif /* ! _NO_PROTO */

#endif   /* _H_LD_DATA */
