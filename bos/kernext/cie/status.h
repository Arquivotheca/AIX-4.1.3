/* @(#)57   1.7  src/bos/kernext/cie/status.h, sysxcie, bos411, 9428A410j 4/18/94 16:22:15 */

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   mapAsyncStatus
 *   new_STATBLK
 *   free_STATBLK
 *   mkStartDoneStatus
 *   mkHaltDoneStatus
 *   mkNullBlkStatus
 *   mkLostStatusStatus
 *   mkTXDoneStatus
 *   mkCDLIStatus
 *
 * DESCRIPTION:
 *
 *    Status Block External Interface
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

#if !defined(STATUS_H)
#define  STATUS_H

/*---------------------------------------------------------------------------*/
/*                         Status Block Format Code                          */
/*---------------------------------------------------------------------------*/

typedef enum STATBLK_FORMAT  STATBLK_FORMAT;

enum STATBLK_FORMAT
{
   STAT_CDLI,
   STAT_COMIO
};

/*---------------------------------------------------------------------------*/
/*                   Status Block Encapsulation Structure                    */
/*---------------------------------------------------------------------------*/

typedef struct STATBLK         STATBLK;         // Status Block

struct   STATBLK
{
   STATBLK_FORMAT            format;     // Format Code
   union
   {
      cio_stat_blk_t         cio;        // COMIO Status Block
      ndd_statblk_t          ndd;        // CDLI Status Block
   } stat;
};

/*---------------------------------------------------------------------------*/
/*               Map A Status Block from CDLI to COMIO Format                */
/*---------------------------------------------------------------------------*/

int
   mapAsyncStatus(
      CIE_DEV_TYPE           devType     ,// I -Device Type Code
      cio_stat_blk_t       * cioStat     ,//  O-COMIO Status Block
      const ndd_statblk_t  * cdliStat     // I -CDLI Status Block
   );

/*---------------------------------------------------------------------------*/
/*                Allocate and bzero a status block structure                */
/*---------------------------------------------------------------------------*/

STATBLK *
   new_STATBLK(
      STATBLK_FORMAT         format      ,// I -Status Block Format Code
      int                    waitFlag     // I -Wait flag (borrowed from mbuf)
   );

/*---------------------------------------------------------------------------*/
/*                       Free a Status Block Structure                       */
/*---------------------------------------------------------------------------*/

void
   free_STATBLK(
      STATBLK              * s            // IO-Ptr to statblk to be freed
   );

/*---------------------------------------------------------------------------*/
/*                   Create a CIO_START_DONE status block                    */
/*---------------------------------------------------------------------------*/

STATBLK *
   mkStartDoneStatus(
      register DEV         * dev         ,// I -Device
      register netid_t       netid        // I -Net ID
   );

/*---------------------------------------------------------------------------*/
/*                    Create a CIO_HALT_DONE status block                    */
/*---------------------------------------------------------------------------*/

STATBLK *
   mkHaltDoneStatus(
      register DEV         * dev         ,// I -Device
      register netid_t       netid        // I -Net ID
   );

/*---------------------------------------------------------------------------*/
/*                    Create a CIO_NULL_BLK status block                     */
/*---------------------------------------------------------------------------*/

STATBLK *
   mkNullBlkStatus(
      void
   );

/*---------------------------------------------------------------------------*/
/*                   Create a CIO_LOST_STATUS status block                   */
/*---------------------------------------------------------------------------*/

STATBLK *
   mkLostStatusStatus(
      void
   );

/*---------------------------------------------------------------------------*/
/*                     Create a CIO_TX_DONE status block                     */
/*---------------------------------------------------------------------------*/

STATBLK *
   mkTXDoneStatus(
      register int           status      ,// I -Status Code
      register int           write_id     // I -Write Correlator from Extension
   );

/*---------------------------------------------------------------------------*/
/*               Encapsulate an NDD status block for queueing                */
/*---------------------------------------------------------------------------*/

STATBLK *
   mkCDLIStatus(
      register const ndd_statblk_t  * ns           // I -NDD Status Block
   );

#endif
