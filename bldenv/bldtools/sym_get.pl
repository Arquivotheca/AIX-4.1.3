#! /usr/bin/perl
# @(#)61	1.20  src/bldenv/bldtools/sym_get.pl, bldprocess, bos412, GOLDA411a 6/7/93 09:58:48
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: sym_get
#	     write_to_abstracts
#	     remove_dup_apars
#	     get_note
#	     get_abstract
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME: sym_get
#
# FUNCTION: Writes the APAR, the abstract and the corresponding symptoms of a
#	    defect to the 'abstracts' file 'bldglobalpath'
# INPUT: APAR, defect, Abstract, The symptoms file, and the abstracts file
#
# OUTPUT:
#
# SIDE EFFECTS: Also maintains a raw format of the 'abstracts' file called
#		'abstracts.raw'
#
# EXECUTION ENVIRONMENT:  Build process environment
#

$APARS = shift(ARGV);		# The APAR number
$APARS =~ s/[iI][xX]/IX/g;	# converting APAR number prefix to all caps
$DEFECT = shift(ARGV);		# The defect number
$ABSTRACTLIST = shift(ARGV);	# The file with defects and abstracts
$SYMPTOMS = shift(ARGV);	# The symptoms file containing notes of defects
				# for a release level in the 'bldreleasepath'
$ABSTR_FILE = shift(ARGV);	# The abstracts file in the 'bldhistorypath'
$ABSTR_FILE_RAW = "$ABSTR_FILE.raw";	# The raw format of the abstracts file
if (shift(ARGV) eq "-w"){
	&write_to_abstracts;
	exit 0;
}
&get_abstract(*ABSTRACT);
&remove_dup_apars;

###############################################################################

# This function copies the raw abstracts file into the real abstracts file

sub write_to_abstracts{

	local(*INPUT,*OUTPUT); 
	local($apars,$abstract,$note,$DLM);

	$DLM = "<@>";
	open(OUTPUT,">$ABSTR_FILE");
	open(INPUT,"<$ABSTR_FILE_RAW");
	while (<INPUT>){
		chop;
		local($apars,$abstract,$note) = split(/\375\374/);
		$note =~ s/\376(\d+)\377/pack(C,$1)/eg;
		$note =~ s/^/ /;
		$note =~ s/\n/\n /g;
		$note =~ s/ $//;
		print OUTPUT "$apars $abstract\n\n$note\n$DLM\n";
	
	}
}

###############################################################################

# This function removes the duplicate apar entries in the raw abstracts file

sub remove_dup_apars{
	local($printed,$apars,$abstract,$note);
	local(*RAWINPUT,*RAWOUPUT);

	&get_note($SYMPTOMS,*NOTE);
	#exit 0 if ($NOTE eq /^$/);
		# making a backup of the raw file
	rename("$ABSTR_FILE_RAW","$ABSTR_FILE_RAW.bak"); 
	open (RAWINPUT,"<$ABSTR_FILE_RAW.bak");
	open (RAWOUTPUT,">$ABSTR_FILE_RAW");
	while (<RAWINPUT>){
		chop;
		($apars,$abstract,$note) = split(/\375\374/);
		$apars =~ s/[iI][xX]/IX/g;
		if ($APARS eq $apars){
			print RAWOUTPUT "$APARS\375\374$ABSTRACT\375\374$NOTE\n"
				if ($NOTE ne "");
			$printed = 1;
		}
		else{
			print RAWOUTPUT "$apars\375\374$abstract\375\374$note\n";
		}
	}
	print RAWOUTPUT "$APARS\375\374$ABSTRACT\375\374$NOTE\n"
		if((! $printed) && ($NOTE ne ""));
	close(RAWINPUT);
	unlink "$ABSTR_FILE_RAW.bak";
	close(RAWOUTPUT);
}

###############################################################################

# This function returns the symptom string from the symptoms file for a defect

sub get_note{

	local($file,*NOTE) = @_;
        local($shortdefect,$family) = split(/:/,$DEFECT,2);
        local($defect);
	local(*INPUT);
	local($note);
	open (INPUT,"<$file");
	while (<INPUT>){
		chop;
		($defect,$note) = split(/\|/,$_,2);
	
		# if the defect is found, Change the unprintable charactors
		# which were converted from newlines by 'sym_lookup' in 
		# 'prebuild' to newlines in the corresponding note and write 
		# the note to the 'abstracts' file in the releasepath
		# Check for defect with ($DEFECT) and without ($shortdefect)
		# CMVC_FAMILY.  Old file format will not have CMVC_FAMILY
		# and defects would have only come from $DEFAULT_CMVCFAMILY.
	
		if ( ($DEFECT eq $defect) || 
                     ($shortdefect eq $defect &&
                      $family eq $ENV{'DEFAULT_CMVCFAMILY'}) ){
			$note =~ s/^[(\376\d+\377)\s]*[sS][tT][aA][rR][tT]_[sS][yY][mM][pP][tT][oO][mM](.*)[sS][tT][oO][pP]_[sS][yY][mM][pP][tT][oO][mM][(\376\d+\377)\s]*$/$1/;
			$note =~ s/^[(\376\d+\377)\s]*(.*)[(\376\d+\377)\s]*$/$1/;
			$NOTE = $note;
			last;
		}
		else{
			next;
		}
	}
}

###############################################################################

# This function returns the abstract of a defect from the abstractlist file

sub get_abstract{

	local(*abstract) = @_;
	local(*ABSTR_LIST);
        local($defect,$family) = split(/:/,$DEFECT,2);
	open(ABSTR_LIST,"<$ABSTRACTLIST");
	while (<ABSTR_LIST>){
		chop;
		# Remove leading spaces.  Split may not correctly get 
		# abstract if they remain.
		$_ =~ s/^ *//;
		# Check for defect with ($DEFECT) and without ($defect)
		# CMVC_FAMILY.  Old file format will not have CMVC_FAMILY
		# and defects would have only come from $DEFAULT_CMVCFAMILY.
		if ( /^0*$DEFECT / || 
                     ( /^0*$defect / && 
                       $family eq $ENV{'DEFAULT_CMVCFAMILY'}) ){
			($defect,$abstract) = split(/  /,$_,2);
			last;
		}
	}
}

###############################################################################
