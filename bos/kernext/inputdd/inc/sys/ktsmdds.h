/* @(#)75   1.6  src/bos/kernext/inputdd/inc/sys/ktsmdds.h, inputdd, bos41J, 9519A_all 5/9/95 07:19:14  */
/*
 *   COMPONENT_NAME: INPUTDD
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_KTSMDDS
#define _H_KTSMDDS

#include <sys/types.h>


static struct ktsmdds {

   ulong bus_id;                       /* bus ID                             */
   ulong bus_io_addr;                  /* port address                       */
   int bus_intr_lvl;                   /* interrupt level                    */
   int intr_priority;                  /* interrupt priority                 */
   long slot_addr;                     /* location of adapter                */
   int ipl_phase;                      /* ipl phase (0 = run time)           */

   int device_class;                   /* class of device                    */
#define ADAPTER         0              /*   adapter                          */
#define KBD             1              /*   keyboard                         */
#define TABLET          2              /*   tablet                           */
#define MOUSE           3              /*   mouse                            */

   int device_type;                    /* type of device (from inputdd.h)    */
/*                      0x00                adapter                          */
/*   KS101              0x01                101 key keyboard                 */
/*   KS102              0x02                102 key keyboard                 */
/*   KS106              0x03                106 key keyboard                 */
/*   KSPS2              0x04                ps2 keyboard                     */
/*   TAB6093M11         0x01                6093 model 11 or equivalent      */
/*   TAB6093M12         0x02                6093 model 12 or equivalent      */
/*   MOUSE3B            0x01                3 button mouse                   */
/*   MOUSE2B            0x02                2 button mouse                   */

   char logical_name[17];              /* device logical name                */

/* keyboard extension                                                        */
   uint typamatic_delay;               /* typamatic delay (milliseconds)     */
   uint typamatic_rate;                /* typamatic rate (characters/second) */
   uchar volume;                       /* alarm volume                       */
   uchar click;                        /* clicker configuation               */
   uchar map;                          /* keyboard special map               */
   uchar type;                         /* keyboard special type              */

/* pci/isa extension                                                         */
                                       /* mousedd devno when config kbddd    */
   long devno_link;                    /* kbddd devno when config mosuedd    */

};

#endif
