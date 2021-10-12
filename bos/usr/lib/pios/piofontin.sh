#!/bin/ksh
# @(#)93        1.4  src/bos/usr/lib/pios/piofontin.sh, cmdpios, bos411, 9428A410j 8/27/93 13:16:41
#
# COMPONENT_NAME: (CMDPIOS) read a Multilingual font diskette and create
#                           font files.
#
# FUNCTIONS: piofontin
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#  This shell script will read a Multilingual font diskette and copy
#  the files to the appropriate directory and name the files according
#  to the naming convention for font files.  Names are of the
#  form:
#       codepage.typeface.pitch*10.quality
#  Using pitch times 10 takes care of pitches like 17.5
#
#  This shell script expects a printer type and code page as input:
#
#       piofontin -t PrinterType -c CodePage [-d DisketteDevice ]
#
#  The diskette device is the device that the fonts will be read from.
#
#  This script must be run by root.


DEV=/dev/fd0
FONT_DIR=/usr/lib/lpd/pio/fonts
ERROR=0

# (PIOBASEDIR is for test use only)
if [ "$PIOBASEDIR." != "." ]
then FONT_DIR=$PIOBASEDIR/fonts
fi

# Parse the command line options
while getopts :t:c:d: FLAG 
    do  case $FLAG in
            t)  PRINTER_TYPE=$OPTARG;;
            c)  CP=$OPTARG;;
            d)  DEV=$OPTARG;;
            :)  ERROR=1;;
           \?)  ERROR=1;;
        esac
    done
   
# Display usage message if bad or missing arguments
if [ $ERROR -ne 0 -o $# -eq 0 -o "$PRINTER_TYPE." = "." -o "$CP." = "." ]
then
   dspmsg piobe.cat -s 4 11 \
    'Usage: piofontin -t PrinterType -c CodePage [ -d InputDevice  ] \
     Copies printer fonts from diskette to fonts directory\n'
   exit 1
fi

# Display message if user is not root
if [ -z "`id|fgrep uid=0\(root\)`" ]
then
   dspmsg piobe.cat -s 4 10 \
    'Must have root permissions to perform installation.\n'
   exit 1
fi

# Create the new directory if it does not exist
# The directory should have read permissions for everybody.
umask 002

if [ ! -d $FONT_DIR/$PRINTER_TYPE ]
then
	mkdir $FONT_DIR/$PRINTER_TYPE
	chown root.printq $FONT_DIR/$PRINTER_TYPE
fi

# Each case should do the following for each printer type:
#
# Use dosread to read the files and place them in the font directory.
# Rename the files to the appropriate file names.
# Change ownership and group and set permissions. 
#

ERROR=0
cd $FONT_DIR/$PRINTER_TYPE

case $PRINTER_TYPE in

ibm4201-3 | ibm4202-3 | ibm2380 | ibm2381)
	dosread -D$DEV dlff$CP.bin $CP.courier.120.0
	dosread -D$DEV dlq2$CP.bin $CP.courier.100.3
	dosread -D$DEV dpq2$CP.bin $CP.italic.100.3
	dosread -D$DEV dldp$CP.bin $CP.courier.100.1
	ln $CP.courier.100.1 $CP.courier.100.2
	ln $CP.courier.100.1 $CP.courier.120.1
	ln $CP.courier.100.1 $CP.courier.120.2
	ln $CP.courier.100.3 $CP.courier.120.3
	ln $CP.italic.100.3 $CP.italic.120.3
	chown root.printq $CP.*
	chmod a+r *
   break;;

ibm4207-2 | ibm4208-2 | ibm2390 | ibm2391)
	dosread -D$DEV dl4208a.bin $CP.prestige.120
	dosread -D$DEV dl4208b.bin $CP.courier.120
        ln $CP.prestige.120 $CP.courier.100 
	chown root.printq $CP.*
	chmod a+r $CP.*
   break;;

*)
	cd -; rmdir $FONT_DIR/$PRINTER_TYPE
	ERROR=1
   break;;

esac
   
# Display usage message if bad or missing arguments
if [ $ERROR -ne 0 ]
then
   dspmsg piobe.cat -s 4 11 \
    'Usage: piofontin -t PrinterType -c CodePage [ -d InputDevice  ] \
     Copies printer fonts from diskette to fonts directory\n'
   exit 1
fi

