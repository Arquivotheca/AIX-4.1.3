/* @(#)23	1.1  src/bos/usr/bin/virscan/vstypes.h, cmdsvir, bos411, 9428A410j 4/3/91 19:22:24 */
/*
 *   COMPONENT_NAME: CMDSVIR
 *
 *   FUNCTIONS: NONE
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1990,1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * Useful types.
 */
typedef unsigned char byte;
typedef unsigned int  word;
typedef unsigned int  boolean;

/*
 * Signature data block. Pointers to these go into hash tables.
 */
typedef struct sigdats
{
  byte second_byte;   /* First byte is tested with another table */
  byte third_byte;
  byte *signature;
  char *message;
  byte len_signature;
  struct sigdats *next_sibling;
  boolean com_files : 1;
  boolean exe_files : 1;
  boolean pause_if_found : 1;
  boolean scan_memory : 1;
  boolean at_offset : 1;
  boolean is_sig_frag : 1;
  boolean is_complex_sig : 1;
  boolean disable_mutant_scan : 1;
  long offset;
} sigdat;
