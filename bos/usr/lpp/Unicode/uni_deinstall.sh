#!/bin/ksh
# @(#)21	1.6  src/bos/usr/lpp/Unicode/uni_deinstall.sh, cfgnls, bos411, 9428A410j 4/18/94 10:43:25
#
#   COMPONENT_NAME: CFGNLS
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

if [[ $(id|cut -c1-6) != "uid=0(" ]]
then
   echo "You must be root user to run this shell script"
   return
fi


#
# Is the toolkit already installed?
#
if [ -f /usr/lib/nls/loc/methods/uni.o -o -L /usr/lib/nls/loc/UNIVERSAL ]       
then
        continue
else 
	echo "Unicode toolkit doesn't seem to be installed"
	return
fi

#                                                
# Remove files added for Unicode toolkit            
#
      echo "Removing files needed for toolkit"
      rm -f /usr/lib/nls/loc/methods/uni.o
      rm -f /usr/lib/X11/UNIVERSAL/app-defaults/Aixterm
      rm -f /usr/lib/X11/UNIVERSAL/app-defaults/Mwm
      rm -f /usr/lib/X11/UNIVERSAL/app-defaults/XMdemos
      rm -f /usr/lib/X11/UNIVERSAL/app-defaults/Aixim
      rm -f /usr/lib/X11/UNIVERSAL/app-defaults/XTerm
      rm -f /usr/lib/X11/UNIVERSAL/app-defaults/Msmit
      cd /usr/lib/X11
      rmdir UNIVERSAL/app-defaults
      rmdir UNIVERSAL
      rm -f /usr/lib/lpd/pio/predef/ibm4019.uni
      rm -f /usr/lib/X11/nls/UTF-8
      rm -f /usr/lib/nls/loc/UNIVERSAL.imcfg
      rm -f /usr/lib/nls/csmap/UTF-8
#                                                
# Remove the links that were created for the toolkit    
#
   echo "Removing links needed for toolkit"
   rm -f /usr/lib/nls/loc/UNIVERSAL
   rm -f /usr/lib/nls/loc/UNIVERSAL.UTF-8
   rm -f /usr/lib/X11/UNIVERSAL.UTF-8
   rm -f /usr/lib/nls/loc/UNIVERSAL.im
   rm -f /usr/lib/nls/loc/UNIVERSAL.src
   rm -f /usr/lib/nls/loc/methods/uni.m
   rm -f /usr/lib/nls/loc/UNIVERSAL.UTF-8.im
   rm -f /usr/lib/nls/loc/UNIVERSAL.UTF-8.imcfg
   rm -f /usr/lib/nls/charmap/UTF-8
   rm -f /usr/lib/drivers/uc_utf
   rm -f /usr/lib/drivers/lc_utf
#
   echo "Deinstallation of Unicode toolkit complete"
