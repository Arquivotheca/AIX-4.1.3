# @(#)92	1.9  src/bos/diag/boot/flattendb.sh, diagboot, bos41J, 9517B_all 4/24/95 15:14:04
#
# COMPONENT_NAME: diagboot
#
# FUNCTIONS: 
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
>/tmp/stanzas
>/tmp/stanza1

# Save all the PdDv, PdAt, and PdCn for things that need to be
# used by diagnostic application to define resource for testing.
# Objects like driver or ports are not configured by cfgmgr, therefore
# will not be in the CuDv and will have its Pre-defined objects
# cleanup by instdbcln.

x=`odmget -q"PdDvLn=tty/rs232/tty" CuDv`
if [ "x$x" = x ]
then
	odmget -q"uniquetype = tty/rs232/tty" PdDv >> /tmp/stanza2
	odmget -q"uniquetype = tty/rs232/tty" PdAt >> /tmp/stanza2
	odmget -q"uniquetype = tty/rs232/tty" PdCn >> /tmp/stanza2
fi
x=`odmget -q"PdDvLn=tty/rs422/tty" CuDv`
if [ "x$x" = x ]
then
	odmget -q"uniquetype = tty/rs422/tty" PdDv >> /tmp/stanza2
	odmget -q"uniquetype = tty/rs422/tty" PdAt >> /tmp/stanza2
	odmget -q"uniquetype = tty/rs422/tty" PdCn >> /tmp/stanza2
fi

odmget -q"uniquetype = printer/parallel/opp" PdDv >>/tmp/stanza2
odmget -q"uniquetype = printer/parallel/opp" PdAt >>/tmp/stanza2
odmget -q"uniquetype = printer/parallel/opp" PdCn >>/tmp/stanza2
odmget -q"uniquetype like driver/*/articdiag" PdDv >>/tmp/stanza2
odmget -q"uniquetype like driver/*/articdiag" PdAt >>/tmp/stanza2
odmget -q"uniquetype like driver/*/articdiag" PdCn >>/tmp/stanza2

# If otp and op are not in CuDv, save their Pre-defined objects
# so da can run on slc.

x=`odmget -q"name like otp*" CuDv`
if [ "x$x" = x ]
then
	odmget -q"uniquetype like */otp" PdDv >>/tmp/stanza2
	odmget -q"uniquetype like */otp" PdAt >>/tmp/stanza2
	odmget -q"uniquetype like */otp" PdCn >>/tmp/stanza2
fi
x=`odmget -q"name like op*" CuDv`
if [ "x$x" = x ]
then
	odmget -q"uniquetype like */op" PdDv >>/tmp/stanza2
	odmget -q"uniquetype like */op" PdAt >>/tmp/stanza2
	odmget -q"uniquetype like */op" PdCn >>/tmp/stanza2
fi

# If dials and lpfk are not yet configured saved their
# ODM stanzas so the SA can use it later on.

x=`odmget -q"name like lpfk*" CuDv`
if [ "x$x" = x ]
then
	odmget -q"uniquetype like */lpfk*" PdDv >>/tmp/stanza2
	odmget -q"uniquetype like */lpfk*" PdAt >>/tmp/stanza2
	odmget -q"uniquetype like */lpfk*" PdCn >>/tmp/stanza2
fi
x=`odmget -q"name like dials*" CuDv`
if [ "x$x" = x ]
then
	odmget -q"uniquetype like */dials" PdDv >>/tmp/stanza2
	odmget -q"uniquetype like */dials" PdAt >>/tmp/stanza2
	odmget -q"uniquetype like */dials" PdCn >>/tmp/stanza2
fi

# If mpaa is configured save the driver's ODM stanzas.

x=`odmget -q"name like mpaa*" CuDv`
if [ "x$x" != x ]
then
	odmget -q"uniquetype like driver/*/mpa" PdDv >>/tmp/stanza2
	odmget -q"uniquetype like driver/*/mpa" PdAt >>/tmp/stanza2
	odmget -q"uniquetype like driver/*/mpa" PdCn >>/tmp/stanza2
fi

# If msla is defined, saved pseudo driver's ODM stanzas.

x=`odmget -q"name like msla*" CuDv`
y=`odmget -q"name like cca*" CuDv`
if [ "x$x" != x -o "y$y" != y ]
then
	odmget -q"uniquetype like */gsw" PdDv >>/tmp/stanza2
	odmget -q"uniquetype like */gsw" PdAt >>/tmp/stanza2
	odmget -q"uniquetype like */gsw" PdCn >>/tmp/stanza2
	odmget -q"uniquetype like */hia" PdDv >>/tmp/stanza2
	odmget -q"uniquetype like */hia" PdAt >>/tmp/stanza2
	odmget -q"uniquetype like */hia" PdCn >>/tmp/stanza2
fi


/usr/lib/methods/instdbcln				>>$F1 2>&1

# Collapse object class

odmget PdDv > /etc/objrepos/pddv.add 2>&1
odmdelete -o PdDv > /dev/null 2>&1
odmadd /etc/objrepos/pddv.add
init -c "unlink /etc/objrepos/pddv.add" >/dev/null
odmget PdAt > /etc/objrepos/pdat.add 2>&1
odmdelete -o PdAt > /dev/null 2>&1
odmadd /etc/objrepos/pdat.add
init -c "unlink /etc/objrepos/pdat.add" >/dev/null
odmget PdCn > /etc/objrepos/pdcn.add 2>&1
odmdelete -o PdCn > /dev/null 2>&1
odmadd /etc/objrepos/pdcn.add
init -c "unlink /etc/objrepos/pdcn.add" >/dev/null
odmadd /tmp/stanzas					>>$F1 2>&1
odmadd /tmp/stanza1					>>$F1 2>&1
init -c "unlink /tmp/stanzas"				>>$F1 2>&1
init -c "unlink /tmp/stanza1"				>>$F1 2>&1
odmadd /tmp/stanza2					>>$F1 2>&1
init -c "unlink /tmp/stanza2"				>>$F1 2>&1
exit 0
