# @(#)96        1.2  src/bldenv/findfile/findshipfile.awk.S, bldprocess, bos412, GOLDA411a 2/3/93 16:34:24
#
#   COMPONENT_NAME: BOSBUILD
#
#   FUNCTIONS: findfile
#		
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

awk '
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
