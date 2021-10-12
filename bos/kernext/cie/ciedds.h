/* @(#)41   1.6  src/bos/kernext/cie/ciedds.h, sysxcie, bos411, 9428A410j 4/1/94 15:47:55 */

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 * DESCRIPTION:
 *
 *    COMIO Emulator Device-Dependent Structure
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

#if !defined(CIEDDS_H)
#define  CIEDDS_H

#include "devtype.h"
#include <sys/types.h>
#include <sys/comio.h>
#include <sys/err_rec.h>
#include <sys/ciouser.h>
#include <sys/tokuser.h>
#include <sys/fddiuser.h>
#include <sys/entuser.h>
#include "fddidds.h"

/*---------------------------------------------------------------------------*/
/*                        Device Dependent Structure                         */
/*---------------------------------------------------------------------------*/

typedef struct DDS             DDS;

struct DDS
{
   /*------------------------------------------------------------------------*/
   /*                    Common Device Driver Parameters                     */
   /*------------------------------------------------------------------------*/

   CIE_DEV_TYPE              devType;        // CIE Device Type Code
   CIE_DEV_SUB_TYPE          devSubType;     // Device SubType Code
   char                      devName[16];    // logical name of device
   char                      nddName[16];    // CDLI NDD Name
   int                       recQueueSize;   // size of recv queue, per user
   int                       staQueueSize;   // size of stat queue, per user

   /*------------------------------------------------------------------------*/
   /*                    Device-Type-Specific Parameters                     */
   /*------------------------------------------------------------------------*/

   union
   {
      /*---------------------------------------------------------------------*/
      /*                           E t h e r n e t                           */
      /*---------------------------------------------------------------------*/

      struct dds_ent
      {
         char  hdwAdptAddr[ent_NADR_LENGTH]; // Burned-in adapter address
         char  curAdptAddr[ent_NADR_LENGTH]; // Current adapter address
         int                 netIdOffset;    // Offset to net-id fld
         int                 eTypeOffset;    // Offset to ethertype
         ccc_vpd_blk_t       vpd;            // Ethernet Vital Product Data
      } ent;

      /*---------------------------------------------------------------------*/
      /*                               F D D I                               */
      /*---------------------------------------------------------------------*/

      struct dds_fddi
      {
         char  hdwAdptAddr[FDDI_NADR_LENGTH];// Burned-in adapter address
         char  curAdptAddr[FDDI_NADR_LENGTH];// Current adapter address
         uchar               passBeacon;     // SMT User receives Beacon Frames
         uchar               passSMT;        // SMT User receives SNT Frames
         uchar               passNSA;        // SMT User receives NSA Frames
         fddi_vpd_t          vpd;            // FDDI Vital Product Data
      } fddi;

      /*---------------------------------------------------------------------*/
      /*                         T o k e n   R i n g                         */
      /*---------------------------------------------------------------------*/

      struct dds_tok
      {
         char  hdwAdptAddr[TOK_NADR_LENGTH]; // Burned-in adapter address
         char  curAdptAddr[TOK_NADR_LENGTH]; // Current adapter address
         int                 ringSpeed;      // Ring speed (4/16)
         tok_vpd_t           vpd;            // TokenRing Vital Product Data
      } tok;

   } ds;
};

#endif
