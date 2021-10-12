/* @(#)75       1.6  10/12/93 10:41:19 */
/*
 *   COMPONENT_NAME: SYSXPSLA
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/************************************************************************/
/* PURPOSE:     Defines for the device driver.                          */
/*              note: when compiling code for HYDRA, 'NumDevSupp'       */
/*                    is the only item that must change.                */
/*                                                                      */
/************************************************************************/

#define SHIFTW ('W'<<8)


#define MSLA_START_DIAG (SHIFTW|32)
/*  ioctl(fd, MSLA_START_DIAG );
    This call will cause the interrupt handler to count interrupts
    and take no other action.  rc always 0 (barring system error    */

#define MSLA_QUERY_DIAG (SHIFTW|33)
/*  ioctl(fd, MSLA_QUERY_DIAG, &msla_intr_count );
    This call will result in msla_map showing the number of each
    type of interrupt received since the MSLA_START_DIAG call or
    since the previous MSLA_QUERY_DIAG call.

    if MSLA_START DIAG had not been previously issued
	rc =  xxx  (-1?)
    else
	rc = 0;                                                         */

#define MSLA_STOP_DIAG  (SHIFTW|34)
/*  ioctl(fd, MSLA_STOP_DIAG );
    resume normal interrupt handling              */

#define MSLA_GET_ADDR   (SHIFTW|35)
/*  ioctl(fd, MSLA_GET_ADDR, &msla_map );
     Results in 32 bit addresses places int msla map
	if success
	    rc =  0
	else
	    rc = -1                     */

#define MSLA_RET_ADDR   (SHIFTW|36)
/*  ioctl(fd, MSLA_RET_ADDR );
     Access to bus nolonger needed
     rc always = 0;                        */

#define MSLA_MOD_POS        (SHIFTW|37)
#define        MP_ENA_CARD  0x1
#define        MP_DIS_CARD  0x2
#define        MP_ENA_PARITY   0x3
#define        MP_DIS_PARITY   0x4
#define        MP_STOP      0x5
/* NOTE: Must have issued successful MSLA_START_DIAG
	 prior to the following ioctls:

   ioctl(fd, MSLA_MOD_POS, MP_ENA_CARD);
      - vdd sets enable card bit
   ioctl(fd, MSLA_MOD_POS, MP_DIS_CARD);
      - vdd resets enable card bit
   ioctl(fd, MSLA_MOD_POS, MP_ENA_PARITY);
      - vdd sets enable parity bit
   ioctl(fd, MSLA_MOD_POS, MP_DIS_PARITY);
      - vdd resets enable parity bit
   ioctl(fd, MSLA_MOD_POS, MP_STOP);
      - vdd restores pos settings saved from MP_START
   for all MSLA_MOD_POS calls
	    rc = 0 on success, otherwise rc = -1;       */

#define MSLA_LOAD_UCODE  (SHIFTW|38)
/* ioctl(fd, MSLA_LOAD_UCODE);
    vdd restores adapter to usable state.               */


#define MSLA_START_DMA   (SHIFTW|39)
/*  ioctl(fd, MSLA_START_DMA,&dma_test_parms);
    device driver to start dma test
    if pass
	rc = 0
    else
	rc = -1      */


#define MSLA_STOP_DMA   (SHIFTW|40)
/*  ioctl(fd, MSLA_STOP_DMA );
    device driver to stop dma test
    if pass
	rc = 0
    else
	rc = -1      */

/***************************************************************/
/* MSLA  Diagnostic  Interrupt  Structure                      */
/***************************************************************/
struct msla_intr_count
{
	ulong total_cnt  ;      /* status bit 0 */
	ulong parity_cnt ;      /* status bit 1 */
};


struct msla_map
{
	ulong msla_mem_start;
	ulong msla_io_start;
};


struct dma_test_parms {
        ulong ubuff_adr;	/* user buffer holding dma data */
        ulong * dma_adr_p;	/* pointer to user location     */
				/* receiving dma_adr+page offset*/
        int   count;		/* number of bytes in dma xfer  */
};

