#!/usr/bin/ksh
# @(#)07        1.12  src/bldenv/pkgtools/makecdfs.sh, pkgtools, bos41J, 9523A_all 6/1/95 11:48:29
#
# COMPONENT_NAME: bosboot
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.


################################################################################
usage()
{
	echo "${self}  [ -d destDir ] [ -s sourceDir ] [ -i imagesList ]"\
		"[ -p protoFile ]\\n          [ -n ] [ -t installType ]\n"
	echo "  where destDir, sourceDir, imagesList, and protoFile are fully"\
		"qualified\\n  pathnames.\n"
	echo "  FLAGS:"
	echo "  -d  Destination directory.  The directory location where"\
		"the filesystem\\n      will be created.  Default: pwd"
	echo "  -s  Initial source directory path.  The directory location"\
		"from which the\\n      input files can be found.  ${self}"\
		"will look for install images in the\\n      subdirectory"\
		"'inst.images'.\\n      Default: ${SRC_DIR}"
	echo "  -i  A file which lists the images to include in the"\
		"filesystem.  The images\\n      included in this list must"\
		"exist in the subdirectory 'inst.images'.  If\\n      no list"\
		"is specified, ${self} will include all images found in the"\
		"\\n      subdirectory 'inst.images'."
	echo "  -p  A proto file that specifies the cdrom filesystem contents."\
		" Default:\\n      ${SRC_DIR}/inst.images/cdfs.proto"
	echo "  -n  Do not create a table of contents file for the images in"\
		"the\\n      subdirectory 'inst.images'."
	echo "  -t  Bos install type.  Valid values are 'full','client', "\
		"'personal' or 'eserver'."
	exit 1
}

################################################################################
getOpts() {
	while getopts ":d:i:np:s:t:v" option
	do
		case $option in
			d) DEST_DIR=$OPTARG;;
			i) IMAGESLIST=$OPTARG;;
			n) MAKE_TOC=no;;
			p) PROTO=$OPTARG;;
			s) SRC_DIR=$OPTARG;;
			t) INST_TYPE=$OPTARG;;
			v) VERBOSE=true;;
			:) echo "${self}:  The \"$OPTARG\" option requires an"\
				"argument.\n"
			   usage;;
			\?) usage;;
		esac
	done
}

################################################################################
check_args() {

	# check the IMAGESLIST variable.  The default is the null value.
	if [ -n "$IMAGESLIST" -a ! -s "$IMAGESLIST" ]; then
		echo "The file \""$IMAGESLIST"\" cannot be accessed."
		exit 1
	fi

	# check the PROTO variable.  This variable has been initialized.
	if [ ! -s "$PROTO" ]; then
		echo "The file \""$PROTO"\" cannot be accessed."
		exit 1
	fi

	# check the SRC_DIR variable.  This variable has been initialized.
	if [ ! -d "$SRC_DIR" ]; then
		echo "The directory \"$SRC_DIR\"\\ndoes not exist."
		exit 1
	elif [ ! -r "$SRC_DIR" ]; then
		echo "Insufficent permission for \"$SRC_DIR\"."
		exit 1
	fi

	# check the INST_TYPE variable.  The default is the null value.
	if [ -n "$INST_TYPE" ]; then
		case "$INST_TYPE" in
			client | full | eserver) ;;
			personal) INST_BUNDLE=${SRC_DIR}/src/bos/usr/lpp/bosinst/data/powerdt.bnd
				if [ ! -r $INST_BUNDLE ]; then
					echo "${self}: The file $INST_BUNDLE\\ndoes "\
					"not exist or is not readable by this process."\
					"This file is the install\\n bundle required"\
					"to create a \"$INST_TYPE\" type CDROM.\n"
					exit 1
				fi ;;

			*) echo "${self}: ERROR: The value \"$INST_TYPE\","\
				"specified for the \"-t\" flag, is invalid.\n"
				usage;;
		esac
		INST_TEMPLATE=${SRC_DIR}/src/bos/usr/lpp/bosinst/data/bosinst.data.$INST_TYPE
		if [ ! -r $INST_TEMPLATE ]; then
			echo "${self}: The file $INST_TEMPLATE\\ndoes not"\
				"exist or is not readable by this process."\
				"This file is the install\\ntemplate required"\
				"to create a \"$INST_TYPE\" type CDROM.\n"
			exit 1
		fi
	fi

	# check the DEST_DIR variable.  The default is the current directory.
	# keep this one last since it asks the user a question, and all
	#	other arguments should be verified before bothering the user
	if [ -z "$DEST_DIR" ]; then
		echo "No destination directory has been specified.  Do you"\
			"wish to use the\\ncurrent directory? [y or n] y\b\c"
		read RESPONSE
		if [ -z "$RESPONSE" -o "$RESPONSE" = "y" ]; then
			DEST_DIR=$(pwd)
		else
			echo "${self}: No destination directory - exiting."
			exit 1
		fi
	elif [ ! -d "$DEST_DIR" ]; then
		echo "The directory \"$DEST_DIR\"\\ndoes not exist."
		exit 1
	elif [ ! -w "$DEST_DIR" ]; then
		echo "Insufficent permission for \"$DEST_DIR\"."
		exit 1
	fi

}

################################################################################
# Let user confirm input values before continuing.

confirmFlags() {
	echo "\\n\\nA filesystem will be created in $DEST_DIR\\npopulated"\
		"by files from $SRC_DIR\n"
	if [ -z "$IMAGESLIST" ]
	then
		echo "All images in ${SRC_DIR}/inst.images will be included\n"
	else
		echo "Images specified in $IMAGESLIST will be included\n"
	fi

	echo "Do you want to continue? [y or n] y\b\c"
	read RESPONSE
	if [ -n "$RESPONSE" -a "$RESPONSE" != "y" ]; then
		echo "Exiting ${self}."
		exit 1
	fi
}

################################################################################
populate_dir() {
/usr/bin/awk '
BEGIN {
	message1=" of the proto file is unacceptable."
	first_line = 1
	src_dir = ARGV[1]
	dest_dir = ARGV[2]
	if (ARGV[4] == "true")
		chown_flag=1
	if (ARGV[3] == "true") {
		print "Verbose output..."
		verbose=1
	}
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ""
	# if the src_dir is "/", then set it to null to avoid confusion
	if (src_dir == "/")
		src_dir = ""
}

/^(\t| )*#/ { next }		# skip comments

NF > 1 && first_line == 1 {
	# process first line of proto file
	# it looks like: "<noboot> 0 0"
	first_line = 0
	for (i=2; i<=NF; i++)
		if ($i !~ /^[0-9]+$/) {
			print "Line number " NR message1
			exit 1
		}
	next
}

NF == 4 && $2 ~ /^[0-9][0-9][0-9]$/ && $3 ~ /^[0-9]+$/ && $4 ~ /^[0-9]+$/ {
	# this is the very first directory entry
	# it looks like: "d-g-  755  3 3"
	next
}

NF > 4 && ($3 !~ /^[0-9][0-9][0-9]$/ || $4 !~ /^[0-9]+$/ || $5 !~ /^[0-9]+$/) {
	# verify umask and uid/gid
	print "Line number " NR message1
	exit 1
}

NF == 5 && length($2) == 4 && $2 ~ /^d/ {
	# this line is the beginning of a new directory
	full_path=full_path "/" $1
	if (verbose)
		print "Creating directory: " dest_dir full_path
	system("/usr/bin/mkdir " dest_dir full_path)
	system("/usr/bin/chmod " $3 " " dest_dir full_path)
	if (chown_flag)
		system("/usr/bin/chown " $4 ":" $5 " " dest_dir full_path)
	next
}

$0 ~ /^(\t| )*\$(\t| )*$/ {
	# this line is the closure of a directory
	if (full_path != "")
		full_path=substr(full_path, 1, match(full_path, /\/[^\/]+$/)-1)
	next
}

NF == 6 && length($2) == 4 && $2 == "----" {
	# this line is a regular file entry
	if (verbose) {
		print "Copying file: " src_dir $6 " to"
		print "	" dest_dir full_path "/" $1
	}
	if (system("test -r " src_dir $6)) {
		# the "test" returns a 1 to "system" if the file is not found
		print "Not found: " src_dir $6
		exit 1
	} else {
		system("/usr/bin/cp " src_dir $6 " " dest_dir full_path "/" $1)
		system("/usr/bin/chmod " $3 " " dest_dir full_path "/" $1)
		if (chown_flag)
			system("/usr/bin/chown " $4 ":" $5 " " dest_dir full_path "/" $1)
	}
	next
}

NF == 6 && length($2) == 4 && $2 == "l---" {
	# this line is a symbolic link entry
	if (verbose) {
		print "Creating sym link: " dest_dir full_path "/" $1 "->"
		print "	" $6
	}
	system("/usr/bin/ln -s " $6 " " dest_dir full_path "/" $1)
#	if (chown_flag)
#		system("/usr/bin/chown -h " $4 ":" $5 " " dest_dir full_path "/" $1)
	next
}

NF > 0 {
	print "Line number " NR message1
	exit 1
}

' $*
}

################################################################################
# If a bos install type was specified, get the corresponding bosinst.data file.
# If INST_BUNDLE is defined, get the corresponding bundle file.

get_data_file() {
	if [ -n "$INST_TYPE" ]; then
		/usr/bin/cp $INST_TEMPLATE \
			$DEST_DIR/usr/lpp/bosinst/bosinst.template
		if [ $? -ne 0 ]; then
			echo "Could not copy $INST_TEMPLATE\\n\\tto the"\
				"destination directory.  Exiting ${self}."
			exit 1
		fi
	fi
	if [ -n "$INST_BUNDLE" ]; then
		/usr/bin/cp $INST_BUNDLE \
			$DEST_DIR/post-install.bnd
		if [ $? -ne 0 ]; then
			echo "Could not copy $INST_BUNDLE\\n\\tto the"\
				"destination directory.  Exiting ${self}."
			exit 1
		fi
	fi
}

################################################################################
# Copy the install images specified in $IMAGESLIST from ${SRC_DIR}/inst.images
# If no images_list was given, copy all images in ${SRC_DIR}/inst.images

getImages() {

	cd ${SRC_DIR}/inst.images
	echo "Copying install images...\n"
	if [ -z "$IMAGESLIST" ]; then
		/usr/bin/cp * $DEST_DIR/usr/sys/inst.images 2>>/tmp/${self}.err.$$
	else
		/usr/bin/cat $IMAGESLIST | /usr/bin/xargs -i /usr/bin/cp {} $DEST_DIR/usr/sys/inst.images \
			2>>/tmp/${self}.err.$$
	fi
}

################################################################################
# Make table of contents in $DEST_DIR/usr/sys/inst.images directory

maketoc() {
	echo "Creating a table of contents...\n"

	#---------------------------------------------------------
	# The executable used here is the 3.2 version of inutoc, |
	# customized to create a toc in 4.1 format.  This can be |
	# changed when this script is run from a 4.1 machine.    |
	#---------------------------------------------------------
	TOC_PATH=/afs/austin/depts/e94s/projects/bpdt/bin
	$TOC_PATH/inutoc.mkcdfs $DEST_DIR/usr/sys/inst.images
	if [ $? -ne 0 ]
	then
		echo "${self}: WARNING: inutoc command failed.\\nDo you want"\
			"to continue? [y or n] y\b\c"
		read RESPONSE
		if [ -n "$RESPONSE" -a "$RESPONSE" != "y" ]; then
			echo "Exiting ${self}."
			exit 1
		fi
	fi
}


################################################################################
# Begin Main program

# initialize a few things
self=${0##*/}		# get rightmost pattern
MAKE_TOC=yes
SRC_DIR=/afs/austin/aix/project/aix411/build/latest
PROTO=${SRC_DIR}/inst.images/cdfs.proto
VERBOSE=false
CHOWN_FLAG=false
unset IMAGESLIST DEST_DIR INST_TYPE

getOpts $*

check_args

confirmFlags

# determine the user id and call populate_dir with appropriate chown flag
#	(need to be root in order to chown files to root)
if [ $(/usr/bin/id -ru) -eq "0" ]; then
	CHOWN_FLAG=true
fi

populate_dir ${SRC_DIR} ${DEST_DIR} ${VERBOSE} ${CHOWN_FLAG} ${PROTO}
if [ $? -ne 0 ]; then
	echo "The directory population was not successful.  Exiting ${self}."
	exit 1
fi

get_data_file

getImages

[ ${MAKE_TOC} = "yes" ] && maketoc

#-------------------------------------------------
# Inform the user of the output files in /tmp    |
#-------------------------------------------------
if [ -s /tmp/${self}.err.$$ ]
then
	echo "\\nErrors occurred while running ${self}.  Check the error log"\
		"\\n/tmp/${self}.err.$$ to verify the new filesystem in"\
		"\\n$DEST_DIR."
fi
