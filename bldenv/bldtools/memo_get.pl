#! /usr/bin/perl
# @(#)07	1.10  src/bldenv/bldtools/memo_get.pl, bldprocess, bos412, GOLDA411a 6/1/93 19:14:12
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: memo_get
#	     write_to_memo_info
#	     remove_dup_apars
#	     get_note
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
# FUNCTION: Writes the APAR and the corresponding memo of a
#	    defect to the 'memo_info' file in the 'bldhistorypath'
# INPUT: APAR, defect, The memos file, and the memo_info file
#
# OUTPUT:
#
# SIDE EFFECTS: Also maintains a raw format of the 'memo_info' file called
#		'memo_info.raw'
#
# EXECUTION ENVIRONMENT:  Build process environment
#

$APARS = shift(ARGV);		# The APAR number
$APARS =~ s/[iI][xX]/IX/g;	# converting APAR number prefix to all caps
$DEFECT = shift(ARGV);		# The defect number
$MEMOS = shift(ARGV);		# The memos file containing notes of defects
				# of a release build in the 'bldreleasepath'
$MEMO_INFO_FILE = shift(ARGV);	# The memo_info file in the 'bldhistorypath'
$MEMO_INFO_FILE_RAW = "$MEMO_INFO_FILE.raw"; # The raw format of the 
					     # memo_info file
if (shift(ARGV) eq "-w"){
	&write_to_memo_info;
	exit 0;
}
&remove_dup_apars;

###############################################################################

# This function copies the raw abstracts file into the real abstracts file

sub write_to_memo_info{

	local(*INPUT,*OUTPUT); 
	local($apars,$note,$DLM);

	$DLM = "<@>";
	open(OUTPUT,">$MEMO_INFO_FILE");
	open(INPUT,"<$MEMO_INFO_FILE_RAW");
	while (<INPUT>){
		chop;
		local($apars,$note) = split(/\375\374/);
		$note =~ s/\376(\d+)\377/pack(C,$1)/eg;
		$note =~ s/^/ /;
		$note =~ s/\n/\n /g;
		$note =~ s/ $//;
		print OUTPUT "$apars\n\n$note$DLM\n";
	
	}
}

###############################################################################

# This function removes the duplicate apar entries in the raw abstracts file

sub remove_dup_apars{
	local($printed,$apars,$note);
	local(*RAWINPUT,*RAWOUPUT);

	&get_note($MEMOS,*NOTE);
	#exit 0 if ($NOTE eq /^$/);
		# making a backup of the raw file
	rename("$MEMO_INFO_FILE_RAW","$MEMO_INFO_FILE_RAW.bak"); 
	open (RAWINPUT,"<$MEMO_INFO_FILE_RAW.bak");
	open (RAWOUTPUT,">$MEMO_INFO_FILE_RAW");
	while (<RAWINPUT>){
		chop;
		($apars,$note) = split(/\375\374/);
		$apars =~ s/[iI][xX]/IX/g;
		if ($APARS eq $apars){
			print RAWOUTPUT "$APARS\375\374$NOTE\n"
				if ($NOTE ne "");
			$printed = 1;
		}
		else{
			print RAWOUTPUT "$apars\375\374$note\n";
		}
	}
	print RAWOUTPUT "$APARS\375\374$NOTE\n"
		if((! $printed) && ($NOTE ne ""));
	close(RAWINPUT);
	unlink "$MEMO_INFO_FILE_RAW.bak";
	close(RAWOUTPUT);
}

###############################################################################

# This function returns the symptom string from the symptoms file for a defect

sub get_note{

	local($file,*NOTE) = @_;
	local($defect);
	local(*INPUT);
	local($note);
	open (INPUT,"<$file");
	while (<INPUT>){
		chop;
		($defect,$note) = split(/\|/,$_,2);
                if ( ! ($defect =~ /.*:.*/) )
		{
			$defect .= ":" . $ENV{'DEFAULT_CMVCFAMILY'};
		}
	
		# if the defect is found, Change the unprintable charactors
		# which were converted from newlines by 'sym_lookup' in 
		# 'prebuild' to newlines in the corresponding note and write 
		# the note to the 'abstracts' file in the releasepath
	
		if ($DEFECT eq $defect){
			$note =~ s/^[(\376\d+\377)\s]*[sS][tT][aA][rR][tT]_[Mm][Ee][mM][Oo](.*)[sS][tT][oO][pP]_[mM][Ee][Mm][oO][(\376\d+\377)\s]*$/$1/;
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
