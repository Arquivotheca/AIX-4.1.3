static char sccsid[] = "@(#)57	1.24  src/bos/kernel/io/machdd/nvdd.c, machdd, bos411, 9428A410j 4/22/94 12:23:57";
/*
 * COMPONENT_NAME: (MACHDD) Machine Device Driver
 *
 * FUNCTIONS: nvrw(), nvled(), nvblock_crc(),
 *            chk_nvblock_crc(), Crc_Block(),
 *            Check_Crc()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <sys/systemcfg.h>
#include <sys/nvdd.h>
#include "md_extern.h"

void nvblock_crc();
void *md_mapit();
void md_unmapit();
extern struct md_bus_info md_bus[];
extern int enter_dbg;

ulong NVRAM_size;		/* size of NVRAM */
ulong NVRAM_base;		/* base address of NVRAM */
ulong NVRAM_swbase;		/* base address of dynamic SW data area */
int nvstatus = NVRAM_OK;

/* This array defines the nvram blocks: */
struct nvblock nv_blocks[] = {
	{ 0, 0 },					/* 0 - not used */
	{ 0, 0 },					/* 1 - not used */
	{ NVRAM_DUMP_BASE, NVRAM_DUMP_SIZE},		/* 2 - dump */
	{ NVRAM_ERRLOG_BASE, NVRAM_ERRLOG_SIZE}		/* 3 - error log */
};


/***********************************************************************
 * NAME: nvrw
 *
 * FUNCTION: Read from or write to NVRAM reserved block
 *
 * EXECUTION ENVIRONMENT:  Process or interrupt; must be pinned
 *
 * NOTES: If write, then update the block's crc
 *
 * RETURN VALUE DESCRIPTION: -1 if error, else the number of bytes
 *                            read or written
 *
 ***********************************************************************
 */

int
nvrw(	int rbarw,	/* 1 = read (NVREAD), 0 = write */
	int type,	/* index of reserved block */
	uchar *dptr,	/* pointer to data buffer */
	int dstart,	/* offset into reserved block */
	int dlen) 	/* number bytes to r/w */
{
    int rc = 0;
    struct nvblock *nvp;

/* BEGIN nvrw */
    if (type > MAX_NVBLOCK)
	return -1;
    nvp = &nv_blocks[type];
    if (dlen > (nvp->len-dstart))	/* limit check */
	dlen = nvp->len - dstart;
    if (NVREAD == rbarw) {
	mdget(MD_NVRAM_IDX, IOCC_RGN, NVRAM_swbase + nvp->base + dstart, 
	    dlen, dptr, MV_BYTE, &rc);
    } else {
	mdput(MD_NVRAM_IDX, IOCC_RGN, NVRAM_swbase + nvp->base + dstart, 
	    dlen, dptr, MV_BYTE, &rc);
	nvblock_crc(nvp);	/* update the crc of this block */
#ifdef _RSPC
	if (__rspc())
	    nvwr_xlate_rspc();	/* update real NVRAM */
#endif _RSPC
    }
    return (rc ? -1 : dlen);
} /* END nvrw */



/*
 ***********************************************************************
 * NAME: nvled
 *
 * FUNCTION: Sets the LEDs to the given value.
 *
 * EXECUTION ENVIRONMENT:  Process or interrupt; must be pinned
 *
 * NOTES:
 *    this routine will eventually set up a queue of requests
 *    so that leds will be displayed in succession
 *    
 * RETURN VALUE: None
 *
 ***********************************************************************
 */

int
nvled(ulong ledv) 		/* Value to set into LEDs	*/
{
   ulong *led;

   if (enter_dbg) {	/* debugging support */
       printf("LED{%03X}\n", ledv);
   }
   ledv = ledv << 20;
   led = (ulong *) (NVRAM_base + LED_ADDRESS);
   mdput(MD_NVRAM_IDX, IOCC_RGN, led,
      sizeof(long), &ledv, MV_WORD, NULL);
} /* END nvled */


/* 
 ******************************************************************
 * The following routines are the CRC checking and generating 
 * routines used to check NVRAM s/w data area structures.
 ****************************************************************** 
 */

/*
 ***********************************************************************
 * NAME: Crc_Block
 *
 * FUNCTION: Obtain crc for block
 *
 * EXECUTION ENVIRONMENT:  Process or interrupt; must be pinned
 *
 * NOTES: Calculates crc and stores it in data->target
 *    
 * RETURN VALUE: None 
 *
 ***********************************************************************
 */
static int
Crc_Block(struct d_st *data)
{
    ulong bcrc;
    ulong *pnvaddr = data->target;

    bcrc = crc32(data->source, data->size);
    *pnvaddr = bcrc;
}


/*
 ***********************************************************************
 * NAME: nvblock_crc
 *
 * FUNCTION: Update the crc of the specified NVRAM block
 *
 * EXECUTION ENVIRONMENT:  Process or interrupt; must be pinned
 *
 * RETURN VALUE: None 
 *
 ***********************************************************************
 */
void
nvblock_crc(struct nvblock *nvp)
{
    struct d_st data;
    void *ptr;

    if (INVALID == (ptr = md_mapit(MD_NVRAM_IDX, IOCC_RGN, 
      nvp->base + NVRAM_swbase, nvp->len))) {
        return;
    } else {
        data.source = (ulong)ptr + nvp->base + NVRAM_swbase;
        data.size   = nvp->len;
        /* crc is stored in the last byte of the block, after len bytes
           of data; total block size is len + 4 bytes */
        data.target = data.source + data.size; 
        if (md_bus[MD_NVRAM_IDX].io_excpt == TRUE)
            md_exp_hand(Crc_Block, &data, ptr);
        else
	    Crc_Block(&data);
    }
    md_unmapit(MD_NVRAM_IDX, ptr);
}


/*
 ***********************************************************************
 * NAME: Check_Crc
 *
 * FUNCTION: Calculate crc and store in data->size (will be 0
 *           if the block crc was correct)
 *
 * EXECUTION ENVIRONMENT:  Process or interrupt; must be pinned
 *
 * RETURN VALUE: None 
 *
 ***********************************************************************
 */
static int
Check_Crc(struct d_st *data)
{
    data->size = crc32(data->source, data->size);
}


/*
 ***********************************************************************
 * NAME: chk_nvblock_crc
 *
 * FUNCTION: Check the crc of the specified NVRAM block
 *
 * EXECUTION ENVIRONMENT:  Process or interrupt; must be pinned
 *
 * RETURN VALUE: 0 if crc correct
 *
 ***********************************************************************
 */
int
chk_nvblock_crc(struct nvblock *nvp)	/* Check block crc */
{
    struct d_st data;
    void *ptr;

    if (INVALID == (ptr = md_mapit(MD_NVRAM_IDX, IOCC_RGN,
      nvp->base + NVRAM_swbase, nvp->len + CRC_SIZE))) {
        return -1;
    } else {
        data.source = (ulong)ptr + nvp->base + NVRAM_swbase;
        data.size   = nvp->len + CRC_SIZE;
        if (md_bus[MD_NVRAM_IDX].io_excpt == TRUE)
            md_exp_hand(Check_Crc, &data, ptr);
        else
	    Check_Crc(&data);
    }
    md_unmapit(MD_NVRAM_IDX, ptr);
    return data.size;	/* 0 means crc OK, else crc returned */
}

