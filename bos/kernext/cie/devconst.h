/* @(#)13   1.4  src/bos/kernext/cie/devconst.h, sysxcie, bos411, 9428A410j 4/27/94 15:06:17 */

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *
 * DESCRIPTION:
 *
 *    Mapping structures for COMIO Emulator Constants contained
 *    in ciedata.c.
 *
 *
 *
 *                       struct DEVMGR
 *                       +-------------+
 *                       | Eye Catcher |
 *                       +-------------+
 *                       | &compTime   |
 *                       +-------------+
 *  +--------------------| devTypeData |
 *  |                    +-------------+
 *  |                    |             |
 *  |                    .             .
 *  |                    .             .
 *  |                    .             .
 *  |
 *  |
 *  |  struct DEV_TYPE_DEF[]
 *  +->+------------+------------+
 *     |  iodtSize  |  &iodTable |
 *     +------------+------------+
 *     .            .     |      .
 *     .            .     |      .
 *     .            .     |      .
 *                        |
 *                        |
 *  +---------------------+
 *  |
 *  |
 *  |  struct IODT_ENTRY[iodtSize]
 *  +->+------------+------------+
 *     | IOCTL Cmd  |  &handler  |
 *     +------------+------------+
 *     | IOCTL Cmd  |  &handler  |
 *     +------------+------------+
 *     | IOCTL Cmd  |  &handler  |
 *     +------------+------------+
 *     | IOCTL Cmd  |  &handler  |
 *     +------------+------------+
 *     |            |            |
 *     .            .            .
 *     .            .            .
 *     .            .            .
 *
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#if !defined(DEVCONST_H)
#define  DEVCONST_H

typedef struct DEV_TYPE_DEF    DEV_TYPE_DEF;    // Device Type Definition
typedef struct IODT_ENTRY      IODT_ENTRY;      // IOCTL Dispatch Table Entry

/*---------------------------------------------------------------------------*/
/*           Type Definition for IOCTL Handler Routine Entry Point           */
/*---------------------------------------------------------------------------*/

typedef int                    IOCTL_HANDLER(
                                  register DEV    * dev,
                                  register CHN    * chn,
                                  register void   * arg,
                                  register int      devFlag
                               );


/*---------------------------------------------------------------------------*/
/*                       Device-Type Definition Block                        */
/*---------------------------------------------------------------------------*/

struct DEV_TYPE_DEF
{
   int                       iodtSize;    // Size of IOCTL Dispatch Table
   IODT_ENTRY              * iodTable;    // Ptr to IOCTL Dispatch Table
};

/*---------------------------------------------------------------------------*/
/*                    IOCTL Handler Dispatch Table Entry                     */
/*---------------------------------------------------------------------------*/

struct IODT_ENTRY
{
   int                       cmd;         // IOCTL Command Code
   IOCTL_HANDLER           * handler;     // Addr of IOCTL Command Handler
};

#endif
