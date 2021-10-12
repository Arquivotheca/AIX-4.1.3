/* @(#)96  1.32  src/bos/usr/include/inuerr.h, cmdinstl, bos41J, 9510A_all 2/27/95 12:15:28 */
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inuerr.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

 /**-----------------------------------------------------------------
  **   NOTE - All return codes should be <= 255.  Problems exist if a 
  **   number > 255 is returned from main (> 8 bits).
  **----------------------------------------------------------------*/

#ifndef _H_INUERR
#define _H_INUERR

#define INUGOOD       0 /* Normal return, no errors occurred                  */
#define INUNOLPP      2 /* No options specified can be applied                */
#define INUCANCL      5 /* update procedure cancelled update                  */
#define INUBADLOG     6 /* Error while opening log file                       */
#define INUCANRV      7 /* update cancelled, but recovery needed              */
#define INUABORT      8 /* user abort                                         */
#define INUBFFER      9 /* Error running bffcreate                            */
#define INUNOGRP     10 /* User not in install group                          */
#define INUVPDER     11 /* Error returned from VPD call                       */
#define INURESTR     12 /* Error running restore                              */
#define INUFS        20 /* Too many mounted file systems.                     */
#define INUACCS      91 /* Permission / access error                          */
#define INUOPEN      92 /* Unable to open file                                */
#define INUCREAT     93 /* Unable to create a file                            */
#define INUUNREC     98 /* Failure during error recovery                      */
#define INUINVAL     99 /* Invalid input parameter                            */
#define INUUNKWN    100 /* Received unknown return code                       */
#define INUNOSAV    101 /* Specified save directory doesn't exist             */
#define INUBADC1    102 /* Copy of file failed in inurecv                     */
#define INUNOARP    103 /* arp file does not exist                            */
#define INUNOSVF    104 /* Saved file not found in save directory             */
#define INUBADSC    105 /* Failed trying to create save directory             */
#define INUBADRC    106 /* Failed trying to restore updated files             */
#define INUBADC2    107 /* Copy of file failed in inusave                     */
#define INUFULLD    108 /* Disk full error from /etc/restore                  */
#define INUNOMK     109 /* mkdir failed making dir in inu_mk_dir.c            */
#define INUBADAR    110 /* archive failed in inu_archive.c                    */
#define INUSYSERR   115 /* A library or kernal routine failed                 */
#define INUMOUNT    116 /* Unable to mount a need file system                 */
#define INUUMOUNT   117 /* Unable to umount a file system                     */
#define INUSETVAR   118 /* Unable to set environment variable                 */
#define INUINTER    119 /* Command was interrurpt                             */
#define INUUSE      120 /* Command is already in use                          */
#define INUSYSFL    130 /* inusave: system call failed                        */
#define INUBADUL    131 /* inurecv: UPDATE_LIST (update.list) is bad          */
#define INUBADARL   132 /* inurecv: ARCHIVE_LIST(archive.list) is bad         */
#define INUVPDADD   133 /* inurecv: failed recovering vpd entry               */
#define INUSRTFL    134 /* inusave: Sort Failed                               */
#define INUENVNS    135 /* inusave, inurecv: Environment var not set          */
#define INUNOWRPR   136 /* inusave: No write permissions                      */
#define INUVPDFL    137 /* Failed performing a vpd access/update              */
#define INUBADIR    138 /* Bad dir passed to a cmd                            */
#define INUCRTOC    139 /* Failed creating toc linked list                    */
#define INUMALLOC   140 /* malloc call failed                                 */
#define INUSAGE     141 /* Generic usage error                                */
#define INUSUMMARY  142 /* inu_summary detected one or more installp failure  */
#define INUNIM      143 /* installp run from cmdline in the NIM environment   */
#define INUSYNTAX   200 /* Syntax error                                       */
#define INUBADMN    201 /* An invalid - parameter was specified               */
#define INUTOOFW    202 /* One or more parameters were missing                */
#define INUNOALF    203 /* No al_vv.rr.llll files were found                  */
#define INUTOOMN    204 /* Too many parameters were found                     */
#define INUNORP1    205 /* inurecv failed replacing constituent               */
#define INUNORP2    206 /* inurest failed replacing constituent               */
#define INUNOAP1    207 /* Apply list doesn't exist in inusave                */
#define INUNOAP2    208 /* Apply list doesn't exist in inurest                */
#define INULIBER    209 /* Error extracting liblpp.a                          */
#define INUNODEV    211 /* Cannot open device                                 */
#define INUNOINV    212 /* Inventory file not found                           */
#define INUNOWRT    213 /* inu_mk_dir can not write to needed dir             */
#define INUCHDIR    214 /* Cannot change directories                          */
#define INUNOTIN    220 /* Not installp media                                 */
#define INUNOTUP    221 /* Not updatep media                                  */
#define INUPEND     249 /* Pending updates are outstanding                    */
#define INUELVL     250 /* Error in mod level param (inudocm)                 */
#define INUECNTL    251 /* No restore of /usr/sys/inst_updt/control           */
#define INUNODIR    253 /* No /usr/lpp/pgm-name directory exists              */
#define INUBADLP    254 /* An lpp name specified is invalid                   */
#define INUNOSPC    255 /* Not enough disk space to install/update            */

/** ---------------------------------------------------------------- *
 **  Values for the "installp.summary" file
 **  --------------------------------------
 **  The format of each line of the colon-delimited "installp.summary" 
 **  (ascii file) is
 **    "<completion code>:<option name>:<final state>:<part>:<level>:"
 **
 **   Field Description
 **   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 **   <completion code>  -  result of installation for the option 
 **   <option name>      -  name of option installed/rejected
 **   <final state>      -  final state of <option name>
 **   <part>             -  part of option installed/rejected
 **   <level>            -  the level of option installed/rejected 
 **
 **  An sample line of this file could be 
 **     "0:bosnet.tcpip.obj:3:U:3.2.0.0:"
 **  which says that the usr part of the bosnet.tcpip.obj option at
 **  level 3.2.0.0 had a successful installation with a final state 
 **  of APPLIED.
 ** ---------------------------------------------------------------- */

/** --------------------------------------------------------------------- *
 ** Values for the <completion code> field, including
 **  - Installation sucess code
 **  - Installp.Summary Pre-Installation Failure/Warning codes (IS_PIFW_)
 **  - Installation failure code
 ** --------------------------------------------------------------------- */

#define IS_INST_SUCCESS                   0

#define IS_PIFW_NOT_ON_MEDIA             400
#define IS_PIFW_REQUISITE_FAIL           401
#define IS_PIFW_ALREADY_INSTALLED        402
#define IS_PIFW_ALREADY_SEDED            403
#define IS_PIFW_TO_BE_SEDED              404
#define IS_PIFW_BROKEN                   405
#define IS_PIFW_MUST_COMMIT              406
#define IS_PIFW_NO_USR_MEANS_NO_ROOT     407
#define IS_PIFW_NOTH_TO_APPLY            408
#define IS_PIFW_NOTH_TO_COMMIT           409
#define IS_PIFW_NOTH_TO_REJECT           410
#define IS_PIFW_NOTH_TO_DEINSTL          411
#define IS_PIFW_MUST_DO_ROOT_TOO         412
#define IS_PIFW_ALREADY_COMMIT           413
#define IS_PIFW_OTHER_BROKENS            414

#define IS_PIFW_DEINST_3_1               415
#define IS_PIFW_DEINST_3_2               416
#define IS_PIFW_FAILED_PRE_D             417
#define IS_PIFW_NO_DEINST_BOS            418
#define IS_PIFW_DEINST_MIG               420

#define IS_PIFW_PART_INCONSIST           421
#define IS_PIFW_CAN_BE_SEDED             422
#define IS_PIFW_ROOT_CAN_BE_SEDED        423
#define IS_PIFW_COMMITTED_NO_REJECT      424
#define IS_PIFW_SUP_OF_BROKEN            425

#define IS_PIFW_OEM_MISMATCH		 426
#define IS_PIFW_OEM_REPLACED		 427
#define IS_PIFW_OEM_BASELEVEL		 428

#define IS_INST_FAILURE                  500

/** --------------------------------------- *
 **  Values for the <final state> field
 ** --------------------------------------- */

/**  These values are found in the swvpd.h include file.
     They are the values defines that begin with ST_.
     ST_AVAILABLE, for example. **/

/** --------------------------------------- *
 **  Values for the <part> field
 ** --------------------------------------- */
#define IS_USR_PART      'U'
#define IS_ROOT_PART     'R'
#define IS_SHARE_PART    'S'

#endif /* _H_INUERR */

