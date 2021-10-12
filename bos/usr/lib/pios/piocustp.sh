#!/bin/ksh
# @(#)37	1.11  src/bos/usr/lib/pios/piocustp.sh, cmdpios, bos41J, 9523B_all 6/7/95 16:35:33
#
# COMPONENT_NAME: (cmdpios) Printer Backend
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1992, 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
######
# Add attributes for Terminal Connected Printer Queues to virtual printer
# custom colon file.
######
# cmdline:
# piocustp -i colon_filename
######
# Called from mkvirprt
######


# Get command line option (colon filename)
#################
while getopts i: opt ; do
	case $opt in
	i)	CFILE=$OPTARG;;
	esac
done

# Get queue name
#############
qname=$(basename $CFILE | cut -f1 -d:)

# Get queue device name
#############
qdname=$(basename $CFILE | cut -f2 -d:)

# Get tty device name
######
TTYDEV=/dev/$(grep ":mn:" $CFILE | cut -f5 -d:)

# Get adapter hardware discipline from the ODM.
###########

ADAPTER=$(
   MAJOR=`ls -l $TTYDEV | awk -F, '{print $1}' | awk '{print $5}'`
   odmget -q "resource=ddins AND value2=$MAJOR" CuDvDr | grep value1 | cut -f2 -d\"
)
# Kludge to match the value returned from I_LIST call in pioout
if [[ "$ADAPTER" = "isa/rsdd_rspc" || "$ADAPTER" = "pcmcia/pcrsdd" ]]
then
	ADAPTER=stydd
fi
ADAPTER=${ADAPTER%%dd}

# Attribute default values
#################
rs_bufsiz=10
rs_delay=200000
lion_priority=30
cxma_maxcps=100
cxma_maxchar=50
cxma_bufsiz=100

# Get printer type
#################################################################
PRINTER=$(grep ":mt:" $CFILE | cut -f5 -d:)

# Set virtual printer attribute
###############################
case "$PRINTER" in
	ibm4201-2|ibm4201-3)
	# IBM 4201 
		cxma_maxcps=270
		;;
	ibm4207|ibm4208-2)
	# IBM 4207 & 4208
		cxma_maxcps=270
		;;
	ibm4019)
	# IBM 4019
		;;
	hplj-2)
	# HPLJ II
		;;
	ibm4202-2|ibm4202-3)
	# IBM 4202
		cxma_maxcps=270
		;;
	ibm5204)
	# IBM 5204
		cxma_maxcps=350
		;;
	ibm2380|ibm2381)
	# IBM 2380 & 2381 
		cxma_maxcps=320
		;;
	ibm2390|ibm2391)
	# IBM 2390 & 2391
		cxma_maxcps=200
		;;
	ibm4029)
	# IBM 4029
		;;
	hplj-3)
	# HPLJ III
		;;
esac

# Update virtual printer custom file
#############################
echo ":599:__TAP::" >> $CFILE
echo ":613:y0::$ADAPTER" >> $CFILE
case "$ADAPTER" in
rs)
	echo ":600:y1::$rs_bufsiz\n:601:y2::$rs_delay" >> $CFILE ;;
cxia)
	echo ":600:y1::$rs_bufsiz\n:601:y2::$rs_delay" >> $CFILE ;;
sty)
	echo ":600:y1::$rs_bufsiz\n:601:y2::$rs_delay" >> $CFILE ;;
lion)
	echo ":602:y1::$lion_priority" >> $CFILE ;;
cxma)
	echo ":603:y1::$cxma_maxcps\n:604:y2::$cxma_maxchar\n:605:y3::$cxma_bufsiz" >> $CFILE ;;
*)      print "Error: piocustp: <$ADAPTER> not a valid adapter!" 1>&2
	exit 1;;
esac
exit 0
