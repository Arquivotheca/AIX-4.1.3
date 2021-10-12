#! /usr/bin/perl
# @(#)60	1.12  src/bldenv/pkgtools/subptf/bldaddptfids.pl, pkgtools, bos412, GOLDA411a 6/24/93 15:05:59
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS:    bldaddptfids
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1992
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# FUNCTION: Assign PTFIDs to the PTF groups; a PTFID may be a real ID from
#           Retain or a dummy ID.  
#
#           PTFIDS are associated with APARS which are associated
#	    with defects; a PTFID assigned to a group must be indirectly
#	    associated with one of the defects in the group.
#
# INPUT: 
#   idsource (parameter) - specifics if real PTFID are being used
#   dummyid (parameter) - if a dummy id used, this is the seed - where to start
#   GROUPFILE (parameter) - file of PTF groups
#   DEFECTAPARS (parameter) - file of defects and associated unused APARS
#   APARSPTFIDS (parameter) - file of apars and associated PTFIDs
#   APARSPTFIDSORG (parameter) -  the complete list of all preallocated PTFIDs
#
# OUTPUT:
#   APARSPTFIDS - the list of unused PTFID are written back to the file
#   <stdout> - the PTF group lines, with PTFIDs instead of a group numbers
#
# SIDE EFFECTS: 
#	A temporary file, TMPAPARPTFID, is created and removed at the end of
#	this command
#
# FORMATS: (each file format if for one line - multiple lines are in each file)
#   APARSPTFIDS 	- <APAR>|<PTFID>
#   DEFECTAPARS 	- <DEFECT>|<APAR>
#   GROUPFILE 		- <GROUP>|<FILENAME>|<DIRECT DEFECTS>|<INDIRECT DEFECTS>
#   APARSPTFIDSORG 	- <APAR>|<PTFID>
#   <stdout> 		- <PTFID>|<FILENAME>|<DIRECT DEFECTS>|<INDIRECT DEFECTS>
#
#   Example GROUPFILE:
#     1|!!/ship/usr/lpp/diagnostics/dskt/dsktimage|bos320.43405,bos320.44547|
#     1|!!/ship/usr/ccs/lib/libc.a.min|bos320.43405|
#     1|!!/ship/lib/inst_updt/libc.a/shr.o|bos320.43405|
#     1|!!/ship/lib/inst_updt/libs.a/shr.o|bos320.43405|bos320.41002
#     1|!!/ship/lib/inst_updt/libodm.a/shr.o|bos320.44547|
#     2|!!/ship/bin/ipcrm|bos320.35773|
#     3|!!/ship/usr/bin/mail|bos320.24568|
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (fatal)
#

# Declare common perl code/constants.
push(@INC,split(/:/,$ENV{"PATH"})); # define search path
do 'bldperlconst';      # define constants
do 'bldperlfunc';       # define general functions
do 'bldperllog';        # init perl interface to log functions

chop($tmppath=`ksh bldtmppath`); ## path for tmp files

# &logset("+l -c$0");  # init log function
%SIG=('HUP','Abort','INT','Abort','QUIT','Abort',
      'TERM','Abort','ABRT','Abort');
($idsource,$dummyid,$groupfile,$defectaparsfile,$aparptfidsfile,$aparptfidsorg)
    = (@ARGV);

## get prefix for generating dummy ptfids -- just in case needed
if ($ENV{"PTFPREFIX"} eq undef) {
     ## use BLDCYCLE, format is expect to be like 9214 or 9214A
     $cycle=$ENV{"BLDCYCLE"}; $cycle =~ s/^..//;  ## delete first 2 chars
     if ($cycle =~ /[A-Y]$/) ## if alpha on the end, use it for prefix
          { $prefix=chop($cycle); $prefix .= $cycle; }
     else ## if no alpha on end, then use 'Z' for prefix
          { $prefix="Z" . $cycle; }
}
else {  ## if prefix for dummy id is defined in the environment, then use it
     $prefix=$ENV{"PTFPREFIX"};
}

$familycode = 0;

## read file which maps APARs to PTFIDs
open (APARSPTFIDS,"<$aparptfidsfile") || 
     &log('-x',"unable to open $aparptfidsfile ($!)");
while (<APARSPTFIDS>) {
     chop;
     ($apar,$ptfid) = split(/\|/);
     $aparptfid{$apar} .= $ptfid . ",";
}

## read file which maps defects to APARs, then map the defect to the PTFID
## associated with the APAR
open (DEFECTAPARS,"<$defectaparsfile") || 
     &log('-x',"unable to open $defectaparsfile ($!)");;
while (<DEFECTAPARS>) {
     chop;
     ($dnum,$apar,$release,$family) = split(/\|/);
     #  Append a "family identifier" to the defect number to distinguish
     #  the same number from different families.
     if ( $family eq undef ) {
         &log("$lvar -e","unable to determine family for defect $dnum in aparfile $workpath");
     }

     if ($families{$family} eq undef) {
         $familycode = $familycode +1;
         $families{$family} = $familycode;
     }
     $fcode = $families{$family};
     $cmvcfam{$fcode} = $family;
     $defect = $dnum . "." . $fcode;
     $defectapar{$defect} .= $apar;
     $defectptfids{$defect} .= $aparptfid{$apar};
}


## sort the PTFIDs associated with each defect; this will put the first PTFID
## created in retain on the front of the list so it will be used first -- in
## most cases it will be the only one used; the first PTFID created in retain 
## for an APAR has special data associated with it
foreach $defect (keys %defectptfids) { 
     $defectptfids{$defect} = &Uniquesort($SEP,$defectptfids{$defect});
}

## A PTF group can have an undetermined number of defects contained in it;
## each defect has at least one PTFID assigned to it; therefore, when looking
## for a potential PTFID to assign to a group, all the defects associated
## with the group may need to be known; for this reason, the group lines 
## cannot be processed one-by-one, but must first be all read so the complete
## set of defects associated with the group can be accumulated.
open (GROUPFILE,$groupfile) || 
     &log('-x',"unable to open $groupfile ($!)");
while ($inputline=<GROUPFILE>) {
     chop $inputline;
     ($group,$file,$direct,$indirect) = split('\|',$inputline);
     foreach $dtoken (split(/$SEP/,$direct . $SEP . $indirect)) {
          next if $dtoken eq undef;
          ($release,$dnum)=split(/\./,$dtoken);
          #  Append the "family identifier" to the defect number so that
          #  the same number from different familes is distinguished.
          $fcode = 0;
          if ($releaseFamily{$release} eq undef) {
              chop($family=`ksh bldgetfamily $release`);
              if ( $family eq undef ) {
                  &log("$lvar -e","unable to determine family for release $release");
              }
              elsif ($families{$family} eq undef) {
                  &log("$lvar -e","family $family was not found for any release in aparfile $workpath.");
              }
              else {
                  $releaseFamily{$release} = $families{$family};
                  $fcode = $releaseFamily{$release};
              }
          }
          else {
              $fcode = $releaseFamily{$release};
          }
          $defect = $dnum . "." . $fcode;
          $inputlines{$inputline}=$DEFINED;   ## save line for reprocessing
          $groupdefects{$group} .= $defect . $SEP;  ## save defects in group
     }
}

## open the file of predefined PTFIDs in case a dynamically generated PTFID
## (generated because not enought predefines) must be added to the list
open (APARSPTFIDSORG,">>$aparptfidsorg") || 
     &log('-x',"unable to open $aparptfidsfile ($!)");

## now reprocess the GROUPFILE data, replacing PTFIDs for group numbers
foreach $inputline (sort keys %inputlines) {
     ## parse the line orginally from GROUPFILE
     ($group,$file,$direct,$indirect) = split('\|',$inputline);
     ## if a ptfid has not already been assigned to the group
     if (($ptfid=$groupptfid{$group}) eq undef) {
          ## if the PTFIDs are coming from Retain
          if ($idsource eq "retain") {
               ## get the first unused PTFID that is assigned to the defect
               foreach $defect (split(/$SEP/,$groupdefects{$group})) {
                    foreach $aptfid (split(/$SEP/,$defectptfids{$defect})) {
                         next if ($usedptfid{$aptfid});
                         $ptfid=$aptfid; last;
                    }
               }
               ## if there are no more unused PTFIDs left (all the predefined
               ##    PTFIDs are used up)
               if ($ptfid eq undef) {
                    ## get another PTFID directly from Retain
                    $apar=$defectapar{$defect};
                    ($dnum,$fcode) = split('.',$defect);
                    $family = $cmvcfam{$fcode};
                    $ptfid=`bldgetptfid $dnum $apar $family`;
                    if ($?) {  ## if failed to get PTFID from Retain
                         &log("-e","bldgetptfid failed");
                         undef $ptfid;
                    } 
                    else {  ## if successfully got PTFID from Retain
                         chop ($ptfid);
			 ## add the PTFID to the list of predefined IDs; if
			 ## neccessary to rerun, this ID is now predefined
                         print APARSPTFIDSORG "$apar|$ptfid\n";
		 	 if ( $! != 0 ) {
	  			&log("-x","$aparptfidsorg write error ($!)");
			 }
                         &log("-b +l","PTFID $ptfid retrieved from Retain");
                    }
               }
          }
          ## if a PTFID has not yet been defined, use a dummy ID
          if ($ptfid eq undef) {
               $ptfid=$prefix . $dummyid++;     
          }
     }
     ## output the PTF group line with the PTFID replacing the group number
     print "$ptfid|$file|$direct|$indirect\n";
     if ( $! != 0 ) {
	    &log("$lvar -x","write error ($!)");
     }
     $groupptfid{$group} = $ptfid;  ## the group now has a PTFID
     $usedptfid{$ptfid} = $DEFINED; ## mark the PTDID as used
}

## write the unused PTFIDs to a file
$tmpaparptfidsfile="$tmppath/aparptfidsfile";
open (TMPAPARPTFID,">$tmpaparptfidsfile") || 
     &log('-x',"unable to open $tmpaparptfidsfile ($!)");
foreach $apar (keys %aparptfid) {
     foreach $aptfid (split(/,/,$aparptfid{$apar})) {
          print TMPAPARPTFID "$apar|$aptfid\n" if (! $usedptfid{$aptfid});
 	  if ( $! != 0 ) {
	 	&log("-x","$tmpaparptfidsfile write error ($!)");
	  }
     }     
}
close (TMPAPARPTFID);
# copy unused PTFIDs back to PTFID file
`cp $tmpaparptfidsfile  $aparptfidsfile` &&
     &log ("-x","unable to update $aparptfidsfile ($!)");
`rm $tmpaparptfidsfile`;

exit $SUCCESS;

sub Abort {
        exit $FATAL;
}

