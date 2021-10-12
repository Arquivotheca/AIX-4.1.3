/* @(#)46   1.6  src/bos/kernext/cie/devtable.h, sysxcie, bos411, 9428A410j 4/1/94 15:49:06 */

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   new_DEVTABLE
 *   free_DEVTABLE
 *   mkDevHandle
 *   dhValid
 *   getDEV
 *   setDEV
 *   clrDEV
 *   incrDevCount
 *   decrDevCount
 *
 * DESCRIPTION:
 *
 *    Device Table
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

#if !defined(DEVTABLE_H)
#define  DEVTABLE_H

#define  CIE_NUM_MAJ    16   /* Number of different major numbers  */
#define  CIE_MAX_DEV    32   /* Number of devices per major number */

/*---------------------------------------------------------------------------*/
/*                       Major Device Number Structure                       */
/*---------------------------------------------------------------------------*/

typedef struct DEVMAJOR      DEVMAJOR;

struct DEVMAJOR
{
   int                       num;                 // Major Number
   int                       devCount;            // Device Count
   DEV                     * dev[CIE_MAX_DEV];    // Array of DEV pointers
};

/*---------------------------------------------------------------------------*/
/*                               Device Table                                */
/*---------------------------------------------------------------------------*/

struct DEVTABLE
{
   char                      iCatcher[4]; // Eye Catcher
   int                       devCount;    // Number of Devices Open
   int                       majCount;    // Number of Major Numbers in use
   DEVMAJOR                  devMajor[CIE_NUM_MAJ];
};

/*---------------------------------------------------------------------------*/
/*                               Device Handle                               */
/*---------------------------------------------------------------------------*/

struct DEVHANDLE
{
   short                     devIndex;    // Index into channel table
   short                     devMinor;    // Device Minor Number
};

/*---------------------------------------------------------------------------*/
/*                   Allocate memory for the Device Table                    */
/*---------------------------------------------------------------------------*/

DEVTABLE *
   new_DEVTABLE(
      void
   );

/*---------------------------------------------------------------------------*/
/*                  Free memory occupied by a Device Table                   */
/*---------------------------------------------------------------------------*/

void
   free_DEVTABLE(
      DEVTABLE             * dt           // IO-Device Table
   );

/*---------------------------------------------------------------------------*/
/*           Convert a device number to an internal device handle            */
/*---------------------------------------------------------------------------*/

int
   mkDevHandle(
      register DEVTABLE    * dt          ,// I -Device Table
      register dev_t         devno       ,// I -Device Number
      register DEVHANDLE   * dhp          //  O-Ptr to output handle
   );

/*---------------------------------------------------------------------------*/
/*                         Validate a Device Handle                          */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

BOOLEAN
   dhValid(
      const DEVTABLE       * dt          ,// I -Device Table
      const DEVHANDLE &      dh           // I -Device Handle
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  dhValid(dt,dh)                                                     \
   (((dh).devIndex >= 0 && (dh).devIndex < (dt)->majCount) &&               \
    ((dh).devMinor >= 0 && (dh).devMinor < CIE_MAX_DEV))

/*---------------------------------------------------------------------------*/
/*                  Get a DEV pointer from a Device Handle                   */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

DEV *
   getDEV(
      const DEVTABLE       * dt          ,// I -Device Table
      const DEVHANDLE &      dh           // I -Device Handle
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  getDEV(dt,dh)                                                      \
   (dhValid(dt,dh)                                                          \
      ? ((dt)->devMajor[(dh).devIndex].dev[(dh).devMinor])                  \
      : NULL)

/*---------------------------------------------------------------------------*/
/*         Set the Device Table entry for a handle to point to a DEV         */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

int
   setDEV(
      const DEVTABLE       * dt          ,// I -Device Table
      const DEVHANDLE &      dh          ,// I -Device Handle
      const DEV            * p            // I -Ptr to DEV Object
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  setDEV(dt,dh,p)                                                    \
   (dhValid(dt,dh)                                                          \
      ? (((dt)->devMajor[(dh).devIndex].dev[(dh).devMinor]=p),0)            \
      : EINVAL)

/*---------------------------------------------------------------------------*/
/*              Clear the Device Table Entry for a given hancle              */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

int
   clrDEV(
      const DEVTABLE       * dt          ,// I -Device Table
      const DEVHANDLE &      dh           // I -Device Handle
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  clrDEV(dt,dh)                                                      \
   (dhValid(dt,dh)                                                          \
      ? (((dt)->devMajor[(dh).devIndex].dev[(dh).devMinor]=NULL),0)         \
      : EINVAL)

/*---------------------------------------------------------------------------*/
/*       Increment the device counts for a handle in the device table        */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

int
   incrDevCount(
      const DEVTABLE       * dt          ,// I -Device Table
      const DEVHANDLE &      dh           // I -Device Handle
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  incrDevCount(dt,dh)                                                \
   ((++(dt)->devCount),(++(dt)->devMajor[(dh).devIndex].devCount))

/*---------------------------------------------------------------------------*/
/*       Decrement the device counts for a handle in the device table        */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

int
   decrDevCount(
      const DEVTABLE       * dt          ,// I -Device Table
      const DEVHANDLE &      dh           // I -Device Handle
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  decrDevCount(dt,dh)                                                \
   ((--(dt)->devCount),(--(dt)->devMajor[(dh).devIndex].devCount))

#endif
