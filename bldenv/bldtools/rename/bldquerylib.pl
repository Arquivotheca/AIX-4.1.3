#! /usr/bin/perl
# @(#)62	1.8  src/bldenv/bldtools/rename/bldquerylib.pl, bldtools, bos412, GOLDA411a 8/16/93 15:08:08
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: bldquery
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME: bldquerylib
#
# FUNCTION: 
#
# INPUT:
#
# OUTPUT: Standard output
#
# EXECUTION ENVIRONMENT: Build process environment
#

do 'bldperlconst';
do 'bldperlfunc';

$BQTOP = &bldhostsfile_perl("get_afs_base",$FALSE,"HOSTSFILE_AFSBASE");
chop $BQTOP;
if ( $BQTOP eq "" ) {
   printf(STDERR
          "bldhostsfile_perl(get_afs_base,$FALSE,HOSTSFILE_AFSBASE) failed\n");
   &cleanup;
}

############################################################################
# function:	mapdbm
# description:	maps a given associative array to a DBM file
# input:	pointer to source associative array contained in *srcarray; 
#		dbm file name contained in $filename 
# output:	DBM file containing elements of srcarray
# remarks:	
############################################################################
sub	mapdbm {
	# get function input parameters
	local(*srcarray,$filename) = @_;

	# declare destination associative array
	local(%destarray);

	# open desitination associatie array DBM file
	dbmopen(%destarray,"$filename",0750);

	# transfer source associative array to DBM associative array
	for $key (keys %srcarray) {
		$destarray{$key} = $srcarray{$key};
	}
	dbmclose(%destarray);
}

############################################################################
# function:	getshipfileids
# description:	searches dependency tree for ship file ids and stores them
#		in the array @shipids
# input:	$id:		file id of the file from which to begin the 
#				search thru the dependency tree 
#		*shipids	pointer to array where ship file ids are to
#				be stored
#		*targettree	pointer to dependency tree array
# output:	ship file ids stored in shipids array
# remarks:	
############################################################################
sub	getshipfileids {
	# get function input parameters
	local($id,*shipids,*targettree) = @_;
	local(%alltargets);

	# get all targets in the dependency tree underneath the give file id
	# and store them in the @alltargets array
	&getchildfileids($id,*alltargets,*targettree,$FALSE);

	# for each file id in alltargets, push only those that are ship files
	# onto the shipids stack
	foreach $targetid (keys %alltargets) {
		if ($decode{$targetid} =~ /.\/ship/) {
			push(@shipids,$targetid);
		}
	}
}

############################################################################
# function:	getsrcfileids
# description:	searches dependency tree for source file ids and stores them
#		in the array @srcids
# input:	$id:		file id of the file from which to begin the 
#				search thru the dependency tree 
#		*srcids		pointer to array where source file ids are to
#				be stored
#		*deptree	pointer to dependency tree array
#		*srcrels	pointer to array containing list of releases
#				for which a given file is a source file; used
#				for determining if a file is a source file
#				since all source files are contained in this
#				array
# output:	source file ids stored in srcids array
# remarks:	
############################################################################
sub	getsrcfileids {
	# get function input parameters
	local($id,*srcids,*deptree,*srcrels) = @_;
	local(%alldependents);

	# get all dependents in the dependency tree underneath the give file 
	# id and store them in the @alldependents array
	&getchildfileids($id,*alldependents,*deptree,$FALSE);

	# for each file id in alldependents, push only those that are ship 
	# files onto the shipids stack
	foreach $depid (keys %alldependents) {
		if (defined $srcrels{$depid}) {
			push(@srcids,$depid);
		}
	}
}

############################################################################
# function:	getchildfileids
# description:	searches dependency tree for file ids in the dependency chain
#		underneath a given file
# input:	$id:		file id of the file from which to begin the 
#				search thru the dependency tree 
#		*childlist	pointer to associative array where child file 
#				ids are to be stored
#		*depend_tree	pointer to dependency tree array
# output:	child file ids stored in childids array
# remarks:	
############################################################################
sub	getchildfileids {
	# get function input parameters
	local($id,*childids,*depend_tree) = @_;

	(@stack) = ();

	# seed stack w/ first file id
	push(@stack,$id);

	# pop id off top of stack as long as stack is not empty
	while(defined ($id=pop(@stack))) {
		# if the release filter is set and the current file is
		# not a build environment file and the file does not belong
		# to the release specified by the release filter then skip it
		if ($RELFILTER ne "") {
			if ((! defined $bldenv{$id}) && 
		            ($decode{$id} !~ /$RELFILTER/)) {
			    next;
			}
		}

		# if the current file id is not already defined in the 
		# %childids array then add it now
		if (! defined $childids{$id}) {
			$childids{$id} = $DEFINED;
			if (defined $depend_tree[$id]) {
				push(@stack,split(/ /,$depend_tree[$id]));
			}
		}
	}
}

sub	ParseLPPOption {
	local($lppopt) = @_;

	local(@subname) = split(/\./,$lppopt);
	local($lppname) = $subname[0];
	if ($subname[$#subname] eq "data") {
		$lppname = "$lppname.data";
	}
	return($lppname);
}
