#! /usr/bin/perl
# @(#)31	1.12  src/bldenv/bldtools/rename/showsummary.pl, bldtools, bos412, GOLDA411a 3/28/94 17:34:10
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: showsummary
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
# NAME: showsummary
#
# FUNCTION: Queries the status file and generates a report containing status
#	    information about which processes completed or failed for a given
#	    build cycle
#
# INPUT: filename(given on command line), build cycle (in BLDCYCLE environment
# 	 variable)
#
# OUTPUT: Standard output
#
# EXECUTION ENVIRONMENT: Build process environment
#

push(@INC,split(/:/,$ENV{"PATH"}));          # Including path 
do 'bldperlconst';
do 'bldperlfunc';

#######################################################################
# function:	bydate
# description:	compares two dates for sorting
# input:	2 date strings in the variables $a and $b of the form:
#
#		MONTH DAY HOUR:MINUTE
#
#		e.g.: Sep 17 12:45
#
# output:	 1 if first date is later than second
# 		 0 if first date is equal to second
# 		-1 if first date is earlier than second
#######################################################################
sub	bydate {
	return(&datecmp($a,$b));
}

#######################################################################
# function:	byalpha
# description:	compares two strings for sorting
# input:	2 strings in the variables $a and $b
# output:	 1 if first date is later than second
# 		 0 if first date is equal to second
# 		-1 if first date is earlier than second
#######################################################################
sub	byalpha {
	"$a" <=> "$b";
}

#######################################################################
# function:	print_array
# description:	prints out the contents of an array; only prints a
#		a specified # of elements on any given line then 
#		automatically prints new-line and tab indentation
#		for next line; also prints out title of array at
#		beginning of first line
# input:	array,# elements per line,title
#
# output:	 array title and array elements
#######################################################################
sub	print_array {
	local($token);
	local(*a) = @_;
	local($cnt) = $#a;
	if ($cnt >= 0) {
		local($max) = $_[1];
		local($title) = $_[2];
		print $title;
		for ($i = 0; $i <= $cnt; $i++) {
			if (($i > 0) && (($i % $max) == 0)) {
				print "\n\t\t\t";
			}
			$token = pop(@a);
			print $token," ";
		}
		print "\n";
	}
}

#######################################################################
# function:	print_report
# description:	uses collected information read from status file and
#		print out the full summary report
# input:	none
# output:	full summary report
#######################################################################
sub	print_report {
	# print current build cycle
	print "Current build cycle:\t",$ENV{'BLDCYCLE'},"\n";

	# print out header for active bldcycles
	&print_array(*activelist,1,"Active build cycles:\t");

	# if no prebuild entries were found print out "no entries found" 
	# msg and exit
	if (! defined $prebuild_found) {
		print "No entries found in status file for build cycle: ",
		       $ENV{'BLDCYCLE'},"\n";
		exit 0;
	}

	# print out prebuild-cmvcextract info
	print "prebuild:\n";
	print "   -cmvcextract:\t";
	print $#prebuildcex_keys + 1," extracts across ",
	      $#datecex_keys + 1," release(s)\n";

	# print out prebuild-levelmerge info
	print "   -levelmerge:\t\t";
	print (($#prebuildcex_keys + 1) - ($#temp1 + 1)," merged, ",
	        $#bmg + 1," not merged\n");
	&print_array(*bmg,2,"    (not merged)\t");

	# print out prebuild-cmvcmerge info
	print "   -cmvcmerge:\t\t";
	print (($#prebuildcex_keys + 1) - ($#omg + 1)," merged, ",
	        $#omg + 1," not merged\n");
	&print_array(*omg,2,"    (not merged)\t");

	# print out prebuild-cmvccommit info
	print "   -cmvccommit:\t\t";
	print (($#prebuildcex_keys + 1) - ($#oct + 1)," committed, ",
	        $#oct + 1," not committed\n");
	&print_array(*oct,2,"    (not committed)\t");
	print "\n";

	print "bldabstracts:\t\t";
	if (defined $bldabstracts) {
		print "$bldabstracts\n\n";
	} else {
		print "not done\n\n";
	}

	print "bldretain:\t\t";
	print (($#bldretain_keys + 1) - ($#bldretain_f + 1)," success, ",
	       $#bldretain_f + 1," failure\n");
	&print_array(*bldretain_f,5,"    (failure)\t\t");
	print "\n";

	# exit now if no v3bld entries were found in status file
	if (! defined $v3bld_found) {
		exit 0;
	}

	# print out v3bld info
	print "v3bld:\n";
	print "    (summary)\t\t";
	print ($#rels_built + 1," releases built, ",
	        $#rels_pending + 1, " pending\n");
	&print_array(*rels_pending,5,"    (pending)\t\t");
	print "\n";
	
	print "postbuild:\t\t";
	if (defined $postbuild) {
		print "$postbuild\n\n";
	} else {
		print "not done\n\n";
	}

	# exit now if no subptf entries were found in status file
	if (! defined $bldptf_found) {
		exit 0;
	}
	
	# print out bldptf-getlists info
	print "subptf:\n";
	print "   -getlists:\t\t";
	print (($#v3blddate_keys + 1) - ($#gls_f + 1 + $#gls_nd + 1),
	       " success, ",$#gls_f + 1," failure, ", $#gls_nd + 1, 
	       " not done\n");
	&print_array(*gls_f,5,"    (failure)\t\t");
	&print_array(*gls_nd,5,"    (not done)\t\t");
	
	# if a bldptf-normalizexref entry was found, print out its status value
	if (defined $bldptfxrf) {
		print "   -normalizexref:\t",$bldptfxrf,"\n";
	}
	
	# print out bldptf-finddependents info
	print "   -finddependents:\t";
	print (($#v3blddate_keys + 1) - ($#fdp_f + 1 + $#fdp_nd + 1),
	       " success, ",$#fdp_f + 1," failure, ", $#fdp_nd + 1, 
	       " not done\n");
	&print_array(*fdp_f,5,"    (failure)\t\t");
	&print_array(*fdp_nd,5,"    (not done)\t\t");
	
	# if a bldptf-package entry was found, print out its status value
	if (defined $bldptfpkg) {
		print "   -package:\t\t",$bldptfpkg,"\n";
	}
	print "\n";
	
	print "postpackage:\t\t";
	if (defined $postpackage) {
		print "$postpackage\n\n";
	} else {
		print "not done\n\n";
	}

	# exit now if no genptf entries were found in status file
	if (! defined $genptf_found) {
		exit 0;
	}
	
	# print out genptf-lpp info
	print "genptf (LPPs):\n";
	print "    (summary)\t\t";
	print (($#bldptflpp_keys + 1) - ($#lpp_f + 1 + $#lpp_nd + 1),
	        " success, ",$#lpp_f + 1," failure, ", $#lpp_nd + 1, 
		" not done\n");
	&print_array(*lpp_f,5,"    (failure)\t\t");
	&print_array(*lpp_nd,5,"    (not done)\t\t");

	print "\nsniff test:\t\t";
	if (defined $sniff) {
		print "$sniff\n";
	} else {
		print "not done\n";
	}
}
	
################################### M A I N ###################################

if (! defined $ENV{'BLDCYCLE'}) {
	exit 1;
}

# reading status file

while (<>) {
	chop;

	# parsing line of data from status file
	($type,$bldcycle,$release,$level,$name,$date,
        $lpp,$misc,$subtype,$status)=split(/\|/);

	# parsing bldcycle entries
	if ($type =~ /bldcycle/) {
		$activebldcycle{$bldcycle} = $status;
	}

	# if entry does not pertain to selected build cycle, skip it
	if ($ENV{'BLDCYCLE'} ne $bldcycle) {
		next;
	}

	# parsing prebuild entries
	if ($type =~ /prebuild/) {
		# at least one prebuild entry found; set flag
		$prebuild_found = 1;

		# parsing prebuild-cmvcextract entries
		if ($subtype =~ /cmvcextract/) {

			# updating associative array whose keys are release/
			# level combinations for prebuild-cmvcextract entries
			$prebuildcex{$release."(".$level.")"}=$status;

			# updating associative array whose keys are release
			# names for prebuild-cmvcextract entries and whose
			# values are the dates for those entries; only over-
			# write the value of an entry if its date is earlier
			# than that of the current line
			if ((! defined $datecex{$release}) || 
			    (&datecmp($datecex{$release},$date) == -1)) {
				$datecex{$release}=$date;
			}
			$cexv3{$date} .= "$type $release ";
		}
		# parsing prebuild-levelmerge entries
		if ($subtype =~ /levelmerge/) {
			# updating associative array whose keys are release/
			# level combinations for prebuild-levelmerge entries
			$prebuildbmg{$release."(".$level.")"}=$status;
		}
		# parsing prebuild-cmvcmerge entries
		elsif ($subtype =~ /cmvcmerge/) {
			# updating associative array whose keys are release/
			# level combinations for prebuild-cmvcmerge entries
			$prebuildomg{$release."(".$level.")"}=$status;
		}
		# parsing prebuild-cmvccommit entries
		elsif ($subtype =~ /cmvccommit/) {
			# updating associative array whose keys are release/
			# level combinations for prebuild-cmvccommit entries
			$prebuildoct{$release."(".$level.")"}=$status;
		}
	}

	elsif ($type =~ /bldabstracts/) {
		$bldabstracts = $status;
	}

	elsif ($type =~ /bldretain/) {
		$bldretain{$release} = $status;
	}

	# parsing v3bld entries
	elsif ($type =~ /v3bld/) {
		# at least one v3bld entry found; set flag
		$v3bld_found = 1;

		# updating associative array whose keys are release/level
		# combinations for v3bld entries
		$v3bld{$release."(".$level.")"}=$status;

		# updating associative array whose keys are release names
		# for v3bld entries and whose values are the date of the
		# entries; only overwrite value for an entry if its date 
		# is earlier than that of the current line
		if ((! defined $v3blddate{$release}) || 
		    (&datecmp($v3blddate{$release},$date) == -1)) {
			$v3blddate{$release}=$date;
		}
		$cexv3{$date} .= "$type $release ";
	}

	elsif ($type =~ /postbuild/) {
		$postbuild = $status;
	}

	# parsing subptf entries
	elsif ($type =~ /subptf/) {
		# at least one subptf entry found; set flag
		$bldptf_found = 1;

		# updating associative array whose keys are release names
		# for bldptf-getlists entries
		if ($subtype =~ /getlists/) {
			$bldptfgls{$release}=$status;
		}

		# updating associative array whose keys are release names
		# for bldptf-finddependents entries
		elsif ($subtype =~ /finddependents/) {
			$bldptffdp{$release}=$status;
		}

		# updating associative array whose keys are release names
		# for bldptf-lpp entries
		elsif ($subtype =~ /lpp/) {
			$bldptflpp{$lpp}=$status;
		}

		# updating associative array whose keys are release names
		# for bldptf-normalizexref entries
		elsif ($subtype =~ /normalizexref/) {
			$bldptfxrf = $status;
		}

		# updating associative array whose keys are release names
		# for bldptf-package entries
		elsif ($subtype =~ /package/) {
			$bldptfpkg = $status;
		}
	}

	if ($type =~ /postpackage/) {
		$postpackage = $status;
	}

	# parsing genptf entries
	elsif ($type =~ /genptf/) {
		# at least one genptf entry found; set flag
		$genptf_found = 1;

		# updating associative array whose keys are release names
		# for genptf-lpp entries
		$genptf{$lpp} = $status;
	}

	# get sniff test status
	elsif ($type =~ /sniff/) {
		$sniff = $status;
	}
}

# extracting keys from associative arrays

(@cexv3_dates)         = sort bydate (keys %cexv3);
(@activebldcycle_keys) = (keys %activebldcycle);
(@prebuildcex_keys)    = (keys %prebuildcex);
(@datecex_keys)        = (keys %datecex);
(@prebuildbmg_keys)    = (keys %prebuildbmg);
(@prebuildomg_keys)    = (keys %prebuildomg);
(@prebuildoct_keys)    = (keys %prebuildoct);
(@bldretain_keys)      = (keys %bldretain);
(@v3bld_keys)          = (keys %v3bld);
(@v3blddate_keys)      = (keys %v3blddate);
(@bldptflpp_keys)      = (keys %bldptflpp);

######################### active build cycle list #######################

@activebldcycles = sort byalpha @activebldcycle_keys;
foreach $cyc (@activebldcycles) {
	push(@activelist,$cyc." (".$activebldcycle{$cyc}." phase)");
}

######################### prebuild-levelmerge ###########################

foreach $rellev (@prebuildcex_keys) {
	# if a prebuild-levelmerge entry does not exist with the same 
	# release/level value as the current prebuild-cmvcextract key then 
	# the release was not merged at the given level
	if (! defined $prebuildbmg{$rellev}) {
		push(@bmg,$rellev);
	}
}

########################## prebuild-cmvcmerge ##########################

foreach $rellev (@prebuildcex_keys) {
	# if a prebuild-cmvcmerge entry does not exist with the same 
	# release/level value as the current prebuild-cmvcextract key then 
	# the release was not merged at the given level
	if (! defined $prebuildomg{$rellev}) {
		push(@omg,$rellev);
	}
}

######################### prebuild-cmvccommit ##########################
foreach $rellev (@prebuildcex_keys) {
	# if a prebuild-cmvccommit entry does not exist with the same 
	# release/level value as the current prebuild-cmvcextract key then 
	# the release was not committed at the given level
	if (! defined $prebuildoct{$rellev}) {
		push(@oct,$rellev);
	}
}

##############################  bldretain  ##############################

foreach $rel (@bldretain_keys) {
	if ($bldretain{$rel} eq "failure") {
		push(@bldretain_f,$rel);
	}
}

##############################    v3bld    ##############################

# opening release list file
open(RELLIST,$ENV{'BLDENV'}."/usr/bin/RELEASE_LIST");

foreach $date (@cexv3_dates) {
	@cexv3_type_rel = split(/ /,$cexv3{$date});
	for ($i = 0; $i < $#cexv3_type_rel; $i += 2) {
		$type = $cexv3_type_rel[$i];
		$release = $cexv3_type_rel[$i+1];
		if ($type eq "prebuild") {
			$pending{$release} = $DEFINED;
		} elsif ($type eq "v3bld") {
			$built{$release} = $DEFINED;
			if (defined $pending{$release}) {
				delete $pending{$release} ;
			}
		}
	}
}

(@rels_pending) = (keys %pending);
(@rels_built) = (keys %built);

########################## bldptf-getlists ##############################

# getting each v3bld release key
foreach $rel (@v3blddate_keys) {
	# if there is a bldptf-getlists entry defined for the release and its
	# status value is "success" then bldptf-getlists completed successfully
	if (defined $bldptfgls{$rel}) {
		if ($bldptfgls{$rel} eq "failure") {
			# if there is a bldptf-getlists entry defined for the 
			# release but its status value was failure then 
			# bldptf-getlists did not complete successfully
			push(@gls_f,$rel);
		}
	}
	# if there was no bldptf-getlists entry define for the release then
	# bldptf-getlists was not even done on the release
	else {
		push(@gls_nd,$rel);
	}
}

######################## bldptf-finddependents ###########################

# getting each v3bld release key
foreach $rel (@v3blddate_keys) {
	# if there is a bldptf-finddependents entry defined for the release and
	# its status value is "success" then bldptf-getlists completed 
	# successfully
	if (defined $bldptffdp{$rel}) {
		# if there is a bldptf-finddependents entry defined for the 
		# release but its status value was failure then bldptf-
		# findependents did not complete successfully
		if ($bldptffdp{$rel} eq "failure") {
			push(@fdp_f,$rel);
		}
	}
	# if there was no bldptf-finddependents entry define for the release 
	# then bldptf-getlists was not even done on the release
	else {
		push(@fdp_nd,$rel);
	}
}

##############################   genptf-LPPs  ############################

# get each release associated with a bldptf-lpp entry
foreach $rel (@bldptflpp_keys) {
	# if there was a genptf entry with the same lpp and its status is
	# success then it completed successfully
	if (defined $genptf{$rel}) {
		# if there was a genptf entry with the same lpp but its 
		# status is failure then it failed
		if ($genptf{$rel} eq "failure") {
			push(@lpp_f,$rel);
		}
	}
	# if there was no genptf entry with the same release then it was not 
	# done
	else {
		push(@lpp_nd,$rel);
	}
}

&print_report;
