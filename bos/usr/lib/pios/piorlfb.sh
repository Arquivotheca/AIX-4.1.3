#!/usr/bin/ksh
# @(#)96        1.4  src/bos/usr/lib/pios/piorlfb.sh, cmdpios, bos411, 9428A410j 5/24/94 13:38:17
#
# COMPONENT_NAME: (CMDPIOS)
#
# FUNCTIONS: (Runs printer backend and sends its output to a remote
#	      queue (via 'rembak'))
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#  Usage:	piorlfb -f {+|!} -S Server -P Queue -N Filter \
#			[ -o BackendFlag ... ] PrintFile ...
#
#  Examples:	piorlfb -f ! -S piobe -P asc -N /usr/lib/lpd/aixshort \
#			-o -p -o 12 -o -f -o p /etc/motd
#		PIORLFB_DEBUG= piorlfb -f! -S galaxy -P centaur \
#			-N /canny lightyear

set -o nounset

# Perform initialization
typeset -r	piorlfb_tmpdf=/var/tmp/piorlfb_$$
typeset -i	piorlfb_exitcode=0		# exit code


# Function piorlfb_cleanup_exit
# Performs clean up and exits.
function piorlfb_cleanup_exit
{
   $DEBUG_SET

   /usr/bin/rm -f $piorlfb_tmpdf
   trap - TERM

   exit ${1:-${piorlfb_exitcode}}
}		# end - function piorlfb_cleanup_exit


# Set up a signal handler
trap 'trap TERM;kill -15 0;piorlfb_cleanup_exit 0' TERM
exec 1>/dev/null

# Declare functions defined later.
typeset -fu	piorlfb_proc_flags

# Define and initialize a few variables.
typeset 	DEBUG_SET=""
typeset 	DEBUG_USET=""
typeset		debug_tst
typeset -r	piorlfb_pgnm=${0##*/}		# program name
typeset		piorlfb_pbflags=""		# printer backend flags
typeset		piorlfb_rbflags=""		# rembak flags
typeset -r	PIOBE=/usr/lib/lpd/piobe
typeset -r	REMBAK=/usr/lib/lpd/rembak
typeset -r	DSPMSG=/usr/bin/dspmsg
typeset -r	PIORLFB_PRINT_USAGE="$DSPMSG piobe.cat -s 4 48 \
		'Usage: %1\$s -f {+|!} -S Server -P Queue -N Filter\n\t[ -o BackendFlag ... ] PrintFile ...\n' \
		$piorlfb_pgnm >/dev/console 2>&1"
typeset -r	AIXSHORTFILTER=aixshort

# Set debug flags on, if PIORLFB_DEBUG variable was set.
debug_tst=${PIORLFB_DEBUG-DEBUG}
[[ $debug_tst != DEBUG ]] &&
{ 
   DEBUG_SET="set -x"; DEBUG_USET="set +x"
}


# Function piorlfb_proc_flags
function piorlfb_proc_flags
{
   $DEBUG_SET
   typeset		flag
   typeset -i		fflag=0
   typeset -i		Sflag=0
   typeset -i		Pflag=0
   typeset -i		Nflag=0
   typeset		fval
   typeset		Nval
   typeset		ptflag=""

   while getopts :f:S:P:N:o: flag
   do
      case $flag in
         f)   fflag=1
	      fval="$OPTARG"
	      ;;
	 S)   Sflag=1
	      piorlfb_rbflags="$piorlfb_rbflags -$flag '$OPTARG'"
	      ;;
	 P)   Pflag=1
	      piorlfb_rbflags="$piorlfb_rbflags -$flag '$OPTARG'"
	      ;;
	 N)   Nflag=1
	      piorlfb_rbflags="$piorlfb_rbflags -$flag '$OPTARG'"
	      Nval="$OPTARG"
	      ;;
	 o)   piorlfb_pbflags="$piorlfb_pbflags '$OPTARG'"
	      ;;
         :)   $DSPMSG piobe.cat -s 4 30 \
	 	 '%1$s: %2$s requires a value\n' \
	 	 $piorlfb_pgnm $OPTARG >/dev/console 2>&1
	      eval $PIORLFB_PRINT_USAGE
	      piorlfb_cleanup_exit 1
	      ;;
         \?)  $DSPMSG piobe.cat -s 4 31 \
	 	 '%1$s: unknown option %2$s\n' \
	 	 $piorlfb_pgnm $OPTARG >/dev/console 2>&1
	      eval $PIORLFB_PRINT_USAGE
	      piorlfb_cleanup_exit 1
	      ;;
      esac
   done
   shift OPTIND-1

   # If any of the requisite flags was omitted, display error and exit.
   # Also, if no print files were specified, exit.
   [[ $fflag = 0 || $Sflag = 0 || $Pflag = 0 || $Nflag = 0 || $# = 0 ]] &&
      { eval $PIORLFB_PRINT_USAGE; piorlfb_cleanup_exit 1;}

   # Add file names to the backend flag list.
   piorlfb_pbflags="$piorlfb_pbflags ""$@"

   # Depending on the type of remote server, pass appropriate pass-through
   # flags.
   [[ $fval = "!" ]] &&
   {
      if [[ ${Nval##*/} = $AIXSHORTFILTER ]]
      then
	 ptflag=" -o -d -o p -o -j -o 0 -o -J -o 0 -o -Z -o 0"
      else
	 ptflag=" -o -f -o l"
      fi
   }
   piorlfb_rbflags="${piorlfb_rbflags}${ptflag}"

   return 0
}		# end - function piorlfb_proc_flags


# Main
# Main body of the script.
{
   $DEBUG_SET

   piorlfb_proc_flags "${@-}"
   eval $PIOBE $piorlfb_pbflags >|$piorlfb_tmpdf
   piorlfb_exitcode=$?
   [[ $piorlfb_exitcode = 0 ]] || piorlfb_cleanup_exit
   unset QUEUE_BACKEND; eval $REMBAK $piorlfb_rbflags $piorlfb_tmpdf
   piorlfb_cleanup_exit $?

   $DEBUG_USET
}		# end - main

