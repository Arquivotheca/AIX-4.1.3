/* @(#)13	1.3  src/bos/diag/tu/sky/exectu.h, tu_sky, bos411, 9428A410j 1/21/94 13:40:48 */
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#define SKYTYPE struct _skytu 
struct tucb_t
{   long    tu, mfg, loop;
    long    r1, r2;
};
struct _skytu
{   struct  tucb_t   header;
    struct  tu_info  *sky_ptr;
    unsigned long    nio_fdes;
/*  lword            nio_fdes; */
};
/* struct cntlparm cptr;*/

