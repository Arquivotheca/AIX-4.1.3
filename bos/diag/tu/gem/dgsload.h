/* @(#)75	1.1  src/bos/diag/tu/gem/dgsload.h, tu_gem, bos411, 9428A410j 5/30/91 12:47:30 */
/*
 * COMPONENT_NAME: tu_gem (dgsload.h) 
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************************************************/
/*  Definition of structure to pass to ucode loader         */
/*                                                          */
/*                                                          */
/************************************************************/
struct ldr {
       unsigned int global_mem;    /* system global mem address    */
       char *ucode_buff;           /* pointer to C25 ucode         */
       int  dat_len;               /* len of data in buffer        */
       char *bif_tst_buf;          /* pointer to bif test ucode    */
       int  b_t_len;               /* length of bif test code..    */
       char  *gcp_buf;             /* pointer to gcp ucode         */
       int   gcp_len;              /* length of gcp ucode          */
       char  *gcp_tbl;             /*                              */
       int   gcp_tbl_len;          /*                              */
       char  *shp_tbl;             /* pointer to SHP GVP Ucode     */
       int   shp_tbl_len;          /* length of GCP tables         */

       int drp_load;               /* These fields tell what to    */
       int shp_load;               /*     load on card...          */
       int gcp_load;               /*                              */

                /* returned values....                  */
                /* slot number ff indicates not present */
       int magslot;
       int shpslot;
       int drpslot;
       int gcpslot;

       int fd;              /* File descriptor              */
       uint r0_load;        /* Command for DRP/SHP          */
       uint r1_load;        /* Interactive Cmmd for DRP/SHP */
       int r3;                       /* reserved        */
       int r4;                       /* reserved        */
       int r5;                       /* reserved        */

} ;
