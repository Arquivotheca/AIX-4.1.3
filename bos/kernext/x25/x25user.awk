# @(#)67  1.2  src/bos/kernext/x25/x25user.awk, sysxx25, bos411, 9428A410j 4/25/91 11:31:51
# 
# COMPONENT_NAME: (sysxx25) X25 device driver
# 
# FUNCTIONS: 
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp.  1987, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
BEGIN      { sz=52; output=0 }
/X25USER ON/,/X25USER OFF/ {output=1} 
/X25USER/  	           {output=0}
output==0                  { next }
/^#define X25_RESERVED_HEADER_LENGTH/   {
        print "#define X25_RESERVED_HEADER_LENGTH (52)"
        next }
	{ print $0 } 

