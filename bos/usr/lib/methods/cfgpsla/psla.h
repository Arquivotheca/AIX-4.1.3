/*                                                            */
/*           FILENAME : psla.h                                */
/*           sccsid[] = "@(#)psla.h   1.1 2/2/90 12:35:36         "   */ 
/*           file : psla.h                                    */

/* 
*************************************************************************
*									* 
* FILE_NAME:  psla.h							*
*									*
* FUNCTIONS : 	Has the required structure used by the device driver    *
* 									*
* ORIGINS : 27 								*
* 									*
* IBM CONFIDENTIAL -- (IBM Confidential Restricted when			*
* combined with the aggregated modules for this product)		*
*                  SOURCE MATERIALS					*
* (C) COPYRIGHT International Business Machines Corp. 1990		*
* All Rights Reserved							*
* Licensed Material - Property of IBM					*
*									*
* US Government Users Restricted Rights -  Use, Duplication or		*
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.	*
* 									*
*************************************************************************
*/

struct psladd {
	uint	dma_level;		/*  dma level            */
	uint	intr_level;		/*  interrupt level      */
	uint	intr_priority;		/*  interrupt priority   */
	uint	start_busmem;		/*  start of adapter mem */
	uint	start_busio;		/*  start of adapter io  */
	uint    slot_number;		/*  slot number          */
	uint	bus_id;			/*  bus id               */
	uint	dma_bus_addr;		/*  dma bus memory addr	 */
	uint	ucode_fd;		/*  ucode file descriptor*/
	uint	ucode_len;		/*  ucode length         */
};

