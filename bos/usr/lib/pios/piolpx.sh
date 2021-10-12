#!/usr/bin/ksh
# @(#)93        1.1  src/bos/usr/lib/pios/piolpx.sh, cmdpios, bos411, 9428A410j 7/23/93 16:41:47
#
# COMPONENT_NAME: (CMDPIOS)
#
# FUNCTIONS: (Runs printer backend and pipes its output to
#	      a printer connected to an Xstation (via 'catlpx'))
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#  Usage:	piolpx PseudoDevice [ BackendFlags ] PrintFile ...
#
#  Examples:	piolpx p@xisland1 -p 12 /etc/motd
#		PIOLPX_DEBUG= piolpx s0@anonx /etc/inetd.conf

set -o nounset

trap 'trap 15;kill -15 0;exit 0' 15
exec 1>/dev/null

# Declare functions defined later.
typeset -fu	piolpx_build_catlpx

# Declare and initialize a few variables.
typeset 	DEBUG_SET=""
typeset 	DEBUG_USET=""
typeset		debug_tst
typeset -r	piolpx_pgnm=${0##*/}		# program name
typeset		piolpx_clflags=""		# catlpx flags and args
typeset		piolpx_xstnm			# xstation name
typeset -i	piolpx_exitcode=0		# exit code
typeset -r	PIOBE=/usr/lib/lpd/piobe
typeset -r	CATLPX=/usr/lpp/x_st_mgr/bin/catlpx
typeset -r	PIOMGPDEV=/usr/lib/lpd/pio/etc/piomgpdev
typeset -r	DSPMSG=/usr/bin/dspmsg

# Set debug flags on, if PIOLPX_DEBUG variable was set.
debug_tst=${PIOLPX_DEBUG-DEBUG}
[[ $debug_tst != DEBUG ]] &&
{ 
   DEBUG_SET="set -x"; DEBUG_USET="set +x"
}


# Function piolpx_build_catlpx
function piolpx_build_catlpx
{
   $DEBUG_SET
   typeset -r		pd=$1
   typeset -r		pdfl=${1}.xstation
   typeset		cinfo
   typeset		interface
   typeset		port
   typeset		speed
   typeset		parity
   typeset		bpc
   typeset		stops

   cinfo=$($PIOMGPDEV -p "$pd" -t xstation -D -a xstation) ||
   {
      $DSPMSG piobe.cat -s 4 24 \
	 '%1$s: Error in extracting communication info from the pseudo-device file %2$s\n' \
	 $piolpx_pgnm $pdfl >/dev/console 2>&1
      exit 2
   }
   piolpx_xstnm=$(print -r - "$cinfo"|tail -1)
   [[ -n $piolpx_xstnm ]] ||
   {
      $DSPMSG piobe.cat -s 4 25 \
	 '%1$s: Error in extracting xstation name from the pseudo-device file %2$s\n' \
	 $piolpx_pgnm $pdfl >/dev/console 2>&1
      exit 2
   }

   cinfo=$($PIOMGPDEV -p "$pd" -t xstation -D -a interface) ||
   {
      $DSPMSG piobe.cat -s 4 24 \
	 '%1$s: Error in extracting communication info from the pseudo-device file %2$s\n' \
	 $piolpx_pgnm $pdfl >/dev/console 2>&1
      exit 2
   }
   interface=$(print -r - "$cinfo"|tail -1)
   [[ -n $interface ]] ||
   {
      $DSPMSG piobe.cat -s 4 26 \
	 '%1$s: Error in extracting interface type from the pseudo-device file %2$s\n' \
	 $piolpx_pgnm $pdfl >/dev/console 2>&1
      exit 2
   }

   [[ $interface = serial ]] &&
   {
      cinfo=$($PIOMGPDEV -p "$pd" -t xstation -D -a port) ||
      {
         $DSPMSG piobe.cat -s 4 24 \
	    '%1$s: Error in extracting communication info from the pseudo-device file %2$s\n' \
	    $piolpx_pgnm $pdfl >/dev/console 2>&1
         exit 2
      }
      port=$(print -r - "$cinfo"|tail -1)
      [[ -n $port ]] ||
      {
         $DSPMSG piobe.cat -s 4 27 \
	    '%1$s: Error in extracting port from the pseudo-device file %2$s\n' \
	    $piolpx_pgnm $pdfl >/dev/console 2>&1
         exit 2
      }
      cinfo=$($PIOMGPDEV -p "$pd" -t xstation -D -a speed) ||
      {
         $DSPMSG piobe.cat -s 4 24 \
	    '%1$s: Error in extracting communication info from the pseudo-device file %2$s\n' \
	    $piolpx_pgnm $pdfl >/dev/console 2>&1
         exit 2
      }
      speed=$(print -r - "$cinfo"|tail -1)
      [[ -n $speed ]] || speed=9600
      cinfo=$($PIOMGPDEV -p "$pd" -t xstation -D -a parity) ||
      {
         $DSPMSG piobe.cat -s 4 24 \
	    '%1$s: Error in extracting communication info from the pseudo-device file %2$s\n' \
	    $piolpx_pgnm $pdfl >/dev/console 2>&1
         exit 2
      }
      case $(print -r - "$cinfo"|tail -1) in
	 odd)		parity=o;;
	 even)		parity=e;;
	 "stuck odd")	parity=so;;
	 "stuck even")	parity=se;;
	 *)		parity=n;;
      esac
      cinfo=$($PIOMGPDEV -p "$pd" -t xstation -D -a bpc) ||
      {
         $DSPMSG piobe.cat -s 4 24 \
	    '%1$s: Error in extracting communication info from the pseudo-device file %2$s\n' \
	    $piolpx_pgnm $pdfl >/dev/console 2>&1
         exit 2
      }
      bpc=$(print -r - "$cinfo"|tail -1)
      [[ -n $bpc ]] || bpc=8
      cinfo=$($PIOMGPDEV -p "$pd" -t xstation -D -a stops) ||
      {
         $DSPMSG piobe.cat -s 4 24 \
	    '%1$s: Error in extracting communication info from the pseudo-device file %2$s\n' \
	    $piolpx_pgnm $pdfl >/dev/console 2>&1
         exit 2
      }
      stops=$(print -r - "$cinfo"|tail -1)
      [[ -n $stops ]] || stops=1

      piolpx_clflags="-$port $speed,$parity,$bpc,$stops"
   }

   return 0
}		# end - function piolpx_build_catlpx


# Main
# Main body of the script.
{
   $DEBUG_SET

   if (( $# < 2 ))
   then
      $DSPMSG piobe.cat -s 4 23 \
	 'Usage: %1$s PseudoDevice [ BackendFlags ] PrintFile ...\n' \
	 $piolpx_pgnm >/dev/console 2>&1
      exit 1
   fi
   piolpx_build_catlpx "$1"; shift
   $PIOBE "${@-}" | eval $CATLPX $piolpx_xstnm $piolpx_clflags
   exit $?

   $DEBUG_USET
}		# end - main

