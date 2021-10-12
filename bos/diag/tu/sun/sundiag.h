/* @(#)60       1.2  src/bos/diag/tu/sun/sundiag.h, tu_sunrise, bos411, 9437A411a 3/30/94 16:25:57 */
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/****************************************************************************
        File:   sundiag.h
*****************************************************************************/

#include <sys/diagex.h>

typedef struct sund_dds {
   diagex_dds_t    d_dds;          /* diagdex_dds structure            */
   uint            ver_num;        /* Version Number                   */
   char            intr_func[80];  /* path of the interrupt function   */
} sundiag_dds;

typedef struct resource  {
   char  mode[4];
   char  odm;                   /* running mode - REG, EMC or OTH       */
   char  dma;                   /* DMA_complete was executed=0, else=1  */
   char  diag;                  /* flag - writing to card is performed  */
   char  intr;                  /* flag - interr.routine is loaded  */
   char  handle;                /* flag - handle is opened          */
} ;

struct attr {
        char *attribute;
        char *value;
};
