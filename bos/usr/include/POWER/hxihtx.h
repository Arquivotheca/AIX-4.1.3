/* @(#)89	1.3  src/bos/usr/include/POWER/hxihtx.h, datok, bos411, 9428A410j 2/20/91 20:55:29 */
/*
 *   COMPONENT_NAME: CMDDIAG
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27 
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/****************************************************************************/
/*   hxihtx.h      J. Freter        ver. 1.0        03/15/88                */
/*                                                                          */
/*   HXIHTX.H                                                               */
/*   this is an include file common to all hardware exercisers and the      */
/*   HXFUPDATE function.  It describes the data structure passed to         */
/*   HXFUPDATE from the hardware exerciser.                                 */
/****************************************************************************/

#define   START     'S'
#define   UPDATE    'U'
#define   ERROR     'E'
#define   FINISH    'F'

struct htx_data {
        char    sdev_id[15];     /* Device id passed as first parameter  */
        char    run_type[4];     /* REG, EMC, or OTH                     */
        int     bad_others;      /* Count of bad ioctl operations        */
        int     bad_reads;       /* Count of bad read operations         */
        int     bad_writes;      /* Count of bad write operations        */
        long    bytes_read;      /* Total bytes read                     */
        long    bytes_writ;      /* Total bytes written                  */
        int     good_others;     /* Count of good ioctl operations       */
        int     good_reads;      /* Count of good read operations        */
        int     good_writes;     /* Count of good write operations       */

        /* error data                                                       */
        int     error_code;
        short   severity_code;   /*  1 = hard error  4 = soft error         */
                                 /*  6 = system software  7 = information   */
        char    HE_name[15];

        /* char    op_type; */      /*  r = read   w = write  o = other operation  */
        char    msg_text[221];
};
