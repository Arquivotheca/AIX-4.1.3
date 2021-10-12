#!/bin/ksh
#
# @(#)07        1.9  src/bos/usr/lib/assist/exit_assistant.sh, cmdassist, bos411, 9428A410j 5/14/94 14:41:25
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

# NAME: exit_assistant
#
# FUNCTION:
#    This program is executed when the user chooses the "Tasks Completed --
#    Exit to AIX Login"
#
# EXECUTION ENVIRONMENT:
#    Executes under install_assist environment.
#
# INPUT VALUES:
#    none
#
# OUTPUT VALUES:
#    none 
#
# RETURNS:
#    0  Successful completion.
#    1  Error.
#


# Set variables for message numbers.  These must match the values
# in the include file cmdassist_msg.h which is generated from the
# cmdassist.msg file.
#
MF_CMDASSIST=cmdassist.cat  # Message file
ASSIST_ERR_SET=1            # Message set number
ASSIST_PGSP_DEFAULT_E=18    # Error increasing pg sp to default

# Set name of file containing information from BOS install.
ASSIST_FILE="/var/adm/sw/__assistinfo"


#-----------------------------------------------------------------------
# If a PAGE_SPACE entry is not in the /var/adm/sw/__assistinfo file,
# this indicates that the page space was at the minimum size on entry
# to the install_assist program and that the user has not executed the
# Add/Show Paging Space dialog screen.  Therefore, set the page space
# size to the default size now.
#-----------------------------------------------------------------------

# ??? Add code to also check for size=32 before changing.

pgspace=`sed -n "s/.*PAGE_SPACE=[ ]*\([/0-9a-zA-Z]*\).*/\1/p"<$ASSIST_FILE`
if [[ -z $pgspace ]]
then

  # Calculate default page size.
  default_size=`/usr/lib/assist/calc_pgspace default_ps`
  if [[ $? != 0 ]]
  then
    # Display error message but do not exit.
    dspmsg -s $ASSIST_ERR_SET $MF_CMDASSIST $ASSIST_PGSP_DEFAULT_E \
'0851-012 install_assist:  An unexplained error occurred\
while attempting to increase the paging space to the\
default size.\n'
  else
    # Increase paging to default size.
    /usr/lib/assist/set_pgspace $default_size
    if [[ $? != 0 ]]
    then
      # Display error message but do not exit.
      dspmsg -s $ASSIST_ERR_SET $MF_CMDASSIST $ASSIST_PGSP_DEFAULT_E \
'0851-012 install_assist:  An unexplained error occurred\
while attempting to increase the paging space to the\
default size.\n'
    fi
  fi

fi


#-----------------------------------------------------------------------
# For any of the sm_*.add files that have not been removed, add them to
# the smit database now.  This puts all tasks on the Installation
# Assistant main menu so that if it is called through smit from the
# command line, the user will now see all tasks.
#-----------------------------------------------------------------------

if [[ -f /usr/lib/assist/sm_migrate.add ]]
then
  ODMDIR=/usr/lib/objrepos odmadd /usr/lib/assist/sm_migrate.add \
      >/dev/null 2>&1
  rm /usr/lib/assist/sm_migrate.add >/dev/null 2>&1
fi

if [[ -f /usr/lib/assist/sm_preserve.add ]]
then
  ODMDIR=/usr/lib/objrepos odmadd /usr/lib/assist/sm_preserve.add \
      >/dev/null 2>&1
  rm /usr/lib/assist/sm_preserve.add  >/dev/null 2>&1
fi

if [[ -f /usr/lib/assist/sm_preinstall.add ]]
then
  ODMDIR=/usr/lib/objrepos odmadd /usr/lib/assist/sm_preinstall.add \
      >/dev/null 2>&1
  rm /usr/lib/assist/sm_preinstall.add  >/dev/null 2>&1 
fi

if [[ -f /usr/lib/assist/sm_overwrite.add ]]
then
  ODMDIR=/usr/lib/objrepos odmadd /usr/lib/assist/sm_overwrite.add \
      >/dev/null 2>&1
  rm /usr/lib/assist/sm_overwrite.add  >/dev/null 2>&1
fi

if [[ -f /usr/lib/assist/sm_instdev.add ]]
then
  ODMDIR=/usr/lib/objrepos odmadd /usr/lib/assist/sm_instdev.add \
      >/dev/null 2>&1
  rm /usr/lib/assist/sm_instdev.add  >/dev/null 2>&1
fi


#-----------------------------------------------------------------------
# Remove the "Task Completed - Exit to AIX Login" item from the smit
# database.
#-----------------------------------------------------------------------

ODMDIR=/usr/lib/objrepos odmdelete -o sm_menu_opt \
    -q "id=assist and id_seq_num=120"  >/dev/null 2>&1


#-----------------------------------------------------------------------
# Change the line for INSTALL_METHOD in the _assistinfo file to
# say "IArestart" (install assistant restart).  This signals the
# gui install assistant to run with the help volume that has the
# "Exit Tasks" item removed.
#-----------------------------------------------------------------------

# Read from the /var/adm/sw/__assistinfo file a line at a time.
while read line
do
  # If a line is found that starts with "INSTALL_METHOD"
  if [[ $line = INSTALL_METHOD* ]]
  then
    # Write the help volume to use in the assistinfo file
    echo INSTALL_METHOD=IArestart >> /tmp/_assist$$
  else
    # Otherwise, write the line read to the temp file.
    echo $line >> /tmp/_assist$$
  fi
done < /var/adm/sw/__assistinfo

# Replace the old /var/adm/sw/__assistinfo with the temp file.
mv /tmp/_assist$$ /var/adm/sw/__assistinfo

#-----------------------------------------------------------------------
# Remove entry for install_assist from inittab.
#-----------------------------------------------------------------------

rmitab install_assist >/dev/null 2>&1

if [[ $SMIT = "a" ]]
then
   exit 0
else
   pid=`ps -ef | grep dthelpview | grep -v grep | sed 's/[ ][ ][ ]*/ /g' | cut -f3 -d" " `
   kill -9 $pid
fi

exit 0

