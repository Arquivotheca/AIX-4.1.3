#!/bin/ksh
# @(#)81        1.4  src/bldenv/bldtools/rename/promote_bad.sh, bldtools, bos412, GOLDA411a 5/2/94 15:23:08
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: promote_bad
#
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1992
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME: promote_bad
#
#? promote_bad:
#?
#?   Moves PTFs to the "bad" directory, update side files, and informs
#?   ctcb@ctgate and bteam@ctgate.
#?
#?   Flags:
#?     f <fixing defect number> (default: prompt user)
#?     d <bad defect number> (default: get from ptfs.bad.fixed/prompt user)
#?     t <to directory> (default: $PROD/bad)
#?     p <from directory> (default: $PROD)
#?     e - Do NOT prompt for editor.
#?     s - Do NOT check for superseding PTFs.
#?     m - Do NOT move PTFs, just update the ptfs.bad* files.
#?     i - Ignore the fact that the PTF is not in the "from" directory
#?         (implies -m is in use as well)
#?     h - Generates this message
#?
#?   Perform the following functions:
#?     o Ensures that the PTF is in the "from" directory.
#?     o Asks for the "fixing" defect number.
#?     o Finds all the PTFs that supersede the "bad" PTF in the "from" dir.
#?     o Ensures that an index file exists in the "to" directory.
#?     o Uses promote_prod to move the PTFs from "from" to "to".
#?     o Gets fixing defect abstract.
#?     o Sends a note informing of the move to:
#?     o Appends to move information to ptfs.bad
#?     o Appends PTF/bad_defect/fixing_defect information to ptfs.bad.fixed
#?     o Gives the user the opportunity to edit ptfs.bad.

typeset -u ORIG
typeset -u PTFS
typeset -L7 PTF
typeset -L7 PTF1
typeset -L7 PTF2
typeset -L10 COMP
typeset -L8 STATE
typeset -L8 OWNER
typeset -L3 SEV

PATH=/usr/afs/bin:$PATH
CMVC=/afs/austin/local/bin
ART_TOOLS=/afs/austin/depts/d66s/art_tools
BLDENV=/afs/austin/aix/325/bldenv/prod/usr/bin
PROD=/afs/austin/aix/ptf.images/320/prod
BAD=$PROD/bad
BAD_INFO=$BAD/ptfs.bad
BAD_NOTE=/tmp/bad.note
BAD_HEAD=$ART_TOOLS/bad.head
DEFECT_NOTE=/tmp/defect.note
DEFECT_HEAD=$ART_TOOLS/cmvc.defect.head
IFSold="$IFS"

if [[ "$1" = "-h" || "$1" = "-?" ]]; then      # help was requested ?
   grep "#"? $0 | cut -c4-                     # yes, display help comment
   exit 314                                    # and exit this script
fi

FIX=""
DEF=""
APP_OPT=""
TO=$BAD
FROM=$PROD
EDIT=1
SUP=1
MOVE=1
IGNORE=0
while getopts d:f:t:p:esmih OPT; do
   case $OPT in
      f) FIX=$OPTARG;;
      d) DEF=$OPTARG;;
      t) TO=$OPTARG;;
      p) FROM=$OPTARG;;
      e) EDIT=0;;
      s) SUP=0;;
      m) MOVE=0
         APP_OPT="-i";;
      i) IGNORE=1
         APP_OPT="-i"
         MOVE=0;;
      *) grep "#"? $0 | cut -c4-
         exit 314;;
   esac
done
shift OPTIND-1

ORIG="$*"
PTF="$ORIG"
PRIOs="badp serv buil"
LOC=$FROM

BY_WHOM="$(whoami)@$(uname -n)"
WHEN=$(date +"%y/%m/%d %T")
if [[ $IGNORE = "0" ]]; then
   [[ "$MOVE" = "0" ]] && LOC=$TO
   [[ -f $LOC/$PTF.ptf ]]
   FOUND=$?
else
   FOUND=0
fi

if [[ "$FOUND" = "0" ]]; then
   if [[ "$FIX" = "" ]]; then
      print "Promote to \"$TO\" is preformed by:"
      print "$BY_WHOM on $WHEN"
      print " "
      print "Please enter \"fixing\" defect number:"
      read FIX dummy
   fi
   if [[ "$FIX" = "" ]]; then
      print "Defect number must be entered!!!"
   else
      PTFS="$ORIG"
      if [[ "$SUP" = "1" ]]; then
         print "Searching for superseding PTFs..."
         IFS=": "
         for PTF in $ORIG; do
            grep $PTF $PROD/index | grep supersede | \
            while read lpp1 PTF1 sup lpp2 PTF2; do
               if [[ ("$PTF2" = "$PTF") && ("$PTFS" != *"$PTF1"*)]]; then
                                       # superseding PTF is not in the list
                  PTFS="$PTFS $PTF1"   # so add it
               fi
            done
         done
         IFS="$IFSold"
      fi
      if [[ "$MOVE" = "1" ]]; then
         touch $TO/index
         print "promote_prod -b $FROM -p $TO $PTFS"
###         $BLDENV/promote_prod -b $FROM -p $TO $PTFS 2>&1 | \
###            grep -v "unable to duplicate owner and mode after move."
         $BLDENV/promote_prod -b $FROM -p $TO $PTFS 2>&1
         DONE=$?
         EXTRA=""
      else
         DONE=0
         EXTRA="(not moved)"
      fi
      if [[ "$DONE" = "0" ]]; then
         $CMVC/Report -view defectView -where "name=$FIX" -family aix | \
               tail -3 | read pre NAME COMP STATE ori OWNER SEV AGE PRIO ABS
         if [[ ("$?" != "0") || ("$NAME" = "") ]]; then
            NAME=$FIX
            ABS="*** had problems getting information from CMVC ***"
         fi
         if [[ "$PRIOs" != *"$PRIO"* ]]; then
            ABS="$PRIO $ABS"
            PRIO="<none>"
         fi
         if [[ "$PRIO" != "badp" ]]; then
            mail -s "badptf:$FIX" test@mariscal < /dev/null
         fi
         if [[ "$MOVE" = "1" ]]; then
            cp $BAD_HEAD $BAD_NOTE
            print "$NAME $COMP $STATE $OWNER $SEV $AGE $PRIO" >> $BAD_NOTE
            print "       $ABS" >> $BAD_NOTE
            print "$PTFS" >> $BAD_NOTE
            /usr/lib/sendmail -t < $BAD_NOTE
            rm -f $BAD_NOTE
         fi

         if [[ "$IGNORE" = "0" ]]; then
            print "PTF $PTFS now in \"$TO\""
            cp $DEFECT_HEAD $DEFECT_NOTE
            print "$PTFS" >> $DEFECT_NOTE
            $CMVC/Defect -note $FIX -remarks - < $DEFECT_NOTE -family aix
            rm -f $DEFECT_NOTE
         fi

         for PTF in $PTFS; do
            print "$PTF defect $FIX - $ABS $WHEN $BY_WHOM $EXTRA" >> $BAD_INFO
            $ART_TOOLS/append_bad.fixed $APP_OPT $PTF $FIX $DEF
         done

         if [[ "$EDIT" = "1" ]]; then
            print "Enter editor name to edit $BAD_INFO"
            read EDITOR
            [[ "$EDITOR" != "" ]] && $EDITOR $BAD_INFO
         fi
      else
         print "PTF $PTFS was NOT moved!!!!!!!!"
      fi
   fi
else
   print "PTF $PTF NOT found on $FROM"
fi
