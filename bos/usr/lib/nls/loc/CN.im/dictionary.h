/* @(#)34	1.1  src/bos/usr/lib/nls/loc/CN.im/dictionary.h, ils-zh_CN, bos41B, 9504A 12/19/94 14:34:26  */
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************************************************************************/
/*                                                                            */
/* MODULE NAME:         dictionary                                            */
/*                                                                            */
/* DESCRIPTIVE NAME:    Chinese Input Method make dictionary file             */
/*                                                                            */
/* FUNCTION:                                                                  */
/*                                                                            */
/******************************************************************************/
/*
#include "cnedinit.h"
#include "chinese.h"
*/

#define ITEMLEN    512
#define PYCODELEN  8
#define FSSCODELEN 4
#define FSCODELEN  5
#define CODELEN    3
/*************************************/
/*    Dictionary Head Structure      */
/*************************************/
struct dictinfo {
      char   d_name[8] ;           /* The name of tne dictionary              */
      int    d_owner   ;           /* User id which create the dictionary     */
      int    d_ctime   ;           /* Creating time                           */
      int    d_mtime   ;           /* Modifying time                          */
      struct Index *d_iaddr  ;     /* The address of the index table          */
      int    d_ilen;               /* The length of the index table           */
      char   *d_baddr;             /* The address of BOBY                     */
      int    d_blen;               /* The length of BOBY                      */
};   

struct Index {
      unsigned char *index;
      int           length;
} ;

struct Index  PyIndex[26];
struct Index  LeIndex[85];
struct Index  EnIndex[26];
struct Index  FssIndex[25];
struct Index  FsIndex[5];

