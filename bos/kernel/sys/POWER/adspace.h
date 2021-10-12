/* @(#)92	1.35  src/bos/kernel/sys/POWER/adspace.h, sysvmm, bos411, 9434B411a 8/23/94 12:23:19 */
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Management
 *
 * FUNCTIONS: Machine-dependent address-space manipulation macros.
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_ADSPACE
#define _H_ADSPACE

/*
 *       NOTE:  See <sys/m_types.h> for definition of address space.
 */

#include  <sys/types.h>
#include  <sys/seg.h>
#include  <sys/systemcfg.h>

#ifdef	_KERNEL

/*
 *  Routines which operate on virtual memory handles, moving them into
 *  and out of segment registers and address space arrays.
 */

/*  vm_geth -- Virtual memory get handle
 *              Parameters:  (1) caddr_t     32-bit virtual address, from which
 *                                             segment register is deduced
 *              Returns:         vmhandle_t  Handle, from segment register
 */

extern vmhandle_t vm_geth();

/*  vm_seth -- Virtual memory set handle
 *              Parameters:  (1) vmhandle_t  Handle to set into segment register
 *                           (2) caddr_t     32-bit virtual address, from which
 *                                             segment register is deduced
 *              Returns:         nothing
 */

extern void vm_seth();


/*  getadsp() - Get address of process'a adspace
 *		Parameters: None
 *
 *		returns:	adspace_t *
 */
extern adspace_t *getadsp();

/*  as_geth -- Address space get handle
 *              Parameters:  (1) adspace_t * Ptr to address space struct
 *                           (2) caddr_t     32-bit virtual address, from which
 *                                             segment register is deduced
 *              Returns:         vmhandle_t  Handle from segment register entry
 */
extern vmhandle_t as_geth();

/*  as_puth -- Address space put handle
 *              Parameters:  (1) adspace_t * Ptr to address space struct
 *                           (2) vmhandle_t  handle originally from adspace
 *              Returns:         nothing
 */
extern void as_puth();

/* as_seth -- Address space set handle
 *              Parameters:  (1) adspace_t * Ptr to address space struct
 *                           (2) vmhandle_t  Handle to set into segment register
 *                           (3) caddr_t     32-bit virtual address, from which
 *                                             segment register is deduced
 *              Returns:         nothing
 */
extern void as_seth();

/*  as_getsrval -- Address space get handle without attach count
 *              Parameters:  (1) adspace_t * Ptr to address space struct
 *                           (2) caddr_t     32-bit virtual address, from which
 *                                             segment register is deduced
 *              Returns:         vmhandle_t  Handle from segment register entry
 */
extern vmhandle_t as_getsrval();


/*
 *  Routines which operate on keys, in handles:
 */
#define	VM_PRIV		0		/* privileged access authority */
#define	VM_UNPRIV	1		/* unprivileged access authority */

/*  vm_getkey -- Virtual memory get key
 *              Parameters:  (1) vmhandle_t  Handle from which to extract key
 *              Returns:         int         Key
 */

#define vm_getkey(k) ((((ulong)k)>>30)&0x00000001)

/*  vm_setkey -- Virtual memory set key
 *              Parameters:  (1) vmhandle_t  Handle into which to set key
 *                           (2) int         Key
 *              Returns:         vmhandle_t  Handle, with new key set
 */

#ifdef _POWER
/*
 * The vm_setkey macro should only be used in the base kernel (vm_handle
 * is the exported interface) but the _KERNSYS define wasn't used previously
 * to enforce this.  Uses of vm_setkey outside of the base kernel continue
 * to get the old definition.
 */
#ifdef _KERNSYS
extern int key_value;
#define vm_setkey(h,k) ((vmhandle_t)(((k) ? key_value : 0)| \
                                      (((ulong)h)&0x9FFFFFFF)))
#else
#define vm_setkey(h,k) ((vmhandle_t)(((((ulong)k)&0x00000001)<<30)| \
                                      (((ulong)h)&0xBFFFFFFF)))
#endif /* _KERNSYS */
#endif /* _POWER */

/* 
 *  Routines which permanently allocate regions 
 */

/*  vm_ralloc -- Virtual memory region allocate
 *              Parameters:  (1) caddr_t     32-bit virtual address (start of
 *                                             segment) which is to be allocated
 *              Returns:         int         0 if allocate succeeded
 *                                           ENOMEM if required segment
 *						 register not available
 */

#define vm_ralloc(addr) vm_allocsr(&csa->as,addr)

extern int vm_allocsr();

/*  as_ralloc -- Address space region allocate
 *              Parameters:  (1) adspace_t * Ptr to address space structure
 *                           (2) caddr_t     32-bit virtual address (start of
 *                                             segment) which is to be allocated
 *              Returns:         int         0 if allocate succeeded
 *                                           ENOMEM if required segment
 *						 register not available
 */

#define as_ralloc(adsp,addr) vm_allocsr(adsp,addr)


/*  Routines which "attach" an addressable object to the kernel's address
 *  space or to a specified address space.
 *
 *  These routines:
 *     (1) Temporarily allocate a segment register or address space entry
 *     (2) Load the register or entry with the specified vmhandle_t
 *     (3) Modify the specifed virtual address so that it will select the
 *           proper segment register/address space entry
 *     (4) Return the modified address for use as a 32-bit address
 */

/*  vm_att -- Virtual memory attach
 *              Parameters:  (1) vmhandle_t  Handle to use
 *                           (2) caddr_t     Offset of object (can be passed in as a
 *                                             32-bit address; high bits are ignored)
 *              Returns:         caddr_t     32-bit address to use to address object
 */

extern caddr_t vm_att();

/*  as_att -- Address space attach 
 *              Parameters:  (1) adspace_t * Ptr to address space structure
 *                           (2) vmhandle_t  Handle to use
 *                           (3) caddr_t     Offset of object (can be passed in as a
 *                                             32-bit address; high bits are ignored)
 *              Returns:         caddr_t     32-bit address to use to address object
 */

extern caddr_t as_att();

/*  vm_uatt -- Virtual memory current user space attach
 *              This routine allows easy access from the kernel to the
 *              user's address space, such as by copyin() and copyout().
 *              Parameters:  (1) caddr_t     32-bit address in process address
 *                                             space of the current process
 *              Returns:         caddr_t     32-bit address to use to address object
 */

#define vm_uatt(a) (vm_att(as_geth(&U.U_adspace, (a)),(a)))


/*
 *  Routines to release or detach a permanently- or temporarily-allocated 
 *    region, which was allocated via vm_ralloc(), as_ralloc(), vm_att(), 
 *    as_att(), or vm_uatt().
 */

/*  vm_det -- Virtual memory detach
 *              Parameters:  (1) caddr_t     32-bit address
 *              Returns:         nothing
 */

extern void vm_det();


/*  as_det -- Address space detach
 *              Parameters:  (1) adspace_t * Ptr to address space
 *                           (2) caddr_t     32-bit address
 *              Returns:       
 *			      0 = successful
 *		 	      EINVAL = detatching invalid address
 */

extern int as_det();


/*  
 *  Routine to initialize an address space, so that nothing is addressable
 *  and no regions are allocated.
 */

/*  as_init -- Address space initialization
 *              Parameters:  (1) adspace_t * Ptr to address space
 *              Returns:         nothing
 */

#ifdef _POWER           /* POWER version: */
#define as_init(adsp) { int i;                                \
                        for (i=0; i<NSEGS; i++)               \
                            (adsp)->srval[i] = NULLSEGVAL;    \
                        (adsp)->alloc = 0; }   
#endif /* _POWER */

/*  
 *  Routine to copy the current address space, making everything that
 *  is addressable in the current address space addressable in the new
 *  address space.
 */

/*  as_fork -- Copy address space for fork()
 *              Parameters:  (1) adspace_t * Ptr to address space
 *              Returns:         nothing
 */

#define as_fork(a)					\
	(a)->alloc = u.u_save.as.alloc,			\
	forksregs((a)->srval)


/*  
 *  Routines to assist in using segment registers to address I/O space,
 *  for memory-mapped I/O operations.
 */


/*  
 *  Routines to assist in using segment registers to address I/O space.
 */

/*  BUSIO_ATT -- I/O address attach
 *              Parameters:  (1) ulong       Bus ID value for segment register
 *                           (2) ulong       I/O address
 *              Returns:         caddr_t     32-bit "I/O" address
 *
 *  BUSIO_DET -- I/O address detach
 *		Parameters:  (1) ulong	     32-bit "I/O" address
 *		Returns:		     nothing
 */

#ifdef _POWER           /* POWER version: */
#define BUSIO_ATT(bid,io_addr)	io_att((~(0xf) & (bid)),io_addr)
#define BUSIO_DET(addr)		io_det(addr)
#endif /* _POWER */

/*  
 *  Routines to assist in using segment registers to address memory-
 *  mapped I/O space.
 */

/*  BUSMEM_ATT -- I/O Memory address attach
 *              Parameters:  (1) ulong       Bus ID value for segment register
 *                           (2) ulong       I/O memory address
 *					     note:  an actual memory address in
 *					     the range desired must be supplied
 *					     if the most significant nibble is
 *					     non-zero.
 *              Returns:         caddr_t     32-bit "I/O" memory address
 *
 *  BUSMEM_DET -- I/O Memory address detach
 *		Parameters:  (1) ulong	     32-bit "I/O" memory address
 *		Returns:		     nothing
 */

#ifdef _POWER           /* POWER version: */
#define BUSMEM_ATT(bid,mem_addr)			\
	((__power_pc()) ?						\
	(	io_att((~(0xf) & (bid)) | ((mem_addr)>>28), mem_addr))	\
	:								\
	(	io_att((~(0xf) & (bid)) | ((mem_addr)>>28),  		\
		      ((bid) & 0x40) ? 0x04000000 | ((mem_addr) & 0x00ffffff) : mem_addr)))
#define BUSMEM_DET(addr)		io_det(addr)
#endif /* _POWER */

/*  
 *  Routines to assist in using segment registers to address IOCC
 *  address space.
 */

/*  IOCC_ATT -- IOCC address attach
 *              Parameters:  (1) ulong       Bus ID value for segment register
 *                           (2) ulong       IOCC address
 *              Returns:         caddr_t     32-bit IOCC address
 *
 *  IOCC_DET -- IOCC address detach
 *		Parameters:  (1) ulong	     32-bit IOCC address
 *		Returns:		     nothing
 */

#ifdef _POWER           /* POWER version: */
#define IOCC_ATT(bid,iocc_addr)			\
		io_att(((0x1ff00000 & (bid)) | 0x800c00e0), iocc_addr)
#define IOCC_DET(addr)			io_det(addr)
#endif /* _POWER */

/*  io_att -- (T=1) I/O address attach (T=1 access)
 *              Parameters:  (1) ulong       Value for segment register
 *                           (2) ulong       Offset within segment
 *              Returns:         caddr_t     32-bit "I/O" address
 */
caddr_t io_att();



/*  io_det -- (T=1) I/O address detach
 *		Parameters: (1) ulong        32-bit "I/O" address
 *		Returns:		     nothing
 */
void io_det();

/*  iomem_att -- (T=0) I/O address attach (T=0 access)
 *		Parameters:  (1) struct io_map * IO map structure
 *              Returns:         void *     32-bit "I/O" address
 */
struct io_map {
	int key;			/* structure version number */
	int flags;			/* flags for mapping */
	int size;			/* size of address space needed */
	int bid;			/* bus ID */
	long long busaddr;		/* bus address */
};


#define	IO_MEM_MAP			1	/* structure version number */
#define IOM_RDONLY			0x0001	/* read only mapping */

#ifdef _NO_PROTO
void *iomem_att();
#else
void *iomem_att(struct io_map *iop);
#endif

/*  iomem_det -- (T=0) I/O address detach
 *		Parameters: (1) ulong        32-bit "I/O" address
 *		Returns:		     nothing
 */

#ifdef _NO_PROTO
void iomem_det();
#else
void iomem_det(void *eaddr);
#endif

/* IO_MAP_INIT -- This macro can be used to initialize an io_map structure.
 * 	Additional function (smaller mapping granularity, readonly) is
 *	available by filling out individual fields in mapping structure
 *
 *      WARNING: region to map can not cross 256Meg (SEGSIZE) boundary
 */
#define IO_MAP_INIT(iomap, bid, busaddr) 				\
{									\
	(iomap)->key = IO_MEM_MAP;					\
	(iomap)->flags = 0;						\
	(iomap)->size = SEGSIZE;					\
	(iomap)->bid = bid;						\
	(iomap)->busaddr = busaddr;					\
}

#endif /* _KERNEL */

#endif /* _H_ADSPACE */

