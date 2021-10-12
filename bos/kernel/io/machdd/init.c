static char sccsid[] = "@(#)87	1.15  src/bos/kernel/io/machdd/init.c, machdd, bos41J, 9516a 4/18/95 11:15:30";
/*
 * COMPONENT_NAME: (MACHDD) Machine Device Driver
 *
 * FUNCTIONS: mdinit, nvinit, rwv_check, nvsize, nvblock_zero,
 *	      build_bus_table
 *
 * ORIGINS: 27, 83
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
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/mdio.h>
#include <sys/iplcb.h>
#include <sys/vmker.h>
#include <sys/systemcfg.h>
#include <sys/sys_resource.h>
#include <sys/ioacc.h>
#include <sys/nvdd.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include "md_extern.h"
#ifdef _RSPC
#include <sys/system_rspc.h>
#endif	/* _RSPC */

extern caddr_t BaseCustom;
extern long Base_size;

extern ulong NVRAM_size;
extern ulong NVRAM_base;
extern ulong NVRAM_swbase;
extern struct nvblock nv_blocks[];
extern struct md_bus_info md_bus[];
extern ulong md_valid_buid[];
extern int md_nio_bus;

#define MCB_TOKEN 0xC3C243C3   /* NVRAM MAGIC PALINDROME */
#define NVOK  1
#define NVBAD 0
#define N1_KBYTES 1024
#define N2_KBYTES 2*N1_KBYTES

#define RS6K_IO		3	/* RS6K I/O BUC */

void nvblock_zero();
void build_bus_table();


/*
 * NAME: mdinit
 *
 * FUNCTION:
 *    Initialize machine device driver global data.  Check validity
 *    of NVRAM - Initialize if necessary.
 *
 * EXECUTION ENVIRONMENT:  Initialization only
 *
 * NOTES:
 *    devno : major and minor device numbers
 *     iodn : io device number, 0 if not applicable, -1 driver delete
 *     ilev : (unused)
 *   ddilen : device-dependent info length (0 if none)
 *   ddiptr : device-dependent info, NULL ptr if none
 *
 * RETURN VALUE DESCRIPTION:    always returns 0
 *
 */
int
mdinit(dev_t devno, int iodn, int ilev, int ddilen, char *ddiptr)
{
    IPL_CB_PTR	iplcb ;

    build_bus_table();	/* Create table of known buses */

    BaseCustom = baseload(&Base_size);	/* read base customization info */

    iplcb = (IPL_CB_PTR)(vmker.iplcbptr) ;
    nvinit((IPL_INFO*)(iplcb->s0.ipl_info_offset + vmker.iplcbptr));

    MD_LOCK_ALLOC(); /* Allocate the Machine Device Driver serialization lock */
    MD_LOCK_INIT(); /* Initialize Machine Device Driver serialization lock */

#ifdef _RSPC_UP_PCI
    if (__rspc_up_pci()) {
        lock_alloc(&md_pci_lock, LOCK_ALLOC_PIN, MD_LOCK_CLASS, -1);
        simple_lock_init(&md_pci_lock);
    }
#endif	/* _RSPC_UP_PCI */

#ifdef _RSPC
    if (__rspc()) {
        lock_alloc(&md_nv_lock, LOCK_ALLOC_PIN, MD_LOCK_CLASS, -1);
        simple_lock_init(&md_nv_lock);
    }
#endif	/* _RSPC */

#ifdef _RS6K_SMP_MCA
    if (__rs6k_smp_mca())
	    /* initialize BUMP interface */
	    mdbumpinit();
#endif /* _RS6K_SMP_MCA */
    return 0;
} /* mdinit() */


/*
 ***********************************************************************
 * NAME: nvinit
 *
 * FUNCTION: Controls initialization of NVRAM and DRAM structures to
 *           manage NVRAM.
 *
 * EXECUTION ENVIRONMENT:  Initialization only
 *
 * NOTES:    The size of NVRAM is determined, all static NVRAM areas
 *           are CRC checked, and the dynamic S/W area is checked.
 *           Any areas with bad CRCs are rebuilt and "nvstatus" is
 *           set to indicate the reset.
 *
 * DATA STRUCTURES:
 * 	NVRAM_start : NVRAM start address (modified during init to be
 * 		      NVRAM addr of 1st usable byte of dyn s/w area/
 *      NVRAM_end   : Address of last byte of NVRAM + 1
 *      nvstatus    : status indicator for NVRAM init. (essentially 
 *      	      the return code of NVRAM initialization).
 *      NVRAM       : various areas of NVRAM may be reset if errors are
 *      	      detected.
 *
 * RETURN VALUE DESCRIPTION: 
 *      If a problem is detected with NVRAM, "nvstatus" is set to
 *      indicate the problem.  "nvstatus" is checked during the 
 *      battery test of diagnostics which is run via CRON.  If 
 *      "nvstatus" contains an error indicator, diagnostics generates
 *      an errlog.
 *
 ***********************************************************************
 */

void
nvinit(IPL_INFO *ipl_info) 	/* ptr to IPL_INFO area of IPL_CB so can 
				   check model and section 1 valid flags */
{
    ulong 	 segreg   ;
    int 	 status   ;
    int          newcrc   ;	/* holds CRC generation results		 */
    ulong 	 *pnvaddr ;     /* ptr to area of NVRAM being checked.   */
    ulong        pnvdata  ;
    struct ros_cb *nv_ros ;     /* ptr to NVRAM area 1 (ROS area.)	 */
    int b;
    static long nvsize();
    struct ipl_cb *iplcb;
    struct ipl_directory *idir_ptr;

/* BEGIN nvinit */

/* Find the NVRAM base address */
#ifdef _POWER_RS
    if (__power_rs()) {
	NVRAM_base = NVRAM_BASE;
    }
#endif	/* _POWER_RS */
#ifdef _RS6K
    if (__rs6k()) {
	NVRAM_base = &sys_resource_ptr->nvram;
    }
#endif	/* _RS6K */
#ifdef _RSPC
    if (__rspc()) {
	iplcb = (struct ipl_cb *)vmker.iplcbptr;
	idir_ptr = (struct ipl_directory *)(&iplcb->s0);
	NVRAM_base = (ulong)((char *)iplcb + idir_ptr->nvram_cache_offset);
    }
#endif	/* _RSPC */

    nvled(0x814);

/* 1) Get size of nvram */
    if ((0 == (NVRAM_size = nvsize())) || (NVRAM_base == 0))
    {
	nvled(0x815);
	nvstatus = NVRAM_BAD;
	ASSERT( (NVRAM_size != 0) || (NVRAM_base != 0) );
	return;
    }

/* Setup for 8K or 32K NVRAM size */
    if (NVRAM_size <= 8*N1_KBYTES)
	NVRAM_swbase = NVRAM_base + SWDATA_BASE_8K;
    else
	NVRAM_swbase = NVRAM_base + SWDATA_BASE;

#ifdef _RSPC
    if (__rspc())
	nvrd_xlate_rspc();	/* Fetch the vital data from real NVRAM */
#endif

/* 2) Compute CRC on NVRAM area 1 */
    status = ipl_info->nvram_section_1_valid & SECT1_STATUS_MASK ;
    if (status == SECT1_ROS_INITED)
    {
       /* 
	*-------------------------------------------------------
	* ROS has initialized section 1, so we 
	* inidicate NVRAM has been reset.
	*-------------------------------------------------------
	*/
	status   = 0 ;
	nvstatus = NVRAM_RESET ;
    }
    else
    {
       /* 
	*-------------------------------------------------------
	* It is unclear what has happened to section 1 during 
	* ROS control.
	*    1. With new ROS
	*       a. 00 indicates CRC was/is bad and
	*          ROS could not correctly initialize the area.
	*       b. 01 indicates CRC is OK.
	*    2. With "old" ROS (release 1) (and non Salmon ROS??)
	*       a. 00 CRC bad ??
	*       b. 01 CRC OK ??
	*       in either case, ROS does not init section 1.
	* Given these conditions, follow same processing as
	* release 1 (get 2 chances to correct CRC before 
	* declaring NVRAM bad).
	*-------------------------------------------------------
	*/
	status = 0;
    }

    do 
    {
	newcrc = 0;
	nv_ros = (struct ros_cb *)mdget(MD_NVRAM_IDX, IOCC_RGN, NVRAM_base, 
					sizeof(struct ros_cb), 0,
					MV_BYTE, NULL);
	if (nv_ros && crc32(nv_ros, sizeof(struct ros_cb))) 
	{
	    int i;
	    nvstatus = NVRAM_RESET;
	    newcrc = 1;
	    nv_ros->nv_size = NVRAM_size;
	    nv_ros->nv_version = NV_VERSION;
	    for (i=0; i < 16; i++)
		nv_ros->scsi_adap_id[i] = (uchar) 0x07;
	    for (i = 0; i < 56; i++)
		nv_ros->mem_config[i] = 0;
	    for (i = 0; i < 256; i++)
		nv_ros->mem_data[i] = (uchar) 0x0;
	    bzero(&nv_ros->prevdev, sizeof(struct devdesc));
	    nv_ros->norm_dev_list[0] = (uchar) 0x0;
	    nv_ros->serv_dev_list[0] = (uchar) 0x0;
	    nv_ros->drv0.drv_magic = 0x0;
	    nv_ros->drv1.drv_magic = 0x0;
	    nv_ros->ros_crc = crc32(nv_ros, 
				    sizeof(struct ros_cb)-sizeof(ulong));
	    mdput(MD_NVRAM_IDX, IOCC_RGN, NVRAM_base, sizeof(struct ros_cb),
		  nv_ros, MV_BYTE, NULL);
	}
    } while (newcrc && ++status < 2);

    if (nvstatus == NVRAM_RESET && status == 2) 
    {
	nvstatus = NVRAM_BAD;
        MD_PG_FREE(nv_ros);
	nvled(0x816);
	return;
    }

    if (nv_ros->nv_version != NV_VERSION) {
	nv_ros->nv_version = NV_VERSION;
	nv_ros->ros_crc = crc32(nv_ros, sizeof(struct ros_cb)-sizeof(ulong));
	mdput(MD_NVRAM_IDX, IOCC_RGN, NVRAM_base, sizeof(struct ros_cb), 
		nv_ros, MV_BYTE, NULL);
    }

    MD_PG_FREE(nv_ros);

/* 3) Check Crc of each block in the s/w area */
/* If the crc is bad, we zero the block. */
    for (b=0; b<=MAX_NVBLOCK; b++) {
	if (nv_blocks[b].len == 0)
	    continue;
	if (chk_nvblock_crc(&nv_blocks[b]))
	    nvblock_zero(&nv_blocks[b]);		/* zero the block */
    }

    nvled(0xfff);
} /* END nvinit */


/* 
 ***********************************************************************
 * NAME     : rwv_check
 *
 * FUNCTION : perform the read/write/verify check used to detect
 *            the end of NVRAM.
 *
 * EXECUTION ENVIRONMENT:  Initialization only
 *
 * NOTES    : This function temporarily modifies NVRAM.  If a power 
 *            failure occurs at just the right place, NVRAM may be
 *            corrupted.
 *
 * DATA STRUCTURES : no global data structures are modified.
 *
 * RETURN VALUES :
 * 	NVOK  : the verify was correct and address is valid NVRAM space
 *      NVBAD : the verify failed meaning the address is not valid 
 *              NVRAM space
 *********************************************************************** 
 */
static int
rwv_check(ulong addr)
{
    long  *firstv ;
    long  *oldv ; 
    long  *chkv ;
    long  magic ;
    long  zero  ;
    int   rc    ; 
    int   chk   ;

/* BEGIN rwv_check */

    zero  = 0 ;
    magic = MCB_TOKEN ;
    chk   = NVOK ;

   /* 
    * Due to a problem in the entry level diskless machine H/W,
    * NVRAM wraps in 8K increments up to 32K.  To alleviate
    * this problem, set the 1st long int of NVRAM to zero and
    * then check it later so that the wrap can be detected.
    */
    firstv = (long *) mdget(MD_NVRAM_IDX, IOCC_RGN, NVRAM_base, 
	    sizeof(long), 0, MV_BYTE, &rc) ;
    if (rc)
    {
	MD_PG_FREE(oldv) ;
	return(NVBAD) ;
    }
    mdput(MD_NVRAM_IDX, IOCC_RGN,NVRAM_base, sizeof(zero), &zero, MV_BYTE, &rc);

   /* 
    * Now save the value at the current check address, write
    * out the know pattern, read it back and verify it
    */
    oldv  = (long *) mdget(MD_NVRAM_IDX, IOCC_RGN, addr, 
			sizeof(long), 0, MV_BYTE, &rc);
    if (rc) 
    {
	chk =  NVBAD;
    }
    else
    {
	mdput(MD_NVRAM_IDX, IOCC_RGN, addr, sizeof(long), &magic, MV_BYTE, &rc);
	chkv = (long *) mdget(MD_NVRAM_IDX, IOCC_RGN, addr, sizeof(long), 0, 
		MV_BYTE, NULL);

	if ((rc) || (*chkv != MCB_TOKEN))
	{
	    chk = NVBAD;
	}
	else if (addr > NVRAM_base)
	{
	   /* 
	    * To catch the wrap problem, get the value at the 1st
	    * location of NVRAM and check it against the known
	    * pattern
	    */
	    MD_PG_FREE(chkv) ;
	    chkv = (long *) mdget(MD_NVRAM_IDX, IOCC_RGN, NVRAM_base, 
		      sizeof(long), 0, MV_BYTE, NULL) ;
	    if (*chkv == MCB_TOKEN)
		chk = NVBAD ;
	}
    }

    mdput(MD_NVRAM_IDX, IOCC_RGN, addr, sizeof(long), oldv, MV_BYTE, NULL);
    mdput(MD_NVRAM_IDX, IOCC_RGN, NVRAM_base, sizeof(long),firstv,MV_BYTE,&rc);
    if (rc)
	chk = NVBAD ;

    MD_PG_FREE(firstv) ;
    MD_PG_FREE(oldv);
    MD_PG_FREE(chkv);
    return chk;
} /* END rwv_check */

/* 
 ***********************************************************************
 * NAME     : nvsize
 *
 * FUNCTION : Determine the size of NVRAM
 *
 * EXECUTION ENVIRONMENT:  Initialization only
 *
 * NOTES    :
 *
 * DATA STRUCTURES : no global data affected by this function
 *
 * RETURN VALUES : 
 * 	offset of LAST valid verify (should be 1 byte past end of 
 * 		NVRAM). By using offset, it is the same as the size.
 ***********************************************************************
 */
static long
nvsize()
{
    ulong addr = 0;
    int chk = NVOK;
    struct ipl_cb *iplcb;
    struct ipl_directory *idir_ptr;
    struct system_info *info_ptr;

/* BEGIN nvsize */

#ifdef _RS6K
    if (__rs6k()) {
	/* get the nvram size from the iplcb */
	iplcb = (struct ipl_cb *)vmker.iplcbptr;
	idir_ptr = (struct ipl_directory *)(&iplcb->s0);
	info_ptr = (struct system_info *)((char *)iplcb + 
		idir_ptr->system_info_offset);
	return(info_ptr->nvram_size);
    }
#endif	/* _RS6K */

#ifdef _POWER_RS
    if (__power_rs()) {
	while (chk) 
	{
	    chk = rwv_check(NVRAM_base + addr);
	    if (chk)
	         addr += N2_KBYTES;
	}
	return(addr);
    }
#endif	/* _POWER_RS */

#ifdef _RSPC
    if (__rspc()) {
	/* get the nvram size from the iplcb */
	iplcb = (struct ipl_cb *)vmker.iplcbptr;
	idir_ptr = (struct ipl_directory *)(&iplcb->s0);
	return(idir_ptr->nvram_cache_size);
    }
#endif	/* _RSPC */

    return addr;
} /* END nvsize */


/* 
 ***********************************************************************
 * NAME     : nvblock_zero
 *
 * FUNCTION : Fill a nvram block with 0's
 *
 * EXECUTION ENVIRONMENT:  Initialization only
 *
 * NOTES    :
 *
 * DATA STRUCTURES : no global data affected by this function
 *
 * RETURN VALUES :  None
 ***********************************************************************
 */
void
nvblock_zero(struct nvblock *nvp)
{
    char *blkp;
    int rc;

    blkp = mdget(MD_NVRAM_IDX, IOCC_RGN, nvp->base + NVRAM_swbase, 
	nvp->len+CRC_SIZE, 0, MV_BYTE, &rc);
    if (blkp == NULL)
 	    return;
    bzero(blkp, nvp->len+CRC_SIZE);
    mdput(MD_NVRAM_IDX, IOCC_RGN, nvp->base + NVRAM_swbase, 
	nvp->len+CRC_SIZE, blkp, MV_BYTE, &rc);
    MD_PG_FREE(blkp);
}


/* 
 ***********************************************************************
 * NAME     : build_bus_table
 *
 * FUNCTION : If bucs exist, walk the ROS iplcb buc_info structures to find 
 *            the BUIDs/BIDs of the buses;  else use the old assignments.
 *            Also create a table of all BUIDs found;
 *            this table will be used to verify good BUID when ioctlx 
 *            is used to pass in an arbitrary BUID value.
 *
 * EXECUTION ENVIRONMENT:  Initialization only
 *
 * NOTES    : 	If not POWER_RS, assumes BUCs are present.  Otherwise,
 *           	no buses will be accessable.
 *
 * DATA STRUCTURES : md_bus[] array of md_bus_info structs initialized
 *                   md_valid_buid[] BUID global array initialized
 *
 * RETURN VALUES :  None
 ***********************************************************************
 */
static void 
build_bus_table()
{
	struct ipl_cb *iplcb;
	struct ipl_directory *idir_ptr;
	struct buc_info *buc_ptr;
	ulong num_bucs, buc, buid;
	int i;
	int bus, rgn, region;

	/* First, initialize everything to INVALID */
	for (bus=0; bus<MD_BUS_TABSZ; bus++) {	/* init bus table */
		for (rgn=0; rgn<MD_MAX_RGN; rgn++)
			md_bus[bus].bid[rgn] = INVALID;
		md_bus[bus].map = INVALID;
		md_bus[bus].io_excpt = INVALID;
		md_bus[bus].hw_busnum = INVALID;
	}
	for (buid=0; buid<MD_MAX_BUID; buid++)		/* init buid table */
		md_valid_buid[buid] = INVALID;		/* invalid flag */

#ifdef _POWER_RS
	/* Setup buses for Power RS */
	if (__power_rs()) {	/* power_rs - no bucs */
		for (bus=0; bus<MD_MAX_IOCC; bus++) {
		   md_bus[bus].bid[IO_RGN] = BUS0 | BYPASS_TCW | (bus << 20);
		   md_bus[bus].bid[IOCC_RGN] = BUS0 | IOCC_SELECT | (bus << 20);
		   md_bus[bus].map = MAP_T1;
		   md_bus[bus].io_excpt = TRUE;
		}
		/* Setup special entry for NVRAM */
		md_bus[MD_NVRAM_IDX].bid[IOCC_RGN] = BUS0 | IOCC_SELECT;
		md_bus[MD_NVRAM_IDX].map = MAP_T1;
		md_bus[MD_NVRAM_IDX].io_excpt = TRUE;
		return;	/* we're done */
	}
#endif	/* _POWER_RS */

#if defined _RS6K || defined _RSPC
	/* Setup NVRAM entry */
	if (__rs6k() || __rspc()) {
		md_bus[MD_NVRAM_IDX].map = NO_MAP;
		md_bus[MD_NVRAM_IDX].io_excpt = FALSE;
	}
#endif	/* _RS6K | _RSPC */
	
	/* Look at the buc_info structs */
	iplcb = (struct ipl_cb *)vmker.iplcbptr;
	idir_ptr = (struct ipl_directory *)(&iplcb->s0);
	buc_ptr = (struct buc_info *)((char *)iplcb + 
		idir_ptr->buc_info_offset);
	num_bucs = buc_ptr->num_of_structs;
	buid = 0;
	for (buc = 0; buc < num_bucs; buc++)
	{	/* point to next buc structure */
		buc_ptr = (struct buc_info *)(((char *)iplcb + 
			idir_ptr->buc_info_offset) + 
			(buc * buc_ptr->struct_size));

		switch(buc_ptr->device_type) {
		  case RS6K_IO:
		    if (buc_ptr->IOCC_flag) {		 /* IOCC? */
			/* if location field exists, get bus # */
			if (buc_ptr->struct_size >
			  ((uint)&buc_ptr->location - (uint)buc_ptr)) {
			    if (buc_ptr->location[2] >= '0')
				bus = buc_ptr->location[2] - '0';
			    else
				bus = 0;
			}
			else
			    bus = 0;
			if (bus < MD_MAX_BUS) {
				md_bus[bus].bid[IO_RGN] = 
					BYPASS_TCW|BUID_T|ADDR_INCR|
					(buc_ptr->buid_data[0].buid_value<<20);
				md_bus[bus].bid[IOCC_RGN] = 
					IOCC_SELECT | BUID_T | ADDR_INCR |
					(buc_ptr->buid_data[0].buid_value<<20);
				md_bus[bus].map = MAP_T1;
				md_bus[bus].io_excpt = TRUE;
			}
		    }
		    if (buc_ptr->num_of_buids)
			for (i=0; i<buc_ptr->num_of_buids; i++)
			    if (buid < MD_MAX_BUID)
				md_valid_buid[buid++] =
 				    buc_ptr->buid_data[i].buid_value << 20;
		    break;

		  case A_BUS_BRIDGE:
		    if ((buc_ptr->device_id_reg == PCI_BUS_ID) || 
		        (buc_ptr->device_id_reg == ISA_BUS_ID)) {
			    bus = buc_ptr->location[2] - '0';
			    if (bus < MD_MAX_BUS) {
			      if (buc_ptr->IOCC_flag)	/* NIO? */
				  md_nio_bus = bus;	/* save it */
			      for (rgn=0; rgn<buc_ptr->num_of_buids; rgn++) {
				region = BID_REGION(
					buc_ptr->buid_data[rgn].buid_value);
				if ((region==PCI_IOMEM) || (region==ISA_IOMEM))
				    md_bus[bus].bid[IO_RGN] = 
					buc_ptr->buid_data[rgn].buid_value;
				if ((region==ISA_BUSMEM)||(region==PCI_BUSMEM))
				    md_bus[bus].bid[MEM_RGN] = 
					buc_ptr->buid_data[rgn].buid_value;
				if (region == PCI_CFGMEM)
				    md_bus[bus].bid[CFG_RGN] = 
					buc_ptr->buid_data[rgn].buid_value;
				}
				md_bus[bus].map = MAP_T0;
				md_bus[bus].io_excpt = FALSE;
				/*
				 * Save the hardware configured bus number
				 */
				md_bus[bus].hw_busnum = buc_ptr->bscr_value;
#ifdef _RSPC
				/*
				 * Save the PCI base/data addresses
				 */
				if (__rspc()) {
                                   if (buc_ptr->device_id_reg == PCI_BUS_ID)
                                   {
                                      if (buc_ptr->mem_addr1 == 0)
                                      {
                                       md_bus[bus].pci_base_addr = PCI_CFG_ADDR;
                                       md_bus[bus].pci_data_addr = PCI_CFG_DATA;
                                      }
                                      else
                                      {
				       md_bus[bus].pci_base_addr = 
                                                             buc_ptr->mem_addr1;
				       md_bus[bus].pci_data_addr = 
                                                             buc_ptr->mem_addr2;
                                      }
                                   }
				}
#endif	/* _RSPC */
			    }
		    }
		    break;
		}
	}
}

