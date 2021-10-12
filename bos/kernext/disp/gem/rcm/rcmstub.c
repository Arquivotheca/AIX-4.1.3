static char sccsid[] = "@(#)82	1.6.1.9  src/bos/kernext/disp/gem/rcm/rcmstub.c, sysxdispgem, bos411, 9428A410j 1/19/93 12:23:20";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		DISABLE_THRESHHOLDS
 *		ENABLE_THRESHHOLDS
 *		iggm_check_rcxp_links
 *		iggm_command_list
 *		iggm_lock_domain
 *		iggm_lock_hw
 *		iggm_set_gp_priority
 *		iggm_unlock_domain
 *		iggm_unlock_hw
 *		read_data
 *		write_byte
 *		write_data_bytes
 *		write_data_words
 *		wtfifo
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



int db_hft;

iggm_set_gp_priority() { return(0); }

iggm_lock_hw() { return(0); }
iggm_unlock_hw() { return(0); }
iggm_lock_domain() { return(0); }
iggm_unlock_domain() { return(0); }
iggm_command_list() { return(0); }

iggm_check_rcxp_links() { return(0); }
#include "gemincl.h"
#include "gemrincl.h"

typedef struct 
 {

   ulong	data[16];

 } *pdata;

void write_data_words(FIFO_NUM, bufp, len, pDevP)
     rGemRCMPrivPtr   pDevP;                                       
     uint	      FIFO_NUM, len;
     ulong	      *bufp;
{
  pdata		pFifo;
  register int	i, j;

  if (len <= 512)
  {
    memcpy(FIFO_P[FIFO_NUM], bufp, len);
  }
  else
  {
    pFifo = (pdata)FIFO_P[FIFO_NUM] ;
    j = len / 64;
    for(i=j; i; i--)
    { *pFifo = *((pdata)bufp);
	bufp += 16;
    }
    if (len % 64)
      memcpy(pFifo, bufp, len % 64);
  }

}

void write_data_bytes(FIFO_NUM, bufp, len, pDevP)
     char	     *bufp;
     rGemRCMPrivPtr  pDevP;
     uint	     FIFO_NUM, len;
{
  register int i;
  
  for(i=len; i; i--) {
    *FIFO_P[FIFO_NUM] = *bufp;
    bufp++;
  }

}

void read_data(FIFO_NUM, bufp, len, pDevP)
     char               *bufp;
     rGemRCMPrivPtr     pDevP;   
     uint  	        FIFO_NUM, len;
{
  register int  i, testval;


/* @A*/
  testval=FifoLen-len;
  while ( ((*IUR_P[ImmDataFifo]) & IURMASK) > testval);
  for (i = len; i; i--) 
  {  
      *bufp = *FIFO_P[FIFO_NUM];
      bufp++;
  }

}

void wtfifo(FIFO_NUM, bufp, len, seg_reg, pDevP)
     rGemRCMPrivPtr  pDevP;
     uint	     FIFO_NUM, len;
     ulong	     *bufp;
     ulong           seg_reg;

{
  register int i,j;
  ushort hi_thresh;
  ulong size, needed;

  /*
   * Compute max we can put into FIFO without tripping hi threshold 
   */

  hi_thresh=(ushort)*(THRES_P[FIFO_NUM]) - 4;
  size = hi_thresh / 8 * 4;

  /*
   * compute available space needed in order to put SIZE bytes into
   * the fifo without tripping the hi threshhold
   */

  needed = size + FifoLen - hi_thresh;

  for (j=len/size; j; j--) 
  {
    FIFO_AVAL(FIFO_NUM, needed, seg_reg, pDevP);
    write_data_words( FIFO_NUM, bufp, size, pDevP);
    bufp += size / 4;
  }

  size = len % size;
  needed = size + FifoLen - hi_thresh;

  FIFO_AVAL(FIFO_NUM, needed, seg_reg, pDevP);
  write_data_words( FIFO_NUM, bufp, size, pDevP);

}

void write_byte(FIFO_NUM, bufp, len, seg_reg, pDevP)
     char	     *bufp;
     rGemRCMPrivPtr  pDevP;
     uint	     FIFO_NUM, len;
     ulong           seg_reg;
{
  register int i,j;
  uint	hi_thresh;
  ulong size, remainder;

  /*
   * Compute max we can put into FIFO without tripping hi threshold 
   */

  hi_thresh=(uint)*(THRES_P[FIFO_NUM]) - 4;
  j=len/hi_thresh;

  remainder= len - (j*hi_thresh);
  
  for ( size=hi_thresh; j; j--) 
  {
    FIFO_AVAL(FIFO_NUM, size, seg_reg, pDevP);
    write_data_bytes( FIFO_NUM, bufp, len, pDevP);
  }

  size=remainder;
  FIFO_AVAL(FIFO_NUM, size, seg_reg, pDevP);
  write_data_bytes( FIFO_NUM, bufp, size, pDevP);

}

  /*
   * Status register constants for looking at threshold enables
   */

#define DISABLE_THRESHHOLDS(FIFO_NUM, SAVEAREA, seg_reg, pDevP)		\
{ 									\
  ulong *pHI, *pLO;							\
  /*									\
   * Disable interrupts							\
   */									\
   SAVEAREA.oldlevel = i_disable(INTMAX);				\
									\
  /*									\
   * Save State of threshold enables                   			\
   */									\
   GMBASE_INIT(seg_reg, pDevP);                                         \
   TER_INIT(FIFO_NUM, pDevP);  				                \
   pHI = TER_P[FIFO_NUM];				                \
   pLO = TER_P[FIFO_NUM]+1;				                \
/*\
   SAVEAREA.enable[FIFO_NUM].hi = *pHI;			                \
   SAVEAREA.enable[FIFO_NUM].lo = *pLO;			                \
*/\
  /*							                \
   * Set high and low threshold interrupts off	                	\
   */						                	\
   *pHI = DISABLE_THRESH;				                \
   *pLO = DISABLE_THRESH;				                \
							                \
  /*							                \
   * Enable interrupts				               	 	\
   */									\
   i_enable(SAVEAREA.oldlevel);						\
   }

#define ENABLE_THRESHHOLDS(FIFO_NUM, SAVEAREA, seg_reg, pDevP)		\
{									\
  ulong *pHI, *pLO;							\
  ulong  hi_mask, lo_mask, *p;						\
  /*									\
   * Disable interrupts							\
   */									\
   SAVEAREA.oldlevel = i_disable(INTMAX);				\
									\
   /*									\
    * Set threshold enables according to saved values			\
    */									\
   GMBASE_INIT(seg_reg, pDevP);                                         \
   TER_INIT(FIFO_NUM, pDevP);  				                \
   pHI = TER_P[FIFO_NUM];				                \
   pLO = TER_P[FIFO_NUM]+1;				                \
   *pHI = ENABLE_THRESH;				                \
   *pLO = ENABLE_THRESH;				                \
/*\
    *pHI = SAVEAREA.enable[FIFO_NUM].hi;				\
    *pLO = SAVEAREA.enable[FIFO_NUM].lo;				\
*/\
									\
  /*									\
   * Enable interrupts							\
   */									\
   i_enable(SAVEAREA.oldlevel);						\
 }

typedef struct thresh_save  {
  ulong		oldlevel;
    struct	{
      ulong	hi;
      ulong	lo;
    } enable[4];
} thresh_save;
