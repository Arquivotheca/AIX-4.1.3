/* @(#)65	1.2  src/bos/kernext/inputdd/inc/sys/sgio_dds.h, inputdd, bos41J, 9513A_all 3/28/95 16:04:10  */
/*
 *   COMPONENT_NAME: INPUTDD
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_SGIODDS
#define _H_SGIODDS

#include <sys/types.h>

#define NAMESIZE 16

typedef struct sgio_dds
{
  char devname[NAMESIZE];
  char tty_device[7];
  uint device_class;                   /* class of device                    */
#define S_DIALS         1              /*   serial dials                     */
#define S_LPFKS         2              /*   serial lpfks                     */
} sgio_dds_t;

#endif /* _H_SGIODDS */
