/* @(#)27 1.5  src/bos/kernel/sys/POWER/bootrecord.h, bosboot, bos411, 9428A410j 9/24/93 16:07:47 */
#ifndef _H_BOOTRECORD
#define _H_BOOTRECORD
/*
 *   COMPONENT_NAME: BOSBOOT
 *
 *   FUNCTIONS: none
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
#include <sys/types.h>         /* include for definition of unique_id struct */

/*****************************************************************************/
/*                                                                           */
/* This file defines the IPL Record template. The IPL Record information is  */
/* contained on one sector.                                                  */
/*                                                                           */
/* Acronyms used in this file:                                               */
/*   PSN          Physical Sector Number                                     */
/*   IPL          Initial Program Load                                       */
/*   BLV          Boot    Logical Volume                                     */
/*   SLV          Service Logical Volume                                     */
/*                                                                           */
/* General notes:                                                            */
/*   Fragmentation implies that code which is loaded into memory may not be  */
/*   loaded in one completely contiguous area and inherently refers to memory*/
/*   above the first megabyte bound of memory since the processor environment*/
/*   requires the first meg to be good. That is, code which crosses or is    */
/*   completely contained above the first meg of memory may need to be       */
/*   fragmented. This is reflected in the value of the "boot_frag" flag which*/
/*   is defined below.                                                       */
/*                                                                           */
/* Usage examples for this file:                                             */
/*                                                                           */
/*   IPL_REC_PTR my_rec_ptr          defines my_rec_ptr as a pointer to the  */
/*                                   struct "ipl_rec_area".                  */
/*                                                                           */
/*   IPL_REC my_rec                  can be used to define memory space for  */
/*                                   an ipl record.                          */
/*                                                                           */
/*   sizeof(IPL_REC)                 will return the size (in bytes) of      */
/*                                   ipl_rec_area                            */
/*                                                                           */
/*   my_rec.field_name               either of these can be used to access   */
/*   my_rec_ptr->field_name          an element of the ipl record (shown     */
/*                                   here as "field_name").                  */
/*                                                                           */
/*****************************************************************************/

#define SBSIZE 0x20000			/* Save base size is 64K	*/

typedef struct ipl_rec_area
{
    unsigned int      IPL_record_id;    /* This physical volume contains a   */
                                        /* valid IPL record if and only if   */
                                        /* this field contains IPLRECID      */

#define IPLRECID 0xc9c2d4c1             /* Value is EBCIDIC 'IBMA'           */

    char              reserved1[20];
    unsigned int      formatted_cap;    /* Formatted capacity. The number of */
                                        /* sectors available after formatting*/
                                        /* The presence or absence of bad    */
                                        /* blocks does not alter this value. */

    char              last_head;        /* THIS IS DISKETTE INFORMATION      */
                                        /* The number of heads minus 1. Heads*/
                                        /* are number from 0 to last_head.   */

    char              last_sector;      /* THIS IS DISKETTE INFORMATION      */
                                        /* The number of sectors per track.  */
                                        /* Sectors are numbered from 1 to    */
                                        /* last_sector.                      */

    char              reserved2[6];

    unsigned int      boot_code_length; /* Boot code length in sectors. A 0  */
                                        /* value implies no boot code present*/

    unsigned int      boot_code_offset; /* Boot code offset. Must be 0 if no */
                                        /* boot code present, else contains  */
                                        /* byte offset from start of boot    */
                                        /* code to first instruction.        */

    unsigned int      boot_lv_start;    /* Contains the PSN of the start of  */
                                        /* the BLV.                          */

    unsigned int      boot_prg_start;   /* Boot code start. Must be 0 if no  */
                                        /* boot code present, else contains  */
                                        /* the PSN of the start of boot code.*/

    unsigned int      boot_lv_length;   /* BLV length in sectors.            */

    unsigned int      boot_load_add;    /* 512 byte boundary load address for*/
                                        /* boot code.                        */

    char              boot_frag;        /* Boot code fragmentation flag. Must*/
                                        /* be 0 if no fragmentation allowed, */
                                        /* else must be 0x01.                */

    char	      boot_emulation;	/* ROS network emulation flag */
					/* 0x0 => not an emul support image   */
					/* 0x1 => ROS network emulation code  */
					/* 0x2 => AIX code supporting ROS emul*/

    char              reserved3[2];

    ushort            basecn_length;    /* Number of sectors for base        */
                                        /* customization. Normal mode.       */

    ushort            basecs_length;    /* Number of sectors for base        */
                                        /* customization. Service mode.      */

    unsigned int      basecn_start;     /* Starting PSN value for base       */
                                        /* customization. Normal mode.       */

    unsigned int      basecs_start;     /* Starting PSN value for base       */
                                        /* customization. Service mode.      */

    char              reserved4[24];

    unsigned int      ser_code_length;  /* Service code length in sectors.   */
                                        /* A 0 value implies no service code */
                                        /* present.                          */

    unsigned int      ser_code_offset;  /* Service code offset. Must be 0 if */
                                        /* no service code is present, else  */
                                        /* contains byte offset from start of*/
                                        /* service code to first instruction.*/

    unsigned int      ser_lv_start;     /* Contains the PSN of the start of  */
                                        /* the SLV.                          */

    unsigned int      ser_prg_start;    /* Service code start. Must be 0 if  */
                                        /* service code is not present, else */
                                        /* contains the PSN of the start of  */
                                        /* service code.                     */

    unsigned int      ser_lv_length;    /* SLV length in sectors.            */

    unsigned int      ser_load_add;     /* 512 byte boundary load address for*/
                                        /* service code.                     */

    char              ser_frag;         /* Service code fragmentation flag.  */
                                        /* Must be 0 if no fragmentation     */
                                        /* allowed, else must be 0x01.       */

    char	      ser_emulation;	/* ROS network emulation flag */
					/* 0x0 => not an emul support image   */
					/* 0x1 => ROS network emulation code  */
					/* 0x2 => AIX code supporting ROS emul*/

    char              reserved5[2];

    unique_id_t       pv_id;            /* The unique identifier for this    */
                                        /* physical volume.                  */
    char              dummy[512 - 128 - sizeof(unique_id_t)];
}IPL_REC, *IPL_REC_PTR;
#endif /* _H_BOOTRECORD */
