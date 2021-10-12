#!/bin/ksh
#
# @(#)18        1.36  src/bos/usr/lib/assist/assist_main.sh, cmdassist, bos41J, 9521B_all 5/25/95 13:20:27
#
#   COMPONENT_NAME:  cmdassist 
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


# NAME: install_assist
#
# FUNCTION:
#    The entry of the install_assist program in inittab causes the system to 
#    come up in the Installation Assistant on each reboot until the 
#    install_assist entry is removed from inittab.
#
# EXECUTION ENVIRONMENT:
#    Executes under install_assist environment.
#
# INPUT VALUES:
#    None.
#
# OUTPUT VALUES:
#    None.
#
# RETURNS:
#    0       Successful completion.
#    1       Error.
#


# Set variables for message numbers.  These must match the values
# in the include file cmdassist_msg.h which is generated from the
# cmdassist.msg file.
#
MF_CMDASSIST=cmdassist.cat  # Message file
ASSIST_ERR_SET=1            # Message set number
ASSIST_TTY1_I=1             # Set Terminal Type screen
ASSIST_TTY2_I=2             # Set Terminal Type screen (error version)
ASSIST_LANG_I=25            # GUI or Ascii language-based choice

TRUE=1
FALSE=0

# Set variable to name of file containing assistant information stored
# by BOS install.
ASSIST_FILE="/var/adm/sw/__assistinfo"

# Set variable to minimum page space size created by BOS install.
PGSP_MIN=32


#
# ---------------- Install Assistant (IA) Functions ----------------------
#

# ------------------------------------------------------------------
# add_odm():
# Add all the ODM stanzas to ODM depending on what kind
# of installation is being performed.
# ------------------------------------------------------------------

add_odm() {
case $install_method in
    migrate):
      if [[ -f /usr/lib/assist/sm_migrate.add ]]
      then
        ODMDIR=/usr/lib/objrepos odmadd /usr/lib/assist/sm_migrate.add
        rm /usr/lib/assist/sm_migrate.add
      fi
      ;;
    preserve):
      if [[ -f /usr/lib/assist/sm_migrate.add ]]
      then
        ODMDIR=/usr/lib/objrepos odmadd /usr/lib/assist/sm_migrate.add
        rm /usr/lib/assist/sm_migrate.add
      fi
      if [[ -f /usr/lib/assist/sm_preserve.add ]]
      then
        ODMDIR=/usr/lib/objrepos odmadd /usr/lib/assist/sm_preserve.add
        rm /usr/lib/assist/sm_preserve.add
      fi
      ;;
    preinstall):
      if [[ -f /usr/lib/assist/sm_migrate.add ]]
      then
        ODMDIR=/usr/lib/objrepos odmadd /usr/lib/assist/sm_migrate.add
        rm /usr/lib/assist/sm_migrate.add
      fi
      if [[ -f /usr/lib/assist/sm_preserve.add ]]
      then
        ODMDIR=/usr/lib/objrepos odmadd /usr/lib/assist/sm_preserve.add
        rm /usr/lib/assist/sm_preserve.add
      fi
      if [[ -f /usr/lib/assist/sm_preinstall.add ]]
      then
        ODMDIR=/usr/lib/objrepos odmadd /usr/lib/assist/sm_preinstall.add
        rm /usr/lib/assist/sm_preinstall.add
      fi
      ;;
    overwrite|IArestart):
      if [[ -f /usr/lib/assist/sm_migrate.add ]]
      then
        ODMDIR=/usr/lib/objrepos odmadd /usr/lib/assist/sm_migrate.add
        rm /usr/lib/assist/sm_migrate.add
      fi
      if [[ -f /usr/lib/assist/sm_preserve.add ]]
      then
        ODMDIR=/usr/lib/objrepos odmadd /usr/lib/assist/sm_preserve.add
        rm /usr/lib/assist/sm_preserve.add
      fi
    if [[ -f /usr/lib/assist/sm_preinstall.add ]]
    then
      ODMDIR=/usr/lib/objrepos odmadd /usr/lib/assist/sm_preinstall.add
      rm /usr/lib/assist/sm_preinstall.add
    fi
    if [[ -f /usr/lib/assist/sm_overwrite.add ]]
    then
      ODMDIR=/usr/lib/objrepos odmadd /usr/lib/assist/sm_overwrite.add
      rm /usr/lib/assist/sm_overwrite.add
    fi
    ;;
  esac

  # Check the machine type.  If necessary, add isa.add
  machine=`bootinfo -T`
  if [[ $machine = rspc ]]; then
    if [[ -f /usr/lib/assist/sm_isa.add ]]; then
     ODMDIR=/usr/lib/objrepos odmadd /usr/lib/assist/sm_isa.add
     rm /usr/lib/assist/sm_isa.add
    fi
  fi

  # Install the install_device stanzas so that
  # this option is always available in the install assistant.
  #
  if [[ -f /usr/lib/assist/sm_instdev.add ]]
  then
    ODMDIR=/usr/lib/objrepos odmadd /usr/lib/assist/sm_instdev.add
    rm /usr/lib/assist/sm_instdev.add
  fi
}

do_page_space() 
{
#------------------------------------------------------------------------
# Get the total size of the page space for the system.
#------------------------------------------------------------------------

pgspace=`lsps -s |
  awk ' NR>1 { print substr($1,1,length($1)-2) }'`

#------------------------------------------------------------------------
# If the page space size is greater than the minimum size created by BOS
# install, then write the page space size to the /var/adm/sw/__assistinfo
# file in an entry of the form "PAGE_SPACE=value" where value is the page
# space size in megabytes.
#------------------------------------------------------------------------

if ((pgspace > $PGSP_MIN))
then

  # Set flag for page space line found to false.
  pg_sp=0

  # Read from the /var/adm/sw/__assistinfo file a line at a time.
  while read line
  do
    # If a line is found that starts with "PAGE_SPACE"
    if [[ $line = PAGE_SPACE* ]]
    then
      # Write the new page space size information to a temp file.
      echo PAGE_SPACE=$pgspace >> /tmp/_assist$$
      # Set flag for page space line found to true.
      pg_sp=1
    else
      # Otherwise, write the line read to the temp file.
      echo $line >> /tmp/_assist$$
    fi
  done < /var/adm/sw/__assistinfo

  # If a line for page space was not found in /var/adm/sw/__assistinfo
  if [[ $pg_sp = 0 ]]
  then
    # Write the new page space size information to the temp file.
    echo PAGE_SPACE=$pgspace >> /tmp/_assist$$
  fi

  # Replace the old /var/adm/sw/__assistinfo with the temp file.
  mv /tmp/_assist$$ /var/adm/sw/__assistinfo

fi #end if page space
} # end do_page_space




#
# -------------------- IA Ascii Functions --------------------
#


#------------------------------------------------
# term_msg:
# Display initial message for terminal selection
#------------------------------------------------
term_msg() {
   awk '{for (i=1; i<24; i++) printf("\n"); exit}' < /usr/lib/assist/assist_main
   dspmsg -s $ASSIST_ERR_SET $MF_CMDASSIST $ASSIST_TTY1_I \
'                           Set Terminal Type \n\
The terminal is not properly initialized.  Please enter a terminal type\
and press Enter.  Some terminal types are not supported in\
non-English languages.\n\n\
     ibm3101          tvi912          vt330\
     ibm3151          tvi920          vt340\
     ibm3161          tvi925          wyse30\
     ibm3162          tvi950          wyse50\
     ibm3163          vs100           wyse60\
     ibm3164          vt100           wyse100\
     ibmpc            vt320           wyse350\
     lft              sun\n\n\
                      +-----------------------Messages------------------------\
                      | If the next screen is unreadable, press Break (Ctrl-c)\
    88  Help ?        | to return to this screen.\
    99  Exit          | \
                      | \n\
>>> Choice []: '
}


#------------------------------------------------
# error_msg:
# Display error message for terminal selection
#------------------------------------------------
error_msg() {
   awk '{for (i=1; i<24; i++) printf("\n"); exit}' < /usr/lib/assist/assist_main
    dspmsg -s $ASSIST_ERR_SET $MF_CMDASSIST $ASSIST_TTY2_I \
'                           Set Terminal Type \n\
The terminal is not properly initialized.  Please enter a terminal type\
and press Enter.  Some terminal types are not supported in\
non-English languages.\n\n\
     ibm3101          tvi912          vt330\
     ibm3151          tvi920          vt340\
     ibm3161          tvi925          wyse30\
     ibm3162          tvi950          wyse50\
     ibm3163          vs100           wyse60\
     ibm3164          vt100           wyse100\
     ibmpc            vt320           wyse350\
     lft              sun\n\n\
                      +-----------------------Messages------------------------\
                      | ERROR:  Undefined terminal type.  Please try again.\
    88  Help ?        | \
    99  Exit          | If the next screen is unreadable, press Break (Ctrl-c)\
                      | to return to this screen.\n\
>>> Choice []: '
}


#------------------------------------------------
# help_msg:
# Display help message for terminal selection
# display until 99 is entered
#------------------------------------------------
help_msg() {
awk '{for (i=1; i<24; i++) printf("\n"); exit}' < /usr/lib/assist/assist_main
    dspmsg -s $ASSIST_ERR_SET $MF_CMDASSIST $ASSIST_TTY2_I \
'                    Help for Set Terminal Type \n\
The terminal type must be correctly set in order for the System\
Management Interface Tool (SMIT) to display readable menus.\n\
If you have a graphics display, then enter \"lft\" for your terminal\
type.\n\
There may be other terminal types in addition to those listed that\
are valid for your system (if the terminal information data is\
installed).  If you enter a terminal type that is not valid, you will\
receive an error, and you can try again.\n\
If you enter a valid terminal type but one that does not correctly\
match your terminal, you may find that the next screen is unreadable.\
If this happens, then press Break to return to the Set Terminal Type\
screen.  For most terminal types, Break is the Ctrl-c key sequence\
(that is, while holding down the Ctrl key, also press the c key).\n\n\n\
>>> 99  Previous Menu \n\
>>> Choice [99]: '
read answer
term_msg
}


# ------------------------------------------------------------
# get_term():
# Enter a while loop.  First get the user input. Then check for a valid
# code - either a valid terminal type, a help message code (88),
# or an exit code (99).  If an invalid code is entered, then
# the error screen will be printed and the user will be prompted
# for another entry.
# Once a valid terminal type has been entered, both the while
# loop and the get_term function will be exited.
# ------------------------------------------------------------
get_term() {
   invalid_term=1
   while [[ $invalid_term = 1 ]]; do
     # Get input from user
     #
     read answer
     if [[ -n $answer ]]
     then
       TERM="$answer"
       # Check if terminal type entered is valid.
       # Specifically exclude type dumb
       prefix=`echo "$TERM" | sed "s/\(.\).*/\1/"`
       file="/usr/share/lib/terminfo/$prefix/$TERM"
       if [ -s "$file" -a $TERM != "dumb" ]
       then
         # A valid term type was entered.  We now can exit the
         # get_term loop procedure.
         invalid_term=0
       else
          # terminal type is not valid.  Check for help message or
          # exit code.  If neither is true, then ask the user
          # for another entry with the error screen.  
          case $answer in
          88):
            # Enter the help message in the term_help 
            # function. (see end of this file)
            help_msg
            ;;
          99):
            # exit
            exit 1
            ;;
           *):
            # display error message
            error_msg
            ;;
           esac
     fi #end if-else terminal type check
    else  #no terminal entered
     error_msg
    fi
   done  #end while terminal type invalid
} # end of get_term



#
# ----------------------------- Main -----------------------------
#

#------------------------------------------------------------------------
# Check to see if assist_main was called from the app_manager
#------------------------------------------------------------------------
if [[ $1 = "-appman" ]];then
  CALLED_FROM_APPMAN=1
else
  CALLED_FROM_APPMAN=0
fi

#------------------------------------------------------------------------
# Get the MESSAGES= value out of the bosinst.data file to determine what
# message environment the customer installed the system in. This will
# determine what Electronic Licensing support to look for and will 
# determine what graphical install_assistant environments to look for.
#
#
#------------------------------------------------------------------------
if [ -s "/var/adm/ras/bosinst.data" ] ; then
	LANG_INFO=`/usr/bin/grep -E "^(	| )*MESSAGES" /var/adm/ras/bosinst.data | /usr/bin/cut -d= -f2`
	typeset LANG_INFO
	if [ "${#LANG_INFO}" -lt 5 ] ; then
		LANG_DIR=$LANG
	else
		set $LANG_INFO
		LANG_DIR=$1
	fi
else
	LANG_DIR=$LANG
fi
#
# Special exception for Canadian French. If the Cultural Convention is 
# Canadian, then the message catalogs and sdl files will come from
# the fr_CA directories for Electronic License Support 
#
if [ "$LANG" = "fr_CA" ] ; then
	LLIC_DIR=$LANG
else
	LLIC_DIR=$LANG_DIR
fi
#
# Make sure we can access electronic license catalogs
# (If we can't then set language license directory (LLIC_DIR)  invalid
#
if [ "$LLIC_DIR" = "C" -a ! -s "/usr/lib/nls/msg/C/softlic.cat" ] ; then
	LLIC_DIR=C@LFT
else
	if [ "$LLIC_DIR" != "C" ] ; then
		/usr/bin/dspmsg -s 1 softlic.cat 12 >/dev/null 2>&1
		if [ "$?" -ne 0 ] ; then
			LLIC_DIR=C@LFT
		fi
	fi
fi
export LANG_DIR LLIC_DIR

#------------------------------------------------------------------------
# First try to launch the gui install assistant.  If that fails,
# Then run the ascii version.
#------------------------------------------------------------------------


#------------------------------------------------------------------------
# Check to see if the desktop is installed.  If the desktop is 
# installed during the install assistant session, the desktop
# will have to be launched before the install assistant is exited. 
#------------------------------------------------------------------------
`lslpp -l X11.Dt.rte > /dev/null 2>&1 `
if [[ $? = 0 ]]
then
  DT_INSTALLED=1
else
  DT_INSTALLED=0
fi

#------------------------------------------------------------------------
# If the /var/adm/sw/__assistinfo file does not exist, then set install
# method to preinstall and install device to null.  If it does exist,
# then set install method based on INSTALL_METHOD entry in the file
# and set install device based on INSTALL_DEVICE entry.
#------------------------------------------------------------------------

if [[ ! -s $ASSIST_FILE ]]
then
  install_method="preinstall"
  install_device=""
  echo INSTALL_METHOD=$install_method >> $ASSIST_FILE
  echo INSTALL_DEVICE=$install_device >> $ASSIST_FILE
else
  install_method=`LC_ALL=C sed -n "s?.*INSTALL_METHOD=[ ]*\([/0-9a-zA-Z]*\).*?\1?p"<$ASSIST_FILE`
  if [[ -z $install_method ]]
  then
    install_method="overwrite"
  fi
  install_device=`LC_ALL=C sed -n "s?.*INSTALL_DEVICE=[ ]*\([/0-9a-zA-Z]*\).*?\1?p"<$ASSIST_FILE`
  if [[ -z $install_device ]]
  then
    install_device=""
  fi
fi

#------------------------------------------------------------------------
# Add the odm stanzas for the install assistant
#------------------------------------------------------------------------
add_odm

#------------------------------------------------------------------------
# If we are running on a PTS, it means we are not running from
# inittab.  We are probably running from the command line, and the
# install assistant has been run before.  In this case, we don't need
# to re-calculate the paging space.
#------------------------------------------------------------------------
`tty | grep pts | grep -v grep > /dev/null`
IS_IT_PTS=$?

#If called from appman, then run like it's a command-line call
if [[ $CALLED_FROM_APPMAN = 1 ]]; then
  IS_IT_PTS=0
fi

if [ $IS_IT_PTS != 0 ]
then
  # Calculate the paging space
  do_page_space
else
  # if from a pts don't try to start desktop
  DT_INSTALLED=1
fi


#------------------------------------------------------------------------
# Check to see if the console is an LFT.
#------------------------------------------------------------------------
if [ `lscons` = '/dev/lft0' ]; then
  CONS_IS_LFT=$TRUE
else
  CONS_IS_LFT=$FALSE
fi


#------------------------------------------------------------------------
# If the terminal is an LFT and if X11.vsm.rte (GUI system management) is
# installed, then call the GUI version of the Installation Assistant
# (based on the method of BOS installation).
# Also if it is a PTS, then either X is already up and running or we are
# logged in remotely, and we should try to run the gui version first.
#------------------------------------------------------------------------

launch_smit=$TRUE
rm -f /tmp/.gui_ok 2>/dev/null
rm -f /tmp/.gui.dat 2>/dev/null
IS_IT_DTBOOT=0
export CURSES="-C"
ps -ef | grep "dtlogin" | grep -v grep >/dev/null
if [ "$?" = 0 -a -s "/.bootsequence" -a $CONS_IS_LFT = $TRUE ] 
then
	export eval `/usr/bin/grep "XAUTHORITY" /.bootsequence`
	export DISPLAY=:0
	IS_IT_DTBOOT=1   
	CURSES=" "
	TERM=lft
fi
IS_IT_LFT=`/usr/lpp/bosinst/bi_io -l`
if [ $IS_IT_LFT = 1 -a $IS_IT_PTS != 0 ] ; then
	CURSES=" "
	TERM=lft
fi
if [[ $IS_IT_LFT = 1 || $IS_IT_PTS = 0 ]]
then
  SAVE_TERM=$TERM
  TERM=lft

  # if from command line, we don't want to restart X
  if [[ $IS_IT_PTS = 0  || $IS_IT_DTBOOT = 1 ]]
  then
    COMMAND=
  else
    COMMAND=xinit
  fi

  ` lslpp -l X11.vsm.rte > /dev/null 2>&1 	&&
    lslpp -l X11.Dt.helprun > /dev/null 2>&1 	&&
    lslpp -l X11.apps.aixterm > /dev/null 2>&1 	&&
    lslpp -l X11.apps.msmit > /dev/null 2>&1 `
  if [ $? -eq 0 ]
  then
      case $install_method in
        overwrite):
  	  if [ -f "/usr/dt/appconfig/help/$LANG_DIR/Complete.sdl" -o \
	     -f "/usr/dt/appconfig/help/C/Complete.sdl" ]
	  then
             	$COMMAND /usr/lib/assist/assist.xinitrc Complete -- -v
	  fi
          ;;
        preinstall):
	  if [ -f "/usr/dt/appconfig/help/$LANG_DIR/Preinstall.sdl" -o \
	     -f "/usr/dt/appconfig/help/C/Preinstall.sdl" ]
	  then
        	$COMMAND /usr/lib/assist/assist.xinitrc Preinstall -- -v
	  fi
          ;;
        preserve):
  	  if [ -f "/usr/dt/appconfig/help/$LANG_DIR/Preserve.sdl" -o \
	     -f "/usr/dt/appconfig/help/C/Preserve.sdl" ]
	  then
        	$COMMAND /usr/lib/assist/assist.xinitrc Preserve -- -v
	  fi
          ;;
        migrate):
	  if [ -f "/usr/dt/appconfig/help/$LANG_DIR/Migrate.sdl" -o \
	     -f "/usr/dt/appconfig/help/C/Migrate.sdl" ]
	  then
        	$COMMAND /usr/lib/assist/assist.xinitrc Migrate -- -v
	  fi
          ;;
        IArestart):
	  if [ -f "/usr/dt/appconfig/help/$LANG_DIR/IArestart.sdl" -o \
	     -f "/usr/dt/appconfig/help/C/IArestart.sdl" ]
	  then
        	$COMMAND /usr/lib/assist/assist.xinitrc IArestart -- -v
	  fi
          ;;
      esac
  fi #check for all needed software
	if [ -s /tmp/.gui.dat ] ; then
		eval `grep "MWM_PID" /tmp/.gui.dat`
      		rm -f /tmp/.gui.dat
	fi
    if [ -f /tmp/.gui_ok ]
    then
      	# If successful launch of GUI, set launch smit flag to false.
      	launch_smit=$FALSE
      	rm -f /tmp/.gui_ok
    else			# if dthelpview failed, reset TERM.  This is
      TERM=$SAVE_TERM		# for remote login from an ascii head.
	
    fi # end if [ -f /tmp/.gui_ok ]
fi

# ---------------------- Ascii SMIT loop starts here ---------------------------

if [[ $launch_smit = $TRUE ]]; then
    
    # loop until SMIT is exited normally or the 
    # user enters 99 at the terminal selection screen
    loop=1
    while [[ $loop = 1 ]]
    do
      # Get a valid terminal type if necessary. 
      if [[ $TERM = "" || $TERM = dumb ]] ; then
	 # Set up the trap in case the user presses ctrl-c while
         # in smit.  This puts the user back into the terminal
         # selection screen by resetting loop to 1.
         # 
         trap 'TERM="dumb";loop=1' 2
         term_msg
         get_term
      fi 
      # assume that smit will exit normally and set
      # loop to 0.  If Ctrl_C is pressed while in
      # smit, the trap will set loop to 1 (if a terminal
      # type had to be selected).
      loop=0
      # Make them sign the license agreement
      if [ -s "/usr/lib/nls/msg/$LLIC_DIR/softlic.cat" -a ! -s "/var/adm/.license.sig" ]; then
         trap '' 2
         env TERM=$TERM SMIT_SHELL=n ODMDIR=/etc/objrepos smit $CURSES -l /smit.log softlic
	 RC=$? 
	 trap '-' 2
      	if [ "$RC" = 0 -o "$RC" = 2 -a ! -s "/var/adm/.license.sig" ]; then
		loop=1
        else
          # launch SMIT if license is signed
	  if [ ! "$SOFTLIC_ONLY" ] ; then
          	env TERM=$TERM ODMDIR=/etc/objrepos smit $CURSES -l /smit.log assist
	  fi	
        fi
      else
          # launch SMIT if no license is required or already signed 
	  if [ ! "$SOFTLIC_ONLY" ] ; then
          	env TERM=$TERM ODMDIR=/etc/objrepos smit $CURSES -l /smit.log assist
	  fi	
      fi
    done #end while
fi # end (launch SMIT is true)

#------------------------------------------------------------------------
# Call routine to check install device and unmount if a mounted CD.
#------------------------------------------------------------------------
if [ $MWM_PID ] ; then
	kill -9 $MWM_PID >/dev/null 2>&1 
fi
#------------------------------------------------------------------------
# Check for temporary software license request                       
# If "0000" is in the temporary license file on next boot, fbcheck will
# invoke install assist to get the license.
#------------------------------------------------------------------------
if [ -s "/var/adm/.license.sig" ] ; then
	LIC_NAME=`cat /var/adm/.license.sig`
	if [ "$LIC_NAME" = "0000" ] ; then
		/usr/bin/cp /var/adm/.license.sig /.license.sig.tmp
	else
		/usr/bin/rm /.license.sig.tmp >/dev/null 2>&1
	fi
fi
if [ -n "$install_device" ]; then
  /usr/lib/assist/unmount_cd $install_device
fi

# Check to see if Desktop was installed.  If it was then invoke it.
# This is so if a user installs Dt inside the assistant, it will
# come up when they exit.  Init will not reread the inittab once
# it is invoked.  Without this, the user would have to reboot to
# get Dt.  We do it here and not in the gui loop just in case
# the user hits Cntrl-Alt-Backspace.

`lslpp -l X11.Dt.rte > /dev/null 2>&1 `
if [[ $? = 0 && \
      $DT_INSTALLED = 0 && \
      $IS_IT_PTS = 1 ]]
then
 /etc/rc.dt
fi
