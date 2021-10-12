# @(#)19        1.2  src/bldenv/proto_con/proto.awk, bldprocess, bos412, GOLDA411a 2/3/93 17:12:42
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

BEGIN {
npath = split(ship,shipdir,":")
}

function findfile( file ) {

for (i = 1; i <= npath; i++)
{
  file1 = shipdir[i] file
  
  if (system("test -r " file1)) {
   print "File : " file1 " not found"
  }
  else
  {
    print "File found as: " file1
    break
  }
}
}

{
if (NF == 6) 
{
  findfile($6)
}
else
{
  print $0
}
}
