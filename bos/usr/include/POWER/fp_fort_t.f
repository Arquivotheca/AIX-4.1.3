* @(#)62	1.2  src/bos/usr/include/POWER/fp_fort_t.f, libccnv, bos411, 9428A410j 5/13/93 15:54:36
*
* COMPONENT_NAME: (libccnv)
*
* ORIGINS: 27
*
* (C) COPYRIGHT International Business Machines Corp. 1991,1993
* All Rights Reserved
* Licensed Materials - Property of IBM
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

* This file is used (in a BLOCK DATA subprogram) to initialize
* the "fptrap_comm" named common area defined in fp_fort_c.f.
*
      data fp_enbl_summ          /z'000000F8'/
      data trp_invalid           /z'00000080'/
      data trp_overflow          /z'00000040'/
      data trp_underflow         /z'00000020'/
      data trp_div_by_zero       /z'00000010'/
      data trp_inexact           /z'00000008'/
*
      data fp_trap_sync          /1/
      data fp_trap_off           /0/
      data fp_trap_query         /2/
      data fp_trap_imp           /3/
      data fp_trap_imp_rec       /4/
      data fp_trap_fastmode    /128/
      data fp_trap_error        /-1/
      data fp_trap_unimpl       /-2/
