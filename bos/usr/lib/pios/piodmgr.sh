#!/usr/bin/ksh
# @(#)95        1.8  src/bos/usr/lib/pios/piodmgr.sh, cmdpios, bos411, 9438C411a 9/23/94 15:42:39
#
# COMPONENT_NAME: (CMDPIOS)
#
# FUNCTIONS: (performs various operations on the printer backend's alternate
#	      ODM database like compacting, turning on and off)
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
#  Usage:	piodmgr { -c | -h }
#
#  Examples:	piodmgr -c
#		PIODMGR_DEBUG= piodmgr -h
#
#  Note:	1.  This script must be run with root or printq privileges.
#		2.  Make sure that SMIT print spooling menus are not used
#		    and print queues are not created and/or modified, while
#		    this script is run.

set -o nounset

# Declare functions defined later.
typeset -fu	piodmgr_compact_odm

# Declare and initialize a few variables.
typeset 	DEBUG_SET=""
typeset 	DEBUG_USET=""
typeset		debug_tst
typeset -r	piodmgr_pgnm=${0##*/}		# program name
typeset -i	piodmgr_cflag=0
typeset -i	piodmgr_hflag=0
typeset -r	\
   piodmgr_vdir=${PIOVARDIR:-${PIOBASEDIR:-/var/spool/lpd/pio/@local}}
typeset -r	piodmgr_sdir=$piodmgr_vdir/smit
typeset -r	piodmgr_cfgfl=$piodmgr_vdir/piocfg
typeset -i	piodmgr_exitcode=0		# exit code
typeset -r	PIODMGR_PRINT_USAGE="dspmsg piobe.cat -s 4 47 \
		'Usage: %1\$s { -c | -h }\n' $piodmgr_pgnm >&2"
typeset -r	tmpdir=/tmp

# Set debug flags on, if PIODMGR_DEBUG variable was set.
debug_tst=${PIODMGR_DEBUG-DEBUG}
[[ $debug_tst != DEBUG ]] &&
{ 
   DEBUG_SET="set -x"; DEBUG_USET="set +x"
}


# Function piodmgr_compact_odm
function piodmgr_compact_odm
{
   $DEBUG_SET
   typeset -i		cfgexists=0
   typeset		oldhnm=
   typeset		newhnm
   typeset		s
   typeset -r		socdeffl=$tmpdir/socdefs_$$.cre
   typeset -r		socstzfls="$tmpdir/sm_cmd_opt_$$ $tmpdir/sm_cmd_hdr_$$ \
				  $tmpdir/sm_name_hdr_$$ $tmpdir/sm_menu_opt_$$"
   typeset -r		vpdir=${piodmgr_vdir%/*}
   typeset		tmps
   typeset -r		CLNUP_TFLS="rm -f $socdeffl $socstzfls"

   # Determine the old hostname (if any)
   [[ -r $piodmgr_cfgfl ]] && cfgexists=1
   if (( cfgexists == 1 ))
   then
      oldhnm=$(egrep '^[ 	]*hostname[ 	]*=' $piodmgr_cfgfl 2>/dev/null)
      oldhnm=$(print ${oldhnm#*=})
   fi

   # Determine the current host name
   tmps=$(hostname 2>/dev/null)
   if [[ -n $tmps ]]
   then
      newhnm=$(host "$tmps" 2>/dev/null || print - "$tmps")
   else
      newhnm=$(uname -n)
   fi
   newhnm=${newhnm%% *}

   # If the 'h' flag is set, perform compaction/updation only if the old
   # and new hostnames are not the same.
   [[ $piodmgr_hflag = 1 && -n $oldhnm && $oldhnm = $newhnm ]] && return 0

   # Set up signal handlers to perform clean up upon receipt of signals.
   trap "trap - EXIT; $CLNUP_TFLS;exit 2" HUP INT QUIT TERM
   trap "trap - EXIT; $CLNUP_TFLS" EXIT

   # Create a SMIT objclass definitions file.
   cat - >|$socdeffl << \EODATA
/* @(#)43       1.8  src/bos/objclass/smit_class.cre, smitobj, bos410, bos4.11293b 1/18/93 10:12:57 */
/*
 * COMPONENT_NAME: CMDSMIT (Input file to odmcreate)
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

class sm_cmd_opt {
	char id_seq_num[17];	key
	char id[65];		key
	vchar disc_field_name[65];
	vchar name[1025];
	vchar name_msg_file[1025];
	long name_msg_set;
	long name_msg_id;
	char op_type[2];
	char entry_type[2];
	long entry_size;
	char required[2];
	vchar prefix[1025];
	char cmd_to_list_mode[2];
	vchar cmd_to_list[1025];
	vchar cmd_to_list_postfix[1025];
	char multi_select[2];
	long value_index;
	vchar disp_values[1025];
	vchar values_msg_file[1025];
	long values_msg_set;
	long values_msg_id;
	vchar aix_values[1025];
	char help_msg_id[17];	key
	vchar help_msg_loc[1025];
	vchar help_msg_base[64];
	vchar help_msg_book[64];
	};

class sm_cmd_hdr {
	char id[65];		key
	vchar option_id[65];
	char has_name_select[2];
	vchar name[1025];
	vchar name_msg_file[1025];
	long name_msg_set;
	long name_msg_id;
	vchar cmd_to_exec[1025];
	char ask[2];
	char exec_mode[2];
	char ghost[2];
	vchar cmd_to_discover[1025];
	vchar cmd_to_discover_postfix[1025];
	long name_size;
	long value_size;
	char help_msg_id[17];	key
	vchar help_msg_loc[1025];
	vchar help_msg_base[64];
	vchar help_msg_book[64];
	};

class sm_menu_opt {
	char id_seq_num[17];	key
	char id[65];		key
	char next_id[65];	key
	vchar text[1025];
	vchar text_msg_file[1025];
	long text_msg_set;
	long text_msg_id;
	char next_type[2];
	char alias[2];
	char help_msg_id[17]; 	key
	vchar help_msg_loc[1025];
	vchar help_msg_base[64];
	vchar help_msg_book[64];
	};

class sm_name_hdr {
	char id[65];		key
	vchar next_id[65];
	vchar option_id[65];
	char has_name_select[2];
	vchar name[1025];
	vchar name_msg_file[1025];
	long name_msg_set;
	long name_msg_id;
	char type[2];
	char ghost[2];
	vchar cmd_to_classify[1025];
	vchar cmd_to_classify_postfix[1025];
	vchar raw_field_name[1025];
	vchar cooked_field_name[1025];
	char next_type[2];
	char help_msg_id[17]; 	key
	vchar help_msg_loc[1025];
	vchar help_msg_base[64];
	vchar help_msg_book[64];
	};

EODATA
   [[ $? = 0 ]] ||
   {
      dspmsg piobe.cat -s 4 41 '0782-572 Error(s) in creating "%1$s"\n' \
	 $socdeffl >&2
      exit 3
   }

   # Extract the SMIT ODM for print queues.
   for s in sm_cmd_opt sm_cmd_hdr sm_name_hdr sm_menu_opt
   do
      ODMDIR=$piodmgr_sdir odmget $s >|$tmpdir/${s}_$$ ||
      {
	 dspmsg piobe.cat -s 4 41 '0782-572 Error(s) in creating "%1$s"\n' \
	    $tmpdir/${s}_$$ >&2
	 exit 3
      }
   done

   # Change the old hostname references to the new hostname.
   [[ -n $oldhnm && $oldhnm != $newhnm ]] &&
   {
      tmps=$(print - "$vpdir"|sed -e "s#\/#\\\/#g")
      for s in sm_cmd_opt sm_cmd_hdr sm_name_hdr sm_menu_opt
      do
         # 'ed' seems to croak if files are bigger than 100K.  Hence 'ex'.
	 TERM=dumb ex $tmpdir/${s}_$$ >/dev/null <<- EODATA
		g/$tmps\/$oldhnm/s//$tmps\/$newhnm/g
		w
		q
		EODATA
	 [[ $? = 0 ]] ||
	 {
	    dspmsg piobe.cat -s 4 41 '0782-572 Error(s) in creating "%1$s"\n' \
	       $tmpdir/${s}_$$ >&2
	    exit 3
	 }
      done
   }

   # Restore signal handlers.  And trap a few signals so that the user doesn't
   # abort this script in a critical region.
   trap - EXIT HUP INT QUIT TERM; trap "" HUP INT QUIT TERM

   # Recreate SMIT ODM objects and reload the data.
   ODMDIR=$piodmgr_sdir odmcreate -c $socdeffl &&
      # chown root.printq $piodmgr_sdir/* &&
      chgrp printq $piodmgr_sdir/* &&
      chmod 664 $piodmgr_sdir/*
   [[ $? = 0 ]] ||
   {
      $CLNUP_TFLS
      dspmsg piobe.cat -s 4 49 \
	 '0782-650 Error in creating ODM in the path %1$s\n' \
	 $piodmgr_sdir >&2
      exit 3
   }
   for s in sm_cmd_opt sm_cmd_hdr sm_name_hdr sm_menu_opt
   do
      ODMDIR=$piodmgr_sdir odmadd $tmpdir/${s}_$$ ||
      {
	 $CLNUP_TFLS
	 dspmsg piobe.cat -s 4 50 \
	    '0782-651 Error in adding ODM objects in the path %1$s\n' \
	    $piodmgr_sdir >&2
	 exit 3
      }
   done

   # Create a link for the new host name.  And change the host name in the
   # config file.
   [[ -d $vpdir/$newhnm ]] ||
   {
   if [[ -n $oldhnm && $oldhnm != $newhnm ]]
   then
      mv -f $vpdir/$oldhnm $vpdir/$newhnm ||
	 ln -s $piodmgr_vdir $vpdir/$newhnm
   else
      ln -s $piodmgr_vdir $vpdir/$newhnm
   fi
   [[ $? = 0 ]] ||
   {
      $CLNUP_TFLS
      dspmsg piobe.cat -s 4 41 '0782-572 Error(s) in creating "%1$s"\n' \
	 $vpdir/$newhnm >&2
      exit 3
   }
   }
   if (( cfgexists == 1 ))
   then
      ed $piodmgr_cfgfl >/dev/null <<- EODATA
	/^[ 	]*hostname[ 	]*=/
	d
	a
	hostname	=	$newhnm
	.
	w
	q
	EODATA
   else
      print - "hostname\t=\t$newhnm">|$piodmgr_cfgfl &&
	 # chown root.printq $piodmgr_cfgfl &&
	 chgrp printq $piodmgr_cfgfl &&
	 chmod 664 $piodmgr_cfgfl
   fi
   [[ $? = 0 ]] ||
   {
      $CLNUP_TFLS
      dspmsg piobe.cat -s 4 41 '0782-572 Error(s) in creating "%1$s"\n' \
         $piodmgr_cfgfl >&2
      exit 3
   }
   # Restore signal handlers and perform clean up.
   trap - HUP INT QUIT TERM; $CLNUP_TFLS


   return 0
}		# end - function piodmgr_compact_odm


# Main
# Main body of the script.
{
   $DEBUG_SET

   # Process flags.
   while getopts :ch flag
   do
      case $flag in
         c)   piodmgr_cflag=1 ;;
         h)   piodmgr_hflag=1 ;;
         :)   dspmsg piobe.cat -s 4 30 \
	 	 '0782-561 %1$s: %2$s requires a value\n' \
	 	 $piodmgr_pgnm $OPTARG >&2
	      eval $PIODMGR_PRINT_USAGE
	      exit 1
	      ;;
         \?)  dspmsg piobe.cat -s 4 31 \
	 	 '0782-562 %1$s: unknown option %2$s\n' \
	 	 $piodmgr_pgnm $OPTARG >&2
	      eval $PIODMGR_PRINT_USAGE
	      exit 1
	      ;;
      esac
   done
   shift OPTIND-1; set --
   if (( ( piodmgr_cflag == 0 && piodmgr_hflag == 0 ) || \
	 ( piodmgr_cflag == 1 && piodmgr_hflag == 1 ) ))
   then
      eval $PIODMGR_PRINT_USAGE
      exit 1
   fi

   if (( piodmgr_cflag == 1 || piodmgr_hflag == 1 ))
   then
      piodmgr_compact_odm
   fi

   $DEBUG_USET
}		# end - main

