#!/bin/ksh
# @(#)20	1.6  src/bos/usr/lpp/Unicode/uni_install.sh, cfgnls, bos411, 9428A410j 4/18/94 10:43:04
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

UNIDIR=/usr/lpp/Unicode

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
	echo "Unicode toolkit seems to be already installed"
	return
fi
#                                                
# Copy new files into the appropriate directory     
#
   echo "Copying files needed for toolkit"
   cp ${UNIDIR}/methods/uni.o /usr/lib/nls/loc/methods/uni.o
#                                                
# Make sure that the structure UNIVERSAL/app-defaults exists
#
   cd /usr/lib/X11
   if [ -d UNIVERSAL ]
   then continue
   else
      mkdir UNIVERSAL
   fi
#
   cd UNIVERSAL
   if [ -d app-defaults ] 
   then continue
   else
      mkdir app-defaults
   fi
   cd
#
   cp ${UNIDIR}/X11/app-defaults/Aixterm \
      /usr/lib/X11/UNIVERSAL/app-defaults/Aixterm
   cp ${UNIDIR}/X11/app-defaults/Mwm \
      /usr/lib/X11/UNIVERSAL/app-defaults/Mwm
   cp ${UNIDIR}/X11/app-defaults/XMdemos \
      /usr/lib/X11/UNIVERSAL/app-defaults/XMdemos
   cp ${UNIDIR}/X11/app-defaults/XTerm \
      /usr/lib/X11/UNIVERSAL/app-defaults/XTerm
   cp ${UNIDIR}/X11/app-defaults/Msmit \
      /usr/lib/X11/UNIVERSAL/app-defaults/Msmit
   cp ${UNIDIR}/X11/app-defaults/Aixim \
      /usr/lib/X11/UNIVERSAL/app-defaults/Aixim
#
   cp ${UNIDIR}/lpd/pio/predef/ibm4019.uni \
      /usr/lib/lpd/pio/predef/ibm4019.uni
   cp ${UNIDIR}/X11/nls/UTF-8 /usr/lib/X11/nls/UTF-8
   cp ${UNIDIR}/loc/UNIVERSAL.imcfg /usr/lib/nls/loc/UNIVERSAL.imcfg
   cp ${UNIDIR}/tty/UTF-8 /usr/lib/nls/csmap/UTF-8

#
# Create the appropriate links                          
#
   echo "Creating links needed for toolkit"
   ln -sf ${UNIDIR}/loc/UNIVERSAL /usr/lib/nls/loc/UNIVERSAL
   ln -sf /usr/lib/nls/loc/UNIVERSAL /usr/lib/nls/loc/UNIVERSAL.UTF-8
   ln -sf /usr/lib/X11/UNIVERSAL /usr/lib/X11/UNIVERSAL.UTF-8
   ln -sf ${UNIDIR}/loc/UNIVERSAL.im /usr/lib/nls/loc/UNIVERSAL.im
   ln -sf ${UNIDIR}/loc/UNIVERSAL.src /usr/lib/nls/loc/UNIVERSAL.src
   ln -sf ${UNIDIR}/loc/UNIVERSAL.m /usr/lib/nls/loc/methods/uni.m
   ln -sf /usr/lib/nls/loc/UNIVERSAL.im /usr/lib/nls/loc/UNIVERSAL.UTF-8.im
   ln -sf /usr/lib/nls/loc/UNIVERSAL.imcfg \
          /usr/lib/nls/loc/UNIVERSAL.UTF-8.imcfg
   ln -sf ${UNIDIR}/charmap/UNIVERSAL.cm \
          /usr/lib/nls/charmap/UTF-8
   ln -sf ${UNIDIR}/tty/uc_utf /usr/lib/drivers/uc_utf
   ln -sf ${UNIDIR}/tty/lc_utf /usr/lib/drivers/lc_utf
#                                                
echo "Installation of Unicode toolkit complete"
