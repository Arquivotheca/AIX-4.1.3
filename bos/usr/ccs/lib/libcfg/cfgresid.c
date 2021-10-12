static char sccsid[] = "@(#)50 1.6 src/bos/usr/ccs/lib/libcfg/cfgresid.c, libcfg, bos41J, 9522A_b 5/31/95 11:36:09";
/*
 *   COMPONENT_NAME: LIBCFG	   cfgresid.c
 *
 *   FUNCTIONS:	get_resid_packet
 *		get_resid_dev
 *		get_resid_mem
 *		get_resid_cpu
 *		get_resid_version
 *		get_resid_vpd
 *		get_irq_packets
 *		get_dma_packets
 *		get_io_packets
 *		get_mem_packets
 *		get_pci_descriptor
 *		get_isa_descriptor
 *		get_bax_descriptor
 *		get_chipid_descriptor
 *		get_L2cache_size
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */


#include	<stdio.h> 
#include	<sys/types.h>
#include	<sys/mdio.h>
#include	<sys/ioctl.h>
#include	<sys/iplcb.h> 
#include	<sys/rosinfo.h> 
#include 	<cf.h>
#include	<sys/residual.h>
#include	<sys/pnp.h>
#include	<sys/pcipnp.h>
#include	<sys/isapnp.h>
#include	<sys/genadpnp.h>

#include	<cfgresid.h> 
#include	"cfgdebug.h"


/* packet sizes */
#define         SMALL   0
#define         LARGE   1
#define         BUF_SIZE   2048
#define RESID_OFFSET( A ) ((ulong)&(((RESIDUAL *)0)->A))


/* Declare functions	*/
int get_resid_packet(int, uchar, uchar, int *, uchar **);
int get_resid_dev(ulong *,CFG_DEVICE **);
int get_resid_mem(long long *,long long *);
int get_resid_cpu(ushort *,CFG_CPU **);
int get_resid_version(uchar *);
int get_resid_vpd(CFG_VPD *);
int get_irq_packets(int, char, int*, CFG_irqpack_t **);
int get_dma_packets(int, char, int*, CFG_dmapack_t **);
int get_io_packets(int, char, int*, CFG_iopack_t **);
int get_mem_packets(int, char, int*, CFG_mempack_t **);
int get_pci_descriptor(int, char, int*, CFG_pci_descriptor_t **);
int get_isa_descriptor(int, char, int*, CFG_isa_descriptor_t **);
int get_bax_descriptor(int, char, int*, CFG_bax_descriptor_t **);
int get_chipid_descriptor(int, char, int*, char **);
int get_L2cache_size(int, char, ulong *);
static int get_packet(int, uchar, uchar, int, int, int *, uchar **);
static int getoff(unsigned int *);
static int iplcb_get(void *, int, int);
static void decode_dev(u_int, char *);

/* Global variables */
static int	fd = -1;	/* file descriptor  for accessing machdd */





/* ---------------------------------------------------------------------------*
 * 
 * NAME: get_resid_packet 
 *
 * FUNCTION: obtain residual heap field when given the index 
 *	     and the tag type
 *
 * RETURNS: 	0 on success or	appropriate error code
 *   
 * ---------------------------------------------------------------------------*/

int
get_resid_packet(dev, offset_type, tag, num, ptr)
int     dev;			/* index into the device array  */
uchar   offset_type;            /* AllocatedOffset, PossibleOffset,
                                   or CompatibleOffset into Heap*/
uchar   tag;                    /* PnP resource tag             */
int     *num;                   /* number of packets in buffer  */
uchar   **ptr;                  /* return packet buffer pointer */
{
	int rc;

	/* Simply call get_packet() with vendor type and addr type set to -1 */
	rc = get_packet(dev, offset_type, tag, -1, -1, num, ptr);
	return(rc);
}



/* ---------------------------------------------------------------------------*
 * 
 * NAME: get_packet 
 *
 * FUNCTION: obtain specified packets from residual heap when given
 *           the device index  and the tag type.
 *
 * RETURNS: 	0 on success or	appropriate error code
 *   
 * ---------------------------------------------------------------------------*/

static int
get_packet(dev, offset_type, tag, vtype, atype, num, ptr)
int     dev;		/* index into the device array  */
uchar   offset_type;	/* AllocatedOffset, PossibleOffset,
			   or CompatibleOffset into Heap*/
uchar   tag;		/* PnP resource tag             */
int	vtype;		/* vendor type to search for in vendor packets	   */
int	atype;		/* address type to search for in general memory pkts */
int     *num;		/* number of packets in buffer  */
uchar   **ptr;		/* return packet buffer pointer */
{

	int	rc;
	ulong	resid;		/* offset in IPLCB to resid data */
	ulong	start;		/* start of device's resid data */
	ulong	dev_offset;	/* Pointer to device's heap pointer */
	ulong	heap_ptr;	/* Device's heap pointer */
	char	fini;		/* Finished processing Flag	   */
	ulong	iplcb_offset;
	PnP_TAG_PACKET *pp;     /* Pointer to union of all PNP pkt types*/
	uchar	p_info[4];	/* Packet Size and Vendor Type */
	uchar	p_tag;		/* Tag byte from packet */
	int	p_vtype;	/* Vendor packet type from packet */
	int	p_atype;	/* address type from general memory packet */
	char	*buf_ptr;
	ulong	buf_offset;
	ulong	buf_size;
	int	tag_size;	/* Tag type large or small */
	int	count;		/* Running count of packets found */
	ulong	size;


	/* Initially set returned count to 0, in case of error */
	*num = 0;
	*ptr = NULL;

	/* Get offset within IPLCB of resid data */
	rc = getoff(&resid);
	if (rc)
		return(rc);

	/* Determine offset to device's heap pointers */
	switch(offset_type) {
		case 'a':	dev_offset =
				    RESID_OFFSET(Devices[dev].AllocatedOffset);
 	                      	break;

		case 'p':	dev_offset =
				    RESID_OFFSET(Devices[dev].PossibleOffset);
                       		break;

		case 'c':	dev_offset =
				    RESID_OFFSET(Devices[dev].CompatibleOffset);
 	                      	break;

        	default:
        	       break;
	} 

	/* Add in offset to resid data */
	dev_offset += resid;

	/* Now get device's heap pointer */
	if (iplcb_get((void *)&heap_ptr, dev_offset, 4)) {
		return(E_DEVACCESS);
	}

	/* Get start of device's heap data */
	start = resid + RESID_OFFSET(DevicePnPHeap[0]) + heap_ptr;


	/* Initialize local variables - used in searching for packets */
	if (tag != 0x84) {
		/* Not looking for a vendor packet so set vendor type to -1 */
		vtype = -1;
	}
	if (vtype != 0x09) {
		/* Not looking for a general memory pkt so set addrtype to -1 */
		atype = -1;
	}

	

	/* Get all occurrances of this tag from heap for this device */ 
	count = 0;		/* Number of packets found starts at 0 */
	iplcb_offset = start;
	buf_offset = 0;
	buf_size = 0;
	buf_ptr = NULL;
	fini = FALSE;

	while(!fini) {
		/* get tag byte from the heap */
		if (iplcb_get((void *)&p_tag,iplcb_offset, 1)) {
			return(E_DEVACCESS);
		}

		/* tag_type returns 0-small or 1-large		*/
		tag_size = tag_type(p_tag);

		/* if the tag is small and not the end of the device's data */
		if (tag_size==SMALL && p_tag!=0x78 && p_tag!=0x79) {
			/* get size of packet */
			size = tag_small_count(p_tag) + 1;	
			p_vtype = -1;
			p_atype = -1;
		}
		else if (tag_size == LARGE) {
			/* if the tag is large then we need to read in the */ 
			/* next four bytes so we can calculate the size and */
			/* have vendor packet type and general memory packet */
			/* type in case we need them. If we don't need these */
			/* last two bytes, then we are reading two bytes more */
			/* than we need, but that should never be a problem. */
			if (iplcb_get((void *)&p_info,(iplcb_offset+1), 4)) {
				return(E_DEVACCESS);
			}
			if (vtype != -1) {
				/* vendor packet type is significant to match */
				p_vtype = p_info[2];
			}
			else {
				/* set p_vtype to match vtype */
				p_vtype = vtype;
			}
			if (atype != -1) {
				/* gen mem pkt type is significant to match */
				p_atype = p_info[3];
			}
			else {
				/* set p_atype to match atype */
				p_atype = atype;
			}

			size = p_info[0] + (p_info[1]<<8) +3;
		}
		else {
			/* Reached end of device's heap data */
			fini = TRUE;
		}

		if ( p_tag==tag && p_vtype==vtype && p_atype==atype) {
			/* tag matched the one we were looking for. */
			/* Note: if either vtype or atype are -1, then they */
			/* are not significant to the search criteris. But */
			/* in this case, p_vtype or p_atype will also be -1 */
			/* so the equality will be true and we have a match. */

			if (buf_offset + size > buf_size) {
				if (buf_size == 0) {
					buf_size = ((buf_offset + size)/BUF_SIZE +1) * BUF_SIZE;
					buf_ptr = (char *)malloc(buf_size);
				}
				else {
					buf_size = ((buf_offset + size)/BUF_SIZE +1) * BUF_SIZE;
					buf_ptr = (char *)realloc(buf_ptr,buf_size);
				}
			}
			if (iplcb_get(buf_ptr+buf_offset,iplcb_offset,size)) {
				return(E_DEVACCESS);
			}
			buf_offset += size;
			count++;
		}

		iplcb_offset += size;
	}

	*num = count;
	*ptr = buf_ptr;
	return(0);
} /* end format_heap */

/* ---------------------------------------------------------------------------*
 * 
 * NAME: get_resid_dev
 *
 * FUNCTION: Obtain the residual data field of ActualNumDevices and the data
 *           associated with all of the devices.  The data returned contains
 *           all of the device data, including the heap pointers, but
 *           not the heap data.
 *
 * RETURNS:
 *   This is a NULL function, so always returns 0.
 * ---------------------------------------------------------------------------*/

int get_resid_dev(tot_dev,ptr)
ulong		*tot_dev;	/* return total number of devices	*/
CFG_DEVICE      **ptr;		/* ptr to buffer for return device data */
{
	int             i,rc;
	ulong		resid;	/* offset in IPLCB to resid data */
	ulong		offset;
	ulong		buf;
	CFG_DEVICE	*tptr;


	/* Initialize returned pointer in case of error */
	*ptr = (CFG_DEVICE *)NULL;


	/* Get offset within IPLCB of resid data */
	rc = getoff(&resid);
	if (!rc){
		/* set offset to resid field for number of devices */
		offset = resid + RESID_OFFSET(ActualNumDevices);

		/* get the number of devices	*/
        	if (iplcb_get((void *)tot_dev,offset,sizeof(ulong))){
           		return(E_DEVACCESS);
        	}

		/* malloc for total number of devices	*/
		tptr=(CFG_DEVICE *) malloc( *tot_dev * sizeof(CFG_DEVICE));
		if (tptr == NULL) {
                	DEBUG_0("malloc failed\n");
                	return(E_MALLOC);
        	}

		/* for each device get the devices 		*/
		/* data and calculate the devices pnp number	*/
		offset = resid + RESID_OFFSET(Devices[0]);
 		for (i=0;i<*tot_dev;i++) {

			/* Get the resid data device structure */
			if (iplcb_get(tptr+i,offset,sizeof(PPC_DEVICE))) {
				return(E_DEVACCESS);
			}

			/* Calculate and Return PNP number */
			decode_dev((uint)(tptr+i)->deviceid.devid,
							(tptr+i)->pnpid);

			/* Point to next resid device entry */
			offset += sizeof(PPC_DEVICE);
		}
		*ptr = tptr;
	}
	return(rc);
}


/* ---------------------------------------------------------------------------*
 *
 * NAME: get_resid_mem
 *
 * FUNCTION: Obtain residual data field of TotalMemory and GoodMemory.
 *	     This function returns the values as long long for future models.
 *
 * RETURNS:
 *   This function returns 0 for success, non-zero for failure.
 * ---------------------------------------------------------------------------*/

int get_resid_mem(tot_mem,good_mem)
long	long *tot_mem;		/* return Total memory in Kbytes	*/
long	long *good_mem;		/* return Total good memory in Mbytes   */
{
	int             rc;
	ulong		totmem;
	ulong		goodmem;
	ulong		offset;
	ulong		resid;	/* offset in IPLCB to resid data */

	/* Get offset within IPLCB of resid data */
	rc = getoff(&resid);
	if (!rc) {

		/* Get offset to total memory size */
		offset = resid + RESID_OFFSET(TotalMemory);

		/* obtain 4 bytes of residual data */
		if (iplcb_get(&totmem,offset, 4)) {
			return(E_DEVACCESS);
		}

		/* Get offset to total memory size */
		offset = resid + RESID_OFFSET(GoodMemory);

		if (iplcb_get(&goodmem,offset, 4)) {
			return(E_DEVACCESS);
		}

		*tot_mem = (long long)totmem;
		*good_mem = (long long)goodmem;

	} 

	return(rc);
}
/* ---------------------------------------------------------------------------*
 *
 * NAME: get_resid_version
 *
 * FUNCTION: obtain residual data field of Version and Revision.
 *           Return them as a string ex. "1.0"
 *
 * RETURNS:
 *   This is a NULL function, so always returns 0.
 * ---------------------------------------------------------------------------*/
int get_resid_version(ptr)
uchar      *ptr;             /* ptr to return version.revision string */
{
	int             rc;
	uchar		tmp1;
	uchar		tmp2;
	ulong		offset;
	ulong		resid;	/* offset in IPLCB to resid data */


	/* Get offset within IPLCB of resid data */
	rc = getoff(&resid);
	if (!rc) {

		/* Get offset to version number */
		offset = resid + RESID_OFFSET(Version);

		if (iplcb_get((void *)&tmp1,offset, 1)) {
			return(E_DEVACCESS);
		}

		/* Get offset to revision number */
		offset = resid + RESID_OFFSET(Revision);

		if (iplcb_get((void *)&tmp2,offset, 1)) {
			return(E_DEVACCESS);
		}
		sprintf(ptr,"%d.%d",tmp1,tmp2);
	}

	return(rc);
}


/* ---------------------------------------------------------------------------*
 *
 * NAME: get_resid_cpu
 *
 * FUNCTION: obtain residual data field of ActualNumCpus.
 *           Obtain the residual data field ActualNumCpus and the data
 *           associated with the CPUs.
 *
 * RETURNS:
 *   This is a NULL function, so always returns 0.
 * ---------------------------------------------------------------------------*/

int get_resid_cpu(num_cpu,ptr)
ushort		*num_cpu;		/* return number of cpus	*/
CFG_CPU		**ptr;			/* ptr to return CPU structs	*/
{
	int		i,rc;
	ulong		resid;
	ulong		offset;
	ushort		ncpus;
	PPC_CPU		*ppcptr;
	CFG_CPU		*tptr;
	int		cputype_index;
	char		cputype[][32]={"PowerPC","PowerPC_601",
					"PowerPC_603","PowerPC_604"};


	/* Initialize return values */
	*ptr=(CFG_CPU *)NULL;
	*num_cpu = 0;

	/* Get offset within IPLCB of resid data */
	rc = getoff(&resid);
	if (!rc) {

		/* Get number of CPU's */
		offset = resid + RESID_OFFSET(MaxNumCpus);
		if (iplcb_get(&ncpus,offset, 2)) {
			return(E_DEVACCESS);
		}

		/* malloc space for getting resid data CPU table */
		ppcptr = (PPC_CPU *) malloc(ncpus * sizeof(PPC_CPU));
		if (ppcptr == NULL) {
			DEBUG_0("malloc failed\n");
			return(E_MALLOC);
		}

		/* Get resid data CPU table */
		offset = resid + RESID_OFFSET(Cpus[0]);
		if (iplcb_get(ppcptr,offset,(ncpus * sizeof(PPC_CPU)))) {
			return(E_DEVACCESS);
		}

		/* malloc space to return CPU array */
		tptr = (CFG_CPU *) malloc(ncpus * sizeof(CFG_CPU));
		if (tptr == NULL) {
			DEBUG_0("malloc failed\n");
			return(E_MALLOC);
		}


		/* Now copy info from resid CPU table to config CPU table */
		for ( i=0; i<ncpus; i++){

			/* copy CPU type */
			(tptr+i)->cputype = (ppcptr+i)->CpuType;

			/* copy CPU state */
			(tptr+i)->cpustate = (ppcptr+i)->CpuState;

			/* Set CPU type string value */
			switch (((tptr+i)->cputype >> 16) & 0xF ) {
				case  0x1 : 	cputype_index = 1;
						break;

				case  0x3 :
				case  0x6 :	cputype_index = 2;
						break;

				case  0x4 :	cputype_index = 3;
						break;

				default   : 	cputype_index = 0;

			} /* end switch */

			strcpy((tptr+i)->cpuname , cputype[cputype_index]);
		} 
	}

	/* free memory used to get resid CPU table */
	free(ppcptr);

	*num_cpu = ncpus;
	*ptr = tptr;
	return(rc);
}



/* ---------------------------------------------------------------------------*
 *
 * NAME: get_resid_vpd
 *
 * FUNCTION: obtain residual data VPD section.
 *
 * RETURNS:
 *   Returns 0 on success, non-zero on error.
 * ---------------------------------------------------------------------------*/
int
get_resid_vpd(vpd)
CFG_VPD	*vpd;
{
	int             i,rc;
	ulong		resid;	/* offset in IPLCB to resid data */
	ulong		offset;
	ulong		buf;
	CFG_DEVICE	*tptr;


	/* Get offset within IPLCB of resid data */
	rc = getoff(&resid);
	if (!rc){
		/* set offset to resid field for number of devices */
		offset = resid + RESID_OFFSET(VitalProductData);

		/* get the number of devices	*/
        	if (iplcb_get((void *)vpd,offset,sizeof(VPD))) {
           		return(E_DEVACCESS);
        	}
	}
	return(rc);
}



/* ---------------------------------------------------------------------------*
 *
 * NAME: getoff
 *
 * FUNCTION: Get the offset to the residual data.
 *
 * RETURNS:
 *   0- success
 *   E_DEVACCESS if iplcb access error
 * ---------------------------------------------------------------------------*/
static int
getoff(offs)
unsigned int *offs;
{
	IPL_DIRECTORY   iplcb_dir;      /* IPL control block directory */


#ifdef USEFILE
	/* if we are reading from a file then set the offset to 0 */
	*offs=0x0;
#else
	/* if not then get actual offset from iplcb */
	if (iplcb_get(&iplcb_dir,128,sizeof(iplcb_dir))) {
		return(E_DEVACCESS);
	}
	*offs=iplcb_dir.residual_offset;
#endif

	return(0);
}/*end getoff */


/* ---------------------------------------------------------------------------*
 *
 * NAME: iplcb_get
 *
 * FUNCTION: Read in a section of the IPL control block.
 *
 * RETURNS:
 *   0- success
 *   E_DEVACCESS if error
 * ---------------------------------------------------------------------------*/

static int
iplcb_get(dest, address, num_bytes)
void	*dest;
int	address;
int	num_bytes;
{
	MACH_DD_IO	mdd;
	int		rc;


	/* here we either get the data from a file or read it in via machdd */

	if (fd == -1) {
		/* machdd not open yet */
#ifdef USEFILE
	if ((fd = (int)fopen("test.file","rb")) == NULL)
		return(E_DEVACCESS);
#else
	if ((fd = open("/dev/nvram",0)) < 0) 
		return(E_DEVACCESS);
#endif
	}

#ifdef USEFILE
	fseek((FILE *)fd, address, SEEK_SET);
       	rc = fread(dest, num_bytes,1,(FILE *)fd);
#else
	mdd.md_addr = (long)address;
	mdd.md_data = dest;
	mdd.md_size = (long)num_bytes;
	mdd.md_incr = MV_BYTE;

	rc = ioctl(fd,MIOIPLCB,&mdd);
	if (rc) {
		DEBUG_0("Error reading Residual Data");
		return(E_DEVACCESS);
	}
#endif

	return(0);
}

/* ---------------------------------------------------------------------------*
 *
 * NAME: decode_dev
 *
 * FUNCTION:  Decode the devid into a string
 *
 * RETURNS: A pointer to the pnp device number
 * 
 * ---------------------------------------------------------------------------*/
static void
decode_dev(u_int devid, char *pnp_code)
{
	int     i,j;
	uchar	tmp;


	pnp_code[0] = ((devid >> 26) & 0x0000001F)+'@';
	pnp_code[1] = ((devid >> 21) & 0x0000001F)+'@';
	pnp_code[2] = ((devid >> 16) & 0x0000001F)+'@';

	for (i=3, j=3; i<7; i++, j--) {
		/* Pick out hex digit */
		tmp = (devid >> (j*4)) & 0x0000000F;

		/* Convert to ascii character */
        	if(tmp > 9)
                	pnp_code[i] = tmp -9 +'@';
        	else
                	pnp_code[i] = tmp +'0';
        }

	/* Add null delimiter */
	pnp_code[7]='\0';
	return;
}

/* ---------------------------------------------------------------------------*
 * 
 * NAME: get_irq_packets()
 *
 * FUNCTION: 
 *      Fills out an irqpack_t array with irq information for the device 
 *      specified by the index parameter.
 *
 * RETURNS:
 *      A return code from cf.h  :
 *
 *      E_OPEN      - Open of /dev/nvram failed
 *      E_DEVACCESS - MDD ioctl failed
 *      E_MALLOC    - malloc failed
 *      ???         - index parameter invalid (this is an internal error)
 *   
 * ---------------------------------------------------------------------------*/
int
get_irq_packets(index, heap_offset, numpackets, packets) 
int index;
char heap_offset;
int *numpackets;
CFG_irqpack_t **packets;
{
	int num, num1, num2, num3;
	uchar *buf1;
	uchar *buf2;
	uchar *buf3;
	unsigned short mask;
	CFG_irqpack_t *pptr;
	CFG_irqpack_t *tptr;
	struct _S4_Pack *irq_p1;
	struct _S4_Pack *irq_p2;
	struct _L4_Pack *irq_p3;
	int inc;
	int rc;


	/* Init return values */
	*packets = (CFG_irqpack_t *)NULL;
	*numpackets = 0;

	/* Get packet data */
	/* Going for the simple IRQ packet */
	rc = get_packet(index, heap_offset, 0x22, -1, -1, &num1, &buf1);
	if (rc != E_OK)
		return rc;

	/* Going for the more complex IRQ packet */
	rc = get_packet(index, heap_offset, 0x23, -1, -1, &num2, &buf2);
	if (rc != E_OK)
		return rc;

	/* Going for the System Interrupt Descriptor packet */
	rc = get_packet(index,  heap_offset, 0x84, 0x0B, -1, &num3, &buf3);
	if (rc != E_OK)
		return rc;

	/* Malloc storage */
	num = num1 + num2 + num3;
	if (num) {
		pptr = (CFG_irqpack_t *)calloc(1, num * sizeof(CFG_irqpack_t));
		if (pptr == NULL)
			return E_MALLOC;
	}
	else
		return E_OK;


	/* Convert resid data formats to return struct format */
	tptr = pptr;

	if (num1) {
		/* Handle simple IRQ packets */
		irq_p1 = (struct _S4_Pack *)buf1;

		for ( ; num1-- ; tptr++) {
			mask = (irq_p1->IRQMask[1] << 8) + irq_p1->IRQMask[0];
			for (tptr->value=0; !(mask & 0x1);
					tptr->value+=1, mask >>= 1);
			tptr->inttype = 1;
			tptr->intctlr = 0;
			tptr->flags = HIGH_EDGE;
			irq_p1 = (struct _S4_Pack *)((char *)irq_p1 + 3);
		}
		free(buf1);
	}

	if (num2) {
		/* Handle the more complex IRQ packets */
		irq_p2 = (struct _S4_Pack *)buf2;

		for ( ; num2-- ; tptr++) {
			mask = (irq_p2->IRQMask[1] << 8) + irq_p2->IRQMask[0];
			for (tptr->value=0; !(mask & 0x1);
					tptr->value+=1, mask >>= 1);
			tptr->inttype = 1;
			tptr->intctlr = 0;
			tptr->flags = irq_p2->IRQInfo;
			irq_p2 = (struct _S4_Pack *)((char *)irq_p2 + 4);
		}
		free(buf2);
	}

	if (num3) {
		/* Handle the System Interrupt Descriptor packets */
		irq_p3 = (struct _L4_Pack *)buf3;

		for ( ; num3-- ; tptr++) {
			/* It is a system interrupt descriptor */
			tptr->inttype = irq_p3->L4_Data.Data[1];
			tptr->intctlr = irq_p3->L4_Data.Data[2];
			tptr->value = (irq_p3->L4_Data.Data[4] << 8) + 
						irq_p3->L4_Data.Data[3];
			if (tptr->value & 0x80) 
				tptr->flags = HIGH_EDGE;
			else
				tptr->flags = HIGH_LEVEL;

			/* Go to next resid packet */
			inc = (irq_p3->Count1 << 8) + irq_p3->Count0 + 3;
			irq_p3 = (struct _L4_Pack *)((char *)irq_p3 + inc);
		}
		free(buf3);
	}

	*packets = pptr;
	*numpackets = num;

	return(E_OK); 
} 

/* ---------------------------------------------------------------------------*
 * 
 * NAME: get_dma_packets()
 *
 * FUNCTION: 
 *      Fills out an CFG_dmapack_t array with dma information for the device 
 *      specified by the index parameter.
 *
 * RETURNS:
 *      A return code from cf.h  :
 *
 *      E_OPEN      - Open of /dev/nvram failed
 *      E_DEVACCESS - MDD ioctl failed
 *      E_MALLOC    - malloc failed
 *      ???         - index parameter invalid (this is an internal error)
 *   
 * ---------------------------------------------------------------------------*/
int get_dma_packets(index, heap_offset, numpackets, packets) 
int index;
char heap_offset;
int *numpackets;
CFG_dmapack_t **packets;
{
	int num;
	int tnum;
	uchar *buf;
	struct _S5_Pack *dma_p;
	CFG_dmapack_t *pptr;
	CFG_dmapack_t *tptr;
	unsigned char mask;
	int rc;


	/* Init return values */
	*packets = (CFG_dmapack_t *)NULL;
	*numpackets = 0;

	/* Get packet data */
	rc = get_packet(index,  heap_offset, 0x2a, -1, -1, &num, &buf);
	if (rc != E_OK)
		return rc;

	/* Convert resid data format to return struct format */
	if (num) {
		pptr = (CFG_dmapack_t *)calloc(1, num * sizeof(CFG_dmapack_t));
		if (pptr == NULL)
			return(E_MALLOC);

		tptr = pptr;
		dma_p = (struct _S5_Pack *)buf;

		tnum = num;
		for ( ; tnum-- ; dma_p++, tptr++) {

			/* Get channel */
			mask = dma_p->DMAMask;
			for (tptr->value = 0; !(mask & 0x1) ; 
				tptr->value += 1, mask >>= 1);

			/* Get flags */
			tptr->flags = dma_p->DMAInfo;
		}
		free(buf);
		*packets = pptr;
		*numpackets = num;
	}

	return E_OK; 
} 


/* ---------------------------------------------------------------------------*
 * 
 * NAME: get_io_packets()
 *
 * FUNCTION: 
 *      Fills out an CFG_iopack_t array with io information for the device 
 *      specified by the index parameter.
 *
 * RETURNS:
 *      A return code from cf.h  :
 *
 *      E_OPEN      - Open of /dev/nvram failed
 *      E_DEVACCESS - MDD ioctl failed
 *      E_MALLOC    - malloc failed
 *      ???         - index parameter invalid (this is an internal error)
 *   
 * ---------------------------------------------------------------------------*/
int get_io_packets(index, heap_offset, numpackets, packets) 
int index;
char heap_offset;
int *numpackets;
CFG_iopack_t **packets;
{
	int num, num1, num2, num3;
	uchar *buf1;
	uchar *buf2;
	uchar *buf3;
	CFG_iopack_t *pptr;
	CFG_iopack_t *tptr;
	struct _S9_Pack *io_p1;
	struct _S8_Pack *io_p2;
	GenericAddr *io_p3;
	int inc;
	int rc;


	/* Init return values */
	*packets = (CFG_iopack_t *)NULL;
	*numpackets = 0;

	/* Get packet data */
	/* Going for fixed I/O port packets */
	rc = get_packet(index, heap_offset, 0x4b, -1, -1, &num1, &buf1);
	if (rc != E_OK)
		return rc;

	/* Going for variable I/O port packets */
	rc = get_packet(index, heap_offset, 0x47, -1, -1, &num2, &buf2);
	if (rc != E_OK)
		return rc;

	/* Going for Generic Address Descriptor packets */
	rc = get_packet(index, heap_offset, 0x84, 0x09, ISAIOAddr,&num3,&buf3);
	if (rc != E_OK)
		return rc;


	/* Malloc storage */
	num = num1 + num2 + num3;
	if (num) {
		pptr = (CFG_iopack_t *)calloc(1, num * sizeof(CFG_iopack_t));
		if (pptr == NULL)
			return(E_MALLOC);
	}
	else
		return(E_OK);

	/* Convert resid data format to return struct format */
	tptr = pptr;

	if (num1) {
		/* Handle the fixed I/O port packets */
		io_p1 = (struct _S9_Pack *)buf1;

		for ( ; num1-- ; tptr++, io_p1++) {
			tptr->sigbits = 10;
			tptr->min   = (io_p1->Range[1] << 8) + io_p1->Range[0];
			tptr->max   = (io_p1->Range[1] << 8) + io_p1->Range[0];
			tptr->incr  = 1;
			tptr->width = io_p1->IONum;
		}
		free(buf1);
	}

	if (num2) {
		/* Handle the variable I/O port packets */
		io_p2 = (struct _S8_Pack *)buf2;

		for ( ; num2-- ; tptr++, io_p2++)
		{
			if (io_p2->IOInfo & 0x01)
				tptr->sigbits = 16;
			else
				tptr->sigbits = 10;

			tptr->min = (io_p2->RangeMin[1] << 8)
						+ io_p2->RangeMin[0];
			tptr->max = (io_p2->RangeMax[1] << 8)
						+ io_p2->RangeMax[0];
			tptr->incr  = io_p2->IOAlign;
			tptr->width = io_p2->IONum;
		}
		free(buf2);
	}

	if (num3) {
		/* Handle the Generic Address Descriptor packets */
		io_p3 = (GenericAddr *)buf3;

		for ( ; num3-- ; tptr++) {
			tptr->sigbits = io_p3->AddrInfo;
			tptr->min     = io_p3->Address[0]
						+ (io_p3->Address[1] << 8)
						+ (io_p3->Address[2] << 16)
						+ (io_p3->Address[3] << 24);
			tptr->max     = tptr->min;
			tptr->incr    = 1;
			tptr->width   = io_p3->Length[0]
						+ (io_p3->Length[1] << 8)
						+ (io_p3->Length[2] << 16)
						+ (io_p3->Length[3] << 24);

			/* Go to next resid packet */
			inc = (io_p3->Count1 << 8) + io_p3->Count0 + 3;
			io_p3 = (GenericAddr *)((char *)io_p3 + inc);
		}
		free(buf3);
	}

	*packets = pptr;
	*numpackets = num;

	return E_OK; 
} 

/* ---------------------------------------------------------------------------*
 * 
 * NAME: get_mem_packets()
 *
 * FUNCTION: 
 *      Fills out an CFG_mempack_t array with mem information for the device 
 *      specified by the index parameter.
 *
 * RETURNS:
 *      A return code from cf.h  :
 *
 *      E_OPEN      - Open of /dev/nvram failed
 *      E_DEVACCESS - MDD ioctl failed
 *      E_MALLOC    - malloc failed
 *      ???         - index parameter invalid (this is an internal error)
 *   
 * ---------------------------------------------------------------------------*/
int get_mem_packets(index, heap_offset, numpackets, packets) 
int index;
char heap_offset;
int *numpackets;
CFG_mempack_t **packets;
{
	int num, num1, num2;
	uchar *buf1;
	uchar *buf2;
	CFG_mempack_t *pptr;
	CFG_mempack_t *tptr;
	struct _L1_Pack *mem_p1;
	GenericAddr *mem_p2;
	int inc;
	int rc;


	/* Init return values */
	*packets = (CFG_mempack_t *)NULL;
	*numpackets = 0;

	/* Get packet data */
	/* Going for the bus memory packets */
	rc = get_packet(index,  heap_offset, 0x81, -1, -1, &num1, &buf1);
	if (rc != E_OK)
		return rc;

	/* Going for the Generic Address Descriptor packets */
	rc = get_packet(index, heap_offset, 0x84, 0x09, IOMemAddr,&num2,&buf2);
	if (rc != E_OK)
		return rc;


	/* Malloc storage */
	num = num1 + num2;
	if (num) {
		pptr = (CFG_mempack_t *)calloc(1, num * sizeof(CFG_mempack_t));
		if (pptr == NULL)
			return(E_MALLOC);
	}
	else
		return(E_OK);

	/* Convert buffer format to return struct format */
	tptr = pptr;

	if (num1) {
		/* Handle the bus memory packets */
		mem_p1 = (struct _L1_Pack *)buf1;

		for ( ; num1-- ; mem_p1++, tptr++) {
			tptr->sigbits = 24;
			tptr->min     = (mem_p1->Data[2] << 16)
					+ (mem_p1->Data[1] << 8);
			tptr->max     = (mem_p1->Data[4] << 16)
					+ (mem_p1->Data[3] << 8);
			tptr->incr    = (mem_p1->Data[6] << 8 )
					+ mem_p1->Data[5];
			tptr->width   = (mem_p1->Data[8] << 16)
					+ (mem_p1->Data[7] << 8);
		}
		free(buf1);
	}

	if (num2) {
		/* Handle the Generic Address Descriptor packets */
		mem_p2 = (GenericAddr *)buf2;

		for ( ; num2-- ; tptr++) {
			tptr->sigbits = mem_p2->AddrInfo;
			tptr->min     = mem_p2->Address[0]
						+ (mem_p2->Address[1] << 8)
						+ (mem_p2->Address[2] << 16)
						+ (mem_p2->Address[3] << 24);
			tptr->max     = tptr->min;
			tptr->incr    = 1;
			tptr->width   = mem_p2->Length[0]
						+ (mem_p2->Length[1] << 8)
						+ (mem_p2->Length[2] << 16)
						+ (mem_p2->Length[3] << 24);

			/* Go to next resid packet */
			inc = (mem_p2->Count1 << 8) + mem_p2->Count0 + 3;
			mem_p2 = (GenericAddr *)((char *)mem_p2 + inc);
		}
		free(buf2);
	}

	*packets = pptr;
	*numpackets = num;

	return(E_OK); 
} 

/* ---------------------------------------------------------------------------*
 * 
 * NAME: get_pci_descriptor()
 *
 * FUNCTION: 
 *      Fills out an CFG_pci_descriptor_t array with 
 *	information for the device
 *      specified by the index parameter. There never be more than one PCI
 *      Bridge Descriptor per device, and this function returns only the first.
 *
 * RETURNS:
 *      A return code from cf.h  :
 *
 *      E_OPEN      - Open of /dev/nvram failed
 *      E_DEVACCESS - MDD ioctl failed
 *      E_MALLOC    - malloc failed
 *      ???         - index parameter invalid (this is an internal error)
 *   
 * ---------------------------------------------------------------------------*/
int get_pci_descriptor(index, heap_offset, numpackets, packets) 
int index;
char heap_offset;
int *numpackets;
CFG_pci_descriptor_t **packets;
{
	int num,i;
	uchar *buf;
	PCIInfoPack *pci_p;
	IntMap *int_p;
	int numslots;
	unsigned short mask;
	CFG_pci_slot_t *slot_p;
	CFG_pci_descriptor_t *pptr;
	int rc;


	/* Init return values */
	*packets = (CFG_pci_descriptor_t *)NULL;
	*numpackets = 0;

	/* Get packet data */
	rc = get_packet(index,  heap_offset, 0x84, 0x03, -1, &num, &buf);
	if (rc != E_OK)
		return(rc);

	if (num == 0)
		/* No PCI bridge descriptor packet */
		return(E_OK);


	/* Malloc storage */
	pci_p = (PCIInfoPack *)buf;
	numslots = ((pci_p->Count1 << 8) + pci_p->Count0 - 21) / sizeof(IntMap);
	pptr = (CFG_pci_descriptor_t *)calloc(1, sizeof(CFG_pci_descriptor_t) 
			+ (numslots - 1) * sizeof(CFG_pci_slot_t));
	if (pptr == NULL)
		return(E_MALLOC);


	/* Convert buffer format to return struct format */
	pptr->numslots = numslots;
	pptr->busnum = pci_p->BusNumber;

	int_p = pci_p->Map;
	slot_p = pptr->slotdata;
	for (i=0 ; i<numslots ; i++) {
		(slot_p+i)->slotnumber = (int_p+i)->SlotNumber; 
		(slot_p+i)->devfunc = (int_p+i)->DevFuncNumber;
		(slot_p+i)->inttype = (int_p+i)->IntCtrlType;
		(slot_p+i)->intctlr = (int_p+i)->IntCtrlNumber;

		mask = (int_p+i)->Int[0];
		mask = ((mask & 0xff00) >> 8) + ((mask & 0x00ff) << 8);
		(slot_p+i)->inta=mask;

		mask = (int_p+i)->Int[1];
		mask = ((mask & 0xff00) >> 8) + ((mask & 0x00ff) << 8);
		(slot_p+i)->intb = mask;

		mask = (int_p+i)->Int[2];
		mask = ((mask & 0xff00) >> 8) + ((mask & 0x00ff) << 8);
		(slot_p+i)->intc = mask;

		mask = (int_p+i)->Int[3];
		mask = ((mask & 0xff00) >> 8) + ((mask & 0x00ff) << 8);
		(slot_p+i)->intd = mask;
	}

	free(buf);
	*packets = pptr;
	*numpackets = 1;

	return(E_OK); 
} 

/* ---------------------------------------------------------------------------*
 * 
 * NAME: get_bax_descriptor()
 *
 * FUNCTION: 
 *      Fills out an CFG_bax_descriptor_t array with bus address translation 
 *      information for the device specified by the index parameter.
 *
 * RETURNS:
 *      A return code from cf.h  :
 *
 *      E_OPEN      - Open of /dev/nvram failed
 *      E_DEVACCESS - MDD ioctl failed
 *      E_MALLOC    - malloc failed
 *      ???         - index parameter invalid (this is an internal error)
 *   
 * ---------------------------------------------------------------------------*/
int
get_bax_descriptor(index, heap_offset, numpackets, packets) 
int index;
char heap_offset;
int *numpackets;
CFG_bax_descriptor_t **packets;
{
	int num, tnum;
	uchar *buf;
	struct _L4_Pack *bax_p;
	CFG_pci_slot_t *slot_p;
	CFG_bax_descriptor_t *pptr;
	CFG_bax_descriptor_t *tptr;
	int rc;
	int inc;


	/* Init return values */
	*packets = (CFG_bax_descriptor_t *)NULL;
	*numpackets = 0;

	/* Get packet data */
	rc = get_packet(index,  heap_offset, 0x84, 0x05, -1, &num, &buf);
	if (rc != E_OK)
		return(rc);

	if (num == 0) {
		/* No bridge address translation descriptor packets */
		return(E_OK);
	}

	/* Malloc storage  for return structures */
	pptr = (CFG_bax_descriptor_t *)calloc(1,
					num * sizeof(CFG_bax_descriptor_t));
	if (pptr == NULL)
		return(E_MALLOC);


	/* Convert buffer format to return struct format */
	tptr = pptr;
	bax_p = (struct _L4_Pack *)buf;
	tnum = num;
	for ( ; tnum-- ; tptr++) {
		tptr->flags = bax_p->L4_Data.Data[1];
		tptr->type  = bax_p->L4_Data.Data[2];
		tptr->conv  = bax_p->L4_Data.Data[3];
		tptr->min   = bax_p->L4_Data.Data[13]
		                  + (bax_p->L4_Data.Data[14] << 8) 
		                  + (bax_p->L4_Data.Data[15] << 16) 
		                  + (bax_p->L4_Data.Data[16] << 24)
		                  + (bax_p->L4_Data.Data[17] << 32)
		                  + (bax_p->L4_Data.Data[18] << 40)
		                  + (bax_p->L4_Data.Data[19] << 48)
		                  + (bax_p->L4_Data.Data[20] << 56);
		tptr->max   = bax_p->L4_Data.Data[21]       
		                  + (bax_p->L4_Data.Data[22] << 8) 
		                  + (bax_p->L4_Data.Data[23] << 16) 
		                  + (bax_p->L4_Data.Data[24] << 24)
		                  + (bax_p->L4_Data.Data[25] << 32)
		                  + (bax_p->L4_Data.Data[26] << 40)
		                  + (bax_p->L4_Data.Data[27] << 48)
		                  + (bax_p->L4_Data.Data[28] << 56);
		tptr->max   = tptr->min + (tptr->max - 1);
		tptr->sys   = bax_p->L4_Data.Data[5]       
		                  + (bax_p->L4_Data.Data[6] << 8) 
		                  + (bax_p->L4_Data.Data[7] << 16) 
		                  + (bax_p->L4_Data.Data[8] << 24)
		                  + (bax_p->L4_Data.Data[9] << 32)
		                  + (bax_p->L4_Data.Data[10] << 40)
		                  + (bax_p->L4_Data.Data[11] << 48)
		                  + (bax_p->L4_Data.Data[12] << 56);
	

		/* Increment pointer to next packet */
		inc = (bax_p->Count1 << 8) + bax_p->Count0 + 3;
		bax_p = (struct _L4_Pack *)((char *)bax_p + inc);
	}

	free(buf);
	*packets = pptr;
	*numpackets = num;

	return(E_OK); 
} 

 /* ---------------------------------------------------------------------------*
 *
 * NAME: get_chipid_descriptor()
 *
 * FUNCTION:
 *      Returns pointer to list of chip id's
 *
 * RETURNS:
 *      A return code from cf.h  :
 *
 *      E_OPEN      - Open of /dev/nvram failed
 *      E_DEVACCESS - MDD ioctl failed
 *      E_MALLOC    - malloc failed
 *      ???         - index parameter invalid (this is an internal error)
 *
 * ---------------------------------------------------------------------------*/
int get_chipid_descriptor(index, heap_offset, numpackets, packets)
int index;
char heap_offset;
int *numpackets;
char **packets;
{
	int num;
	uchar *buf;
	unsigned char *pptr;
	unsigned char *tptr;
	int     i;
	int     rc;
	long 	devid;


	/* Init return values */
	*packets = (char *)NULL;
	*numpackets = 0;

	/* Get packet data */
	rc = get_packet(index, heap_offset, 0x75, -1, -1, &num, &buf);
	if (rc != E_OK)
		return rc;
	
	if(num){
	        if ((pptr=(uchar *)malloc( (8*num) * sizeof(uchar))) == NULL) {
                    DEBUG_0("malloc failed\n");
                    return(E_MALLOC);
        	}

        	/* convert and pass back the data */
		tptr = pptr;
		for(i=0;i<num;i++){
			devid=(long)( (buf[2+(i*6)]<<24) +
			      (buf[3+(i*6)]<<16) +
			      (buf[4+(i*6)]) +
			      (buf[5+(i*6)]<<8) );
			decode_dev((uint)devid,tptr);
			tptr += 8;
		}

		free(buf);
		*packets = pptr;
		*numpackets = num;
	}

	return(E_OK);
} /* end get_chipid_descriptor() */


/* ---------------------------------------------------------------------------*
 *
 * NAME: get_L2cache_size()
 *
 * FUNCTION:
 *      Returns pointer to L2 cache size
 *
 * RETURNS:
 *      A return code from cf.h  :
 *
 *      E_OPEN      - Open of /dev/nvram failed
 *      E_DEVACCESS - MDD ioctl failed
 *      E_MALLOC    - malloc failed
 *      ???         - index parameter invalid (this is an internal error)
 *
 * ---------------------------------------------------------------------------*/
int get_L2cache_size(index, heap_offset, size)
int   index;		/* index into the device array  */
char  heap_offset;	/* 'a'-allocated, 'p'-  possible, or 'c' -
			    compatible indicator 	*/
ulong *size;		/* return size of L2 cache	*/
{
        int 		num;
        uchar	 	*buf;
        int    		rc;


        /* Init return values */
        *size = 0;

        /* Get packet data */
        rc = get_packet(index, heap_offset, 0x84, 0x02, -1, &num, &buf);
        if (rc != E_OK)
                return(rc);
        if (num) {
		/* pass back the size value 	  */
		*size= buf[4] + (buf[5]<<8) + (buf[6]<<16) + (buf[7]<<24);
		free(buf);
	}

	return(rc);
} /* end get_L2cache_size() */


/* ---------------------------------------------------------------------------*
 * 
 * NAME: get_isa_descriptor()
 *
 * FUNCTION: 
 *      Fills out an CFG_isa_descriptor_t array with information for the device
 *      specified by the index parameter. There may never be more than one ISA
 *      Bridge Descriptor per device, and this function returns the first.
 *
 * RETURNS:
 *      A return code from cf.h  :
 *
 *      E_OPEN      - Open of /dev/nvram failed
 *      E_DEVACCESS - MDD ioctl failed
 *      E_MALLOC    - malloc failed
 *      ???         - index parameter invalid (this is an internal error)
 *   
 * ---------------------------------------------------------------------------*/
int get_isa_descriptor(index, heap_offset, numpackets, packets) 
int index;
char heap_offset;
int *numpackets;
CFG_isa_descriptor_t **packets;
{
	int num, i;
	uchar *buf;
	ISAInfoPack *isa_p;
	unsigned short mask;
	CFG_isa_descriptor_t *pptr;
	int rc;


	/* Init return values */
	*numpackets = 0;
	*packets = (CFG_isa_descriptor_t *)NULL;

	/* Get packet data */
	rc = get_packet(index,  heap_offset, 0x84, 0x0A, -1, &num, &buf);
	if (rc != E_OK)
		return rc;

	if (num == 0) {
		/* No ISA bridge descriptor */
		return(E_OK);
	}

	/* Malloc storage */
	pptr = (CFG_isa_descriptor_t *)calloc(1, sizeof(CFG_isa_descriptor_t));
	if (pptr == NULL)
		return E_MALLOC;

	/* Convert buffer format to return struct format */
	isa_p = (ISAInfoPack *)buf;
	pptr->inttype = isa_p->IntCtrlType;
	pptr->intctlr = isa_p->IntCtrlNumber;

	for (i = MAX_ISA_INTS ; i-- ; ) {
		mask = (isa_p->Int)[i];
		mask = ((mask & 0xff00) >> 8) + ((mask & 0x00ff) << 8);
		(pptr->irq)[i] = mask;
	}

	free(buf);
	*packets = pptr;
	*numpackets = 1;

	return(E_OK); 
} /* end of get_isa_descriptor */ 
