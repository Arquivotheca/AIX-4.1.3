# ! /usr/bin/ksh
# @(#)27    1.4  src/bos/usr/lib/boot/update_proto.sh, bosboot, bos411, 9428A410j 6/2/94 09:45:36
#
# COMPONENT_NAME: (BOSBOOT) Base Operating System Boot
#
# FUNCTIONS: update the proto file for base devices
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME: update_proto.sh
#
# FUNCTION:
#	update_proto command updates the proto file with entries
#	from an entry file.
#
#	each line in the entry file should be of the format:
#	<dest file name>  <type> <mode> 0 0 <full-path-name-of-source-file>
#	e.g.,
#	cfghscsi   ---- 777 0 0 /usr/lib/methods/cfghscsi
#	8d77.42.02 l--- 777 0 0 /usr/lib/methods/8d77.44.02
#	/usr/lib/drivers/pse/pse --- 777 0 0 /usr/lib/drivers/pse/pse
#
#	If the target file in an entry already exists in the proto file,
#	then the new entry will not be updated.  If an entry does not contain
#	a full path target name, the source file's path name will be taken as
#	the target path name.  This program assumes that the entries in the
#	entry file are unique.
#
# EXECUTION ENVIRONMENT:
#
#	CALLED BY:	bosboot
#
#	SYNTAX:
#		  update_proto entry_file base_protofile
#
#	OUTPUT:
#		A proto file including entries from the entry file
#		is written to stdout.
#
#	RETURN VALUE:
#		exit with return code 0 if successful.
#		exit with return code 1 if failure.
#

updproto () {
# syntax:
# updproto entry_file protofile
#
   awk 'BEGIN {
		path = "/"
		sp = 0
		stack[1] = 0	# stack
		num_entry = 0	# num of entries
		num_dirs  = 0	# num of dir to add files
		dirnames[1] = ""	# dir to add files
		filename = ARGV[1]
		ARGV[1] = ""
		get_entries(filename)
		}

	$2 ~ /d---/ 	{
			print $0
			path = path $1 "/"
			stack[++sp] = dir_to_addf(path) # check if dir to add files
			next
			}

	$1 ~ /\$/	{
			newsubdir(path)
			if ( stack[sp--] )
				add_files(path)
			print $0
			path = reduce_path(path)
			next
			}

	# not directory nor end-of-directory
	 		{
			print $0
			if ( !stack[sp] )  next
			check_dups( $0, path )	 # check for duplicate entries
			}

	function dirpath(Path,   arry) {
	# Retrieve the dir pathname
 		return substr(Path, 1, length(Path) - length(arry[split(Path, arry,"/")]))
	}

	function reduce_path(Path,   arry) {
	# Move one dir level up
		return substr(Path, 1, length(Path) - length(arry[split(Path,arry,"/") -1]) - 1)
	}

	function gettabs(Path,   i, n, tabs, arry) {
	# Generate tabs for indentation
		n = split(Path,arry,"/")
		tabs=""
		for( i = 2; i < n ; i++ )
			tabs = tabs "\t"
		 return tabs
	}

	function newsubdir(Path,   plen, tabs, i, prefix, newdir, arry) {
	# Check if a new subdir needs to be created and add files to it.
		plen = length(Path)
		tabs = gettabs(Path)
		i = 1;
		while ( i <= num_dirs) {
			prefix = substr(dirnames[i],1,plen)
			if(prefix == Path) {
				newdir = substr(dirnames[i],plen)
				split(newdir, arry, "/")
				print tabs arry[2] "\td--- 755 0 0"
				Path = Path arry[2] "/" 	# next level dir
				stack[++sp] = dir_to_addf(Path)
				if( stack[sp] )  i--		# adjust index
				newsubdir(Path)			# new subdir in newdir?
				if(stack[sp--]) add_files(Path)
				print tabs "\t$"
				Path = reduce_path(Path)
			}
		i++
		}
	}

	function dir_to_addf(Path,   i) {
	# Check if there are files to be added in this dir
		for( i = 1 ; i <= num_dirs; i++) {
			if( Path == dirnames[i] ) {
				if( i != num_dirs )
				    dirnames[i] = dirnames[num_dirs];
				num_dirs--
				return 1
			}
		}
		return 0
	}

	function check_dups(Src,Path,   i, fullname, arry) {
	# Check for duplicates entries in the dir
		split(Src, arry)
		fullname = Path arry[1]
		for( i = 1; i <= num_entry; i++ ) {
		 	split(entries[i],arry)
			if( fullname == arry[1] ) {   # check and discard duplicates
				if( i != num_entry ) {
					entries[i] = entries[num_entry]
					i--
				}
				num_entry--
			}
		}
	}

	function add_files(Path,   i, plen, dirp, tabs, arry) {
	# Add files to the directory
		plen = length(Path)
		tabs = gettabs(Path)
		for( i = 1; i <= num_entry; i++ )	 {
			split(entries[i], arry)
			dirp = dirpath(arry[1])
 			if( dirp == Path ) {
				print tabs substr(entries[i],plen+1)
				if( i != num_entry ) {
					entries[i] = entries[num_entry]
					i--
				}
				num_entry--
			}
		}
	}

	function get_entries(File,   i, found, arry, dirp) {
	# Setup entries with full path names
		while ( getline < File > 0 ) {
			if( split($0, arry) > 0 ) {
			   if ( arry[1] ~ /^\// )  {  # full path name
			   	dirp = dirpath(arry[1])
				entries[++num_entry] = $0
			   }
			   else {		#  only base file name
				dirp = dirpath(arry[6])
				entries[++num_entry] = dirp $0
			   }
			   found = 0
			   for( i = num_dirs; i > 0 ; i-- )
				if(dirnames[i] == dirp) {
					found = 1
					break
			   	}
			   if( found ) continue
			   dirnames[++num_dirs] = dirp
			}
		}

  	} ' $1 $2
}


#############################################################################
#	Main
#############################################################################

cat=/usr/bin/cat
# Parse the arguments
[ "$1" = "-?" -o $# -ne 2 ] && {
	echo " Usage: upd_proto entry-file old-proto-file"
	exit 1
}

UPDATEF=$1	# file with proto entries
OLD_PROTO=$2	#

[ ! -s $OLD_PROTO ] && exit 1

# check for the update file
[ ! -s $UPDATEF ] &&  {
	$cat $OLD_PROTO
	exit 0
}

updproto $UPDATEF $OLD_PROTO
exit 0
