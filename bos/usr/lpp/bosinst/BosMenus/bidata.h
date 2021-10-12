/*
 * static char sccsid[] = "@(#) 46 1.2 src/bos/usr/lpp/bosinst/BosMenus/bidata.h, bosinst, bos411, 9428A410j 93/09/30 09:44:46";
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: include file for datadaemon
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Defines for datadaemon
 */

#define GET_BOSINST_FIELD 		(1 << 0)  /* 1   */
#define GET_IMAGEDATA_FIELD 		(1 << 1)  /* 2   */
#define CHANGE_BOSINST_FIELD 	 	(1 << 2)  /* 4   */
#define CHANGE_IMAGEDATA_FIELD 		(1 << 3)  /* 8   */
#define DELETE_BOSINST_DISK		(1 << 4)  /* 16  */
#define ADD_BOSINST_DISK		(1 << 5)  /* 32  */
#define WRITE_FILES			(1 << 6)  /* 64  */
#define EXIT				(1 << 7)  /* 128 */
#define GET_STATUS			(1 << 8)  /* 256 */
#define GET_ALL_DISKS			(1 << 9)  /* 512 */
#define DEL_ALL_DISKS			(1 << 10) /* 1024*/

/* the names of the pipes:*/
#define BIDATA_COMMAND "/../bidatacommand"
#define BIDATA_RESPONSE "/../bidataresponse"

/* defines for data status */
#define LV_DUP	1
#define FS_DUP  2
#define FS_DEV  3
#define TDD_DUP 4

/* structure for communication */
struct bidata
{

    int type;		/* type of request 		*/
    char stanza[30];	/* stanza of request 		*/
    char field[80];	/* field of interest 		*/
    char value[80];	/* modify value			*/
    char auxfield[80];	/* retreival criteria field	*/	
    char auxvalue[80];	/* retrieval criteria value	*/
};

struct ddresponse
{
    int rc;		/* return code			*/
    char data[512];	/* return data			*/
};
