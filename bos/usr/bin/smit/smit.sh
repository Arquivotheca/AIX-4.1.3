#! /bin/ksh
# @(#)10        1.10  src/bos/usr/bin/smit/smit.sh, cmdsmit, bos41J, 9509A_all 2/27/95 17:26:15
#
#   COMPONENT_NAME: CMDSMIT
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1990,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


# in case the user has 'set -u' in the .kshrc
set +u


# check the command options looking for -C(urses) or -M(otif)
echo " $* " | grep " \-[DfhtvXx]*C[DfhtvXxolsnmd]* " > /dev/null
if [ $? -eq 0 ]
then
   Cflag=TRUE
fi

echo " $* " | grep " \-[DfhtvXx]*M[DfhtvXxolsnmd]* " > /dev/null
if [ $? -eq 0 ]
then
   Mflag=TRUE
fi


# if both flags are specified, display the usage statement
if [ -n "$Cflag" ] && [ -n "$Mflag" ]
then
   exec /usr/bin/smitty -?
fi

# if -C flag specified, run smitty
if [ -n "$Cflag" ]
then
   exec /usr/bin/smitty $@
fi

 
# if -M or DISPLAY is set, try to start msmit
if [ -n "$Mflag" -o "$DISPLAY" ]
then
   #If msmit and all the X libraries are available, start Motif smit
   if [ -f "/usr/lib/libX11.a" -a \
        -f "/usr/lib/libXt.a" ]
   then
      # check what version of Motif is running
      ls -l /usr/lib/libXm.a | grep "/usr/lpp/X11/Motif1.2/lib/libXm.a" > /dev/null
      if [ $? -eq 0 ]
      then
   	 if [ -f "/usr/lpp/X11/Motif1.2/lib/libXm.a" -a \
	      -f "/usr/lpp/X11/Motif1.2/bin/msmit" ]
         then
           exec /usr/lpp/X11/Motif1.2/bin/msmit $@
	 fi
      else
         if [ -f "/usr/lib/libXm.a" -a \
              -f "/usr/lpp/X11/bin/msmit"  ]
         then
           exec /usr/lpp/X11/bin/msmit $@
         fi
      fi
   fi
fi

#Otherwise start Curses smit
exec /usr/bin/smitty $@
