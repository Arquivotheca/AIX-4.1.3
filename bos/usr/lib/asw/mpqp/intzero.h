/*
static char sccsid[] = "@(#)31  1.3  src/bos/usr/lib/asw/mpqp/intzero.h, ucodmpqp, bos411, 9428A410j 1/30/91 17:18:31";
*/

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS:	Typedefs and defines for DMA structures and masks. 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*
   The Bus master DMA channel on Contender is referenced with the following
   basic structures and definitions.
*/

/*
   The main DMA Channel Control Block definition is a bit misleading.
   The BM DMA CCB exists in 186 I/O space, so defining a structure to
   reference the internal fields is only done so that "C" programs
   can setup the whole block and invoke an assembler routine to do
   the multiple I/O word writes required to initialize the channel.
   An additional flag is used by the C code as a lock on access to the
   Bus Master DMA channel -- "running" is TRUE when a BM DMA operation
   is in progress; false otherwise.
*/

typedef struct BM_DMA_CCB
{
	unsigned short		t_count;	/* transfer count */
	unsigned char far	*card_addr;	/* card address */
	unsigned long		host_addr;	/* system address */
	unsigned short		cc_reg;		/* channel control register */
	unsigned char far	*list_addr;	/* list address pointer */
	unsigned short		running;	/* flag if DMA in progress */
} bm_dma_ccb;

/*
   The Bus Master (Contender) DMA channel control word bit definitions
*/

#define	BDCCW_START		(ushort)0x0001	/* Start/Stop Channel */
						/* 1 = Channel Enable */

#define	BDCCW_FROMHOST		(ushort)0x0002	/* Direction Bit */
						/* 1 = Read System Memory */

#define	BDCCW_TCINTR		(ushort)0x0004	/* Terminal Count Int. En. */
						/* 1 = Enabled */

#define	BDCCW_LISTENABLE	(ushort)0x0008	/* List Chaining Enable */
						/* 1 = Enabled */

#define	BDCCW_HOSTINCR		(ushort)0x0010	/* System Adddress Increment */
						/* 1 = Increment */

#define	BDCCW_CARDINCR		(ushort)0x0020	/* Card Address Increment */
						/* 1 = Increment */

#define	BDCCW_MEMORY		(ushort)0x0040	/* Memory/IO Cycle Select */
						/* 1 = Memory Cycle */

#define	BDCCW_TCSTOP		(ushort)0x0080	/* Channel Stop on TC = 0 */
						/* 1 = Stop on terminal cnt */

/*
   The Receive DMA interface with the host system driver operates from
   an eight "unsigned long" length queue with normal in/out pointer
   operations.
*/

typedef struct ULONG_Q
{
	unsigned char		length;
	unsigned char		end;
	unsigned char		out;
	unsigned char		in;
	unsigned long		q [9];
} ulong_q;
