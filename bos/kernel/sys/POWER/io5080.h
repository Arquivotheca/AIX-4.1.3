/* @(#)67  1.3  src/bos/kernel/sys/POWER/io5080.h, sysxmsla, bos411, 9428A410j 2/18/91 14:02:26 */
/*
 * COMPONENT_NAME: (SYSXMSLA) Microchannel 5088 Serial Link Adapter
 *
 * FUNCTIONS:
 *
 * ORIGINS: 30
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/xmem.h>
#include <sys/err_rec.h>

#ifndef H_IO5080
#define H_IO5080

#define WRITEX     0x77
#define READX      0x17
#define GTSESSION  0x97

/***********************************************************/
/*************** 5080 IO Structure *************************/
/***********************************************************/
typedef struct Iobuf
{
	int size;  /* buffer size in bytes         */
	int used;  /*returned bytes processed count*/
	char *buf; /* buffer list                  */
}Iobuf ; 
typedef struct Io_5080
{
	int MOREDATA;      /* More Data flag               */
	int WAIT;          /* Wait on IO                   */
	int NODATA;        /* Mo Data on READX             */
	int     iobufcount;/* buffer count                 */
	int     iobufused; /* returned processed count     */
	struct Iobuf *iobuf;
}Io_5080 ;

union ctl_arg 
{
	int value;
	uint *addr;
	struct  Io_5080 *io5080;
};
#endif /*H_IO5080*/
