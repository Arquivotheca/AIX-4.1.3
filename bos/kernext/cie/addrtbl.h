/* @(#)35   1.7  src/bos/kernext/cie/addrtbl.h, sysxcie, bos411, 9428A410j 4/26/94 10:46:17 */

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   new_ADDRTABLE
 *   free_ADDRTABLE
 *   enableGroup
 *   disableGroup
 *   disableAllGroups
 *
 * DESCRIPTION:
 *
 *    Group/Multicast Address Tracking
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

#if !defined(ADDRTBL_H)
#define  ADDRTBL_H

typedef struct ADDRTABLE     ADDRTABLE;
typedef struct ADDRENTRY     ADDRENTRY;

/*---------------------------------------------------------------------------*/
/*                      One entry in the address table                       */
/*---------------------------------------------------------------------------*/

struct ADDRENTRY
{
   ADDRENTRY               * next;        // Ptr to next entry
   ushort                    refCount;    // Reference Counter
   char                      addr[1];     // Address
};

/*---------------------------------------------------------------------------*/
/*          Address table used for TR Group Addrs and Ent Multicast          */
/*---------------------------------------------------------------------------*/

struct ADDRTABLE
{
   ushort                    tSize;      // Number of table entries
   ushort                    aBytes;     // Length of address in bytes
   ushort                    eBytes;     // Length of entry in bytes
   ADDRENTRY               * freeList;   // Index of free list head
   ADDRENTRY               * allocList;  // Index of allocated list head
   ADDRENTRY                 e[1];       // Dynamically sized address table
};

/*---------------------------------------------------------------------------*/
/*                    Convert an entry index to a pointer                    */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

ADDRENTRY *
   ePtr(
      ADDRTABLE            * t           ,// I -The Address Table
      int                  * n            // I -Entry Index
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  ePtr(t,n)    ((ADDRENTRY *)((n>=0 && n<t->tSize) ? ((char *)(t->e) + (n*t->eBytes)) : NULL))

/*---------------------------------------------------------------------------*/
/*                   Convert an entry pointer to an index                    */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

ADDRENTRY *
   eIndex(
      ADDRTABLE            * t           ,// I -The Address Table
      ADDRENTRY            * p            // I -Entry Pointer
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  eIndex(t,e)  ((e) ? ( ((int)(  (char *)(e) - (char *)((t)->e)  )) / (t)->eBytes) : -1)

/*---------------------------------------------------------------------------*/
/*     Allocate a new address table tSize elements with addr aBytes long     */
/*---------------------------------------------------------------------------*/

ADDRTABLE *
   new_ADDRTABLE(
      ushort                 tSize       ,
      ushort                 aBytes
   );

/*---------------------------------------------------------------------------*/
/*                         Destroy an address table                          */
/*---------------------------------------------------------------------------*/

void
   free_ADDRTABLE(
      ADDRTABLE            * t
   );

/*---------------------------------------------------------------------------*/
/*                     Enable a Group/Multicast Address                      */
/*---------------------------------------------------------------------------*/

int
   enableGroup(
      register ndd_t       * nddp        ,// I -NDD
      register ADDRTABLE   * t           ,// IO-The Address Table
      register int         * chnRef      ,// IO-Channel's Address Ref Table
      register const char  * addr         // I -Group Addr to be enabled
   );

/*---------------------------------------------------------------------------*/
/*                     Disable a Group/Multicast Address                     */
/*---------------------------------------------------------------------------*/

int
   disableGroup(
      register ndd_t       * nddp        ,// I -NDD
      register ADDRTABLE   * t           ,// IO-The Address Table
      register int         * chnRef      ,// IO-Channel's Address Ref Table
      register const char  * addr         // I -Group Addr to be enabled
   );

/*---------------------------------------------------------------------------*/
/*                 Disable all group addresses for a channel                 */
/*---------------------------------------------------------------------------*/

int
   disableAllGroups(
      register ndd_t       * nddp        ,// I -NDD
      register ADDRTABLE   * t           ,// IO-The Address Table
      register int         * chnRef       // IO-Channel's Address Ref Table
   );

#endif
