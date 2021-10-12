# @(#)09        1.2  src/bldenv/findfile/findshipfile.S, bldprocess, bos412, GOLDA411a 2/22/93 22:13:22
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
$ODE_TOOLS/usr/bin/awk '
BEGIN {
   ship = ENVIRON["SHIP_PATH"]
   npath = split(ship,shipdir,":")
}

function findfile( file ) {

for (i = 1; i <= npath; i++)
{
  file1 = shipdir[i] file

  if (system("test -r " file1)) {
   flag = 0
  }
  else
  {
    print file1
    flag = 1
    break
  }
}
  if (flag == 0) print "Error: " file " not found in ship tree"
}

{
  findfile($0)
}'
