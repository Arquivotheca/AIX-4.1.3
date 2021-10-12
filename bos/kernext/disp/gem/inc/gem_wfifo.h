/* @(#)72	1.7.1.5  src/bos/kernext/disp/gem/inc/gem_wfifo.h, sysxdispgem, bos411, 9428A410j 1/19/93 12:43:44 */
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		LWFIFO
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


/***********************************************************************/
/*                                                                     */
/*  lwfifo(f,bufptr,length)     *   Large write to fifo                */
/*                              *   meant for writes of >= 32 bytes    */
/*      int f;                  *   fifo number  (0,1,2,3)             */
/*      char *bufptr;           *   pointer to data buffer             */
/*      int length;             *   bytes  of data to write            */
/*                                                                     */
/***********************************************************************/
#define LWFIFO(f,bufptr,length)                                           \
  {                                                                       \
  ushort write_index;             /* offset into this fifo             */ \
  int bytes_to_end;               /* bytes to end of fifo              */ \
  volatile int bytes_left;        /* bytes to wrap around to front of     \
					      fifo                     */ \
  volatile int bytes_to_write;                                            \
  ulong fifo_start;                                                       \
									  \
  write_index  =  (ushort) ld->fifo_ptr[f];                               \
  fifo_start  = (ulong) ld->fifo_ptr[f] - write_index;                    \
  bytes_to_end =  SixtyFourK - write_index ;                              \
  bytes_left  =  (length) - bytes_to_end;                                 \
  bytes_to_write = (bytes_to_end > length) ? length : bytes_to_end;       \
									  \
 /**********************************************************************/ \
 /*  Spin reading the count reg until there is enough space available  */ \
 /*  for the write.  Spinning is not real desirable, but it seems to   */ \
 /*  be the best method available.  We might want to put in a timeout. */ \
 /* The same applies for the loop a couple of lines further down       */ \
 /**********************************************************************/ \
  while (length > SixtyFourK - (((*(ld->fifo_cnt_reg[f]))                 \
				    & FIFOInUseMask) + 0xc));             \
      memcpy(ld->fifo_ptr[f],(bufptr), bytes_to_write);                   \
									  \
  if (bytes_left > 0)                                                     \
  {                                                                       \
     memcpy((int *)fifo_start,                                            \
		       ((char *)(bufptr))+bytes_to_write, bytes_left);    \
  }                                                                       \
									  \
    if (f == ImmBLTFIFO)                                                  \
    {                                                                     \
	 *(ld->fifo_add_reg[f]) = (length + 0x3) &  0xFFFFFFFC;           \
	 (ld->fifo_ptr[f]) = (ulong *)(fifo_start +                       \
		  (ushort)(write_index + ((length + 0x3) & 0xFFFFFFFC))); \
									  \
    }                                                                     \
    else                                                                  \
    {                                                                     \
     *(ld->fifo_add_reg[f]) = length;                                     \
     (ld->fifo_ptr[f]) = (ulong *)(fifo_start +                           \
		     (ushort)(write_index +length));                      \
    }                                                                     \
 }
