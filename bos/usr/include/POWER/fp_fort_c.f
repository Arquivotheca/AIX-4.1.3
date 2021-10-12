* @(#)61	1.2  src/bos/usr/include/POWER/fp_fort_c.f, libccnv, bos411, 9428A410j 5/13/93 15:54:26
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

* This file is used as a FORTRAN language include file to 
* define a named common "fptrap_comm" which contains the 
* arguments to the fp_enable() and fp_trap() calls, so that
* these calls can be made directly from FORTRAN programs.
*
* The fp_fort_t.f file should be used in a BLOCK DATA subprogram
* to initialize the "fptrap_comm" common area.
*
      integer fp_enbl_summ,     ! enable all traps
     $     trp_invalid,         ! enable invalid operation trap
     $     trp_overflow,        ! enable overflow trap
     $     trp_underflow,       ! enable underflow trap
     $     trp_div_by_zero,     ! enable divide by zero trap
     $     trp_inexact          ! enable inexact trap
*
* The following are used as arguments for fp_trap().
* Not all arguments are valid on all platforms -- fp_trap returns
* fp_trap_unimpl if the requested mode is not supported by
* the hardware.
*
      integer fp_trap_sync,     ! turn synchronous execution on 
     $     fp_trap_off,         ! turn trapping off
     $     fp_trap_query,       ! querry state
     $     fp_trap_error,       ! return code for invalid argument
     $     fp_trap_imp,         ! turn imprecise trap mode on
     $     fp_trap_imp_rec,     ! turn imprecise recoverable mode on
     $     fp_trap_fastmode,    ! set fastest mode on this platform
     $     fp_trap_unimpl       ! return code -- requested mode not
                                !    available
*      
      common /fptrap_comm/ fp_enbl_summ, trp_invalid, trp_overflow,
     $     trp_underflow, trp_div_by_zero, trp_inexact, fp_trap_sync,
     $     fp_trap_off, fp_trap_query, fp_trap_error,
     $     fp_trap_imp, fp_trap_imp_rec, fp_trap_fastmode,
     $     fp_trap_unimpl  
