#!/bin/ksh
# @(#)41	1.3  src/bos/diag/da/disks/dgvars.sh, dadisks, bos411, 9428A410j 5/30/91 17:13:57
#
# COMPONENT_NAME: (CMDDIAG)  DISK DA LVM Query
#
# FUNCTIONS: Processes LV HLC output for disk DAs
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1991
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#                                                                   
###################################################################

findval ()
{
	awk '
	{
		#print "DEBUG nm= " nm
		n = index($0, nm);
		#print "DEBUG n= " n
		if( n >= 1 ) {
			n = n + length( nm );
			$0 = substr( $0, n+1);
			print $1
		}
	}
	' nm="$1"
}
if [ $# -ne 1 ]; then
	echo Usage: $0 pv-name >&2
	exit 7
fi

pv_name=$1
pvls_file=/tmp/d.lspv
vgls_file=/tmp/d.lsvg
trap "rm -f ${pvls_file} ${vgls_file}"		0
trap "rm -f ${pvls_file} ${vgls_file}; exit"	1 2 3 15

lspv ${pv_name} > ${pvls_file}
if [ $? -ne 0 ]; then
	exit 101
fi

VOLGROUP=`	findval "VOLUME GROUP:"		< ${pvls_file}`
NDESCPV=`	findval "VG DESCRIPTORS:"	< ${pvls_file}`

## echo $VOLGROUP $VGDSPV
lsvg ${VOLGROUP} > ${vgls_file}
if [ $? -ne 0 ]; then
	exit 102
fi

TOTALPVS=`	findval "TOTAL PVs:"		< ${vgls_file}`
ACTIVEPVS=`	findval "ACTIVE PVs:"		< ${vgls_file}`
NDESCVG=`	findval "VG DESCRIPTORS:"	< ${vgls_file}`

echo $VOLGROUP
echo $NDESCPV
echo $TOTALPVS
echo $ACTIVEPVS
echo $NDESCVG
exit 0
