#! /bin/ksh
# @(#)14	1.2  src/bldenv/bldtools/rename/genlmlinkdata.sh, bldprocess, bos412, GOLDA411a 3/23/93 12:53:46
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS: 	genlmlinkdata
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# NAME: genlmlinkdata
#
# FUNCTION: creates a file containing a command's dependencies of object files
#	    in libraries. It uses the symbol map generated during linking to
#	    the file.
#
# INPUT: cmdname,mapfile
#
# OUTPUT: 'lmlinkdata' file in the 'bldglobalpath'
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 2 (failure)
#
if (($# != 2))
then 
	print -u2 "Usage: $(basename $0) commandName mapFile"
	exit 2
fi
cmdname=$1
mapfile=$2
Pathname=${mapfile%/*}/
. bldinitfunc
bldinit
LMLINKDATA="$(bldglobalpath)/lmlinkdata"
awk 'BEGIN {
	command=ARGV[1] ; ARGV[1] = ""
	PathName=ARGV[2] ; ARGV[2] = ""
}
/^.*\.a\[.*/ {
	gsub(/.*from /,"")
	gsub(/\[/,":")
	gsub(/\].*/,"")
	FS = ":"
	if (index(dpndsc[$1],$2) == 0)
		dpndsc[$1] = dpndsc[$1] $2 ","
}
END { 
	for(lib in dpndsc){ 
		obj_files = substr(dpndsc[lib],0,length(dpndsc[lib])-1) 
		if ((index(lib,"./") == 1) || (index(lib,"../") == 1) || (index(lib,"/") == 0))
			lib = PathName lib
		print command,lib,obj_files
	}
}' $cmdname $Pathname $mapfile >> $LMLINKDATA
