#! /bin/sh
# @(#)73        1.1  src/bldenv/sbtools/mksb/cvtsbrc.sh, bldprocess, bos412, GOLDA411a 4/29/93 12:26:25
#
# Copyright (c) 1990, 1991, 1992  
# Open Software Foundation, Inc. 
#  
# Permission is hereby granted to use, copy, modify and freely distribute 
# the software in this file and its documentation for any purpose without 
# fee, provided that the above copyright notice appears in all copies and 
# that both the copyright notice and this permission notice appear in 
# supporting documentation.  Further, provided that the name of Open 
# Software Foundation, Inc. ("OSF") not be used in advertising or 
# publicity pertaining to distribution of the software without prior 
# written permission from OSF.  OSF makes no representations about the 
# suitability of this software for any purpose.  It is provided "as is" 
# without express or implied warranty. 
#
#
# ODE 2.1.1

echo "Saving original .sandboxrc as .sandboxrc.orig"

mv $HOME/.sandboxrc $HOME/.sandboxrc.orig

echo "Creating new .sandboxrc from .sandboxrc.orig"

cat $HOME/.sandboxrc.orig | \
   sed -e "s:mksb default_sandbox_base:mksb -dir:"  \
     -e "s:mksb default_machines:mksb -m:"  \
     -e "s:mksb copy_link_tools:mksb -tools:"  \
     -e "s:mksb copy_link_objs:mksb -obj:"  \
     -e "s:mksb copy_link_sources:mksb -src:" > $HOME/.sandboxrc

echo "Conversion done."
