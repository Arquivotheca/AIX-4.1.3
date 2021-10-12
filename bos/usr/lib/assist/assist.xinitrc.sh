#!/bin/ksh
#
# @(#)08        1.29  src/bos/usr/lib/assist/assist.xinitrc.sh, cmdassist, bos41J, 9511A_all 3/12/95 11:35:50
#
#   COMPONENT_NAME:  cmdassist 
#
#   FUNCTIONS: assist.xinitrc
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# This script is invoked by /usr/lpp/X11/bin/xinit

#***********************************************************
# HINT: For XDM users.  It would be useful for you to set
#       your PATH in your shell's configuration file (ie, 
#       kshrc, or cshrc).  This will allow clients started 
#       within your XINITRC script to have the proper user
#       environment.  Korne shell users should remember to
#       set ENV=$HOME/.kshrc in their .profile file.
#***********************************************************
XINITRCMSG=/usr/lib/nls/msg/$LANG

#unset for kanji
if [ "$LC_MESSAGES" = "C@lft" ]
then
   unset LC_MESSAGES
fi

#****************************************************************
#								
#  Set the X server's keyboard keysyms to the match the	
#  engravings of the user's keyboard.			
#
#   - Querying odm (querykbd) to get keyboard id (e.g. En_US).
#
#   - If querykbd returns NULL or there is no keyboard file found,
#  then the environment variable $LANG is used for the keymap.
#
#  Don't remap keybaord if this is an Xstation
#
#*****************************************************************
if [ -z "$XSTATION" ]
then
    KBD=""
    XDIR=/usr/lpp/X11/defaults/xmodmap

    if [ -r $HOME/.Xkeyboard ]
    then
        KBD=$HOME/.Xkeyboard
    else
        KBD_LANG=`/usr/lpp/X11/bin/querykbd`

        if [ "$KBD_LANG" = "NULL" ]
        then
	    dspmsg $XINITRCMSG/xinit.cat 2 '1356-801 xinit: Failed to query odm for keyboard id\n'
        else
            if [ "$KBD_LANG" != "C.hft" ]
            then
                if [ -r $IMKEYMAPPATH/$KBD_LANG/keyboard ]
                then
                     KBD=$IMKEYMAPPATH/$KBD_LANG/keyboard
                else
                    if [ "$IMKEYMAPPATH" = "/usr/lib/nls/im.alt"     	\
                        -a -r $XDIR/$KBD_LANG/keyboard.alt ]
                    then
                        KBD=$XDIR/$KBD_LANG/keyboard.alt
                    else
                        if [ -r $XDIR/$KBD_LANG/keyboard ]
                        then
                            KBD=$XDIR/$KBD_LANG/keyboard
                        fi
                    fi
                fi
            fi
        fi
    fi
    if [ "$KBD" != "" ]
    then
        xmodmap $KBD
    else
        if [ "$IMKEYMAPPATH" = "/usr/lib/nls/im.alt"   \
                    -a -r $XDIR/$LANG/keyboard.alt ]
        then
            xmodmap $XDIR/$LANG/keyboard.alt
        else
            if [ -r $XDIR/$LANG/keyboard ]
            then
	        xmodmap $XDIR/$LANG/keyboard
            fi
        fi
    fi
fi

#****************************************************************
#  Set up LANG-dependent default font resources for
#  Motif applications that lack app-default files.
#****************************************************************

if [ $LANG = "zh_TW" ]
then
   if [ -f $HOME/.Xdefaults ]
   then
       xrdb -merge $HOME/.Xdefaults
   fi
   xrdb -merge << STOP
*fontList: *-sung-*:
STOP
fi

if [ $LANG = "ko_KR" ]
then
   if [ -f $HOME/.Xdefaults ]
   then
       xrdb -merge $HOME/.Xdefaults
   fi
   xrdb -merge << STOP
*fontList: *-myungjo-*:
STOP
fi

#****************************************************************
#								*
#  Start the X clients.  Change the following lines to		*
#  whatever command(s) you desire!				*
#								*
#  The default clients are an analog clock (xclock), an hft 	*
#  terminal emulator (aixterm), the X Desktop Manager (xdt), 	*
#  and the Motif Window Manager (mwm).				*
#								*
#****************************************************************

if [ -n $1 ] ; then
	# Start the GUI installation assist
	# Start the window manage in the background. Runninng this in the
	# background causes an exit from X when the assist.xinitrc program
	# is exited.

	# This will retain the system resources
	xrdb -retain -merge /usr/lib/assist/assist.resources

	# This will set up the root menu window (ie Workspace menu)
	mwm > /dev/null 2>&1 &
	MWM_PID=$!
	echo "MWM_PID=$MWM_PID" >/tmp/.gui.dat
	# See if we're running from command line or remotely 
    `tty | grep pts | grep -v grep > /dev/null`
    PTS=$?

    # Put a pretty backdrop if xsetroot is installed and if we are
	# locally on the command line
    if [ -f /usr/bin/X11/xsetroot -a $PTS -eq 1 -a ! "$SOFTLIC_ONLY" ] ; then
		xsetroot -fg grey60 -bg grey40 -bitmap /usr/lib/assist/bitmaps/Toronto.bm
	fi
	#
	# Software license management code goes here
   	#
	if [ ! -s /var/adm/.license.sig ] ; then 
		if [  "$LLIC_DIR" = "en_US" -a \
		-f /usr/dt/appconfig/help/C/License.sdl -o \
		-f /usr/dt/appconfig/help/$LLIC_DIR/License.sdl ] ; then
		# This will invoke the dthelpview for licensing purposes
			loop=1
    			while [[ $loop = 1 ]]
    			do
      				loop=0
					XAPPLRESDIR=/usr/lib/X11/app-defaults \
		    		DTUSERHELPSEARCHPATH=/usr/dt/appconfig/help/$LLIC_DIR/%H.sdl:/usr/dt/appconfig/help/C/%H.sdl /usr/dt/bin/dthelpview \
			-xrm "*background: light gray" \
			-xrm "*topShadowColor: white" \
			-xrm "*executionPolicy: help_execute_all" \
			 -h License > /dev/null 2>&1 &
				DT_PID=$!
				echo "MWM_PID=$MWM_PID" >/tmp/.gui.dat
				echo "DT_PID=$DT_PID" >>/tmp/.gui.dat 
				wait $DT_PID 
				#
				# Check if license is signed
				#
				if [ "$?" = 0 -a ! -s /var/adm/.license.sig -a \
		   			$PTS -eq 1 ] ; then
					loop=1
				fi
			done
	   	else  # graphical license management not available
		#
		# See if we need to run smit based license management
		#
    		# loop until SMIT is exited normally 
			loop=1
    		while [[ $loop = 1 ]]
    		do
      			# assume that smit will exit normally and set
      			# loop to 0.  If Ctrl_C is pressed while in
      			# smit, the trap will set loop to 1 
      			loop=0
      			# Make them sign the license agreement
      			if [ -s "/usr/lib/nls/msg/$LLIC_DIR/softlic.cat" -a ! -s "/var/adm/.license.sig" ]; then
					trap ' ' 2
           			env TERM=$TERM SMIT_SHELL=n ODMDIR=/etc/objrepos aixterm -e smit -C -l /smit.log softlic
					RC=$?
					trap '-' 2
      				if [ "$RC" = 0 -o "$RC" = 2 -a \
					 ! -s "/var/adm/.license.sig" ]; then
						loop=1
        			fi
      			fi
    		done #end while
		fi
	fi  # If license not signed
#
# End of Software license agreement support
#
	GUI_MSG=0
	if [[ $LANG_DIR != "en_US" && $LANG_DIR != "C" ]]; then
	  	if [ ! -f "/usr/dt/appconfig/help/$LANG_DIR/Migrate.sdl" -a \
			! "$SOFTLIC_ONLY" ]; then
      			env TERM=$TERM ODMDIR=/etc/objrepos aixterm -e smit -C \
				 -l /smit.log lang_msg 
			if [ "$?" = 1 ] ; then
			# 
			# Response was to run the xlated SMIT install_assistant
			#
				GUI_MSG=1
			fi
		fi
	fi
	if [ $GUI_MSG = 0 ] ; then
	#
	# This will invoke the dthelpview in either the correct language or C
	#
		if [ ! "$SOFTLIC_ONLY" ] ; then
			XAPPLRESDIR=/usr/lib/X11/app-defaults \
		 	DTUSERHELPSEARCHPATH=/usr/dt/appconfig/help/%L/%H.sdl:/usr/dt/appconfig/help/C/%H.sdl /usr/dt/bin/dthelpview \
			-name IADthelpview \
			-xrm "*background: light gray" \
			-xrm "*topShadowColor: white" \
			-xrm "*executionPolicy: help_execute_all" \
		 	-h $1 > /dev/null 2>&1
		else
	  		> /tmp/.gui_ok
		fi
		# 0 - exited normally
		# 137 - exited through a kill -9 or by selecting "Exit, Tasks Complete"
       		if [ $? -eq 0 -o $? -eq 137 ]; then
	  		> /tmp/.gui_ok
		fi
        else
                # Bring up motif smit
                env TERM=$TERM ODMDIR=/etc/objrepos smit -l /smit.log assist
                > /tmp/.gui_ok
	fi # GUI_MSG = 0
fi
