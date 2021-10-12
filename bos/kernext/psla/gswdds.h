/* @(#)05       1.5  10/12/93 10:27:05 */
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
/* PURPOSE:     Define the Define Device Structure.                     */
/*                                                                      */
/************************************************************************/

/* INCLUDE FILES:                                                       */
#include <sys/types.h>

/*----------------------------------------------------------------------*/
/*      System dds passed to driver.                                    */
/*----------------------------------------------------------------------*/
struct mslapsla_dds {
	uint    dma_level;              /* dma level                    */
	uint    intr_level;             /* interrupt level              */
	uint    intr_priority;          /* interrupt priority           */
	uint    start_busmem;           /* start of adapter mem         */
	uint    start_busio;            /* start of adapter io          */
	uint    slot_number;            /* slot number                  */
	uint    bus_id;                 /* bus id                       */
	uint    dma_bus_addr;           /* dma bus addr                 */
	uint    ucode_fd;               /* ucode file descriptor        */
	uint    ucode_len;              /* ucode length                 */
};

/*----------------------------------------------------------------------*/
/*      MSLA    Ras Log structure                                       */
/*----------------------------------------------------------------------*/

struct dds_gsw_err {
	uint    len;                    /* length                       */
	char    class;                  /* class                        */
	char    subclass;               /* subclass                     */
	char    mask;                   /* mask                         */
	char    type;                   /* type                         */
	uint    data_len;               /* length                       */
	char    comp_id[8];             /* compilation id               */
} ;

/*----------------------------------------------------------------------*/
/*      Complete dds.                                                   */
/*----------------------------------------------------------------------*/

struct gsw_dds {
	struct mslapsla_dds     sd;     /* system dds from config       */
	struct dds_gsw_err      er;     /* error log                    */
};


