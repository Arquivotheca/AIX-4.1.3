#!/usr/bin/ksh
# @(#)30        1.2  src/bos/usr/lpp/bosinst/printers/getprtpkgs.sh, cmdpios, bos411, 9428A410j 3/23/94 10:25:05
#
# COMPONENT_NAME: (CMDPIOS)
#
# FUNCTIONS: (Generates a list of printer packages to be installed)
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#	Script to prepare a list of printer packages to be installed.

set -o nounset
typeset -r	pgnm=${0##*/}
typeset -r	USAGE="Usage:\t$pgnm -o ODMdbpath -p PkgListFile"
typeset		flag
typeset -i	oflag=0
typeset -i	pflag=0
typeset		ptype			# printer type
typeset		odmdb			# ODM database path
typeset		pkglf			# package list file
typeset		pkgnm			# printer package name

# Process the flags
while getopts :o:p: flag
do
   case $flag in
      o)   oflag=1;odmdb="$OPTARG";;
      p)   pflag=1;pkglf="$OPTARG";;
      :)   print -u2 "$pgnm: -$OPTARG needs an argument\n$USAGE";exit 1;;
      ?)   print -u2 "$pgnm: -$OPTARG is an invalid flag\n$USAGE";exit 1;;
   esac
done
[[ $oflag = 1 && $pflag = 1 ]] || { print -u2 "$USAGE";exit 1;}
exec 9>>$pkglf || { print -u2 "Error in opening $pkglf";exit 2;}

# Check the list of printers configured and add their corresponding package
# names to the package list file.
for ptype in $( ODMDIR=$odmdb odmget -q 'PdDvLn like printer/*/*' CuDv|
		fgrep PdDvLn|sed -e 's/"//g'| ### cut -d= -f2|
		cut -d/ -f3 )
do
   case $ptype in
      #		4.1 printers
      ibm*) case $ptype in
         ibm2380 | ibm2380-2 | ibm2381 | ibm2381-2 | ibm2390 | ibm2390-2 | \
         ibm2391 | ibm2391-2 | ibm3812-2 | ibm3816 | ibm4019 | ibm4029 | \
         ibm4037 | ibm4039 | ibm4070 | ibm4072 | ibm4076 | ibm4079 | \
         ibm4201-2 | ibm4201-3 | ibm4202-2 | ibm4202-3 | ibm4207-2 | \
         ibm4208-2 | ibm4208-502 | ibm4212 | ibm4216-31 | ibm4216-510 | \
         ibm4224 | ibm4226 | ibm4234 | ibm5202 | ibm5204 | ibm5327 | \
         ibm5572 | ibm5573 | ibm5579 | ibm5584 | \
         ibm5587 | ibm5587H | ibm5589 | ibm6180 | ibm6182 | \
         ibm6184 | ibm6185-1 | ibm6185-2 | ibm6186 | ibm6252 | ibm6262 | \
         ibm7372)
					pkgnm=printers.$ptype.rte;;
         ibm5575|ibm5577|ibm5585)	pkgnm=printers.$ptype.com;;
         *)				continue;;	# skip unknown printers
            esac;;
      bull*) case $ptype in
         bull1021 | bull1070 | bull201 | bull411 | bull422 | bull451 | \
         bull454 | bull721 | bull922 | bull923 | bull924 | bull924N | \
         bull970 | bullpr88)
					pkgnm=printers.$ptype.rte;;
         *)				continue;;	# skip unknown printers
            esac;;

      #		3.2 printers
      [0-9]*) case $ptype in
         2380 | 2380-2 | 2381 | 2381-2 | 2390 | 2390-2 | 2391 | 2391-2 | \
         3812-2 | 3816 | 4019 | 4029 | 4037 | 4039 | 4070 | 4072 | 4076 | \
         4079 | 4201-2 | 4201-3 | 4202-2 | 4202-3 | 4207-2 | 4208-2 | 4212 | \
         4216-31 | 4224 | 4226 | 4234 | 5202 | 5204 | 6180 | 6182 | 6184 | \
         6185-1 | 6185-2 | 6186 | 6252 | 6262 | 7372)
 					pkgnm=printers.ibm$ptype.rte;;
         5575 | 5577 | 5585)		pkgnm=printers.ibm$ptype.com;;
         *)				continue;;	# skip unknown printers
            esac;;
      prt*) case $ptype in
         prt1021)			pkgnm=printers.bull1021.rte;;
         prt1072)			pkgnm=printers.bull1070.rte;;
         prt0201)			pkgnm=printers.bull201.rte;;
         prt0411)			pkgnm=printers.bull411.rte;;
         prt0422)			pkgnm=printers.bull422.rte;;
         prt4512|prt4513)		pkgnm=printers.bull451.rte;;
         prt4542|prt4543)		pkgnm=printers.bull454.rte;;
         prt0721)			pkgnm=printers.bull721.rte;;
         prt9222)			pkgnm=printers.bull922.rte;;
         prt9232)			pkgnm=printers.bull923.rte;;
         prt9242)			pkgnm=printers.bull924.rte;;
         prt9282)			pkgnm=printers.bull924N.rte;;
         prt9702)			pkgnm=printers.bull970.rte;;
         pr88)				pkgnm=printers.bullpr88.rte;;
         *)				continue;;	# skip unknown printers
            esac;;

      #		Misc printers
      canlbp | canlbp-A404PS | canlbp-B406G | \
      dp2000 | dp2665 | hplj-2 | hplj-3 | hplj-3si | hplj-4 | \
      oki801ps | p9012 | qms100 | ti2115)
					pkgnm=printers.$ptype.rte;;
      # skip unrecognized printers
      *)			continue;;
   esac
   egrep "^[ 	]*$pkgnm[ 	]*$" $pkglf >/dev/null || print -u9 - "$pkgnm"
done

exec 9>&-

exit 0
