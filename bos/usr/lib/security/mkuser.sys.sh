# @(#)18	1.9  src/bos/usr/lib/security/mkuser.sys.sh, cmdsuser, bos411, 9428A410j 3/3/93 16:31:24
#
#   COMPONENT_NAME: CMDSUSER
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# Check the number of arguments first
#
if [ $# -lt 4 ] 
then
	exit 1
fi

#
# Create the named directory if it does not already exist
# and set the file ownership and permission
#
if [ ! -d $1 ]
then
	mkdir $1
	chgrp $3 $1
	chown $2 $1
fi

#
# Copy the user's default .profile if it does not already
# exist and change the file ownership, etc.
#
if [ `basename $4` != "csh" ] && [ ! -f $1/.profile ]
then
	cp /etc/security/.profile $1/.profile
	chmod u+rwx,go-w $1/.profile
	chgrp $3 $1/.profile
	chown $2 $1/.profile

else
   if [ `basename $4` = "csh" ] && [ ! -f $1/.login ] 
   then
	echo "#!/bin/csh" > "$1"/.login
	echo "set path = ( /usr/bin /etc /usr/sbin /usr/ucb \$HOME/bin /usr/bin/X11 /sbin . )" >> "$1"/.login
	echo "setenv MAIL \"/var/spool/mail/\$LOGNAME\"" >> "$1"/.login
	echo "setenv MAILMSG \"[YOU HAVE NEW MAIL]\"" >> "$1"/.login
	echo "if ( -f \"\$MAIL\" && ! -z \"\$MAIL\") then" >> "$1"/.login
        echo "	echo \"\$MAILMSG\"" >> "$1"/.login
	echo "endif" >> "$1"/.login
	chmod u+rwx,go-w $1/.login
	chgrp $3 $1/.login
	chown $2 $1/.login
   fi
fi
